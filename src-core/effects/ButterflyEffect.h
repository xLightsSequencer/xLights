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

class ButterflyEffect : public RenderableEffect
{
public:
    ButterflyEffect(int id);
    virtual ~ButterflyEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }

    static std::string sColorsDefault;
    static int sStyleDefault;
    static int sChunksDefault;
    static int sChunksMin;
    static int sChunksMax;
    static int sSkipDefault;
    static int sSkipMin;
    static int sSkipMax;
    static int sSpeedDefault;
    static int sSpeedMin;
    static int sSpeedMax;
    static std::string sDirectionDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
