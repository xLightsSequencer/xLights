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

// GPU port of the Twinkle effect per-light strobe (mirrors TwinkleFunctions.ispc
// — keep the two in lockstep). Each thread owns one strobe entry in the state
// array: it advances the per-light state (duration++, the New-Method finish
// flag, or the reRandomize wrap) and writes its unique pixel from a CPU-built
// RGBA LUT. All per-pixel work is integer, and the brightness/color LUT is
// precomputed on the CPU with the exact scalar double math, so this path is
// byte-identical to the historical scalar renderer.
//
// The state array is a flat int32 array with a stride of 6 (matching
// StrobeClass): [x, y, duration, colorindex, strobing, isByNode]. It is
// read-modify-written in place and copied back to the CPU cache after dispatch.

// RenderBuffer::rngMix64 (splitmix64 finalizer), bit-identical.
static inline ulong twinkleMix64(ulong z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9UL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBUL;
    return z ^ (z >> 31);
}

// Bit-identical emulation of RenderBuffer::hashRandom(index):
// mix64(frameSeed ^ (uint64(index) * 0xD1B54A32D192ED03)) >> 32.
static inline uint twinkleHashRandom(ulong frameSeed, uint index) {
    ulong z = frameSeed ^ (ulong(index) * 0xD1B54A32D192ED03UL);
    return uint(twinkleMix64(z) >> 32);
}

kernel void TwinkleEffect(constant MetalTwinkleData &d [[buffer(0)]],
                          device int *states           [[buffer(1)]],
                          device const uchar4 *lut     [[buffer(2)]],
                          device uchar4 *result        [[buffer(3)]],
                          uint si                      [[thread_position_in_grid]]) {
    if (si >= d.curNumStrobe) return;

    int base = int(si) * 6;
    int sx = states[base + 0];
    int sy = states[base + 1];
    int duration = states[base + 2];
    int colorindex = states[base + 3];
    int strobing = states[base + 4];

    int max_modulo = d.max_modulo;
    uint umax_modulo2 = uint(d.max_modulo2);
    uint ucolorcnt = uint(d.colorcnt);

    int dnext = duration + 1;
    bool skip = (d.new_algorithm != 0 && strobing == 0) || (dnext < 0);
    if (!skip) {
        if (dnext == max_modulo) {
            dnext = 0;
            if (d.new_algorithm != 0) {
                strobing = 0;
            } else if (d.reRandomize != 0) {
                uint k = si * 131101u;
                dnext = dnext - int(twinkleHashRandom(d.frameSeed, k) % umax_modulo2);
                colorindex = int(twinkleHashRandom(d.frameSeed, k + 1u) % ucolorcnt);
            }
        }
        int i7 = dnext;
        int cl = i7 < 0 ? 0 : (i7 > max_modulo ? max_modulo : i7);
        int lutIdx = colorindex * d.lutStride + cl;
        if (lutIdx < 0) {
            lutIdx = 0;
        }
        if (lutIdx >= d.lutSize) {
            lutIdx = d.lutSize - 1;
        }
        int pidx = sy * int(d.width) + sx;
        if (pidx >= 0 && pidx < int(d.npix)) {
            result[pidx] = lut[lutIdx];
        }
    }

    states[base + 2] = dnext;
    states[base + 3] = colorindex;
    states[base + 4] = strobing;
}
