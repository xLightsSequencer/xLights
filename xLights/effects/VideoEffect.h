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
					std::string filename, double starttime, int cropLeft, int cropRight, int cropTop, int cropBottom, bool keepaspectratio, std::string durationTreatment, bool synchroniseAudio);
        virtual bool CanBeRandom() override {return false;}
        virtual void SetDefaultParameters(Model *cls) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
        virtual std::list<std::string> GetFileReferences(const SettingsMap &SettingsMap) override;
        virtual bool AppropriateOnNodes() const override { return false; }
        static bool IsVideoFile(std::string filename);

        // Currently not possible but I think changes could be made to make it support partial
        //virtual bool CanRenderPartialTimeInterval() const override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent)override;
        virtual bool needToAdjustSettings(const std::string& version) override { return true; };
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
};

#endif // VIDEOEFFECT_H
