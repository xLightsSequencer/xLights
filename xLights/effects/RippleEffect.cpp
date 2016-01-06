#include "RippleEffect.h"
#include "RipplePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

RippleEffect::RippleEffect(int id) : RenderableEffect(id, "Ripple")
{
    //ctor
}

RippleEffect::~RippleEffect()
{
    //dtor
}
wxPanel *RippleEffect::CreatePanel(wxWindow *parent) {
    return new RipplePanel(parent);
}

#define RENDER_RIPPLE_CIRCLE     0
#define RENDER_RIPPLE_SQUARE     1
#define RENDER_RIPPLE_TRIANGLE   2

#define MOVEMENT_EXPLODE    0
#define MOVEMENT_IMPLODE    1

void RippleEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    const std::string &Object_To_DrawStr = SettingsMap["CHOICE_Ripple_Object_To_Draw"];
    const std::string &MovementStr = SettingsMap["CHOICE_Ripple_Object_To_Draw"];
    int Ripple_Thickness = SettingsMap.GetInt("SLIDER_Ripple_Thickness", 1);
    bool CheckBox_Ripple3D = SettingsMap.GetBool("CHECKBOX_Ripple3D");
    float cycles = SettingsMap.GetFloat("TEXTCTRL_Ripple_Cycles", 1.0);
    
    int Object_To_Draw = "Circle" == Object_To_DrawStr ? RENDER_RIPPLE_CIRCLE :
        "Square" == Object_To_DrawStr ? RENDER_RIPPLE_SQUARE : RENDER_RIPPLE_TRIANGLE;
    int Movement = "Explode" == MovementStr ? MOVEMENT_EXPLODE : MOVEMENT_IMPLODE;
    
    int i,ColorIdx;
    int xc,yc;
    
    wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    size_t colorcnt=buffer.GetColorCount();
    
    i=0;
    double position = buffer.GetEffectTimeIntervalPosition(cycles); // how far are we into the row> value is 0.0 to 1.0
    
    float rx = position;
    xc = buffer.BufferWi/2;
    yc = buffer.BufferHt/2;
    
    ColorIdx=(int)(rx * colorcnt);
    if(ColorIdx==colorcnt) ColorIdx--; // ColorIdx goes from 0-3 when colorcnt goes from 1-4. Make sure that is true
    
    double radius;
    buffer.palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
    int explode;
    switch (Object_To_Draw)
    {
            
        case RENDER_RIPPLE_SQUARE:
            explode=1;
            if(Movement==MOVEMENT_EXPLODE)
            {
                // This is the object expanding out, or explode looikng
                int x1 = xc - (xc*rx);
                int x2 = xc + (xc*rx);
                int y1 = yc - (yc*rx);
                int y2 = yc + (yc*rx);
                Drawsquare(buffer, Movement,  x1,  x2,  y1, y2, Ripple_Thickness, CheckBox_Ripple3D,hsv);
            }
            else if(Movement==MOVEMENT_IMPLODE)
            {
                int x1 = (xc*rx);
                int x2 = buffer.BufferWi - (xc*rx);
                int y1 =  (yc*rx);
                int y2 = buffer.BufferHt - (yc*rx);
                Drawsquare(buffer, Movement,  x1,  x2,  y1, y2, Ripple_Thickness, CheckBox_Ripple3D,hsv);
            }
            break;
        case RENDER_RIPPLE_CIRCLE:
            if(Movement==MOVEMENT_IMPLODE)
                radius = xc-(xc*rx);
            else
                radius = (xc*rx);
            
            
            Drawcircle(buffer, Movement,xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
            radius=radius/2;
            /*Drawcircle( Movement,xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
             radius=radius/2;
             Drawcircle( Movement,xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
             radius=radius/2;
             Drawcircle( Movement,xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
             */
            break;
        case RENDER_RIPPLE_TRIANGLE:
            break;
    }
}

void RippleEffect::Drawsquare(RenderBuffer &buffer, int Movement, int x1, int x2, int y1,int y2,int Ripple_Thickness,int CheckBox_Ripple3D,wxImage::HSVValue hsv)
{
    int i,x,y;
    xlColor color(hsv);
    
    for (i=0; i<Ripple_Thickness; i++)
    {
        if (CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0-((float(i)/2.0)/float(Ripple_Thickness)));
            } else {
                hsv.value *= 1.0-((float(i)/2.0)/float(Ripple_Thickness)); // we multiply by 1.0 when Ripple_Thickness=0
                color = hsv;
            }
        }
        if(Movement==MOVEMENT_EXPLODE)
        {
            for(y=y1+i; y<=y2-i; y++)
            {
                buffer.SetPixel(x1+i,y,color); // Turn pixel
                buffer.SetPixel(x2-i,y,color); // Turn pixel
            }
            for(x=x1+i; x<=x2-i; x++)
            {
                buffer.SetPixel(x,y1+i,color); // Turn pixel
                buffer.SetPixel(x,y2-i,color); // Turn pixel
            }
        }
        if(Movement==MOVEMENT_IMPLODE)
        {
            for(y=y2+i; y>=y1-i; y--)
            {
                buffer.SetPixel(x1-i,y,color); // Turn pixel
                buffer.SetPixel(x2+i,y,color); // Turn pixel
            }
            for(x=x2+i; x>=x1-i; x--)
            {
                buffer.SetPixel(x,y1-i,color); // Turn pixel
                buffer.SetPixel(x,y2+i,color); // Turn pixel
            }
        }
    }
}
void RippleEffect::Drawcircle(RenderBuffer &buffer, int Movement,int xc,int yc,double radius,wxImage::HSVValue hsv, int Ripple_Thickness,int CheckBox_Ripple3D)
{
    double degrees,radian;
    int x,y;
    float i;
    xlColor color(hsv);
    
    for (i=0; i<Ripple_Thickness; i+=.5)
    {
        if(CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0- (float(i)/float(Ripple_Thickness)));
            } else {
                hsv.value *= 1.0-(float(i)/float(Ripple_Thickness)); // we multiply by 1.0 when steps=0
                color = hsv;
            }
        }
        if(Movement==MOVEMENT_EXPLODE)
        {
            radius = radius - i;
        }
        else
        {
            radius = radius + i;
        }
        for (degrees=0.0; degrees<360.0; degrees+=1.0)
        {
            radian = degrees * (M_PI/180.0);
            x = radius * cos(radian) + xc;
            y = radius * sin(radian) + yc;
            buffer.SetPixel(x,y,color); // Turn pixel
        }
    }
    
}

