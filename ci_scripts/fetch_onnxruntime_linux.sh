#!/usr/bin/env bash
#
# fetch_onnxruntime_linux.sh — download the prebuilt ONNX Runtime release and
# stage it where the Linux make build expects it, for HTDemucs stem
# separation (src-core/media/StemSeparator.cpp, HAVE_ORT backend). Windows
# gets its (DirectML) copy of the same pinned version via
# ci_scripts/fetch_dependencies.ps1; macOS uses CoreML instead and never
# calls this script.
#
# The artifacts are GitHub release assets of github.com/microsoft/onnxruntime.
# The pinned version lives in ci_scripts/onnxruntime_version.txt and is shared
# with the Windows fetch so both platforms run the same model/runtime pairing.
#
# If the download fails (e.g. offline build) the script prints a notice and
# exits non-zero, staging nothing partial. ONNX Runtime is a required
# dependency for the Linux build, so the Makefile stops rather than silently
# dropping stem separation.
#
# XLIGHTS_ONNXRUNTIME_OPTIONAL=1 (set by the Ubuntu CI workflow) softens that:
# a failed fetch still stages nothing, but exits 0 so the build proceeds
# without ONNX Runtime — build_scripts/linux/onnxruntime.mak then drops the
# HAVE_ORT define/include/link flags because the staged library is absent,
# StemSeparator.cpp compiles to its no-backend fallback, and the stem menu
# entries are simply not offered. Local builds keep the strict behavior so a
# silently degraded developer tree can't go unnoticed. Useful when the
# upstream github.com/microsoft/onnxruntime release is unavailable (it has
# gone 404 for entire GitHub outages before) and CI should still build
# everything else.
#
# Usage:
#   ci_scripts/fetch_onnxruntime_linux.sh [platform] [--force]
#     platform : linux-x64 | linux-aarch64 (default: auto-detect)
#     --force  : re-download even if the library is already staged
#
set -u

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Shared exit path for "we could not stage ONNX Runtime". Default behavior is
# strict (exit 1, the Makefile stops). With XLIGHTS_ONNXRUNTIME_OPTIONAL=1 the
# failure is reported and the script exits 0 so the build continues without
# the library (see the header comment).
fail() {
    if [ "${XLIGHTS_ONNXRUNTIME_OPTIONAL:-}" = "1" ]; then
        echo "fetch_onnxruntime_linux: XLIGHTS_ONNXRUNTIME_OPTIONAL=1 — continuing without ONNX Runtime; HTDemucs stem separation will be DISABLED." >&2
        exit 0
    fi
    echo "$1" >&2
    exit 1
}

VERSION="$(tr -d ' \t\r\n' < "$SCRIPT_DIR/onnxruntime_version.txt")"
BASE_URL="https://github.com/microsoft/onnxruntime/releases/download/v${VERSION}"

PLATFORM=""
FORCE=0
for arg in "$@"; do
    case "$arg" in
        --force) FORCE=1 ;;
        linux-x64|linux-aarch64) PLATFORM="$arg" ;;
        *) echo "fetch_onnxruntime_linux: ignoring unknown argument '$arg'" >&2 ;;
    esac
done

if [ -z "$PLATFORM" ]; then
    uname_m="$(uname -m 2>/dev/null || echo unknown)"
    case "$uname_m" in
        aarch64|arm64) PLATFORM="linux-aarch64" ;;
        *)             PLATFORM="linux-x64" ;;
    esac
fi

ASSET="onnxruntime-${PLATFORM}-${VERSION}.tgz"
URL="${BASE_URL}/${ASSET}"

LIB_DIR="lib/linux"
INC_DIR="include/onnxruntime"
SENTINEL="$ROOT_DIR/$LIB_DIR/libonnxruntime.so"
# Records which VERSION the staged copy came from, so a bump to
# onnxruntime_version.txt actually re-fetches (see the skip check below).
STAMP="$ROOT_DIR/$LIB_DIR/.onnxruntime_version"

STAMPED=""
[ -f "$STAMP" ] && STAMPED="$(tr -d ' \t\r\n' < "$STAMP")"
if [ "$FORCE" -eq 0 ] && [ -f "$SENTINEL" ] && [ "$STAMPED" = "$VERSION" ]; then
    echo "fetch_onnxruntime_linux: onnxruntime $VERSION already staged at $LIB_DIR — skipping (use --force to re-download)."
    exit 0
fi
if [ -f "$SENTINEL" ]; then
    echo "fetch_onnxruntime_linux: staged copy is ${STAMPED:-unversioned}, want $VERSION — re-fetching."
fi

echo "fetch_onnxruntime_linux: $PLATFORM $VERSION -> staging into $ROOT_DIR"

TMP_DIR="$(mktemp -d 2>/dev/null || mktemp -d -t ort)"
cleanup() { rm -rf "$TMP_DIR"; }
trap cleanup EXIT

TARBALL="$TMP_DIR/$ASSET"
. "$SCRIPT_DIR/gh_release_fetch.sh"
if ! gh_release_fetch microsoft/onnxruntime "v$VERSION" "$ASSET" "$TARBALL"; then
    fail "fetch_onnxruntime_linux: download failed ($URL) — stem separation will be disabled."
fi

if ! tar -xzf "$TARBALL" -C "$TMP_DIR"; then
    fail "fetch_onnxruntime_linux: could not extract $ASSET."
fi

# The tarball expands to a single top-level onnxruntime-<platform>-<version>/ dir
# containing include/ and lib/.
EXTRACTED_DIR="$TMP_DIR/onnxruntime-${PLATFORM}-${VERSION}"
if [ ! -d "$EXTRACTED_DIR" ]; then
    EXTRACTED_DIR="$(find "$TMP_DIR" -maxdepth 1 -type d -name 'onnxruntime-*' | head -n1)"
fi
if [ -z "$EXTRACTED_DIR" ] || [ ! -d "$EXTRACTED_DIR/include" ] || [ ! -d "$EXTRACTED_DIR/lib" ]; then
    fail "fetch_onnxruntime_linux: unexpected archive layout in $ASSET."
fi

mkdir -p "$ROOT_DIR/$INC_DIR"
cp -f "$EXTRACTED_DIR/include/"*.h "$ROOT_DIR/$INC_DIR/"

mkdir -p "$ROOT_DIR/$LIB_DIR"
rm -f "$ROOT_DIR/$LIB_DIR"/libonnxruntime.so*
cp -fP "$EXTRACTED_DIR/lib/"libonnxruntime.so* "$ROOT_DIR/$LIB_DIR/"

# Written last, so an interrupted run leaves no stamp and the next one re-fetches.
printf '%s\n' "$VERSION" > "$STAMP"

echo "fetch_onnxruntime_linux: staged onnxruntime $VERSION — HTDemucs stem separation ENABLED."
