#include "PinwheelEffect.h"
#include "PinwheelPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/pinwheel-16.xpm"
#include "../../include/pinwheel-24.xpm"
#include "../../include/pinwheel-32.xpm"
#include "../../include/pinwheel-48.xpm"
#include "../../include/pinwheel-64.xpm"


PinwheelEffect::PinwheelEffect(int id) : RenderableEffect(id, "Pinwheel", pinwheel_16, pinwheel_24, pinwheel_32, pinwheel_48, pinwheel_64)
{
    //ctor
}

PinwheelEffect::~PinwheelEffect()
{
    //dtor
}
wxPanel *PinwheelEffect::CreatePanel(wxWindow *parent) {
    return new PinwheelPanel(parent);
}

void PinwheelEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {


    int pinwheel_arms = SettingsMap.GetInt("SLIDER_Pinwheel_Arms", 1);
    int pinwheel_twist = SettingsMap.GetInt("SLIDER_Pinwheel_Twist", 1);
    int pinwheel_thickness = SettingsMap.GetInt("SLIDER_Pinwheel_Thickness", 1);
    bool pinwheel_rotation = SettingsMap.GetBool("CHECKBOX_Pinwheel_Rotation");
    const std::string &pinwheel_3d = SettingsMap["CHOICE_Pinwheel_3D"];
    int xc_adj = SettingsMap.GetInt("SLIDER_PinwheelXC", 0);
    int yc_adj = SettingsMap.GetInt("SLIDER_PinwheelYC", 0);
    int pinwheel_armsize = SettingsMap.GetInt("SLIDER_Pinwheel_ArmSize", 1);
    int pspeed = SettingsMap.GetInt("TEXTCTRL_Pinwheel_Speed", 1);
    
    
    int a,xc,ColorIdx,base_degrees;
    float t,tmax;
    float radius;
    HSVValue hsv,hsv0,hsv1;
    size_t colorcnt=buffer.GetColorCount();
    
    xc= (int)(std::max(buffer.BufferWi, buffer.BufferHt)/2);
    //
    //  phi = a +b*phi
    radius = xc/100.0;
    
    double pos = (buffer.curPeriod - buffer.curEffStartPer) * pspeed * buffer.frameTimeInMs / 50;
    
    int degrees_per_arm=1;
    if(pinwheel_arms>0) degrees_per_arm= 360/pinwheel_arms;
    float armsize = (pinwheel_armsize/100.0);
    for(a=1; a<=pinwheel_arms; a++)
    {
        ColorIdx=a%colorcnt;
        buffer.palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
        if(pinwheel_rotation==1) // do we have CW rotation
        {
            base_degrees = (a-1)*degrees_per_arm + pos; // yes
        }
        else
        {
            base_degrees = (a-1)*degrees_per_arm - pos; // no, we are CCW
        }
        Draw_arm(buffer, base_degrees, xc*armsize, pinwheel_twist,hsv,xc_adj,yc_adj);
        if(pinwheel_thickness>0)
        {
            tmax= (pinwheel_thickness/100.0)*degrees_per_arm/2.0;
            hsv1=hsv;
            xlColor color(hsv1);
            for (t=1; t<=tmax; t++)
            {
                if(pinwheel_3d=="3D")
                {
                    if (buffer.allowAlpha) {
                        color.alpha = 255.0 * ((tmax-t)/tmax);
                    } else {
                        hsv1.value = hsv.value * ((tmax-t)/tmax);
                        color = hsv1;
                    }
                }
                else if(pinwheel_3d=="3D Inverted")
                {
                    if (buffer.allowAlpha) {
                        color.alpha = 255.0 * ((t)/tmax);
                    } else {
                        hsv1.value = hsv.value * ((t)/tmax);
                        color = hsv1;
                    }
                }
                Draw_arm(buffer, base_degrees-t, xc*armsize, pinwheel_twist,color,xc_adj,yc_adj);
                Draw_arm(buffer, base_degrees+t, xc*armsize, pinwheel_twist,color,xc_adj,yc_adj);
            }
        }
    }
}

void PinwheelEffect::Draw_arm(RenderBuffer &buffer,
                              int base_degrees,int max_radius,int pinwheel_twist,
                              const xlColor &rgb,int xc_adj,int yc_adj)
{
    float r,phi;
    int x,y,xc,yc;
    float pi_180 = M_PI/180;
    int degrees_twist,degrees;
    xc= (int)(buffer.BufferWi/2);
    yc= (int)(buffer.BufferHt/2);
    xc = xc + (xc_adj/100.0)*xc; // xc_adj is from -100 to 100
    yc = yc + (yc_adj/100.0)*yc;
    
    
    for(r=0.0; r<=max_radius; r+=0.5)
    {
        degrees_twist=(r/max_radius)*pinwheel_twist;
        degrees = base_degrees + degrees_twist;
        phi = degrees * pi_180;
        x = r * cos (phi) + xc;
        y = r * sin (phi) + yc;
        buffer.SetPixel(x, y, rgb);
    }
}