/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Hand-maintained C++ header for BarsFunctions.ispc.
// Struct layout must match the ISPC struct in BarsFunctions.ispc exactly.
// DO NOT add Metal-specific or platform-specific types here.

#pragma once
#include <stdint.h>

#ifdef __cplusplus
namespace ispc {
#endif // __cplusplus

#ifndef __ISPC_VECTOR_uint8_t4__
#define __ISPC_VECTOR_uint8_t4__
#ifdef _MSC_VER
__declspec(align(4)) struct uint8_t4 { uint8_t v[4]; };
#else
struct uint8_t4 { uint8_t v[4]; } __attribute__((aligned(4)));
#endif
#endif

#define MAX_ISPC_BARS_COLORS 8

// Mirror of the ISPC BarsData struct.
// HSV components are stored as three separate float arrays to avoid any
// alignment ambiguity from struct-of-struct layouts.
struct BarsData {
    uint32_t width;
    uint32_t height;
    int32_t  direction;     // 0-7, alternates remapped before calling
    int32_t  barSize;       // barHt (vertical) or barWi (horizontal), in pixels
    int32_t  blockSize;     // colorCount * barSize
    int32_t  f_offset;      // animation offset in pixels
    int32_t  newCenter;     // center pixel for expand/compress modes
    int32_t  colorCount;
    int32_t  highlight;
    int32_t  show3D;
    int32_t  gradient;
    int32_t  allowAlpha;
    int32_t  useFirstColorForHighlight;
    uint8_t4 colorsAsRGBA[MAX_ISPC_BARS_COLORS];
    float    colorsH[MAX_ISPC_BARS_COLORS];
    float    colorsS[MAX_ISPC_BARS_COLORS];
    float    colorsV[MAX_ISPC_BARS_COLORS];
    uint8_t4 highlightColor;
};

#if defined(__cplusplus) && (!defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C)
extern "C" {
#endif // __cplusplus
    // Render pixels [startIdx, endIdx) into result (the full pixel buffer).
    extern void BarsEffectISPC(const struct BarsData *data,
                               int32_t startIdx,
                               int32_t endIdx,
                               uint8_t4 *result);
#if defined(__cplusplus) && (!defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C)
} // extern "C"
#endif // __cplusplus

#ifdef __cplusplus
} // namespace ispc
#endif // __cplusplus
