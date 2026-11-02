# 0001: Stable Label Identity

## Status

Proposed. No identity API or save-format change is implemented by bootstrap.

## Context

AA-CF-001 through AA-CF-004 and AA-SV-001/002 require stable branch meaning.
`operand_t` stores positional `id` and borrowed `code_line_t *jptr`. Reorder repair
rebuilds numeric IDs from surviving pointers; save loading resolves positions after
all lines are built. No better existing logical identity mechanism was found.
Display position changes during editing; addresses change after reload and can
dangle after deletion. Neither alone is a durable logical identity.

## Decision

Propose program-owned stable logical label IDs, immutable for each label lifetime,
and explicit resolution/validation. IDs must not be reused while stale references
could survive; define generation/overflow behavior before selecting a representation.
Keep display numbering derived, not authoritative. Define deletion semantics from
characterization before changing them, especially shared labels and incomplete jumps.

Preserve v1 saves: translate stored line positions to logical IDs only after loading
the complete program; serialize compatible positions at the storage boundary.
Do not silently add a disk field/version. A future persistent-ID format needs a
separate approved migration and old-save fixture evidence.

## Alternatives Considered

- Keep pointers plus repair: smallest change, but lifetime proof remains difficult.
- Use line positions only: simple persistence, but editing can retarget branches.
- Use globally unique persistent IDs now: unnecessary format churn and new state.
- Stable program-local IDs with v1 adapters: preferred bounded proposal.

## Consequences

Requires a program owner, resolver, explicit invalid-target status, transactional
edit behavior, and translation tests. Risks include off-by-one positions, forward
references, accidental target reuse, allocation-failure partial edits, and changes
to label deletion/tutorial behavior. A model extraction must precede implementation.

## Validation

Stage 1 first characterizes reorder/delete, forward/backward saves, `_` operands,
and reloads. Stage 5 compares logical targets before/after moves and v1 round trips.
Run the complete suite, sanitizer checks, isolated fixture tests, and manual smoke
acceptance. These new semantic checks do not exist yet.

## Rollback or Supersession

Keep v1 serialization and temporary cw_* adapters throughout migration. Revert the
bounded implementation patch before removing adapters; never rewrite user saves
as rollback. A human-accepted replacement ADR supersedes this record explicitly.
