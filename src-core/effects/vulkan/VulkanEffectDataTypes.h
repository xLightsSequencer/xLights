/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#pragma once

// Parameter structs shared between the C++ dispatch code and the GLSL
// kernels (push-constant blocks / std430 SSBOs).  These mirror the structs
// in MetalEffectDataTypes.h field for field; the .ispc.h headers remain the
// algorithmic reference.  Rules for keeping the layouts in sync:
//   - bool        -> uint32_t here, uint in GLSL (never Metal's 1-byte bool)
//   - uchar4      -> xlvk::uchar4 here, uint in GLSL (unpackPx/packPx)
//   - simd::float3-> xlvk::float3 (16-byte aligned) here, vec3 + pad in GLSL
//   - static_assert the size of every struct
#ifdef HAVE_VULKAN

#include <cstdint>

namespace xlvk {
struct uchar4 {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;
};
static_assert(sizeof(uchar4) == 4, "uchar4 must be 4 bytes");

struct alignas(16) float3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};
static_assert(sizeof(float3) == 16, "float3 must pad to 16 bytes (matches simd::float3 and std430 vec3+pad)");
} // namespace xlvk

struct RotoZoomData {
    uint32_t width;
    uint32_t height;

    float offset;
    float xrotation;
    int32_t xpivot;
    float yrotation;
    int32_t ypivot;

    float zrotation;
    float zoom;
    float zoomquality;
    int32_t pivotpointx;
    int32_t pivotpointy;
};
static_assert(sizeof(RotoZoomData) == 48, "RotoZoomData layout drifted from the GLSL push-constant block");

struct TentBlurData {
    uint32_t width;
    uint32_t height;
    int32_t halfK; // (kernelWidth - 1) / 2, tent weights halfK+1-|i|
};
static_assert(sizeof(TentBlurData) == 12, "TentBlurData layout drifted from the GLSL push-constant block");

#endif
