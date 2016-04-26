#include "SingleStrandEffect.h"
#include "SingleStrandPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"


#include "../../include/singleStrand.xpm"

SingleStrandEffect::SingleStrandEffect(int id)
    : RenderableEffect(id, "SingleStrand", singleStrand, singleStrand, singleStrand, singleStrand, singleStrand)
{
    //ctor
    tooltip = "Single Strand";
}

SingleStrandEffect::~SingleStrandEffect()
{
    //dtor
}

wxPanel *SingleStrandEffect::CreatePanel(wxWindow *parent) {
    return new SingleStrandPanel(parent);
}


int mapX(int x, int max, int direction, int &second) {
    second = -1;
    switch (direction) {
        case 0: //
            return x;
        case 1:
            return max - x - 1;
        case 2:
            second = max + x;
            return max - x - 1;
        case 3:
            second = max * 2 - x - 1;
            return x;
    }
    return -1;
}

int mapDirection(const wxString & d) {
    if ("Left" == d) {
        return 1;
    }
    if ("Right" == d) {
        return 0;
    }
    if ("From Middle" == d) {
        return 2;
    }
    if ("To Middle" == d) {
        return 3;
    }
    
    return 0;
}


void SingleStrandEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    if ("Skips" == SettingsMap["NOTEBOOK_SSEFFECT_TYPE"]) {
        RenderSingleStrandSkips(buffer, effect,
                                SettingsMap.GetInt("SLIDER_Skips_BandSize"),
                                SettingsMap.GetInt("SLIDER_Skips_SkipSize"),
                                SettingsMap.GetInt("SLIDER_Skips_StartPos"),
                                SettingsMap["CHOICE_Skips_Direction"],
                                SettingsMap.GetInt("SLIDER_Skips_Advance", 0));
    } else {
        RenderSingleStrandChase(buffer,
                                SettingsMap.Get("CHOICE_SingleStrand_Colors", "Palette"),
                                SettingsMap.GetInt("SLIDER_Number_Chases"),
                                SettingsMap.GetInt("SLIDER_Color_Mix1"),
                                SettingsMap.Get("CHOICE_Chase_Type1", "Left-Right"),
                                SettingsMap.GetBool("CHECKBOX_Chase_3dFade1"),
                                SettingsMap.GetBool("CHECKBOX_Chase_Group_All"),
                                SettingsMap.GetFloat("TEXTCTRL_Chase_Rotations", 1.0));
    }
}


void SingleStrandEffect::RenderSingleStrandSkips(RenderBuffer &buffer, Effect *eff, int Skips_BandSize, int Skips_SkipSize, int Skips_StartPos,
                                         const std::string & Skips_Direction, int advances)
{
    int x = Skips_StartPos - 1;
    xlColor color;
    int second = 0;
    int max = buffer.BufferWi;
    int direction = mapDirection(Skips_Direction);
    if (direction > 1) {
        max /= 2;
    }
    
    size_t colorcnt = buffer.GetColorCount();
    double position = buffer.GetEffectTimeIntervalPosition() * (advances + 1.0) * 0.99;
    
    x += int(position) * Skips_BandSize;
    while (x > max) {
        x -= (Skips_BandSize +  Skips_SkipSize) * colorcnt;
    }
    
    if (buffer.needToInit) {
        buffer.needToInit = false;
        std::lock_guard<std::recursive_mutex> lock(eff->GetBackgroundDisplayList().lock);
        int rects = (Skips_SkipSize + Skips_BandSize) * (buffer.curEffEndPer - buffer.curEffStartPer + 1);
        eff->GetBackgroundDisplayList().resize(rects * 6);
    }
    
    int firstX = x;
    int colorIdx = 0;
    
    while (x < max) {
        buffer.palette.GetColor(colorIdx, color);
        colorIdx++;
        if (colorIdx >= colorcnt) {
            colorIdx = 0;
        }
        for (int cnt = 0; cnt < Skips_BandSize && x < max; cnt++) {
            int mappedX = mapX(x, max, direction, second);
            if (mappedX >= 0 && mappedX < buffer.BufferWi) {
                for (int y = 0; y < buffer.BufferHt; y++) {
                    buffer.SetPixel(mappedX, y, color);
                }
            }
            if (second >= 0 && second < buffer.BufferWi) {
                for (int y = 0; y < buffer.BufferHt; y++) {
                    buffer.SetPixel(second, y, color);
                }
            }
            x++;
        }
        x += Skips_SkipSize;
    }
    colorIdx = buffer.GetColorCount() - 1;
    x = firstX - 1;
    while (x >= 0) {
        x -= Skips_SkipSize;
        
        buffer.palette.GetColor(colorIdx, color);
        colorIdx--;
        if (colorIdx < 0) {
            colorIdx = buffer.GetColorCount() - 1;
        }
        for (int cnt = 0; cnt < Skips_BandSize && x >= 0; cnt++) {
            int mappedX = mapX(x, max, direction, second);
            if (mappedX >= 0 && mappedX < buffer.BufferWi) {
                for (int y = 0; y < buffer.BufferHt; y++) {
                    buffer.SetPixel(mappedX, y, color);
                }
            }
            if (second >= 0 && second < buffer.BufferWi) {
                for (int y = 0; y < buffer.BufferHt; y++) {
                    buffer.SetPixel(second, y, color);
                }
            }
            x--;
        }
    }
    
    max = Skips_SkipSize + Skips_BandSize - 1;
    if (max >=  buffer.BufferWi) {
        max = buffer.BufferWi - 1;
    }
    buffer.CopyPixelsToDisplayListX(eff, 0, 0, max);
}

int mapChaseType(const std::string &Chase_Type) {
    if ("Left-Right" == Chase_Type) {
        return 0;
    }
    if ("Right-Left" == Chase_Type) {
        return 1;
    }
    if ("Bounce from Left" == Chase_Type) {
        return 2;
    }
    if ("Bounce from Right" == Chase_Type) {
        return 3;
    }
    if ("Dual Bounce" == Chase_Type) {
        return 4;
    }
    if ("From Middle" == Chase_Type) {
        return 5;
    }
    if ("To Middle" == Chase_Type) {
        return 6;
    }
    return 0;
}
void SingleStrandEffect::RenderSingleStrandChase(RenderBuffer &buffer,
                                         const std::string & ColorSchemeName,int Number_Chases, int chaseSize,
                                         const std::string &Chase_Type1,
                                         bool Chase_Fade3d1,bool Chase_Group_All,
                                         float chaseSpeed)
{
    
    int x,y,i,width;
    int MaxNodes;
    int Dual_Chases = 0;
    float dx;
    
    int ColorScheme = "Palette" == ColorSchemeName;
    
    bool R_TO_L1 = 0;
    
    int chaseType = mapChaseType(Chase_Type1);
    
    size_t colorcnt=buffer.GetColorCount(); // global now set to how many colors have been picked
    y=buffer.BufferHt;
    i=0;
    
    int curEffStartPer, curEffEndPer;
    
    buffer.GetEffectPeriods( curEffStartPer, curEffEndPer);
    
    if (Chase_Group_All) MaxNodes= buffer.BufferWi*buffer.BufferHt;
    else MaxNodes=buffer.BufferWi;
    
    int MaxChase=MaxNodes*(chaseSize/100.0);
    if(MaxChase<1) MaxChase=1;
    
    int nodes_per_color = int(MaxChase/colorcnt);
    if(nodes_per_color<1)  nodes_per_color=1;    //  fill chase buffer
    
    int Mirror = 0;
    int AutoReverse=0;
    int &ChaseDirection = buffer.tempInt;
    
    if (buffer.needToInit) {
        buffer.needToInit = false;
        ChaseDirection = chaseType == 0 || chaseType == 2 || chaseType == 6; // initialize it once at the beggining of this sequence.
    }
    switch (chaseType)
    {
        case 6:
            Mirror = 1;
        case 0: // "Normal. L-R"
            R_TO_L1=1;
            break;
        case 5:
            Mirror = 1;
        case 1: // "Normal. R-L"
            R_TO_L1=0;
            break;
        case 2: // "Auto reverse l-r"
            R_TO_L1=1;
            AutoReverse=1;
            break;
        case 3: // "Auto reverse r-l"
            AutoReverse=1;
            break;
        case 4: // "Bounce"
            Dual_Chases=1;
            break;
    }
    
    //double rtval = GetEffectTimeIntervalPosition(chaseSpeed) * (AutoReverse ? 1.999 : 0.999);
    if (Chase_Group_All) {
        width=MaxNodes;
    } else {
        width=buffer.BufferWi;
    }
    if (Mirror) {
        width /= 2;
    }
    if (width == 0) {
        width = 1;
    }
    double chaseOffset = width * chaseSize / 100.0 - 1;
    
    double rtval;
    if (AutoReverse) {
        rtval = (double)(buffer.curPeriod-buffer.curEffStartPer)/(double)(buffer.curEffEndPer-buffer.curEffStartPer);
        rtval *= chaseSpeed;
        while (rtval > 1.0) {
            rtval -= 1.0;
        }
        rtval *= 2.0;
    } else {
        rtval = (double)(buffer.curPeriod-buffer.curEffStartPer)/(double)(buffer.curEffEndPer-buffer.curEffStartPer + (Number_Chases == 1 ? 1 : 0));
        rtval *= chaseSpeed;
        while (rtval > 1.0) {
            rtval -= 1.0;
        }
    }
    
    
    if (Number_Chases < 1) Number_Chases=1;
    if (ColorScheme < 0) ColorScheme=0;
    dx = double(width)/double(Number_Chases);
    if(dx<1) dx=1.0;
    
    double startState = (width + width * chaseSize / 100.0 - 1) * rtval;
    if (Number_Chases > 1) {
        startState = width * rtval;
    }
    if (chaseOffset < 0) {
        chaseOffset = 0;
        startState = (width - 1) * rtval;
    }
    for(int chase=0; chase<Number_Chases; chase++)
    {
        if (AutoReverse) {
            x = chase*dx + width*rtval - width * chaseSize / 200.0;
        } else {
            double x1 = chase*dx + startState - chaseOffset; // L-R
            int maxChaseWid = (width * chaseSize/100.0);
            if (round(chaseOffset) == maxChaseWid) {
                x1 = chase*dx + startState - trunc(chaseOffset);
            }
            x = std::round(x1);
        }
        
        draw_chase(buffer, x, Chase_Group_All, ColorScheme,Number_Chases,AutoReverse, width,chaseSize,Chase_Fade3d1,ChaseDirection, Mirror); // Turn pixel on
        if(Dual_Chases) {
            draw_chase(buffer, x, Chase_Group_All,ColorScheme,Number_Chases,AutoReverse,width,chaseSize,Chase_Fade3d1,!ChaseDirection, Mirror); //
        }
    }
}

void SingleStrandEffect::draw_chase(RenderBuffer &buffer,
                                    int x, bool GroupAll,
                                    int ColorScheme,
                                    int Number_Chases,
                                    bool AutoReverse,
                                    int width,
                                    int Chase_Width,
                                    bool Chase_Fade3d1,
                                    int ChaseDirection,
                                    bool mirror)
{
    float  orig_v;
    int new_x,i,max_chase_width,pixels_per_chase;
    size_t colorcnt=buffer.GetColorCount();
    int ColorIdx;
    
    max_chase_width = width * Chase_Width/100.0;
    if (max_chase_width < 1) {
        max_chase_width = 1;
    }
    pixels_per_chase = width/Number_Chases;
    if (pixels_per_chase < 1) {
        pixels_per_chase = 1;
    }
    
    HSVValue hsv;
    buffer.palette.GetHSV(0, hsv);
    orig_v = hsv.value;
    xlColor color;
    
    
    /*
     RRRRGGGG........+........................
     .RRRRGGGG.......+........................
     ..RRRRGGGG......+........................
     ...RRRRGGGG.....+........................
     ....RRRRGGGG....+........................
     .....RRRRGGGG...+........................
     ......RRRRGGGG..+........................
     .......RRRRGGGG.+........................
     ........RRRRGGGG+..............<===========   this is what fist version would end at
     .........RRRRGGG+........................
     .........RRRRGG+........................
     .........RRRRG+........................
     .........RRRR+........................
     .........RRR+........................
     */
    
    int firstX = x;
    int direction = 1;
    
    if (AutoReverse) {
        if (firstX < 0 && firstX > -max_chase_width) {
            firstX = -firstX - 1;
            direction = -1;
        }
        if (firstX < 0 || firstX >= width) {
            int dif = - firstX;
            if (firstX < 0) {
                firstX = -firstX - 1;
                direction = -1;
                dif = 0;
            } else {
                dif = firstX - width + 1;
                firstX = width;
                direction = -1;
            }
            
            while (dif) {
                dif--;
                firstX += direction;
                if (firstX == (width - 1)) {
                    direction = -1;
                }
                if (firstX == 0) {
                    direction = 1;
                }
            }
        }
    }
    
    if(max_chase_width>=1)
    {
        for (i=0; i<max_chase_width; i++)
        {
            if(ColorScheme==0) {
                if (max_chase_width) hsv.hue = 1.0 - (i*1.0/max_chase_width); // rainbow hue
                color = hsv;
            }
            
            if (AutoReverse) {
                new_x = firstX + direction;
                
                while (new_x < 0) {
                    direction = 1;
                    new_x = 0;
                }
                while (new_x >= width) {
                    direction = -1;
                    new_x = width - 1;
                }
                firstX = new_x;
            } else if (Number_Chases > 1) {
                new_x = x+i;
                
                while (new_x < 0) {
                    new_x += width;
                }
                while (new_x >= width) {
                    new_x -= width;
                }
            } else {
                new_x = x+i;
            }
            
            //new_x=new_x%BufferWi;
            if(i < pixels_per_chase) // as long as the chase fits, keep drawing it
            {
                if (ColorScheme != 0) {
                    if (colorcnt==1) {
                        ColorIdx=0;
                    } else {
                        ColorIdx=((double)((max_chase_width - i + 1)*colorcnt))/max_chase_width;
                    }
                    if (ColorIdx >= colorcnt) ColorIdx=colorcnt-1;
                    if (Chase_Fade3d1) {
                        if (buffer.allowAlpha) {
                            buffer.palette.GetColor(ColorIdx, color);
                            color.alpha = 255.0 * (i + 1.0)/max_chase_width;
                        } else {
                            buffer.palette.GetHSV(ColorIdx, hsv);
                            hsv.value=orig_v - ((max_chase_width - (i + 1.0))/max_chase_width); // fades data down over chase width
                            if (hsv.value<0.0) hsv.value=0.0;
                            color = hsv;
                        }
                    } else {
                        buffer.palette.GetColor(ColorIdx, color);
                    }
                    
                }
                if (ChaseDirection == 0) {// are we going R-L?
                    new_x = width - new_x - 1;
                }
                if (new_x >= 0 && new_x <= width) {
                    if (GroupAll) {
                        int y = 0;
                        int mirrorx = buffer.BufferWi*buffer.BufferHt - new_x - 1;
                        int mirrory = 0;
                        while (new_x >= buffer.BufferWi) {
                            y++;
                            new_x -= buffer.BufferWi;
                        }
                        while (mirrorx >= buffer.BufferWi) {
                            mirrory++;
                            mirrorx -= buffer.BufferWi;
                        }
                        if (Chase_Fade3d1) {
                            xlColor c;
                            buffer.GetPixel(new_x, y, c);
                            if (c != xlBLACK) {
                                int a = color.alpha;
                                color = color.AlphaBlend(c);
                                color.alpha = c.alpha > a ? c.alpha : a;
                            }
                        }
                        buffer.SetPixel(new_x,y,color); // Turn pixel on
                        if (mirror) {
                            buffer.SetPixel(mirrorx,mirrory,color); // Turn pixel on
                        }
                    } else {
                        if (Chase_Fade3d1) {
                            xlColor c;
                            buffer.GetPixel(new_x, 0, c);
                            if (c != xlBLACK) {
                                int a = color.alpha;
                                color = color.AlphaBlend(c);
                                color.alpha = c.alpha > a ? c.alpha : a;
                            }
                        }
                        for (int y=0; y<buffer.BufferHt; y++) {
                            buffer.SetPixel(new_x,y,color); // Turn pixel on
                            if (mirror) {
                                buffer.SetPixel(buffer.BufferWi - new_x - 1,y,color); // Turn pixel on
                            }
                        }
                    }
                }
                
            }
        }
    }
}

