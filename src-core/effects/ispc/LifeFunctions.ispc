/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for Conway's Game of Life (mirrors LifeFunctions.metal — keep the
// math in lockstep). Each lane owns one pixel/cell: it reads its 8 wrapped
// neighbours from the PREVIOUS generation (a read-only copy of the RenderBuffer
// TempBuf), applies the birth/survival rule for the selected ruleset, and writes
// the new generation. Newly-born cells get their colour from
// GetMultiColorBlend(hashRand01(index)), replicated bit-identically here so the
// ISPC/Metal/scalar renders agree. Pixels are packed RGBA uint32 (little-endian:
// byte0=red .. byte3=alpha), matching xlColor's memory layout.

struct LifeISPCData {
    unsigned int32 width;
    unsigned int32 height;
    int npix;         // min(GetPixelCount(), width*height); wrapped neighbours whose
                      // linear index >= npix count as black (matches the scalar's
                      // GetTempPixelRGB bounds guard) and bound the prev/result buffers
    int type;         // ruleset 0..4
    int numColors;    // palette.Size() (>= 1)
    unsigned int64 frameSeed; // RenderBuffer::hashRandomFrameSeed() for this frame
};

// RenderBuffer::rngMix64 (splitmix64 finalizer), bit-identical.
static inline unsigned int64 lifeMix64(unsigned int64 z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

// RenderBuffer::hashRand01(index) cast to float (GetMultiColorBlend takes a float).
// hashRand01 = (mix64(frameSeed ^ index*K) >> 11) * 2^-53 in double; the double is
// then narrowed to float at the call boundary. Computed here in double to mirror
// the scalar exactly.
static inline float lifeHashRand01f(unsigned int64 frameSeed, unsigned int32 index) {
    unsigned int64 z = frameSeed ^ ((unsigned int64)index * 0xD1B54A32D192ED03ull);
    unsigned int64 r53 = lifeMix64(z) >> 11;
    double hr = (double)r53 * (1.0d / 9007199254740992.0d);
    return (float)hr;
}

// RenderBuffer::ChannelBlend, bit-identical: c1 + floor(ratio*(c2-c1) + 0.5).
// The product is a float (float ratio * int channel delta) and the +0.5/floor run
// in double, exactly as the scalar promotes the float product before adding 0.5 —
// a float-multiply feeding a double-add cannot be fused into an FMA, so no rounding
// is skipped.
static inline unsigned int8 lifeChannelBlend(unsigned int8 a, unsigned int8 b, float ratio) {
    float m = ratio * (float)((int)b - (int)a);
    double v = (double)m + 0.5d;
    double f = floor(v);
    return (unsigned int8)((int)a + (int)f);
}

// RenderBuffer::GetMultiColorBlend(hashRand01(index), circular=false), returning
// the packed RGBA birth colour (alpha 255).
static inline unsigned int lifeBirthColor(const uniform LifeISPCData* uniform d,
                                          const uniform unsigned int* uniform palette,
                                          unsigned int32 index) {
    int colorcnt = d->numColors;
    if (colorcnt <= 1) {
        // GetMultiColorBlend returns GetColor(0) verbatim (its own alpha) here;
        // only the 2-colour blend below forces alpha 255.
        return palette[0];
    }
    float nf = lifeHashRand01f(d->frameSeed, index);
    if (nf >= 1.0f) nf = 0.99999f;
    if (nf < 0.0f) nf = 0.0f;
    float realidx = nf * (float)(colorcnt - 1);
    int idx1 = (int)floor(realidx);
    int idx2 = (idx1 + 1) % colorcnt;
    float ratio = realidx - (float)idx1;

    idx1 = min(idx1, colorcnt - 1); // no-op guard before the gathers
    idx2 = min(idx2, colorcnt - 1);
    unsigned int c1 = palette[idx1];
    unsigned int c2 = palette[idx2];

    unsigned int8 r1 = (unsigned int8)(c1 & 0xFFu);
    unsigned int8 g1 = (unsigned int8)((c1 >> 8) & 0xFFu);
    unsigned int8 b1 = (unsigned int8)((c1 >> 16) & 0xFFu);
    unsigned int8 r2 = (unsigned int8)(c2 & 0xFFu);
    unsigned int8 g2 = (unsigned int8)((c2 >> 8) & 0xFFu);
    unsigned int8 b2 = (unsigned int8)((c2 >> 16) & 0xFFu);

    unsigned int outr = (unsigned int)lifeChannelBlend(r1, r2, ratio);
    unsigned int outg = (unsigned int)lifeChannelBlend(g1, g2, ratio);
    unsigned int outb = (unsigned int)lifeChannelBlend(b1, b2, ratio);
    return 0xFF000000u | (outb << 16) | (outg << 8) | outr;
}

static inline bool lifeIsLive(unsigned int c) {
    return (c & 0x00FFFFFFu) != 0u;
}

export void LifeEffectISPC(const uniform LifeISPCData* uniform d,
                           uniform int startIdx, uniform int endIdx,
                           const uniform unsigned int* uniform prev,
                           const uniform unsigned int* uniform palette,
                           uniform unsigned int* uniform result) {
    uniform int width = (int)d->width;
    uniform int height = (int)d->height;
    uniform int type = d->type;
    uniform int npix = d->npix;

    // Neighbour offsets (match Life_CountNeighbors):
    //     2   3   4
    //     1   X   5
    //     0   7   6
    const uniform int n_x[8] = { -1, -1, -1, 0, 1, 1, 1, 0 };
    const uniform int n_y[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };

    foreach (index = startIdx ... endIdx) {
        int x0 = index % width;
        int y0 = index / width;

        int gi = min(index, npix - 1);
        unsigned int selfc = prev[gi];
        bool live = lifeIsLive(selfc);

        int cnt = 0;
        for (uniform int i = 0; i < 8; ++i) {
            int nx = (x0 + n_x[i]) % width;
            int ny = (y0 + n_y[i]) % height;
            if (nx < 0) nx += width;
            if (ny < 0) ny += height;
            int nidx = ny * width + nx;
            // Out-of-allocation neighbours read as black (scalar GetTempPixelRGB
            // bounds guard); clamp the gather index so the read is always in range.
            int rd = min(nidx, npix - 1);
            if (nidx < npix && lifeIsLive(prev[rd])) ++cnt;
        }

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

        unsigned int out = 0u;
        if (survive) {
            out = selfc;
        } else if (birth) {
            out = lifeBirthColor(d, palette, (unsigned int32)index);
        }
        result[gi] = out;
    }
}
