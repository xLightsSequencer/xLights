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

// GPU port of the axis-aligned Meteors gather (mirrors MeteorsFunctions.ispc). The
// CPU snapshots the live meteors into the parts buffer with the swirl already folded
// into the axis coordinate; each thread inverts the scatter for one output pixel and
// the last covering meteor in draw order wins (SetPixel overwrite). Float math, so a
// handful of pixels may differ by +/-1 from the double-precision CPU path.
//
// A meteor's axis coord (parts[].a) fixes the only line of the buffer it can ever
// cover -- the column it falls down (vertical/icicle) or the row it crosses
// (horizontal) -- so the CPU buckets the snapshot by line (MeteorsEffect::
// BucketMeteorsByLine) and a thread scans only its own line's meteors:
// lineItems[lineStart[line] .. lineStart[line+1]), global indices into parts, in
// ascending order. Restricting the scan cannot change which meteor wins a pixel --
// the ones left out fail the a == x / a == y test anyway -- but it turns the flat
// O(pixels x meteors) scan into O(pixels x meteors-on-this-line).

// Bit-identical local copy of RenderBuffer::rngMix64 (RenderBuffer.h). frameSeed is
// hashRandomFrameSeed(); hashRand01(index) == (mix64(frameSeed ^ index*K) >> 11) * 2^-53.
static inline ulong meteorMix64(ulong z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9UL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBUL;
    return z ^ (z >> 31);
}
static inline float meteorHashRand01(ulong frameSeed, uint index) {
    ulong z = frameSeed ^ (ulong(index) * 0xD1B54A32D192ED03UL);
    return float(meteorMix64(z) >> 11) * (1.0f / 9007199254740992.0f);
}

// xlColor::fromHSV (Color.cpp) in float — clamped hue/sat/value, lround via floor(x+0.5).
static inline uchar4 meteorFromHSV(float hue, float saturation, float value) {
    float v = clamp(value, 0.0f, 1.0f);
    float s = clamp(saturation, 0.0f, 1.0f);
    float red, green, blue;
    if (s == 0.0f) {
        red = v; green = v; blue = v;
    } else {
        float h = clamp(hue, 0.0f, 1.0f) * 6.0f;
        int i = (int)floor(h);
        float f = h - (float)i;
        float p = v * (1.0f - s);
        if (i == 1) {
            red = v * (1.0f - s * f);         green = v;                          blue = p;
        } else if (i == 2) {
            red = p;                          green = v;                          blue = v * (1.0f - s * (1.0f - f));
        } else if (i == 3) {
            red = p;                          green = v * (1.0f - s * f);         blue = v;
        } else if (i == 4) {
            red = v * (1.0f - s * (1.0f - f)); green = p;                         blue = v;
        } else if (i == 5) {
            red = v;                          green = p;                          blue = v * (1.0f - s * f);
        } else { // i == 0 or i == 6
            red = v;                          green = v * (1.0f - s * (1.0f - f)); blue = p;
        }
    }
    uchar4 out;
    out.r = (uchar)floor(red * 255.0f + 0.5f);
    out.g = (uchar)floor(green * 255.0f + 0.5f);
    out.b = (uchar)floor(blue * 255.0f + 0.5f);
    out.a = 255;
    return out;
}

kernel void MeteorsEffect(constant MetalMeteorsData &d          [[buffer(0)]],
                          device uchar4 *result                 [[buffer(1)]],
                          device const MetalMeteorParticle *parts [[buffer(2)]],
                          device const int *lineItems           [[buffer(3)]],
                          device const int *lineStart           [[buffer(4)]],
                          uint index                            [[thread_position_in_grid]]) {
    if (index >= d.width * d.height) return;
    int W = (int)d.width;
    int H = (int)d.height;
    int x = (int)(index % d.width);
    int y = (int)(index / d.width);
    int TL = d.tailLength < 1 ? 1 : d.tailLength;
    int mode = d.mode, dir = d.direction, scheme = d.colorScheme, allowAlpha = d.allowAlpha;

    // horizontal meteors cross a row, vertical/icicle ones fall down a column
    int line = (mode == 1) ? y : x;
    int itemStart = lineStart[line];
    int itemEnd = lineStart[line + 1];

    int bestN = -1;
    int bestPh = 0;
    for (int i = itemStart; i < itemEnd; i++) {
        int n = lineItems[i];
        int a = parts[n].a;
        int base = parts[n].base;
        bool cover = false;
        int ph = 0;
        if (mode == 0) {                 // vertical
            if (a == x) {
                ph = (dir == 0) ? (y - base) : (H - y - base);
                if (ph >= 0 && ph <= TL) cover = true;
            }
        } else if (mode == 1) {          // horizontal
            if (a == y) {
                ph = (dir == 2) ? (x - base) : (W - x - base);
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
        uchar4 col;
        if (mode == 2) {
            bool colored = (bestPh == 0) || (bestPh <= parts[bestN].h - parts[bestN].base);
            if (colored) {
                col = meteorFromHSV(parts[bestN].hue, parts[bestN].sat, parts[bestN].val);
            } else {
                col = meteorFromHSV(0.0f, 0.0f, 0.4f); // white icicle body
            }
        } else {
            float hue, sat, val;
            if (scheme == 0) {           // rainbow
                uint idx = (uint)bestN * 131101u + (uint)bestPh;
                hue = meteorHashRand01(d.frameSeed, idx);
                sat = 1.0f; val = 1.0f;
            } else {
                hue = parts[bestN].hue; sat = parts[bestN].sat; val = parts[bestN].val;
            }
            float fade = 1.0f - (float)bestPh / (float)TL;
            if (allowAlpha) {
                col = meteorFromHSV(hue, sat, val);
                col.a = (uchar)(255.0f * fade);
            } else {
                col = meteorFromHSV(hue, sat, val * fade);
            }
        }
        result[index] = col;
    } else if (mode == 2 && d.wantBkg) {
        if ((x % 3) == 0) {
            int k = (x / 3) % 5;
            int s = (k == 0) ? 0 : (k == 1) ? 5 : (k == 2) ? 1 : (k == 3) ? 2 : 4;
            int yy = y - s;
            if (yy >= 0 && (yy % 3) == 0) {
                result[index] = uchar4(100, 50, 255, 255);
            }
        }
    }
}
