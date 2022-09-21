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

#define PAN_MIN -180
#define PAN_MAX 180

#define TILT_MIN -180
#define TILT_MAX 180

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
protected:
    xlEffectPanel *CreatePanel(wxWindow *parent) override;
private:
    void SetDMXColorPixel(int chan, uint8_t value, RenderBuffer &buffer);
    std::pair<uint8_t, uint8_t> renderPath(DMXPathType effectType, double eff_pos, long length, int height, int width, int x_off, int y_off, int rot);
    std::pair<float, float> calcLocation(DMXPathType effectType, float degpos);
    DMXPathType DecodeType(const std::string& shape) const; 

    int ScaleToDMX(int value, int degresOfMovement) const;
};

