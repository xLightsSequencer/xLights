#include "RippleEffect.h"
#include "RipplePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"


#include "../../include/ripple-16.xpm"
#include "../../include/ripple-24.xpm"
#include "../../include/ripple-32.xpm"
#include "../../include/ripple-48.xpm"
#include "../../include/ripple-64.xpm"

RippleEffect::RippleEffect(int id) : RenderableEffect(id, "Ripple", ripple_16, ripple_24, ripple_32, ripple_48, ripple_64)
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
#define RENDER_RIPPLE_STAR       3

#define MOVEMENT_EXPLODE    0
#define MOVEMENT_IMPLODE    1

void RippleEffect::SetDefaultParameters(Model *cls) {
    RipplePanel *rp = (RipplePanel*)panel;
    if (rp == nullptr) {
        return;
    }

    rp->BitmapButton_Ripple_CyclesVC->SetActive(false);
    rp->BitmapButton_Ripple_ThicknessVC->SetActive(false);

    SetChoiceValue(rp->Choice_Ripple_Object_To_Draw, "Circle");
    SetChoiceValue(rp->Choice_Ripple_Movement, "Explode");

    SetSliderValue(rp->Slider_Ripple_Thickness, 3);
    SetSliderValue(rp->Slider_Ripple_Cycles, 10);
    SetSliderValue(rp->Slider_Ripple_Points, 5);

    SetCheckBoxValue(rp->CheckBox_Ripple3D, false);
}

void RippleEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    const std::string &Object_To_DrawStr = SettingsMap["CHOICE_Ripple_Object_To_Draw"];
    const std::string &MovementStr = SettingsMap["CHOICE_Ripple_Movement"];
    int Ripple_Thickness = GetValueCurveInt("Ripple_Thickness", 3, SettingsMap, oset, RIPPLE_THICKNESS_MIN, RIPPLE_THICKNESS_MAX);
    bool CheckBox_Ripple3D = SettingsMap.GetBool("CHECKBOX_Ripple3D", false);
    float cycles = GetValueCurveDouble("Ripple_Cycles", 1.0, SettingsMap, oset, RIPPLE_CYCLES_MIN, RIPPLE_CYCLES_MAX, 10);
    int points = SettingsMap.GetInt("SLIDER_RIPPLE_POINTS", 5);

    int Object_To_Draw;
    if (Object_To_DrawStr == "Circle")
    {
        Object_To_Draw = RENDER_RIPPLE_CIRCLE;
    }
    else if (Object_To_DrawStr == "Square")
    {
        Object_To_Draw = RENDER_RIPPLE_SQUARE;
    }
    else if (Object_To_DrawStr == "Triangle")
    {
        Object_To_Draw = RENDER_RIPPLE_TRIANGLE;
    }
    else
    {
        Object_To_Draw = RENDER_RIPPLE_STAR;
    }
    int Movement = "Explode" == MovementStr ? MOVEMENT_EXPLODE : MOVEMENT_IMPLODE;
    
    int ColorIdx;
    int xc,yc;
    
    HSVValue hsv;
    size_t colorcnt=buffer.GetColorCount();
    
    double position = buffer.GetEffectTimeIntervalPosition(cycles); // how far are we into the row> value is 0.0 to 1.0
    
    float rx = position;
    xc = buffer.BufferWi/2;
    yc = buffer.BufferHt/2;
    
    ColorIdx=static_cast<int>(rx * colorcnt);
    if(ColorIdx==colorcnt) ColorIdx--; // ColorIdx goes from 0-3 when colorcnt goes from 1-4. Make sure that is true
    
    double radius;
    double side;
    buffer.palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
    switch (Object_To_Draw)
    {
            
        case RENDER_RIPPLE_SQUARE:
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
            break;
        case RENDER_RIPPLE_STAR:
            if (Movement == MOVEMENT_IMPLODE)
                radius = xc - (xc*rx);
            else
                radius = (xc*rx);
            Drawstar(buffer, Movement, xc, yc, radius, points, hsv, Ripple_Thickness, CheckBox_Ripple3D);
            break;
        case RENDER_RIPPLE_TRIANGLE:
            if (Movement == MOVEMENT_IMPLODE)
                side = xc - (xc*rx);
            else
                side = (xc*rx);

            Drawtriangle(buffer, Movement, xc, yc, side, hsv, Ripple_Thickness, CheckBox_Ripple3D);
            break;
    }
}

void RippleEffect::Drawtriangle(RenderBuffer &buffer, int Movement, int xc, int yc, double side, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
{
    int i;
    xlColor color(hsv);

#define ROOT3DIV3 0.577350269
#define SIN30 0.5
#define COS30 0.866025404

    for (i = 0; i<Ripple_Thickness; i++)
    {
        double radius = (side + i) * ROOT3DIV3;
        double ytop = yc + radius;
        int xtop = xc;

        double xleft = xc - radius * COS30;
        double yleft = yc - radius * SIN30;

        double xright = xleft + side + i;
        double yright = yleft;

        if (CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0 - ((float(i) / 2.0) / float(Ripple_Thickness)));
            }
            else {
                hsv.value *= 1.0 - ((float(i) / 2.0) / float(Ripple_Thickness)); // we multiply by 1.0 when Ripple_Thickness=0
                color = hsv;
            }
        }
        buffer.DrawLine(xtop, ytop, xleft, yleft, color);
        buffer.DrawLine(xtop, ytop, xright, yright, color);
        buffer.DrawLine(xleft, yleft, xright, yright, color);
    }
}

void RippleEffect::Drawsquare(RenderBuffer &buffer, int Movement, int x1, int x2, int y1,int y2,int Ripple_Thickness,int CheckBox_Ripple3D,HSVValue &hsv)
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
void RippleEffect::Drawcircle(RenderBuffer &buffer, int Movement,int xc,int yc,double radius,HSVValue &hsv, int Ripple_Thickness,int CheckBox_Ripple3D)
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
            radius = radius + i;
        }
        else
        {
            radius = radius - i;
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

void RippleEffect::Drawstar(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, int points, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
{
    double offsetangle = 0.0;
    switch (points)
    {
    case 4:
        break;
    case 5:
        offsetangle = 90.0 - 360.0 / 5;
        break;
    case 6:
        offsetangle = 30.0;
        break;
    case 7:
        offsetangle = 90.0 - 360.0 / 7;
        break;
    }

    xlColor color(hsv);

    for (double i = 0; i<Ripple_Thickness; i += .5)
    {
        if (CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0 - (float(i) / float(Ripple_Thickness)));
            }
            else {
                hsv.value *= 1.0 - (float(i) / float(Ripple_Thickness)); // we multiply by 1.0 when steps=0
                color = hsv;
            }
        }
        if (Movement == MOVEMENT_EXPLODE)
        {
            radius = radius + i;
        }
        else
        {
            radius = radius - i;
        }
        double InnerRadius = radius / 2.618034;    // divide by golden ratio squared

        double increment = 360.0 / points;

        for (double degrees = 0.0; degrees<361.0; degrees += increment) // 361 because it allows for small rounding errors
        {
            if (degrees > 360.0) degrees = 360.0;
            double radian = (offsetangle + degrees) * (M_PI / 180.0);
            int xouter = radius * cos(radian) + xc;
            int youter = radius * sin(radian) + yc;

            radian = (offsetangle + degrees + increment / 2.0) * (M_PI / 180.0);
            int xinner = InnerRadius * cos(radian) + xc;
            int yinner = InnerRadius * sin(radian) + yc;

            buffer.DrawLine(xinner, yinner, xouter, youter, color);

            radian = (offsetangle + degrees - increment / 2.0) * (M_PI / 180.0);
            xinner = InnerRadius * cos(radian) + xc;
            yinner = InnerRadius * sin(radian) + yc;

            buffer.DrawLine(xinner, yinner, xouter, youter, color);
        }
    }
}

