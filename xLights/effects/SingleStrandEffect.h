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

#define SINGLESTRAND_ROTATIONS_MIN 1
#define SINGLESTRAND_ROTATIONS_MAX 500
#define SINGLESTRAND_ROTATIONS_DIVISOR 10

#define SINGLESTRAND_CHASES_MIN 1
#define SINGLESTRAND_CHASES_MAX 20

#define SINGLESTRAND_COLOURMIX_MIN 1
#define SINGLESTRAND_COLOURMIX_MAX 100

#define SINGLESTRAND_OFFSET_MIN -5000
#define SINGLESTRAND_OFFSET_MAX 5000
#define SINGLESTRAND_OFFSET_DIVISOR 10

#define SINGLESTRAND_FXINTENSITY_MIN 0
#define SINGLESTRAND_FXINTENSITY_MAX 255

#define SINGLESTRAND_FXSPEED_MIN 0
#define SINGLESTRAND_FXSPEED_MAX 255

class SingleStrandEffect : public RenderableEffect
{
public:
    SingleStrandEffect(int id);
    virtual ~SingleStrandEffect();
    virtual void SetDefaultParameters() override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Chase_Rotations")
            return SINGLESTRAND_ROTATIONS_MIN;
        if (name == "E_VALUECURVE_Number_Chases")
            return SINGLESTRAND_CHASES_MIN;
        if (name == "E_VALUECURVE_Color_Mix1")
            return SINGLESTRAND_COLOURMIX_MIN;
        if (name == "E_VALUECURVE_Chase_Offset")
            return SINGLESTRAND_OFFSET_MIN;
        if (name == "E_VALUECURVE_FX_Intensity")
            return SINGLESTRAND_FXINTENSITY_MIN;
        if (name == "E_VALUECURVE_FX_Speed")
            return SINGLESTRAND_FXSPEED_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Chase_Rotations")
            return SINGLESTRAND_ROTATIONS_MAX;
        if (name == "E_VALUECURVE_Number_Chases")
            return SINGLESTRAND_CHASES_MAX;
        if (name == "E_VALUECURVE_Color_Mix1")
            return SINGLESTRAND_COLOURMIX_MAX;
        if (name == "E_VALUECURVE_Chase_Offset")
            return SINGLESTRAND_OFFSET_MAX;
        if (name == "E_VALUECURVE_FX_Intensity")
            return SINGLESTRAND_FXINTENSITY_MAX;
        if (name == "E_VALUECURVE_FX_Speed")
            return SINGLESTRAND_FXSPEED_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

    virtual int GetSettingVCDivisor(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Chase_Rotations")
            return SINGLESTRAND_ROTATIONS_DIVISOR;
        if (name == "E_VALUECURVE_Chase_Offset")
            return SINGLESTRAND_OFFSET_DIVISOR;
        return RenderableEffect::GetSettingVCDivisor(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;

private:
    void RenderSingleStrandChase(RenderBuffer& buffer,
                                 const std::string& ColorScheme, int Number_Chases, int chaseSize,
                                 const std::string& Chase_Type1,
                                 bool Chase_3dFade1, bool Chase_Group_All,
                                 float chaseSpeed, float offset);
    void RenderSingleStrandSkips(RenderBuffer& buffer, Effect* eff, int Skips_BandSize,
                                 int Skips_SkipSize, int Skips_StartPos, const std::string& Skips_Direction, int advances);
    void RenderSingleStrandFX(RenderBuffer& buffer, Effect* eff, int intensity, int speed, const std::string& fx, const std::string& palette);
    void draw_chase(RenderBuffer& buffer,
                    int x, bool group, int ColorScheme, int Number_Chases, bool autoReverse, int width,
                    int Color_Mix1, bool Chase_Fade3d1, int ChaseDirection, bool mirror);
};
