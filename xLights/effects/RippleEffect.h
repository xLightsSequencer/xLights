#ifndef RIPPLEEFFECT_H
#define RIPPLEEFFECT_H

#include "RenderableEffect.h"

#include "../Color.h"

class RippleEffect : public RenderableEffect
{
    public:
        RippleEffect(int id);
        virtual ~RippleEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
    
    void Drawcircle(RenderBuffer &buffer, int Movement, int xc, int yc, double radius,HSVValue &hsv, int Ripple_Thickness,int CheckBox_Ripple3D);
    void Drawsquare(RenderBuffer &buffer, int Movement, int x1, int x2, int y1, int y2, int Ripple_Thickness, int CheckBox_Ripple3D, HSVValue &hsv);
    void Drawtriangle(RenderBuffer &buffer, int Movement, int xc, int yc, double side, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D);

};

#endif // RIPPLEEFFECT_H
