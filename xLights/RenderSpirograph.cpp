#include <cmath>

#include "RgbEffects.h"

void RgbEffects::RenderSpirograph(int int_R, int int_r, int int_d,bool Animate)
{
#define PI 3.14159265
    int i,x,y,k,xc,yc,ColorIdx;
    int mod1440,state360,d_mod;
    srand(1);
    float R,r,d,d_orig,t;
    double hyp,x2,y2;
    wxImage::HSVValue hsv,hsv0,hsv1; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    size_t colorcnt=GetColorCount();

    xc= (int)(BufferWi/2); // 20x100 flex strips with 2 fols per strip = 40x50
    yc= (int)(BufferHt/2);
    R=xc*(int_R/100.0);   //  Radius of the large circle just fits in the width of model
    r=xc*(int_r/100.0); // start little circle at 1/4 of max width
    if(r>R) r=R;
    d=xc*(int_d/100.0);

    //  palette.GetHSV(1, hsv1);
    //
    //    A hypotrochoid is a roulette traced by a point attached to a circle of radius r rolling around the inside of a fixed circle of radius R, where the point is a distance d from the center of the interior circle.
    //The parametric equations for a hypotrochoid are:[citation needed]
    //
    //  more info: http://en.wikipedia.org/wiki/Hypotrochoid
    //
    //x(t) = (R-r) * cos t + d*cos ((R-r/r)*t);
    //y(t) = (R-r) * sin t + d*sin ((R-r/r)*t);

    mod1440  = state%1440;
    state360 = state%360;
    d_orig=d;
    for(i=1; i<=360; i++)
    {
        if(Animate) d = (int)(d_orig+state/2)%100; // should we modify the distance variable each pass through?
        t = (i+mod1440)*PI/180;
        x = (R-r) * cos (t) + d*cos (((R-r)/r)*t) + xc;
        y = (R-r) * sin (t) + d*sin (((R-r)/r)*t) + yc;

        if(colorcnt>0) d_mod = (int) BufferWi/colorcnt;
        else d_mod=1;

        x2= pow ((x-xc),2);
        y2= pow ((y-yc),2);
        hyp = (sqrt(x2 + y2)/BufferWi) * 100.0;
        ColorIdx=(int)(hyp / d_mod); // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked

        if(ColorIdx>=colorcnt) ColorIdx=colorcnt-1;

        palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx


        palette.GetHSV(0, hsv0);
        ColorIdx=(state+rand()) % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
        palette.GetHSV(ColorIdx, hsv1); // Now go and get the hsv value for this ColorIdx

        SetPixel(x,y,hsv);
//        if(i<=state360) SetPixel(x,y,hsv); // Turn pixel on
//        else SetPixel(x,y,hsv1);
    }
}
