# Assembly Architect Repository Instructions

## Repository Context

For significant changes, read the project context, architecture index, invariant
catalog, quality gates, and the affected implementation, header, callers, and tests:

- `docs/PROJECT_CONTEXT.md`
- `docs/architecture/README.md`
- `docs/architecture/invariants.md`
- `docs/governance/quality-gates.md`

Use `scripts/create_copilot_context_bundle.sh` when an external review needs a
committed-project snapshot and working-tree diff. Do not treat generated context
dumps as repository documentation or commit them.

For work that changes an active ownership or compatibility boundary, consult
`docs/architecture/current-code-window.md`, the relevant proposal, and applicable
ADRs. Update migration records only when the task changes or completes a tracked
migration boundary. Unrelated maintenance does not require migration-state work.

Path-specific guidance under `.github/instructions` supplements this file.
Player-facing text and presentation for matching paths are governed by
`.github/instructions/ui-language.instructions.md`; keep its terminology and
presentation rules there rather than duplicating them here.

## Mission and Safety

This C/SDL2 game teaches x86-inspired assembly. Educational correctness is a
functional requirement. Preserve the implemented assembly semantics, deterministic
execution, player-visible behavior, version-1 save compatibility, levels, tutorials,
and the relationship between labels, jumps, execution state, highlights, and
instructional feedback. Compilation alone does not demonstrate gameplay.

## Verified Conventions

- The Makefile is authoritative. The verified platform is macOS with Apple Clang
  and SDL2, SDL2_image, SDL2_ttf, and SDL2_mixer frameworks. Do not claim other
  production platforms without verification.
- No `-std` is specified; preserve the verified Apple Clang GNU C17 default.
  Preserve `-Wall -Wextra -Wpedantic -Werror`, including CFLAGS overrides.
- Match neighboring tabs, braces, explicit state structures, and prefixes such
  as `cw_`, `cl_`, `sv_`, `mc_`, and `ha_`. Avoid speculative abstractions,
  duplicated helpers, and new mutable globals or singletons.
- Check allocation/error paths, use structured statuses for expected failures,
  and make ownership (owned, borrowed, or transferred) explicit. Partial
  initialization must be safe to destroy.
- Tests may include production `.c` files with macro doubles. Preserve Makefile
  test exclusions, test-only defines, and asset symlinks. Run from the repository
  root; asset lookup depends on executable location.

## Current Ownership

| Module | Current responsibility |
| --- | --- |
| `code_program_cp` | Owns the ordered instruction lines and exposes borrowed indexed/iterator views. |
| `control_flow_cf` | Assigns stable label identities, translates version-1 positional targets, repairs/resolves/validates jumps, and applies jump execution-state changes. |
| `code_edit_ce` | Owns edit-session state and result-valued operand/line operations, cloning, and pending-operand queries. |
| `code_layout_cly` | Computes selected code-window geometry and hit-test values. |
| `code_interaction_ci` | Maps already-computed hit results to prioritized edit actions. |
| `code_renderer_cr` | Draws code lines and code-window presentation; owns its comma and line-number assets. |
| `code_window_cw` | SDL-aware singleton coordinator and compatibility API; retains line/operand construction, save adapters, hit acquisition, held-line edit orchestration, presentation repair, and some geometry. |

These are partial boundaries, not proof that the full controller extraction is
complete. `stages.c` still sequences input/edit actions; `code_window_cw` still
owns active compatibility adapters and remains a singleton. Do not remove or
reclassify wrappers merely because a corresponding module exists. Verify callers
and tests before changing ownership. The architecture documents describe the
remaining work; do not turn their proposed end-state into a claim about current
behavior.

## Mandatory Invariants

See `docs/architecture/invariants.md` for evidence and known coverage gaps. These
are requirements, not claims that current code or tests prove them.

- Every jump resolves to exactly one valid label identity.
- Reordering cannot silently retarget a jump.
- Referenced-label removal has a defined outcome and leaves no dangling pointer.
- Forward and backward jumps load deterministically; existing version-1 saves
  remain readable.
- A failed edit leaves the program unchanged.
- Rendering and hit testing do not mutate semantic program state.
- Ownership is explicit and partial initialization can be safely destroyed.
- Highlights reflect actual pending/execution state.
- Tests are deterministic; use controlled clocks/phases, not arbitrary sleeps.

## Verification and Prohibitions

Use the commands and limitations in `docs/governance/quality-gates.md`; run focused
checks first and all applicable required gates before completion. Preserve
`make all`, `make test`, every existing test/assertion, warning policy, analysis,
and sanitizer findings. Classify known baseline failures separately; never hide,
weaken, or suppress findings without documented root-cause analysis.

Do not change save formats without an approved migration, auto-update visual
baselines, change gameplay/educational design outside scope, or add a build system
when Make fits. Use only documented, exact-path cleanup procedures after preview;
never delete editor files or use recursive discovery/broad cleanup wildcards.

Do not commit, push, create branches/PRs/issues, install software, use privileges,
or change remote settings without explicit authorization. Do not merge, approve
your own work, bypass CI, or push to a protected default branch. Treat PR code as
untrusted; never build it with `pull_request_target` or privileged write tokens.
Keep Actions least-privilege and version-pinned, and do not claim remote checks ran
locally.

## Completion

Add focused coverage for behavior changes and identify characterization coverage
for preserved behavior. Document public API/ownership changes and update
architecture or migration records only when the change affects those boundaries.
Report exact checks, checks not run and why, relevant risks, rollback, and required
manual gameplay checks. Only a human maintainer performs final approval and merge.
