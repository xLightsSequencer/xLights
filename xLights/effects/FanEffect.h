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

#define FAN_CENTREX_MIN 0
#define FAN_CENTREX_MAX 100

#define FAN_CENTREY_MIN 0
#define FAN_CENTREY_MAX 100

#define FAN_STARTRADIUS_MIN 0
#define FAN_STARTRADIUS_MAX 2500

#define FAN_STARTANGLE_MIN 0
#define FAN_STARTANGLE_MAX 360

#define FAN_ENDRADIUS_MIN 0
#define FAN_ENDRADIUS_MAX 2500

#define FAN_REVOLUTIONS_MIN 0
#define FAN_REVOLUTIONS_MAX 3600

#define FAN_BLADES_MIN 1
#define FAN_BLADES_MAX 16

#define FAN_BLADEWIDTH_MIN 5
#define FAN_BLADEWIDTH_MAX 100

#define FAN_BLADEANGLE_MIN -360
#define FAN_BLADEANGLE_MAX 360

#define FAN_NUMELEMENTS_MIN 1
#define FAN_NUMELEMENTS_MAX 4

#define FAN_ELEMENTWIDTH_MIN 5
#define FAN_ELEMENTWIDTH_MAX 100

#define FAN_DURATION_MIN 0
#define FAN_DURATION_MAX 100

#define FAN_ACCEL_MIN -10
#define FAN_ACCEL_MAX 10

class FanEffect : public RenderableEffect
{
public:
    FanEffect(int id);
    virtual ~FanEffect();

    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual void SetDefaultParameters() override;
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2,
                                     xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps) override;
    virtual bool SupportsRadialColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Fan_CenterX")
            return FAN_CENTREX_MIN;
        if (name == "E_VALUECURVE_Fan_CenterY")
            return FAN_CENTREY_MIN;
        if (name == "E_VALUECURVE_Fan_Start_Radius")
            return FAN_STARTRADIUS_MIN;
        if (name == "E_VALUECURVE_Fan_Start_Angle")
            return FAN_STARTANGLE_MIN;
        if (name == "E_VALUECURVE_Fan_End_Radius")
            return FAN_ENDRADIUS_MIN;
        if (name == "E_VALUECURVE_Fan_Revolutions")
            return FAN_REVOLUTIONS_MIN;
        if (name == "E_VALUECURVE_Fan_Num_Blades")
            return FAN_BLADES_MIN;
        if (name == "E_VALUECURVE_Fan_Blade_Width")
            return FAN_BLADEWIDTH_MIN;
        if (name == "E_VALUECURVE_Fan_Blade_Angle")
            return FAN_BLADEANGLE_MIN;
        if (name == "E_VALUECURVE_Fan_Num_Elements")
            return FAN_NUMELEMENTS_MIN;
        if (name == "E_VALUECURVE_Fan_Element_Width")
            return FAN_ELEMENTWIDTH_MIN;
        if (name == "E_VALUECURVE_Fan_Duration")
            return FAN_DURATION_MIN;
        if (name == "E_VALUECURVE_Fan_Accel")
            return FAN_ACCEL_MIN;

        return RenderableEffect::GetSettingVCMin(name);
    }
    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Fan_CenterX")
            return FAN_CENTREX_MAX;
        if (name == "E_VALUECURVE_Fan_CenterY")
            return FAN_CENTREY_MAX;
        if (name == "E_VALUECURVE_Fan_Start_Radius")
            return FAN_STARTRADIUS_MAX;
        if (name == "E_VALUECURVE_Fan_Start_Angle")
            return FAN_STARTANGLE_MAX;
        if (name == "E_VALUECURVE_Fan_End_Radius")
            return FAN_ENDRADIUS_MAX;
        if (name == "E_VALUECURVE_Fan_Revolutions")
            return FAN_REVOLUTIONS_MAX;
        if (name == "E_VALUECURVE_Fan_Num_Blades")
            return FAN_BLADES_MAX;
        if (name == "E_VALUECURVE_Fan_Blade_Width")
            return FAN_BLADEWIDTH_MAX;
        if (name == "E_VALUECURVE_Fan_Blade_Angle")
            return FAN_BLADEANGLE_MAX;
        if (name == "E_VALUECURVE_Fan_Num_Elements")
            return FAN_NUMELEMENTS_MAX;
        if (name == "E_VALUECURVE_Fan_Element_Width")
            return FAN_ELEMENTWIDTH_MAX;
        if (name == "E_VALUECURVE_Fan_Duration")
            return FAN_DURATION_MAX;
        if (name == "E_VALUECURVE_Fan_Accel")
            return FAN_ACCEL_MAX;

        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
};
