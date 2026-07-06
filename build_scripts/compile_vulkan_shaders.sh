#!/bin/bash
# Compile the Vulkan GLSL compute kernels (src-core/effects/vulkan/shaders/*.comp)
# to the SPIR-V headers under shaders/compiled/ that the vulkan/ C++ #includes.
# These headers are GENERATED at build time (gitignored), not committed — glslc
# is a build tool here, exactly like ispc for the .ispc kernels.  The build
# (Makefile `vulkanshaders` step / CMake custom target) runs this before
# compiling the C++.  Diff-guarded so untouched kernels produce no churn (so an
# incremental rebuild doesn't needlessly recompile the TU that includes them).
#
# Requires glslc (Ubuntu/Debian: apt install glslc; or the Vulkan SDK).

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SHADER_DIR="$SCRIPT_DIR/../src-core/effects/vulkan/shaders"
OUT_DIR="$SHADER_DIR/compiled"

# Honor a GLSLC override (CMake passes the find_program result); else PATH.
GLSLC="${GLSLC:-glslc}"
if ! command -v "$GLSLC" > /dev/null; then
    echo "ERROR: glslc not found — required to build the Vulkan compute backend." >&2
    echo "       Install it (Ubuntu/Debian: sudo apt-get install glslc) or the Vulkan SDK," >&2
    echo "       or configure with -DXLIGHTS_USE_VULKAN=OFF to build without GPU compute." >&2
    exit 1
fi

mkdir -p "$OUT_DIR"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

changed=0
for comp in "$SHADER_DIR"/*.comp; do
    name="$(basename "$comp" .comp)"
    out="$OUT_DIR/$name.spv.h"

    "$GLSLC" -O --target-env=vulkan1.1 -mfmt=c -I "$SHADER_DIR" -o "$TMP/$name.inc" "$comp"

    {
        echo "// Generated from $name.comp by build_scripts/compile_vulkan_shaders.sh - do not edit"
        echo "static const uint32_t ${name}_spv[] ="
        cat "$TMP/$name.inc"
        echo ";"
    } > "$TMP/$name.spv.h"

    if ! cmp -s "$TMP/$name.spv.h" "$out"; then
        mv "$TMP/$name.spv.h" "$out"
        echo "updated: $name.spv.h"
        changed=1
    fi
done

if [ "$changed" -eq 0 ]; then
    echo "All Vulkan shader headers up to date."
fi
