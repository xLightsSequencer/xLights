/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Per-pixel gather for the axis-aligned Meteors sub-styles (Down/Up, Left/Right,
// Icicles). The CPU keeps the sequential spawn/move/expire bookkeeping and, each
// frame, snapshots the live meteor list into the "parts" array (with the swirl
// already folded into the axis coordinate). This kernel inverts the scatter: each
// output pixel scans the meteors, finds every trail phase that covers it, and the
// last one in draw order wins (SetPixel overwrite semantics). Radial styles
// (Implode/Explode) are NOT handled here - they stay on the scalar path.

struct MeteorParticle {
    int    a;      // primary axis coord: column (vertical/icicle) or row (horizontal)
    int    base;   // meteor.y (vertical/icicle) or meteor.x (horizontal)
    int    h;      // icicle drip length (meteor.h); unused for vertical/horizontal
    double hue;    // meteor.hsv (range/palette; also the icicle color, incl. rainbow's default black)
    double sat;
    double val;
};

struct MeteorsISPCData {
    int width;
    int height;
    int mode;         // 0 = vertical, 1 = horizontal, 2 = icicle
    int direction;    // raw MeteorsEffect: 0 Down, 1 Up, 2 Left, 3 Right, 6/7 Icicle
    int tailLength;
    int colorScheme;  // 0 = rainbow, 1 = range, 2 = palette
    int allowAlpha;
    int numMeteors;
    int wantBkg;      // icicle: draw the dim background icicles
    unsigned int64 frameSeed; // RenderBuffer::hashRandomFrameSeed() for the rainbow scheme
};

// Bit-identical local copy of RenderBuffer::rngMix64 (RenderBuffer.h). frameSeed is
// hashRandomFrameSeed() (base seed with curPeriod mixed in); hashRand01(index) ==
// (mix64(frameSeed ^ index*K) >> 11) * 2^-53.
static inline unsigned int64 meteorMix64(unsigned int64 z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EB;
    return z ^ (z >> 31);
}
static inline double meteorHashRand01(unsigned int64 frameSeed, unsigned int32 index) {
    unsigned int64 z = frameSeed ^ ((unsigned int64)index * 0xD1B54A32D192ED03);
    return (double)(meteorMix64(z) >> 11) * (1.0d / 9007199254740992.0d);
}

// RenderBuffer/xlColor::fromHSV in double, matching Color.cpp bit-for-bit (clamped
// hue/sat/value, lround via floor(x+0.5)).
static inline uint8<4> meteorFromHSV(double hue, double saturation, double value) {
    double v = clamp(value, 0.0d, 1.0d);
    double s = clamp(saturation, 0.0d, 1.0d);
    double red, green, blue;
    if (s == 0.0d) {
        red = v; green = v; blue = v;
    } else {
        double h = clamp(hue, 0.0d, 1.0d) * 6.0d;
        int i = (int)floor(h);
        double f = h - (double)i;
        double p = v * (1.0d - s);
        if (i == 1) {
            red = v * (1.0d - s * f);        green = v;                          blue = p;
        } else if (i == 2) {
            red = p;                          green = v;                          blue = v * (1.0d - s * (1.0d - f));
        } else if (i == 3) {
            red = p;                          green = v * (1.0d - s * f);        blue = v;
        } else if (i == 4) {
            red = v * (1.0d - s * (1.0d - f)); green = p;                         blue = v;
        } else if (i == 5) {
            red = v;                          green = p;                          blue = v * (1.0d - s * f);
        } else { // i == 0 or i == 6
            red = v;                          green = v * (1.0d - s * (1.0d - f)); blue = p;
        }
    }
    uint8<4> out;
    out[0] = (uint8)(int)floor(red * 255.0d + 0.5d);
    out[1] = (uint8)(int)floor(green * 255.0d + 0.5d);
    out[2] = (uint8)(int)floor(blue * 255.0d + 0.5d);
    out[3] = 255;
    return out;
}

// One "line" of the buffer: the column a vertical/icicle meteor falls down, or the
// row a horizontal meteor crosses. A meteor's axis coord (parts[].a) fixes the only
// line it can ever cover, so the caller buckets the snapshot by line and passes just
// that line's meteors here (items[itemStart..itemEnd), global indices into parts, in
// ascending order). Restricting the scan cannot change which meteor wins a pixel --
// the ones left out fail the a == x / a == y test anyway -- but it turns the flat
// O(pixels x meteors) scan into O(pixels x meteors-on-this-line).
export void MeteorsEffectLineISPC(const uniform MeteorsISPCData * uniform d,
                                  const uniform MeteorParticle * uniform parts,
                                  const uniform int * uniform items,
                                  uniform int itemStart, uniform int itemEnd,
                                  uniform int line,
                                  uniform uint8<4> * uniform result) {
    uniform int W = d->width;
    uniform int H = d->height;
    uniform int mode = d->mode;
    uniform int dir = d->direction;
    uniform int TL = d->tailLength < 1 ? 1 : d->tailLength;
    uniform int scheme = d->colorScheme;
    uniform int allowAlpha = d->allowAlpha;
    uniform int wantBkg = d->wantBkg;
    uniform unsigned int64 frameSeed = d->frameSeed;

    // matches RenderIcicleDrip's ystaggered[] pattern
    const uniform int yStag0 = 0, yStag1 = 5, yStag2 = 1, yStag3 = 2, yStag4 = 4;

    // horizontal walks the row "line"; vertical/icicle walk the column "line"
    uniform int lineLen = (mode == 1) ? W : H;

    foreach (p = 0 ... lineLen) {
        int x = (mode == 1) ? p : line;
        int y = (mode == 1) ? line : p;
        int index = y * W + x;

        int bestN = -1;
        int bestPh = 0;

        for (uniform int i = itemStart; i < itemEnd; i++) {
            uniform int n = items[i];
            uniform int a = parts[n].a;
            uniform int base = parts[n].base;
            bool cover = false;
            int ph = 0;
            if (mode == 0) {                 // vertical
                if (a == x) {
                    ph = (dir == 0) ? (y - base) : (H - y - base); // Down : Up
                    if (ph >= 0 && ph <= TL) cover = true;
                }
            } else if (mode == 1) {          // horizontal
                if (a == y) {
                    ph = (dir == 2) ? (x - base) : (W - x - base); // Left : Right
                    if (ph >= 0 && ph <= TL) cover = true;
                }
            } else {                         // icicle
                if (a == x) {
                    ph = y - base;
                    if (ph >= 0 && ph <= TL && y >= parts[n].h) cover = true;
                }
            }
            if (cover) { bestN = n; bestPh = ph; }
        }

        if (bestN >= 0) {
            int bn = bestN < 0 ? 0 : bestN; // guard the gather address on masked lanes
            uint8<4> col;
            if (mode == 2) {
                bool colored = (bestPh == 0) || (bestPh <= parts[bn].h - parts[bn].base);
                if (colored) {
                    col = meteorFromHSV(parts[bn].hue, parts[bn].sat, parts[bn].val);
                } else {
                    col = meteorFromHSV(0.0d, 0.0d, 0.4d); // white icicle body
                }
            } else {
                double hue, sat, val;
                if (scheme == 0) {           // rainbow
                    unsigned int32 idx = (unsigned int32)bestN * 131101u + (unsigned int32)bestPh;
                    hue = meteorHashRand01(frameSeed, idx);
                    sat = 1.0d; val = 1.0d;
                } else {
                    hue = parts[bn].hue; sat = parts[bn].sat; val = parts[bn].val;
                }
                double fade = 1.0d - (double)bestPh / (double)TL;
                if (allowAlpha) {
                    col = meteorFromHSV(hue, sat, val);
                    col[3] = (uint8)(255.0d * fade);
                } else {
                    col = meteorFromHSV(hue, sat, val * fade);
                }
            }
            result[index] = col;
        } else if (mode == 2 && wantBkg) {
            if ((x % 3) == 0) {
                int k = (x / 3) % 5;
                int s = (k == 0) ? yStag0 : (k == 1) ? yStag1 : (k == 2) ? yStag2 : (k == 3) ? yStag3 : yStag4;
                int yy = y - s;
                if (yy >= 0 && (yy % 3) == 0) {
                    uint8<4> bg;
                    bg[0] = 100; bg[1] = 50; bg[2] = 255; bg[3] = 255;
                    result[index] = bg;
                }
            }
        }
    }
}
