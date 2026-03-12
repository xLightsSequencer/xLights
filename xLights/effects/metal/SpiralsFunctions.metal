/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <metal_stdlib>
using namespace metal;

#include "MetalEffectDataTypes.h"

// Helper: convert HSV (h in [0,1], s in [0,1], v in [0,1]) to RGBA uchar4
static uchar4 HSVtoRGBA(float3 hsv, float alpha) {
    float h = hsv.x * 360.0f;
    float s = hsv.y;
    float v = hsv.z;

    float c = v * s;
    float x = c * (1.0f - fabs(fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    float r, g, b;
    if (h < 60.0f)       { r = c; g = x; b = 0; }
    else if (h < 120.0f) { r = x; g = c; b = 0; }
    else if (h < 180.0f) { r = 0; g = c; b = x; }
    else if (h < 240.0f) { r = 0; g = x; b = c; }
    else if (h < 300.0f) { r = x; g = 0; b = c; }
    else                 { r = c; g = 0; b = x; }
    return uchar4(
        (uchar)((r + m) * 255.0f),
        (uchar)((g + m) * 255.0f),
        (uchar)((b + m) * 255.0f),
        (uchar)(alpha * 255.0f)
    );
}

// Spirals effect — per-pixel kernel using inverted strand mapping.
//
// CPU algorithm writes:  x_dest = (strand + SpiralState/10 + y*Rotation/height) % width
//                        where strand = (strand_base + thick) % width
//
// GPU algorithm inverts: for pixel (px, py) compute
//                        x_src = (px - SpiralState/10 - py*Rotation/height) mod width
//                        then find which spiral owns x_src.
kernel void SpiralsEffect(constant MetalSpiralsData &data [[buffer(0)]],
                          device uchar4 *result            [[buffer(1)]],
                          uint index                       [[thread_position_in_grid]])
{
    uint px = index % data.width;
    uint py = index / data.width;

    if (px >= data.width || py >= data.height) return;

    // Invert the CPU mapping to find the "source x" that this pixel came from.
    // CPU:  x_dest = (strand + SpiralState/10 + y*Rotation/height) % width
    // So:   x_src  = x_dest - SpiralState/10 - y*Rotation/height
    float x_src_f = (float)px - data.spiralState / 10.0f - (float)py * data.rotation / (float)data.height;

    // Normalize to [0, width)
    float w = (float)data.width;
    x_src_f = x_src_f - floor(x_src_f / w) * w;   // true modulo, always >= 0

    // Check each spiral to see if this pixel belongs to it.
    // Spiral ns has: strand_base = (int)(ns * deltaStrands)
    //                thick range: [0, spiralThickness)
    //                so x range:  [(strand_base + 0) % width, ..., (strand_base + thick) % width]
    // Rather than iterating all strands, do it arithmetically:
    //   The spiral owns pixel if x_src_f falls in [strand_base, strand_base + spiralThickness) mod width

    int ownerSpiral = -1;
    float ownerThick = 0.0f;  // fractional thickness position within the spiral [0, spiralThickness)

    for (int ns = 0; ns < data.spiralCount; ns++) {
        float strand_base_f = (float)ns * data.deltaStrands;
        int   strand_base   = (int)strand_base_f;

        // Thickness offset from strand_base: how far is x_src from strand_base?
        float diff = x_src_f - (float)strand_base;
        // Normalize diff to [0, width)
        diff = diff - floor(diff / w) * w;

        if (diff < data.spiralThickness) {
            ownerSpiral = ns;
            ownerThick  = diff;
            break;
        }
    }

    if (ownerSpiral < 0) {
        // No spiral owns this pixel — leave it black/transparent
        result[index] = uchar4(0, 0, 0, 0);
        return;
    }

    int colorIdx = ownerSpiral % data.colorCount;
    float3 hsv = data.colorsAsHSV[colorIdx];
    uchar4 rgba = data.colorsAsRGBA[colorIdx];

    if (data.show3D) {
        // 3D shading: fade value based on thickness position
        // CPU: if Rotation < 0: f = (thick+1)/SpiralThickness
        //      else:             f = (SpiralThickness - thick) / SpiralThickness
        float f;
        if (data.rotation_sign < 0.0f) {
            f = (ownerThick + 1.0f) / data.spiralThickness;
        } else {
            f = (data.spiralThickness - ownerThick) / data.spiralThickness;
        }
        f = clamp(f, 0.0f, 1.0f);

        if (data.allowAlpha) {
            result[index] = uchar4(rgba.r, rgba.g, rgba.b, (uchar)(f * 255.0f));
        } else {
            float3 fadedHSV = float3(hsv.x, hsv.y, hsv.z * f);
            result[index] = HSVtoRGBA(fadedHSV, 1.0f);
        }
    } else {
        result[index] = uchar4(rgba.r, rgba.g, rgba.b, rgba.a);
    }
}
