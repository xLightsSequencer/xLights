#ifndef PINWHEELEFFECT_H
#define PINWHEELEFFECT_H

#include "RenderableEffect.h"

class xlColor;

class PinwheelEffect : public RenderableEffect
{
    public:
        PinwheelEffect(int id);
        virtual ~PinwheelEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
    
        void Draw_arm(RenderBuffer &buffer,
                      int base_degrees,int max_radius,int pinwheel_twist,
                      const xlColor &rgb,int xc_adj,int yc_adj);

};

#endif // PINWHEELEFFECT_H
