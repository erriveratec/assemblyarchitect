# Migration State

| Stage | Status | Pull request | Compatibility wrappers | Required checks | Notes |
| --- | --- | --- | --- | --- | --- |
| 0 Inventory/bootstrap | In progress | Local draft only | Existing cw_* unchanged | Baseline, full tests, infrastructure validation | See bootstrap-report.md; remote CI not executed |
| 1 Characterization | Not started | None | Existing cw_* | Full tests, new characterization, sanitizer | Next scope for human approval |
| 2 Explicit context | Not started | None | cw_* planned; retire 10/11 | Lifecycle, partial init, sanitizer | Depends on 1 |
| 3 Pure layout | Not started | None | Layout adapters planned | Geometry, resize, boundaries | Depends on 2 |
| 4 Program model | Not started | None | Ownership adapters planned | SDL-independent model, sanitizer | Depends on 3 |
| 5 Label identity | Not started | None | v1 positional translation planned | Jump/reorder/save fixtures | Depends on 4 and accepted ADR |
| 6 Control flow | Not started | None | Validation adapters planned | Delete/invalid target, sanitizer | Depends on 5 |
| 7 Edit commands | Not started | None | Edit adapters planned | Rollback/session tests | Depends on 6 |
| 8 Interaction | Not started | None | Input adapters planned | Modal/boundary tests | Depends on 7 |
| 9 Renderer | Not started | None | Rendering adapters planned | Non-mutation/phase/pixel checks | Depends on 8 |
| 10 Caller migration | Not started | None | Retain until last caller migrates | Caller integration, saves, smoke | Depends on 9 |
| 11 Wrapper removal | Not started | None | Remove only unused adapters | All gates and invariant audit | Depends on 10 |

Only update a stage after recording actual evidence. Bootstrap completion does not
authorize architecture extraction. No wrapper has been added or removed here.

Bounded bootstrap corrections add pinned SDL DMG verification and portable relative
test-asset links. Static Analysis is manual-only via `workflow_dispatch`, currently
known to fail on four existing findings until bounded triage is completed. Healthy
headless/sanitizer nightly checks remain scheduled; no analyzer repair is included.
