#!/usr/bin/env bash
set -euo pipefail

# Assembly Architect context bundle generator for macOS Bash 3.2.
# Produces two complementary text files on the Desktop:
#   1. Project context: tracked architecture, build, APIs, tests, and source text.
#   2. Worktree review: staged, unstaged, and untracked change evidence.
#
# Read-only with respect to the repository. It does not build, test, clean,
# install, access the network, commit, push, or modify remote settings.
#
# Usage:
#   bash create_copilot_context_bundle.sh
#   bash create_copilot_context_bundle.sh /path/to/output-directory

if ! command -v git >/dev/null 2>&1; then
  printf 'ERROR: git is required.\n' >&2
  exit 1
fi

repo_root="$(git rev-parse --show-toplevel 2>/dev/null || true)"
if [[ -z "$repo_root" ]]; then
  printf 'ERROR: run inside the Assembly Architect Git repository.\n' >&2
  exit 1
fi

cd "$repo_root"
repo_name="$(basename "$repo_root")"
timestamp="$(date -u '+%Y%m%dT%H%M%SZ')"
default_output_dir="${HOME}/Desktop"
output_dir="${1:-$default_output_dir}"

if [[ ! -d "$output_dir" ]]; then
  printf 'ERROR: output directory does not exist: %s\n' "$output_dir" >&2
  exit 1
fi

case "$(cd "$output_dir" && pwd -P)" in
  "$repo_root/.git"|"$repo_root/.git/"*)
    printf 'ERROR: refusing to write inside .git.\n' >&2
    exit 1
    ;;
esac

context_output="${output_dir}/${repo_name}_copilot_project_context_${timestamp}.txt"
review_output="${output_dir}/${repo_name}_copilot_worktree_review_${timestamp}.txt"
context_tmp="$(mktemp "${TMPDIR:-/tmp}/assemblyarchitect-context.XXXXXX")"
review_tmp="$(mktemp "${TMPDIR:-/tmp}/assemblyarchitect-review.XXXXXX")"
trap 'rm -f "$context_tmp" "$review_tmp"' EXIT HUP INT TERM

MAX_TEXT_BYTES=$((2 * 1024 * 1024))

section() {
  local output="$1"
  local title="$2"
  printf '\n\n================================================================================\n' >> "$output"
  printf '%s\n' "$title" >> "$output"
  printf '================================================================================\n' >> "$output"
}

run_capture() {
  local output="$1"
  local title="$2"
  shift 2
  section "$output" "$title"
  printf '$' >> "$output"
  printf ' %q' "$@" >> "$output"
  printf '\n' >> "$output"
  "$@" >> "$output" 2>&1 || {
    local status=$?
    printf '\n[command exited with status %s]\n' "$status" >> "$output"
  }
}

file_size() {
  local path="$1"
  if stat -f '%z' "$path" >/dev/null 2>&1; then
    stat -f '%z' "$path"
  else
    stat -c '%s' "$path"
  fi
}

is_sensitive_path() {
  local path="$1"
  case "$path" in
    .env|.env.*|*/.env|*/.env.*|*.pem|*.key|*.p12|*.pfx|*.mobileprovision|\
    *credentials*|*secret*|*token*|*id_rsa*|*id_ed25519*|.npmrc|.pypirc|\
    .netrc|*/.netrc|data/saves/*)
      return 0
      ;;
    *)
      return 1
      ;;
  esac
}

is_generated_or_dump_path() {
  local path="$1"
  case "$path" in
    build/*|AssemblyArchitect.app/*|*.dSYM/*|assemblyArchitect|\
    *_project_dump*.txt|*_context_*.txt|*_review_*.txt|\
    create_project_dump.sh|agents_dump_script.sh|\
    assemblyarchitect_bootstrap_dump*.sh)
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

print_worktree_file() {
  local output="$1"
  local path="$2"
  printf '\n--------------------------------------------------------------------------------\n' >> "$output"
  printf 'FILE: %s\n' "$path" >> "$output"
  printf '%s\n' '--------------------------------------------------------------------------------' >> "$output"

  if [[ -L "$path" ]]; then
    printf '[symlink -> %s]\n' "$(readlink "$path")" >> "$output"
    return
  fi
  if [[ ! -e "$path" ]]; then
    printf '[deleted from working tree]\n' >> "$output"
    return
  fi
  if is_sensitive_path "$path"; then
    printf '[content omitted: sensitive or player-save path]\n' >> "$output"
    return
  fi
  if is_generated_or_dump_path "$path"; then
    printf '[content omitted: generated, build, or dump utility]\n' >> "$output"
    return
  fi

  local size
  size="$(file_size "$path")"
  printf '[size: %s bytes]\n' "$size" >> "$output"
  if (( size > MAX_TEXT_BYTES )); then
    printf '[content omitted: exceeds %s-byte limit]\n' "$MAX_TEXT_BYTES" >> "$output"
    return
  fi
  if ! is_probably_text "$path"; then
    printf '[content omitted: binary or non-text file]\n' >> "$output"
    return
  fi
  cat -- "$path" >> "$output"
  if [[ -s "$path" ]] && [[ "$(tail -c 1 -- "$path" | wc -l | tr -d ' ')" == "0" ]]; then
    printf '\n[NOTE: file has no final newline]\n' >> "$output"
  fi
}

print_head_file() {
  local output="$1"
  local path="$2"
  printf '\n--------------------------------------------------------------------------------\n' >> "$output"
  printf 'FILE AT HEAD: %s\n' "$path" >> "$output"
  printf '%s\n' '--------------------------------------------------------------------------------' >> "$output"

  if is_sensitive_path "$path"; then
    printf '[content omitted: sensitive or player-save path]\n' >> "$output"
    return
  fi
  if is_generated_or_dump_path "$path"; then
    printf '[content omitted: generated, build, or dump path]\n' >> "$output"
    return
  fi

  local size
  size="$(git cat-file -s "HEAD:$path" 2>/dev/null || printf '0')"
  printf '[HEAD size: %s bytes]\n' "$size" >> "$output"
  if (( size > MAX_TEXT_BYTES )); then
    printf '[content omitted: exceeds %s-byte limit]\n' "$MAX_TEXT_BYTES" >> "$output"
    return
  fi

  if git show "HEAD:$path" | LC_ALL=C grep -Iq '' 2>/dev/null; then
    git show "HEAD:$path" >> "$output"
  else
    printf '[content omitted: binary or non-text file]\n' >> "$output"
  fi
}

cat > "$context_tmp" <<HEADER
ASSEMBLY ARCHITECT COPILOT PROJECT CONTEXT
Generated UTC: $(date -u '+%Y-%m-%dT%H:%M:%SZ')
Repository root: $repo_root
Repository name: $repo_name
Host: $(uname -s) $(uname -m)

PURPOSE
- Durable project context for Microsoft Copilot Chat and GitHub Copilot in VS Code.
- Describes the committed repository at HEAD, architecture, build/test commands,
  APIs, agents, workflows, and tracked source/test text.
- Does not include player-save contents, binary assets, secrets, or build outputs.
- Pair this file with the worktree review file when asking about uncommitted work.
HEADER

run_capture "$context_tmp" 'GIT IDENTITY AND RECENT HISTORY' git -c color.ui=false log --oneline --decorate -15
run_capture "$context_tmp" 'CURRENT BRANCH AND REMOTES' git -c color.ui=false status --short --branch
run_capture "$context_tmp" 'TAGS AND CHECKPOINTS' git -c color.ui=false tag --list --sort=-creatordate
run_capture "$context_tmp" 'TRACKED REPOSITORY MANIFEST' git ls-tree -r --name-only HEAD
run_capture "$context_tmp" 'TOP-LEVEL DIRECTORY STRUCTURE' find . -maxdepth 3 -path './.git' -prune -o -path './build' -prune -o -path './AssemblyArchitect.app' -prune -o -print

section "$context_tmp" 'BUILD, TEST, AND QUALITY ENTRY POINTS'
printf '%s\n' 'Make targets discovered from the committed Makefile:' >> "$context_tmp"
git show HEAD:Makefile | awk -F: '/^[A-Za-z0-9_.%()$@+\/-]+[[:space:]]*:/ { print $1 }' | sort -u >> "$context_tmp" || true
printf '\nCommands normally used for refactor validation:\n' >> "$context_tmp"
printf '%s\n' 'make check-infrastructure' 'make format-check' 'make -B all' 'make test' 'make test-headless' 'make test-sanitize' 'make verify' 'make analyze (known baseline may fail; read governance docs)' >> "$context_tmp"

section "$context_tmp" 'PUBLIC C API INVENTORY'
for header in source/*.h source/**/*.h; do
  [[ -f "$header" ]] || continue
  case "$header" in
    *internal.h) continue ;;
  esac
  symbols="$(grep -E '^[[:space:]]*(extern[[:space:]]+)?(const[[:space:]]+)?(void|bool|int|float|double|size_t|SDL_Rect|[A-Za-z_][A-Za-z0-9_]*_t[[:space:]]*\*?)[[:space:]*]+[A-Za-z_][A-Za-z0-9_]*[[:space:]]*\(' "$header" 2>/dev/null || true)"
  if [[ -n "$symbols" ]]; then
    printf '\n--- %s ---\n%s\n' "$header" "$symbols" >> "$context_tmp"
  fi
done

section "$context_tmp" 'MODULE AND TEST SIZE SUMMARY'
for path in source/*.c source/**/*.c tests/*.c; do
  [[ -f "$path" ]] || continue
  printf '%8s  %s\n' "$(wc -l < "$path" | tr -d ' ')" "$path" >> "$context_tmp"
done

section "$context_tmp" 'GITHUB WORKFLOW, AGENT, AND EXTERNAL REFERENCE SUMMARY'
if command -v rg >/dev/null 2>&1; then
  rg -n --hidden --glob '!.git/**' --glob '.github/**' --glob 'scripts/**' --glob 'Makefile' \
    '(name:|on:|runs-on:|permissions:|uses:|https?://|curl[[:space:]]|wget[[:space:]]|pull_request_target|secrets\.)' \
    .github scripts Makefile >> "$context_tmp" 2>&1 || true
else
  grep -R -n -E 'name:|runs-on:|permissions:|uses:|https?://|pull_request_target|secrets\.' .github scripts Makefile >> "$context_tmp" 2>&1 || true
fi

section "$context_tmp" 'CUSTOM AGENT FRONTMATTER'
if compgen -G '.github/agents/*.agent.md' >/dev/null; then
  for path in .github/agents/*.agent.md; do
    printf '\n--- %s ---\n' "$path" >> "$context_tmp"
    awk 'NR == 1 && $0 == "---" {inside=1; print; next} inside {print; if ($0 == "---") exit}' "$path" >> "$context_tmp"
  done
else
  printf '[no custom agents found]\n' >> "$context_tmp"
fi

section "$context_tmp" 'COMMITTED TEXT FILE CONTENTS'
while IFS= read -r path; do
  case "$path" in
    README.md|Makefile|.gitignore|.github/*.md|.github/**/*.md|.github/**/*.yml|.github/**/*.yaml|\
    docs/*.md|docs/**/*.md|scripts/*.rb|scripts/*.sh|source/*.c|source/*.h|source/**/*.c|source/**/*.h|tests/*.c|tests/*.h)
      print_head_file "$context_tmp" "$path"
      ;;
  esac
done < <(git ls-tree -r --name-only HEAD)

cat > "$review_tmp" <<HEADER
ASSEMBLY ARCHITECT COPILOT WORKTREE REVIEW
Generated UTC: $(date -u '+%Y-%m-%dT%H:%M:%SZ')
Repository root: $repo_root
Repository name: $repo_name
Host: $(uname -s) $(uname -m)

PURPOSE
- Review staged, unstaged, deleted, and untracked work relative to HEAD.
- Pair with the project-context file for full background.
- No build, test, cleanup, network, commit, push, or remote action is run.
HEADER

run_capture "$review_tmp" 'GIT REPOSITORY SUMMARY' git -c color.ui=false status --short --branch
run_capture "$review_tmp" 'GIT STATUS INCLUDING ALL UNTRACKED FILES' git -c color.ui=false status --short --untracked-files=all
run_capture "$review_tmp" 'CHANGED FILE STATISTICS' git -c color.ui=false diff --stat HEAD
run_capture "$review_tmp" 'CHANGED FILE NAME/STATUS' git -c color.ui=false diff --name-status HEAD
run_capture "$review_tmp" 'UNSTAGED DIFF' git -c color.ui=false diff --no-ext-diff
run_capture "$review_tmp" 'STAGED DIFF' git -c color.ui=false diff --cached --no-ext-diff
run_capture "$review_tmp" 'WHITESPACE CHECK' git -c color.ui=false diff --check HEAD
run_capture "$review_tmp" 'PROTECTED-PATH CHANGE CHECK' git -c color.ui=false diff --name-only HEAD -- data/saves data/levels data/tutorials fonts img sound

section "$review_tmp" 'WORKTREE FILE INVENTORY'
printf '%-5s %-8s %-12s %s\n' 'TYPE' 'STATUS' 'BYTES' 'PATH' >> "$review_tmp"
changed_paths=()
changed_path_count=0
while IFS= read -r -d '' path; do
  [[ -n "$path" ]] || continue
  changed_paths+=("$path")
  changed_path_count=$((changed_path_count + 1))
done < <({ git diff --name-only -z HEAD; git ls-files --others --exclude-standard -z; } | sort -zu)

for path in ${changed_paths[@]+"${changed_paths[@]}"}; do
  status="$(git status --short --untracked-files=all -- "$path" | head -n 1 | cut -c1-2)"
  if [[ -L "$path" ]]; then
    kind='LINK'; bytes='-'
  elif [[ -f "$path" ]]; then
    kind='FILE'; bytes="$(file_size "$path")"
  elif [[ -d "$path" ]]; then
    kind='DIR'; bytes='-'
  else
    kind='GONE'; bytes='-'
  fi
  printf '%-5s %-8s %-12s %s\n' "$kind" "${status:-??}" "$bytes" "$path" >> "$review_tmp"
done

section "$review_tmp" 'CURRENT CONTENTS OF CREATED OR MODIFIED TEXT FILES'
if (( changed_path_count == 0 )); then
  printf '[working tree has no changed or untracked files]\n' >> "$review_tmp"
else
  for path in "${changed_paths[@]}"; do
    print_worktree_file "$review_tmp" "$path"
  done
fi

section "$review_tmp" 'REVIEW CHECKLIST'
cat >> "$review_tmp" <<'CHECKLIST'
[ ] No secret, credential, key, provisioning profile, or player-save content is included.
[ ] Every changed production behavior has a regression test.
[ ] The staged inventory excludes unrelated saves, levels, tutorials, and assets.
[ ] Makefile and workflow commands reference real targets.
[ ] Compiler warnings, tests, sanitizers, and checksum verification were not weakened.
[ ] Uncommitted work is reviewed against the committed project context.
[ ] Hosted checks are not claimed as passing unless a GitHub run was observed.
CHECKLIST

mv "$context_tmp" "$context_output"
mv "$review_tmp" "$review_output"
trap - EXIT HUP INT TERM

printf 'Created Copilot context bundle:\n'
printf '  Project context: %s\n' "$context_output"
printf '  Worktree review: %s\n' "$review_output"
printf '\nReview both files for sensitive material before sharing.\n'
