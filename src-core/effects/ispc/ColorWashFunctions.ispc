/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the ColorWash effect.
// Implements the same per-pixel logic as ColorWashFunctions.metal so that
// Windows and Linux get a SIMD-accelerated path that matches macOS Metal.

struct ColorWashData {
    unsigned int width;
    unsigned int height;
    int          shimmerBlack;  // 1 if this is a shimmer black frame
    int          horizFade;
    int          vertFade;
    int          reverseFades;
    int          allowAlpha;
    uint8<4>     color;         // pre-blended palette color (RGBA)
    float        colorH;        // HSV hue   (0..1)
    float        colorS;        // HSV saturation (0..1)
    float        colorV;        // HSV value (0..1)
};

// HSV → RGBA conversion (alpha = 255).
// Mirrors the inline HSV→RGB in ColorWashFunctions.metal.
static inline uint8<4> cwHsv2rgba(float h, float s, float v) {
    float r, g, b;
    if (s == 0.0f) {
        r = g = b = v;
    } else {
        float hh = (h - floor(h)) * 6.0f;
        int   i  = (int)hh;
        float f  = hh - (float)i;
        float p  = v * (1.0f - s);
        float q  = v * (1.0f - s * f);
        float t  = v * (1.0f - s * (1.0f - f));
        if      (i == 0) { r = v; g = t; b = p; }
        else if (i == 1) { r = q; g = v; b = p; }
        else if (i == 2) { r = p; g = v; b = t; }
        else if (i == 3) { r = p; g = q; b = v; }
        else if (i == 4) { r = t; g = p; b = v; }
        else             { r = v; g = p; b = q; }
    }
    uint8<4> out;
    out[0] = (uint8)(clamp(r, 0.0f, 1.0f) * 255.0f);
    out[1] = (uint8)(clamp(g, 0.0f, 1.0f) * 255.0f);
    out[2] = (uint8)(clamp(b, 0.0f, 1.0f) * 255.0f);
    out[3] = 255;
    return out;
}

export void ColorWashEffectISPC(const uniform ColorWashData * uniform data,
                                uniform int startIdx,
                                uniform int endIdx,
                                uniform uint8<4> * uniform result)
{
    // Shimmer black frame: fill pixels with transparent black.
    if (data->shimmerBlack) {
        foreach (index = startIdx ... endIdx) {
            result[index][0] = 0;
            result[index][1] = 0;
            result[index][2] = 0;
            result[index][3] = 0;
        }
        return;
    }

    float halfWi = (float)(data->width  - 1) * 0.5f;
    float halfHt = (float)(data->height - 1) * 0.5f;

    foreach (index = startIdx ... endIdx) {
        int px = index % data->width;
        int py = index / data->width;

        float multH = 1.0f;
        float multV = 1.0f;

        if (data->horizFade) {
            float dist = abs((float)px - halfWi);
            if (data->reverseFades) {
                multH = (halfWi > 0.0f) ? dist / halfWi : 0.0f;
            } else {
                multH = (halfWi > 0.0f) ? 1.0f - dist / halfWi : 1.0f;
            }
            multH = clamp(multH, 0.0f, 1.0f);
        }

        if (data->vertFade) {
            float dist = abs((float)py - halfHt);
            if (data->reverseFades) {
                multV = (halfHt > 0.0f) ? dist / halfHt : 0.0f;
            } else {
                multV = (halfHt > 0.0f) ? 1.0f - dist / halfHt : 1.0f;
            }
            multV = clamp(multV, 0.0f, 1.0f);
        }

        if (data->allowAlpha) {
            float alpha = (float)data->color[3] * multH * multV;
            result[index][0] = data->color[0];
            result[index][1] = data->color[1];
            result[index][2] = data->color[2];
            result[index][3] = (uint8)clamp(alpha, 0.0f, 255.0f);
        } else {
            float v = clamp(data->colorV * multH * multV, 0.0f, 1.0f);
            result[index] = cwHsv2rgba(data->colorH, data->colorS, v);
        }
    }
}
