## Summary

## Related Issue and Migration Stage

Include scope approval and relevant AA-* invariant IDs.

## Existing Behavior Preserved

## Responsibility Moved or Infrastructure Added

## API Changes

Include compatibility wrapper removal stage, or state none.

## Ownership and Lifetime Changes

## Assembly-Semantic Impact

## Save Compatibility Impact

Include v1 fixture evidence, or explicitly state untested aspects.

## Test Changes

Explain how tests detect breakage, including direct-C doubles/link exclusions.

## Commands Executed and Results

| Exact command | Exit status | Outcome and evidence |
| --- | --- | --- |
| Fill with commands actually executed | | |

Distinguish pre-existing failures, introduced defects, environment limits, and
nondeterminism. Compilation is not a complete gameplay smoke test.

## Checks Not Run and Reason

## Risks

## Rollback Plan

## Screenshots or Visual Artifacts

When relevant, include deterministic before/after metadata and human approval for
baseline changes. No personal saves or full workspace artifacts.

## Reviewer Checklist

- [ ] Scope is bounded and human-approved.
- [ ] No unrelated changes or unauthorized refactor/gameplay work.
- [ ] No quality gate, compiler warning, test, or assertion weakened/skipped.
- [ ] Tests detect the intended regression and exercise real behavior.
- [ ] Ownership, error paths, partial initialization, and lifetimes are clear.
- [ ] Public API and save compatibility impact are documented.
- [ ] Architecture and migration documentation are current.
- [ ] Risks, rollback and unrun checks are explicit.
- [ ] No unresolved Critical or Major findings.
- [ ] Human approval/merge only; no agent self-approval or CI bypass.
