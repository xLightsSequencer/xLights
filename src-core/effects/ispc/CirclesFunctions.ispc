/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Circles effect.
// Covers radial, radial-3D, metaballs, regular filled, and fading modes.
// Bubbles (outline-only) and spatial palette colours fall back to scalar.

#define MAX_ISPC_CIRCLES_BALLS   20
#define MAX_ISPC_CIRCLES_COLORS   8

#define CIRCLES_MODE_RADIAL     0
#define CIRCLES_MODE_RADIAL_3D  1
#define CIRCLES_MODE_METABALLS  2
#define CIRCLES_MODE_REGULAR    3
#define CIRCLES_MODE_FADING     4

struct CirclesBall {
    float x;
    float y;
    float radius;
    int   colorIdx;
};

struct CirclesData {
    unsigned int width;
    unsigned int height;
    int          mode;
    int          numBalls;
    int          colorCount;
    int          allowAlpha;
    int          wrap;
    // Radial / Radial3D params (unused in ball modes)
    int          cx;
    int          cy;
    int          barSize;    // barht = BufferHt / (thickness+1)
    int          blockSize;  // blockHt = colorCount * barSize
    int          f_offset;
    int          maxRadius;
    int          number;
    int          effectState;
    CirclesBall  balls[MAX_ISPC_CIRCLES_BALLS];
    uint8<4>     colorsAsRGBA[MAX_ISPC_CIRCLES_COLORS];
    float        colorsH[MAX_ISPC_CIRCLES_COLORS];
    float        colorsS[MAX_ISPC_CIRCLES_COLORS];
    float        colorsV[MAX_ISPC_CIRCLES_COLORS];
};

// ── Helper: HSV → RGB (components in [0,1]) ──────────────────────────────────
static inline void circ_hsv2rgb(float h, float s, float v,
                                 float &r, float &g, float &b) {
    if (s == 0.0f) {
        r = v; g = v; b = v;
    } else {
        float hh    = (h - floor(h)) * 6.0f;
        int   i     = (int)hh;
        float f     = hh - (float)i;
        float p     = v * (1.0f - s);
        float q     = v * (1.0f - s * f);
        float t_val = v * (1.0f - s * (1.0f - f));
        if      (i == 0) { r = v;     g = t_val; b = p;     }
        else if (i == 1) { r = q;     g = v;     b = p;     }
        else if (i == 2) { r = p;     g = v;     b = t_val; }
        else if (i == 3) { r = p;     g = q;     b = v;     }
        else if (i == 4) { r = t_val; g = p;     b = v;     }
        else             { r = v;     g = p;     b = q;     }
    }
}

// ── Helper: RGB → HSV ─────────────────────────────────────────────────────────
static inline void circ_rgb2hsv(float r, float g, float b,
                                 float &h, float &s, float &v) {
    float maxC  = max(max(r, g), b);
    float minC  = min(min(r, g), b);
    float delta = maxC - minC;
    v = maxC;
    s = 0.0f;
    h = 0.0f;
    if (delta > 0.0001f) {
        s = delta / maxC;
        if      (maxC == r) h = (g - b) / delta;
        else if (maxC == g) h = (b - r) / delta + 2.0f;
        else                h = (r - g) / delta + 4.0f;
        h = h / 6.0f;
        if (h < 0.0f) h += 1.0f;
    }
}

// ── Helper: HSV → packed RGBA (alpha = 255) ──────────────────────────────────
static inline uint8<4> circ_hsv2rgba(float h, float s, float v) {
    float r, g, b;
    circ_hsv2rgb(h, s, v, r, g, b);
    uint8<4> out;
    out[0] = (uint8)clamp(r * 255.0f, 0.0f, 255.0f);
    out[1] = (uint8)clamp(g * 255.0f, 0.0f, 255.0f);
    out[2] = (uint8)clamp(b * 255.0f, 0.0f, 255.0f);
    out[3] = 255;
    return out;
}

// ── Helper: additive HSV blend ────────────────────────────────────────────────
// Converts both colours to RGB, adds, clamps, converts back to HSV.
// Modifies (ah, as_val, av) in-place.
static inline void addHSV(float &ah, float &as_val, float &av,
                           float bh, float bs_val, float bv) {
    float ar, ag, ab;
    circ_hsv2rgb(ah, as_val, av, ar, ag, ab);
    float br, bgc, bb;
    circ_hsv2rgb(bh, bs_val, bv, br, bgc, bb);
    float sr = clamp(ar + br, 0.0f, 1.0f);
    float sg = clamp(ag + bgc, 0.0f, 1.0f);
    float sb = clamp(ab + bb,  0.0f, 1.0f);
    circ_rgb2hsv(sr, sg, sb, ah, as_val, av);
}

// ── Main export ───────────────────────────────────────────────────────────────
export void CirclesEffectISPC(const uniform CirclesData * uniform data,
                               uniform int startIdx, uniform int endIdx,
                               uniform uint8<4> * uniform result)
{
    uniform int mode = data->mode;
    uniform int w    = (int)data->width;
    uniform int h    = (int)data->height;

    // ── RADIAL / RADIAL_3D ────────────────────────────────────────────────
    if (mode == CIRCLES_MODE_RADIAL || mode == CIRCLES_MODE_RADIAL_3D) {
        uniform int bs   = data->barSize  > 0 ? data->barSize  : 1;
        uniform int bk   = data->blockSize > 0 ? data->blockSize : 1;
        uniform int foff = data->f_offset;

        foreach (index = startIdx ... endIdx) {
            int   px   = index % w;
            int   py   = index / w;
            float dx   = (float)px - (float)data->cx;
            float dy   = (float)py - (float)data->cy;
            float dist = sqrt(dx * dx + dy * dy);
            int   ii   = (int)dist;

            if (ii > data->maxRadius) {
                // outside ring — leave buffer background untouched
            } else if (mode == CIRCLES_MODE_RADIAL_3D) {
                uniform float denom = (float)data->maxRadius /
                                      (float)(data->number > 0 ? data->number : 1);
                float hue = (float)(ii + data->effectState) /
                            (denom > 0.0f ? denom : 1.0f);
                hue = hue - floor(hue);
                result[index] = circ_hsv2rgba(hue, 1.0f, 1.0f);
            } else {
                int n        = ii - foff + bk;
                int colorIdx = ((n % bk) + bk) % bk / bs;
                colorIdx     = clamp(colorIdx, 0, data->colorCount - 1);
                result[index] = circ_hsv2rgba(data->colorsH[colorIdx],
                                              data->colorsS[colorIdx],
                                              data->colorsV[colorIdx]);
            }
        }
        return;
    }

    // ── METABALLS ─────────────────────────────────────────────────────────
    if (mode == CIRCLES_MODE_METABALLS) {
        foreach (index = startIdx ... endIdx) {
            int px = index % w;
            int py = index / w;

            float totalSum = 0.0f;
            float accH = 0.0f, accS = 0.0f, accV = 0.0f;

            for (uniform int ii = 0; ii < data->numBalls; ii++) {
                float dx  = (float)px - data->balls[ii].x;
                float dy  = (float)py - data->balls[ii].y;
                float d   = sqrt(dx * dx + dy * dy);
                float val = (d > 0.0001f) ? (data->balls[ii].radius / d) : 1.0f;
                totalSum += val;
                if (val > 0.30f) {
                    int   cidx   = clamp(data->balls[ii].colorIdx, 0, data->colorCount - 1);
                    float ball_h = data->colorsH[cidx];
                    float ball_s = data->colorsS[cidx];
                    float ball_v = val > 1.0f ? 1.0f : val;
                    addHSV(accH, accS, accV, ball_h, ball_s, ball_v);
                }
            }

            if (totalSum >= 0.90f) {
                result[index] = circ_hsv2rgba(accH, accS, accV);
            }
            // below threshold — leave buffer background untouched
        }
        return;
    }

    // ── REGULAR / FADING ──────────────────────────────────────────────────
    // CIRCLES_MODE_REGULAR: solid filled circles, last-writer-wins per pixel.
    // CIRCLES_MODE_FADING:  brightness fades from centre toward edge.
    foreach (index = startIdx ... endIdx) {
        int px = index % w;
        int py = index / w;

        uint8<4> outColor;
        bool hit = false;

        for (uniform int ii = 0; ii < data->numBalls; ii++) {
            float ballR = data->balls[ii].radius;
            float dx    = (float)px - data->balls[ii].x;
            float dy    = (float)py - data->balls[ii].y;

            if (data->wrap) {
                if (dx >  (float)w * 0.5f) dx -= (float)w;
                if (dx < -(float)w * 0.5f) dx += (float)w;
                if (dy >  (float)h * 0.5f) dy -= (float)h;
                if (dy < -(float)h * 0.5f) dy += (float)h;
            }

            float d = sqrt(dx * dx + dy * dy);

            if (d <= ballR) {
                int cidx = clamp(data->balls[ii].colorIdx, 0, data->colorCount - 1);

                if (mode == CIRCLES_MODE_FADING) {
                    float bv    = data->colorsV[cidx];
                    float alpha = bv * (1.0f - d / ballR);
                    if (alpha > 0.0f) {
                        if (data->allowAlpha) {
                            uint8<4> base = data->colorsAsRGBA[cidx];
                            uint8<4> c;
                            c[0] = base[0];
                            c[1] = base[1];
                            c[2] = base[2];
                            c[3] = (uint8)clamp((float)base[3] * (1.0f - d / ballR),
                                                0.0f, 255.0f);
                            outColor = c;
                        } else {
                            outColor = circ_hsv2rgba(data->colorsH[cidx],
                                                     data->colorsS[cidx],
                                                     alpha);
                        }
                        hit = true;
                    }
                } else {
                    outColor = data->colorsAsRGBA[cidx];
                    hit = true;
                }
            }
        }

        if (hit) {
            result[index] = outColor;
        }
        // no hit — leave buffer background untouched
    }
}
