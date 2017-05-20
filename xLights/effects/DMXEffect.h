#ifndef DMXEFFECT_H
#define DMXEFFECT_H

#include "RenderableEffect.h"

#define DMX_MIN 0
#define DMX_MAX 255

class DMXEffect : public RenderableEffect
{
    public:
        DMXEffect(int id);
        virtual ~DMXEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual void SetPanelStatus(Model *cls) override;
        virtual void SetDefaultParameters(Model *cls) override;
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        virtual bool needToAdjustSettings(const std::string& version) override { return true; };
        virtual void adjustSettings(const std::string &version, Effect *effect) override;
    private:
        bool SetDMXSinglColorPixel(int chan, int num_channels, const SettingsMap &SettingsMap, double eff_pos, xlColor& color, RenderBuffer &buffer);
        bool SetDMXRGBNode(int node, int num_channels, const SettingsMap &SettingsMap, double eff_pos, xlColor& color, RenderBuffer &buffer, const std::string& string_type);
        void SetColorBasedOnStringType(int value, int slot, xlColor& color, const std::string& string_type);
};

#endif // DMXEFFECT_H
