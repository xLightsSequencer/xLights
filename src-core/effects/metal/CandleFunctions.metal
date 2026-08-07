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

// GPU port of the Candle perNode flame simulation (mirrors CandleFunctions.ispc —
// keep the math in lockstep). Each thread owns one pixel/node: it runs both
// Update() passes (red then green, sharing the wind byte) against the persistent
// 5-byte CandleState and writes the pixel. The state buffer is read-modify-write
// in place and copied back to the CPU-side cache after dispatch, so the CPU cache
// stays the single source of truth.

// RenderBuffer::rngMix64 (splitmix64 finalizer), bit-identical.
static inline ulong candleMix64(ulong z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9UL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBUL;
    return z ^ (z >> 31);
}

// Bit-identical integer emulation of uint8_t(RenderBuffer::hashRand01(index) * 255.0):
// hashRand01 = (mix64(seed ^ index*K) >> 11) * 2^-53 exactly, so x = r53*255 is the
// real product scaled by 2^53. The double multiply rounds x to 53 significant bits
// (round-to-nearest-even) before the uint8_t truncation, so replicate that rounding.
static inline int candleRand255(ulong frameSeed, uint index) {
    ulong z = frameSeed ^ (ulong(index) * 0xD1B54A32D192ED03UL);
    ulong x = (candleMix64(z) >> 11) * 255UL;
    ulong t = x >> 53;
    if (t != 0UL) {
        uint sh = 0;
        while ((t >> sh) != 0UL) {
            sh++;
        }
        ulong keep = x >> sh;
        ulong rem = x & ((1UL << sh) - 1UL);
        ulong halfBit = 1UL << (sh - 1);
        if (rem > halfBit || (rem == halfBit && (keep & 1UL) != 0UL)) {
            keep++;
        }
        x = keep << sh;
    }
    return int(x >> 53);
}

// CandleEffect::Update, bit-identical.
static inline void candleUpdate(constant MetalCandleData &d, uint seed,
                                thread uchar &flameprime, thread uchar &flame, thread uchar &wind) {
    if (candleRand255(d.frameSeed, seed) < d.windVariability) {
        wind = uchar(candleRand255(d.frameSeed, seed + 1u));
    }
    if (int(wind) > d.windBaseline) {
        wind--;
    }
    if (flame < 255) {
        flame++;
    }
    if (candleRand255(d.frameSeed, seed + 2u) < (int(wind) >> d.windCalmness)) {
        flame = uchar(candleRand255(d.frameSeed, seed + 3u));
    }
    if (flame > flameprime) {
        if (int(flameprime) < (255 - d.flameAgility)) {
            flameprime += uchar(d.flameAgility);
        }
    } else {
        if (int(flameprime) > d.flameAgility) {
            flameprime -= uchar(d.flameAgility);
        }
    }
}

static inline uchar candlePaletteBlend(uchar a, uchar b, uchar p) {
    return uchar((int(a) * (255 - int(p)) + int(b) * int(p)) / 255);
}

kernel void CandleEffect(constant MetalCandleData &d [[buffer(0)]],
                         device uchar *states        [[buffer(1)]],
                         device uchar4 *result       [[buffer(2)]],
                         uint index                  [[thread_position_in_grid]]) {
    if (index >= d.width * d.height) return;
    uint x = index % d.width;
    uint y = index / d.width;
    uint stateIdx = y * d.maxWid + x;
    if (stateIdx >= d.numStates) return;

    uchar flameprimer = states[stateIdx * 5 + 0];
    uchar flamer = states[stateIdx * 5 + 1];
    uchar wind = states[stateIdx * 5 + 2];
    uchar flameprimeg = states[stateIdx * 5 + 3];
    uchar flameg = states[stateIdx * 5 + 4];

    uint seed = stateIdx * 131101u;
    candleUpdate(d, seed, flameprimer, flamer, wind);
    candleUpdate(d, seed + 4u, flameprimeg, flameg, wind);

    if (flameprimeg > flameprimer) {
        flameprimeg = flameprimer;
    }
    if (flameg > flamer) {
        flameprimeg = flameprimer;
    }

    states[stateIdx * 5 + 0] = flameprimer;
    states[stateIdx * 5 + 1] = flamer;
    states[stateIdx * 5 + 2] = wind;
    states[stateIdx * 5 + 3] = flameprimeg;
    states[stateIdx * 5 + 4] = flameg;

    uchar4 out;
    if (d.usePalette) {
        out.r = candlePaletteBlend(d.c1.r, d.c2.r, flameprimer);
        out.g = candlePaletteBlend(d.c1.g, d.c2.g, flameprimer);
        out.b = candlePaletteBlend(d.c1.b, d.c2.b, flameprimer);
    } else {
        out.r = flameprimer;
        out.g = flameprimeg / 2;
        out.b = 0;
    }
    out.a = 255;
    result[index] = out;
}
