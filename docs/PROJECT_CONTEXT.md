# Project Context for Copilot and Contributors

This file is the durable entry point for understanding Assembly Architect before making changes. Keep it concise and current. Detailed architecture and governance remain in their dedicated documents.

## Product

Assembly Architect is an educational C and SDL game that teaches x86 assembly through gameplay and visual representations. Educational accuracy, deterministic execution, save compatibility, memory safety, and stable player-visible behavior are functional requirements.

## Supported environment

- Primary verified platform: macOS on Apple Silicon
- Compiler: Apple Clang
- Build system: GNU Make-compatible Makefile
- Libraries: SDL2, SDL2_image, SDL2_ttf, and SDL2_mixer frameworks
- Hosted verification: GitHub Actions macOS jobs, ASan/UBSan, headless SDL, infrastructure checks, and CodeQL

Do not claim Linux production support unless it is separately established and tested.

## Start here

Before a significant change, read:

1. `.github/copilot-instructions.md`
2. `README.md`
3. `docs/architecture/README.md`
4. `docs/architecture/invariants.md`
5. `docs/governance/quality-gates.md`
6. The implementation, header, callers, and tests for the target subsystem

## Current architecture anchors

- `code_program_cp`: ordered assembly-program ownership
- `control_flow_cf`: labels, jump identities, resolution, and repair
- `code_edit_ce`: semantic editing transactions and pending edit state
- `code_interaction_ci`: translation of computed input hits into semantic actions
- `code_layout_cly`: pure code-window geometry
- `code_renderer_cr`: code-window presentation
- `code_window_cw`: SDL-aware orchestration and remaining compatibility adapters

Update this list when responsibility ownership changes.

## Required local gates

Run the applicable focused tests during development. Before considering a change complete, run:

```text
make check-infrastructure
make format-check
make -B all
make test
make test-headless
make test-sanitize
make verify
git diff --check
```

`make analyze` is a separate diagnostic. Document known baseline findings rather than suppressing or misreporting them.

## Safety boundaries

- Do not write tests against tracked player saves.
- Do not change levels, tutorials, or assembly semantics as an incidental refactor.
- Do not weaken `-Werror`, tests, sanitizers, or dependency checksum verification.
- Do not automatically update visual baselines.
- Keep rendering free of semantic program mutation.
- Failed edits must leave program state unchanged.
- Preserve version-1 save compatibility unless an explicit migration is approved and tested.

## Refactor workflow

For future refactors:

1. Generate the Copilot project-context and worktree-review files with `scripts/create_copilot_context_bundle.sh`.
2. Give Copilot the project-context file plus the subsystem-specific request.
3. Ask the refactor planner for a concise bounded plan only when behavior or ownership is uncertain.
4. Use the architecture implementer for the change.
5. Run focused tests, then the required gates.
6. Use the integration fixer for concrete failures.
7. Use the regression reviewer for the final diff when the change is substantial.
8. Perform the affected manual gameplay smoke test.
9. Commit and push only after local validation.

## Maintenance rule

This document should remain under roughly 150 lines. It is an index and current-state summary, not an execution log. Do not place one-time prompts, completed migration transcripts, terminal logs, or historical CI output here. Git history already preserves completed work.
