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

class SpirographEffect : public RenderableEffect
{
public:
    SpirographEffect(int id);
    virtual ~SpirographEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    // Cached from Spirograph.json by OnMetadataLoaded().
    static int sRDefault;
    static int sRMin;
    static int sRMax;
    static int srDefault;
    static int srMin;
    static int srMax;
    static int sdDefault;
    static int sdMin;
    static int sdMax;
    static int sAnimateDefault;
    static int sAnimateMin;
    static int sAnimateMax;
    static int sSpeedDefault;
    static int sSpeedMin;
    static int sSpeedMax;
    static int sLengthDefault;
    static int sLengthMin;
    static int sLengthMax;
    static int sWidthDefault;
    static int sWidthMin;
    static int sWidthMax;

protected:
    virtual void OnMetadataLoaded() override;
};
