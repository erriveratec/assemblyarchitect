# Manual GitHub Administration

Nothing in this document has been applied remotely. Local origin HEAD is `main`;
verify the actual default branch and Actions policy in GitHub before activation.

## Branch Ruleset

In Settings -> Rules -> Rulesets, create an active branch ruleset targeting the
default branch. Require pull requests, at least one human approval, CODEOWNER
approval when real owners are configured, dismissal of stale approvals, resolution
of conversations, and branches current with the default branch before merging.
Block force pushes and branch deletion. Restrict bypass to an audited human
emergency process; agents have no bypass or merge authority. Prefer squash merging
for one bounded migration stage, retaining the issue/evidence in the PR description.

Workflow jobs use stable literal names. After the first successful GitHub runs,
select the emitted check contexts in the ruleset rather than typing a guessed
workflow/job combination. Matrix/dynamic job naming is intentionally avoided.

| Workflow | Job name / intended check context | Required activation |
| --- | --- | --- |
| CI | macOS Clang | After hosted validation |
| CI | Headless SDL | After hosted validation |
| CI | ASan and UBSan | After hosted validation |
| CI | Infrastructure and Formatting | After hosted validation |
| CodeQL | Analyze C-C++ | After security service/policy validation |

Static Analysis is manual-only in the Nightly workflow: its job condition permits
only `workflow_dispatch`, not the daily schedule. It is currently known to fail
on four existing findings until bounded triage is completed. Healthy headless and
sanitizer checks remain scheduled. Do not require this knowingly failing analysis
check or silently ignore findings; manual runs retain failure and diagnostic logs.
Linux GCC/Clang checks are not configured: existing framework build is macOS-only.
Do not configure expected-but-nonexistent check names, which would block every PR.

## CODEOWNERS Deferred

Local metadata names `erriveratec/assemblyarchitect`; the README's old clone URL
names `ernestoriv7`. Commit authorship/remote ownership does not prove a user's
current repository write access. No CODEOWNERS file with an invented owner is added.
A human must verify actual usernames/teams and write permission, then apply this
comment-only proposal by replacing REAL_OWNER with a verified account/team:

```text
# Example only, not an active CODEOWNERS file.
# * @REAL_OWNER
# /source/code_window_cw* @REAL_OWNER
# /source/code_line_cl* @REAL_OWNER
# /source/storage/ @REAL_OWNER
# /source/file_fl* @REAL_OWNER
# /source/gameplay/ @REAL_OWNER
# /source/game_mechanics_mc* @REAL_OWNER
# /tests/ @REAL_OWNER
# /.github/workflows/ @REAL_OWNER
# /.github/copilot-instructions.md @REAL_OWNER
# /.github/agents/ @REAL_OWNER
# /docs/ @REAL_OWNER
```

There is no committed screenshot-golden directory. Add its exact path if/when a
reviewed deterministic baseline system exists. Later CODEOWNERS lines override
earlier matches; test pattern coverage before requiring owner approval.

## Security and Actions

Manually verify/enable dependency graph and Dependabot alerts where supported;
secret scanning and push protection where available; CodeQL code scanning plan
eligibility and organization-managed analysis (avoid duplicate CodeQL); Actions
allowed-action policy and read-only default token permissions. Require approval
for outside/fork contributors' workflows, and never run untrusted PRs on privileged
self-hosted runners. Review workflow changes before authorizing any execution.
No repository secrets are needed by these workflows. Never use
pull_request_target to build PR code. Pin actions at stable majors or stricter
organization-required SHAs. Restrict Actions write permissions except security
event upload in CodeQL. Enable cloud Copilot only after checking its supported
runner/tool policy; the native macOS setup limitation is documented in the report.

Review nightly artifacts on the next working day. Enforce release blocking for
Critical/Major findings even when the failing job is scheduled rather than PR-required.
