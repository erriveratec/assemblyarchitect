#!/usr/bin/env bash
set -euo pipefail

test "$(uname -s)" = Darwin
: "${RUNNER_TEMP:?Run only in an explicitly authorized hosted macOS job}"
: "${GITHUB_ENV:?GitHub environment file is required}"
framework_dir="$RUNNER_TEMP/assemblyarchitect-frameworks"
mount_dir="$RUNNER_TEMP/assemblyarchitect-framework-mount"
mkdir -p "$framework_dir" "$mount_dir"
trap 'hdiutil detach "$mount_dir" >/dev/null 2>&1 || true' EXIT

while read -r repository version framework expected_digest; do
    if [[ ! "$expected_digest" =~ ^[0-9a-f]{64}$ ]]; then
        printf 'Missing or malformed SHA-256 for %s %s\n' "$framework" "$version" >&2
        exit 1
    fi
    filename="${framework}-${version}.dmg"
    image="$RUNNER_TEMP/${framework}-${version}.dmg"
    curl --fail --location --retry 3 --proto '=https' --proto-redir '=https' --tlsv1.2 \
        "https://github.com/libsdl-org/${repository}/releases/download/release-${version}/${filename}" \
        --output "$image"
    digest_record="$(shasum -a 256 "$image")"
    actual_digest="${digest_record%% *}"
    if [[ "$actual_digest" != "$expected_digest" ]]; then
        printf 'SHA-256 mismatch for %s %s %s\n' "$framework" "$version" "$filename" >&2
        exit 1
    fi
    printf '%s %s %s: SHA-256 verified\n' "$framework" "$version" "$filename"
    hdiutil attach -readonly -nobrowse -mountpoint "$mount_dir" "$image"
    test -d "$mount_dir/${framework}.framework"
    ditto "$mount_dir/${framework}.framework" "$framework_dir/${framework}.framework"
    hdiutil detach "$mount_dir"
done <<'FRAMEWORKS'
SDL 2.32.0 SDL2 731af5f5b907008688ba2191cdd29dbe30d7f7c9c8ccf4ac501bd183f3d9b8e4
SDL_image 2.8.5 SDL2_image 9955c384384d734b0cc432df5be73c38d3beab55ef1dc45d385f8ca24ccdd3df
SDL_ttf 2.24.0 SDL2_ttf 677404e23a1a5f6efcbe64697471c5161283e4af5a6403dcb92235d7d83805cd
SDL_mixer 2.8.1 SDL2_mixer d74052391ee4d91836bf1072a060f1d821710f3498a54996c66b9a17c79a72d1
FRAMEWORKS

printf 'FRAMEWORK_DIR=%s\nDYLD_FRAMEWORK_PATH=%s\n' "$framework_dir" "$framework_dir" >> "$GITHUB_ENV"
