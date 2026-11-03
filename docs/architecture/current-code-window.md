# Current Code Window

Inventory began at bootstrap revision `18b5d18`; the local migration changes are
listed below. This describes observed behavior, not a claim that every invariant
is proven.

## Files and State

| Existing file | Responsibility |
| --- | --- |
| `source/code_window_cw.c` | Orchestration and remaining compatibility implementation: SDL-backed line construction and clone factories, label/jump operand rebuilding, save entry points, hit acquisition, layout coordination, scrolling and animation claims |
| `source/code_program_cp.{c,h}` | Ordered line owner using existing `List`; production and direct-source fixtures use read-only indexed and iterator APIs, with no raw-list compatibility view |
| `source/control_flow_cf.{c,h}` | Production stable identity assignment, v1 positional target translation, target repair/resolution, validation and jump execution-state intervals; SDL operand presentation remains in `cw_*` |
| `source/code_layout_cly.{c,h}` | Pure value-based rectangle, spacing, offset, row-position and strict-boundary hit calculations; some geometry remains in `cw_*` |
| `source/code_edit_ce.{c,h}` | Explicit edit-session state, result-valued operand/line mutations, reusable result-valued deep cloning, and pending-operand queries used by production callers |
| `source/code_interaction_ci.{c,h}` | Pure ordered hit-result-to-action translation, used by the stage edit dispatcher |
| `source/code_renderer_cr.{c,h}` | Line/operand/grouped-label/comma/line-number, challenge/code-box and stage-title presentation with explicit renderer texture ownership |
| `source/code_window_cw.h` | Public `cw_*` compatibility surface, SDL rectangles, borrowed line pointers; MAX_CODE_LINES 99 |
| `source/code_window_cw_internal.h` | Pure rectangle/highlight helpers and CW_CHALLENGE_TEXT_TESTING factory seam |
| `source/code_line_cl.{c,h}` | Instruction/operand/line allocation, destruction, IDs, state, button ownership |
| `source/list.{c,h}` | Generic nodes; values are managed by callers |
| `source/stages.c` | Edit/input sequencing, held-line state, stage initialization/destruction, run/reset/result orchestration |
| `source/storage/save_sv.c`, `source/file_fl.c` | Text saves, level configuration, code serialization and storage delegation |
| `source/game_mechanics_mc.c` | Execution, register/buffer transfers, jump dispatch |
| `source/gameplay/code_state_cs.c` | Held-line and code-state queries for tutorial/interaction decisions |

The current working tree consolidates the program, edit session, renderer assets,
challenge/stage assets, label allocator, code/scroll/text rectangles,
CODE_BOX_W/CODE_BOX_Y, highlight flags/phases, pending scroll and three active
jump button-animation claims in one private `code_window_state_t`. It remains one
compatibility singleton, not independent contexts. A challenge text factory
callback exists only under the test define. `stages.c` still retains held-line
state across frames. SDL renderer/font/window globals live in `source/sdl_config.c`.

## APIs and Main Callers

The header exposes program lifecycle; saved-line construction and post-load jump
presentation repair; click/right-click/operand detection; pending-state
cancellation; held-instruction editing; layout/scroll setters and queries;
drawing/highlights and sorting. Semantic execution reset, operand-slot state
transitions, and pure instruction spacing are delegated to `control_flow_cf`,
`code_edit_ce`, and `code_layout_cly`; the reset coordinator remains in `cw_*`
to release presentation animation claims.
The public API includes `code_line_t *` and `operand_t *`, not opaque model IDs.
Program membership, ordering, count and indexed reads use `code_program_cp`;
execution jump transitions use `control_flow_cf` directly.

Stages call editing, sorting, drawing, and cleanup. Level initialization creates
the program, configures assets, and loads saved code. Storage calls
`cw_add_saved_line` and `cw_update_saved_jump_instructions`; file serialization
reads ordered lines from `code_program_cp`. Game mechanics applies jump transitions
through `control_flow_cf`. Tutorials, operand highlights, arrows, and code state
use owning model/edit modules for migrated reads, while geometry and hit acquisition
still use controller adapters.

Right-click cloning routes through `cw_clone_rclicked_line` to
`ce_clone_line`. The edit-layer API owns rollback and editable-state derivation;
the controller factories construct fresh SDL-backed instruction and operand
objects. The stage only enters held-line placement after `CE_OK`. LABEL lines
remain ineligible for right-click cloning, matching `cw_chk_rclick_code`.

## Ownership and Lifetime

`code_program_cp` owns inserted line values and destroys them through the registered
line destructor. `cw_destroy_code_window_assets` destroys program lines before
destroying the model; renderer textures have separate process lifetime and are
released by `cw_destroy_renderer_assets` at app shutdown. `cw_clear_code_list` destroys
values/nodes but retains the list container. `List_destroy` does not destroy values.
A line owns its instruction and operands, which own buttons/textures through their
destructors. `operand_t.jptr` borrows a target line; it must not free that line.
Getters return borrowed lines invalidated by removal/clear/destruction. New cloned
lines, labels, and jump operands are allocated values transferred to their caller
or list/line owner; ownership is mostly implicit in the current API.

`ce_replace_operand` transfers the new operand to the line on `CE_OK` and returns
the previous operand to the caller, which must destroy it only if non-NULL. Empty
slots return NULL. On failure, the caller retains the new operand. The
`cw_assign_op_to_line` adapter consumes its argument, destroying it on failure;
on success it destroys only a non-NULL displaced operand. `cl_destroy_operand`
continues to require a non-NULL argument. Edit and controller-selection tests
cover empty slots, state transitions and exactly-once replacement cleanup.

`ce_clone_line` returns `CE_OK` with one complete, independent line owned by the
caller. On failure it returns an explicit status, leaves the output NULL, and
destroys all successfully constructed clone objects. Its presentation factories
must return fresh instruction/operand objects with independent buttons and
textures. Present operands are copied; missing slots remain missing, and
execution/selection presentation state is normalized from the cloned slots.
Ordinary operands have no jump metadata. Jump operands own their UI objects but
borrow the source's same LABEL through `jptr` and copy its stable target identity;
insertion preserves the jump operand and button while refreshing its owned label
texture and positional display ID. Clone coverage is in
`code_line_clone_test`, including factory-failure rollback and held-line placement.

Jump replacement validates the target LABEL and its nonzero stable identity,
not the positional operand `id`; ordinary REG/BUF/IMM validation is unchanged.
The session has no program reference, so the controller checks target membership
and establishes label identity before calling the edit module. A missing operand
target identity is filled from the label on successful replacement; a conflicting
identity is rejected without replacing the operand.

`cw_create_automatic_jump_label` is the temporary Stage 10/11 stage/controller
adapter for new jumps. It inserts one candidate label, keeps the existing held-line
positioning behavior, then uses a private result-returning assignment helper.
The helper transfers the operand only on success; failure retains caller ownership.
The transaction removes and destroys its candidate label and destroys any unassigned
candidate operand on failure. On success the jump is COMPLETE and its edit session
is idle. The existing void assignment wrapper retains consume-on-failure behavior.
`automatic_jump_label_test` covers the real stage/controller path, repeated updates,
insertion/operand-allocation/invalid-target rollback, replacement cleanup and target
highlighting, off-program rejection, and forward/backward v1 loading.

Held-line pointers and animation claims can outlive an individual frame. Code
releases animation claims before relevant operand/list changes. Proving all partial
initialization and concurrent animation-writer paths safe remains outside current
coverage; do not infer safety from normal destructor tests alone.

## Labels, Jumps, and Persistence

`operand_t` contains a button, integer ID, borrowed `code_line_t *jptr`, and the
working tree's program-local `target_label_identity`. Labels hold a nonzero
`label_identity`; IDs are assigned after saved lines are loaded and retained when
labels move. Display labels are regenerated; jump operand `id` continues to encode
the current list position for v1 persistence, while execution resolves identity
through `control_flow_cf`.
After edits, `control_flow_cf` repairs logical target links while
`update_label_instructions` and `update_jump_instructions` still rebuild SDL
label/jump operands or mark missing targets MISSING_OP1.
Deleting a jump through the held-instruction path also removes its target label.
Shared-target behavior and all deletion-order combinations lack dedicated coverage.

Storage format v1 uses `[player]`, `[level NN]`, `unlocked`, `code_begin`/`code_end`.
`cw_add_saved_line` mutates its input token buffer, constructs SDL-backed buttons,
and appends a line. Jumps initially store numeric positions, then storage performs
one post-load repair after every line has been read. `cf_apply_jump_execution`
uses resolved target identity and program order to mark the existing forward and
backward execution intervals COMPLETE or EXECUTED.
`file_fl.c` serializes instruction/operand IDs; incomplete ordinary operands use `_`.
Save updates write a `.tmp` file and rename it. No save round-trip guarantee is
established by the present test inventory.

## Coupling and Existing Evidence

Semantic lines contain UI buttons. Construction needs text textures, renderer,
fonts, dimensions, and operand factories from registers/buffers/immediates.
The held-instruction function both edits the list and moves/draws buttons using
mouse state. Drawing shares animation, escape-menu, and typography state. Tutorials
consume pending/execution/highlight state; stages coordinate frame order and results.

All original tests remain registered. Standalone tests cover layout, program,
control flow, edit, interaction and renderer non-mutation. The code-window
implementation is currently 2,361 lines versus 3,127 at baseline, with 39 public
`cw_*` functions. Raw-list
compatibility and model pass-throughs have been removed; challenge/code-box
presentation and pending-state queries moved to owning modules. It remains above
the requested 1,800-line maximum, with edit orchestration and substantial layout,
interaction and control-flow presentation adaptation still in cw. Most relevant
tests are operand_change_selection, operand_highlights,
tutorial_text (challenge lifecycle),
highlight_animation_sync, modal_pointer_capture, run_result_render_order,
execution_arrow_geometry, tutorial_arrow_animation, text_box_layout, and
display_resize. Other tests cover buffers/registers/value transport/config/outline.
Several include production C directly with macro substitutions and Make link
exclusions. Tests use controlled animation phases/software rendering, not real
window/audio acceptance. Remaining evidence gaps include allocation-failure
rollback, full edit rollback across allocation and list failures, active-renderer
pixel equivalence, complete renderer non-mutation snapshots, and real gameplay
save/load smoke testing on this continued tree.
