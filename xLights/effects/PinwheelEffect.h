#ifndef PINWHEELEFFECT_H
#define PINWHEELEFFECT_H

#include "RenderableEffect.h"

class xlColor;

#define PINWHEEL_X_MIN -100
#define PINWHEEL_X_MAX 100

#define PINWHEEL_Y_MIN -100
#define PINWHEEL_Y_MAX 100

#define PINWHEEL_ARMSIZE_MIN 0
#define PINWHEEL_ARMSIZE_MAX 400

#define PINWHEEL_SPEED_MIN 0
#define PINWHEEL_SPEED_MAX 50

#define PINWHEEL_THICKNESS_MIN 0
#define PINWHEEL_THICKNESS_MAX 100

#define PINWHEEL_TWIST_MIN -360
#define PINWHEEL_TWIST_MAX 360

class PinwheelEffect : public RenderableEffect
{
    public:
        PinwheelEffect(int id);
        virtual ~PinwheelEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsRadialColorCurves(const SettingsMap &SettingsMap) override { return true; }
        virtual bool needToAdjustSettings(const std::string &version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }
        virtual bool SupportsRenderCache(const SettingsMap& settings) const override { return true; }

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
