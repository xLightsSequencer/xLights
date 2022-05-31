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

#define DMX_MIN 0
#define DMX_MAX 255

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
        virtual bool CanBeRandom() override {return true;}
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual void SetPanelStatus(Model *cls) override;
        virtual void SetDefaultParameters() override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }
    protected:
        virtual xlEffectPanel *CreatePanel(wxWindow *parent) override;
    private:
        void SetDMXColorPixel(int chan, uint8_t value, RenderBuffer &buffer);
        std::pair<uint8_t, uint8_t> renderPath(DMXPathType effectType, double eff_pos, long length, int height, int width, int x_off, int y_off, int rot);
        std::pair<float, float> calcLocation(DMXPathType effectType, float degpos);
        DMXPathType DecodeType(const std::string& shape);

};

