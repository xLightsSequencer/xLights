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

// Fill effect — per-pixel kernel. The CPU (MetalFillEffect.mm / FillEffect::BuildFillLut)
// precomputes, with the exact scalar double-precision color math, a per-line color LUT
// (bound at buffer(2)) plus a per-line painted mask (bound at buffer(3)), indexed by the
// physical row for Up/Down or column for Left/Right. Each thread maps its pixel to a line
// and copies the LUT color when that line is painted, leaving unpainted pixels untouched
// (so cleared and Persistent buffers both match the scalar renderer). Byte-identical to the
// scalar renderer. Kept in lockstep with FillFunctions.ispc.
kernel void FillEffect(constant MetalFillData &data [[buffer(0)]],
                       device uchar4 *result        [[buffer(1)]],
                       constant uchar4 *lut         [[buffer(2)]],
                       constant uchar  *painted     [[buffer(3)]],
                       uint index                   [[thread_position_in_grid]])
{
    int line = data.vertical ? int(index / data.width) : int(index % data.width);
    if (painted[line] != 0) {
        result[index] = lut[line];
    }
}
