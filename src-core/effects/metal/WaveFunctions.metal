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

// Wave effect — the None-fill (constant color) case. The per-column vertical
// band [y1,y2] is precomputed on the CPU in double precision (incl. std::sin)
// and passed in `cols` (cols[2x]=y1, cols[2x+1]=y2; y1 > y2 means the column
// draws nothing), so this kernel is pure integer band-testing and is
// byte-identical to the scalar renderer. Rainbow/Palette fills need a
// double-precision hue division to stay byte-identical, which Metal lacks, so
// the wrapper routes those to the ISPC CPU path instead.
// Must stay in lockstep with the band logic in WaveFunctions.ispc.
kernel void WaveEffect(constant MetalWaveData &data,
                       device uchar4* result,
                       device const int* cols,
                       uint index [[thread_position_in_grid]])
{
    if (index >= (data.width * data.height)) return;
    int width = (int)data.width;
    int height = (int)data.height;
    int x = (int)index % width;
    int Y = (int)index / width;
    int yy = Y - data.yoffset;

    int y1 = cols[2 * x];
    int y2 = cols[2 * x + 1];

    // Mirror wins on overlap: reflected band [height-y2, height-y1] is drawn last
    // by the scalar renderer.
    bool inBand = false;
    if (data.mirror != 0) {
        int mlo = height - y2;
        int mhi = height - y1;
        if (yy >= mlo && yy <= mhi) {
            inBand = true;
        }
    }
    if (!inBand && yy >= y1 && yy <= y2) {
        inBand = true;
    }

    if (inBand) {
        result[index] = data.noneColor;
    }
}
