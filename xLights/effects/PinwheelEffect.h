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

class xlColor;

#define PINWHEEL_X_MIN -100
#define PINWHEEL_X_MAX 100

#define PINWHEEL_Y_MIN -100
#define PINWHEEL_Y_MAX 100

#define PINWHEEL_ARMSIZE_MIN 0
#define PINWHEEL_ARMSIZE_MAX 400

#define PINWHEEL_SPEED_MIN 0
#define PINWHEEL_SPEED_MAX 50

#define PINWHEEL_THICKNESS_MIN 0
#define PINWHEEL_THICKNESS_MAX 100

#define PINWHEEL_TWIST_MIN -360
#define PINWHEEL_TWIST_MAX 360

#define PINWHEEL_OFFSET_MIN 0
#define PINWHEEL_OFFSET_MAX 360

class PinwheelEffect : public RenderableEffect
{
public:
    PinwheelEffect(int id);
    virtual ~PinwheelEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool SupportsRadialColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
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
        if (name == "E_VALUECURVE_PinwheelXC")
            return PINWHEEL_X_MIN;
        if (name == "E_VALUECURVE_PinwheelYC")
            return PINWHEEL_Y_MIN;
        if (name == "E_VALUECURVE_Pinwheel_ArmSize")
            return PINWHEEL_ARMSIZE_MIN;
        if (name == "E_VALUECURVE_Pinwheel_Speed")
            return PINWHEEL_SPEED_MIN;
        if (name == "E_VALUECURVE_Pinwheel_Thickness")
            return PINWHEEL_THICKNESS_MIN;
        if (name == "E_VALUECURVE_Pinwheel_Twist")
            return PINWHEEL_TWIST_MIN;
        if (name == "E_VALUECURVE_Pinwheel_Offset")
            return PINWHEEL_OFFSET_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_PinwheelXC")
            return PINWHEEL_X_MAX;
        if (name == "E_VALUECURVE_PinwheelYC")
            return PINWHEEL_Y_MAX;
        if (name == "E_VALUECURVE_Pinwheel_ArmSize")
            return PINWHEEL_ARMSIZE_MAX;
        if (name == "E_VALUECURVE_Pinwheel_Speed")
            return PINWHEEL_SPEED_MAX;
        if (name == "E_VALUECURVE_Pinwheel_Thickness")
            return PINWHEEL_THICKNESS_MAX;
        if (name == "E_VALUECURVE_Pinwheel_Twist")
            return PINWHEEL_TWIST_MAX;
        if (name == "E_VALUECURVE_Pinwheel_Offset")
            return PINWHEEL_OFFSET_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    enum Pinwheel3DType {
        PW_3D_NONE,
        PW_3D,
        PW_3D_Inverted,
        PW_SWEEP
    };
    
    class PinwheelData {
    public:
        PinwheelData(int i) : pinwheel_arms(i), colorarray(i), colorIsSpacial(i), colorsAsHSV(i), colorsAsColor(i) {
            
        }
        std::vector<size_t> colorarray;
        std::vector<bool> colorIsSpacial;
        std::vector<HSVValue> colorsAsHSV;
        std::vector<xlColor> colorsAsColor;
        
        bool hasSpacial = false;
        int pinwheel_arms;
        Pinwheel3DType pw3dType;
        int xc_adj = 0;
        int yc_adj = 0;
        float tmax;
        int degrees_per_arm;
        bool pinwheel_rotation;
        int pinwheel_twist;
        int max_radius;
        int poffset;
        float pos;
    };

    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
    
    void RenderNewMethod(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer);
    virtual void RenderNewArms(RenderBuffer& buffer, PinwheelData &data);
    void RenderOldMethod(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer);
    
    void Draw_arm(RenderBuffer& buffer, int base_degrees, int max_radius, int pinwheel_twist, int xc_adj, int yc_adj, int colorIdx, Pinwheel3DType pinwheel_3d, float round);

    Pinwheel3DType to3dType(const std::string& pinwheel_3d);
    void adjustColor(Pinwheel3DType pw3dType, xlColor& color, HSVValue& hsv, bool allowAlpha, float round);
};
