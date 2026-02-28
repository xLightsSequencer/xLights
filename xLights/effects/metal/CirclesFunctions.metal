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

// HSV → RGBA uchar4
static uchar4 circHsv2rgba(float h, float s, float v) {
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

// Additive HSV blend (matches RenderBuffer::Get2ColorAdditive)
static float3 addHSV(float3 a, float3 b) {
    // Convert both to RGB, add, convert back
    float4 ca, cb;
    {
        float h = a.x, s = a.y, v = a.z;
        float r, g, bl;
        if (s == 0.0f) { r = g = bl = v; }
        else {
            float hh = fmod(h, 1.0f) * 6.0f;
            int i = int(hh); float f = hh - float(i);
            float p = v*(1-s), q = v*(1-s*f), t = v*(1-s*(1-f));
            switch(i){case 0:r=v;g=t;bl=p;break;case 1:r=q;g=v;bl=p;break;
                       case 2:r=p;g=v;bl=t;break;case 3:r=p;g=q;bl=v;break;
                       case 4:r=t;g=p;bl=v;break;default:r=v;g=p;bl=q;break;}
        }
        ca = float4(r,g,bl,0);
    }
    {
        float h = b.x, s = b.y, v = b.z;
        float r, g, bl;
        if (s == 0.0f) { r = g = bl = v; }
        else {
            float hh = fmod(h, 1.0f) * 6.0f;
            int i = int(hh); float f = hh - float(i);
            float p = v*(1-s), q = v*(1-s*f), t = v*(1-s*(1-f));
            switch(i){case 0:r=v;g=t;bl=p;break;case 1:r=q;g=v;bl=p;break;
                       case 2:r=p;g=v;bl=t;break;case 3:r=p;g=q;bl=v;break;
                       case 4:r=t;g=p;bl=v;break;default:r=v;g=p;bl=q;break;}
        }
        cb = float4(r,g,bl,0);
    }
    // Add and clamp
    float3 sum = clamp(float3(ca.x+cb.x, ca.y+cb.y, ca.z+cb.z), 0.0f, 1.0f);
    // RGB → HSV
    float maxC = max(max(sum.x, sum.y), sum.z);
    float minC = min(min(sum.x, sum.y), sum.z);
    float delta = maxC - minC;
    float h = 0, s = 0, v = maxC;
    if (delta > 0.0001f) {
        s = delta / maxC;
        if (maxC == sum.x) h = fmod((sum.y - sum.z) / delta, 6.0f);
        else if (maxC == sum.y) h = (sum.z - sum.x) / delta + 2.0f;
        else h = (sum.x - sum.y) / delta + 4.0f;
        h /= 6.0f;
        if (h < 0) h += 1.0f;
    }
    return float3(h, s, v);
}

kernel void CirclesEffect(constant MetalCirclesData &data [[buffer(0)]],
                          device uchar4 *result            [[buffer(1)]],
                          uint index                       [[thread_position_in_grid]])
{
    uint px = index % data.width;
    uint py = index / data.width;
    if (px >= data.width || py >= data.height) return;

    int mode = data.mode;

    // ── RADIAL / RADIAL_3D ──────────────────────────────────────────────────
    if (mode == CIRCLES_MODE_RADIAL || mode == CIRCLES_MODE_RADIAL_3D) {
        float dx = (float)px - (float)data.cx;
        float dy = (float)py - (float)data.cy;
        float dist = sqrt(dx * dx + dy * dy);
        int   ii   = (int)dist;

        if (ii > data.maxRadius) {
            result[index] = uchar4(0, 0, 0, 0);
            return;
        }

        int n = ii - data.f_offset + data.blockSize;
        int colorIdx = ((n % data.blockSize) + data.blockSize) % data.blockSize / data.barSize;
        colorIdx = clamp(colorIdx, 0, data.colorCount - 1);

        if (mode == CIRCLES_MODE_RADIAL_3D) {
            float hue = (float)(ii + data.effectState) / ((float)data.maxRadius / (float)data.number);
            hue = hue - floor(hue);
            result[index] = circHsv2rgba(hue, 1.0f, 1.0f);
        } else {
            float3 hsv = data.colorsAsHSV[colorIdx];
            result[index] = circHsv2rgba(hsv.x, hsv.y, hsv.z);
        }
        return;
    }

    // ── META BALLS (PLASMA) ──────────────────────────────────────────────────
    if (mode == CIRCLES_MODE_METABALLS) {
        float sum = 0.0f;
        float3 hsv = float3(0, 0, 0);

        for (int ii = 0; ii < data.numBalls; ii++) {
            float dx = (float)px - data.balls[ii].x;
            float dy = (float)py - data.balls[ii].y;
            float d  = sqrt(dx * dx + dy * dy);
            float val = (d > 0.0001f) ? (data.balls[ii].radius / d) : 1.0f;
            sum += val;
            if (val > 0.30f) {
                int cidx = clamp(data.balls[ii].colorIdx, 0, data.colorCount - 1);
                float3 temp = data.colorsAsHSV[cidx];
                temp.z = val > 1.0f ? 1.0f : val;
                hsv = addHSV(hsv, temp);
            }
        }
        if (sum >= 0.90f) {
            result[index] = circHsv2rgba(hsv.x, hsv.y, hsv.z);
        } else {
            result[index] = uchar4(0, 0, 0, 0);
        }
        return;
    }

    // ── REGULAR / FADING FILLED CIRCLES ─────────────────────────────────────
    // Multiple balls: last one (highest index) that covers this pixel wins.
    // Matches CPU DrawCircle/DrawFadingCircle loop order (ii=0..numBalls-1, later overwrites earlier).
    uchar4 outColor = uchar4(0, 0, 0, 0);
    bool   hit      = false;

    for (int ii = 0; ii < data.numBalls; ii++) {
        float dx = (float)px - data.balls[ii].x;
        float dy = (float)py - data.balls[ii].y;
        float d  = sqrt(dx * dx + dy * dy);
        float r  = data.balls[ii].radius;
        if (d > r) continue;

        int cidx = clamp(data.balls[ii].colorIdx, 0, data.colorCount - 1);

        if (mode == CIRCLES_MODE_FADING) {
            // DrawFadingCircle: brightness = full_brightness * (1 - d/r)
            float3 hsv = data.colorsAsHSV[cidx];
            float alpha = hsv.z * (1.0f - d / r);
            if (alpha <= 0.0f) continue;
            if (data.allowAlpha) {
                uchar4 base = data.colorsAsRGBA[cidx];
                float a = (float)base.a * (1.0f - d / r);
                outColor = uchar4(base.r, base.g, base.b, (uchar)a);
            } else {
                outColor = circHsv2rgba(hsv.x, hsv.y, alpha);
            }
        } else {
            // CIRCLES_MODE_REGULAR: solid fill
            outColor = data.colorsAsRGBA[cidx];
        }
        hit = true;
    }

    result[index] = hit ? outColor : uchar4(0, 0, 0, 0);
}
