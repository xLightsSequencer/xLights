#!/usr/bin/env bash
#
# fetch_klightmapper.sh — download the prebuilt KLightMapper desktop scan
# library (Map-from-Lights camera scan) and stage it where the Windows/Linux
# builds expect it. macOS gets KLightMapper through the Xcode project's Swift
# package (see macOS/.../Package.resolved), so this script is for Windows + Linux.
#
# The artifacts are GitHub release assets of github.com/KulpLights/KLightMapper.
# The pinned version lives in ci_scripts/klightmapper_version.txt and should
# track the macOS Swift-package pin so all three desktops use the same engine.
#
# This is best-effort: if the download fails (e.g. offline build) the script
# prints a notice and exits non-zero, but it stages nothing partial. The build
# systems treat a missing library as "camera scan disabled" — XLIGHTS_HAVE_KLIGHTMAPPER
# is only defined when the staged lib is present (CMake find_library, the VS
# project's Exists() check, the Linux klightmapper.mak wildcard).
#
# Usage:
#   ci_scripts/fetch_klightmapper.sh [platform] [--force]
#     platform : linux-x64 | linux-arm64 | windows-x64 (default: auto-detect)
#     --force  : re-download even if the library is already staged
#
set -u

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

VERSION="$(tr -d ' \t\r\n' < "$SCRIPT_DIR/klightmapper_version.txt")"
BASE_URL="https://github.com/KulpLights/KLightMapper/releases/download/${VERSION}"

PLATFORM=""
FORCE=0
for arg in "$@"; do
    case "$arg" in
        --force) FORCE=1 ;;
        linux-x64|linux-arm64|windows-x64) PLATFORM="$arg" ;;
        *) echo "fetch_klightmapper: ignoring unknown argument '$arg'" >&2 ;;
    esac
done

# Auto-detect platform when not given explicitly.
if [ -z "$PLATFORM" ]; then
    uname_s="$(uname -s 2>/dev/null || echo unknown)"
    uname_m="$(uname -m 2>/dev/null || echo unknown)"
    case "$uname_s" in
        Linux)
            case "$uname_m" in
                aarch64|arm64) PLATFORM="linux-arm64" ;;
                *)             PLATFORM="linux-x64" ;;
            esac
            ;;
        MINGW*|MSYS*|CYGWIN*) PLATFORM="windows-x64" ;;
        Darwin)
            echo "fetch_klightmapper: macOS uses the Xcode Swift package for KLightMapper — nothing to fetch."
            exit 0
            ;;
        *)
            echo "fetch_klightmapper: cannot auto-detect platform ($uname_s/$uname_m); pass linux-x64|linux-arm64|windows-x64" >&2
            exit 1
            ;;
    esac
fi

# Where each platform's pieces are staged (paths relative to the repo root).
case "$PLATFORM" in
    windows-x64)
        ASSET="klightmapper-windows-x64.zip"
        LIB_NAME="klightmapper.lib"; LIB_DIR="lib/windows64"
        DLL_NAME="klightmapper.dll"; DLL_DIR="bin64"
        SENTINEL="$ROOT_DIR/$LIB_DIR/$LIB_NAME"
        ;;
    linux-x64|linux-arm64)
        ASSET="klightmapper-${PLATFORM}.zip"
        LIB_NAME="libklightmapper.so"; LIB_DIR="lib/linux"
        DLL_NAME=""; DLL_DIR=""
        SENTINEL="$ROOT_DIR/$LIB_DIR/$LIB_NAME"
        ;;
esac
INC_DIR="include/klightmapper"

if [ "$FORCE" -eq 0 ] && [ -f "$SENTINEL" ]; then
    echo "fetch_klightmapper: $LIB_NAME already staged at $LIB_DIR — skipping (use --force to re-download)."
    exit 0
fi

echo "fetch_klightmapper: $PLATFORM $VERSION -> staging into $ROOT_DIR"

TMP_DIR="$(mktemp -d 2>/dev/null || mktemp -d -t klm)"
cleanup() { rm -rf "$TMP_DIR"; }
trap cleanup EXIT

ZIP="$TMP_DIR/$ASSET"
URL="$BASE_URL/$ASSET"
echo "fetch_klightmapper: downloading $URL"
if ! curl -fL --retry 3 -o "$ZIP" "$URL"; then
    echo "fetch_klightmapper: download failed ($URL) — camera scan will be disabled." >&2
    exit 1
fi

# Extract the archive without assuming a particular unzip tool is installed.
# The Linux build containers don't ship `unzip`, so fall back to bsdtar (which
# reads zip) and then python3's zipfile module before giving up.
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

if ! extract_zip "$ZIP" "$TMP_DIR/x"; then
    echo "fetch_klightmapper: could not extract $ASSET (no unzip/bsdtar/python3)." >&2
    exit 1
fi

# Stage the boundary headers (klm/api.h, klm/scan_api.h) -> include/klightmapper/klm/.
mkdir -p "$ROOT_DIR/$INC_DIR/klm"
cp -f "$TMP_DIR/x/klm/"*.h "$ROOT_DIR/$INC_DIR/klm/"

# Stage the link library.
mkdir -p "$ROOT_DIR/$LIB_DIR"
cp -f "$TMP_DIR/x/$LIB_NAME" "$ROOT_DIR/$LIB_DIR/$LIB_NAME"

# Windows also ships a runtime DLL that gets copied next to the exe / into the installer.
if [ -n "$DLL_NAME" ]; then
    mkdir -p "$ROOT_DIR/$DLL_DIR"
    cp -f "$TMP_DIR/x/$DLL_NAME" "$ROOT_DIR/$DLL_DIR/$DLL_NAME"
fi

echo "fetch_klightmapper: staged $LIB_NAME (+ headers${DLL_NAME:+ + $DLL_NAME}) — Map-from-Lights camera scan ENABLED."
