# Migration State

| Stage | Status | Pull request | Compatibility wrappers | Required checks | Notes |
| --- | --- | --- | --- | --- | --- |
| 0 Inventory/bootstrap | Complete | Merged to `main` | Existing cw_* unchanged | Local tests, hosted CI, CodeQL, and manual smoke test | Hosted validation passed on commit `7d29cea`; see bootstrap-report.md |
| 1 Characterization | Complete | Local working-tree change | Existing cw_* | Full tests, headless, sanitizer | Reorder, shared-label deletion, forward/backward v1 round trip, `_`, repeated lifecycle; renderer has a semantic snapshot non-mutation test |
| 2 Explicit context | In progress | Local working-tree change | cw_* wrappers annotated for Stage 10/11 | Focused lifecycle, full suite, sanitizer | Private code-window state consolidated; still one legacy owner, no independent public contexts |
| 3 Pure layout | In progress | Local working-tree change | Live layout/render adapters retained | Standalone geometry, resize, boundaries, headless | Rectangle, spacing, offsets, row hit testing and line-position helpers extracted; instruction-window spacing now calls cly directly; controller coordinates remaining geometry |
| 4 Program model | Complete | Local working-tree change | No raw-List compatibility API remains | SDL-independent model tests, full suite, sanitizer | Model owns ordered lines; production and direct-source fixtures use indexed model reads or its read-only iterator |
| 5 Label identity | In progress | Local working-tree change | v1 positional translation retained | Jump/reorder/save fixtures | In-memory identities assigned and used for execution; production constructors initialize IDs; further path/fixture review remains |
| 6 Control flow | In progress | Local working-tree change | cw saved-load and reset orchestration retained | Delete/invalid target, sanitizer | `control_flow_cf` owns identity assignment, v1 target translation, repair, resolution, validation, execution-state intervals and semantic reset; cw reset also releases visual animation claims and still rebuilds SDL label/jump operands |
| 7 Edit commands | In progress | Local working-tree change | cw mutation adapters retained | Rollback/session tests | `code_edit_ce` owns operand replacement/cancellation, validated operand-slot selection/state transitions, line insertion/removal/movement and pending-state queries; UI hit acquisition, animation and drag/delete orchestration remain outside ce |
| 8 Interaction | In progress | Local working-tree change | cw hit adapters retained | Modal/boundary tests | `code_interaction_ci` translates ordered stage hit results; SDL hit acquisition and some pointer/session state remain in stages/cw |
| 9 Renderer | In progress | Local working-tree change | cw draw coordinator retained | Non-mutation/phase/pixel checks | `code_renderer_cr` owns line/operand/label/comma/line-number and challenge/code-box presentation; animation claims and scroll/layout coordination remain in cw |
| 10 Caller migration | In progress | Local working-tree change | Retain remaining presentation/input adapters | Caller integration, saves, smoke | Code-state, game mechanics, serialization, interaction rules, stages, levels, arrows, instruction-window spacing and direct-source fixtures use owning modules where applicable; SDL hit acquisition and presentation still use cw_* |
| 11 Wrapper removal | In progress | Local working-tree change | Remaining wrappers have production callers or presentation/orchestration ownership | All gates and invariant audit | Removed unused label-hit API, made candidate factories private, and made code-line x/y/spacing helpers controller-local; 39 public cw_* functions remain. Reset remains a visual-claim coordinator delegating semantic reset to control flow |

Only update a stage after recording actual evidence. The maintainer's task request
authorizes continuing the remaining migration locally; it does not authorize Git
write operations. Stage 4's raw-list compatibility adapter and selected unused
wrappers have been removed. Caller migration, controller reduction, and wrapper
retirement remain incomplete.

Bounded bootstrap corrections add pinned SDL DMG verification and portable relative
test-asset links. Static Analysis is manual-only via `workflow_dispatch`, currently
known to fail on four existing findings until bounded triage is completed. Healthy
headless/sanitizer nightly checks remain scheduled; no analyzer repair is included.

## Stage 0 Completion Evidence

Local and hosted validation complete. GitHub Actions passed macOS Clang,
the 18-test regression suite, Headless SDL, ASan/UBSan, Infrastructure and
Formatting, and CodeQL C/C++ analysis. Manual macOS display, audio, input,
level-loading, save-loading, execution, and restart smoke testing passed.

The application launched successfully and reported an adaptive minimum window
size of `1280x720` for display 0.

Static Analysis remains manual-only with four separately documented
pre-existing findings in `source/ui/screens/select_level_sl.c`. Completing
Stage 0 does not classify, suppress, repair, or resolve those findings.

Stages 2 through 10 remain partial working-tree changes, not completed
architecture. Stage 11 has not started. Stage 1 characterization and the
layout/model/control-flow/edit/interaction/renderer slices are recorded below.

## Local Migration Evidence

- Final cleanup continuation (2026-11-02): inventory found 45 public cw declarations
	at entry; removed the unused `cw_ms_rel_in_label`, made the SDL candidate
	factories private, and made code-line x/y/spacing helpers controller-local.
	Migrated instruction-window spacing to `code_layout_cly`; the execution-arrow
	adapter remains because it returns live button position. Added
	`ce_select_operand` so edit-session selection and semantic pending-state
	restoration belong to `code_edit_ce`, while controller hit acquisition and
	jump-animation claims remain in the UI layer. Added `cf_reset_execution` and
	retained `cw_reset_code_execution` as orchestration to release visual claims
	before semantic reset. The final controller is 2,361 lines with 39 public
	cw_* functions. Production `cw_*` callers remain for initialization/shutdown,
	stage/event/draw coordination, SDL hit acquisition and presentation, saved-line
	construction/visual jump repair, and live geometry/accessor adapters; no raw
	List or model query pass-through was restored. The automatic jump regression
	was updated to exercise private candidate factories. CE tests now cover slot
	selection, switching, cancellation restoration, and unrelated incomplete-line
	state; control-flow tests cover execution reset. Focused model/control-flow/
	jump/highlight, edit/interaction/selection/modal, layout/renderer/tutorial/
	animation/arrow groups all passed.
	`make check-infrastructure`, `make format-check`, `make -B all`, `make test`,
	`make test-headless`, `make test-sanitize`, `make verify`, and `git diff --check`
	returned 0. Normal, headless, sanitizer, and verify runs each executed all 25
	registered tests. `make analyze` returned 2 on four existing
	`select_level_sl.c` uninitialized-button-pointer findings at lines 98, 581,
	584, and 587; later translation units were not analyzed, and no findings were
	suppressed. Protected saves, levels, tutorials, fonts, images, and sounds have
	no diff. Real interactive gameplay was not repeated for this local cleanup;
	v1 forward/backward jump loading and all specified automatic-label regressions
	remain covered by `automatic_jump_label_test`. No commit or push was made.
- Bounded repeated-label correction (2026-11-02): deterministic production defect
	in the uncommitted refactor, not a test/environment failure or nondeterminism.
	Before production changes, a new `test_jump_position_validation` assertion
	reproduced the valid-target rejection with `make test-code-edit-ce` (exit 2).
	`ce_replace_operand` checked positional jump operand IDs against ordinary
	REG/BUF/IMM enums before reaching its jump validation. The stage handler had
	already inserted the automatic label, discarded assignment failure through a
	void wrapper, and left the jump MISSING_OP1 for another frame to repeat.
	The edit module now validates jumps by LABEL/identity semantics, while ordinary
	operand rules remain unchanged. The controller validates owning-program
	membership and establishes identity; ce has no owning-program argument.
	`cw_create_automatic_jump_label` owns the bounded insert/position/assign
	transaction. Its private boolean assignment helper transfers ownership only
	on success. Failure removes the candidate from cp, destroys the label once,
	and destroys an unassigned candidate operand once. The existing void wrapper
	still consumes rejected operands. The prior `replaced != NULL` destructor
	guard and strict destructor assertion are preserved.
	The new `make test-automatic-jump-label` target links real stages/controller,
	program, edit, control-flow and SDL-backed constructors. Narrow doubles supply
	no input selection and inject insertion, jump-operand allocation and invalid
	target failures; counting adapters check cleanup. After ten additional handler
	calls there is exactly one LABEL, an unchanged target identity, a COMPLETE
	jump, no pending operand and an idle edit session. Tests also cover replacement
	with one old-operand destruction, old-label preservation and new-target
	highlighting; off-program target rejection; and forward/backward v1 loading.
	Edit tests exercise actual label indices 0, 1, 2 and IMM_MAX + 10 for JMP/JE/JNE,
	and reject NULL/non-label/identity-less/conflicting targets atomically.
	`make test-code-edit-ce`, `make test-control-flow-cf`,
	`make test-operand-change-selection`, `make test-operand-highlights`,
	`make test-automatic-jump-label`, `make test`, `make test-headless` and
	`make test-sanitize` returned 0 after the fixes. Retained full-suite logs are
	under `/tmp/assemblyarchitect-jump-label.UsCvLA`. During new-harness development,
	compile failures (wrong link variable, nonexistent NOP) and runtime failures
	(invalid save token, missing renderer-number assets, wrong highlight entry
	point) returned 2 and were corrected; no assertion or gate was weakened.
	The missing-assets failure was diagnosed with halt-on-error UBSan. No such
	finding remains in the full sanitizer run. `make verify` first returned 2 for
	the new test's missing final newline, then returned 0 after correction.
	The final sanitizer rerun also returned 0. Normal, headless, sanitizer and
	verify logs each record all 25 test executables. `git diff --check` returned 0.
	The exact protected-path diff returned 0 and listed only the pre-existing
	`data/levels/15/level.cfg` and `data/saves/player_01.sav` edits. Aggregate
	SHA-256 of tracked protected content remained
	`843afb2f9dd92bb0be308bbb65af95a36fb81da400696c47a8526f3c42fdc5d9`
	before and after full validation; no protected change is attributable to this
	fix. Static analysis was not rerun: the separately documented four-finding
	baseline remains outside this correction, without suppression or repair.
	No save format, tracked data, tutorials, assets, display numbering or ordinary
	compatibility semantics were changed by this correction. Real interactive
	gameplay and exhaustive allocation failure inside shared UI factories remain
	unverified; no broad factory rewrite or architectural extraction is included.
	Rollback is limited to this validation/transaction patch, its test target and
	coverage, and these documentation additions. Preserve all earlier uncommitted
	refactor/data changes. No cleanup, installation, commit, push or remote change
	was performed. Preserves AA-MD-002, AA-CF-001/004, AA-RN-002 and AA-MM-001 for
	the tested path without claiming completion of the broader migration.
- Bounded operand-assignment crash correction: the new controller-selection
	regression reproduced `cl_destroy_operand(NULL)` at code_line_cl.c:984 before
	the production patch (`make test-operand-change-selection`, exit 2). The
	existing selection test had passed without empty-slot assignment coverage.
	This is a deterministic production ownership-cleanup defect in the uncommitted
	refactor, not a destructor/API defect. `cw_assign_op_to_line` now destroys the
	displaced operand only when non-NULL; the strict destructor assertion remains.
	Edit tests check NULL displaced operands for both empty slots, exact previous
	operand identity and resulting line/session state. The existing selection
	fixture includes the real controller with a counting destructor adapter and
	matching Make link exclusion, checking zero cleanup calls for empty slots and
	exactly one for either occupied slot. All other `cl_destroy_operand` calls
	were reviewed: remaining calls have non-NULL guards, assertions or prior
	dereferences; no other unchecked nullable cleanup result was found. Selection
	factory allocation-failure behavior is not proven by this audit.
	`make test-code-edit-ce`, `make test-operand-change-selection`,
	`make test-operand-highlights`, `make test`, `make test-headless` and
	`make test-sanitize` each returned 0 after the patch. `make verify` and
	`git diff --check` also returned 0. Aggregate SHA-256 of tracked data, fonts,
	images and sounds matched before/after validation:
	`45c2031ecde131340cb2dab98075ab5556bdc6e7bb35fdcd4e81f2f00687c1d5`.
	The pre-existing level-15 configuration and player-01 save edits were preserved;
	this correction changed no tracked saves, levels, tutorials or assets.
	A debugger-only baseline
	attempt was blocked by macOS process attachment; reproduction used the test
	executable instead. No cleanup or static analysis was run. This preserves
	AA-IN-002 and AA-MM-001 without completing the broader migration stages.
	Rollback is limited to this guard, added regression instrumentation/coverage,
	its Make link exclusion and ownership/evidence documentation; unrelated local
	refactor and data edits must be retained. Real interactive gameplay was not
	exercised by this bounded correction.
- Stage 1: added deterministic in-memory characterization for shared-target jump
	deletion (legacy result: the label is removed and the other jump becomes
	`MISSING_OP1`), reorder target preservation, forward/backward positional v1
	round-trip, `_` operand persistence, and repeated create/clear/destroy cycles.
	Existing tests cover operand cancellation/replacement, geometry boundaries,
	resize, and renderer non-mutation of line order/state/label identity for its
	SDL-independent empty-asset path. Pixel equivalence and active animation-group
	non-mutation are not yet established.
- Stage 2: consolidated private code-window list, geometry rectangles, highlight
	values and animation claims under one internal state structure. This remains a
	compatibility singleton and does not provide independent contexts or injected
	partial initialization.
- Stage 3: introduced `code_layout_cly` for pure rectangle, spacing, offset,
	line-position and strict boundary hit calculations. Existing `cw_*` adapters
	delegate to it. Operand/comma/text-box geometry remains in `code_window_cw.c`.
- Stage 4: introduced `code_program_cp` with explicit ownership, status results,
  ordered access/mutation and SDL-free tests. Production and direct-source fixtures
  use indexed reads or its read-only iterator; the internal raw-List view/header
  have been removed.
- Stages 5/6: added program-local 64-bit label identities and target snapshots;
	v1 files still encode positional IDs. `control_flow_cf` assigns identities,
	translates v1 positions after full load, repairs reordered/removed targets, and
	resolves/validates jumps and applies the existing forward/backward execution
	intervals with explicit outcomes. SDL-backed label/jump operand rebuilding and
	compatibility load entry points remain in cw; full invariant closure is not
	claimed.
- Stage 7: added `code_edit_ce` explicit idle/moving/replacing session states and
	result-valued begin/cancel/replace/insert/remove/move operations. Incompatible
	operand replacement leaves the old operand and line state unchanged. Production
	cw adapters use these operations, with UI positioning and animation outside ce.
	Pending-any/operand-1/operand-2 queries now live in ce and are used by stage,
	level and code-state callers.
- Stage 8: added pure `code_interaction_ci` hit-result translation and routed the
	stage edit dispatcher through it while preserving hit-test short-circuit order.
	SDL event/hit acquisition and persistent held-line state remain outside ci.
- Stage 9: added `code_renderer_cr` for code-line, operand, grouped-label, comma,
	line-number, challenge/code-box and stage-title presentation. Assets have explicit init/destroy APIs and are stored
	in the cw state; process-owned renderer assets survive per-level code-window
	destruction and are released explicitly at app shutdown. Animation-claim
	mutation, scroll and layout coordination remain in cw.
- Stage 10/11: migrated game mechanics, storage serialization, code-state capture,
	interaction rules, stages, levels and arrows to model/control-flow/edit APIs for
	applicable reads and transitions. Removed raw-list access and unused model query
	and jump-execution wrappers. Remaining layout/hit-test/save-construction/edit
	adapters remain in cw; migration is not complete.
- ASan/UBSan exposed three pre-existing label/jump text allocation overflows in
	`code_window_cw.c`; four related constructor size calculations were corrected.
	No suppressions were added.
- Validation for this continued working tree: focused model, control-flow, edit,
  interaction, layout, renderer, operand-highlight, operand-selection,
  modal-capture, animation-sync, tutorial-text and arrow tests passed. Full
  infrastructure, formatting, build, headless, sanitizer, verify and whitespace
  commands were run; capture output was truncated and individual chain exit codes
  were not retained here. No tracked saves, levels, tutorials, fonts, images, or
  sounds were modified. Hosted CI and manual gameplay smoke checks were not run.
- Final `make analyze` stopped at four documented pre-existing findings in
	`source/ui/screens/select_level_sl.c` (uninitialized sector button pointers);
	later translation units were not analyzed. No findings were suppressed.
- `code_window_cw.c` was 3,127 lines at the original baseline and measured 2,350
	at an earlier migration checkpoint; the current cleanup checkpoint measures
	2,361 lines. It remains above the request's 1,800-line maximum (and the
	1,200-1,600 aim). Broad pure-layout extraction, semantic edit ownership,
	remaining caller migration, and wrapper retirement remain incomplete; this is
	not a completed refactor.
