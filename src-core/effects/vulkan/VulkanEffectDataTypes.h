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

#include <cstddef>
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

struct BoxBlurData {
    uint32_t width;
    uint32_t height;
    int32_t d; // window is [x-d, x+u] x [y-d, y+u]
    int32_t u;
};
static_assert(sizeof(BoxBlurData) == 16, "BoxBlurData layout drifted from the GLSL push-constant block");

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
    int32_t sparkleFrame;
    xlvk::uchar4 sparkleColor;

    uint32_t isChromaKey;
    int32_t chromaSensitivity;
    xlvk::uchar4 chromaColor;
};
static_assert(sizeof(LayerBlendingData) == 80, "LayerBlendingData layout drifted from the GLSL push-constant block");

// ---------------------------------------------------------------------------
// Per-effect parameter structs.  These exceed 128 bytes, so they are bound as
// std430 SSBOs (arena binding 5) rather than push constants.  Layouts mirror
// the corresponding MetalEffectDataTypes.h structs; xlvk::float3 (16-byte
// aligned) matches a std430 vec3 array (stride 16).

#define MAX_VULKAN_BARS_COLORS 8

// Bars direction, after alternates are remapped (custom/spatial → CPU).
#define BARS_DIR_UP        0
#define BARS_DIR_DOWN      1
#define BARS_DIR_EXPAND    2
#define BARS_DIR_COMPRESS  3
#define BARS_DIR_LEFT      4
#define BARS_DIR_RIGHT     5
#define BARS_DIR_HEXPAND   6
#define BARS_DIR_HCOMPRESS 7

struct VulkanBarsData {
    uint32_t width;
    uint32_t height;

    int32_t direction;
    int32_t barSize;
    int32_t blockSize;
    int32_t f_offset;
    int32_t newCenter;
    int32_t colorCount;
    int32_t highlight;
    int32_t show3D;
    int32_t gradient;
    int32_t allowAlpha;
    int32_t useFirstColorForHighlight;

    xlvk::uchar4 colorsAsRGBA[MAX_VULKAN_BARS_COLORS];
    xlvk::float3 colorsAsHSV[MAX_VULKAN_BARS_COLORS];
    xlvk::uchar4 highlightColor;
};
static_assert(offsetof(VulkanBarsData, colorsAsHSV) == 96, "VulkanBarsData HSV offset drifted from std430");
static_assert(offsetof(VulkanBarsData, highlightColor) == 224, "VulkanBarsData highlightColor offset drifted from std430");
static_assert(sizeof(VulkanBarsData) == 240, "VulkanBarsData size drifted from the GLSL std430 block");

// A lone (non-array) float3 member occupies 12 bytes in std430 but 16 in a
// C++ alignas(16) type; the GLSL side declares such members as vec4 (using
// only .xyz) so the layouts match.  Array-of-float3 uses stride 16 on both
// sides and needs no such adjustment.
struct VulkanColorWashData {
    uint32_t width;
    uint32_t height;
    xlvk::uchar4 color;
    xlvk::float3 colorHSV; // GLSL vec4
    int32_t horizFade;
    int32_t vertFade;
    int32_t reverseFades;
    int32_t shimmerBlack;
    int32_t allowAlpha;
};
static_assert(offsetof(VulkanColorWashData, colorHSV) == 16, "VulkanColorWashData HSV offset drifted from std430");
static_assert(offsetof(VulkanColorWashData, horizFade) == 32, "VulkanColorWashData horizFade offset drifted from std430");
static_assert(sizeof(VulkanColorWashData) == 64, "VulkanColorWashData size drifted from the GLSL std430 block");

struct VulkanShockwaveData {
    uint32_t width;
    uint32_t height;
    int32_t xc_adj;
    int32_t yc_adj;
    float radius1;
    float radius2;
    float radius_center;
    float half_width;
    xlvk::uchar4 color;
    xlvk::float3 colorHSV; // GLSL vec4
    int32_t blend;
    int32_t allowAlpha;
};
static_assert(offsetof(VulkanShockwaveData, colorHSV) == 48, "VulkanShockwaveData HSV offset drifted from std430");
static_assert(offsetof(VulkanShockwaveData, blend) == 64, "VulkanShockwaveData blend offset drifted from std430");
static_assert(sizeof(VulkanShockwaveData) == 80, "VulkanShockwaveData size drifted from the GLSL std430 block");

#define MAX_VULKAN_FAN_COLORS 8

struct VulkanFanData {
    uint32_t width;
    uint32_t height;

    int32_t  xc_adj;
    int32_t  yc_adj;

    float    radius1;
    float    radius2;
    float    max_radius;

    float    blade_div_angle;
    float    blade_width_angle;
    float    color_angle;
    float    element_angle;
    float    element_size;
    float    angle_offset;
    float    start_angle;
    float    blade_angle;

    int32_t  reverse_dir;
    int32_t  blend_edges;
    int32_t  allowAlpha;
    int32_t  num_colors;

    xlvk::uchar4 colorsAsRGBA[MAX_VULKAN_FAN_COLORS];
    xlvk::float3 colorsAsHSV[MAX_VULKAN_FAN_COLORS];
};
static_assert(offsetof(VulkanFanData, colorsAsRGBA) == 76, "VulkanFanData colorsAsRGBA offset drifted from std430");
static_assert(offsetof(VulkanFanData, colorsAsHSV) == 112, "VulkanFanData colorsAsHSV offset drifted from std430");
static_assert(sizeof(VulkanFanData) == 240, "VulkanFanData size drifted from the GLSL std430 block");

#define MAX_VULKAN_SPIRALS_COLORS 8

struct VulkanSpiralsData {
    uint32_t width;
    uint32_t height;

    int32_t  spiralCount;
    int32_t  colorCount;
    int32_t  paletteRepeat;

    float    deltaStrands;
    float    spiralThickness;
    float    spiralState;
    float    rotation;

    int32_t  show3D;
    int32_t  allowAlpha;
    float    rotation_sign;

    xlvk::uchar4 colorsAsRGBA[MAX_VULKAN_SPIRALS_COLORS];
    xlvk::float3 colorsAsHSV[MAX_VULKAN_SPIRALS_COLORS];
};
static_assert(offsetof(VulkanSpiralsData, colorsAsRGBA) == 48, "VulkanSpiralsData colorsAsRGBA offset drifted from std430");
static_assert(offsetof(VulkanSpiralsData, colorsAsHSV) == 80, "VulkanSpiralsData colorsAsHSV offset drifted from std430");
static_assert(sizeof(VulkanSpiralsData) == 208, "VulkanSpiralsData size drifted from the GLSL std430 block");

#define MAX_VULKAN_GALAXY_COLORS 8

// Mirrors MetalGalaxyData / ispc::GalaxyISPCData — the Galaxy "New Render
// Method" gather.  All-scalar struct (no uchar4/float3), so std430 layout
// matches the natural C++ layout byte-for-byte.
struct VulkanGalaxyData {
    uint32_t width;
    uint32_t height;
    float    pos_x;
    float    pos_y;
    float    radius1;
    float    radius2;
    float    width1;
    float    width2;
    float    revs;
    float    start_angle;
    int32_t  reverse_dir;
    int32_t  inward;
    int32_t  blend_edges;
    float    head_end_of_tail;
    float    tail_end_of_tail;
    float    color_length;
    int32_t  num_colors;
    float    palR[MAX_VULKAN_GALAXY_COLORS];
    float    palG[MAX_VULKAN_GALAXY_COLORS];
    float    palB[MAX_VULKAN_GALAXY_COLORS];
};
static_assert(offsetof(VulkanGalaxyData, palR) == 68, "VulkanGalaxyData palR offset drifted from std430");
static_assert(offsetof(VulkanGalaxyData, palG) == 100, "VulkanGalaxyData palG offset drifted from std430");
static_assert(offsetof(VulkanGalaxyData, palB) == 132, "VulkanGalaxyData palB offset drifted from std430");
static_assert(sizeof(VulkanGalaxyData) == 164, "VulkanGalaxyData size drifted from the GLSL std430 block");

#define MAX_VULKAN_CIRCLES_BALLS  20
#define MAX_VULKAN_CIRCLES_COLORS  8

#define CIRCLES_MODE_RADIAL        0
#define CIRCLES_MODE_RADIAL_3D     1
#define CIRCLES_MODE_METABALLS     2
#define CIRCLES_MODE_REGULAR       3
#define CIRCLES_MODE_FADING        4

struct VulkanCirclesBall {
    float x, y, radius;
    int32_t colorIdx;
};
static_assert(sizeof(VulkanCirclesBall) == 16, "VulkanCirclesBall must be 16 bytes (std430 array stride)");

struct VulkanCirclesData {
    uint32_t width;
    uint32_t height;

    int32_t mode;
    int32_t numBalls;
    int32_t colorCount;
    int32_t allowAlpha;
    int32_t wrap;

    int32_t cx, cy;
    int32_t barSize;
    int32_t blockSize;
    int32_t f_offset;
    int32_t maxRadius;
    int32_t number;
    int32_t effectState;

    VulkanCirclesBall balls[MAX_VULKAN_CIRCLES_BALLS];

    xlvk::uchar4 colorsAsRGBA[MAX_VULKAN_CIRCLES_COLORS];
    xlvk::float3 colorsAsHSV[MAX_VULKAN_CIRCLES_COLORS];
};
static_assert(offsetof(VulkanCirclesData, balls) == 60, "VulkanCirclesData balls offset drifted from std430");
static_assert(offsetof(VulkanCirclesData, colorsAsRGBA) == 380, "VulkanCirclesData colorsAsRGBA offset drifted from std430");
static_assert(offsetof(VulkanCirclesData, colorsAsHSV) == 416, "VulkanCirclesData colorsAsHSV offset drifted from std430");
static_assert(sizeof(VulkanCirclesData) == 544, "VulkanCirclesData size drifted from the GLSL std430 block");

#define MAX_VULKAN_PLASMA_COLORS 8

// Mirrors PlasmaData.  Metal compiles 5 PlasmaEffectStyleN kernels and picks
// functions[colorScheme]; the Vulkan port unifies all 5 color mappings into
// one PlasmaEffect.comp kernel selected by colorScheme (a uniform).
struct VulkanPlasmaData {
    uint32_t width;
    uint32_t height;

    float style;
    float lineDensity;
    float time;
    float sinTime5;
    float cosTime3;
    float sinTime2;

    int32_t colorScheme;
    int32_t numColors;
    xlvk::uchar4 colors[MAX_VULKAN_PLASMA_COLORS];
};
static_assert(offsetof(VulkanPlasmaData, colorScheme) == 32, "VulkanPlasmaData colorScheme offset drifted from std430");
static_assert(offsetof(VulkanPlasmaData, colors) == 40, "VulkanPlasmaData colors offset drifted from std430");
static_assert(sizeof(VulkanPlasmaData) == 72, "VulkanPlasmaData size drifted from the GLSL std430 block");

#define MAX_VULKAN_BUTTERFLY_COLORS 8

// Mirrors ButterflyData.  Metal compiles 6 kernels (ButterflyEffectStyle1..5 +
// ButterflyEffectPlasmaStyles) indexed by functions[style]; the Vulkan port
// unifies them into one ButterflyEffect.comp kernel with switch(style), where
// style is the same 1..10 selector Metal used.
struct VulkanButterflyData {
    uint32_t width;
    uint32_t height;

    float offset;
    int32_t chunks;
    int32_t skip;
    int32_t curState;

    int32_t colorScheme;
    int32_t numColors;
    int32_t style;

    float plasmaTime;

    xlvk::uchar4 colors[MAX_VULKAN_BUTTERFLY_COLORS];
};
static_assert(offsetof(VulkanButterflyData, colors) == 40, "VulkanButterflyData colors offset drifted from std430");
static_assert(sizeof(VulkanButterflyData) == 72, "VulkanButterflyData size drifted from the GLSL std430 block");

#define MAX_VULKAN_PINWHEEL_ARMS 16

// Mirrors MetalPinwheelData.  Only PinwheelEffectStyle0 is compiled on the
// Metal side and it already switches on pw3dType internally, so this is a
// direct 1:1 port.  Written by the RenderNewArms override (no settings
// parsing).  Partial-write (leaves the pre-cleared buffer outside the arms).
struct VulkanPinwheelData {
    uint32_t width;
    uint32_t height;

    int32_t pinwheel_arms;
    int32_t xc_adj;
    int32_t yc_adj;
    int32_t degrees_per_arm;
    int32_t pinwheel_twist;
    int32_t max_radius;
    int32_t poffset;
    int32_t pw3dType;
    int32_t pinwheel_rotation;

    float tmax;
    float pos;

    int32_t allowAlpha;
    int32_t numColors;

    xlvk::uchar4 colorsAsColor[MAX_VULKAN_PINWHEEL_ARMS];
    xlvk::float3 colorsAsHSV[MAX_VULKAN_PINWHEEL_ARMS];
};
static_assert(offsetof(VulkanPinwheelData, colorsAsColor) == 60, "VulkanPinwheelData colorsAsColor offset drifted from std430");
static_assert(offsetof(VulkanPinwheelData, colorsAsHSV) == 128, "VulkanPinwheelData colorsAsHSV offset drifted from std430");
static_assert(sizeof(VulkanPinwheelData) == 384, "VulkanPinwheelData size drifted from the GLSL std430 block");

#define VULKAN_KALEIDOSCOPE_KTYPE_SQUARE2  0
#define VULKAN_KALEIDOSCOPE_KTYPE_RADIAL   1
#define VULKAN_KALEIDOSCOPE_KTYPE_TRIANGLE 2

// A plain 2-float vertex (GLSL vec2) — std430 vec2-array stride is 8, matching
// this struct's natural 8-byte layout.
struct VulkanKaleidoscopeVertex {
    float x;
    float y;
};
static_assert(sizeof(VulkanKaleidoscopeVertex) == 8, "VulkanKaleidoscopeVertex must be 8 bytes (matches GLSL vec2)");

// Mirrors the 3-way Metal kernel choice (fnSquare2/fnRadial/fnTriangle).
// Source-reading (ADDENDUM 2): reads a source snapshot, partial-write.
struct VulkanKaleidoscopeData {
    uint32_t width;
    uint32_t height;

    float cx;
    float cy;
    float size;
    float rotRad;

    int32_t kType;
    int32_t maxIter;

    VulkanKaleidoscopeVertex v[3];
};
static_assert(offsetof(VulkanKaleidoscopeData, v) == 32, "VulkanKaleidoscopeData v offset drifted from std430");
static_assert(sizeof(VulkanKaleidoscopeData) == 56, "VulkanKaleidoscopeData size drifted from the GLSL std430 block");

// Mirrors WarpData plus a warpType selector (the port unifies Metal's 13
// WarpEffectXxx kernels into one WarpEffect.comp switched on warpType).
// Source-reading (ADDENDUM 2): every type samples the source snapshot.
// All-scalar, so std430 matches the natural C++ layout.
struct VulkanWarpData {
    uint32_t width;
    uint32_t height;

    uint32_t xPos;
    uint32_t yPos;

    float speed;
    float progress;
    float frequency;

    int32_t direction;
    int32_t warpType;
};
static_assert(sizeof(VulkanWarpData) == 36, "VulkanWarpData size drifted from the GLSL std430 block");

#define MAX_VULKAN_TREE_PPB 512

// Mirrors MetalTreeData.  Full-write kernel — every pixel is set.
struct VulkanTreeData {
    uint32_t width;
    uint32_t height;
    int32_t  ppb;         // pixels_per_branch
    int32_t  frame;       // light sweep limit (x <= frame)
    int32_t  branch_row;  // b — branch row currently being lit
    int32_t  f_mod;       // sweep position within the row
    int32_t  showlights;
    xlvk::uchar4 bgColors[MAX_VULKAN_TREE_PPB]; // indexed by mod-1, mod in 1..ppb
    xlvk::uchar4 lightColors[5];                // indexed by branch % 5
};
static_assert(offsetof(VulkanTreeData, bgColors) == 28, "VulkanTreeData bgColors offset drifted from std430");
static_assert(offsetof(VulkanTreeData, lightColors) == 2076, "VulkanTreeData lightColors offset drifted from std430");
static_assert(sizeof(VulkanTreeData) == 2096, "VulkanTreeData size drifted from the GLSL std430 block");

#define VULKAN_SHIMMER_MAX_LUT 1024
#define SHIMMER_LUT_FLAT   0
#define SHIMMER_LUT_X      1
#define SHIMMER_LUT_Y      2
#define SHIMMER_LUT_RANDOM 3

// Mirrors MetalShimmerData.  The Metal LUT (setBytes at index 2) rides inside
// the params SSBO here as a fixed array (larger spatial LUTs fall back to CPU).
// frameSeed is a 64-bit RNG seed split into two 32-bit halves (no native uint64
// in std430); the kernel emulates splitmix64 with umulExtended().
struct VulkanShimmerData {
    uint32_t width;
    uint32_t height;
    int32_t  lutMode;
    int32_t  colorCount;
    uint32_t frameSeedLo;
    uint32_t frameSeedHi;
    xlvk::uchar4 lut[VULKAN_SHIMMER_MAX_LUT];
};
static_assert(offsetof(VulkanShimmerData, lut) == 24, "VulkanShimmerData lut offset drifted from std430");
static_assert(sizeof(VulkanShimmerData) == 4120, "VulkanShimmerData size drifted from the GLSL std430 block");

// Mirrors MetalCandleData.  Stateful perNode flame sim: the wrapper uploads the
// CPU state cache, dispatches (read/write state + write result), commits, waits,
// and reads the state back — synchronously inside Render.
struct VulkanCandleData {
    uint32_t width;
    uint32_t height;
    uint32_t maxWid;      // stride of the state array rows
    uint32_t numStates;
    uint32_t frameSeedLo;
    uint32_t frameSeedHi;
    int32_t  windVariability;
    int32_t  flameAgility;
    int32_t  windCalmness;
    int32_t  windBaseline;
    int32_t  usePalette;
    xlvk::uchar4 c1;
    xlvk::uchar4 c2;
};
static_assert(offsetof(VulkanCandleData, c1) == 44, "VulkanCandleData c1 offset drifted from std430");
static_assert(offsetof(VulkanCandleData, c2) == 48, "VulkanCandleData c2 offset drifted from std430");
static_assert(sizeof(VulkanCandleData) == 52, "VulkanCandleData size drifted from the GLSL std430 block");

// GPU round-trip layout for CandleEffect.h's CandleState (5 packed uint8_t).
// Each field is widened to a full uint32 (no 8-bit GLSL storage; avoids
// cross-thread partial-word writes).  VulkanCandleEffect.cpp packs/unpacks.
struct VulkanCandleState {
    uint32_t flameprimer;
    uint32_t flamer;
    uint32_t wind;
    uint32_t flameprimeg;
    uint32_t flameg;
};
static_assert(sizeof(VulkanCandleState) == 20, "VulkanCandleState size drifted from the flat state-buffer layout in CandleEffect.comp");

// Mirrors MetalWaveData.  None-fill (constant-color) case only; the per-column
// band [y1,y2] is uploaded separately as the Cols SSBO (binding 1).
struct VulkanWaveData {
    uint32_t width;
    uint32_t height;
    int32_t  yoffset;
    int32_t  mirror;
    xlvk::uchar4 noneColor;
};
static_assert(offsetof(VulkanWaveData, noneColor) == 16, "VulkanWaveData noneColor offset drifted from std430");
static_assert(sizeof(VulkanWaveData) == 20, "VulkanWaveData size drifted from the GLSL std430 block");

// Mirrors MetalGarlandsData.  colors[] (binding 1, packed uchar4) and yb[]
// (binding 2, int32) are uploaded fresh every frame (buffMax scales with the
// buffer, so no fixed LUT fits here).
struct VulkanGarlandsData {
    uint32_t width;         // BufferWi
    uint32_t height;        // BufferHt
    int32_t  buffMax;       // ring count (BufferHt for Up/Down, BufferWi for Left/Right)
    int32_t  garlandType;   // 0..4
    int32_t  dir;           // 0..3 (Up/Down/Left/Right, after Up-then-Down folding)
    float    invPS;         // 1 / PixelSpacing
    float    posOffOverPS;  // positionOffset / PixelSpacing
};
static_assert(offsetof(VulkanGarlandsData, dir) == 16, "VulkanGarlandsData dir offset drifted from std430");
static_assert(offsetof(VulkanGarlandsData, invPS) == 20, "VulkanGarlandsData invPS offset drifted from std430");
static_assert(sizeof(VulkanGarlandsData) == 28, "VulkanGarlandsData size drifted from the GLSL std430 block");

// Mirrors MetalFillData.  Per-line color LUT (binding 1) and painted mask
// (binding 2) ride in real SSBOs uploaded via createSharedBuffer.
struct VulkanFillData {
    uint32_t width;
    uint32_t height;
    int32_t  vertical;  // 1 => line is the row (y) [Up/Down]; 0 => the column (x) [Left/Right]
};
static_assert(sizeof(VulkanFillData) == 12, "VulkanFillData size drifted from the GLSL std430 block");

// Mirrors MetalMeteorsData.  Axis-aligned gather only; the CPU meteor
// simulation stays shared (VulkanMeteorsEffect hooks GatherMeteors).  frameSeed
// (rainbow scheme RNG) split into two 32-bit halves (no native uint64).
struct VulkanMeteorsData {
    uint32_t width;
    uint32_t height;
    int32_t  mode;         // 0 = vertical, 1 = horizontal, 2 = icicle
    int32_t  direction;    // raw MeteorsEffect: 0 Down, 1 Up, 2 Left, 3 Right, 6/7 Icicle
    int32_t  tailLength;
    int32_t  colorScheme;  // 0 = rainbow, 1 = range, 2 = palette
    int32_t  allowAlpha;
    int32_t  numMeteors;
    int32_t  wantBkg;      // icicle: draw dim background icicles
    uint32_t frameSeedLo;
    uint32_t frameSeedHi;
};
static_assert(offsetof(VulkanMeteorsData, frameSeedLo) == 36, "VulkanMeteorsData frameSeedLo offset drifted from std430");
static_assert(sizeof(VulkanMeteorsData) == 44, "VulkanMeteorsData size drifted from the GLSL std430 block");

// Mirrors MetalMeteorParticle (24B).  Bound as a flat uint[] in MeteorsEffect.comp
// (an array-of-struct stride would round to 32 under std430 rule 9).
struct VulkanMeteorParticle {
    int32_t a;      // primary axis coord: column (vertical/icicle) or row (horizontal)
    int32_t base;   // meteor.y (vertical/icicle) or meteor.x (horizontal)
    int32_t h;      // icicle drip length; 0 otherwise
    float   hue;
    float   sat;
    float   val;
};
static_assert(sizeof(VulkanMeteorParticle) == 24, "VulkanMeteorParticle must be 24 bytes (matches the flat 6-word GLSL layout)");

// Mirrors MetalTwinkleData.  Stateful per-light strobe sim (StrobeClass state
// cache + CPU LUT uploaded, read/write state + scatter-write result, commit,
// wait, read state back — like VulkanCandleData).  Dispatched per-strobe.
struct VulkanTwinkleData {
    uint32_t width;         // BufferWi (pixel-index stride)
    uint32_t npix;          // pixel-write bound (GetPixelCount())
    uint32_t curNumStrobe;  // number of live strobe entries (grid bound)
    int32_t  max_modulo;
    int32_t  max_modulo2;
    int32_t  colorcnt;
    int32_t  lutStride;     // max_modulo + 1
    int32_t  lutSize;       // colorcnt * lutStride
    int32_t  new_algorithm;
    int32_t  reRandomize;
    uint32_t frameSeedLo;
    uint32_t frameSeedHi;
};
static_assert(offsetof(VulkanTwinkleData, curNumStrobe) == 8, "VulkanTwinkleData curNumStrobe offset drifted from std430");
static_assert(offsetof(VulkanTwinkleData, frameSeedLo) == 40, "VulkanTwinkleData frameSeedLo offset drifted from std430");
static_assert(sizeof(VulkanTwinkleData) == 48, "VulkanTwinkleData size drifted from the GLSL std430 block");

// Mirrors MetalLifeData.  Source-reading: prev[] (a snapshot of GetTempBuf())
// and palette[] uploaded fresh each frame; result is the shared pixel buffer.
// CPU reads result back to seed the next generation (commit+wait in Render).
struct VulkanLifeData {
    uint32_t width;
    uint32_t height;
    int32_t  npix;         // min(GetPixelCount(), width*height); wrapped neighbour
                           // index >= npix counts as black (scalar's bounds guard)
    int32_t  type;         // ruleset 0..4
    int32_t  numColors;    // palette.Size() (>= 1)
    uint32_t frameSeedLo;
    uint32_t frameSeedHi;
};
static_assert(offsetof(VulkanLifeData, frameSeedLo) == 20, "VulkanLifeData frameSeedLo offset drifted from std430");
static_assert(sizeof(VulkanLifeData) == 28, "VulkanLifeData size drifted from the GLSL std430 block");

#endif
