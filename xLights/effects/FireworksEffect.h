#ifndef FIREWORKSEFFECT_H
#define FIREWORKSEFFECT_H

#include "RenderableEffect.h"

#define FIREWORKSCOUNT_MIN 1
#define FIREWORKSCOUNT_MAX 100

#define FIREWORKSFADE_MIN 1
#define FIREWORKSFADE_MAX 100

#define FIREWORKSVELOCITY_MIN 1
#define FIREWORKSVELOCITY_MAX 10

#define FIREWORKSXVELOCITY_MIN -100
#define FIREWORKSXVELOCITY_MAX 100

#define FIREWORKSYVELOCITY_MIN -100
#define FIREWORKSYVELOCITY_MAX 100

#define FIREWORKSXLOCATION_MIN -1
#define FIREWORKSXLOCATION_MAX 100

#define FIREWORKSYLOCATION_MIN -1
#define FIREWORKSYLOCATION_MAX 100

class FireworksEffect : public RenderableEffect
{
    public:
        FireworksEffect(int id);
        virtual ~FireworksEffect();
        virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
        virtual void SetDefaultParameters() override;
        virtual void SetPanelStatus(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
        virtual bool AppropriateOnNodes() const override { return false; }
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        void SetPanelTimingTracks() const;
        static std::pair<int, int> GetFireworkLocation(int width, int height, int overridex = -1, int overridey = -1);
        virtual bool needToAdjustSettings(const std::string &version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
};
#endif // FIREWORKSEFFECT_H