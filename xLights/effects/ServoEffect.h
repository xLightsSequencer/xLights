#ifndef SERVOEFFECT_H
#define SERVOEFFECT_H

#include "RenderableEffect.h"


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
        virtual bool needToAdjustSettings(const std::string& version) override { return false; };
        //virtual void adjustSettings(const std::string &version, Effect *effect) override;
    private:

};

#endif // SERVOEFFECT_H
