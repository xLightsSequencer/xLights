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

// Blend between two RGBA colors by fraction t in [0,1]
static uchar4 blendRGBA(uchar4 c1, uchar4 c2, float t) {
    return uchar4(
        (uchar)((float)c1.r + t * ((float)c2.r - (float)c1.r)),
        (uchar)((float)c1.g + t * ((float)c2.g - (float)c1.g)),
        (uchar)((float)c1.b + t * ((float)c2.b - (float)c1.b)),
        (uchar)((float)c1.a + t * ((float)c2.a - (float)c1.a))
    );
}

// HSV → RGB, returns uchar4 with alpha=255
static uchar4 barsHsv2rgba(float3 hsv) {
    float h = hsv.x, s = hsv.y, v = hsv.z;
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
    return uchar4((uchar)(r * 255.0f), (uchar)(g * 255.0f), (uchar)(b * 255.0f), 255);
}

// Blend two HSV colors and return RGBA
static uchar4 blendHSV(float3 hsv1, float3 hsv2, float t) {
    // Blend hue along the shortest arc
    float dh = hsv2.x - hsv1.x;
    if (dh > 0.5f) dh -= 1.0f;
    if (dh < -0.5f) dh += 1.0f;
    float3 blended = float3(
        fmod(hsv1.x + t * dh + 1.0f, 1.0f),
        hsv1.y + t * (hsv2.y - hsv1.y),
        hsv1.z + t * (hsv2.z - hsv1.z)
    );
    return barsHsv2rgba(blended);
}

// Compute final pixel color given bar-index n, barSize, colorCount, and modifier flags.
// n = BufferDim + coord + f_offset  (the raw bar position counter used by CPU)
static uchar4 computeBarColor(int n, constant MetalBarsData &data) {
    int absN   = abs(n % data.blockSize);
    int colorIdx = absN / data.barSize;
    int color2   = (colorIdx + 1) % data.colorCount;
    float pct    = (float)abs(n % data.barSize) / (float)data.barSize;

    // Highlight: boundary of bar → use highlight color
    if (data.highlight && (n % data.barSize) == 0) {
        return data.highlightColor;
    }

    uchar4 rgba;
    if (data.allowAlpha) {
        rgba = data.colorsAsRGBA[colorIdx];
        if (data.gradient) {
            rgba = blendRGBA(rgba, data.colorsAsRGBA[color2], pct);
        }
        if (data.show3D) {
            int numerator = data.barSize - abs(n % data.barSize) - 1;
            rgba.a = (uchar)(255.0f * (float)numerator / (float)data.barSize);
        }
    } else {
        float3 hsv = data.colorsAsHSV[colorIdx];
        if (data.gradient) {
            return blendHSV(hsv, data.colorsAsHSV[color2], pct);
        }
        if (data.show3D) {
            int numerator = data.barSize - abs(n % data.barSize) - 1;
            hsv.z *= (float)numerator / (float)data.barSize;
        }
        rgba = barsHsv2rgba(hsv);
    }
    return rgba;
}

// Bars effect — per-pixel kernel.
// All direction modes (0-7) handled; custom (12,13) and spatial fall back to CPU.
kernel void BarsEffect(constant MetalBarsData &data [[buffer(0)]],
                       device uchar4 *result         [[buffer(1)]],
                       uint index                    [[thread_position_in_grid]])
{
    uint px = index % data.width;
    uint py = index / data.width;
    if (px >= data.width || py >= data.height) return;

    int dir = data.direction;
    bool isVertical = (dir <= BARS_DIR_COMPRESS);

    int n;

    if (isVertical) {
        // Vertical modes: bars run horizontally, animate along Y.
        // CPU outer loop: y from -2*H to 2*H, writes to mapped output row.
        // We invert: given output py, find the loop y that wrote to it.
        int y;
        switch (dir) {
            case BARS_DIR_DOWN:
                // writes to y  →  loop_y = py
                y = (int)py;
                break;
            case BARS_DIR_EXPAND:
                // writes to y (y <= newCenter) and to newCenter+(newCenter-y) (y <= newCenter)
                // output py: if py <= newCenter → loop_y = py
                //            if py >  newCenter → loop_y = 2*newCenter - py
                y = ((int)py <= data.newCenter) ? (int)py : (2 * data.newCenter - (int)py);
                break;
            case BARS_DIR_COMPRESS:
                // writes to y (y >= newCenter) and to newCenter+(newCenter-y) (y >= newCenter)
                // output py: if py >= newCenter → loop_y = py
                //            if py <  newCenter → loop_y = 2*newCenter - py
                y = ((int)py >= data.newCenter) ? (int)py : (2 * data.newCenter - (int)py);
                break;
            default: // BARS_DIR_UP (0)
                // writes to BufferHt - y - 1  →  loop_y = BufferHt - py - 1
                y = (int)data.height - (int)py - 1;
                break;
        }
        n = (int)data.height + y + data.f_offset;
    } else {
        // Horizontal modes: bars run vertically, animate along X.
        int x;
        switch (dir) {
            case BARS_DIR_RIGHT:
                // writes to BufferWi - x - 1  →  loop_x = BufferWi - px - 1
                x = (int)data.width - (int)px - 1;
                break;
            case BARS_DIR_HEXPAND:
                // writes to x (x <= newCenter) and newCenter+(newCenter-x)
                x = ((int)px <= data.newCenter) ? (int)px : (2 * data.newCenter - (int)px);
                break;
            case BARS_DIR_HCOMPRESS:
                x = ((int)px >= data.newCenter) ? (int)px : (2 * data.newCenter - (int)px);
                break;
            default: // BARS_DIR_LEFT (4)
                x = (int)px;
                break;
        }
        n = (int)data.width + x + data.f_offset;
    }

    result[index] = computeBarColor(n, data);
}
