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

#define DMXPATH_ROTATION_MIN 0
#define DMXPATH_ROTATION_MAX 360

class DMXPathAssistPanel;

enum class DMXPathType {
    Circle,
    Square,
    Diamond,
    Line,
    Leaf,
    Eight,
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
        if (name == "E_VALUECURVE_DMXPath_Rotation")
            return DMXPATH_ROTATION_MAX;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_DMXPath_Rotation")
            return DMXPATH_ROTATION_MIN;
        return RenderableEffect::GetSettingVCMax(name);
    }

    static std::pair<int, int> RenderPath(DMXPathType effectType, double eff_pos, int height, int width, int x_off, int y_off, int rot);
    static DMXPathType DecodeType(const std::string& shape); 
    static std::pair<float, float> CalcLocation(DMXPathType effectType, float degpos);

    AssistPanel* GetAssistPanel(wxWindow* parent, xLightsFrame* xl_frame) override;
    bool HasAssistPanel() override
    {
        return true;
    }

protected:
    xlEffectPanel *CreatePanel(wxWindow *parent) override;
private:
    void SetDMXColorPixel(int chan, uint8_t value, RenderBuffer &buffer);    

    int ScaleToDMX(float value, float degresOfMovement) const;


    DMXPathAssistPanel* m_dmxAssistPanel = nullptr;

};

