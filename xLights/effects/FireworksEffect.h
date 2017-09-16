#ifndef FIREWORKSEFFECT_H
#define FIREWORKSEFFECT_H

#include "RenderableEffect.h"


class FireworksEffect : public RenderableEffect
{
    public:
        FireworksEffect(int id);
        virtual ~FireworksEffect();
        virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void SetPanelStatus(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        void SetPanelTimingTracks();
};

#endif // FIREWORKSEFFECT_H
