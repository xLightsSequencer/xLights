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
};
