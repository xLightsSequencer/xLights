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

#include "../Color.h"

#define RIPPLE_CYCLES_MIN 0
#define RIPPLE_CYCLES_MAX 300
#define RIPPLE_CYCLES_DIVISOR 10

#define RIPPLE_THICKNESS_MIN 1
#define RIPPLE_THICKNESS_MAX 100

#define RIPPLE_ROTATION_MIN -360
#define RIPPLE_ROTATION_MAX 360

#define RIPPLE_XC_MIN -100
#define RIPPLE_XC_MAX 100

#define RIPPLE_YC_MIN -100
#define RIPPLE_YC_MAX 100

#define RIPPLE_SPACING_MIN 1
#define RIPPLE_SPACING_MAX 400
#define RIPPLE_SPACING_DIVISOR 10

#define RIPPLE_SCALE_MIN 0
#define RIPPLE_SCALE_MAX 500

#define RIPPLE_VELOCITY_MIN 0
#define RIPPLE_VELOCITY_MAX 300
#define RIPPLE_VELOCITY_DIVISOR 10

#define RIPPLE_DIRECTION_MIN -360
#define RIPPLE_DIRECTION_MAX 360

#define RIPPLE_TWIST_MIN -450
#define RIPPLE_TWIST_MAX 450
#define RIPPLE_TWIST_DIVISOR 10

#define RIPPLE_OUTLINE_MIN 0
#define RIPPLE_OUTLINE_MAX 100
#define RIPPLE_OUTLINE_DIVISOR 10

class RippleEffect : public RenderableEffect
{
public:
    RippleEffect(int id);
    virtual ~RippleEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    //virtual void SetPanelStatus(Model* cls) override;
    //virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }
    virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap& SettingsMap) const override;
    virtual bool CleanupFileLocations(xLightsFrame* frame, SettingsMap& SettingsMap) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;

    virtual bool SupportsRadialColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
        // Not sure this updates dynamically as one would want
        //std::string ds = SettingsMap.Get("CHOICE_Ripple_Draw_Style", "Old");
        //return ds != "Old";
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Ripple_Cycles")
            return RIPPLE_CYCLES_MIN;
        if (name == "E_VALUECURVE_Ripple_Thickness")
            return RIPPLE_THICKNESS_MIN;
        if (name == "E_VALUECURVE_Ripple_Rotation")
            return RIPPLE_ROTATION_MIN;
        if (name == "E_VALUECURVE_Ripple_XC")
            return RIPPLE_XC_MIN;
        if (name == "E_VALUECURVE_Ripple_YC")
            return RIPPLE_YC_MIN;
        if (name == "E_VALUECURVE_Ripple_Spacing")
            return RIPPLE_SPACING_MIN;
        if (name == "E_VALUECURVE_Ripple_Scale")
            return RIPPLE_SCALE_MIN;
        if (name == "E_VALUECURVE_Ripple_Velocity")
            return RIPPLE_VELOCITY_MIN;
        if (name == "E_VALUECURVE_Ripple_Direction")
            return RIPPLE_DIRECTION_MIN;
        if (name == "E_VALUECURVE_Ripple_Twist")
            return RIPPLE_TWIST_MIN;
        if (name == "E_VALUECURVE_Ripple_Outline")
            return RIPPLE_OUTLINE_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Ripple_Cycles")
            return RIPPLE_CYCLES_MAX;
        if (name == "E_VALUECURVE_Ripple_Thickness")
            return RIPPLE_THICKNESS_MAX;
        if (name == "E_VALUECURVE_Ripple_Rotation")
            return RIPPLE_ROTATION_MAX;
        if (name == "E_VALUECURVE_Ripple_XC")
            return RIPPLE_XC_MAX;
        if (name == "E_VALUECURVE_Ripple_YC")
            return RIPPLE_YC_MAX;
        if (name == "E_VALUECURVE_Ripple_Spacing")
            return RIPPLE_SPACING_MAX;
        if (name == "E_VALUECURVE_Ripple_Scale")
            return RIPPLE_SCALE_MAX;
        if (name == "E_VALUECURVE_Ripple_Velocity")
            return RIPPLE_VELOCITY_MAX;
        if (name == "E_VALUECURVE_Ripple_Direction")
            return RIPPLE_DIRECTION_MAX;
        if (name == "E_VALUECURVE_Ripple_Twist")
            return RIPPLE_TWIST_MAX;
        if (name == "E_VALUECURVE_Ripple_Outline")
            return RIPPLE_OUTLINE_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

    virtual int GetSettingVCDivisor(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Ripple_Cycles")
            return RIPPLE_CYCLES_DIVISOR;
        if (name == "E_VALUECURVE_Ripple_Spacing")
            return RIPPLE_SPACING_DIVISOR;
        if (name == "E_VALUECURVE_Ripple_Twist")
            return RIPPLE_TWIST_DIVISOR;
        if (name == "E_VALUECURVE_Ripple_Velocity")
            return RIPPLE_VELOCITY_DIVISOR;
        if (name == "E_VALUECURVE_Ripple_Outline")
            return RIPPLE_OUTLINE_DIVISOR;
        return RenderableEffect::GetSettingVCDivisor(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;

private:

    // Old-style draw functions
    void Drawcircle(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
    void Drawsquare(RenderBuffer& buffer, int Movement, int x1, int x2, int y1, int y2, int Ripple_Thickness, int CheckBox_Ripple3D, HSVValue& hsv);
    void Drawtriangle(RenderBuffer& buffer, int Movement, int xc, int yc, double side, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
    void Drawstar(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, int points, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D, float rotation);
    void Drawheart(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
    void Drawpolygon(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, int points, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D, float rotation);
    void Drawsnowflake(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, int points, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D, double rotation);
    void Drawtree(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
    void Drawcandycane(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
    void Drawcrucifix(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
    void Drawpresent(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
};
