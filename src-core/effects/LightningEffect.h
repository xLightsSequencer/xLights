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

class xlColor;

class LightningEffect : public RenderableEffect
{
public:
    LightningEffect(int id);
    virtual ~LightningEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    virtual bool needToAdjustSettings(const std::string& version) override;

    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;

    // Cached from Lightning.json by OnMetadataLoaded().
    static int sNumberBoltsDefault;
    static int sNumberBoltsMin;
    static int sNumberBoltsMax;
    static int sNumberSegmentsDefault;
    static int sNumberSegmentsMin;
    static int sNumberSegmentsMax;
    static bool sForkedLightningDefault;
    static int sTopXDefault;
    static int sTopXMin;
    static int sTopXMax;
    static int sTopYDefault;
    static int sTopYMin;
    static int sTopYMax;
    static int sBotXDefault;
    static int sWidthDefault;
    static std::string sDirectionDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
