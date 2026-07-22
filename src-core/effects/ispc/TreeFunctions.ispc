/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Tree effect. All per-pixel work is integer math; the two
// color tables (background brightness per position-in-branch, garland light per
// branch row) are precomputed by the caller with the exact scalar double-precision
// color math, so this path is byte-identical to the scalar renderer.
// Must stay in lockstep with TreeFunctions.metal.

#define MAX_TREE_ISPC_PPB 512

struct TreeISPCData {
    unsigned int width;
    unsigned int height;
    int          ppb;         // pixels_per_branch
    int          frame;       // light sweep limit (x <= frame)
    int          branch_row;  // b — branch row currently being lit
    int          f_mod;       // sweep position within the row
    int          showlights;
    uint8<4>     bgColors[MAX_TREE_ISPC_PPB]; // indexed by mod-1, mod in 1..ppb
    uint8<4>     lightColors[5];              // indexed by branch % 5
};

export void TreeEffectISPC(const uniform TreeISPCData * uniform data,
                           uniform int startIdx,
                           uniform int endIdx,
                           uniform uint8<4> * uniform result) {
    uniform int width = (int)data->width;
    uniform int ppb = data->ppb;
    uniform int b = data->branch_row;
    uniform int odd_even = b % 2;

    foreach (index = startIdx ... endIdx) {
        int x = index % width;
        int y = index / width;

        int mod = y % ppb;
        if (mod == 0)
            mod = ppb;
        uint8<4> color = data->bgColors[mod - 1];

        if (data->showlights) {
            int branch = (y - 1) / ppb;
            int row = ppb - mod;
            int m = x % 6;
            if (m == 0)
                m = 6;
            if (branch <= b && x <= data->frame &&
                ((row == 3 && (m == 1 || m == 6)) ||
                 (row == 2 && (m == 2 || m == 5)) ||
                 (row == 1 && (m == 3 || m == 4)))) {
                int s_odd_row = width - x + 1;
                if ((odd_even == 0 && x <= data->f_mod) || (odd_even == 1 && s_odd_row <= data->f_mod)) {
                    int r = branch % 5;
                    if (r < 0)
                        r = 0; // branch is -1 only when y==0 && ppb==1; hue clamps to red
                    color = data->lightColors[r];
                }
            }
        }
        result[index] = color;
    }
}
