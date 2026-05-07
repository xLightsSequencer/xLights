#!/usr/bin/env bash

set -euo pipefail

MODE="${INCLUDE_POLICY_MODE:-strict}"
ALLOWLIST_FILE="ci_scripts/include_policy_allowlist.txt"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --mode)
            MODE="$2"
            shift 2
            ;;
        --allowlist)
            ALLOWLIST_FILE="$2"
            shift 2
            ;;
        *)
            echo "Unknown argument: $1" >&2
            exit 2
            ;;
    esac
done

if [[ "$MODE" != "warn" && "$MODE" != "strict" ]]; then
    echo "Invalid mode '$MODE'. Use 'warn' or 'strict'." >&2
    exit 2
fi

if ! command -v rg >/dev/null 2>&1; then
    echo "ripgrep (rg) is required." >&2
    exit 2
fi

CORE_DIRS=(
    "src-core/ai"
    "src-core/diagnostics"
    "src-core/discovery"
    "src-core/graphics"
    "src-core/import_export"
    "src-core/lyrics"
    "src-core/render"
    "src-core/effects"
    "src-core/models"
    "src-core/outputs"
    "src-core/controllers"
    "src-core/utils"
    "src-core/media"
    "src-core/XmlSerializer"
)

# `src-apple-core/` is the Apple-framework-isolated peer to `src-core/`. Code
# there exposes a pure C++ bridge that `src-core/` can call into; the
# dependency is one-way. Anything in `src-apple-core/` that includes a
# `src-core/` package header would re-introduce the licensing entanglement
# this split is meant to prevent.
APPLE_CORE_DIR="macOS/src-apple-core"

if [[ ! -f "$ALLOWLIST_FILE" ]]; then
    echo "Allowlist file '$ALLOWLIST_FILE' not found." >&2
    exit 2
fi

VIOLATIONS=()
BLOCKING=()
ALLOWED_COUNT=0

is_forbidden_include() {
    local include_path="$1"
    if [[ "$include_path" == *"xLightsMain.h" ]]; then
        return 0
    fi
    if [[ "$include_path" == *"xLightsApp.h" ]]; then
        return 0
    fi
    if [[ "$include_path" == ui/* || "$include_path" == */ui/* ]]; then
        return 0
    fi
    if [[ "$include_path" == common/* || "$include_path" == */common/* ]]; then
        return 0
    fi
    if [[ "$include_path" == wx/* ]]; then
        return 0
    fi
    return 1
}

while IFS= read -r entry; do
    file_path="${entry%%:*}"
    rest="${entry#*:}"
    line_num="${rest%%:*}"
    line_text="${rest#*:}"

    include_path="$(printf '%s' "$line_text" | sed -E 's/^[[:space:]]*#include[[:space:]]*[<"]([^">]+)[">].*/\1/')"

    if ! is_forbidden_include "$include_path"; then
        continue
    fi

    key="$file_path|$include_path"
    if grep -Fxq "$key" "$ALLOWLIST_FILE"; then
        ALLOWED_COUNT=$((ALLOWED_COUNT + 1))
        VIOLATIONS+=("ALLOWLISTED: ${file_path}:${line_num} -> ${include_path}")
    else
        BLOCKING+=("${file_path}:${line_num} -> ${include_path}")
    fi
done < <(rg -n --no-heading --color never '^\s*#include\s*(<wx/[^>]+>|"[^"]+")' "${CORE_DIRS[@]}" -g '*.{h,cpp}')

# Reverse direction: `src-apple-core/` must not include anything from
# `src-core/`. Detect by checking whether the literal include path resolves
# to a real file under `src-core/<include_path>`. wx is also forbidden from
# the bridge layer.
if [[ -d "$APPLE_CORE_DIR" ]]; then
    while IFS= read -r entry; do
        file_path="${entry%%:*}"
        rest="${entry#*:}"
        line_num="${rest%%:*}"
        line_text="${rest#*:}"

        include_path="$(printf '%s' "$line_text" | sed -E 's/^[[:space:]]*#(include|import)[[:space:]]*[<"]([^">]+)[">].*/\2/')"

        violating_target=""
        if [[ "$include_path" == wx/* ]]; then
            violating_target="$include_path"
        elif [[ -f "src-core/${include_path}" ]]; then
            violating_target="src-core/${include_path}"
        else
            continue
        fi

        key="$file_path|$include_path"
        if grep -Fxq "$key" "$ALLOWLIST_FILE"; then
            ALLOWED_COUNT=$((ALLOWED_COUNT + 1))
            VIOLATIONS+=("ALLOWLISTED: ${file_path}:${line_num} -> ${violating_target}")
        else
            BLOCKING+=("${file_path}:${line_num} -> ${violating_target}")
        fi
    done < <(rg -n --no-heading --color never '^\s*#(include|import)\s*(<wx/[^>]+>|"[^"]+")' "${APPLE_CORE_DIR}" -g '*.{h,hpp,cpp,mm,m}')
fi

# Detect stale allowlist entries (entries that no longer match any source violation)
STALE=()
while IFS= read -r line; do
    [[ -z "$line" || "$line" == \#* ]] && continue
    al_file="${line%%|*}"
    al_include="${line#*|}"
    if [[ ! -f "$al_file" ]]; then
        STALE+=("$line  (file does not exist)")
    elif ! rg -q "^\\s*#(include|import)\\s*[<\"]${al_include}[>\"]" "$al_file" 2>/dev/null; then
        STALE+=("$line  (include not found in file)")
    fi
done < "$ALLOWLIST_FILE"

echo "Core include boundary check"
echo "Mode: ${MODE}"
echo "Allowlist: ${ALLOWLIST_FILE}"
echo ""

if [[ ${#STALE[@]} -gt 0 ]]; then
    echo "Stale allowlist entries (${#STALE[@]}):"
    printf '  %s\n' "${STALE[@]}"
    echo ""
    echo "These entries can be removed from ${ALLOWLIST_FILE}."
    echo ""
fi

if [[ ${#VIOLATIONS[@]} -gt 0 ]]; then
    echo "Allowlisted violations (${ALLOWED_COUNT}):"
    printf '  %s\n' "${VIOLATIONS[@]}"
    echo ""
fi

if [[ ${#BLOCKING[@]} -gt 0 ]]; then
    echo "New violations (${#BLOCKING[@]}):"
    printf '  %s\n' "${BLOCKING[@]}"
    echo ""
    echo "To temporarily allow one, add an exact key to ${ALLOWLIST_FILE}:"
    echo "  path/to/file.cpp|relative/include/path.h"
    echo ""
    if [[ "$MODE" == "strict" ]]; then
        exit 1
    fi
    echo "WARN mode: not failing build (pass --mode strict or set INCLUDE_POLICY_MODE=strict to enforce)."
    exit 0
fi

echo "No new boundary violations found."
