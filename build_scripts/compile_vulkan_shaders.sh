#!/bin/bash
# Compile the Vulkan GLSL kernels to the SPIR-V headers the C++ #includes:
#   - compute kernels:  src-core/effects/vulkan/shaders/*.comp
#   - graphics shaders: src-core/graphics/vulkan/shaders/*.vert|*.frag
# The headers land under each dir's compiled/ and are GENERATED at build time
# (gitignored), not committed — glslc is a build tool here, exactly like ispc
# for the .ispc kernels.  The build (Makefile `vulkanshaders` step / CMake
# custom target) runs this before compiling the C++.  Diff-guarded so untouched
# kernels produce no churn (so an incremental rebuild doesn't needlessly
# recompile the TU that includes them).
#
# Requires glslc (Ubuntu/Debian: apt install glslc; or the Vulkan SDK).

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SHADER_DIR="$SCRIPT_DIR/../src-core/effects/vulkan/shaders"
GFX_SHADER_DIR="$SCRIPT_DIR/../src-core/graphics/vulkan/shaders"

# Honor a GLSLC override (CMake passes the find_program result); else PATH.
GLSLC="${GLSLC:-glslc}"
if ! command -v "$GLSLC" > /dev/null; then
    echo "ERROR: glslc not found — required to build the Vulkan backend." >&2
    echo "       Install it (Ubuntu/Debian: sudo apt-get install glslc) or the Vulkan SDK," >&2
    echo "       or configure with -DXLIGHTS_USE_VULKAN=OFF to build without GPU compute." >&2
    exit 1
fi

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
changed=0

compile_dir() {
    local dir="$1"
    shift
    local out_dir="$dir/compiled"
    mkdir -p "$out_dir"

    local src name base ext sym hdr
    for src in "$@"; do
        [ -e "$src" ] || continue
        name="$(basename "$src")"
        base="${name%.*}"
        ext="${name##*.}"
        # .comp keeps the historical Name.spv.h / Name_spv naming; stage
        # shaders encode the stage so name.vert and name.frag can coexist.
        if [ "$ext" = "comp" ]; then
            sym="${base}_spv"
            hdr="$base.spv.h"
        else
            sym="${base}_${ext}_spv"
            hdr="$base.$ext.spv.h"
        fi

        "$GLSLC" -O --target-env=vulkan1.1 -mfmt=c -I "$dir" -o "$TMP/inc" "$src"

        {
            echo "// Generated from $name by build_scripts/compile_vulkan_shaders.sh - do not edit"
            echo "static const uint32_t ${sym}[] ="
            cat "$TMP/inc"
            echo ";"
        } > "$TMP/$hdr"

        if ! cmp -s "$TMP/$hdr" "$out_dir/$hdr"; then
            mv "$TMP/$hdr" "$out_dir/$hdr"
            echo "updated: $hdr"
            changed=1
        fi
    done
}

compile_dir "$SHADER_DIR" "$SHADER_DIR"/*.comp
compile_dir "$GFX_SHADER_DIR" "$GFX_SHADER_DIR"/*.vert "$GFX_SHADER_DIR"/*.frag

if [ "$changed" -eq 0 ]; then
    echo "All Vulkan shader headers up to date."
fi
