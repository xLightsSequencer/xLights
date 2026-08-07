/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// CPU path for the Fire effect's "New Render Method" (mirrors
// FireFunctions.metal - keep the integer math in lockstep).
//
// The classic method rebuilds the flame grid bottom-up every frame, so row y
// depends on rows this same pass just wrote: strictly serial in y, with no
// useful parallelism to exploit.  The new method instead derives row y from the
// PREVIOUS frame's rows y-1/y-2, which makes every row independent given last
// frame's grid - so the x loop vectorises here and the whole grid becomes one
// dispatch on the GPU.
//
// Walking y downward keeps that correct in a single buffer: a row is only
// overwritten once the rows above have already consumed it.  Row 0 is reseeded
// last for the same reason.

struct FireData {
    int width;                // grid stride (maxMWi)
    int height;               // grid rows (maxMHt)
    int step;                 // per-row brightness change
    int maxRow;               // rows [maxRow, height) are already all zero
    unsigned int64 frameSeed; // RenderBuffer::hashRandomFrameSeed()
};

// Bit-identical local copies of RenderBuffer::rngMix64/hashRandom
// (RenderBuffer.h), plus randInt(0, range-1)'s multiply-shift mapping applied to
// a hashed draw instead of the serial stream.  Both variabilities are needed:
// the interior runs varying, the peeled edge columns run uniform.
static inline unsigned int64 fireMix64(unsigned int64 z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EB;
    return z ^ (z >> 31);
}
static inline uniform unsigned int64 fireMix64(uniform unsigned int64 z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EB;
    return z ^ (z >> 31);
}

static inline unsigned int32 fireHashRandom(unsigned int64 seed, unsigned int32 index) {
    return (unsigned int32)(fireMix64(seed ^ ((unsigned int64)index * 0xD1B54A32D192ED03)) >> 32);
}
static inline uniform unsigned int32 fireHashRandom(uniform unsigned int64 seed, uniform unsigned int32 index) {
    return (uniform unsigned int32)(fireMix64(seed ^ ((uniform unsigned int64)index * 0xD1B54A32D192ED03)) >> 32);
}

static inline unsigned int32 fireRandBelow(unsigned int32 r, unsigned int32 range) {
    return (unsigned int32)(((unsigned int64)r * (unsigned int64)range) >> 32);
}
static inline uniform unsigned int32 fireRandBelow(uniform unsigned int32 r, uniform unsigned int32 range) {
    return (uniform unsigned int32)(((uniform unsigned int64)r * (uniform unsigned int64)range) >> 32);
}

// One grid cell the general way, dropping the out-of-grid taps exactly as
// GetFireBuffer's -1 return does in the scalar path.  Used for the two edge
// columns and for row 1, where the tap count differs from the interior.
static inline uniform int fireCell(const uniform FireData* uniform d,
                                   const uniform int grid[],
                                   uniform int x, uniform int y) {
    uniform int W = d->width;
    uniform int base = (y - 1) * W;
    uniform int sum = grid[base + x];
    uniform int n = 1;
    if (x > 0) {
        sum += grid[base + x - 1];
        n += 1;
    }
    if (x < W - 1) {
        sum += grid[base + x + 1];
        n += 1;
    }
    if (y >= 2) {
        sum += grid[base - W + x];
        n += 1;
    }
    uniform int ni = sum / n;
    if (ni > 0) {
        uniform unsigned int32 r = fireHashRandom(d->frameSeed, (uniform unsigned int32)(y * W + x));
        ni += fireRandBelow(r, 100) < 20 ? d->step : -d->step;
        ni = clamp(ni, 0, 199);
    }
    return ni;
}

// Advances the grid one frame. Returns the highest row that ended up non-zero,
// so the caller can bound the next frame's work (the flame front climbs at most
// one row per frame).
export uniform int FireAdvanceTemporalISPC(const uniform FireData* uniform d,
                                           uniform int grid[]) {
    uniform int W = d->width;
    uniform int step = d->step;
    uniform int highest = 0;

    uniform int top = d->maxRow < d->height ? d->maxRow : d->height;
    for (uniform int y = top - 1; y >= 1; --y) {
        uniform int base = (y - 1) * W;
        uniform int prev2 = base - W;
        uniform int row = y * W;
        uniform bool nonZero = false;

        if (y == 1) {
            // Row 1 has no (x, y-2) tap, so its divisor differs from the
            // interior's constant 4.  It is a single row - do it uniformly
            // rather than dragging a varying integer divide into the hot loop.
            for (uniform int x = 0; x < W; ++x) {
                uniform int ni = fireCell(d, grid, x, 1);
                grid[row + x] = ni;
                if (ni != 0) {
                    nonZero = true;
                }
            }
        } else {
            // Edge columns peeled: they have 3 taps, the interior has 4, so the
            // interior divide becomes an exact >> 2.
            uniform int e0 = fireCell(d, grid, 0, y);
            uniform int e1 = W > 1 ? fireCell(d, grid, W - 1, y) : e0;
            nonZero = (e0 != 0) || (e1 != 0);

            foreach (x = 1 ... W - 1) {
                int sum = grid[base + x - 1] + grid[base + x] + grid[base + x + 1] + grid[prev2 + x];
                int ni = sum >> 2; // sum is a sum of palette indices, never negative
                if (ni > 0) {
                    unsigned int32 r = fireHashRandom(d->frameSeed, (unsigned int32)(row + x));
                    ni += fireRandBelow(r, 100) < 20 ? step : -step;
                    ni = clamp(ni, 0, 199);
                }
                grid[row + x] = ni;
                if (any(ni != 0)) {
                    nonZero = true;
                }
            }
            grid[row] = e0;
            grid[row + W - 1] = e1;
        }

        if (nonZero && highest == 0) {
            highest = y;
        }
    }

    // Reseed the base row last - rows 1 and 2 read it above.
    foreach (x = 0 ... W) {
        grid[x] = 150 + (int)fireRandBelow(fireHashRandom(d->frameSeed, (unsigned int32)x), 50);
    }
    return highest;
}
