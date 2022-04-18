#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"

#define WAVE_NUMBER_MIN 180
#define WAVE_NUMBER_MAX 3600

#define WAVE_THICKNESS_MIN 0
#define WAVE_THICKNESS_MAX 100

#define WAVE_HEIGHT_MIN 0
#define WAVE_HEIGHT_MAX 100

#define WAVE_SPEED_MIN 0
#define WAVE_SPEED_MAX 5000
#define WAVE_SPEED_DIVISOR 100

#define WAVE_YOFFSET_MIN -250
#define WAVE_YOFFSET_MAX 250

class WaveEffect : public RenderableEffect
{
public:
    WaveEffect(int id);
    virtual ~WaveEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool CanBeRandom() override
    {
        return false;
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
};
