# Architecture Decision Records

Create sequential `NNNN-short-title.md` records. A proposed record is not approval
or an implementation claim. Human scope approval is required before changing
semantics, saves, invariants, dependencies, or the migration sequence.

## Template

```markdown
# NNNN: Title
## Status
Proposed | Accepted | Rejected | Superseded (link the replacement)
## Context
Verified behavior, callers, ownership, problem, constraints, invariant IDs.
## Decision
Bounded choice, API/lifetime impact, compatibility strategy, approval evidence.
## Alternatives Considered
Tradeoffs grounded in actual code and tests.
## Consequences
Benefits, costs, risks, migration dependencies, unresolved questions.
## Validation
Exact commands, regression tests, manual acceptance, outcomes not yet verified.
## Rollback or Supersession
How to restore the prior implementation without losing compatible saves.
```

[0001: Stable label identity](0001-stable-label-identity.md) is proposed only.
