#pragma once

#include <simd/simd.h>


struct ButterflyData
{
    simd::char4 colors[8];

    float offset;
    int chunks;
    int skip;

    uint16_t colorScheme;
    uint16_t width;
    uint16_t height;
    uint16_t numColors;

};
