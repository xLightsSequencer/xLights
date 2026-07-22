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

// Garlands effect. Per-pixel inversion of the scalar ring scatter: the ring->pixel
// map is strictly increasing per column (injective), so each pixel computes the one
// ring that covers it and reads that ring's precomputed blend color. All per-pixel
// work is integer math; the colors[] and yb[] tables are precomputed by the caller
// with the exact scalar double math, so this kernel is byte-identical to the scalar
// renderer. invPS / posOffOverPS only seed a candidate window (the integer yb table
// decides exactly), so their float approximation never affects the output.
// Must stay in lockstep with GarlandsFunctions.ispc.
kernel void GarlandsEffect(constant MetalGarlandsData &data,
                           device uchar4* result,
                           device const uchar4* colors,
                           device const int* yb,
                           uint index [[thread_position_in_grid]])
{
    if (index >= (data.width * data.height)) return;

    int width = (int)data.width;
    int buffMax = data.buffMax;
    int type = data.garlandType;
    int dir = data.dir;
    float invPS = data.invPS;
    float posOffOverPS = data.posOffOverPS;

    int bx = (int)index % width;
    int by = (int)index / width;

    // Map the buffer pixel back to (column, ring-axis value v) for this direction.
    int col;
    int v;
    if (dir <= 1) {
        col = bx;
        v = (dir == 0) ? by : (buffMax - by - 1);
    } else {
        col = by;
        v = (dir == 2) ? (buffMax - bx - 1) : bx;
    }

    // Amount subtracted from y for this column, per garland type (x%5 / x%6 pattern).
    int m5 = col % 5;
    int m6 = col % 6;
    int po = 0;
    if (type == 1) {
        if (m5 == 2) po = 2;
        else if (m5 == 1 || m5 == 3) po = 1;
    } else if (type == 2) {
        if (m5 == 2) po = 4;
        else if (m5 == 1 || m5 == 3) po = 2;
    } else if (type == 3) {
        if (m6 == 3) po = 6;
        else if (m6 == 2 || m6 == 4) po = 4;
        else if (m6 == 1 || m6 == 5) po = 2;
    } else if (type == 4) {
        if (m5 == 1 || m5 == 3) po = 2;
    }

    // Analytic seed for the covering ring (float — candidate selection only).
    float rg = ((float)(v + po) - 1.0f) * invPS + posOffOverPS;
    int base = (int)floor(rg);

    // yadj(r) = max(yb[r] - po, r) is strictly increasing in r, so at most one ring
    // maps to v: the clamp region gives r == v, the free region a ring near base.
    // Check both; the integer yb table decides exactly.
    int cands[7] = { v, base - 2, base - 1, base, base + 1, base + 2, base + 3 };
    int foundRing = -1;
    for (int c = 0; c < 7; c++) {
        int r = cands[c];
        if (r >= 0 && r < buffMax) {
            int ya = yb[r] - po;
            if (ya < r) ya = r;
            if (ya == v) foundRing = r;
        }
    }

    if (foundRing >= 0) {
        result[index] = colors[foundRing];
    }
}
