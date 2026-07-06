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

// Mirrors TransitionData in MetalEffectDataTypes.h ("out" is renamed —
// it is a reserved word in GLSL).
struct TransitionData {
    uint32_t width;
    uint32_t height;

    uint32_t pWidth;
    uint32_t pHeight;

    float adjust;
    float progress;

    uint32_t hasPrev;
    uint32_t reverse;
    uint32_t isOut;
};
static_assert(sizeof(TransitionData) == 36, "TransitionData layout drifted from the GLSL push-constant block");

// Mirrors LayerBlendingData in MetalEffectDataTypes.h with the portability
// rules applied (bool -> uint32_t, uchar4 -> packed uint via xlvk::uchar4).
struct LayerBlendingData {
    int32_t nodeCount;
    uint32_t bufferWi;
    uint32_t bufferHi;
    uint32_t useMask;

    float hueAdjust;
    float valueAdjust;
    float saturationAdjust;

    int32_t brightness;
    int32_t contrast;
    float fadeFactor;
    float effectMixThreshold;
    uint32_t effectMixVaries;
    uint32_t brightnessLevel;
    int32_t mixTypeData;

    int32_t outputSparkleCount;
    xlvk::uchar4 sparkleColor;

    uint32_t isChromaKey;
    int32_t chromaSensitivity;
    xlvk::uchar4 chromaColor;
};
static_assert(sizeof(LayerBlendingData) == 76, "LayerBlendingData layout drifted from the GLSL push-constant block");

#endif
