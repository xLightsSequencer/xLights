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

// LUT modes — must match ShimmerEffect::ShimmerLutMode and ShimmerFunctions.ispc.
#define SHIMMER_LUT_FLAT 0
#define SHIMMER_LUT_X 1
#define SHIMMER_LUT_Y 2
#define SHIMMER_LUT_RANDOM 3

// Bit-identical local copy of RenderBuffer::rngMix64/hashRandom (RenderBuffer.h);
// frameSeed is RenderBuffer::hashRandomFrameSeed() (base seed with curPeriod mixed in).
static inline ulong shimmerMix64(ulong z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9UL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBUL;
    return z ^ (z >> 31);
}

static inline uint shimmerHashRandom(ulong seed, uint index) {
    return uint(shimmerMix64(seed ^ (ulong(index) * 0xD1B54A32D192ED03UL)) >> 32);
}

// Shimmer effect — per-pixel kernel. The per-frame duty-cycle/color selection is
// done on the CPU (see MetalShimmerEffect.mm); every "on" frame fills the whole
// buffer from a CPU-built color LUT. Kept in lockstep with ShimmerFunctions.ispc
// so CPU/ISPC/Metal render bit-identically.
kernel void ShimmerEffect(constant MetalShimmerData &data [[buffer(0)]],
                          device uchar4 *result            [[buffer(1)]],
                          constant uchar4 *lut             [[buffer(2)]],
                          uint index                       [[thread_position_in_grid]])
{
    int lutIdx = 0;
    if (data.lutMode == SHIMMER_LUT_RANDOM) {
        // randInt-style fixed-point mapping of the hash to a palette index
        uint r = shimmerHashRandom(data.frameSeed, index);
        lutIdx = int((ulong(r) * uint(data.colorCount)) >> 32);
    } else if (data.lutMode == SHIMMER_LUT_X) {
        lutIdx = index % data.width;
    } else if (data.lutMode == SHIMMER_LUT_Y) {
        lutIdx = index / data.width;
    }
    result[index] = lut[lutIdx];
}
