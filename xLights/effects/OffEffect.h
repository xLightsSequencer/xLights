#ifndef OFFEFFECT_H
#define OFFEFFECT_H

#include "RenderableEffect.h"

class OffEffect : public RenderableEffect
{
    public:
        OffEffect(int id);
        virtual ~OffEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual std::string GetEffectString() override { return "";};

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // OFFEFFECT_H
