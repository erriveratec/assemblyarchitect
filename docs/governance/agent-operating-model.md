# Agent Operating Model

1. Refactor planner reads implementations/headers/callers/tests/storage/build,
   inventories ownership/invariants, and writes one bounded issue specification.
2. A human maintainer or explicitly authorized automation approves scope.
3. Architecture implementer makes one reversible stage patch and prepares a PR;
   branch/commit/push/PR operations require explicit authorization.
4. Test engineer independently evaluates or adds deterministic regression coverage.
5. CI runs real local Make entry points.
6. Regression reviewer performs surrounding-code, read-only review.
7. Integration fixer reproduces and classifies failures before minimally repairing.
8. CI and independent review rerun after changes.
9. A human maintainer approves and merges. Agents never approve their own work,
   bypass gates, merge, or modify protection settings.

Findings use Critical, Major, Minor, Suggestion as defined in the invariant catalog.
Zero unresolved Critical/Major findings is required for completion. Reports include
verified current behavior, invariant IDs, files, exclusions, API/ownership impact,
exact commands/results, unrun checks/reasons, risks, rollback, and dependencies.

Escalate unclear assembly semantics, save-format impact, golden-image changes,
invariant changes, third-party dependencies, CI/governance weakening, work beyond
approved scope, or unverified player-visible changes. Do not silently reinterpret
the issue to accommodate a defect. Classify failures as production, test,
environment, or nondeterminism, and distinguish pre-existing from introduced.

Profiles use documented VS Code tool aliases `read`, `search`, `edit`, `execute`.
Planner and reviewer deliberately have read/search only: no terminal or edit tool.
The planner returns a documentation/issue draft for a human or authorized writer
to apply. This is stricter than documentation-only editing because generic `edit`
cannot enforce a path boundary. Reviewer verification is performed by a separate
authorized runner; granting terminal access would undermine read-only authority.
Implementer/test/fixer have read/search/edit/execute, with task-specific scope.
No profile receives GitHub approval/merge tools or unrestricted tool wildcards.
These are supported local aliases; GitHub service interpretation and organization
tool availability must be checked when enabling the cloud coding agent.

The local customization syntax reference used during bootstrap is the installed
VS Code agent-customization skill's `references/agents.md`. Frontmatter is limited
to name, description, tools; no imaginary per-path ACL, model, or merge field.

Nightly failures belong to the human maintainer on duty. Review artifacts on the
next working day, reproduce with the same tool versions/seed (if any), open a
bounded follow-up, and link triage evidence. Critical safety failures block release;
nightly-only scheduling does not make a failure optional or invisible.
