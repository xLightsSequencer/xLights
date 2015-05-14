#include <cmath>

#include "RgbEffects.h"


void RgbEffects::RenderPinwheel(int pinwheel_arms, int pinwheel_twist,
                                int pinwheel_thickness,bool pinwheel_rotation,
                                const wxString &pinwheel_3d, int xc_adj, int yc_adj,int pinwheel_armsize)
{
    int a,xc,ColorIdx,base_degrees;
    float t,tmax;
    srand(1);
    float radius;
    wxImage::HSVValue hsv,hsv0,hsv1; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    size_t colorcnt=GetColorCount();

    xc= (int)(std::max(BufferWi, BufferHt)/2);


    //
    //  phi = a +b*phi
    radius = xc/100.0;

    ColorIdx=rand()% colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx


    int degrees_per_arm=1;
    if(pinwheel_arms>0) degrees_per_arm= 360/pinwheel_arms;
    float armsize = (pinwheel_armsize/100.0);
    for(a=1; a<=pinwheel_arms; a++)
    {

        ColorIdx=a%colorcnt;
        palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
        if(pinwheel_rotation==1) // do we have CW rotation
        {
            base_degrees = (a-1)*degrees_per_arm + state; // yes
        }
        else
        {
            base_degrees = (a-1)*degrees_per_arm - state; // no, we are CCW
        }
        Draw_arm( base_degrees, xc*armsize, pinwheel_twist,hsv,xc_adj,yc_adj);
        if(pinwheel_thickness>0)
        {
            tmax= (pinwheel_thickness/100.0)*degrees_per_arm/2.0;
            hsv1=hsv;
            xlColor color(hsv1);
            for (t=1; t<=tmax; t++)
            {
                if(pinwheel_3d=="3D")
                {
                    if (allowAlpha) {
                        color.alpha = 255.0 * ((tmax-t)/tmax);
                    } else {
                        hsv1.value = hsv.value * ((tmax-t)/tmax);
                        color = hsv1;
                    }
                }
                else if(pinwheel_3d=="3D Inverted")
                {
                    if (allowAlpha) {
                        color.alpha = 255.0 * ((t)/tmax);
                    } else {
                        hsv1.value = hsv.value * ((t)/tmax);
                        color = hsv1;
                    }
                }
                Draw_arm( base_degrees-t, xc*armsize, pinwheel_twist,color,xc_adj,yc_adj);
                Draw_arm( base_degrees+t, xc*armsize, pinwheel_twist,color,xc_adj,yc_adj);
            }
        }
    }
}

void RgbEffects::Draw_arm( int base_degrees,int max_radius,int pinwheel_twist,
                           const xlColor &rgb,int xc_adj,int yc_adj)
{
    float r,phi;
    int x,y,xc,yc;
    float pi_180 = M_PI/180;
    int degrees_twist,degrees;
    xc= (int)(BufferWi/2);
    yc= (int)(BufferHt/2);
    xc = xc + (xc_adj/100.0)*xc; // xc_adj is from -100 to 100
    yc = yc + (yc_adj/100.0)*yc;


    for(r=0.0; r<=max_radius; r+=0.5)
    {
        degrees_twist=(r/max_radius)*pinwheel_twist;
        degrees = base_degrees + degrees_twist;
        phi = degrees * pi_180;
        x = r * cos (phi) + xc;
        y = r * sin (phi) + yc;
        SetPixel(x, y, rgb);
    }
}




