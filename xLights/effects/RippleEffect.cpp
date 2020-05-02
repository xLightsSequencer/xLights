/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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
#define RENDER_RIPPLE_POLYGON    4
#define RENDER_RIPPLE_HEART      5
#define RENDER_RIPPLE_TREE       6
#define RENDER_RIPPLE_CANDYCANE  7
#define RENDER_RIPPLE_SNOWFLAKE  8
#define RENDER_RIPPLE_CRUCIFIX   9
#define RENDER_RIPPLE_PRESENT    10

#define MOVEMENT_EXPLODE    0
#define MOVEMENT_IMPLODE    1

void RippleEffect::SetDefaultParameters() {
    RipplePanel *rp = (RipplePanel*)panel;
    if (rp == nullptr) {
        return;
    }

    rp->BitmapButton_Ripple_CyclesVC->SetActive(false);
    rp->BitmapButton_Ripple_ThicknessVC->SetActive(false);
    rp->BitmapButton_Ripple_RotationVC->SetActive(false);

    SetChoiceValue(rp->Choice_Ripple_Object_To_Draw, "Circle");
    SetChoiceValue(rp->Choice_Ripple_Movement, "Explode");

    SetSliderValue(rp->Slider_Ripple_Thickness, 3);
    SetSliderValue(rp->Slider_Ripple_Cycles, 10);
    SetSliderValue(rp->Slider_Ripple_Points, 5);
	SetSliderValue(rp->Slider_Ripple_Rotation, 0);

    SetCheckBoxValue(rp->CheckBox_Ripple3D, false);
}

void RippleEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    const std::string &Object_To_DrawStr = SettingsMap["CHOICE_Ripple_Object_To_Draw"];
    const std::string &MovementStr = SettingsMap["CHOICE_Ripple_Movement"];
    int Ripple_Thickness = GetValueCurveInt("Ripple_Thickness", 3, SettingsMap, oset, RIPPLE_THICKNESS_MIN, RIPPLE_THICKNESS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool CheckBox_Ripple3D = SettingsMap.GetBool("CHECKBOX_Ripple3D", false);
    float cycles = GetValueCurveDouble("Ripple_Cycles", 1.0, SettingsMap, oset, RIPPLE_CYCLES_MIN, RIPPLE_CYCLES_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 10);
    int points = SettingsMap.GetInt("SLIDER_RIPPLE_POINTS", 5);
    int rotation = GetValueCurveInt("Ripple_Rotation", 0, SettingsMap, oset, RIPPLE_ROTATION_MIN, RIPPLE_ROTATION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

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
    else if (Object_To_DrawStr == "Star")
    {
        Object_To_Draw = RENDER_RIPPLE_STAR;
    }
	else if (Object_To_DrawStr == "Polygon")
	{
		Object_To_Draw = RENDER_RIPPLE_POLYGON;
	}
	else if (Object_To_DrawStr == "Heart")
	{
		Object_To_Draw = RENDER_RIPPLE_HEART;
	}
	else if (Object_To_DrawStr == "Tree")
	{
		Object_To_Draw = RENDER_RIPPLE_TREE;
	}
	else if (Object_To_DrawStr == "Candy Cane")
	{
		Object_To_Draw = RENDER_RIPPLE_CANDYCANE;
	}
	else if (Object_To_DrawStr == "Snow Flake")
	{
		Object_To_Draw = RENDER_RIPPLE_SNOWFLAKE;
	}
	else if (Object_To_DrawStr == "Crucifix")
	{
		Object_To_Draw = RENDER_RIPPLE_CRUCIFIX;
	}
	else if (Object_To_DrawStr == "Present")
	{
		Object_To_Draw = RENDER_RIPPLE_PRESENT;
	}
	else
	{
		Object_To_Draw = RENDER_RIPPLE_CIRCLE;
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
            Drawstar(buffer, Movement, xc, yc, radius, points, hsv, Ripple_Thickness, CheckBox_Ripple3D, rotation);
            break;
        case RENDER_RIPPLE_TRIANGLE:
            if (Movement == MOVEMENT_IMPLODE)
                side = xc - (xc*rx);
            else
                side = (xc*rx);

            Drawtriangle(buffer, Movement, xc, yc, side, hsv, Ripple_Thickness, CheckBox_Ripple3D);
            break;
		case RENDER_RIPPLE_POLYGON:
			if (Movement == MOVEMENT_IMPLODE)
				radius = xc - (xc*rx);
			else
				radius = (xc*rx);
			Drawpolygon(buffer, Movement, xc, yc, radius, points, hsv, Ripple_Thickness, CheckBox_Ripple3D, rotation);
			break;
		case RENDER_RIPPLE_HEART:
			if (Movement == MOVEMENT_IMPLODE)
				radius = xc - (xc*rx);
			else
				radius = (xc*rx);
			Drawheart(buffer, Movement, xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
			break;
		case RENDER_RIPPLE_TREE:
			if (Movement == MOVEMENT_IMPLODE)
				radius = xc - (xc*rx);
			else
				radius = (xc*rx);
			Drawtree(buffer, Movement, xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
			break;
		case RENDER_RIPPLE_CANDYCANE:
			if (Movement == MOVEMENT_IMPLODE)
				radius = xc - (xc*rx);
			else
				radius = (xc*rx);
			Drawcandycane(buffer, Movement, xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
			break;
		case RENDER_RIPPLE_SNOWFLAKE:
			if (Movement == MOVEMENT_IMPLODE)
				radius = xc - (xc*rx);
			else
				radius = (xc*rx);
			Drawsnowflake(buffer, Movement, xc, yc, radius, points, hsv, Ripple_Thickness, CheckBox_Ripple3D, rotation);
			break;
		case RENDER_RIPPLE_CRUCIFIX:
			if (Movement == MOVEMENT_IMPLODE)
				radius = xc - (xc*rx);
			else
				radius = (xc*rx);
			Drawcrucifix(buffer, Movement, xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
			break;
		case RENDER_RIPPLE_PRESENT:
			if (Movement == MOVEMENT_IMPLODE)
				radius = xc - (xc*rx);
			else
				radius = (xc*rx);
			Drawpresent(buffer, Movement, xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
			break;
        default:
            wxASSERT(false);
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

void RippleEffect::Drawstar(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, int points, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D, float rotation)
{
    double offsetangle = 0.0;
    switch (points)
    {
	case 3:
		offsetangle = 90.0 - 360.0 / 3;
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
	case 8:
		offsetangle = 90.0 - 360.0 / 8;
		break;
    default:
        wxASSERT(false);
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
            double radian = (rotation + offsetangle + degrees) * (M_PI / 180.0);
            int xouter = radius * cos(radian) + xc;
            int youter = radius * sin(radian) + yc;

            radian = (rotation + offsetangle + degrees + increment / 2.0) * (M_PI / 180.0);
            int xinner = InnerRadius * cos(radian) + xc;
            int yinner = InnerRadius * sin(radian) + yc;

            buffer.DrawLine(xinner, yinner, xouter, youter, color);

            radian = (rotation + offsetangle + degrees - increment / 2.0) * (M_PI / 180.0);
            xinner = InnerRadius * cos(radian) + xc;
            yinner = InnerRadius * sin(radian) + yc;

            buffer.DrawLine(xinner, yinner, xouter, youter, color);
        }
    }
}

void RippleEffect::Drawpolygon(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, int points, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D, float rotation)
{
	double increment = 360.0 / points;

	if (points % 2 != 0)
		rotation += 90;
	if (points == 4)
		rotation += 45;
	if (points == 8)
		rotation += 22.5;

	xlColor color(hsv);

	for (double i = 0; i < Ripple_Thickness; i += .5)
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

		if (radius >= 0)
		{
			for (double degrees = 0.0; degrees < 361.0; degrees += increment) // 361 because it allows for small rounding errors
			{
				if (degrees > 360.0) degrees = 360.0;
				double radian = (rotation + degrees) * M_PI / 180.0;
				int x1 = std::round(radius * cos(radian)) + xc;
				int y1 = std::round(radius * sin(radian)) + yc;

				radian = (rotation + degrees + increment) * M_PI / 180.0;
				int x2 = std::round(radius * cos(radian)) + xc;
				int y2 = std::round(radius * sin(radian)) + yc;

				buffer.DrawLine(x1, y1, x2, y2, color);

				if (degrees == 360.0) degrees = 361.0;
			}
		}
		else
		{
			break;
		}
	}
}

void RippleEffect::Drawsnowflake(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, int points, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D, double rotation)
{
	double increment = 360.0 / (points * 2);
	double angle = rotation;

	xlColor color(hsv);

	if (radius >= 0)
	{
		for (int i = 0; i < points * 2; i++)
		{
			double radian = angle * M_PI / 180.0;

			int x1 = std::round(radius * cos(radian)) + xc;
			int y1 = std::round(radius * sin(radian)) + yc;

			radian = (180 + angle) * M_PI / 180.0;

			int x2 = std::round(radius * cos(radian)) + xc;
			int y2 = std::round(radius * sin(radian)) + yc;

			buffer.DrawLine(x1, y1, x2, y2, color);

			angle += increment;
		}
	}
}

void RippleEffect::Drawheart(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
{
    xlColor color(hsv);

	for (float i = 0; i<Ripple_Thickness; i += .5)
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
		if (radius >= 0)
		{
			for (double x = -2.0; x <= 2.0; x += 0.01f)
			{
				double y1 = std::sqrt(1.0 - (std::abs(x) - 1.0) * (std::abs(x) - 1.0));
				double y2 = std::acos(1.0 - std::abs(x)) - M_PI;

				double r = radius;
				if (r >= 0)
				{
					buffer.SetPixel(std::round(x * r / 2.0) + xc, std::round(y1 * r / 2.0) + yc, color);
					buffer.SetPixel(std::round(x * r / 2.0) + xc, std::round(y2 * r / 2.0) + yc, color);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			break;
		}
	}
}

void RippleEffect::Drawtree(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
{
	struct line
	{
		wxPoint start;
		wxPoint end;

		line(const wxPoint s, const wxPoint e)
		{
			start = s;
			end = e;
		}
	};

	const line points[] = { line(wxPoint(3,0), wxPoint(5,0)),
		line(wxPoint(5,0), wxPoint(5,3)),
		line(wxPoint(3,0), wxPoint(3,3)),
		line(wxPoint(0,3), wxPoint(8,3)),
		line(wxPoint(0,3), wxPoint(2,6)),
		line(wxPoint(8,3), wxPoint(6,6)),
		line(wxPoint(1,6), wxPoint(2,6)),
		line(wxPoint(6,6), wxPoint(7,6)),
		line(wxPoint(1,6), wxPoint(3,9)),
		line(wxPoint(7,6), wxPoint(5,9)),
		line(wxPoint(2,9), wxPoint(3,9)),
		line(wxPoint(5,9), wxPoint(6,9)),
		line(wxPoint(6,9), wxPoint(4,11)),
		line(wxPoint(2,9), wxPoint(4,11))
	};
	int count = sizeof(points) / sizeof(line);

    xlColor color(hsv);

	for (float i = 0; i<Ripple_Thickness; i += .5)
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
		if(radius >= 0)
		{
			for (int j = 0; j < count; ++j)
			{
				int x1 = std::round(((double)points[j].start.x - 4.0) / 11.0 * radius);
				int y1 = std::round(((double)points[j].start.y - 4.0) / 11.0 * radius);
				int x2 = std::round(((double)points[j].end.x - 4.0) / 11.0 * radius);
				int y2 = std::round(((double)points[j].end.y - 4.0) / 11.0 * radius);
				buffer.DrawLine(xc + x1, yc + y1, xc + x2, yc + y2, color);
			}
		}
		else
		{
			break;
		}
	}
}

void RippleEffect::Drawcrucifix(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
{
	struct line
	{
		wxPoint start;
		wxPoint end;

		line(const wxPoint s, const wxPoint e)
		{
			start = s;
			end = e;
		}
	};

	const line points[] = { line(wxPoint(2,0), wxPoint(2,6)),
		line(wxPoint(2,6), wxPoint(0,6)),
		line(wxPoint(0,6), wxPoint(0,7)),
		line(wxPoint(0,7), wxPoint(2,7)),
		line(wxPoint(2,7), wxPoint(2,10)),
		line(wxPoint(2,10), wxPoint(3,10)),
		line(wxPoint(3,10), wxPoint(3,7)),
		line(wxPoint(3,7), wxPoint(5,7)),
		line(wxPoint(5,7), wxPoint(5,6)),
		line(wxPoint(5,6), wxPoint(3,6)),
		line(wxPoint(3,6), wxPoint(3,0)),
		line(wxPoint(3,0), wxPoint(2,0))
	};
	int count = sizeof(points) / sizeof(line);

    xlColor color(hsv);

	for (float i = 0; i<Ripple_Thickness; i += .5)
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
		if (radius >= 0)
		{
			for (int j = 0; j < count; ++j)
			{
				int x1 = std::round(((double)points[j].start.x - 2.5) / 7.0 * radius);
				int y1 = std::round(((double)points[j].start.y - 6.5) / 10.0 * radius);
				int x2 = std::round(((double)points[j].end.x - 2.5) / 7.0 * radius);
				int y2 = std::round(((double)points[j].end.y - 6.5) / 10.0 * radius);
				buffer.DrawLine(xc + x1, yc + y1, xc + x2, yc + y2, color);
			}
		}
		else
		{
			break;
		}
	}
}

void RippleEffect::Drawpresent(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
{
	struct line
	{
		wxPoint start;
		wxPoint end;

		line(const wxPoint s, const wxPoint e)
		{
			start = s;
			end = e;
		}
	};

	const line points[] = { line(wxPoint(0,0), wxPoint(0,9)),
		line(wxPoint(0,9), wxPoint(10,9)),
		line(wxPoint(10,9), wxPoint(10,0)),
		line(wxPoint(10,0), wxPoint(0,0)),
		line(wxPoint(5,0), wxPoint(5,9)),
		line(wxPoint(5,9), wxPoint(2,11)),
		line(wxPoint(2,11), wxPoint(2,9)),
		line(wxPoint(5,9), wxPoint(8,11)),
		line(wxPoint(8,11), wxPoint(8,9))
	};
	int count = sizeof(points) / sizeof(line);

    xlColor color(hsv);

	for (float i = 0; i<Ripple_Thickness; i += .5)
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
		if (radius >= 0)
		{
			for (int j = 0; j < count; ++j)
			{
				int x1 = std::round(((double)points[j].start.x - 5) / 7.0 * radius);
				int y1 = std::round(((double)points[j].start.y - 5.5) / 10.0 * radius);
				int x2 = std::round(((double)points[j].end.x - 5) / 7.0 * radius);
				int y2 = std::round(((double)points[j].end.y - 5.5) / 10.0 * radius);
				buffer.DrawLine(xc + x1, yc + y1, xc + x2, yc + y2, color);
			}
		}
		else
		{
			break;
		}

	}
}

void RippleEffect::Drawcandycane(RenderBuffer &buffer, int Movement, int xc, int yc, double radius, HSVValue &hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
{
	double originalRadius = radius;
    xlColor color(hsv);

	for (float i = 0; i<Ripple_Thickness; i += .5)
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
		if (radius >= 0)
		{
			// draw the stick
			int y1 = std::round((double)yc + originalRadius / 6.0);
			int y2 = std::round((double)yc - originalRadius / 2.0);
			int x = std::round((double)xc + radius / 2.0);
			buffer.DrawLine(x, y1, x, y2, color);

			// draw the hook
			double r = radius / 3.0;
			for (double degrees = 0.0; degrees < 180; degrees += 1.0)
			{
				double radian = degrees * (M_PI / 180.0);
				x = std::round((r ) * buffer.cos(radian) + xc + originalRadius / 6.0);
				int y = std::round((r) * buffer.sin(radian) + y1);
				buffer.SetPixel(x, y, color);
			}
		}
		else
		{
			break;
		}
	}
}