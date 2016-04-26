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
    private:
};

#endif // DMXEFFECT_H
