#ifndef PINWHEELEFFECT_H
#define PINWHEELEFFECT_H

#include "RenderableEffect.h"

class xlColor;

class PinwheelEffect : public RenderableEffect
{
    public:
        PinwheelEffect(int id);
        virtual ~PinwheelEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsRadialColorCurves(const SettingsMap &SettingsMap) { return true; }
    
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;

};

#endif // PINWHEELEFFECT_H
