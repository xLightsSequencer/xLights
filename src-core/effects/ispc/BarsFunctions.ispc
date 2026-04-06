/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Bars effect.
// Implements the same per-pixel logic as BarsFunctions.metal so that
// Windows and Linux get a SIMD-accelerated path that matches macOS Metal.

#define MAX_ISPC_BARS_COLORS 8

// Direction constants — must match GetDirection() in BarsEffect.cpp
#define BARS_DIR_UP        0
#define BARS_DIR_DOWN      1
#define BARS_DIR_EXPAND    2
#define BARS_DIR_COMPRESS  3
#define BARS_DIR_LEFT      4
#define BARS_DIR_RIGHT     5
#define BARS_DIR_HEXPAND   6
#define BARS_DIR_HCOMPRESS 7

struct BarsData {
    unsigned int width;
    unsigned int height;
    int          direction;     // 0-7 after alternate remapping; custom falls back to CPU
    int          barSize;       // barHt (vertical) or barWi (horizontal), in pixels
    int          blockSize;     // colorCount * barSize
    int          f_offset;      // animation offset in pixels
    int          newCenter;     // center pixel for expand/compress modes
    int          colorCount;
    int          highlight;
    int          show3D;
    int          gradient;
    int          allowAlpha;
    int          useFirstColorForHighlight;
    uint8<4>     colorsAsRGBA[MAX_ISPC_BARS_COLORS];
    float        colorsH[MAX_ISPC_BARS_COLORS];
    float        colorsS[MAX_ISPC_BARS_COLORS];
    float        colorsV[MAX_ISPC_BARS_COLORS];
    uint8<4>     highlightColor;
};

// Full HSV → RGBA conversion (alpha = 255).
// Mirrors barsHsv2rgba() from BarsFunctions.metal.
static inline uint8<4> barsHsv2rgba(float h, float s, float v) {
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

// Blend two HSV colors along the shortest hue arc and return RGBA.
// Mirrors blendHSV() from BarsFunctions.metal.
static inline uint8<4> blendHSV(float h1, float s1, float v1,
                                 float h2, float s2, float v2,
                                 float t) {
    float dh = h2 - h1;
    if (dh >  0.5f) dh -= 1.0f;
    if (dh < -0.5f) dh += 1.0f;
    float bh = h1 + t * dh;
    bh = bh - floor(bh);
    return barsHsv2rgba(bh, s1 + t * (s2 - s1), v1 + t * (v2 - v1));
}

// Linearly blend two RGBA colors.
static inline uint8<4> blendRGBA(uint8<4> c1, uint8<4> c2, float t) {
    uint8<4> out;
    out[0] = (uint8)((float)c1[0] + t * ((float)c2[0] - (float)c1[0]));
    out[1] = (uint8)((float)c1[1] + t * ((float)c2[1] - (float)c1[1]));
    out[2] = (uint8)((float)c1[2] + t * ((float)c2[2] - (float)c1[2]));
    out[3] = (uint8)((float)c1[3] + t * ((float)c2[3] - (float)c1[3]));
    return out;
}

// Compute the output RGBA for bar-position integer n.
// Mirrors computeBarColor() from BarsFunctions.metal exactly.
static inline uint8<4> computeBarColor(int n, const uniform BarsData * uniform data) {
    int absBlockN = abs(n % data->blockSize);
    int colorIdx  = absBlockN / data->barSize;
    int color2    = (colorIdx + 1) % data->colorCount;
    int absBarN   = abs(n % data->barSize);
    float pct     = (float)absBarN / (float)data->barSize;

    // Highlight: at the boundary of each bar use the highlight color
    if (data->highlight && (n % data->barSize) == 0) {
        return data->highlightColor;
    }

    uint8<4> rgba;
    if (data->allowAlpha) {
        rgba = data->colorsAsRGBA[colorIdx];
        if (data->gradient) {
            rgba = blendRGBA(rgba, data->colorsAsRGBA[color2], pct);
        }
        if (data->show3D) {
            int num = data->barSize - absBarN - 1;
            rgba[3] = (uint8)(255.0f * (float)num / (float)data->barSize);
        }
    } else {
        float h = data->colorsH[colorIdx];
        float s = data->colorsS[colorIdx];
        float v = data->colorsV[colorIdx];
        if (data->gradient) {
            return blendHSV(h, s, v,
                            data->colorsH[color2],
                            data->colorsS[color2],
                            data->colorsV[color2],
                            pct);
        }
        if (data->show3D) {
            int num = data->barSize - absBarN - 1;
            v *= (float)num / (float)data->barSize;
        }
        rgba = barsHsv2rgba(h, s, v);
    }
    return rgba;
}

// Main export function: renders pixels [startIdx, endIdx) into result.
// result must point to the start of the full pixel buffer (size = width * height).
export void BarsEffectISPC(const uniform BarsData * uniform data,
                           uniform int startIdx,
                           uniform int endIdx,
                           uniform uint8<4> * uniform result) {
    uniform int  dir       = data->direction;
    uniform bool isVert    = (dir <= BARS_DIR_COMPRESS);
    uniform int  width     = (int)data->width;
    uniform int  height    = (int)data->height;
    uniform int  newCenter = data->newCenter;
    uniform int  f_offset  = data->f_offset;

    foreach (index = startIdx ... endIdx) {
        int px = index % width;
        int py = index / width;

        int n;
        if (isVert) {
            // Vertical directions: bars run horizontally, animate along Y.
            int y;
            if (dir == BARS_DIR_DOWN) {
                y = py;
            } else if (dir == BARS_DIR_EXPAND) {
                y = (py <= newCenter) ? py : (2 * newCenter - py);
            } else if (dir == BARS_DIR_COMPRESS) {
                y = (py >= newCenter) ? py : (2 * newCenter - py);
            } else { // BARS_DIR_UP
                y = height - py - 1;
            }
            n = height + y + f_offset;
        } else {
            // Horizontal directions: bars run vertically, animate along X.
            int x;
            if (dir == BARS_DIR_RIGHT) {
                x = width - px - 1;
            } else if (dir == BARS_DIR_HEXPAND) {
                x = (px <= newCenter) ? px : (2 * newCenter - px);
            } else if (dir == BARS_DIR_HCOMPRESS) {
                x = (px >= newCenter) ? px : (2 * newCenter - px);
            } else { // BARS_DIR_LEFT
                x = px;
            }
            n = width + x + f_offset;
        }

        result[index] = computeBarColor(n, data);
    }
}
