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

#define GALAXY_CENTREX_MIN 0
#define GALAXY_CENTREX_MAX 100

#define GALAXY_CENTREY_MIN 0
#define GALAXY_CENTREY_MAX 100

#define GALAXY_STARTRADIUS_MIN 0
#define GALAXY_STARTRADIUS_MAX 250

#define GALAXY_ENDRADIUS_MIN 0
#define GALAXY_ENDRADIUS_MAX 250

#define GALAXY_STARTANGLE_MIN 0
#define GALAXY_STARTANGLE_MAX 360

#define GALAXY_REVOLUTIONS_MIN 0
#define GALAXY_REVOLUTIONS_MAX 3600

#define GALAXY_STARTWIDTH_MIN 0
#define GALAXY_STARTWIDTH_MAX 255

#define GALAXY_ENDWIDTH_MIN 0
#define GALAXY_ENDWIDTH_MAX 255

#define GALAXY_DURATION_MIN 0
#define GALAXY_DURATION_MAX 100

#define GALAXY_ACCEL_MIN -10
#define GALAXY_ACCEL_MAX 10

class GalaxyEffect : public RenderableEffect
{
public:
    GalaxyEffect(int id);
    virtual ~GalaxyEffect();

    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2,
                                     xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps) override;
    virtual void SetDefaultParameters() override;
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
        if (name == "E_VALUECURVE_Galaxy_CenterX")
            return GALAXY_CENTREX_MIN;
        if (name == "E_VALUECURVE_Galaxy_CenterY")
            return GALAXY_CENTREY_MIN;
        if (name == "E_VALUECURVE_Galaxy_Start_Radius")
            return GALAXY_STARTRADIUS_MIN;
        if (name == "E_VALUECURVE_Galaxy_End_Radius")
            return GALAXY_ENDRADIUS_MIN;
        if (name == "E_VALUECURVE_Galaxy_Start_Angle")
            return GALAXY_STARTANGLE_MIN;
        if (name == "E_VALUECURVE_Galaxy_Revolutions")
            return GALAXY_REVOLUTIONS_MIN;
        if (name == "E_VALUECURVE_Galaxy_Start_Width")
            return GALAXY_STARTWIDTH_MIN;
        if (name == "E_VALUECURVE_Galaxy_End_Width")
            return GALAXY_ENDWIDTH_MIN;
        if (name == "E_VALUECURVE_Galaxy_Duration")
            return GALAXY_DURATION_MIN;
        if (name == "E_VALUECURVE_Galaxy_Accel")
            return GALAXY_ACCEL_MIN;

        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Galaxy_CenterX")
            return GALAXY_CENTREX_MAX;
        if (name == "E_VALUECURVE_Galaxy_CenterY")
            return GALAXY_CENTREY_MAX;
        if (name == "E_VALUECURVE_Galaxy_Start_Radius")
            return GALAXY_STARTRADIUS_MAX;
        if (name == "E_VALUECURVE_Galaxy_End_Radius")
            return GALAXY_ENDRADIUS_MAX;
        if (name == "E_VALUECURVE_Galaxy_Start_Angle")
            return GALAXY_STARTANGLE_MAX;
        if (name == "E_VALUECURVE_Galaxy_Revolutions")
            return GALAXY_REVOLUTIONS_MAX;
        if (name == "E_VALUECURVE_Galaxy_Start_Width")
            return GALAXY_STARTWIDTH_MAX;
        if (name == "E_VALUECURVE_Galaxy_End_Width")
            return GALAXY_ENDWIDTH_MAX;
        if (name == "E_VALUECURVE_Galaxy_Duration")
            return GALAXY_DURATION_MAX;
        if (name == "E_VALUECURVE_Galaxy_Accel")
            return GALAXY_ACCEL_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
};
