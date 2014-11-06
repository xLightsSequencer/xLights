#include <cmath>

#include "RgbEffects.h"


void RgbEffects::RenderPinwheel(int pinwheel_arms, int pinwheel_twist,
                                int pinwheel_thickness,bool pinwheel_rotation,int pinwheel_3d)
{
    int i,a,x,y,xc,yc,ColorIdx,base_degrees;
    int mod1440,state360,d_mod;
    float t,tmax;
    srand(1);
    float R,r,radius,d,d_orig;
    double hyp,x2,y2;
    wxImage::HSVValue hsv,hsv0,hsv1; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    size_t colorcnt=GetColorCount();

    xc= (int)(BufferWi/2); // 20x100 flex strips with 2 fols per strip = 40x50
    yc= (int)(BufferHt/2);


    //
    //  phi = a +b*phi
    float phi;
    radius = xc/100.0;
    float PI6 = M_PI*6;
    float PI2 = M_PI*2;
    float pi_180 = M_PI/2;
    float phi_step = PI6/300.0;
    float calc_phi;
    int istart=state%1440;
    int iend = istart+90;

    ColorIdx=rand()% colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx


    int degrees_per_arm=1;
    if(pinwheel_arms>0) degrees_per_arm= 360/pinwheel_arms;
    for(a=1; a<=pinwheel_arms; a++)
    {

        ColorIdx=a%colorcnt;
        palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
        base_degrees = (a-1)*degrees_per_arm + state;
        Draw_arm( base_degrees, xc, pinwheel_twist,hsv);
        if(pinwheel_thickness>0)
        {
            tmax= (pinwheel_thickness/100.0)*degrees_per_arm/2.0;
            hsv1=hsv;
            for (t=1; t<=tmax; t++)
            {
                if(pinwheel_3d==1)
                {
                    hsv1.value = hsv.value * ((tmax-t)/tmax);
                }
                else if(pinwheel_3d==2)
                {
                    hsv1.value = hsv.value * ((t)/tmax);
                }
                Draw_arm( base_degrees-t, xc, pinwheel_twist,hsv1);
                Draw_arm( base_degrees+t, xc, pinwheel_twist,hsv1);
            }
        }
    }
}

void RgbEffects::Draw_arm( int base_degrees,int max_radius,int pinwheel_twist,wxImage::HSVValue hsv)
{
    float r,phi;
    int x,y,xc,yc;
    float pi_180 = M_PI/180;
    int degrees_twist,degrees;
    size_t colorcnt=GetColorCount();
    xc= (int)(BufferWi/2); // 20x100 flex strips with 2 fols per strip = 40x50
    yc= (int)(BufferHt/2);

    for(r=0.0; r<=max_radius; r+=0.5)
    {
        degrees_twist=(r/max_radius)*pinwheel_twist;
        degrees = base_degrees + degrees_twist;

        phi = degrees * pi_180;

        x = r * cos (phi) + xc;
        y = r * sin (phi) + yc;


        SetPixel(x,y,hsv);
    }
}




