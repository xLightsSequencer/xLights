/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Twinkle effect per-light strobe (mirrors
// TwinkleFunctions.metal — keep the two in lockstep). Each lane owns one strobe
// entry in the state array: it advances the per-light state (duration++, the
// New-Method finish flag, or the reRandomize wrap) and writes its unique pixel
// from a CPU-built RGBA LUT. All per-pixel work is integer, and the brightness/
// color LUT is precomputed on the CPU with the exact scalar double math, so this
// path is byte-identical to the historical scalar renderer.
//
// The state array is a flat int32 array with a stride of 6 (matching
// StrobeClass): [x, y, duration, colorindex, strobing, isByNode]. It is
// read-modify-written in place so the CPU-side cache stays the single source of
// truth.

struct TwinkleISPCData {
    unsigned int width;      // BufferWi (pixel-index stride)
    unsigned int npix;       // pixel-write bound (GetPixelCount())
    int max_modulo;
    int max_modulo2;
    int colorcnt;
    int lutStride;           // max_modulo + 1
    int lutSize;             // colorcnt * lutStride
    int new_algorithm;
    int reRandomize;
    unsigned int64 frameSeed;
};

// RenderBuffer::rngMix64 (splitmix64 finalizer), bit-identical.
static inline unsigned int64 twinkleMix64(unsigned int64 z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

// Bit-identical emulation of RenderBuffer::hashRandom(index):
// mix64(frameSeed ^ (uint64(index) * 0xD1B54A32D192ED03)) >> 32.
static inline unsigned int twinkleHashRandom(unsigned int64 frameSeed, unsigned int index) {
    unsigned int64 z = frameSeed ^ ((unsigned int64)index * 0xD1B54A32D192ED03ull);
    return (unsigned int)(twinkleMix64(z) >> 32);
}

export void TwinkleEffectISPC(const uniform TwinkleISPCData * uniform d,
                              uniform int startIdx,
                              uniform int endIdx,
                              uniform int * uniform states,
                              const uniform uint8<4> * uniform lut,
                              uniform uint8<4> * uniform result) {
    uniform int width = (int)d->width;
    uniform int npix = (int)d->npix;
    uniform int max_modulo = d->max_modulo;
    uniform unsigned int umax_modulo2 = (unsigned int)d->max_modulo2;
    uniform unsigned int ucolorcnt = (unsigned int)d->colorcnt;
    uniform int lutStride = d->lutStride;
    uniform int lutSize = d->lutSize;

    foreach (si = startIdx ... endIdx) {
        int base = si * 6;
        int sx = states[base + 0];
        int sy = states[base + 1];
        int duration = states[base + 2];
        int colorindex = states[base + 3];
        int strobing = states[base + 4];

        // ispc (1.30.0) silently drops `++`/`--` on varying ints under a mask,
        // so use explicit assignment form for every state mutation.
        int dnext = duration + 1;
        bool skip = (d->new_algorithm != 0 && strobing == 0) || (dnext < 0);
        if (!skip) {
            if (dnext == max_modulo) {
                dnext = 0;
                if (d->new_algorithm != 0) {
                    strobing = 0;
                } else if (d->reRandomize != 0) {
                    unsigned int k = (unsigned int)si * 131101u;
                    dnext = dnext - (int)(twinkleHashRandom(d->frameSeed, k) % umax_modulo2);
                    colorindex = (int)(twinkleHashRandom(d->frameSeed, k + 1u) % ucolorcnt);
                }
            }
            int i7 = dnext;
            int cl = i7 < 0 ? 0 : (i7 > max_modulo ? max_modulo : i7);
            int lutIdx = colorindex * lutStride + cl;
            if (lutIdx < 0) {
                lutIdx = 0;
            }
            if (lutIdx >= lutSize) {
                lutIdx = lutSize - 1;
            }
            int pidx = sy * width + sx;
            if (pidx >= 0 && pidx < npix) {
                result[pidx] = lut[lutIdx];
            }
        }

        states[base + 2] = dnext;
        states[base + 3] = colorindex;
        states[base + 4] = strobing;
    }
}
