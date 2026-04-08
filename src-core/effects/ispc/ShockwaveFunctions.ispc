/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Shockwave effect.
// Implements the same per-pixel logic as ShockwaveFunctions.metal so that
// Windows and Linux get a SIMD-accelerated path that matches macOS Metal.

struct ShockwaveData {
    unsigned int width;
    unsigned int height;
    float        xc_adj;        // center x in pixels
    float        yc_adj;        // center y in pixels
    float        radius1;       // inner edge of ring
    float        radius2;       // outer edge of ring
    float        radius_center; // midline of ring
    float        half_width;    // half thickness of ring
    int          blend_edges;
    int          allowAlpha;
    uint8<4>     color;         // pre-blended RGBA
    float        colorH;        // HSV hue   (blend_edges non-alpha path)
    float        colorS;        // HSV saturation
    float        colorV;        // HSV value
};

// HSV → RGBA, alpha = 255. Mirrors the Metal hsv2rgb helper.
static inline uint8<4> shockwaveHsv2rgba(float h, float s, float v) {
    float r, g, b;
    if (s == 0.0f) {
        r = g = b = v;
    } else {
        float hh = (h - floor(h)) * 6.0f;
        int   i  = (int)hh;
        float f  = hh - (float)i;
        float p  = v * (1.0f - s);
        float q  = v * (1.0f - s * f);
        float t2 = v * (1.0f - s * (1.0f - f));
        if      (i == 0) { r = v;  g = t2; b = p;  }
        else if (i == 1) { r = q;  g = v;  b = p;  }
        else if (i == 2) { r = p;  g = v;  b = t2; }
        else if (i == 3) { r = p;  g = q;  b = v;  }
        else if (i == 4) { r = t2; g = p;  b = v;  }
        else             { r = v;  g = p;  b = q;  }
    }
    uint8<4> out;
    out[0] = (uint8)(clamp(r, 0.0f, 1.0f) * 255.0f);
    out[1] = (uint8)(clamp(g, 0.0f, 1.0f) * 255.0f);
    out[2] = (uint8)(clamp(b, 0.0f, 1.0f) * 255.0f);
    out[3] = 255;
    return out;
}

export void ShockwaveEffectISPC(const uniform ShockwaveData * uniform data,
                                uniform int startIdx,
                                uniform int endIdx,
                                uniform uint8<4> * uniform result) {
    uniform int   width         = (int)data->width;
    uniform float xc_adj        = data->xc_adj;
    uniform float yc_adj        = data->yc_adj;
    uniform float radius1       = data->radius1;
    uniform float radius2       = data->radius2;
    uniform float radius_center = data->radius_center;
    uniform float half_width    = data->half_width;
    uniform int   blend_edges   = data->blend_edges;
    uniform int   allowAlpha    = data->allowAlpha;
    uniform uint8<4> color      = data->color;
    uniform float colorH        = data->colorH;
    uniform float colorS        = data->colorS;
    uniform float colorV        = data->colorV;

    foreach (index = startIdx ... endIdx) {
        int   x  = index % width;
        int   y  = index / width;
        float x1 = (float)x - xc_adj;
        float y1 = (float)y - yc_adj;
        float r  = sqrt(x1 * x1 + y1 * y1);

        if (r >= radius1 && r <= radius2) {
            if (blend_edges) {
                float color_pct = 1.0f - abs(r - radius_center) / half_width;
                uint8<4> ncolor = color;
                if (allowAlpha) {
                    ncolor[3] = (uint8)(255.0f * color_pct);
                } else {
                    ncolor = shockwaveHsv2rgba(colorH, colorS, colorV * color_pct);
                }
                result[index] = ncolor;
            } else {
                result[index] = color;
            }
        }
    }
}
