#!/bin/bash

set -u

# The project root is the directory that contains this script.
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
OUTPUT_FILE="$HOME/Desktop/assemblyarchitect_project_dump.txt"

SOURCE_DIR="$PROJECT_ROOT/source"
DATA_DIR="$PROJECT_ROOT/data"

if [ ! -d "$SOURCE_DIR" ]; then
    printf 'Error: source directory not found: %s\n' "$SOURCE_DIR" >&2
    exit 1
fi

if [ ! -d "$DATA_DIR" ]; then
    printf 'Error: data directory not found: %s\n' "$DATA_DIR" >&2
    exit 1
fi

# Start with a new output file.
: > "$OUTPUT_FILE"

{
    printf 'ASSEMBLY ARCHITECT PROJECT DUMP\n'
    printf 'Generated: %s\n' "$(date '+%Y-%m-%d %H:%M:%S %Z')"
    printf 'Project root: %s\n' "$PROJECT_ROOT"
    printf '\n'
    printf '============================================================\n'
    printf 'DIRECTORY STRUCTURE\n'
    printf '============================================================\n'
    printf '\n'

    cd "$PROJECT_ROOT" || exit 1
    find source data \
        -not -name '.DS_Store' \
        -not -path '*/.git/*' \
        -print | LC_ALL=C sort

    printf '\n'
    printf '============================================================\n'
    printf 'FILE CONTENTS\n'
    printf '============================================================\n'
    printf '\n'
} >> "$OUTPUT_FILE"

# Read every regular file under source and data in stable sorted order.
while IFS= read -r -d '' FILE; do
    RELATIVE_PATH="${FILE#"$PROJECT_ROOT"/}"

    {
        printf '\n'
        printf '############################################################\n'
        printf 'FILE: %s\n' "$RELATIVE_PATH"
        printf '############################################################\n'
        printf '\n'
    } >> "$OUTPUT_FILE"

    # Keep the generated dump readable and uploadable. Binary files are
    # recorded in the structure but their raw bytes are not inserted.
    if LC_ALL=C grep -Iq '' "$FILE"; then
        cat "$FILE" >> "$OUTPUT_FILE"

        # Ensure the next file header starts on a new line even when the
        # source file has no trailing newline.
        printf '\n' >> "$OUTPUT_FILE"
    else
        {
            printf '[BINARY FILE OMITTED FROM TEXT DUMP]\n'
            printf 'Type: %s\n' "$(file -b "$FILE")"
            printf 'Size: %s bytes\n' "$(stat -f '%z' "$FILE")"
        } >> "$OUTPUT_FILE"
    fi
done < <(
    find "$SOURCE_DIR" "$DATA_DIR" \
        -type f \
        -not -name '.DS_Store' \
        -not -path '*/.git/*' \
        -print0 | LC_ALL=C sort -z
)

printf 'Created: %s\n' "$OUTPUT_FILE"
printf 'Size: %s bytes\n' "$(stat -f '%z' "$OUTPUT_FILE")"
