/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Shimmer effect. The per-frame duty-cycle/color selection
// stays on the CPU (ShimmerEffect::CalcFrameState); every "on" frame fills the
// whole buffer from a CPU-built color LUT. Kept in lockstep with
// ShimmerFunctions.metal so CPU/ISPC/Metal render bit-identically.

// LUT modes — must match ShimmerEffect::ShimmerLutMode and ShimmerFunctions.metal.
#define SHIMMER_LUT_FLAT 0
#define SHIMMER_LUT_X 1
#define SHIMMER_LUT_Y 2
#define SHIMMER_LUT_RANDOM 3

struct ShimmerData {
    unsigned int width;
    unsigned int height;
    int lutMode;
    int colorCount;
    unsigned int64 frameSeed;
};

// Bit-identical local copy of RenderBuffer::rngMix64/hashRandom (RenderBuffer.h);
// frameSeed is RenderBuffer::hashRandomFrameSeed() (base seed with curPeriod mixed in).
static inline unsigned int64 shimmerMix64(unsigned int64 z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EB;
    return z ^ (z >> 31);
}

static inline unsigned int32 shimmerHashRandom(unsigned int64 seed, unsigned int32 index) {
    return (unsigned int32)(shimmerMix64(seed ^ ((unsigned int64)index * 0xD1B54A32D192ED03)) >> 32);
}

export void ShimmerEffectISPC(const uniform ShimmerData* uniform data,
                              uniform int startIdx,
                              uniform int endIdx,
                              const uniform uint8<4>* uniform lut,
                              uniform uint8<4>* uniform result)
{
    foreach (index = startIdx ... endIdx) {
        int lutIdx = 0;
        if (data->lutMode == SHIMMER_LUT_RANDOM) {
            // randInt-style fixed-point mapping of the hash to a palette index
            unsigned int32 r = shimmerHashRandom(data->frameSeed, (unsigned int32)index);
            lutIdx = (int)(((unsigned int64)r * (unsigned int32)data->colorCount) >> 32);
            // Mathematically a no-op ((r * n) >> 32 < n), but required: without an
            // opaque op between the 64-bit hash math and the gather below, ISPC's
            // gather-index analysis hits an assert and the compiler crashes
            // (ispc 1.30/1.31, llvmutil.cpp:893; https://github.com/ispc/ispc/issues/3881).
            lutIdx = min(lutIdx, data->colorCount - 1);
        } else if (data->lutMode == SHIMMER_LUT_X) {
            lutIdx = index % data->width;
        } else if (data->lutMode == SHIMMER_LUT_Y) {
            lutIdx = index / data->width;
        }
        result[index] = lut[lutIdx];
    }
}
