#ifndef FIREEFFECT_H
#define FIREEFFECT_H

#include "RenderableEffect.h"

#define FIRE_GROWTHCYCLES_MIN 0
#define FIRE_GROWTHCYCLES_MAX 200
#define FIRE_GROWTHCYCLES_DIVISOR 10

#define FIRE_HEIGHT_MIN 1
#define FIRE_HEIGHT_MAX 100

#define FIRE_HUE_MIN 0
#define FIRE_HUE_MAX 100

class FireEffect : public RenderableEffect
{
    public:
        FireEffect(int id);
        virtual ~FireEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
protected:
    virtual bool needToAdjustSettings(const std::string &version) override;
    virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
    virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // FIREEFFECT_H
