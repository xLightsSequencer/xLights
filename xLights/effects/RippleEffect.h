#ifndef RIPPLEEFFECT_H
#define RIPPLEEFFECT_H

#include "RenderableEffect.h"

#include "../Color.h"

#define RIPPLE_CYCLES_MIN 0
#define RIPPLE_CYCLES_MAX 300

#define RIPPLE_THICKNESS_MIN 1
#define RIPPLE_THICKNESS_MAX 100

#define RIPPLE_ROTATION_MIN 0
#define RIPPLE_ROTATION_MAX 360

class RippleEffect : public RenderableEffect
{
    public:
        RippleEffect(int id);
        virtual ~RippleEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool AppropriateOnNodes() const override { return false; }
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
    
    void Drawcircle(RenderBuffer &buffer, int Movement, int xc, int yc, double radius,HSVValue &hsv, int Ripple_Thickness,int CheckBox_Ripple3D);
    void Drawsquare(RenderBuffer &buffer, int Movement, int x1, int x2, int y1, int y2, int Ripple_Thickness, int CheckBox_Ripple3D, HSVValue &hsv);
    void Drawtriangle(RenderBuffer &buffer, int Movement, int xc, int yc, double side, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
    void Drawstar(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, int points, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D, float rotation);
	
	void Drawheart(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
	void Drawpolygon(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, int points, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D, float rotation);
	void Drawsnowflake(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, int points, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D, double rotation);
	void Drawtree(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
	void Drawcandycane(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
	void Drawcrucifix(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D);
	void Drawpresent(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D);

};

#endif // RIPPLEEFFECT_H
