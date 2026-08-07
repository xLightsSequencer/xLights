/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Garlands effect. The scalar renderer scatters one ring at
// a time (last ring wins). The ring->pixel map is strictly increasing per column,
// so it is injective and invertible: each pixel here computes the single ring that
// covers it (if any) and reads that ring's precomputed blend color.
//
// All per-pixel work is integer math. The two per-ring tables are precomputed by
// the caller with the exact scalar double math: colors[] (the GetMultiColorBlend
// result per ring) and yb[] (the truncated y = (int)(1 + ring*PixelSpacing -
// positionOffset)). So this path is byte-identical to the scalar renderer.
// invPS / posOffOverPS are only used to seed a small candidate window and never
// affect the output (the integer yb table is the source of truth), so a float
// approximation of the double math is fine there.
// Must stay in lockstep with GarlandsFunctions.metal.

struct GarlandsData {
    unsigned int width;        // BufferWi
    unsigned int height;       // BufferHt
    int          buffMax;      // ring count (BufferHt for Up/Down, BufferWi for Left/Right)
    int          garlandType;  // 0..4
    int          dir;          // 0..3 (Up/Down/Left/Right, after Up-then-Down folding)
    float        invPS;        // 1 / PixelSpacing
    float        posOffOverPS; // positionOffset / PixelSpacing
};

// Amount subtracted from y for column x, per garland type (the x%5 / x%6 pattern).
static inline int garlandOffset(uniform int type, int x) {
    int off = 0;
    if (type == 1) {
        int m = x % 5;
        if (m == 2) off = 2;
        else if (m == 1 || m == 3) off = 1;
    } else if (type == 2) {
        int m = x % 5;
        if (m == 2) off = 4;
        else if (m == 1 || m == 3) off = 2;
    } else if (type == 3) {
        int m = x % 6;
        if (m == 3) off = 6;
        else if (m == 2 || m == 4) off = 4;
        else if (m == 1 || m == 5) off = 2;
    } else if (type == 4) {
        int m = x % 5;
        if (m == 1 || m == 3) off = 2;
    }
    return off;
}

export void GarlandsEffectISPC(const uniform GarlandsData * uniform data,
                               uniform int startIdx,
                               uniform int endIdx,
                               const uniform uint8<4> * uniform colors,
                               const uniform int * uniform yb,
                               uniform uint8<4> * uniform result) {
    uniform int   width        = (int)data->width;
    uniform int   buffMax       = data->buffMax;
    uniform int   type         = data->garlandType;
    uniform int   dir          = data->dir;
    uniform float invPS        = data->invPS;
    uniform float posOffOverPS = data->posOffOverPS;

    foreach (index = startIdx ... endIdx) {
        int bx = index % width;
        int by = index / width;

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

        int po = garlandOffset(type, col);

        // Analytic seed for the covering ring (float — candidate selection only).
        float rg = ((float)(v + po) - 1.0f) * invPS + posOffOverPS;
        int base = (int)floor(rg);

        // yadj(r) = max(yb[r] - po, r) is strictly increasing in r, so at most one
        // ring maps to v: the clamp region gives r == v, the free region a ring near
        // base. Check both; the integer yb table decides exactly.
        int cands[7];
        cands[0] = v;
        cands[1] = base - 2;
        cands[2] = base - 1;
        cands[3] = base;
        cands[4] = base + 1;
        cands[5] = base + 2;
        cands[6] = base + 3;

        int foundRing = -1;
        for (uniform int c = 0; c < 7; c++) {
            int r = cands[c];
            if (r >= 0 && r < buffMax) {
                int gi = min(r, buffMax - 1); // no-op clamp; guards an ispc gather miscompile
                int ya = yb[gi] - po;
                if (ya < r) ya = r;
                if (ya == v) foundRing = r;
            }
        }

        if (foundRing >= 0) {
            int gi = min(foundRing, buffMax - 1); // no-op clamp; guards an ispc gather miscompile
            result[index] = colors[gi];
        }
    }
}
