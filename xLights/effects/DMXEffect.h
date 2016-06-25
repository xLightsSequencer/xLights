#ifndef DMXEFFECT_H
#define DMXEFFECT_H

#include "RenderableEffect.h"


class DMXEffect : public RenderableEffect
{
    public:
        DMXEffect(int id);
        virtual ~DMXEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        virtual bool needToAdjustSettings(const std::string& version) override { return true; };
        virtual void adjustSettings(const std::string &version, Effect *effect) override;
    private:
};

#endif // DMXEFFECT_H
