# Architecture Index

Assembly Architect is a C/SDL2 educational puzzle game, currently verified on
macOS arm64 with Apple Clang and four SDL2 frameworks. The Makefile, not the
machine-specific compilation database, is authoritative for build/link behavior.

- [Current code window](current-code-window.md): observed implementation only.
- [Target code window](target-code-window.md): proposed boundaries, not shipped code.
- [Invariants](invariants.md): obligations, evidence, and missing tests.
- [Architecture decisions](adr/README.md): proposal and approval records.
- [Master plan](../refactor/master-plan.md): bounded migration issue specifications.
- [Migration state](../refactor/migration-state.md): stage tracking.
- [Bootstrap evidence](../refactor/bootstrap-report.md): baseline and validation.
- [Quality gates](../governance/quality-gates.md): executable checks and limitations.

Read current state before target state. A green build does not prove complete
gameplay correctness or save compatibility. Proposed APIs below are not files
that agents should assume already exist.
