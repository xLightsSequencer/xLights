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
    simd::uchar4 colorsAsColor[8];
    simd::float3 colorsAsHSV[8];
    uint16_t numColors;
};
