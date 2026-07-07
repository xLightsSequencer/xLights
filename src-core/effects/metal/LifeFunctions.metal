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

// GPU port of Conway's Game of Life (mirrors LifeFunctions.ispc — keep the math in
// lockstep). Each thread owns one pixel/cell: it reads its 8 wrapped neighbours
// from the PREVIOUS generation (a read-only copy of the RenderBuffer TempBuf),
// applies the birth/survival rule for the selected ruleset, and writes the new
// generation. Newly-born cells get their colour from
// GetMultiColorBlend(hashRand01(index)), replicated bit-identically here.
//
// FP contraction is disabled so the birth-colour blend rounds each float product
// before adding 0.5, exactly as the scalar renderer does (its float product is
// promoted to double before +0.5, which can never fuse into an FMA).
#pragma clang fp contract(off)

// RenderBuffer::rngMix64 (splitmix64 finalizer), bit-identical.
static inline ulong lifeMix64(ulong z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9UL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBUL;
    return z ^ (z >> 31);
}

// RenderBuffer::hashRand01(index) narrowed to float (GetMultiColorBlend's param).
// hashRand01 = (mix64(frameSeed ^ index*K) >> 11) * 2^-53. Metal has no double, but
// float(r53) * 2^-53 == (float)((double)r53 * 2^-53) because scaling by an exact
// power of two commutes with the float rounding, so this matches the scalar.
static inline float lifeHashRand01f(ulong frameSeed, uint index) {
    ulong z = frameSeed ^ (ulong(index) * 0xD1B54A32D192ED03UL);
    ulong r53 = lifeMix64(z) >> 11;
    return float(r53) * (1.0f / 9007199254740992.0f);
}

// RenderBuffer::ChannelBlend, bit-identical: a + floor(ratio*(b-a) + 0.5).
static inline uchar lifeChannelBlend(uchar a, uchar b, float ratio) {
    float m = ratio * float(int(b) - int(a));
    float v = m + 0.5f;
    int f = int(floor(v));
    return uchar(int(a) + f);
}

// RenderBuffer::GetMultiColorBlend(hashRand01(index), circular=false) as RGBA
// (alpha 255).
static inline uchar4 lifeBirthColor(constant MetalLifeData &d,
                                    device const uchar4 *palette,
                                    uint index) {
    int colorcnt = d.numColors;
    if (colorcnt <= 1) {
        // GetMultiColorBlend returns GetColor(0) verbatim (its own alpha) here;
        // only the 2-colour blend below forces alpha 255.
        return palette[0];
    }
    float nf = lifeHashRand01f(d.frameSeed, index);
    if (nf >= 1.0f) nf = 0.99999f;
    if (nf < 0.0f) nf = 0.0f;
    float realidx = nf * float(colorcnt - 1);
    int idx1 = int(floor(realidx));
    int idx2 = (idx1 + 1) % colorcnt;
    float ratio = realidx - float(idx1);

    uchar4 c1 = palette[idx1];
    uchar4 c2 = palette[idx2];
    uchar4 o;
    o.r = lifeChannelBlend(c1.r, c2.r, ratio);
    o.g = lifeChannelBlend(c1.g, c2.g, ratio);
    o.b = lifeChannelBlend(c1.b, c2.b, ratio);
    o.a = 255;
    return o;
}

static inline bool lifeIsLive(uchar4 c) {
    return (c.r | c.g | c.b) != 0;
}

kernel void LifeEffect(constant MetalLifeData &d      [[buffer(0)]],
                       device const uchar4 *prev      [[buffer(1)]],
                       device uchar4 *result          [[buffer(2)]],
                       device const uchar4 *palette   [[buffer(3)]],
                       uint index                     [[thread_position_in_grid]]) {
    int width = int(d.width);
    int height = int(d.height);
    int npix = d.npix;
    if (index >= uint(npix)) return;

    int x0 = int(index) % width;
    int y0 = int(index) / width;

    // Neighbour offsets (match Life_CountNeighbors):
    //     2   3   4
    //     1   X   5
    //     0   7   6
    const int n_x[8] = { -1, -1, -1, 0, 1, 1, 1, 0 };
    const int n_y[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };

    uchar4 selfc = prev[index];
    bool live = lifeIsLive(selfc);

    int cnt = 0;
    for (int i = 0; i < 8; ++i) {
        int nx = (x0 + n_x[i]) % width;
        int ny = (y0 + n_y[i]) % height;
        if (nx < 0) nx += width;
        if (ny < 0) ny += height;
        int nidx = ny * width + nx;
        // Out-of-allocation neighbours read as black (scalar GetTempPixelRGB bounds
        // guard) and stay within the uploaded previous-generation buffer.
        if (nidx < npix && lifeIsLive(prev[nidx])) ++cnt;
    }

    int type = d.type;
    bool survive = false;
    bool birth = false;
    if (type == 0) {
        survive = live && cnt >= 2 && cnt <= 3;
        birth = !live && cnt == 3;
    } else if (type == 1) {
        survive = live && (cnt == 2 || cnt == 3 || cnt == 6);
        birth = !live && (cnt == 3 || cnt == 5);
    } else if (type == 2) {
        survive = live && (cnt == 1 || cnt == 3 || cnt == 5 || cnt == 8);
        birth = !live && (cnt == 3 || cnt == 5 || cnt == 7);
    } else if (type == 3) {
        survive = live && (cnt == 2 || cnt == 3 || cnt >= 5);
        birth = !live && (cnt == 3 || cnt == 7 || cnt == 8);
    } else if (type == 4) {
        survive = live && (cnt >= 5);
        birth = !live && (cnt == 2 || cnt >= 5);
    }

    uchar4 out = uchar4(0, 0, 0, 0);
    if (survive) {
        out = selfc;
    } else if (birth) {
        out = lifeBirthColor(d, palette, index);
    }
    result[index] = out;
}
