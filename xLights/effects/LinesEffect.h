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
#include "../RenderBuffer.h"

#define LINES_SPEED_MIN 1
#define LINES_SPEED_MAX 10

#define LINES_THICKNESS_MIN 1
#define LINES_THICKNESS_MAX 10

class LinesEffect : public RenderableEffect
{
public:
    LinesEffect(int id);
    virtual ~LinesEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    void Render(RenderBuffer& buffer,
                int objects, int segments, int thickness, int speed, int trails, bool fadeTrails);
    virtual void SetDefaultParameters() override;

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Lines_Speed")
            return LINES_SPEED_MIN;
        if (name == "E_VALUECURVE_Lines_Thickness")
            return LINES_THICKNESS_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Lines_Speed")
            return LINES_SPEED_MAX;
        if (name == "E_VALUECURVE_Lines_Thickness")
            return LINES_THICKNESS_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
};
