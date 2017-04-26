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
        enum Pinwheel3DType {
            PW_3D_NONE,
            PW_3D,
            PW_3D_Inverted,
            PW_SWEEP
        };

    
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        virtual void Draw_arm(RenderBuffer &buffer, int base_degrees,int max_radius,int pinwheel_twist, int xc_adj, int yc_adj, int colorIdx, Pinwheel3DType pinwheel_3d, float round);
    
        Pinwheel3DType to3dType(const std::string &pinwheel_3d);
        void adjustColor(Pinwheel3DType pw3dType, xlColor &color, HSVValue &hsv, bool allowAlpha, float round);

};

#endif // PINWHEELEFFECT_H
