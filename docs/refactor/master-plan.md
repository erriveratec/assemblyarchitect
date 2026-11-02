# Code-Window Migration Issue Specifications

No implementation issues have been opened. Each stage below is a bounded issue
specification requiring human scope approval, independent tests/review, current
migration state, and zero unresolved Critical/Major findings. Dependencies are
sequential unless explicitly approved otherwise. Bootstrap does not execute Stage 1.

Common acceptance: preserve all existing tests, represented semantics, v1 saves,
levels/tutorials and UI behavior; run build/full tests, applicable analysis and
sanitizers, and the invariant-specific checks. Every PR records commands, excluded
work, ownership changes, risks, and a reversible patch. Expected new modules are
proposals, not existing paths.

## Stage 0: Architecture Inventory and Bootstrap

- Purpose: establish verified ownership/build facts and governance.
- Preconditions/dependencies: inspect current Makefile/config/source and baseline.
- In scope: instructions, docs, agents, real local gates, safe cleanup, workflows.
- Out of scope: C refactor, semantic fixes, saves/assets, production test changes.
- Expected files: `.github/`, `docs/`, Makefile, infrastructure scripts, README.
- Compatibility: retain production APIs/build/test calling conventions.
- Tests/acceptance: baseline and final full suite; infrastructure/YAML/path checks;
  document unavailable local/remote checks, never label unrun CI green.
- Risks/rollback: inaccurate automation/platform claims; revert infrastructure only.

## Stage 1: Characterization Tests

- Purpose: make current behavior falsifiable before moving code.
- Preconditions/dependencies: Stage 0 evidence and approved test scope.
- In scope: reorder/delete/forward-backward jump, invalid operands, lifecycle,
  isolated v1 save fixtures, rendering non-mutation, boundary hit tests.
- Out of scope: changing behavior to match desired invariants or broad extraction.
- Expected files: `tests/`, Makefile; minimal approved internal seams only.
- Compatibility: use real cw_* paths, respecting direct-C includes/link exclusions.
- Tests/acceptance: new tests demonstrate defect detection; AA-CF/AA-SV uncertainty
  is measured; full suite and sanitizer pass or findings are escalated.
- Risks/rollback: fixtures touching player files or testing only mocks; revert new
  test/seam patch without weakening pre-existing coverage.

## Stage 2: Explicit Code-Window State

- Purpose: make ownership/lifetime visible without splitting responsibilities.
- Preconditions/dependencies: Stage 1 lifecycle characterization.
- In scope: explicit context for existing cw state and initialization/destruction.
- Out of scope: semantics, layout changes, multi-window feature, new singleton.
- Expected files: code_window implementation/header/internal, lifecycle tests.
- Compatibility: existing cw_* adapters temporarily use one explicit owner;
  annotate Stage 10/11 wrapper retirement.
- Tests/acceptance: repeated/partial init destruction, independent context checks,
  AA-MM-001/002, complete suite/sanitizer.
- Risks/rollback: global aliases or stale held pointers; revert context patch.

## Stage 3: Pure Layout

- Purpose: isolate canonical geometry computations.
- Preconditions/dependencies: Stage 2; geometry characterization.
- In scope: proposed `code_layout_cly` value inputs/results and hit rectangles.
- Out of scope: render assets, display redesign, semantic state.
- Expected files: proposed layout module, cw adapters, layout tests/Makefile.
- Compatibility: preserve exact logical canvas values via wrappers.
- Tests/acceptance: canonical/resized geometry and boundaries; AA-RN-003/AA-IN-001;
  no renderer creation for pure calculations; complete suite.
- Risks/rollback: rounding/edge changes; route wrappers back to prior calculations.

## Stage 4: SDL-Independent Program Owner

- Purpose: separate semantic program ownership from buttons/textures.
- Preconditions/dependencies: Stages 2/3 and insertion/removal characterization.
- In scope: proposed `code_program_cp`, read-only views, explicit ownership.
- Out of scope: stable-ID redesign, new save schema, instruction semantics.
- Expected files: proposed program module, cw/cl adapters, model tests/Makefile.
- Compatibility: one authoritative model and temporary UI mapping; no duplicated
  helper implementations or divergent list ownership.
- Tests/acceptance: insert/delete/move, rollback, allocation failures; AA-MD/AA-MM;
  model tests without SDL video/renderer; full suite/sanitizer.
- Risks/rollback: double ownership/UI mapping divergence; revert owner transfer.

## Stage 5: Stable Logical Label Identity

- Purpose: decouple branch identity from display position/address.
- Preconditions/dependencies: Stage 4; human acceptance of ADR 0001.
- In scope: program-local logical IDs and v1 positional translation.
- Out of scope: disk-version changes, educational behavior changes.
- Expected files: program/label types, storage adapters, jump/save tests, ADR.
- Compatibility: retain v1 reads/writes; old cw_* calls adapt temporarily.
- Tests/acceptance: AA-CF-001/002/004 and AA-SV-001/002; forward/backward and
  reorder/round-trip fixtures prove unchanged target meaning; sanitizer/full suite.
- Risks/rollback: ID reuse/position translation; revert implementation, retain v1.

## Stage 6: Control-Flow Validation

- Purpose: centralize resolution and invalid-target outcomes.
- Preconditions/dependencies: Stage 5 and characterized deletion behavior.
- In scope: proposed `control_flow_cf`, explicit validation statuses.
- Out of scope: new branch semantics, optimizer, changing label deletion policy.
- Expected files: proposed control-flow module, model/edit adapters, tests.
- Compatibility: wrappers preserve existing incomplete-operand representation.
- Tests/acceptance: AA-CF-001 through 004, shared/deleted targets and malformed
  load outcomes; no dangling pointers; full suite/sanitizer.
- Risks/rollback: assertions replaced with wrong status behavior; revert extraction.

## Stage 7: Edit Commands and Sessions

- Purpose: isolate transactional edits and frame-spanning edit state.
- Preconditions/dependencies: Stage 6; replacement/cancellation characterization.
- In scope: proposed `code_edit_ce`, explicit commands/session/rollback results.
- Out of scope: UI redesign, new edit features, SDL event polling in model.
- Expected files: proposed edit module, stages/cw adapters, model/integration tests.
- Compatibility: existing interaction routes dispatch commands through wrappers.
- Tests/acceptance: AA-MD-002/AA-IN-002, invalid operand/delete/reorder rollback,
  canceled/partial sessions and full suite/sanitizer.
- Risks/rollback: changed drag policy or retained pointers; restore prior dispatcher.

## Stage 8: Input-to-Action Translation

- Purpose: isolate interpretation of SDL input from semantic mutation.
- Preconditions/dependencies: Stage 7 and modal/boundary tests.
- In scope: proposed `code_interaction_ci`, event/hit-region to command mapping.
- Out of scope: new shortcuts, changed drag/delete permissions, visual changes.
- Expected files: proposed interaction module, stages/mouse/cw adapters, tests.
- Compatibility: preserve consumed-event ordering and modal capture.
- Tests/acceptance: AA-IN-001/002; explicit frame inputs, no sleeps; full suite.
- Risks/rollback: click-through/order regressions; restore previous input dispatch.

## Stage 9: Renderer and Visual State

- Purpose: make rendering presentation-only.
- Preconditions/dependencies: Stage 8; non-mutation/phase/render-order coverage.
- In scope: proposed `code_renderer_cr`, immutable snapshots and asset ownership.
- Out of scope: art/fonts/colors/layout changes, automatic golden regeneration.
- Expected files: proposed renderer module, cw/draw adapters, headless tests.
- Compatibility: preserve output, button animation claims, tutorial phase behavior.
- Tests/acceptance: AA-RN-001/002, snapshot immutability, deterministic pixel/arrow
  checks, lifecycle/sanitizer and full suite, manual visual review.
- Risks/rollback: ownership/phase/render order; restore old rendering through adapter.

## Stage 10: Caller Migration and API Reduction

- Purpose: move callers to explicit boundaries one bounded group at a time.
- Preconditions/dependencies: Stage 9; caller inventory and ownership contracts.
- In scope: stages, storage, execution, tutorial callers and public header narrowing.
- Out of scope: semantics, removing wrappers still used by unconverted callers.
- Expected files: existing cw/stages/storage/gameplay/tutorial headers/callers/tests.
- Compatibility: wrappers remain for remaining callers with named Stage 11 removal.
- Tests/acceptance: each caller group integration tests; AA-ED/AA-SV; full suite,
  sanitizer and real gameplay/save smoke checklist.
- Risks/rollback: hidden direct-C test dependencies; revert one caller group.

## Stage 11: Wrapper Removal and Final Regression

- Purpose: finish ownership/dependency enforcement without behavioral changes.
- Preconditions/dependencies: Stage 10; no legacy caller or external use unresolved.
- In scope: remove unused wrappers/state and update docs/build/API inventory.
- Out of scope: opportunistic optimization, new semantics or persistence versions.
- Expected files: cw compatibility surface, migrated modules/tests/Makefile/docs.
- Compatibility: prove every old responsibility has one owner; retain v1 saves.
- Tests/acceptance: complete invariant audit, all automated gates, fixtures,
  sanitizer/static analysis and human release smoke; zero Critical/Major findings.
- Risks/rollback: missed caller/API coupling; restore removed adapters, not old saves.
