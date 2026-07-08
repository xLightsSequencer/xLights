/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShimmerEffect.h"

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"

#include "../../include/shimmer.xpm"

#include "ispc/ShimmerFunctions.ispc.h"
#include "Parallel.h"

#include <algorithm>

int ShimmerEffect::sDutyFactorDefault = 50;
int ShimmerEffect::sDutyFactorMin = 1;
int ShimmerEffect::sDutyFactorMax = 100;
double ShimmerEffect::sCyclesDefault = 1.0;
double ShimmerEffect::sCyclesMin = 0;
double ShimmerEffect::sCyclesMax = 6000;
int ShimmerEffect::sCyclesDivisor = 10;
bool ShimmerEffect::sUseAllColorsDefault = false;

ShimmerEffect::ShimmerEffect(int id) : RenderableEffect(id, "Shimmer", shimmer, shimmer, shimmer, shimmer, shimmer)
{
    //ctor
}

ShimmerEffect::~ShimmerEffect()
{
    //dtor
}

void ShimmerEffect::OnMetadataLoaded()
{
    sDutyFactorDefault = GetIntDefault("Shimmer_Duty_Factor", sDutyFactorDefault);
    sDutyFactorMin = (int)GetMinFromMetadata("Shimmer_Duty_Factor", sDutyFactorMin);
    sDutyFactorMax = (int)GetMaxFromMetadata("Shimmer_Duty_Factor", sDutyFactorMax);
    sCyclesDefault = GetDoubleDefault("Shimmer_Cycles", sCyclesDefault);
    sCyclesMin = GetMinFromMetadata("Shimmer_Cycles", sCyclesMin);
    sCyclesMax = GetMaxFromMetadata("Shimmer_Cycles", sCyclesMax);
    sCyclesDivisor = GetDivisorFromMetadata("Shimmer_Cycles", sCyclesDivisor);
    sUseAllColorsDefault = GetBoolDefault("Shimmer_Use_All_Colors", sUseAllColorsDefault);
}

bool ShimmerEffect::CalcFrameState(const SettingsMap& SettingsMap, RenderBuffer& buffer, int& colorIdx, bool& useAllColors)
{
    // Defaults were cached from Shimmer.json in OnMetadataLoaded().
    float oset = buffer.GetEffectTimeIntervalPosition();
    int Duty_Factor = GetValueCurveInt("Shimmer_Duty_Factor", sDutyFactorDefault, SettingsMap, oset,
                                       sDutyFactorMin, sDutyFactorMax,
                                       buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    useAllColors = SettingsMap.GetBool("CHECKBOX_Shimmer_Use_All_Colors", sUseAllColorsDefault);
    double cycles = GetValueCurveDouble("Shimmer_Cycles", sCyclesDefault, SettingsMap, oset,
                                        sCyclesMin, sCyclesMax,
                                        buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sCyclesDivisor);
    bool pre2017_7 = SettingsMap.GetBool("CHECKBOX_PRE_2017_7", false);
    int colorcnt = buffer.GetColorCount();

    colorIdx = 0;
    if (pre2017_7) {
        double position = buffer.GetEffectTimeIntervalPosition(cycles);

        colorIdx = round(position * 0.999 * (double)colorcnt);

        double pos2 = position * colorcnt;
        while (pos2 > 1.0) {
            pos2 -= 1.0;
        }
        if (pos2 * 100 > Duty_Factor) {
            return false;
        }
    } else {
        // If cycles are too high maximise out at on and off
        if (cycles > ((double)buffer.curEffEndPer - (double)buffer.curEffStartPer) / 2.0) {
            colorIdx = (buffer.curPeriod - buffer.curEffStartPer) % (2 * colorcnt);
            if (colorIdx % 2 == 0) {
                colorIdx /= 2;
            } else {
                return false;
            }
        } else {
            double position = buffer.GetEffectTimeIntervalPosition(cycles);
            if (position > 1.0)
                position = 0.0;

            // black if we are beyond the duty factor
            if (position >= (double)Duty_Factor / 100.0)
                return false;

            // now we need to work out the color

            // scale up the position
            int cycle = ((buffer.curPeriod - buffer.curEffStartPer) * cycles) / (buffer.curEffEndPer - buffer.curEffStartPer);
            colorIdx = cycle % colorcnt;
        }
    }
    return true;
}

int ShimmerEffect::BuildShimmerLut(RenderBuffer& buffer, int colorIdx, bool useAllColors, xlColorVector& lut)
{
    if (useAllColors) {
        int colorcnt = buffer.GetColorCount();
        lut.resize(colorcnt);
        for (int i = 0; i < colorcnt; i++) {
            buffer.palette.GetColor(i, lut[i]);
        }
        return SHIMMER_LUT_RANDOM;
    }
    int mode = SHIMMER_LUT_FLAT;
    const ColorCurve& cc = buffer.palette.GetColorCurve(colorIdx);
    if (cc.IsActive()) {
        switch (cc.GetTimeCurve()) {
        case TC_RIGHT:
        case TC_LEFT:
            mode = SHIMMER_LUT_X;
            break;
        case TC_UP:
        case TC_DOWN:
            mode = SHIMMER_LUT_Y;
            break;
        default:
            break;
        }
    }
    int n = mode == SHIMMER_LUT_X ? buffer.BufferWi : (mode == SHIMMER_LUT_Y ? buffer.BufferHt : 1);
    lut.resize(n);
    for (int i = 0; i < n; i++) {
        int x = mode == SHIMMER_LUT_X ? i : 0;
        int y = mode == SHIMMER_LUT_Y ? i : 0;
        buffer.palette.GetSpatialColor(colorIdx, (float)x / (float)buffer.BufferWi, (float)y / (float)buffer.BufferHt, lut[i]);
    }
    return mode;
}

void ShimmerEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    int ColorIdx = 0;
    bool Use_All_Colors = false;
    if (!CalcFrameState(SettingsMap, buffer, ColorIdx, Use_All_Colors)) {
        return;
    }
    int colorcnt = buffer.GetColorCount();

    // ISPC is the CPU path. BuildShimmerLut precomputes a per-frame color LUT (flat,
    // spatial X/Y gradient, or per-pixel-random palette) that the kernel indexes, so
    // the kernel handles every mode; there is no scalar fallback.
    xlColorVector lut;
    ispc::ShimmerData sdata;
    sdata.width = buffer.BufferWi;
    sdata.height = buffer.BufferHt;
    sdata.lutMode = BuildShimmerLut(buffer, ColorIdx, Use_All_Colors, lut);
    sdata.colorCount = colorcnt;
    sdata.frameSeed = buffer.hashRandomFrameSeed();

    // Bound the ISPC writes by the actual pixel allocation, not the logical
    // dimensions (variable sub-buffers can leave GetPixelCount() smaller
    // than BufferWi*BufferHt) — the kernel has no bounds check.
    int max = std::min<int>(buffer.GetPixelCount(), buffer.BufferWi * buffer.BufferHt);
    const ispc::uint8_t4* lutData = reinterpret_cast<const ispc::uint8_t4*>(lut.data());
    constexpr int bfBlockSize = 4096;
    int blocks = max / bfBlockSize + 1;
    parallel_for(0, blocks, [&sdata, &buffer, lutData, max](int blk) {
        int start = blk * bfBlockSize;
        int end = std::min(start + bfBlockSize, max);
        ispc::ShimmerEffectISPC(&sdata, start, end, lutData, (ispc::uint8_t4*)buffer.GetPixels());
    });
}
