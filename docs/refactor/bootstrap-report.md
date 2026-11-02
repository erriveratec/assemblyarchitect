# Bootstrap Pull-Request Draft and Evidence

Stage 0 completion record. The bootstrap was merged and pushed to `main`.
Local verification, hosted GitHub Actions, CodeQL C/C++ analysis, and the
manual macOS smoke test completed successfully. No pull request was used,
and no remote repository protection, security, CODEOWNERS, or administrative
settings were changed by this work.

## Bounded Plan (Recorded Before Implementation)

1. Add `.github/copilot-instructions.md` and this evidence/PR draft.
2. Add `docs/architecture/{README,current-code-window,target-code-window,invariants}.md`,
   `docs/architecture/adr/{README,0001-stable-label-identity}.md`, and
   `docs/refactor/{master-plan,migration-state}.md`.
3. Add `docs/governance/{agent-operating-model,quality-gates,branch-protection,visual-baseline-policy,release-smoke-test}.md`.
4. Add five `.github/agents/*.agent.md` profiles.
5. Add bounded Make verification/cleanup entry points, supporting infrastructure
   scripts/configuration, and update README commands as justified by local checks.
6. Add setup, CI, nightly, and CodeQL workflows for verified build mechanisms;
   clearly mark any unsupported platform/requested mechanism as deferred.
7. Add issue/PR templates and ownership guidance, without inventing account access.
8. Independently validate local commands, YAML, profiles, paths, and safety.

No production C/header files, assets, saves, level/tutorial data, or test assertions
are in scope. No code-window extraction. No software installation or remote action.

## Baseline (Before Edits)

Revision: `18b5d18`. Initial `git status --short`: `?? docs/` (user-provided prompt).
`git diff --stat` and `git diff` were empty both before and after the baseline.
Dry runs `make -n all` and `make -n test` returned 0. Recipes contain compiler
invocations, `mkdir -p build/tests`, four asset symlinks, and executable launches.
Neither invokes clean, deletion, installation, privileges, network, Git writes,
or remote changes.

| Exact command | UTC start | UTC end | Exit | Result |
| --- | --- | --- | --- | --- |
| `make -B all` | 2026-11-01 23:11:47 | 2026-11-01 23:11:50 | 0 | Build passed, zero compiler warnings |
| `make test` | 2026-11-01 23:12:00 | 2026-11-01 23:12:56 | 0 | 18 executed, 18 passed, 0 failed, zero compiler warnings |

Logs: `/tmp/assemblyarchitect-bootstrap-baseline-build.log` and
`/tmp/assemblyarchitect-bootstrap-baseline-tests.log`. Exit statuses were captured
from Make itself with zsh `pipestatus[1]`, not from `tee`. No `-k` was used.
No failing tests or nondeterministic failures were observed. Intentional invalid
input diagnostics within passing tests are not compiler failures.

### Test Inventory

The 18 `tests/*_test.c` files all have explicit build-and-execute recipes:
tutorial_text, operand_highlights, level_config_hover, level_hover_layout,
operand_change_selection, sector_completion_navigation, buffer_label_layout,
execution_arrow_geometry, io_rail_geometry, text_box_layout,
tutorial_arrow_animation, highlight_animation_sync, modal_pointer_capture,
run_result_render_order, value_box_transport, registers_highlight_geometry,
display_resize, drawing_outline (each with `_test.c` suffix).

Discovery is an explicit Make list, not automatic wildcard execution. Plain
serial `make test` stops at the first failing compile or executable. No MAKEFLAGS
were set in the baseline. Tests use `SDL_Init(0)` and software surfaces/renderers
or pure helpers; none opens a video/audio device. No Xvfb is required. Tests use
`fonts/DOSVGA437.ttf`, image/data assets, level/tutorial configuration. Sound is
linked and symlinked but no test opens a mixer device. Executable-relative resource
lookup requires the four symlinks under `build/tests`, and some font paths require
repository-root working directory. Several tests directly include production C
with doubles; preserve their link exclusions.

`level_config_hover_test` creates an isolated missing level configuration with
`O_EXCL` and cleans only its own file/directory using atexit. It temporarily writes
under the tracked data directory via an asset symlink; normal completion removed
the fixture. Existing tracked data was unchanged. An abort could leave a fixture.

Generated outputs: root `assemblyArchitect`, `assemblyArchitect.dSYM`, 18 test
binaries and their `.dSYM` bundles under `build/tests`, and `data`, `fonts`, `img`,
`sound` symlinks there. These are ignored. No persistent tracked-file modifications.

### Toolchain and Platform Facts

- Darwin arm64; Apple Clang 21.0.0 (`clang-2100.3.34.2`), target
  `arm64-apple-darwin27.0.0`, installed in CommandLineTools.
- SDK 27.0 at `/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk`.
- No explicit `-std`; compiler macro reports `201710L` (GNU C17 default).
- Four frameworks at `/Library/Frameworks`: SDL2 2.32.0, SDL2_image 2.8.5,
  SDL2_ttf 2.24.0, SDL2_mixer 2.8.1 (verified header macros).
- Make enforces `-Wall -Wextra -Wpedantic -Werror` even on CFLAGS overrides.
- No included Make fragments, existing Actions, agent profiles, or governance
  templates. Existing JSON editor/compilation settings are macOS-specific.
- Relevant environment variables CC/CFLAGS/CPPFLAGS/LDFLAGS/MAKEFLAGS/SDKROOT/
  MACOSX_DEPLOYMENT_TARGET/DEVELOPER_DIR/SDL*/ASAN_OPTIONS/UBSAN_OPTIONS/
  LSAN_OPTIONS/CPATH/LIBRARY_PATH/DYLD* were unset. PATH includes Homebrew LLVM,
  Homebrew, system/CommandLineTools resolution, TeX, and VS Code helper paths;
  despite the LLVM PATH entry, `clang --version` resolves to Apple Clang above.
- Available: Ruby with YAML, clang-format 22.1.8, Node, Python 3.
  Unavailable on PATH: clang-tidy, cppcheck, actionlint, gcovr, Docker.
- README documents macOS Apple Silicon only. Linux/GCC/Windows are not verified.
- Local origin HEAD is `main`; origin URL names `erriveratec/assemblyarchitect`.
  README names a different account. Neither proves CODEOWNER write access.

## Cleanup Defect and Boundary

Original clean recursively deletes every `.dSYM`, uses `assemblyArchitect*`, and
removes swap files. It was dry-run only, never executed. Replace only with exact
known build artifacts; do not clean developer files or legacy ambiguous paths.
The revised recipe and dry-run must be shown to the user before any execution.

## Verification Updates

The local implementation was present; hosted validation and review of the analyzer
finding baseline were still pending at this point in the original local verification
record. No architecture stage had started. Independent read-only review reported
no introduced defects; a subsequent local integration check caught and fixed
Make ignoring FRAMEWORK_DIR from the
workflow environment. The default framework directory is now a conditional default,
and alternate paths gain a runtime search path without changing the native default.

### Original Local Results (Before Corrections)

| Exact command | Exit | Result / evidence |
| --- | --- | --- |
| `make -B all` | 0 | Final forced build passed; zero compiler warnings; `/tmp/assemblyarchitect-bootstrap-final-build.log` |
| `make verify` | 0 | Infrastructure, formatting, build and all 18 regression tests passed; `/tmp/assemblyarchitect-bootstrap-final-verify.log` |
| `make test-headless` | 0 | All 18 tests executed/passed with dummy SDL drivers; `/tmp/assemblyarchitect-bootstrap-headless.log` |
| `make test-sanitize` | 0 | Production built and all 18 ASan/UBSan tests executed/passed, including final rerun; `/tmp/assemblyarchitect-bootstrap-final-sanitize.log` |
| `make analyze` | 2 | Four existing Clang analyzer findings in unchanged select_level_sl.c; stops on third translation unit; `/tmp/assemblyarchitect-bootstrap-analysis.log` |
| `make check-infrastructure format-check` | 0 | Five agent profiles; test/recipe inventory; paths/links; all workflow/issue YAML and Make target references; 115 C/header and 33 infrastructure text files |
| `make -n clean` | 0 | Prints only `ruby scripts/clean_artifacts.rb --execute`; no deletion |
| `make clean-preview` | 0 | Reports 76 exact normal/sanitizer binary/debug paths; no deletion |
| `ruby -c scripts/clean_artifacts.rb` | 0 | Ruby syntax checked |
| `ruby -c scripts/check_infrastructure.rb` | 0 | Ruby syntax checked; format validator was executed successfully |
| `bash -n scripts/ci_frameworks.sh` | 0 | Syntax only; downloads/mounting not executed |
| Ruby `YAML.safe_load` on all seven `.github` YAML documents | 0 | Parsed workflows, composite action, issue form/config; not a hosted Actions run |
| `make -B all FRAMEWORK_DIR=/Library/Frameworks/.` | 0 | Non-default argument-selected framework include/link/rpath verified |
| `FRAMEWORK_DIR=/Library/Frameworks/. make -B all` | 0 | Environment-selected path verified, matching the workflow injection mechanism |
| `FRAMEWORK_DIR=/Library/Frameworks/. make test-value-box-transport` | 0 | Focused alternate-path binary executed successfully |
| `git diff --check` | 0 | No whitespace errors in tracked changes |
| `git diff --exit-code -- source tests data fonts img sound` | 0 | Production/tests/assets/saves unchanged |
| `git status --short --untracked-files=all -- data` | 0 | Empty; no leftover temporary level fixture |

Normal tests remain fail-fast; no `-k` was used for authoritative or final test
verification. No compiler warnings, failed tests, sanitizer findings, or flaky
tests were observed. Intentional invalid-display transition log messages in the
passing resize test are test-path diagnostics, not failed device acceptance.

Static analysis promoted `core.CallAndMessage` findings to errors at
`source/ui/screens/select_level_sl.c` lines 98, 581, 584, and 587 (uninitialized
button-pointer arguments). These are pre-existing analyzer reports, not adjudicated
production defects: production code is byte-for-byte unchanged. No suppression,
repair, or warning reduction was made. Later translation units were not analyzed
after the fail-fast stop. Static Analysis is now manual-only via
`workflow_dispatch` and is currently known to fail until bounded triage resolves
the four existing findings. Manual runs retain nonzero status and always upload
diagnostic logs; analysis is not made a required PR status before that review.
Healthy headless/sanitizer checks remain scheduled nightly. `make verify` excludes
this known-failing diagnostic baseline; it does not claim complete static safety.

### Files Created or Modified

32 files created, two existing files modified. The user's pre-existing untracked
bootstrap prompt is preserved, not counted as our creation. No CODEOWNERS file is
created because repository write-access identities cannot be verified locally.

| File | Purpose |
| --- | --- |
| `.github/copilot-instructions.md` | Mission, actual conventions, invariants, target direction, safety and completion policy |
| `.github/agents/refactor-planner.agent.md` | Read/search-only planning and issue drafts |
| `.github/agents/architecture-implementer.agent.md` | One approved reversible migration stage |
| `.github/agents/test-engineer.agent.md` | Independent deterministic regression coverage |
| `.github/agents/regression-reviewer.agent.md` | Read/search-only review without terminal/edit authority |
| `.github/agents/integration-fixer.agent.md` | Reproduce/classify/minimally repair integration failures |
| `.github/actions/sdl-frameworks/action.yml` | Shared native framework staging action |
| `.github/workflows/copilot-setup-steps.yml` | Required setup job ID, Ubuntu infrastructure-only validation |
| `.github/workflows/ci.yml` | Native full suite, headless, sanitizers, infrastructure/formatting |
| `.github/workflows/nightly.yml` | Implemented headless/sanitizer checks and visible analyzer diagnostics |
| `.github/workflows/codeql.yml` | C/C++ manual native build with limited security-event upload permission |
| `.github/ISSUE_TEMPLATE/refactor-stage.yml` | Fourteen required bounded-stage fields and acceptance checklist |
| `.github/ISSUE_TEMPLATE/config.yml` | Retain blank issue availability |
| `.github/pull_request_template.md` | Evidence, compatibility, ownership, risks, rollback and reviewer checklist |
| `docs/architecture/README.md` | Navigation and current-versus-proposed boundary |
| `docs/architecture/current-code-window.md` | Verified state, APIs, owners, coupling, tests and uncertainties |
| `docs/architecture/target-code-window.md` | Proposed seven-module boundaries and compatibility direction |
| `docs/architecture/invariants.md` | Testable model/control-flow/save/UI/memory/educational catalog |
| `docs/architecture/adr/README.md` | Usable ADR process/template |
| `docs/architecture/adr/0001-stable-label-identity.md` | Proposed logical identity with v1 save translation, not implementation |
| `docs/governance/agent-operating-model.md` | Human-scoped agent workflow, restrictions and escalation |
| `docs/governance/quality-gates.md` | Real commands, gate matrix, exceptions and planned gaps |
| `docs/governance/branch-protection.md` | Manual ruleset/security setup, intended check names, deferred owner proposal |
| `docs/governance/visual-baseline-policy.md` | Deterministic capture and explicit human approval |
| `docs/governance/release-smoke-test.md` | Actual game/player/level/save/control manual acceptance |
| `docs/refactor/master-plan.md` | Twelve dependency-ordered stage issue specifications |
| `docs/refactor/migration-state.md` | Bootstrap in progress; stages 1-11 not started |
| `docs/refactor/bootstrap-report.md` | PR draft, file plan, baseline, final evidence and limitations |
| `scripts/check_infrastructure.rb` | Executable YAML/profile/path/Make-target/test-inventory safety contracts |
| `scripts/check_format.rb` | Non-mutating explicit LF/NUL and infrastructure whitespace policy |
| `scripts/clean_artifacts.rb` | Validated exact artifacts; check/preview/execute modes |
| `scripts/ci_frameworks.sh` | Version-pinned native SDL2 staging under runner temporary directory |
| `Makefile` (modified) | Preserve existing targets; add real gates, safe cleanup and configurable framework path |
| `README.md` (modified) | Correct 18-test inventory, non-destructive commands and platform/gate limitations |

### Workflow and Agent Activation

- CI: pull requests, pushes to main, dispatch; literal jobs `macOS Clang`,
  `Headless SDL`, `ASan and UBSan`, `Infrastructure and Formatting`. Native jobs
  use macos-15; infrastructure-only checks use ubuntu-24.04. No compiler matrix
  is fabricated, so a matrix fail-fast setting is inapplicable.
- Nightly: daily 05:23 UTC and dispatch for `Full Headless and Sanitizers`.
  `Static Analysis` is manual-only: its job condition requires `workflow_dispatch`.
  It is currently known to fail on four existing findings until bounded triage is
  completed. No fictional save/level sweeps, coverage or random jobs.
- CodeQL: pull requests, main pushes, weekly Tuesday 06:41 UTC and dispatch;
  `Analyze C-C++`, c-cpp manual build. Remote plan/policy availability is unknown.
- Copilot setup: required `copilot-setup-steps` ID, dispatch and relevant file
  push/PR triggers; prints versions and runs actual infrastructure checks only.
- All external actions use stable major pins, contents is read-only, jobs have
  timeouts, diagnostic logs have 7/14-day retention, and pipelines retain failures.
  No secrets or self-hosted privileged runners are required. No workflow uses
  pull_request_target, continue-on-error, auto-golden updates or save uploads.
- Profiles use the installed environment's documented read/search/edit/execute
  aliases. Planner/reviewer have read/search only. No profile has merge/approval
  tools; edit-capable agents still require task authorization for side effects.

### Deliberate Adaptations and Known Limits

- No destructive clean was executed, original or revised. The proposed recipe and
  full preview were shown. Cleanup preserves app bundles (possible player saves),
  unrelated debug directories, editor swaps and ambiguous legacy artifacts. It
  handles default normal/sanitizer outputs, not arbitrary developer EXEC overrides.
- A true fresh-deletion build was not performed. `make -B all` forces a full
  single-command compile/link and is the user-authorized clean-equivalent baseline.
- Ubuntu/Linux/GCC production support is not established by this macOS-framework
  Makefile. Cloud Copilot's Ubuntu setup is therefore infrastructure-only, not a
  complete coding-agent production environment. An approved verified Linux port
  or supported native runner mechanism is prerequisite to cloud production work.
- Hosted native workflows use the exact installed framework versions as pinned
  official release DMG URLs, staged without sudo under RUNNER_TEMP. Their network
  availability, runner architecture,
  framework dependency loading and hosted SDL rasterization have not been tested
  locally. Do not equate YAML parsing or a local equivalent-path build with hosted
  success. SHA-256 digests are now pinned and checked before mounting; their exact
  provenance and collection limitations are recorded below.
- During the original bootstrap, no installs, sudo, network calls, commits,
  pushes, merges, new branches, GitHub issues/PRs, or remote settings changes were
  performed. Framework setup was syntax checked only. The subsequently authorized
  checksum collection below downloaded four exact DMGs without mounting. Hosted
  validation subsequently passed on `main` at commit `7d29cea`.
- No actionlint, clang-tidy, cppcheck, coverage tool, Linux container or CodeQL CLI
  is installed/used. Ruby parses YAML and validates specific repository contracts,
  not the complete GitHub Actions schema/service. Existing machine-specific
  compilation database/editor configuration was inspected and left untouched;
  analysis does not rely on it.
- Full clang-format enforcement is deferred rather than rewriting heterogeneous
  production formatting or disguising a zero-file check. The actual format policy
  scans 115 C/header files for LF/NUL plus all bootstrap infrastructure whitespace.
- No fake `test-unit`/`test-integration` aliases, coverage target/threshold, Valgrind
  job, save/level sweep, seeded model operations or screenshot-capture job was
  invented. Missing model/save seams remain Stage 1 work. Existing complete tests
  and their assertions are all preserved.
- Apple ASan uses detect_leaks=0 because LeakSanitizer is unsupported. No leak
  coverage or complete gameplay correctness is claimed. Manual macOS smoke testing
  passed; app packaging/signing and isolated executable save-round-trip coverage
  remain unverified.

### Manual Administrator Actions Remaining

Hosted CI and CodeQL passed on `main` at commit `7d29cea`. A human administrator
should confirm emitted check contexts before selecting them in the ruleset. Check
for organization-managed CodeQL; verify plan/policy availability and cloud
custom-agent tool interpretation. Configure the main
branch ruleset, human approvals, stale-review dismissal, conversation resolution,
current-branch policy, force-push/deletion blocks, restricted bypass and squash merge.
Confirm real owner write access and activate CODEOWNERS using the documented proposal.
Verify dependency graph, Dependabot alerts, secret scanning/push protection where
available, allowed-action/read-only token policy, and fork-workflow approval policy.
Assign nightly triage ownership and review the known analyzer baseline. No settings
have been enabled by this local work.

### Recommended Next Action

Prepare a separately reviewed Stage 1 characterization plan, especially for label
reorder/deletion and isolated v1 save round trips. Do not begin architectural
extraction until that plan is reviewed and approved.

## Approved Pre-Commit Corrections

### SDL DMG Checksums

Locally calculated from exact version-pinned assets downloaded over HTTPS
from official libsdl-org GitHub releases. GitHub release metadata did not
provide publisher-supplied digests.

These are not publisher attestations. Official release API metadata was matched
to the release tags and exact filenames below; each asset's `digest` was null.
The authorized collection downloaded only these four URLs into
`/tmp/assemblyarchitect-sdl-checksums.w5eNuj`, immediately calculated each hash with
`shasum -a 256`, and deleted each DMG. No DMG was mounted or executed, no framework
was installed or copied. `test ! -d "$checksum_dir"` passed after removal, and
Git status was unchanged by collection.

| Official repository | Release tag | Exact filename | Bytes | SHA-256 |
| --- | --- | --- | --- | --- |
| libsdl-org/SDL | release-2.32.0 | SDL2-2.32.0.dmg | 2046247 | `731af5f5b907008688ba2191cdd29dbe30d7f7c9c8ccf4ac501bd183f3d9b8e4` |
| libsdl-org/SDL_image | release-2.8.5 | SDL2_image-2.8.5.dmg | 1552538 | `9955c384384d734b0cc432df5be73c38d3beab55ef1dc45d385f8ca24ccdd3df` |
| libsdl-org/SDL_ttf | release-2.24.0 | SDL2_ttf-2.24.0.dmg | 1672901 | `677404e23a1a5f6efcbe64697471c5161283e4af5a6403dcb92235d7d83805cd` |
| libsdl-org/SDL_mixer | release-2.8.1 | SDL2_mixer-2.8.1.dmg | 1910018 | `d74052391ee4d91836bf1072a060f1d821710f3498a54996c66b9a17c79a72d1` |

Exact collection URLs:

```text
https://github.com/libsdl-org/SDL/releases/download/release-2.32.0/SDL2-2.32.0.dmg
https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.5/SDL2_image-2.8.5.dmg
https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.24.0/SDL2_ttf-2.24.0.dmg
https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.1/SDL2_mixer-2.8.1.dmg
```

Metadata provenance: the official GitHub API endpoint for each row is
`https://api.github.com/repos/{repository}/releases/tags/{release-tag}` using the
exact repository and tag in the table. Runtime rejects missing/malformed lowercase
64-character digests and mismatches before `hdiutil attach`. Curl retains fail,
retry and TLS restrictions, including HTTPS-only redirects. Successful output
identifies component, version and filename; unrelated environment output is removed.
Infrastructure validation enforces all four pins and uses offline command doubles
to check success ordering and refusal of missing, malformed and mismatched hashes.

### Portable Test Assets and Manual Analysis

`scripts/test_asset_links.rb` computes relative links with Ruby Pathname for both
`build/tests` and `build/sanitize/tests`. A phony setup prerequisite also checks
existing output directories. It preserves valid links to the correct repository
assets, repairs dangling links, refuses real files/directories and wrong live
links, and rejects symlinked output ancestors. Only output slots below `build/`
are eligible; production assets are never modified.

Infrastructure fixtures create links at both depths, preserve existing valid
links, repair dangling old-checkout links, refuse occupied real slots and wrong
targets, then rename a temporary checkout-shaped tree. All relative links still
resolve to the moved assets before setup runs again, and remain unchanged after
setup. Valid existing absolute links are also explicitly characterized as preserved.
The actual checkout is never relocated. Its normal links are relative; the existing
sanitizer links are valid absolute links and were preserved per the approved rule.
Only new/repaired links are guaranteed relative; relocating legacy absolute links
requires setup to repair them once they become dangling.

The Nightly workflow keeps both its daily cron and dispatch triggers. Its healthy
headless/sanitizer job remains scheduled; only Static Analysis has the explicit
`github.event_name == 'workflow_dispatch'` condition. The four existing findings
remain untouched, manual analysis still fails and uploads logs with `if: always()`.
No analysis command is authorized during this correction validation.

README's missing final newline is normalized to exactly one newline. The original
one-time bootstrap prompt remains untouched and unstaged. No Desktop dump or review
helper is added. Rollback is limited to this infrastructure/documentation patch;
do not alter production files, saves or analyzer findings.

### Correction Validation Results

Validation logs are under `/tmp/assemblyarchitect-corrections-validation.PImcri`.
All commands below passed on the corrected tree. The final `make verify` repeats
the infrastructure/format checks and all 18 tests after the final documentation
and relocation-test updates. No `make analyze` or cleanup execution was run.

| Command | Exit | Outcome |
| --- | --- | --- |
| Authorized Bash checksum collection | 0 | Four official DMGs hashed and removed; no mounting/installing |
| `test ! -d "$checksum_dir"` | 0 | Collection directory and its four DMGs absent |
| Compare Git status before/after checksum collection | 0 | Unchanged |
| `bash -n scripts/ci_frameworks.sh` | 0 | Syntax valid |
| `git diff --check` | 0 | No whitespace errors |
| `git diff --name-only -- source tests data fonts img sound` | 0 | Empty; protected tree unchanged |
| `make check-infrastructure` | 0 | Four digest pins; offline checksum ordering/failure guards; link safety/relocation; existing contracts |
| `make format-check` | 0 | 115 C/header and 34 infrastructure files checked |
| `make -n test` | 0 | All 18 execution recipes; asset helper included |
| `make -n test-sanitize` | 0 | Isolated sanitizer build/tests; asset helper included |
| `make -n clean` | 0 | Printed delegation only; no deletion |
| `make clean-preview` | 0 | 76 explicit paths previewed; no deletion |
| README exactly-one-final-newline assertion | 0 | Passed |
| `make -B all` | 0 | Forced native build; no compiler warnings |
| `make test` | 0 | 18 test binaries executed and passed |
| `make test-headless` | 0 | 18 test binaries executed and passed |
| `make test-sanitize` | 0 | Production sanitizer build; 18 test binaries executed and passed |
| `make verify` | 0 | Infrastructure, formatting, build and all 18 tests passed |
| Compiler/sanitizer diagnostic log scan | 0 | No compiler warnings/errors, ASan errors or UBSan runtime errors |

Development checks caught and repaired three local defects before final validation:
the initial combined relocation/YAML check exited 1 for workflow indentation;
`make check-infrastructure` exited 2 for a fixture comparison using macOS `/var`
instead of canonical `/private/var`; `make format-check` exited 2 for the new
helper's missing final newline. The same checks passed after each local repair.
Independent read-only review then identified that relocation assertions must run
before setup could repair links; that test weakness was corrected and revalidated.
No production defects were repaired or suppressed.

Files touched in this bounded correction: Makefile, README.md,
scripts/ci_frameworks.sh, scripts/check_infrastructure.rb,
scripts/test_asset_links.rb (new), .github/workflows/nightly.yml,
docs/governance/quality-gates.md, docs/governance/branch-protection.md,
docs/refactor/bootstrap-report.md and docs/refactor/migration-state.md.
Hosted workflow execution, actual DMG staging/loading, app packaging, real
display/audio gameplay and analyzer triage remain unverified in this correction.
Production source, tests, assets, saves, levels, tutorials and code-window
architecture are unchanged. No cleanup, staging, commit, push or remote change.

## Hosted Stage 0 Completion

Stage 0 validation completed on commit `7d29cea`.

Local native build, the full 18-test regression suite, the headless 18-test suite,
and the ASan/UBSan 18-test suite passed.

### Hosted GitHub Actions

- CI / macOS Clang: passed
- CI / Headless SDL: passed
- CI / ASan and UBSan: passed
- CI / Infrastructure and Formatting: passed
- CodeQL / Analyze C-C++: passed

The successful hosted runs used branch `main` at commit `7d29cea`.

GitHub Actions emitted non-blocking Node.js 20 deprecation annotations for
`actions/checkout@v4` and `actions/upload-artifact@v4`. These annotations did not
cause a job failure.

Static Analysis remains manual-only. Four pre-existing findings in
`source/ui/screens/select_level_sl.c` remain unresolved and were not changed or
suppressed.

### Manual macOS Smoke Test

Manual smoke testing passed for:

- Native build and application launch
- Title screen and player selection
- Existing progress loading
- Representative introductory and control-flow levels
- Instruction insertion and movement
- Operand selection and replacement
- Program execution and reset
- Execution arrow and highlight behavior
- Fonts, images, audio, and menus
- Application exit and restart

The application launched successfully and reported:

```text
Adaptive minimum window size set to 1280x720 for display 0
```
