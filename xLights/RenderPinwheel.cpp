#include <cmath>

#include "RgbEffects.h"


void RgbEffects::RenderPinwheel(int int_R)
{
    int i,x,y,xc,yc,ColorIdx;
    int mod1440,state360,d_mod;
    srand(1);
    float R,r,radius,d,d_orig,t;
    double hyp,x2,y2;
    wxImage::HSVValue hsv,hsv0,hsv1; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    size_t colorcnt=GetColorCount();

    xc= (int)(BufferWi/2); // 20x100 flex strips with 2 fols per strip = 40x50
    yc= (int)(BufferHt/2);
    R=xc*(int_R/100.0);   //  Radius of the large circle just fits in the width of model
    r=xc*(int_R/100.0); // start little circle at 1/4 of max width
    if(r>R) r=R;
    d=xc*(int_R*2/100.0);

    mod1440  = state%1440;
    state360 = state%360;
    d_orig=d;

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
    for(i=istart; i<=iend; i++)
    {
        phi = i * pi_180;
        radius =  (phi);
        x = radius * cos (phi) + xc;
        y = radius * sin (phi) + yc;
        ColorIdx=colorcnt=0;
        palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
        hsv.hue = phi/PI6;
        SetPixel(x,y,hsv);
    }
}




