/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ButterflyEffect.h"

#include "ButterflyPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/butterfly-16.xpm"
#include "../../include/butterfly-24.xpm"
#include "../../include/butterfly_32.xpm"
#include "../../include/butterfly-48.xpm"
#include "../../include/butterfly-64.xpm"

#include "../Parallel.h"

#ifndef __WXMSW__
#include "ispc/ButterflyFunctions.ispc.h"
#define HASISPC
#endif

ButterflyEffect::ButterflyEffect(int i) : RenderableEffect(i, "Butterfly", butterfly_16, butterfly_24, butterfly_32, butterfly_48, butterfly_64)
{
    //ctor
}

ButterflyEffect::~ButterflyEffect()
{
    //dtor
}


xlEffectPanel *ButterflyEffect::CreatePanel(wxWindow *parent) {
    return new ButterflyPanel(parent);
}

/*
 01) x*y^3-y*x^3
 (02) (x^2+3*y^2)*e^(-x^2-y^2)
	(03) -x*y*e^(-x^2-y^2)
	(04) -1/(x^2+y^2)
	(05) cos(abs(x)+abs(y))
	(06) cos(abs(x)+abs(y))*(abs(x)+abs(y))
 */

static inline int GetButterflyColorScheme(const std::string &color) {
    if (color == "Rainbow") {
        return 0;
    } else if (color == "Palette") {
        return 1;
    }
    return 0;
}

void ButterflyEffect::SetDefaultParameters() {
    ButterflyPanel *bp = (ButterflyPanel*)panel;
    if (bp == nullptr) {
        return;
    }

    SetChoiceValue(bp->Choice_Butterfly_Colors, "Rainbow");
    SetChoiceValue(bp->Choice_Butterfly_Direction, "Normal");

    bp->BitmapButton_Butterfly_Chunks->SetActive(false);
    bp->BitmapButton_Butterfly_Skip->SetActive(false);
    bp->BitmapButton_Butterfly_Speed->SetActive(false);

    SetSliderValue(bp->Slider_Butterfly_Style, 1);
    SetSliderValue(bp->Slider_Butterfly_Chunks, 1);
    SetSliderValue(bp->Slider_Butterfly_Skip, 2);
    SetSliderValue(bp->Slider_Butterfly_Speed, 10);
}


void ButterflyEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer)
{
    float oset = buffer.GetEffectTimeIntervalPosition();
    const int Chunks = GetValueCurveInt("Butterfly_Chunks", 1, SettingsMap, oset, BUTTERFLY_CHUNKS_MIN, BUTTERFLY_CHUNKS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int Skip = GetValueCurveInt("Butterfly_Skip", 2, SettingsMap, oset, BUTTERFLY_SKIP_MIN, BUTTERFLY_SKIP_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int butterFlySpeed = GetValueCurveInt("Butterfly_Speed", 10, SettingsMap, oset, BUTTERFLY_SPEED_MIN, BUTTERFLY_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    const int Style = SettingsMap.GetInt("SLIDER_Butterfly_Style", 1);
    int ColorScheme = GetButterflyColorScheme(SettingsMap["CHOICE_Butterfly_Colors"]);
    int ButterflyDirection = SettingsMap["CHOICE_Butterfly_Direction"] == "Reverse" ? 1 : 0;

    const int curState = (buffer.curPeriod - buffer.curEffStartPer) * butterFlySpeed * buffer.frameTimeInMs / 50;
    const size_t colorcnt=buffer.GetColorCount();
    const double offset = (ButterflyDirection==1 ? -1 : 1) * double(curState)/200.0;

#ifdef HASISPC
    ispc::ButterflyData data;
    data.offset = offset;
    data.chunks = Chunks;
    data.skip = Skip;
    data.curState = curState;
    data.colorScheme = ColorScheme;
    data.width = buffer.BufferWi;
    data.height = buffer.BufferHt;
    data.numColors = colorcnt;
    for (int x = 0; x < colorcnt; x++) {
        const xlColor &c = buffer.palette.GetColor(x);
        data.colors[x].v[0] = c.red;
        data.colors[x].v[1] = c.green;
        data.colors[x].v[2] = c.blue;
        data.colors[x].v[3] = c.alpha;
    }
    if (Style > 5) {
        // slightly different setup for "plasmas"
        int state = (buffer.curPeriod - buffer.curEffStartPer); // frames 0 to N
        double Speed_plasma = (Style == 10) ? (101-butterFlySpeed)*3 : (101-butterFlySpeed)*5;
        double time = (state+1.0)/Speed_plasma;
        data.plasmaTime = time;
        data.plasmaStyle = Style;
    }
    
    int max = buffer.BufferHt * buffer.BufferWi;
    constexpr int bfBlockSize = 4096;
    int blocks = max / bfBlockSize + 1;
    parallel_for(0, blocks, [&data, &buffer, max, Style](int y) {
        int start = y * bfBlockSize;
        int end = start + bfBlockSize;
        if (end > max) {
            end = max;
        }
        switch (Style) {
            case 1:
                ButterflyEffectStyle1(data, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            case 2:
                ButterflyEffectStyle2(data, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            case 3:
                ButterflyEffectStyle3(data, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            case 4:
                ButterflyEffectStyle4(data, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            case 5:
                ButterflyEffectStyle5(data, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
            default:
                ButterflyEffectPlasmaStyles(data, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                break;
        }
    });
#else
    static const double pi2=6.283185307;
    //  These are for Plasma effect
    static const double pi=3.1415926535897932384626433832;
    const int maxframe=buffer.BufferHt*2;
    const int frame=(buffer.BufferHt * curState / 200)%maxframe;

    const int xc=buffer.BufferWi/2;
    const int yc=buffer.BufferHt/2;
    int block = buffer.BufferHt * buffer.BufferWi > 100 ? 1 : -1;
    parallel_for(0, buffer.BufferWi, [&buffer, Style, &xc, &yc, &offset, frame, maxframe, Chunks, colorcnt, Skip, ColorScheme, butterFlySpeed](int x) {
        double  fractpart, intpart;
        double h=0.0,hue1,hue2;
        xlColor color;
        HSVValue hsv;
        int y, d, x0, y0;
        double n,x1,y1,f;
        double rx,ry,cx,cy,v,time,multiplier;

        for (y=0; y<buffer.BufferHt; y++)
        {
            switch (Style)
            {
                case 1:
                    //  http://mathworld.wolfram.com/ButterflyFunction.html
                    n = std::abs((x*x - y*y) * buffer.sin (offset + ((x+y)*pi2 / float(buffer.BufferHt+buffer.BufferWi))));
                    d = x*x + y*y;
                    
                    //  This section is to fix the colors on pixels at {0,1} and {1,0}
                    x0=x+1;
                    y0=y+1;
                    if((x==0 && y==1))
                    {
                        n = std::abs((x*x - y0*y0) * buffer.sin (offset + ((x+y0)*pi2 / float(buffer.BufferHt+buffer.BufferWi))));
                        d = x*x + y0*y0;
                    }
                    if((x==1 && y==0))
                    {
                        n = std::abs((x0*x0 - y*y) * buffer.sin (offset + ((x0+y)*pi2 / float(buffer.BufferHt+buffer.BufferWi))));
                        d = x0*x0 + y*y;
                    }
                    // end of fix
                    
                    h=d>0.001 ? n/d : 0.0;
                    break;
                    
                case 2:
                    f=(frame < maxframe/2) ? frame+1 : maxframe - frame;
                    x1=(double(x)-buffer.BufferWi/2.0)/f;
                    y1=(double(y)-buffer.BufferHt/2.0)/f;
                    h=sqrt(x1*x1+y1*y1);
                    break;
                    
                case 3:
                    f=(frame < maxframe/2) ? frame+1 : maxframe - frame;
                    f=f*0.1+double(buffer.BufferHt)/60.0;
                    x1 = (x-buffer.BufferWi/2.0)/f;
                    y1 = (y-buffer.BufferHt/2.0)/f;
                    h=buffer.sin(x1) * buffer.cos(y1);
                    break;
                    
                case 4:
                    //  http://mathworld.wolfram.com/ButterflyFunction.html
                    n = ((x*x - y*y) * buffer.sin (offset + ((x+y)*pi2 / float(buffer.BufferHt+buffer.BufferWi))));
                    d = x*x + y*y;
                    
                    //  This section is to fix the colors on pixels at {0,1} and {1,0}
                    x0=x+1;
                    y0=y+1;
                    if((x==0 && y==1))
                    {
                        n = ((x*x - y0*y0) * buffer.sin (offset + ((x+y0)*pi2 / float(buffer.BufferHt+buffer.BufferWi))));
                        d = x*x + y0*y0;
                    }
                    if((x==1 && y==0))
                    {
                        n = ((x0*x0 - y*y) * buffer.sin (offset + ((x0+y)*pi2 / float(buffer.BufferHt+buffer.BufferWi))));
                        d = x0*x0 + y*y;
                    }
                    // end of fix
                    
                    h=d>0.001 ? n/d : 0.0;
                    fractpart = modf (h , &intpart);
                    h=fractpart;
                    if(h<0) h=1.0+h;
                    break;
                    
                case 5:
                    //  http://mathworld.wolfram.com/ButterflyFunction.html
                    n = std::abs((x*x - y*y) * buffer.sin (offset + ((x+y)*pi2 / float(buffer.BufferHt*buffer.BufferWi))));
                    d = x*x + y*y;
                    
                    //  This section is to fix the colors on pixels at {0,1} and {1,0}
                    x0=x+1;
                    y0=y+1;
                    if((x==0 && y==1))
                    {
                        n = std::abs((x*x - y0*y0) * buffer.sin (offset + ((x+y0)*pi2 / float(buffer.BufferHt*buffer.BufferWi))));
                        d = x*x + y0*y0;
                    }
                    if((x==1 && y==0))
                    {
                        n = std::abs((x0*x0 - y*y) * buffer.sin (offset + ((x0+y)*pi2 / float(buffer.BufferHt*buffer.BufferWi))));
                        d = x0*x0 + y*y;
                    }
                    // end of fix
                    
                    h=d>0.001 ? n/d : 0.0;
                    break;
                    
            }
            if(Style<=5)
            {
                
                
                hsv.saturation=1.0;
                hsv.value=1.0;
                if (Chunks <= 1 || int(h*Chunks) % Skip != 0)
                {
                    if (ColorScheme == 0)
                    {
                        hsv.hue=h;
                        buffer.SetPixel(x,y,hsv);
                    }
                    else
                    {
                        buffer.GetMultiColorBlend(h,false,color);
                        buffer.SetPixel(x,y,color);
                    }
                }
            }
            else  // Plasma
            {
                // reference: http://www.bidouille.org/prog/plasma
                
                int state = (buffer.curPeriod - buffer.curEffStartPer); // frames 0 to N
                double Speed_plasma = (Style == 10) ? (101-butterFlySpeed)*3 : (101-butterFlySpeed)*5;
                time = (state+1.0)/Speed_plasma;
                
                v=0;
                
                rx = ((float)x/buffer.BufferWi) -0.5;
                ry = ((float)y/buffer.BufferHt) -0.5;
                
                
                
                // 1st equation
                v=buffer.sin(rx*10+time);
                
                //  second equation
                v+=buffer.sin(10*(rx*buffer.sin(time/2)+ry*buffer.cos(time/3))+time);
                
                //  third equation
                cx=rx+.5*buffer.sin(time/5);
                cy=ry+.5*buffer.cos(time/3);
                v+=buffer.sin ( sqrt(100*((cx*cx)+(cy*cy))+1+time));
                
                
                //    vec2 c = v_coords * u_k - u_k/2.0;
                v += buffer.sin(rx+time);
                v += buffer.sin((ry+time)/2.0);
                v += buffer.sin((rx+ry+time)/2.0);
                //   c += u_k/2.0 * vec2(sin(u_time/3.0), cos(u_time/2.0));
                v += buffer.sin(sqrt(rx*rx+ry*ry+1.0)+time);
                v = v/2.0;
                // vec3 col = vec3(1, sin(PI*v), cos(PI*v));
                //   gl_FragColor = vec4(col*.5 + .5, 1);
                
                buffer.GetMultiColorBlend(h,false,color);
                //color.red=color.green=color.blue=h*255;
                switch (Style)
                {
                    case 6:
                        color.red = (buffer.sin(v*Chunks*pi)+1)*128;
                        color.green= (buffer.cos(v*Chunks*pi)+1)*128;
                        color.blue =0;
                        break;
                    case 7:
                        color.red = 1;
                        color.green= (buffer.cos(v*Chunks*pi)+1)*128;
                        color.blue =(buffer.sin(v*Chunks*pi)+1)*128;
                        break;
                        
                    case 8:
                        color.red = (buffer.sin(v*Chunks*pi)+1)*128;
                        color.green= (buffer.sin(v*Chunks*pi + 2*pi/3)+1)*128;
                        color.blue =(buffer.sin(v*Chunks*pi+4*pi/3)+1)*128;
                        break;
                        
                    case 9:
                        color.red=color.green=color.blue=(buffer.sin(v*Chunks*pi) +1) * 128;
                        break;
                    case 10:
                        if(colorcnt>=2)
                        {
                            buffer.GetMultiColorBlend(h,false,color);
                            
                            hue1=.1;
                            hue1=0;
                            hue2=.7;
                            hue2=1;
                            multiplier=(hue2-hue1)/2;
                            h=hue1+ multiplier*(v+1); // v is between -1 to 1. h
                            h = buffer.sin(v*Chunks*pi+2*pi/3)+1*0.5;
                            
                            hsv.hue=h;
                            //  hsv.hue=hsv.hue + (v+1)/20.0;
                            //color.red = (buffer.sin(v*color.red)+1)*128;
                            //color.green = (buffer.sin(v*color.green)+1)*128;
                            //color.blue = (buffer.sin(v*color.blue)+1)*128;
                            
                        }
                        break;
                }
                
                buffer.SetPixel(x,y,color);
            }
        }
    }, block);
#endif
}

