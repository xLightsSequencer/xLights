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

// Tree effect. All per-pixel work is integer math; the color tables are
// precomputed by the caller with the exact scalar double-precision color math,
// so this kernel is byte-identical to the scalar renderer.
// Must stay in lockstep with TreeFunctions.ispc.
kernel void TreeEffect(constant MetalTreeData &data,
                       device uchar4* result,
                       uint index [[thread_position_in_grid]])
{
    if (index >= (data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;
    int ppb = data.ppb;

    int mod = y % ppb;
    if (mod == 0)
        mod = ppb;
    uchar4 color = data.bgColors[mod - 1];

    if (data.showlights) {
        int b = data.branch_row;
        int branch = (y - 1) / ppb;
        int row = ppb - mod;
        int m = x % 6;
        if (m == 0)
            m = 6;
        if (branch <= b && x <= data.frame &&
            ((row == 3 && (m == 1 || m == 6)) ||
             (row == 2 && (m == 2 || m == 5)) ||
             (row == 1 && (m == 3 || m == 4)))) {
            int odd_even = b % 2;
            int s_odd_row = (int)data.width - x + 1;
            if ((odd_even == 0 && x <= data.f_mod) || (odd_even == 1 && s_odd_row <= data.f_mod)) {
                int r = branch % 5;
                if (r < 0)
                    r = 0; // branch is -1 only when y==0 && ppb==1; hue clamps to red
                color = data.lightColors[r];
            }
        }
    }
    result[index] = color;
}
