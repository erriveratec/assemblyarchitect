# Current Code Window

Inventory at bootstrap revision `18b5d18`. This describes observed behavior, not
an approved design or a claim that every path is correct.

## Files and State

| Existing file | Responsibility |
| --- | --- |
| `source/code_window_cw.c` | Roughly 3,000 lines: list ownership, edit/reorder/delete, operand selection, labels/jumps, save-line construction, execution-state changes, layout, drawing, animation claims |
| `source/code_window_cw.h` | Public `cw_*` compatibility surface, SDL rectangles, borrowed line pointers; MAX_CODE_LINES 99 |
| `source/code_window_cw_internal.h` | Pure rectangle/highlight helpers and CW_CHALLENGE_TEXT_TESTING factory seam |
| `source/code_line_cl.{c,h}` | Instruction/operand/line allocation, destruction, IDs, state, button ownership |
| `source/list.{c,h}` | Generic nodes; values are managed by callers |
| `source/stages.c` | Edit/input sequencing, held-line state, stage initialization/destruction, run/reset/result orchestration |
| `source/storage/save_sv.c`, `source/file_fl.c` | Text saves, level configuration, code serialization and storage delegation |
| `source/game_mechanics_mc.c` | Execution, register/buffer transfers, jump dispatch |
| `source/gameplay/code_state_cs.c` | Held-line and code-state queries for tutorial/interaction decisions |

The code window has a file-static `List *code_list`; code/scroll/text rectangles;
mutable CODE_BOX_W/CODE_BOX_Y; challenge/code-box highlight flags and phase values;
and three active jump button-animation claims which borrow buttons and snapshot
their prior animation fields. `g_challenge_text`, `g_stage_name`, `g_comma_tex`, and
`g_numbers` are non-static globals. A factory callback exists only under the test
define. Stages and code_state also retain edit/held-line state across frames.
SDL renderer/font/window globals live in `source/sdl_config.c`.

## APIs and Main Callers

The header exposes list creation/clear/destruction; saved-line construction and
post-load jump repair; line/operand getters; click/right-click/operand detection;
pending-state cancellation; held-instruction editing; layout/scroll setters and
queries; drawing/highlights; sorting and execution reset/jump-state mutation.
The public API includes `code_line_t *` and `operand_t *`, not opaque model IDs.
Internal non-static helpers such as `get_code_list` are not all public-header APIs.

Stages call editing, sorting, drawing, and cleanup. Level initialization creates
the list, configures assets, and loads saved code. Storage calls `cw_add_saved_line`
and `cw_update_saved_jump_instructions`; file serialization queries instruction and
operand IDs. Game mechanics calls jump/execution APIs. Tutorials, operand highlights,
arrows, and code_state inspect code, pending operands, geometry, and execution state.

## Ownership and Lifetime

The list owns its inserted line values by convention: `cw_destroy_code_window_assets`
destroys line objects before destroying nodes/list. `cw_clear_code_list` destroys
values/nodes but retains the list container. `List_destroy` does not destroy values.
A line owns its instruction and operands, which own buttons/textures through their
destructors. `operand_t.jptr` borrows a target line; it must not free that line.
Getters return borrowed lines invalidated by removal/clear/destruction. New cloned
lines, labels, and jump operands are allocated values transferred to their caller
or list/line owner; ownership is mostly implicit in the current API.

Held-line pointers and animation claims can outlive an individual frame. Code
releases animation claims before relevant operand/list changes. Proving all partial
initialization and concurrent animation-writer paths safe remains outside current
coverage; do not infer safety from normal destructor tests alone.

## Labels, Jumps, and Persistence

`operand_t` contains a button, integer ID, and `code_line_t *jptr`. There is no
separate stable logical label identifier. Display labels are regenerated; jump
operand IDs encode current list positions, while jptr tracks the in-memory target.
After edits, `update_label_instructions` rebuilds label operands and
`update_jump_instructions` rebuilds jumps or marks missing targets MISSING_OP1.
Deleting a jump through the held-instruction path also removes its target label.
Shared-target behavior and all deletion-order combinations lack dedicated coverage.

Storage format v1 uses `[player]`, `[level NN]`, `unlocked`, `code_begin`/`code_end`.
`cw_add_saved_line` mutates its input token buffer, constructs SDL-backed buttons,
and appends a line. Jumps initially store numeric positions, then storage performs
one post-load repair after every line has been read. `cw_operate_jump_instruction`
uses list positions and jptr to mark intervening lines COMPLETE or EXECUTED.
`file_fl.c` serializes instruction/operand IDs; incomplete ordinary operands use `_`.
Save updates write a `.tmp` file and rename it. No save round-trip guarantee is
established by the present test inventory.

## Coupling and Existing Evidence

Semantic lines contain UI buttons. Construction needs text textures, renderer,
fonts, dimensions, and operand factories from registers/buffers/immediates.
The held-instruction function both edits the list and moves/draws buttons using
mouse state. Drawing shares animation, escape-menu, and typography state. Tutorials
consume pending/execution/highlight state; stages coordinate frame order and results.

All 18 tests are listed in the bootstrap report. Most relevant are
operand_change_selection, operand_highlights, tutorial_text (challenge lifecycle),
highlight_animation_sync, modal_pointer_capture, run_result_render_order,
execution_arrow_geometry, tutorial_arrow_animation, text_box_layout, and
display_resize. Other tests cover buffers/registers/value transport/config/outline.
Several include production C directly with macro substitutions and Make link
exclusions. Tests use controlled animation phases/software rendering, not real
window/audio acceptance. Missing dedicated evidence: reorder target preservation,
referenced-label deletion combinations, malformed saves/forward-backward round
trips, allocation-failure rollback, and complete renderer semantic non-mutation.
