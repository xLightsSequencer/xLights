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

class SnowflakesEffect : public RenderableEffect
{
public:
    SnowflakesEffect(int id);
    virtual ~SnowflakesEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;

    static int sCountDefault;
    static int sCountMin;
    static int sCountMax;
    static int sTypeDefault;
    static int sSpeedDefault;
    static int sSpeedMin;
    static int sSpeedMax;
    static std::string sFallingDefault;
    static int sWarmupFramesDefault;

protected:
    virtual void OnMetadataLoaded() override;
    void MoveFlakes(RenderBuffer& buffer, int snowflakeType, const std::string& falling, int count, const xlColor& color1, int& effectState);
};
