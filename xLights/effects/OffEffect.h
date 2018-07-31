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
        virtual std::string GetEffectString() override;
        virtual void SetDefaultParameters() override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // OFFEFFECT_H
