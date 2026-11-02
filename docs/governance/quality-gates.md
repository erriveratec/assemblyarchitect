# Quality Gates

Run all commands from repository root. The verified platform is macOS/Clang with
SDL2 frameworks (versions in bootstrap-report.md). The full suite is 18 explicitly
registered tests, not the README's historical 15. No Xvfb or video/audio subsystem
is needed by software-renderer tests. Dummy-driver execution is an additional
environment check, not a way to skip tests or prove the actual game runs headlessly.
The application requests an accelerated renderer, unlike the tests.

| Gate | Command / workflow | Class | Failure meaning | Owner | Bypass policy |
| --- | --- | --- | --- | --- | --- |
| Build correctness | `make -B all` / CI | PR required | Compile/link or environment defect | Implementer, maintainer | No unsafe/incorrect build bypass |
| Full regression | `make test` / CI | PR required | First failing compile/assertion/executable | Test engineer, maintainer | No skipping or weakening tests |
| Unit characterization | Existing named `test-*` targets; Stage 1 additions | PR relevant | Model/helper regression | Test engineer | No bypass of semantic failures |
| Integration | Full `make test`, direct-C/link-double tests | PR required | Module interaction regression | Implementer | No bypass of Critical/Major failures |
| Headless UI | `make test-headless` | PR required | Software rendering/geometry/phase defect | Test engineer | No automatic golden update |
| ASan/UBSan | `make test-sanitize` | PR required | Memory or undefined behavior finding | Implementer/fixer | No bypass or undocumented suppression |
| Compiler static analysis | `make analyze` / Nightly dispatch-only Static Analysis job | Manual-only; currently known to fail until bounded triage | Four existing findings; later translation units unexamined | Maintainer | No suppression or success conversion; PR activation only after triage |
| Formatting | `make format-check` | PR required | Whitespace policy regression | Author | Legacy debt must be explicit, not reformatted silently |
| Infrastructure | `make check-infrastructure` | PR required | Invalid YAML/profile/path/target/cleanup contract | Maintainer | No bypass of unsafe automation |
| CodeQL | CodeQL workflow | PR when service enabled | C/C++ security finding | Security maintainer | No unresolved Critical/Major findings |
| Save compatibility | Stage 1 isolated fixtures; release checklist | Manual until executable suite exists | Lost progress/changed targets | Maintainer | Critical, no bypass |
| Visual baseline | Existing deterministic pixel assertions + policy | PR relevant/manual | Misleading or corrupted UI | Human reviewer | Dedicated explicit approval only |
| Real gameplay | Release smoke checklist | Manual release required | Device/UI/semantic integration defect | Human maintainer | Critical/Major blocks release |
| Coverage | No trustworthy measured baseline yet | Planned nightly | Coverage unknown, not zero or passing | Test engineer | No fabricated threshold |

Unit/integration labels describe test intent, not a newly invented partition.
Do not add aliases that falsely claim isolated suites. Current tests interleave
pure helpers, full module links, macro doubles, and software SDL assertions.
`make verify` combines reliable PR checks and the complete suite. Healthy headless
and sanitizer checks remain scheduled nightly. Static Analysis runs only through
`workflow_dispatch` and is currently known to fail on four existing findings until
bounded triage is completed. Manual runs preserve the failing exit status and
upload the diagnostic log with `if: always()`; this is not a passing gate.

Sanitizers must use the same flags for compile and link (Make invokes the compiler
once per binary). Set halt-on-error options for both ASan and UBSan. Apple ASan
does not support LeakSanitizer: macOS explicitly uses detect_leaks=0, not a claim
of leak coverage. Never combine ASan with Valgrind. Linux/Valgrind is deferred
because Linux is not currently supported by the framework Makefile and no local
Linux runtime/tool is available. Do not invent a green Linux/GCC job.

Coverage, save/level sweeps, seeded random operations, and deterministic screenshot
capture are planned Stage 1+ work, not implemented nightly jobs. Establish isolated
fixtures/model seams first; measure coverage with a verified toolchain, review
exclusions and then propose a non-decreasing ratchet. No hard threshold exists.
Existing animation tests use controlled phases; no arbitrary sleeps may be added.

Formatting initially checks actual C/header files for LF line endings and NUL bytes,
and all bootstrap text for trailing whitespace and final newlines. It is not a
full clang-format restyling gate; existing C trailing-space/indentation debt is
not silently presented as clang-format clean.
Repository C layout is heterogeneous and source churn is out of scope. Static
analysis uses Clang directly, not the machine-specific/stale compilation database;
the target iterates production translation units and fails at the first file with
findings. A failed run is not evidence that later files were analyzed.
No category-wide suppression is permitted.

Cleanup is an infrastructure safety gate: exact repository artifact paths only,
no swap deletion, recursive discovery/deletion, or executable-name wildcards.
Use the non-destructive clean preview and validator before proposing execution.
During bootstrap, do not execute even the revised cleanup without fresh approval.

Hosted workflow syntax can be checked locally, but hosted runner dependencies,
download availability, CodeQL access, and displayed check names require a real
GitHub run. Unverified remote execution is not a passing local result. Exceptions
for non-critical gates require written maintainer approval, rationale, bounded
expiry, and a linked follow-up; agents cannot grant exceptions.
