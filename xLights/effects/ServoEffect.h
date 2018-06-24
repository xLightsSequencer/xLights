#ifndef SERVOEFFECT_H
#define SERVOEFFECT_H

#include "RenderableEffect.h"

#define SERVO_MIN 0
#define SERVO_MAX 100
#define SERVO_DIVISOR 10

class ServoEffect : public RenderableEffect
{
    public:
        ServoEffect(int id);
        virtual ~ServoEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual void SetPanelStatus(Model *cls) override;
        virtual void SetDefaultParameters() override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        virtual bool needToAdjustSettings(const std::string& version) override { return false; }
};
#endif // SERVOEFFECT_H