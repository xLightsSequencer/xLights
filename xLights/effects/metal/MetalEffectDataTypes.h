#pragma once

#include <simd/simd.h>


struct ButterflyData
{
    simd::uchar4 colors[8];

    float offset;
    int chunks;
    int skip;
    int curState;

    uint16_t colorScheme;
    uint16_t width;
    uint16_t height;
    uint16_t numColors;

    float plasmaTime;
    int plasmaStyle;
};


struct WarpData {
    uint32_t width;
    uint32_t height;
    
    uint32_t xPos;
    uint32_t yPos;
    
    float speed;
    float progress;
    float frequency;
    
    int direction;
};

struct PlasmaData
{
    float Style;
    float state;
    float Line_Density;
    float time;
    float sin_time_5;
    float cos_time_3;
    float sin_time_2;
    uint32_t width;
    uint32_t height;

    simd::uchar4 colors[8];
    uint16_t numColors;
};

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

// allow up to 16 arms, more than that and drop to CPU render
#define MAX_METAL_PINWHEEL_ARMS 16
struct MetalPinwheelData {
    uint32_t width;
    uint32_t height;
    
    int32_t pinwheel_arms;
    int32_t xc_adj = 0;
    int32_t yc_adj = 0;
    int32_t degrees_per_arm;
    int32_t pinwheel_twist;
    int32_t max_radius;
    int32_t poffset;
    int32_t pw3dType;
    int32_t pinwheel_rotation;

    float tmax;
    float pos;
    
    int32_t allowAlpha;
    simd::uchar4 colorsAsColor[MAX_METAL_PINWHEEL_ARMS];
    simd::float3 colorsAsHSV[MAX_METAL_PINWHEEL_ARMS];
    uint16_t numColors;
};


struct MetalShockwaveData {
    uint32_t width;
    uint32_t height;
    
    int32_t xc_adj = 0;
    int32_t yc_adj = 0;
    
    float radius1;
    float radius2;
    float radius_center;
    float half_width;
    
    simd::uchar4 color;
    simd::float3 colorHSV;
    uint16_t blend;
    uint16_t allowAlpha;
};

struct TransitionData {
    uint32_t width;
    uint32_t height;

    uint32_t pWidth;
    uint32_t pHeight;

    float adjust;
    float progress;
    
    bool hasPrev;
    bool reverse;
    bool out;
};

// Fan effect — max 8 palette colors (no spatial support on GPU)
#define MAX_METAL_FAN_COLORS 8

struct MetalFanData {
    uint32_t width;
    uint32_t height;

    int32_t  xc_adj;        // center offset from middle (pixels)
    int32_t  yc_adj;

    float    radius1;       // effective inner radius (after ramp-up/down applied)
    float    radius2;       // effective outer radius
    float    max_radius;    // max(start_radius, end_radius) — used for twist calc

    float    blade_div_angle;   // 360 / num_blades
    float    blade_width_angle; // blade_div_angle * blade_width%
    float    color_angle;       // blade_width_angle / num_colors
    float    element_angle;     // color_angle / num_elements
    float    element_size;      // element_angle * element_width%
    float    angle_offset;      // eff_pos_adj * revolutions
    float    start_angle;       // start_angle parameter (degrees)
    float    blade_angle;       // twist angle (degrees)

    int32_t  reverse_dir;
    int32_t  blend_edges;
    int32_t  allowAlpha;
    int32_t  num_colors;

    simd::uchar4  colorsAsRGBA[MAX_METAL_FAN_COLORS];
    simd::float3  colorsAsHSV[MAX_METAL_FAN_COLORS];   // (h, s, v)
};

// Kaleidoscope effect type enum (must match KaleidoscopeStyleType in MetalKaleidoscopeEffect.mm)
#define KALEIDOSCOPE_STYLE_SQUARE2   0
#define KALEIDOSCOPE_STYLE_6FOLD     1
#define KALEIDOSCOPE_STYLE_8FOLD     2
#define KALEIDOSCOPE_STYLE_12FOLD    3
#define KALEIDOSCOPE_STYLE_RADIAL    4

struct KaleidoscopeTriangleVertex {
    float x;
    float y;
};

struct KaleidoscopeData {
    uint32_t width;
    uint32_t height;

    float cx;
    float cy;
    float size;       // halfSize for Square2, size for triangle types, segments for Radial
    float rotRad;

    int32_t style;    // one of KALEIDOSCOPE_STYLE_* above
    int32_t maxIter;  // max iterations for triangle types

    // Triangle vertices (used for 6-fold, 8-fold, 12-fold)
    KaleidoscopeTriangleVertex v[3];
};

// ColorWash effect
struct MetalColorWashData {
    uint32_t width;
    uint32_t height;

    simd::uchar4 color;     // base blended color (RGBA)
    simd::float3 colorHSV;  // base color as HSV (h,s,v in [0,1])

    int32_t  horizFade;
    int32_t  vertFade;
    int32_t  reverseFades;
    int32_t  shimmerBlack;   // 1 = shimmer odd frame → render black (skip draw)
    int32_t  allowAlpha;
};

// Bars effect — max 8 palette colors (no spatial/custom-dir support on GPU)
#define MAX_METAL_BARS_COLORS 8

// direction constants (must match GetDirection() in BarsEffect.cpp)
#define BARS_DIR_UP           0
#define BARS_DIR_DOWN         1
#define BARS_DIR_EXPAND       2
#define BARS_DIR_COMPRESS     3
#define BARS_DIR_LEFT         4
#define BARS_DIR_RIGHT        5
#define BARS_DIR_HEXPAND      6
#define BARS_DIR_HCOMPRESS    7
// Alternate Up/Down map to 0/1 after f_offset adjustment; Alternate Left/Right to 4/5

struct MetalBarsData {
    uint32_t width;
    uint32_t height;

    int32_t  direction;      // 0-7 after remapping alternates; custom falls back to CPU
    int32_t  barSize;        // barHt (vertical) or barWi (horizontal) in pixels
    int32_t  blockSize;      // colorcnt * barSize
    int32_t  f_offset;       // animation offset in pixels
    int32_t  newCenter;      // center pixel for expand/compress modes
    int32_t  colorCount;     // actual number of palette colors used
    int32_t  highlight;
    int32_t  show3D;
    int32_t  gradient;
    int32_t  allowAlpha;
    int32_t  useFirstColorForHighlight;

    simd::uchar4  colorsAsRGBA[MAX_METAL_BARS_COLORS];
    simd::float3  colorsAsHSV[MAX_METAL_BARS_COLORS];
    simd::uchar4  highlightColor;   // white or first palette color
};

// Circles effect
#define MAX_METAL_CIRCLES_BALLS   20
#define MAX_METAL_CIRCLES_COLORS   8

// mode constants
#define CIRCLES_MODE_RADIAL        0
#define CIRCLES_MODE_RADIAL_3D     1
#define CIRCLES_MODE_METABALLS     2
#define CIRCLES_MODE_REGULAR       3   // solid filled, last-writer wins
#define CIRCLES_MODE_FADING        4   // fading filled disc

struct MetalCirclesBall {
    float x, y, radius;
    int   colorIdx;
};

struct MetalCirclesData {
    uint32_t width;
    uint32_t height;

    int32_t  mode;         // CIRCLES_MODE_*
    int32_t  numBalls;
    int32_t  colorCount;
    int32_t  allowAlpha;

    // Radial / Radial3D params
    int32_t  cx, cy;       // center pixel
    int32_t  barSize;      // barht
    int32_t  blockSize;    // colorCnt * barht
    int32_t  f_offset;     // animation offset
    int32_t  maxRadius;
    int32_t  number;       // used for radial_3D hue cycling
    int32_t  effectState;  // used for radial_3D hue

    MetalCirclesBall balls[MAX_METAL_CIRCLES_BALLS];

    simd::uchar4 colorsAsRGBA[MAX_METAL_CIRCLES_COLORS];
    simd::float3 colorsAsHSV[MAX_METAL_CIRCLES_COLORS];
};

// Spirals effect — max 8 palette colors (no spatial/blend support on GPU)
#define MAX_METAL_SPIRALS_COLORS 8

struct MetalSpiralsData {
    uint32_t width;
    uint32_t height;

    // Per-spiral strand parameters
    int32_t  spiralCount;       // total number of spirals = colorcnt * PaletteRepeat
    int32_t  colorCount;        // number of palette colors
    int32_t  paletteRepeat;     // PaletteRepeat

    float    deltaStrands;      // width / spiralCount
    float    spiralThickness;   // width of each spiral band (in x units)
    float    spiralState;       // SpiralState = position * BufferWi * 10 * Direction  (x10 units)
    float    rotation;          // pixels of x-shift per BufferHt rows

    int32_t  show3D;
    int32_t  allowAlpha;
    float    rotation_sign;     // +1 if rotation >= 0, -1 otherwise (for 3D direction)

    simd::uchar4  colorsAsRGBA[MAX_METAL_SPIRALS_COLORS];
    simd::float3  colorsAsHSV[MAX_METAL_SPIRALS_COLORS];
};

struct LayerBlendingData {
    int32_t nodeCount;
    uint32_t bufferWi;
    uint32_t bufferHi;
    
    bool useMask;
    
    float hueAdjust;
    float valueAdjust;
    float saturationAdjust;
    
    int brightness;
    int contrast;
    float fadeFactor;
    float effectMixThreshold;
    bool effectMixVaries;
    bool brightnessLevel;
    int mixTypeData;
    
    int outputSparkleCount;
    simd::uchar4 sparkleColor;
    
    bool isChromaKey;
    int chromaSensitivity;
    simd::uchar4 chromaColor;
};
