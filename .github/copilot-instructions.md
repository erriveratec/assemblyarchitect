# Assembly Architect Repository Instructions

## Required Repository Context

Before significant implementation, refactoring, save-system, gameplay-semantic,
or architecture work, read:

1. `docs/PROJECT_CONTEXT.md`
2. `docs/architecture/README.md`
3. `docs/architecture/invariants.md`
4. `docs/governance/quality-gates.md`
5. The implementation, headers, callers, and tests for the affected subsystem

Use `scripts/create_copilot_context_bundle.sh` when an external review needs a
complete committed-project snapshot or a working-tree diff.

Do not treat generated context dumps as repository documentation. Context dumps
are temporary review artifacts and must not be committed.

## Mission and Safety

This C/SDL2 game teaches x86-inspired assembly. Educational correctness is a
functional requirement. Preserve represented assembly semantics, deterministic
execution, player-visible behavior, existing saves, levels, tutorials, supported
macOS behavior, and the relationship between labels, jumps, execution, highlights,
and instructional feedback. Compilation alone does not demonstrate gameplay.

Read `docs/architecture/README.md`, the affected implementations, headers,
callers, tests, and `docs/refactor/migration-state.md` before working. Identify
behavior and ownership, establish a baseline, add or identify characterization
coverage, make one bounded change, run the documented checks, update relevant
architecture/migration records, and report checks not run with reasons.

## Verified Conventions

- The Makefile is authoritative. macOS with Clang and SDL2, SDL2_image,
  SDL2_ttf, SDL2_mixer frameworks is the verified platform. Do not invent Linux
  or Windows support. There are no included Make fragments.
- No `-std` is currently specified; the verified Apple Clang default is GNU C17
  (`__STDC_VERSION__=201710L`). Do not silently change the dialect.
- Preserve enforced `-Wall -Wextra -Wpedantic -Werror`, including CFLAGS overrides.
- Match neighboring tabs, braces, explicit state structures, and prefixes such
  as `cw_`, `cl_`, `sv_`, `mc_`, and `ha_`. Avoid speculative abstractions,
  duplicated helpers, and new mutable globals or singletons.
- Check allocations and error paths; use structured status values for expected
  failures. Document whether pointers are owned, borrowed, or transferred.
  Partial initialization must be safe to destroy.
- Tests can include production `.c` files with macro doubles. Preserve the
  Makefile's excluded translation units and test-only defines.
- Run from the repository root. Asset lookup also depends on executable location;
  preserve test asset symlinks. Do not relocate binaries casually.

## Target, Not Current Architecture

| Proposed module | Responsibility |
| --- | --- |
| `code_window_cw` | Orchestration and temporary compatibility API |
| `code_program_cp` | SDL-independent program ownership |
| `control_flow_cf` | Stable labels, jump targets, validation |
| `code_edit_ce` | Commands and explicit edit-session state |
| `code_layout_cly` | Pure geometry |
| `code_interaction_ci` | Input-to-action translation |
| `code_renderer_cr` | Presentation-only SDL rendering |

Desired flow: SDL events -> interaction -> edit commands -> program/control flow;
program + layout + visual state -> renderer; code_window coordinates these.
Model, control flow, and editing must eventually be testable without SDL video
initialization or renderer creation. This bootstrap does not implement these
modules. Compatibility wrappers must name their removal stage.

## Mandatory Invariants

See `docs/architecture/invariants.md` for evidence and outstanding coverage.

- Every jump resolves to exactly one valid label identity.
- Reordering cannot silently retarget a jump.
- Referenced-label removal has a defined outcome and leaves no dangling pointer.
- Forward and backward jumps load deterministically.
- A failed edit leaves the program unchanged.
- Rendering and hit testing do not mutate semantic program state.
- Ownership is explicit; partial initialization can be safely destroyed.
- Existing save fixtures remain readable; highlights reflect actual state.
- Tests are deterministic, with controlled clocks rather than arbitrary sleeps.

These are obligations for migration, not claims that current code proves them.

## Verification and Prohibitions

Use the real commands and limitations in `docs/governance/quality-gates.md`.
Preserve `make all`, `make test`, every existing test and assertion. Never skip,
delete, weaken, or hide failing tests, warning policies, analysis, or sanitizer
findings. Classify pre-existing failures separately from introduced defects.
Do not suppress findings without documented root-cause analysis.

Do not change saves without an approved migration, auto-update visual goldens,
change gameplay/educational design outside approved scope, or begin architectural
extraction during infrastructure work. Do not add a build system when Make fits.
Never delete editor files, recursively find/delete debug bundles, or use broad
cleanup wildcards. Inspect dry runs before cleanup; this bootstrap must not run
`make clean` without the user's additional approval.

Never commit, push, create branches/PRs/issues, install software, use privileges,
or change remote settings without explicit task authorization. Never merge,
approve your own work, bypass CI, or push directly to a protected default branch.
No agent may acquire merge/approval authority. Treat PR code as untrusted; never
build it using `pull_request_target` or privileged write tokens. Keep Actions
least-privilege and version-pinned; do not claim remote checks ran locally.

## Completion

All required checks pass; new behavior has tests and preserved behavior has
characterization coverage. Public API/ownership changes and migration state are
documented. Include risks, rollback, exact command outcomes, and manual checks.
No unresolved Critical or Major findings may remain before human approval.
Only a human maintainer performs final approval and merge.
