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
# If that download fails, the same version is taken from Microsoft's
# Microsoft.ML.OnnxRuntime package on nuget.org (the source the Windows fetch
# already uses): the GitHub repo went 404 for every release from 2026-11-17,
# which broke every Linux build. The pinned version lives in
# ci_scripts/onnxruntime_version.txt and is shared with the Windows fetch so
# both platforms run the same model/runtime pairing.
#
# If both downloads fail (e.g. offline build) the script prints a notice and
# exits non-zero, staging nothing partial. ONNX Runtime is a required
# dependency for the Linux build, so the Makefile stops rather than silently
# dropping stem separation.
#
# Usage:
#   ci_scripts/fetch_onnxruntime_linux.sh [platform] [--force]
#     platform : linux-x64 | linux-aarch64 (default: auto-detect)
#     --force  : re-download even if the library is already staged
#
set -u

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

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

# The build containers don't ship `unzip` (same fallbacks as fetch_klightmapper.sh).
extract_zip() {
    src="$1"; dest="$2"
    mkdir -p "$dest"
    if command -v unzip >/dev/null 2>&1; then
        unzip -oq "$src" -d "$dest" && return 0
    fi
    if command -v bsdtar >/dev/null 2>&1; then
        bsdtar -xf "$src" -C "$dest" && return 0
    fi
    if command -v python3 >/dev/null 2>&1; then
        python3 -m zipfile -e "$src" "$dest" && return 0
    fi
    return 1
}

# Prints the library's SONAME, or nothing if no ELF tool is available.
elf_soname() {
    if command -v readelf >/dev/null 2>&1; then
        readelf -d "$1" 2>/dev/null | sed -n 's/.*Library soname: \[\(.*\)\].*/\1/p'
    elif command -v objdump >/dev/null 2>&1; then
        objdump -p "$1" 2>/dev/null | awk '$1 == "SONAME" { print $2 }'
    fi
}

INC_SRC=""
LIB_SRC=""       # GitHub tarball: lib/ dir holding the .so plus its symlinks
NUGET_SO=""      # NuGet package: the bare libonnxruntime.so, no symlinks

TARBALL="$TMP_DIR/$ASSET"
. "$SCRIPT_DIR/gh_release_fetch.sh"
if gh_release_fetch microsoft/onnxruntime "v$VERSION" "$ASSET" "$TARBALL"; then
    if ! tar -xzf "$TARBALL" -C "$TMP_DIR"; then
        echo "fetch_onnxruntime_linux: could not extract $ASSET." >&2
        exit 1
    fi

    # The tarball expands to a single top-level onnxruntime-<platform>-<version>/ dir
    # containing include/ and lib/.
    EXTRACTED_DIR="$TMP_DIR/onnxruntime-${PLATFORM}-${VERSION}"
    if [ ! -d "$EXTRACTED_DIR" ]; then
        EXTRACTED_DIR="$(find "$TMP_DIR" -maxdepth 1 -type d -name 'onnxruntime-*' | head -n1)"
    fi
    if [ -z "$EXTRACTED_DIR" ] || [ ! -d "$EXTRACTED_DIR/include" ] || [ ! -d "$EXTRACTED_DIR/lib" ]; then
        echo "fetch_onnxruntime_linux: unexpected archive layout in $ASSET." >&2
        exit 1
    fi
    INC_SRC="$EXTRACTED_DIR/include"
    LIB_SRC="$EXTRACTED_DIR/lib"
else
    case "$PLATFORM" in
        linux-aarch64) NUGET_RID="linux-arm64" ;;
        *)             NUGET_RID="linux-x64" ;;
    esac
    NUGET_URL="https://api.nuget.org/v3-flatcontainer/microsoft.ml.onnxruntime/${VERSION}/microsoft.ml.onnxruntime.${VERSION}.nupkg"
    NUPKG="$TMP_DIR/onnxruntime.nupkg"
    NUGET_DIR="$TMP_DIR/nuget"

    echo "fetch_onnxruntime_linux: GitHub release download failed, trying NuGet: $NUGET_URL" >&2
    if ! curl -fL --retry 4 --retry-delay 5 --connect-timeout 30 -o "$NUPKG" "$NUGET_URL"; then
        echo "fetch_onnxruntime_linux: download failed ($URL and $NUGET_URL) — stem separation will be disabled." >&2
        exit 1
    fi
    if ! extract_zip "$NUPKG" "$NUGET_DIR"; then
        echo "fetch_onnxruntime_linux: could not extract the NuGet package (no unzip/bsdtar/python3?)." >&2
        exit 1
    fi
    INC_SRC="$NUGET_DIR/build/native/include"
    NUGET_SO="$NUGET_DIR/runtimes/$NUGET_RID/native/libonnxruntime.so"
    if [ ! -f "$INC_SRC/onnxruntime_c_api.h" ] || [ ! -f "$NUGET_SO" ]; then
        echo "fetch_onnxruntime_linux: unexpected layout in the NuGet package (no $NUGET_RID runtime)." >&2
        exit 1
    fi
fi

mkdir -p "$ROOT_DIR/$INC_DIR"
cp -f "$INC_SRC/"*.h "$ROOT_DIR/$INC_DIR/"

mkdir -p "$ROOT_DIR/$LIB_DIR"
rm -f "$ROOT_DIR/$LIB_DIR"/libonnxruntime.so*
if [ -n "$LIB_SRC" ]; then
    cp -fP "$LIB_SRC/"libonnxruntime.so* "$ROOT_DIR/$LIB_DIR/"
else
    # The NuGet copy is one unversioned file, but its SONAME is versioned
    # (libonnxruntime.so.1) and that is the name the linked binary asks for at
    # run time. Recreate the tarball's layout: real file + SONAME + dev symlinks.
    SONAME="$(elf_soname "$NUGET_SO")"
    [ -n "$SONAME" ] || SONAME="libonnxruntime.so.1"
    REAL="libonnxruntime.so.$VERSION"
    cp -f "$NUGET_SO" "$ROOT_DIR/$LIB_DIR/$REAL"
    [ "$SONAME" != "$REAL" ] && [ "$SONAME" != "libonnxruntime.so" ] && ln -sf "$REAL" "$ROOT_DIR/$LIB_DIR/$SONAME"
    ln -sf "$REAL" "$ROOT_DIR/$LIB_DIR/libonnxruntime.so"
fi

# Written last, so an interrupted run leaves no stamp and the next one re-fetches.
printf '%s\n' "$VERSION" > "$STAMP"

echo "fetch_onnxruntime_linux: staged onnxruntime $VERSION — HTDemucs stem separation ENABLED."
