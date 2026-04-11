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

class PlasmaEffect : public RenderableEffect
{
public:
    PlasmaEffect(int id);
    virtual ~PlasmaEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }

    static int GetPlasmaColorScheme(const std::string &ColorSchemeStr);

    // Cached from Plasma.json by OnMetadataLoaded().
    static std::string sColorDefault;
    static int sStyleDefault;
    static int sLineDensityDefault;
    static int sSpeedDefault;
    static int sSpeedMin;
    static int sSpeedMax;

protected:
    virtual void OnMetadataLoaded() override;
};
