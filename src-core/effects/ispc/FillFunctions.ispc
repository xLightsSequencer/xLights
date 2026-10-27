/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Fill effect. All per-pixel work is integer math: the CPU
// (FillEffect::BuildFillLut) precomputes, with the exact scalar double-precision
// color math, a per-line color LUT plus a per-line painted mask (indexed by the
// physical row for Up/Down or column for Left/Right). Each pixel just maps to its
// line and copies the LUT color when that line is painted, leaving unpainted
// pixels untouched (so cleared and Persistent buffers both match the scalar
// renderer). Byte-identical to the scalar loop. Must stay in lockstep with
// FillFunctions.metal.

struct FillData {
    unsigned int width;
    unsigned int height;
    int          vertical;  // 1 => line is the row (y) [Up/Down]; 0 => the column (x) [Left/Right]
};

export void FillEffectISPC(const uniform FillData * uniform data,
                           uniform int startIdx,
                           uniform int endIdx,
                           const uniform uint8<4> * uniform lut,
                           const uniform uint8 * uniform painted,
                           uniform uint8<4> * uniform result) {
    uniform int w = (int)data->width;
    uniform int vert = data->vertical;
    // line is already in [0, maxLine] by construction; this clamp is a no-op that
    // keeps ispc's gather-index analysis happy at -O1+ (ispc/ispc#3881/#3882).
    uniform int maxLine = (vert ? (int)data->height : w) - 1;

    // Walk rows instead of deriving x/y from index % width: there is no
    // vector integer divide, so ispc runs a varying % or / one lane at a time.
    for (uniform int y = startIdx / max(w, 1); w > 0 && y * w < endIdx; y++) {
        uniform int rowStart = y * w;
        foreach (x = max(startIdx - rowStart, 0) ... min(endIdx - rowStart, w)) {
            int index = rowStart + x;
            int line = vert ? y : x;
            line = min(line, maxLine);
            if (painted[line] != 0) {
                result[index] = lut[line];
            }
        }
    }
}
