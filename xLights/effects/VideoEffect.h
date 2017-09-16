#ifndef VIDEOEFFECT_H
#define VIDEOEFFECT_H

#include "RenderableEffect.h"
#include "../RenderBuffer.h"

class VideoEffect : public RenderableEffect
{
    public:
        VideoEffect(int id);
        virtual ~VideoEffect();
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        void Render(RenderBuffer &buffer,
					std::string filename, double starttime, bool keepaspectratio, std::string durationTreatment, bool synchroniseAudio);
        virtual bool CanBeRandom() override {return false;}
        virtual void SetDefaultParameters(Model *cls) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
        virtual std::list<std::string> GetFileReferences(const SettingsMap &SettingsMap) override;
        static bool IsVideo(const std::string& file);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent)override;
        virtual bool needToAdjustSettings(const std::string& version) override { return true; };
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
};

#endif // VIDEOEFFECT_H
