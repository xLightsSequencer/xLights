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

#include <vector>

#define CURTAIN_SPEED_MIN 0
#define CURTAIN_SPEED_MAX 10

#define CURTAIN_SWAG_MIN 0
#define CURTAIN_SWAG_MAX 10

class CurtainEffect : public RenderableEffect
{
public:
    CurtainEffect(int id);
    virtual ~CurtainEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Curtain_Speed")
            return CURTAIN_SPEED_MIN;
        if (name == "E_VALUECURVE_Curtain_Swag")
            return CURTAIN_SWAG_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }
    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Curtain_Speed")
            return CURTAIN_SPEED_MAX;
        if (name == "E_VALUECURVE_Curtain_Swag")
            return CURTAIN_SWAG_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;

private:
    void DrawCurtain(RenderBuffer& buffer, bool LeftEdge, int xlimit, const std::vector<int>& SwagArray);
    void DrawCurtainVertical(RenderBuffer& buffer, bool topEdge, int ylimit, const std::vector<int>& SwagArray);
};

