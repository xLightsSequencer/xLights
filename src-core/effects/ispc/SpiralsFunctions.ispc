/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Spirals effect.
// Implements a per-pixel inversion of the strand formula so that Windows
// and Linux get a SIMD-accelerated path equivalent to the scalar fallback
// (minus Blend and spatial-color modes, which fall back to scalar).

#define MAX_ISPC_SPIRALS_COLORS 8

struct SpiralsData {
    unsigned int width;
    unsigned int height;
    int          spiralCount;
    int          colorCount;
    float        spiralState;    // SpiralState (long cast to float)
    float        rotation;       // Rotation (degrees per height)
    float        rotation_sign;  // -1 flips the 3D fade side (Movement < 0, xor Reverse 3D)
    float        deltaStrands;   // BufferWi / spiralCount
    float        spiralThickness;
    int          show3D;
    int          allowAlpha;
    uint8<4>     colorsAsRGBA[MAX_ISPC_SPIRALS_COLORS];
    float        colorsH[MAX_ISPC_SPIRALS_COLORS];
    float        colorsS[MAX_ISPC_SPIRALS_COLORS];
    float        colorsV[MAX_ISPC_SPIRALS_COLORS];
};

// HSV -> RGBA, alpha = 255.  Mirrors the Metal / Shockwave helper.
static inline uint8<4> spiralsHsv2rgba(float h, float s, float v) {
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

export void SpiralsEffectISPC(const uniform SpiralsData * uniform data,
                               uniform int startIdx,
                               uniform int endIdx,
                               uniform uint8<4> * uniform result) {
    uniform int   width           = (int)data->width;
    uniform int   height          = (int)data->height;
    uniform float fwidth          = (float)width;
    uniform float fheight         = (float)height;
    uniform int   spiralCount     = data->spiralCount;
    uniform int   colorCount      = data->colorCount;
    uniform float spiralState     = data->spiralState;
    uniform float rotation        = data->rotation;
    uniform float rotation_sign   = data->rotation_sign;
    uniform float deltaStrands    = data->deltaStrands;
    uniform float spiralThickness = data->spiralThickness;
    uniform int   show3D          = data->show3D;
    uniform int   allowAlpha      = data->allowAlpha;

    // Walk rows instead of deriving x/y from index % width: there is no
    // vector integer divide, so ispc runs a varying % or / one lane at a time.
    for (uniform int y = startIdx / max(width, 1); width > 0 && y * width < endIdx; y++) {
        uniform int rowStart = y * width;
        foreach (x = max(startIdx - rowStart, 0) ... min(endIdx - rowStart, width)) {
            int index = rowStart + x;

            // Invert the strand formula:
            //   original: x = (int)(strand + spiralState/10 + y*rotation/height) % width
            //   where strand = (strand_base + thick) % width
            // So raw = x - spiralState/10 - y*rotation/height  (mod width) gives
            // the "virtual strand + thick" coordinate.
            float raw = (float)x - spiralState / 10.0f - (float)y * rotation / fheight;

            // Bring raw into [0, fwidth)
            raw = raw - floor(raw / fwidth) * fwidth;

            // Which spiral stripe does this pixel fall on?
            int   ns  = (int)(raw / deltaStrands);
            float pos = raw - (float)ns * deltaStrands;  // position within stripe [0, deltaStrands)

            if (pos >= 0.0f && pos < spiralThickness) {
                // ns is almost always already in range; only lanes that are not pay
                // for a varying % (same result as (ns % spiralCount) % colorCount).
                int ColorIdx = ns;
                if (ColorIdx < 0 || ColorIdx >= spiralCount) ColorIdx = ColorIdx % spiralCount;
                if (ColorIdx < 0 || ColorIdx >= colorCount) ColorIdx = ColorIdx % colorCount;

                uint8<4> out;
                if (show3D) {
                    // Continuous 3D gradient across the stripe thickness.
                    // rotation_sign >= 0: brightest at leading edge (pos=0), dims outward.
                    // rotation_sign <  0: dims at leading edge, brightest at trailing edge.
                    float f;
                    if (rotation_sign < 0.0f) {
                        f = (pos + 1.0f) / spiralThickness;
                    } else {
                        f = (spiralThickness - pos) / spiralThickness;
                    }
                    f = clamp(f, 0.0f, 1.0f);
                    if (allowAlpha) {
                        out    = data->colorsAsRGBA[ColorIdx];
                        out[3] = (uint8)(255.0f * f);
                    } else {
                        out = spiralsHsv2rgba(data->colorsH[ColorIdx],
                                             data->colorsS[ColorIdx],
                                             data->colorsV[ColorIdx] * f);
                    }
                } else {
                    out = data->colorsAsRGBA[ColorIdx];
                }
                result[index] = out;
            }
        }
    }
}
