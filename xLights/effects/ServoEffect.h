#ifndef SERVOEFFECT_H
#define SERVOEFFECT_H

#include "RenderableEffect.h"

#define SERVO_MIN 0
#define SERVO_MAX 1000
#define SERVO_DIVISOR 10

class ServoEffect : public RenderableEffect
{
    public:
        ServoEffect(int id);
        virtual ~ServoEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
        virtual void SetPanelStatus(Model *cls) override;
        virtual void SetDefaultParameters() override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        virtual bool needToAdjustSettings(const std::string& version) override { return false; }
        int GetPhonemeValue(RenderBuffer& buffer, SequenceElements* elements, const std::string& trackName);
};
#endif // SERVOEFFECT_H