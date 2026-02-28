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

// ColorWash effect — per-pixel kernel.
// A single palette-blended base color is multiplied by per-pixel
// horizontal and/or vertical fade factors.
kernel void ColorWashEffect(constant MetalColorWashData &data [[buffer(0)]],
                            device uchar4 *result             [[buffer(1)]],
                            uint index                        [[thread_position_in_grid]])
{
    uint px = index % data.width;
    uint py = index / data.width;
    if (px >= data.width || py >= data.height) return;

    // Shimmer odd frame: leave pixel black/transparent
    if (data.shimmerBlack) {
        result[index] = uchar4(0, 0, 0, 0);
        return;
    }

    float halfWi = (float)(data.width  - 1) / 2.0f;
    float halfHt = (float)(data.height - 1) / 2.0f;

    float multH = 1.0f;
    float multV = 1.0f;

    if (data.horizFade) {
        float dist = fabs(halfWi - (float)px);
        multH = data.reverseFades ? (dist / halfWi) : (1.0f - dist / halfWi);
    }
    if (data.vertFade) {
        float dist = fabs(halfHt - (float)py);
        multV = data.reverseFades ? (dist / halfHt) : (1.0f - dist / halfHt);
    }

    if (data.allowAlpha) {
        float alpha = (float)data.color.a * multH * multV;
        result[index] = uchar4(data.color.r, data.color.g, data.color.b, (uchar)alpha);
    } else {
        float v = data.colorHSV.z * multH * multV;
        v = clamp(v, 0.0f, 1.0f);

        // HSV → RGB with the faded value
        float h = data.colorHSV.x;
        float s = data.colorHSV.y;
        float r, g, b;
        if (s == 0.0f) {
            r = g = b = v;
        } else {
            float hh = fmod(h, 1.0f) * 6.0f;
            int   i  = int(hh);
            float f  = hh - float(i);
            float p  = v * (1.0f - s);
            float q  = v * (1.0f - s * f);
            float t  = v * (1.0f - s * (1.0f - f));
            switch (i) {
                case 0: r = v; g = t; b = p; break;
                case 1: r = q; g = v; b = p; break;
                case 2: r = p; g = v; b = t; break;
                case 3: r = p; g = q; b = v; break;
                case 4: r = t; g = p; b = v; break;
                default: r = v; g = p; b = q; break;
            }
        }
        result[index] = uchar4((uchar)(r * 255.0f),
                               (uchar)(g * 255.0f),
                               (uchar)(b * 255.0f),
                               255);
    }
}
