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
        virtual bool SupportsRadialColorCurves(const SettingsMap &SettingsMap) override { return true; }
        virtual bool needToAdjustSettings(const std::string &version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        virtual void Draw_arm(RenderBuffer &buffer, int base_degrees,int max_radius,int pinwheel_twist, const xlColor &rgb,int xc_adj,int yc_adj);

};

#endif // PINWHEELEFFECT_H
