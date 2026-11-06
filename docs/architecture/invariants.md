# Testable Invariant Catalog

Severity: Critical = corruption, unsafe memory, or educationally wrong execution;
Major = broken workflow/incorrect UI-state relationship; Minor = localized defect;
Suggestion = optional improvement. Evidence is not proof beyond the cited slice.

| ID | Statement | Why | Current evidence / uncertainty | Verification level | Severity |
| --- | --- | --- | --- | --- | --- |
| AA-MD-001 | Insertion, movement, deletion preserve all non-target instructions and ordering | Student intent must survive edits | cw_player_holding_instruction; dedicated command characterization missing | Unit, integration | Critical |
| AA-MD-002 | Failed edits leave semantic program state unchanged | Invalid operand choices must not corrupt work | code_edit_ce_test checks diagnostic classification and unchanged session/operand pointers; controller capture test checks modal dismissal; broader allocation rollback remains unproved | Unit, sanitizer | Critical |
| AA-CF-001 | Every complete jump resolves to exactly one valid label identity | Branches must teach deterministic control flow | jptr plus positional IDs; missing/invalid targets need characterization | Unit, integration | Critical |
| AA-CF-002 | Reordering cannot silently change the semantic target | Display order is not target identity | repair functions preserve surviving jptr; reorder test missing | Unit, integration | Critical |
| AA-CF-003 | Referenced-label deletion has a defined result, never a dangling pointer | Prevent UAF and accidental branches | update_jump marks MISSING_OP1; deletion/shared-target combinations unproved | Integration, sanitizer | Critical |
| AA-CF-004 | Forward/backward jumps load deterministically after all lines exist | Saved loops/branches must retain meaning | sv_load_level_code runs post-load repair; round-trip tests missing | Save fixture, integration | Critical |
| AA-SV-001 | Existing v1 saves remain readable, including incomplete operands | Preserve student progress | Three tracked player saves; loader supports `_`; full fixtures untested | Save fixture, manual smoke test | Critical |
| AA-SV-002 | Save/load round trips preserve instructions, targets, unlock state | Persistence must not alter a solution | Positional serialization + temporary-file replacement; round-trip seam missing | Save fixture, integration | Critical |
| AA-RN-001 | Drawing and hit testing do not mutate semantic program state | Looking at a solution cannot change it | Geometry helpers tested; renderer shares line/button state; whole-model snapshot test missing | Unit, headless UI | Critical |
| AA-RN-002 | Highlights and arrows correspond to real pending/execution state | Avoid misleading instructional feedback | operand_highlights, highlight_animation_sync, tutorial_arrow_animation, execution_arrow_geometry | Headless UI | Major |
| AA-RN-003 | Canonical 1920x1080 geometry survives physical resizing | Consistent interaction coordinates | display_resize, text_box_layout, drawing_outline | Unit, headless UI, manual smoke test | Major |
| AA-IN-001 | Modal input is captured and boundaries select the intended item only | Prevent hidden edits during dialogs | modal_pointer_capture and instruction_feedback_modal_test cover aggregate consumption, right-button policy, dismissal, and held-release suppression; ax_chk_mouse_hover_rect uses strict interior boundaries | Unit, integration | Major |
| AA-IN-002 | Edit-session cancellation and operand replacement preserve valid prior values | Predictable editing | operand_change_selection; broader command rollback missing | Integration | Major |
| AA-MM-001 | Every allocation has one owner; borrowed targets outlive use | No leaks, UAF, double frees | cl destructors/list value separation; globals/held pointers need audit | Sanitizer, integration | Critical |
| AA-MM-002 | Partial init, destroy, and repeated cycles are safe | Resource errors must not corrupt later sessions | tutorial_text initializes/destroys all configured feedback text; instruction_feedback_modal_test covers repeated controller init/destroy; allocation-failure injection remains absent | Unit, sanitizer | Critical |
| AA-MM-003 | Cloning creates independent owned UI/model objects and rolls back atomically | A copied instruction must survive independent edits/deletion | `ce_clone_line`; `code_line_clone_test` covers ordinary deep copies, unresolved JMP/JE/JNE copies, incomplete slots, held placement, cancellation, and construction failure; `automatic_jump_label_test` covers cloned-jump label transaction rollback | Unit, integration, sanitizer | Critical |
| AA-CF-005 | Cloning JMP/JE/JNE restarts destination selection and never inherits the source LABEL | A copied branch must receive an independent target only after placement | `code_line_clone_test` checks unresolved state, exactly one new label after placement, repeated updates, independent source/clone highlights, reorder repair, and v1 serialization/reconstruction; `automatic_jump_label_test` checks transaction rollback for cloned jumps | Unit, integration | Critical |
| AA-ED-001 | MOV/ADD/CMP/JMP/JE/JNE and IB/OB/register behavior preserve represented semantics | Educational correctness is functional | Execution in game_mechanics; no full semantic suite | Unit, integration, manual smoke test | Critical |
| AA-ED-002 | Levels/tutorial progression, result/unlock, reset/retry remain unchanged | Preserve learning sequence | sector_completion_navigation, tutorial_text, run_result_render_order | Integration, manual smoke test | Major |
| AA-DT-001 | Tests use deterministic input/clock advancement, no arbitrary sleeps | Detect genuine regressions reproducibly | Existing animation tests advance explicit phase; no blanket stability guarantee | Unit, headless UI | Major |

Stages must name the IDs they preserve, add failing-before/fixed-after coverage
where feasible, and record unresolved uncertainty rather than declaring compliance.
