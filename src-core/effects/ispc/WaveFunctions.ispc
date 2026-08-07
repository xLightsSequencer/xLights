/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Wave effect (the CPU path). The per-column vertical wave
// band [y1,y2] is precomputed on the CPU in double precision (including the
// std::sin the wave shapes need), so the kernel only does integer band tests
// plus per-pixel color. A column that draws nothing is encoded as y1 > y2 (an
// empty interval for both the main and mirror bands).
//
// Colors are evaluated with the same precision the scalar renderer used so the
// output is byte-identical: Rainbow via the exact-double xlColor::fromHSV, and
// Palette via GetMultiColorBlend's double-hue -> float-blend.
//
// Must stay in lockstep with WaveFunctions.metal, which implements the None-fill
// case only (the constant-color band). Rainbow/Palette need a double-precision
// hue division to stay byte-identical, which Metal has no type for, so those
// fills run here (the Metal wrapper falls back to this CPU path for them).

#define WAVE_FILL_NONE       0
#define WAVE_FILL_RAINBOW    1
#define WAVE_FILL_PALETTE    2
#define MAX_WAVE_ISPC_COLORS 8

struct WaveISPCData {
    int      width;
    int      height;
    int      fillColor;
    int      mirror;
    int      yoffset;
    int      numColors;
    uint8<4> noneColor;
    uint8<4> palColors[MAX_WAVE_ISPC_COLORS];
};

// xlColor::fromHSV for saturation=1, value=1 (the rainbow fill). Replicated
// verbatim in double so the lround-equivalent rounding matches bit-for-bit.
static inline uint8<4> waveRainbow(double hue) {
    double value = 1.0d;
    double saturation = 1.0d;
    double h = clamp(hue, 0.0d, 1.0d) * 6.0d;
    int i = (int)floor(h);
    double f = h - (double)i;
    double p = value * (1.0d - saturation);
    double red, green, blue;
    if (i == 1) {
        red = value * (1.0d - saturation * f);
        green = value;
        blue = p;
    } else if (i == 2) {
        red = p;
        green = value;
        blue = value * (1.0d - saturation * (1.0d - f));
    } else if (i == 3) {
        red = p;
        green = value * (1.0d - saturation * f);
        blue = value;
    } else if (i == 4) {
        red = value * (1.0d - saturation * (1.0d - f));
        green = p;
        blue = value;
    } else if (i == 5) {
        red = value;
        green = p;
        blue = value * (1.0d - saturation * f);
    } else { // i == 0 or i == 6
        red = value;
        green = value * (1.0d - saturation * (1.0d - f));
        blue = p;
    }
    uint8<4> out;
    out[0] = (uint8)(int)floor(red * 255.0d + 0.5d);
    out[1] = (uint8)(int)floor(green * 255.0d + 0.5d);
    out[2] = (uint8)(int)floor(blue * 255.0d + 0.5d);
    out[3] = 255;
    return out;
}

// RenderBuffer::GetMultiColorBlend(n, circular=false) followed by Get2ColorBlend
// (ChannelBlend), matching the scalar float math exactly. n is the float the
// scalar passes after computing the hue in double.
static inline uint8<4> wavePalette(const uniform WaveISPCData * uniform d, float n, uniform int colorcnt) {
    uint8<4> out;
    if (colorcnt <= 1) {
        // GetMultiColorBlend early-out: palette.GetColor(0, color) (keeps alpha).
        out = d->palColors[0];
        return out;
    }
    if (n >= 1.0f) n = 0.99999f;
    if (n < 0.0f) n = 0.0f;
    float realidx = n * (float)(colorcnt - 1);
    int coloridx1 = (int)floor((double)realidx);
    int coloridx2 = (coloridx1 + 1) % colorcnt;
    float ratio = realidx - (float)coloridx1;
    // Proactive no-op clamp: both indices are already < colorcnt <= 8; keeps the
    // ispc 1.30 gather off its crashy 64-bit path (ispc/ispc#3881/#3882).
    coloridx1 = min(coloridx1, MAX_WAVE_ISPC_COLORS - 1);
    coloridx2 = min(coloridx2, MAX_WAVE_ISPC_COLORS - 1);
    int c1r = (int)d->palColors[coloridx1][0];
    int c1g = (int)d->palColors[coloridx1][1];
    int c1b = (int)d->palColors[coloridx1][2];
    int c2r = (int)d->palColors[coloridx2][0];
    int c2g = (int)d->palColors[coloridx2][1];
    int c2b = (int)d->palColors[coloridx2][2];
    out[0] = (uint8)(c1r + (int)floor((double)(ratio * (float)(c2r - c1r)) + 0.5d));
    out[1] = (uint8)(c1g + (int)floor((double)(ratio * (float)(c2g - c1g)) + 0.5d));
    out[2] = (uint8)(c1b + (int)floor((double)(ratio * (float)(c2b - c1b)) + 0.5d));
    out[3] = 255;
    return out;
}

export void WaveEffectISPC(const uniform WaveISPCData * uniform d,
                           const uniform int * uniform cols,
                           uniform int startIdx,
                           uniform int endIdx,
                           uniform uint8<4> * uniform result) {
    uniform int width = d->width;
    uniform int height = d->height;
    uniform int fill = d->fillColor;
    uniform int mirror = d->mirror;
    uniform int yoff = d->yoffset;
    uniform int numColors = d->numColors;

    foreach (index = startIdx ... endIdx) {
        int x = index % width;
        int Y = index / width;
        int yy = Y - yoff;

        // Proactive no-op clamp before the gather (x is always < width).
        int cx = min(x, width - 1);
        int y1 = cols[2 * cx];
        int y2 = cols[2 * cx + 1];

        // The scalar draws the main band [y1,y2] then, when mirroring, overwrites
        // with the reflected band [height-y2, height-y1] (span = y2-y1), so the
        // mirror wins on overlap. deltay = y2-y1 (the main band) is the color
        // divisor for both bands.
        int bandY1 = y1;
        bool inBand = false;
        if (mirror != 0) {
            int mlo = height - y2;
            int mhi = height - y1;
            if (yy >= mlo && yy <= mhi) {
                bandY1 = mlo;
                inBand = true;
            }
        }
        if (!inBand && yy >= y1 && yy <= y2) {
            bandY1 = y1;
            inBand = true;
        }

        if (inBand) {
            int deltay = y2 - y1;
            int k = yy - bandY1;
            uint8<4> out;
            if (fill == WAVE_FILL_NONE) {
                out = d->noneColor;
            } else if (fill == WAVE_FILL_RAINBOW) {
                double hue = (double)k / (double)deltay;
                out = waveRainbow(hue);
            } else {
                double hue = (double)k / (double)deltay;
                out = wavePalette(d, (float)hue, numColors);
            }
            result[index] = out;
        }
    }
}
