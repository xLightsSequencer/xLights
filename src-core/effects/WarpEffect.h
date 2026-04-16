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

class WarpEffect : public RenderableEffect
{
public:
    enum WarpType {
        WATER_DROPS = 0,
        SINGLE_WATER_DROP,
        CIRCLE_REVEAL,
        BANDED_SWIRL,
        CIRCULAR_SWIRL,
        DISSOLVE,
        RIPPLE,
        DROP,
        WAVY,
        SAMPLE_ON,
        MIRROR,
        COPY,
        FLIP,

        COUNT_WARP_STYLES
    };
    WarpType mapWarpType(const std::string& wt);

    WarpEffect(int id);
    virtual ~WarpEffect();
    virtual bool CanBeRandom() override
    {
        return false;
    }
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const override
    {
        return false;
    }
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;

    // Cached from Warp.json by OnMetadataLoaded().
    static std::string sTypeDefault;
    static std::string sTreatmentDefault;
    static int sXDefault;
    static int sXMin;
    static int sXMax;
    static int sYDefault;
    static int sYMin;
    static int sYMax;
    static int sCycleCountDefault;
    static int sSpeedDefault;
    static int sFrequencyDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
