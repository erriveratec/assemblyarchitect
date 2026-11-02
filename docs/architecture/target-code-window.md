# Proposed Code-Window Architecture

Status: target architecture; partial modules now exist in the working tree.
See migration-state.md for evidence and uncompleted boundaries. Use ASCII to avoid
introducing a diagram-renderer dependency.

```text
SDL events -> code_interaction_ci -> code_edit_ce -> code_program_cp
                                              \-> control_flow_cf
program + code_layout_cly + visual state -> code_renderer_cr -> SDL
code_window_cw coordinates the above and temporarily adapts old callers
```

| Module | Owns / proposed API shape | Legal dependencies | Prohibited dependencies |
| --- | --- | --- | --- |
| code_program_cp | Explicit program context, owned instructions, immutable views; create/destroy/query with status results | C standard library, model types | SDL, buttons, renderer, mouse, storage policy |
| control_flow_cf | Stable label resolution and validation; resolve/validate against program view | Program types and logical IDs | UI pointers, display positions as sole identity, SDL |
| code_edit_ce | Commands and edit-session state; apply command transactionally with status | Program/control flow | SDL event polling, rendering, file I/O |
| code_layout_cly | Pure geometry input/output values; calculate layout/hit regions | Value types and explicit dimensions | Globals, semantic mutation, asset creation |
| code_interaction_ci | Translate event + hit region + session into command | Layout, edit command types, SDL adapter at boundary | Direct list ownership/mutation, storage |
| code_renderer_cr | Draw immutable program/layout/visual snapshots; presentation resource lifetime | SDL, fonts, drawing, read-only views | Edit application, execution decisions, saving |
| code_window_cw | Explicit coordinator context and temporary legacy cw_* wrappers | Boundary modules | New singleton state or duplicated domain logic |

Model APIs should accept explicit contexts, return structured statuses, and
document borrowed views versus transferred values. These are API shapes, not
final symbol names. Do not prescribe a speculative framework before stage tests.
The model, control flow, and edit commands must run without SDL video or renderer.
Keep pure geometry independent of renderer handles even if adapters use SDL_Rect.

Keep existing `cw_*` callers working through wrappers naming Stage 10/11 removal.
Characterize before moving responsibilities. First preserve v1 positional save
translation at the storage boundary; do not serialize raw addresses or introduce
an unapproved disk schema. Run old and new representations against the same tests
during migration, with one authoritative owner at a time. Never duplicate helpers
to simulate separation, move every caller at once, or combine extraction with a
semantic fix. Compatibility rollback must restore a stage without losing saves.

Current working-tree progress: `code_layout_cly` owns selected pure geometry and
Current working-tree progress: `code_layout_cly` owns selected pure geometry and
hit calculations; `code_program_cp` owns ordered lines and lifetime with no raw
List view exposed; `control_flow_cf` owns identity assignment, v1 translation,
target repair, resolution, validation and jump execution-state intervals;
`code_edit_ce` owns operand/line mutations, pending-state queries and explicit
replacement session state; `code_interaction_ci` translates ordered hit results
in the production stage dispatcher; and `code_renderer_cr` draws lines, operands,
labels, commas, line numbers, challenge text and the code box. These partial
modules do not establish the full dependency boundaries shown above. Semantic
edit orchestration, remaining geometry, input acquisition, independent contexts,
wrapper retirement and material controller reduction are outstanding. The
controller remains above its requested 1,800-line maximum.
