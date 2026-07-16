#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"

class ShimmerEffect : public RenderableEffect
{
public:
    ShimmerEffect(int id);
    virtual ~ShimmerEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual FrameParallelism GetFrameParallelism(const SettingsMap& settings) const override { return FrameParallelism::Pure; }
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    // Cached from Shimmer.json by OnMetadataLoaded() — see feedback note on
    // using statics so import/legacy code can read them directly.
    static int sDutyFactorDefault;
    static int sDutyFactorMin;
    static int sDutyFactorMax;
    static double sCyclesDefault;
    static double sCyclesMin;
    static double sCyclesMax;
    static int sCyclesDivisor;
    static bool sUseAllColorsDefault;

protected:
    virtual void OnMetadataLoaded() override;

    // Per-pixel color source modes for the ISPC/Metal kernels — kept in lockstep
    // with ShimmerFunctions.ispc and ShimmerFunctions.metal.
    enum ShimmerLutMode {
        SHIMMER_LUT_FLAT = 0,
        SHIMMER_LUT_X = 1,
        SHIMMER_LUT_Y = 2,
        SHIMMER_LUT_RANDOM = 3
    };

    // Per-frame scalar state shared by the CPU/ISPC path and the Metal wrapper.
    // Returns false when this frame draws nothing (duty-cycle "off").
    bool CalcFrameState(const SettingsMap& settings, RenderBuffer& buffer, int& colorIdx, bool& useAllColors);
    // Builds the per-pixel color lookup table the kernels index (palette colors
    // for random mode, one flat color, or a per-column/per-row spatial gradient)
    // and returns the ShimmerLutMode describing how to index it.
    int BuildShimmerLut(RenderBuffer& buffer, int colorIdx, bool useAllColors, xlColorVector& lut);
};
