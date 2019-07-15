#ifndef STROBEEFFECT_H
#define STROBEEFFECT_H

#include "RenderableEffect.h"


class StrobeEffect : public RenderableEffect
{
    public:
        StrobeEffect(int id);
        virtual ~StrobeEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // STROBEEFFECT_H
