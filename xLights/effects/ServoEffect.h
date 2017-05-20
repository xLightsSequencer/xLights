#ifndef SERVOEFFECT_H
#define SERVOEFFECT_H

#include "RenderableEffect.h"

#define SERVO_MIN 0
#define SERVO_MAX 100

class ServoEffect : public RenderableEffect
{
    public:
        ServoEffect(int id);
        virtual ~ServoEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual void SetPanelStatus(Model *cls) override;
        virtual void SetDefaultParameters(Model *cls) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        virtual bool needToAdjustSettings(const std::string& version) override { return false; }
};
#endif // SERVOEFFECT_H