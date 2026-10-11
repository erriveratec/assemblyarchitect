#!/bin/bash

set -u

# The project root is the directory that contains this script.
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
OUTPUT_FILE="$HOME/Desktop/assemblyarchitect_project_dump.txt"

SOURCE_DIR="$PROJECT_ROOT/source"
DATA_DIR="$PROJECT_ROOT/data"
MAKEFILE_PATH="$PROJECT_ROOT/Makefile"
DUMP_SCRIPT_PATH="$PROJECT_ROOT/create_project_dump.sh"
TESTS_DIR="$PROJECT_ROOT/tests"

if [ ! -d "$SOURCE_DIR" ]; then
    printf 'Error: source directory not found: %s\n' "$SOURCE_DIR" >&2
    exit 1
fi

if [ ! -d "$DATA_DIR" ]; then
    printf 'Error: data directory not found: %s\n' "$DATA_DIR" >&2
    exit 1
fi

if [ ! -f "$MAKEFILE_PATH" ]; then
    printf 'Error: Makefile not found or not a regular file: %s\n' "$MAKEFILE_PATH" >&2
    exit 1
fi

if [ ! -f "$DUMP_SCRIPT_PATH" ]; then
    printf 'Error: dump script not found or not a regular file: %s\n' \
        "$DUMP_SCRIPT_PATH" >&2
    exit 1
fi

if [ ! -d "$TESTS_DIR" ]; then
    printf 'Error: tests directory not found: %s\n' "$TESTS_DIR" >&2
    exit 1
fi

INPUT_PATHS=(
    "$MAKEFILE_PATH"
    "$DUMP_SCRIPT_PATH"
    "$SOURCE_DIR"
    "$DATA_DIR"
    "$TESTS_DIR"
)

find_project_paths() {
    find "${INPUT_PATHS[@]}" \
        \( -name '.DS_Store' -o -name '.git' -o -name '*.dSYM' \
        -o -name '*.o' -o -name '*.a' -o -name '*.so' \
        -o -name '*.dylib' -o -name '*.exe' -o -name '*.out' \
        -o -path "$OUTPUT_FILE" \) -prune -o "$@" -print0 |
    while IFS= read -r -d '' PATH_ENTRY; do
        if [[ "$PATH_ENTRY" == "$TESTS_DIR/"* ]] && [ -f "$PATH_ENTRY" ]; then
            if file -b "$PATH_ENTRY" | LC_ALL=C grep -Eq 'Mach-O|ELF|PE32|MS-DOS executable|current ar archive'; then
                continue
            fi
        fi
        printf '%s\0' "$PATH_ENTRY"
    done
}

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
    while IFS= read -r -d '' PATH_ENTRY; do
        printf '%s\n' "${PATH_ENTRY#"$PROJECT_ROOT"/}"
    done < <(find_project_paths | LC_ALL=C sort -z)

    printf '\n'
    printf '============================================================\n'
    printf 'FILE CONTENTS\n'
    printf '============================================================\n'
    printf '\n'
} >> "$OUTPUT_FILE"

# Read the Makefile and every regular input file in stable sorted order.
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
    find_project_paths -type f | LC_ALL=C sort -z
)

printf 'Created: %s\n' "$OUTPUT_FILE"
printf 'Size: %s bytes\n' "$(stat -f '%z' "$OUTPUT_FILE")"
