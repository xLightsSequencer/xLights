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

class GarlandsEffect : public RenderableEffect
{
public:
    GarlandsEffect(int id);
    virtual ~GarlandsEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    // Cached from Garlands.json by OnMetadataLoaded().
    static int sTypeDefault;
    static int sTypeMin;
    static int sTypeMax;
    static int sSpacingDefault;
    static int sSpacingMin;
    static int sSpacingMax;
    // Garlands_Cycles uses divisor 10: JSON stores min/max as pre-divisor
    // integers (0..200) and default as post-divisor float (1.0). Pre-migration
    // sequences had VCs stored in post-divisor (0..20) space; UpgradeValueCurve
    // with the now-fixed ConvertChangedScale rescales them to (0..200) on load.
    static double sCyclesDefault;
    static double sCyclesMin;
    static double sCyclesMax;
    static int sCyclesDivisor;
    static std::string sDirectionDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
