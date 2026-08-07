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

// GPU port of the Fire effect's "New Render Method" (mirrors FireFunctions.ispc
// - keep the integer math in lockstep).
//
// The classic method rebuilds the flame bottom-up within a frame, so row y
// depends on rows the same pass just wrote.  That is strictly serial in y: the
// only way to run it here is a single threadgroup marching rows behind a
// barrier, which measures no faster than vectorising on the CPU.  The new method
// derives row y from the PREVIOUS frame's rows y-1/y-2, so every cell is
// independent and the whole grid is one dispatch.
//
// Everything is integer, so this is bit-identical to the ISPC path - no
// GPU-vs-libm float drift to reconcile.  The grid stays in a device buffer
// between frames; only the drawn pixels leave the GPU.

// RenderBuffer::rngMix64 (splitmix64 finalizer), bit-identical.
static inline ulong fireMix64(ulong z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9UL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBUL;
    return z ^ (z >> 31);
}

// RenderBuffer::hashRandom(index), bit-identical.
static inline uint fireHashRandom(ulong seed, uint index) {
    return uint(fireMix64(seed ^ (ulong(index) * 0xD1B54A32D192ED03UL)) >> 32);
}

// RenderBuffer::randInt(0, range-1)'s multiply-shift mapping on a hashed draw.
static inline uint fireRandBelow(uint r, uint range) {
    return uint((ulong(r) * ulong(range)) >> 32);
}

// One cell of the temporal advance. `prev` is last frame's grid, `next` this
// frame's; ping-ponged by the caller so every cell can run concurrently.
kernel void FireAdvanceEffect(constant MetalFireData &d [[buffer(0)]],
                              device const int *prev [[buffer(1)]],
                              device int *next [[buffer(2)]],
                              uint2 gid [[thread_position_in_grid]]) {
    int W = d.width;
    int H = d.height;
    int x = int(gid.x);
    int y = int(gid.y);
    if (x >= W || y >= H) {
        return;
    }

    if (y == 0) {
        // The base row is reseeded every frame.  Index space x is disjoint from
        // the noise indices below, which are all >= W for y >= 1.
        next[x] = 150 + int(fireRandBelow(fireHashRandom(d.frameSeed, uint(x)), 50));
        return;
    }

    // Drop the out-of-grid taps exactly as GetFireBuffer's -1 return does.
    int base = (y - 1) * W;
    int sum = prev[base + x];
    int n = 1;
    if (x > 0) {
        sum += prev[base + x - 1];
        n += 1;
    }
    if (x < W - 1) {
        sum += prev[base + x + 1];
        n += 1;
    }
    if (y >= 2) {
        sum += prev[base - W + x];
        n += 1;
    }
    int ni = sum / n;
    if (ni > 0) {
        uint r = fireHashRandom(d.frameSeed, uint(y * W + x));
        ni += fireRandBelow(r, 100) < 20 ? d.step : -d.step;
        ni = clamp(ni, 0, 199);
    }
    next[y * W + x] = ni;
}

// Rasterises the flame grid through the per-frame palette table, applying the
// Location transform.  Mirrors DrawFire(): one thread per drawn pixel.
kernel void FireDrawEffect(constant MetalFireData &d [[buffer(0)]],
                           device const int *grid [[buffer(1)]],
                           device const uchar4 *lut [[buffer(2)]],
                           device uchar4 *result [[buffer(3)]],
                           uint2 gid [[thread_position_in_grid]]) {
    int x = int(gid.x);
    int y = int(gid.y);
    if (x >= d.curWi || y >= d.curHt) {
        return;
    }

    int xp = x;
    int yp = y;
    if (d.loc == 1 || d.loc == 3) {
        yp = d.curHt - y - 1;
    }
    if (d.loc == 2 || d.loc == 3) {
        int t = xp;
        xp = yp;
        yp = t;
    }

    // GetFireBuffer returns -1 outside the grid, which the drawn area can reach
    // when the effect's max buffer is smaller than the current one.
    int idx = (x >= 0 && x < d.width && y >= 0 && y < d.height) ? grid[y * d.width + x] : 0;

    int pix = yp * d.bufferWi + xp;
    if (xp >= 0 && xp < d.bufferWi && yp >= 0 && yp < d.bufferHt && pix < d.npix) {
        result[pix] = lut[idx];
    }
}
