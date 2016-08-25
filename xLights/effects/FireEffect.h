#ifndef FIREEFFECT_H
#define FIREEFFECT_H

#include "RenderableEffect.h"


class FireEffect : public RenderableEffect
{
    public:
        FireEffect(int id);
        virtual ~FireEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // FIREEFFECT_H
