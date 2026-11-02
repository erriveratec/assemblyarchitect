#!/usr/bin/env bash
set -euo pipefail

# Creates a review-safe text dump of all staged, unstaged, and untracked files
# related to the current Git working tree. Compatible with macOS Bash 3.2. It does not modify repository files,
# run builds, invoke cleanup, access the network, commit, or push.
#
# Usage:
#   bash assemblyarchitect_bootstrap_dump_desktop.sh
#   bash assemblyarchitect_bootstrap_dump_desktop.sh /path/to/output.txt

if ! command -v git >/dev/null 2>&1; then
  printf 'ERROR: git is required.\n' >&2
  exit 1
fi

repo_root="$(git rev-parse --show-toplevel 2>/dev/null || true)"
if [[ -z "$repo_root" ]]; then
  printf 'ERROR: run this script inside the Assembly Architect Git repository.\n' >&2
  exit 1
fi

cd "$repo_root"

repo_name="$(basename "$repo_root")"
timestamp="$(date -u '+%Y%m%dT%H%M%SZ')"

# Default to the current macOS user's Desktop. An explicit output path can
# still be passed as the first argument.
desktop_dir="${HOME}/Desktop"
if [[ ! -d "$desktop_dir" ]]; then
  printf 'ERROR: Desktop directory not found: %s\n' "$desktop_dir" >&2
  exit 1
fi
output="${1:-${desktop_dir}/${repo_name}_bootstrap_review_dump_${timestamp}.txt}"

# Refuse to write inside .git and avoid overwriting a repository input file.
case "$output" in
  "$repo_root/.git"|"$repo_root/.git/"*)
    printf 'ERROR: refusing to write inside .git.\n' >&2
    exit 1
    ;;
esac

mkdir -p "$(dirname "$output")"
tmp_output="$(mktemp "${TMPDIR:-/tmp}/assemblyarchitect-dump.XXXXXX")"
trap 'rm -f "$tmp_output"' EXIT

MAX_TEXT_BYTES=$((2 * 1024 * 1024))

section() {
  printf '\n\n================================================================================\n' >> "$tmp_output"
  printf '%s\n' "$1" >> "$tmp_output"
  printf '================================================================================\n' >> "$tmp_output"
}

run_and_capture() {
  local title="$1"
  shift
  section "$title"
  printf '$' >> "$tmp_output"
  printf ' %q' "$@" >> "$tmp_output"
  printf '\n' >> "$tmp_output"
  "$@" >> "$tmp_output" 2>&1 || {
    local status=$?
    printf '\n[command exited with status %s]\n' "$status" >> "$tmp_output"
  }
}

is_sensitive_path() {
  local path="$1"
  case "$path" in
    .env|.env.*|*/.env|*/.env.*|*.pem|*.key|*.p12|*.pfx|*.mobileprovision|\
    *credentials*|*secret*|*token*|*id_rsa*|*id_ed25519*|.npmrc|.pypirc|\
    .netrc|*/.netrc)
      return 0
      ;;
    *)
      return 1
      ;;
  esac
}

is_probably_text() {
  local path="$1"
  [[ -f "$path" ]] || return 1
  LC_ALL=C grep -Iq '' -- "$path" 2>/dev/null
}

file_size() {
  local path="$1"
  if stat -f '%z' "$path" >/dev/null 2>&1; then
    stat -f '%z' "$path"
  else
    stat -c '%s' "$path"
  fi
}

print_file_record() {
  local path="$1"

  printf '\n--------------------------------------------------------------------------------\n' >> "$tmp_output"
  printf 'FILE: %s\n' "$path" >> "$tmp_output"
  printf '%s\n' '--------------------------------------------------------------------------------' >> "$tmp_output"

  if [[ -L "$path" ]]; then
    printf '[symlink -> %s]\n' "$(readlink "$path")" >> "$tmp_output"
    return
  fi

  if [[ ! -e "$path" ]]; then
    printf '[file no longer exists in the working tree]\n' >> "$tmp_output"
    return
  fi

  if is_sensitive_path "$path"; then
    printf '[content omitted: path may contain credentials or secrets]\n' >> "$tmp_output"
    return
  fi

  local size
  size="$(file_size "$path")"
  printf '[size: %s bytes]\n' "$size" >> "$tmp_output"

  if (( size > MAX_TEXT_BYTES )); then
    printf '[content omitted: exceeds %s-byte safety limit]\n' "$MAX_TEXT_BYTES" >> "$tmp_output"
    return
  fi

  if ! is_probably_text "$path"; then
    printf '[content omitted: binary or non-text file]\n' >> "$tmp_output"
    return
  fi

  cat -- "$path" >> "$tmp_output"
  if [[ -s "$path" ]] && [[ "$(tail -c 1 -- "$path" | wc -l | tr -d ' ')" == "0" ]]; then
    printf '\n[NOTE: source file has no final newline]\n' >> "$tmp_output"
  fi
}

{
  printf 'ASSEMBLY ARCHITECT BOOTSTRAP REVIEW DUMP\n'
  printf 'Generated UTC: %s\n' "$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
  printf 'Repository root: %s\n' "$repo_root"
  printf 'Repository name: %s\n' "$repo_name"
  printf 'Host: %s %s\n' "$(uname -s)" "$(uname -m)"
  printf '\nSAFETY NOTES\n'
  printf -- '- Read-only with respect to repository files.\n'
  printf -- '- No build, test, cleanup, install, network, commit, push, or remote action is run.\n'
  printf -- '- Suspected credential files, binary files, and text files over 2 MiB are omitted.\n'
  printf -- '- The dump may still contain repository source and configuration. Review before sharing.\n'
} > "$tmp_output"

run_and_capture 'GIT REPOSITORY SUMMARY' git -c color.ui=false status --short --branch
run_and_capture 'GIT STATUS INCLUDING ALL UNTRACKED FILES' git -c color.ui=false status --short --untracked-files=all
run_and_capture 'CHANGED FILE STATISTICS' git -c color.ui=false diff --stat HEAD
run_and_capture 'CHANGED FILE NAME/STATUS' git -c color.ui=false diff --name-status HEAD
run_and_capture 'UNSTAGED DIFF' git -c color.ui=false diff --no-ext-diff --binary=false
run_and_capture 'STAGED DIFF' git -c color.ui=false diff --cached --no-ext-diff --binary=false
run_and_capture 'WHITESPACE CHECK' git -c color.ui=false diff --check HEAD
run_and_capture 'PRODUCTION-BOUNDARY CHECK' git -c color.ui=false diff --name-only HEAD -- source tests data fonts img sound

section 'WORKTREE FILE INVENTORY FOR REVIEW'
printf '%-4s %-10s %-12s %s\n' 'TYPE' 'STATUS' 'BYTES' 'PATH' >> "$tmp_output"

# Use a NUL-delimited, sorted unique list so spaces and special characters in
# paths are safe. Do not use associative arrays (`declare -A`), because the
# Bash 3.2 bundled with macOS does not support them.
changed_paths=()

while IFS= read -r -d '' path; do
  [[ -n "$path" ]] || continue
  changed_paths+=("$path")
done < <(
  {
    git diff --name-only -z HEAD
    git ls-files --others --exclude-standard -z
  } | sort -zu
)

for path in "${changed_paths[@]}"; do
  status="$(git status --short --untracked-files=all -- "$path" | head -n 1 | cut -c1-2)"
  if [[ -L "$path" ]]; then
    kind='LINK'
    bytes='-'
  elif [[ -f "$path" ]]; then
    kind='FILE'
    bytes="$(file_size "$path")"
  elif [[ -d "$path" ]]; then
    kind='DIR'
    bytes='-'
  else
    kind='GONE'
    bytes='-'
  fi
  printf '%-4s %-10s %-12s %s\n' "$kind" "${status:-??}" "$bytes" "$path" >> "$tmp_output"
done

section 'WORKFLOW ACTIONS AND EXTERNAL URL REFERENCES'
if [[ -d .github ]]; then
  rg -n --hidden --glob '!.git/**' \
    --glob '.github/**' \
    --glob 'scripts/**' \
    --glob 'Makefile' \
    '(uses:|https?://|curl[[:space:]]|wget[[:space:]]|permissions:|pull_request_target|secrets\.)' \
    .github scripts Makefile >> "$tmp_output" 2>&1 || true
else
  printf '[no .github directory]\n' >> "$tmp_output"
fi

section 'CUSTOM AGENT FRONTMATTER'
if compgen -G '.github/agents/*.agent.md' >/dev/null; then
  for path in .github/agents/*.agent.md; do
    printf '\n--- %s ---\n' "$path" >> "$tmp_output"
    awk 'NR == 1 && $0 == "---" {inside=1; print; next} inside {print; if ($0 == "---") exit}' "$path" >> "$tmp_output"
  done
else
  printf '[no custom agent files found]\n' >> "$tmp_output"
fi

section 'CURRENT CONTENTS OF ALL CREATED OR MODIFIED TEXT FILES'
if (( ${#changed_paths[@]} == 0 )); then
  printf '[working tree has no changed or untracked files]\n' >> "$tmp_output"
else
  for path in "${changed_paths[@]}"; do
    # Avoid recursively embedding the dump if the caller deliberately writes it in the repo.
    if [[ "$(cd "$(dirname "$output")" 2>/dev/null && pwd -P)/$(basename "$output")" == "$repo_root/$path" ]]; then
      printf '\nFILE: %s\n[dump output omitted to avoid recursion]\n' "$path" >> "$tmp_output"
      continue
    fi
    print_file_record "$path"
  done
fi

section 'REVIEW CHECKLIST'
cat >> "$tmp_output" <<'CHECKLIST'
Before sharing or committing, verify:
[ ] No credential, token, private key, provisioning profile, or personal save data is included.
[ ] The Makefile diff preserves existing target behavior.
[ ] Cleanup paths are explicit and repository-contained.
[ ] Workflow downloads use official sources, pinned versions, and SHA-256 verification.
[ ] Workflow permissions are least privilege and pull_request_target is absent.
[ ] GitHub Actions references are pinned according to repository policy.
[ ] Custom-agent frontmatter is tested in the installed VS Code/Copilot version.
[ ] No production source, existing tests, levels, saves, tutorials, or assets changed unexpectedly.
[ ] Hosted workflows and CodeQL are not described as validated until they run on GitHub.
CHECKLIST

mv "$tmp_output" "$output"
trap - EXIT

printf 'Created review dump:\n%s\n' "$output"
printf '\nSize: '
if stat -f '%z' "$output" >/dev/null 2>&1; then
  stat -f '%z bytes\n' "$output"
else
  stat -c '%s bytes' "$output"
  printf '\n'
fi
printf '\nReview it before sharing because it contains repository source and configuration.\n'
