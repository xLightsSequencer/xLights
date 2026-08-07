/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Candle perNode flame simulation (mirrors CandleFunctions.metal —
// keep the math in lockstep). Each lane owns one pixel/node: it runs both Update()
// passes (red then green, sharing the wind byte) against the persistent 5-byte
// CandleState and writes the pixel. The state array is read-modify-write in place so
// the CPU-side cache stays the single source of truth.

struct CandleISPCData {
    unsigned int32 width;
    unsigned int32 height;
    unsigned int32 maxWid;
    unsigned int32 numStates;
    unsigned int64 frameSeed;
    int windVariability;
    int flameAgility;
    int windCalmness;
    int windBaseline;
    int usePalette;
    unsigned int8 c1r, c1g, c1b;
    unsigned int8 c2r, c2g, c2b;
};

// RenderBuffer::rngMix64 (splitmix64 finalizer), bit-identical.
static inline unsigned int64 candleMix64(unsigned int64 z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

// Bit-identical integer emulation of uint8_t(RenderBuffer::hashRand01(index) * 255.0):
// hashRand01 = (mix64(seed ^ index*K) >> 11) * 2^-53 exactly, so x = r53*255 is the
// real product scaled by 2^53. The double multiply rounds x to 53 significant bits
// (round-to-nearest-even) before the uint8_t truncation, so replicate that rounding.
static inline int candleRand255(unsigned int64 frameSeed, unsigned int32 index) {
    unsigned int64 z = frameSeed ^ ((unsigned int64)index * 0xD1B54A32D192ED03ull);
    unsigned int64 x = (candleMix64(z) >> 11) * 255ull;
    unsigned int64 t = x >> 53;
    if (t != 0) {
        int sh = 0;
        while ((t >> sh) != 0) {
            sh = sh + 1;
        }
        unsigned int64 keep = x >> sh;
        unsigned int64 rem = x & ((1ull << sh) - 1);
        unsigned int64 halfBit = 1ull << (sh - 1);
        if (rem > halfBit || (rem == halfBit && (keep & 1) != 0)) {
            keep = keep + 1;
        }
        x = keep << sh;
    }
    return (int)(x >> 53);
}

// CandleEffect::Update, bit-identical. All state mutations use explicit
// assignment form: this ispc (1.30.0) silently drops `--` on a varying
// unsigned int8 (masked or not), so `wind--` would never happen.
static inline void candleUpdate(const uniform CandleISPCData* uniform d, unsigned int32 seed,
                                unsigned int8& flameprime, unsigned int8& flame, unsigned int8& wind) {
    if (candleRand255(d->frameSeed, seed) < d->windVariability) {
        wind = (unsigned int8)candleRand255(d->frameSeed, seed + 1);
    }
    if ((int)wind > d->windBaseline) {
        wind = wind - 1;
    }
    if (flame < 255) {
        flame = flame + 1;
    }
    if (candleRand255(d->frameSeed, seed + 2) < ((int)wind >> d->windCalmness)) {
        flame = (unsigned int8)candleRand255(d->frameSeed, seed + 3);
    }
    if (flame > flameprime) {
        if ((int)flameprime < (255 - d->flameAgility)) {
            flameprime = flameprime + (unsigned int8)d->flameAgility;
        }
    } else {
        if ((int)flameprime > d->flameAgility) {
            flameprime = flameprime - (unsigned int8)d->flameAgility;
        }
    }
}

static inline unsigned int8 candlePaletteBlend(unsigned int8 a, unsigned int8 b, unsigned int8 p) {
    return (unsigned int8)(((int)a * (255 - (int)p) + (int)b * (int)p) / 255);
}

export void CandleEffectISPC(const uniform CandleISPCData* uniform d,
                             uniform int startIdx, uniform int endIdx,
                             uniform unsigned int8* uniform states,
                             uniform uint8<4>* uniform result) {
    uniform int width = d->width;
    uniform int maxWid = d->maxWid;
    uniform unsigned int32 numStates = d->numStates;

    foreach (index = startIdx ... endIdx) {
        int x = index % width;
        int y = index / width;
        unsigned int32 stateIdx = (unsigned int32)(y * maxWid + x);
        if (stateIdx < numStates) {
            unsigned int8 flameprimer = states[stateIdx * 5 + 0];
            unsigned int8 flamer = states[stateIdx * 5 + 1];
            unsigned int8 wind = states[stateIdx * 5 + 2];
            unsigned int8 flameprimeg = states[stateIdx * 5 + 3];
            unsigned int8 flameg = states[stateIdx * 5 + 4];

            unsigned int32 seed = stateIdx * 131101u;
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

            uint8<4> out;
            if (d->usePalette) {
                out[0] = candlePaletteBlend(d->c1r, d->c2r, flameprimer);
                out[1] = candlePaletteBlend(d->c1g, d->c2g, flameprimer);
                out[2] = candlePaletteBlend(d->c1b, d->c2b, flameprimer);
            } else {
                out[0] = flameprimer;
                out[1] = flameprimeg / 2;
                out[2] = 0;
            }
            out[3] = 255;
            result[index] = out;
        }
    }
}
