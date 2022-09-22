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

#define DMXPATH_PAN_MIN -180
#define DMXPATH_PAN_MAX 180

#define DMXPATH_TILT_MIN -180
#define DMXPATH_TILT_MAX 180

#define DMXPATH_ROTATION_MIN 0
#define DMXPATH_ROTATION_MAX 360

enum class DMXPathType {
    Circle,
    Square,
    Diamond,
    Line,
    Leaf,
    Eight,
    Custom,
    Unknown
};

class DMXPathEffect : public RenderableEffect
{
public:
    DMXPathEffect(int id);
    virtual ~DMXPathEffect();
    bool CanBeRandom() override {return true;}
    void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    void SetPanelStatus(Model *cls) override;
    void SetDefaultParameters() override;
    bool CanRenderPartialTimeInterval() const override { return true; }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_DMXPath_Pan")
            return DMXPATH_PAN_MIN;
        if (name == "E_VALUECURVE_DMXPath_Tilt")
            return DMXPATH_TILT_MIN;
        if (name == "E_VALUECURVE_DMXPath_Rotation")
            return DMXPATH_ROTATION_MAX;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_DMXPath_Pan")
            return DMXPATH_PAN_MAX;
        if (name == "E_VALUECURVE_DMXPath_Tilt")
            return DMXPATH_TILT_MAX;
        if (name == "E_VALUECURVE_DMXPath_Rotation")
            return DMXPATH_ROTATION_MIN;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    xlEffectPanel *CreatePanel(wxWindow *parent) override;
private:
    void SetDMXColorPixel(int chan, uint8_t value, RenderBuffer &buffer);
    std::pair<int, int> renderPath(DMXPathType effectType, double eff_pos, long length, int height, int width, int x_off, int y_off, int rot);
    std::pair<float, float> calcLocation(DMXPathType effectType, float degpos);
    DMXPathType DecodeType(const std::string& shape) const; 

    int ScaleToDMX(int value, int degresOfMovement) const;
};

