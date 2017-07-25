#ifndef FILLEFFECT_H
#define FILLEFFECT_H

#include "RenderableEffect.h"

class FillEffect : public RenderableEffect
{
    public:
        FillEffect(int id);
        virtual ~FillEffect();

        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool needToAdjustSettings(const std::string &version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
        virtual void SetDefaultParameters(Model *cls) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // FILLEFFECT_H
