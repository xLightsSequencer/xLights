/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SingleStrandEffect.h"
#include "SingleStrandPanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#define XLIGHTS_FX 
#include "FX.h"

#include "../../include/singleStrand-16.xpm"
#include "../../include/singleStrand-64.xpm"

SingleStrandEffect::SingleStrandEffect(int id)
    : RenderableEffect(id, "SingleStrand", singleStrand_16, singleStrand_64, singleStrand_64, singleStrand_64, singleStrand_64)
{
    //ctor
    tooltip = "Single Strand";
}

SingleStrandEffect::~SingleStrandEffect()
{
    //dtor
}

xlEffectPanel *SingleStrandEffect::CreatePanel(wxWindow *parent) {
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
        default:
            break;
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

void SingleStrandEffect::SetDefaultParameters()
{
    SingleStrandPanel *sp = (SingleStrandPanel*)panel;
    if (sp == nullptr) {
        return;
    }

    sp->BitmapButton_Color_Mix1VC->SetActive(false);
    sp->BitmapButton_Number_ChasesVC->SetActive(false);
    sp->BitmapButton_Chase_Rotations->SetActive(false);
    sp->BitmapButton_Chase_OffsetVC->SetActive(false);
    sp->BitmapButton_FX_IntensityVC->SetActive(false);
    sp->BitmapButton_FX_SpeedVC->SetActive(false);

    SetChoiceValue(sp->Choice_SingleStrand_Colors, "Palette");
    SetChoiceValue(sp->Choice_Skips_Direction, "Left");
    SetChoiceValue(sp->Choice_Chase_Type1, "Left-Right");
    SetChoiceValue(sp->Choice_SingleStrand_FX, "Fireworks 1D");
    SetChoiceValue(sp->Choice_FX_Palette, "* Colors Only");

    SetSliderValue(sp->Slider_Number_Chases, 1);
    SetSliderValue(sp->Slider_Color_Mix1, 10);
    SetSliderValue(sp->Slider_Chase_Rotations, 10);
    SetSliderValue(sp->Slider_Chase_Offset, 0);
    SetSliderValue(sp->Slider_Skips_BandSize, 1);
    SetSliderValue(sp->Slider_Skips_SkipSize, 1);
    SetSliderValue(sp->Slider_Skips_StartPos, 1);
    SetSliderValue(sp->Slider_Skips_Advance, 0);
    SetSliderValue(sp->Slider_FX_Intensity, 128);
    SetSliderValue(sp->Slider_FX_Speed, 128);

    SetCheckBoxValue(sp->CheckBox_Chase_3dFade1, false);
    SetCheckBoxValue(sp->CheckBox_Chase_Group_All, false);
}

bool SingleStrandEffect::needToAdjustSettings(const std::string& version) {
    // give the base class a chance to adjust any settings
    return RenderableEffect::needToAdjustSettings(version) || IsVersionOlder("2021.40", version);
}

void SingleStrandEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults) {
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
    if (IsVersionOlder("2020.57", version)) {
        SettingsMap& settings = effect->GetSettings();
        if (settings.Contains("E_CHOICE_Chase_Type1")) {
            std::string val = settings["E_CHOICE_Chase_Type1"];
            if (val == "Dual Bounce") {
                settings["E_CHOICE_Chase_Type1"] = "Dual Chase";
            }
        }
    }
    if (IsVersionOlder("2021.40", version)) {
        SettingsMap& sm = effect->GetSettings();
        wxString rzRotations = sm.Get("E_VALUECURVE_Chase_Rotations", "");
        if (rzRotations.Contains("VALUECURVE") && !rzRotations.Contains("RV=TRUE")) {
            ValueCurve vc;
            vc.SetLimits(1, 500);
            vc.Deserialise(rzRotations);
            sm["E_VALUECURVE_Chase_Rotations"] = vc.Serialise();
            wxASSERT(vc.IsRealValue());
        }
    }
}

void SingleStrandEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    if ("Skips" == SettingsMap["NOTEBOOK_SSEFFECT_TYPE"]) {
        RenderSingleStrandSkips(buffer, effect,
                                SettingsMap.GetInt("SLIDER_Skips_BandSize", 1),
                                SettingsMap.GetInt("SLIDER_Skips_SkipSize", 1),
                                SettingsMap.GetInt("SLIDER_Skips_StartPos", 1),
                                SettingsMap["CHOICE_Skips_Direction"],
                                SettingsMap.GetInt("SLIDER_Skips_Advance", 0));
    } else if ("FX" == SettingsMap["NOTEBOOK_SSEFFECT_TYPE"]) {
        RenderSingleStrandFX(buffer, effect,
                             GetValueCurveInt("FX_Intensity", 128, SettingsMap, eff_pos, SINGLESTRAND_FXINTENSITY_MIN, SINGLESTRAND_FXINTENSITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
                             GetValueCurveInt("FX_Speed", 128, SettingsMap, eff_pos, SINGLESTRAND_FXSPEED_MIN, SINGLESTRAND_FXSPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
                             SettingsMap.Get("CHOICE_SingleStrand_FX", "Blink"), SettingsMap.Get("CHOICE_SingleStrand_FX_Palette", "Default"));
    } else {
        RenderSingleStrandChase(buffer,
                                SettingsMap.Get("CHOICE_SingleStrand_Colors", "Palette"),
                                GetValueCurveInt("Number_Chases", 1, SettingsMap, eff_pos, SINGLESTRAND_CHASES_MIN, SINGLESTRAND_CHASES_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
                                GetValueCurveInt("Color_Mix1", 10, SettingsMap, eff_pos, SINGLESTRAND_COLOURMIX_MIN, SINGLESTRAND_COLOURMIX_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
                                SettingsMap.Get("CHOICE_Chase_Type1", "Left-Right"),
                                SettingsMap.GetBool("CHECKBOX_Chase_3dFade1", false),
                                SettingsMap.GetBool("CHECKBOX_Chase_Group_All", false),
                                GetValueCurveDouble("Chase_Rotations", 1.0, SettingsMap, eff_pos, SINGLESTRAND_ROTATIONS_MIN, SINGLESTRAND_ROTATIONS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), SINGLESTRAND_ROTATIONS_DIVISOR),
                                GetValueCurveDouble("Chase_Offset", 0.0, SettingsMap, eff_pos, SINGLESTRAND_OFFSET_MIN, SINGLESTRAND_OFFSET_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), SINGLESTRAND_OFFSET_DIVISOR));
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
        max++;
        max /= 2;
    }

    size_t colorcnt = buffer.GetColorCount();
    double position = buffer.GetEffectTimeIntervalPosition() * (advances + 1.0) * 0.99;

    x += int(position) * Skips_BandSize;
    while (x > max) {
        x -= (Skips_BandSize + Skips_SkipSize) * colorcnt;
    }

    if (buffer.needToInit)
    {
        buffer.needToInit = false;
        std::lock_guard<std::recursive_mutex> lock(eff->GetBackgroundDisplayList().lock);
        int rects = (Skips_SkipSize + Skips_BandSize) * (buffer.curEffEndPer - buffer.curEffStartPer + 1);
        eff->GetBackgroundDisplayList().resize(rects * 6);
    }

    int firstX = x;
    int colorIdx = 0;

    while (x < max)
    {
        buffer.palette.GetColor(colorIdx, color);
        colorIdx++;
        if (colorIdx >= colorcnt) colorIdx = 0;

        if (buffer.palette.IsSpatial(colorIdx))
        {
            buffer.palette.GetSpatialColor(colorIdx, 1.0 - (float)x / (float)max, 0, color);
        }

        for (int cnt = 0; cnt < Skips_BandSize && x < max; cnt++)
        {
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
    while (x >= 0)
    {
        x -= Skips_SkipSize;

        buffer.palette.GetColor(colorIdx, color);
        if (buffer.palette.IsSpatial(colorIdx))
        {
            buffer.palette.GetSpatialColor(colorIdx, 1.0 - (float)x / (float)firstX, 0, color);
        }

        colorIdx--;
        if (colorIdx < 0) colorIdx = buffer.GetColorCount() - 1;

        for (int cnt = 0; cnt < Skips_BandSize && x >= 0; cnt++)
        {
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
    if (max >= buffer.BufferWi) max = buffer.BufferWi - 1;

    buffer.CopyPixelsToDisplayListX(eff, 0, 0, max);
}

class SingleStrandFXRenderCache : public EffectRenderCache
{
public:
    SingleStrandFXRenderCache(RenderBuffer& buffer) {
        _fx = new WS2812FX();
    };
    virtual ~SingleStrandFXRenderCache(){
        if (_fx != nullptr)
            delete _fx;
    };

    WS2812FX* _fx = nullptr;
};

void SingleStrandEffect::RenderSingleStrandFX(RenderBuffer& buffer, Effect* eff, int intensity, int speed, const std::string& fx, const std::string& palette)
{
    SingleStrandFXRenderCache* cache = dynamic_cast <SingleStrandFXRenderCache*>(buffer.infoCache[id]);
    if (cache == nullptr) {

        // this could happen if the cache type changes
        if (buffer.infoCache[id] != nullptr) {
            delete buffer.infoCache[id];
        }

        cache = new SingleStrandFXRenderCache(buffer);
        buffer.infoCache[id] = cache;
    }

    auto pfx = cache->_fx;
    wxASSERT(pfx != nullptr);

    pfx->SetBuffer(&buffer);

    if (buffer.needToInit) {
        buffer.ClearTempBuf();
        pfx->SetBuffer(&buffer);
        pfx->setSegment(0, 0, buffer.BufferWi);
        pfx->setMode(0, DecodeMode(fx));
        pfx->getSegment(0).palette = DecodePalette(palette);
        pfx->finalizeInit();
    }

    pfx->getSegment(0).intensity = intensity;
    pfx->getSegment(0).speed = speed;
    if (buffer.GetColorCount() == 0) {
        pfx->getSegment(0).colors[0] = convertColour(xlWHITE);
    } else {
        for (size_t i = 0; i < NUM_COLORS; i++) {
            if (buffer.GetColorCount() > i) {
                pfx->getSegment(0).colors[i] = convertColour(buffer.GetPalette().GetColor(i)); // dont use set colour as it transitions
            } else {
                pfx->getSegment(0).colors[i] = 0xFF000000;
            }
        }
    }

    pfx->service();

    buffer.needToInit = false;
}

int mapChaseType(const std::string &Chase_Type) {
    if ("Left-Right" == Chase_Type) return 0;
    if ("Right-Left" == Chase_Type) return 1;
    if ("Bounce from Left" == Chase_Type) return 2;
    if ("Bounce from Right" == Chase_Type) return 3;
    if ("Dual Chase" == Chase_Type) return 4;
    if ("From Middle" == Chase_Type) return 5;
    if ("To Middle" == Chase_Type) return 6;
    if ("Bounce to Middle" == Chase_Type) return 7;
    if ("Bounce from Middle" == Chase_Type) return 8;
    if ("Static Left-Right" == Chase_Type) return 9;
    if ("Static Right-Left" == Chase_Type) return 10;
    if ("Static Dual" == Chase_Type) return 11;
    if ("Static From Middle" == Chase_Type) return 12;
    if ("Static To Middle" == Chase_Type) return 13;
    if ("Static Double-Ended" == Chase_Type) return 14;

    return 0;
}

void SingleStrandEffect::RenderSingleStrandChase(RenderBuffer &buffer,
    const std::string & ColorSchemeName, int Number_Chases, int chaseSize,
    const std::string &Chase_Type1,
    bool Chase_Fade3d1, bool Chase_Group_All,
    float chaseSpeed, float offset)
{
    int ColorScheme = "Palette" == ColorSchemeName;

    int chaseType = mapChaseType(Chase_Type1);

    int curEffStartPer, curEffEndPer;
    buffer.GetEffectPeriods(curEffStartPer, curEffEndPer);

    int MaxNodes;
    if (Chase_Group_All)
    {
        MaxNodes = buffer.BufferWi * buffer.BufferHt;
    }
    else
    {
        MaxNodes = buffer.BufferWi;
    }

    int ChaseDirection = (chaseType == 0 || chaseType == 2 || chaseType == 6 ||
                          chaseType == 9 || chaseType == 13 || chaseType == 14);

    if (buffer.needToInit)
    {
        buffer.needToInit = false;
    }

    bool Mirror = false;
    bool AutoReverse = false;
    bool Dual_Chases = false;
    bool Static = (chaseType >= 9 && chaseType <= 14);
    bool DoubleEnd = (chaseType == 14);
    switch (chaseType)
    {
    case 6:
    case 13:
        Mirror = true;
    case 0: // "Normal. L-R"
    case 9:
        break;
    case 5:
    case 12:
        Mirror = true;
    case 1: // "Normal. R-L"
    case 10:
        break;
    case 2: // "Auto reverse l-r"
        AutoReverse = true;
        break;
    case 3: // "Auto reverse r-l"
        AutoReverse = true;
        break;
    case 4: // "Dual Chase"
    case 11:
        Dual_Chases = true;
        break;
    case 7: // "Bounce to Middle"
    case 8: // "Bounce from Middle"
        Dual_Chases = true;
        AutoReverse = true;
        break;
    case 14:
        Dual_Chases = true;
        break;
    default:
        break;
    }

    int width;
    if (Chase_Group_All) {
        width = MaxNodes;
    }
    else {
        width = buffer.BufferWi;
    }
    
    if (Mirror) {
        if ((width % 2) == 0) {
            width /= 2;
        } else {
            width = (width + 1) / 2;
        }
    }

    if (width == 0) width = 1;

    // Make this a variable since it's used regularly
    int scaledChaseWidth = width * chaseSize / 100.0;
    // Make sure the chase is at least 1 pixel wide
    if (scaledChaseWidth < 1) {
        scaledChaseWidth = 1;
    }

    // This is a 0.0-1.0 value that determine how far along the current chase cycle we are
    double rtval = Static ? 0 : buffer.GetEffectTimeIntervalPosition();
    if (chaseType == 8) {
        // need to start in the middle for Bounce from Middle
        rtval += 0.25 / chaseSpeed;
        if (rtval > 1.0f) {
            rtval -= 1.0f;
        }
    }
    rtval *= chaseSpeed;
    rtval += (offset / 100);
    while (rtval > 1.0) {
        rtval -= 1.0;
    }
    while (rtval < 0.0) {
        rtval += 1.0;
    }
    if (AutoReverse) {
        rtval *= 2.0;
    }

    if (Number_Chases < 1) Number_Chases = 1;
    if (ColorScheme < 0) ColorScheme = 0;
    float dx = double(width) / double(Number_Chases);
    if (dx < 1.0) dx = 1.0;

    // All of this math needs to happen with integers because we can only deal with integer number of pixels when we render, so it doesn't do us any good to deal with floats
    int startState;
    // If we are wrapping, cap the width the buffer width
    if (Number_Chases > 1) {
        startState = width * rtval + 1;
    } else if (DoubleEnd) {
        startState = (width + scaledChaseWidth * 2 - 1) * rtval + 1; // -2) + 1
    } else
    {
        // If we aren't wrapping, add the chaseWidth to the total so the chase fully completes
        // The -1 assures divides the time appropriately so an equal amount of time is spent at each index
        // Imagine a 10 pixel wide buffer, and a 2 pixel wide chase. We want to start the chase with 1 pixel visible, and end with 1 pixel visible, so that means there are actually 11 time slots ( [width] + [scaledChaseWidth] - 1 )
        // The only problem is when we hit 1.0 exactly for the time, we'll roll over to the 12th slot, so the if statement fudges the math so the last slot actually gets 1 extra frame
        // The +1 assure that the chases start at index 0 rather than index -1
        startState = (width + scaledChaseWidth - 1) * rtval + 1; // -2) + 1
        if (rtval >= 0.999999) {
            startState -= 1;
        }
    }

    // Loop through each chase
    for (int chase = 0; chase < Number_Chases; chase++)
    {
        int x;
        // Bouncing chases
        if (AutoReverse) {
            x = chase * dx + width * rtval - scaledChaseWidth / 2.0;
        }
        // Full width chases
        else {
            x = chase * dx + startState - scaledChaseWidth; // L-R
        }

        draw_chase(buffer, DoubleEnd ? width - x - 1 * scaledChaseWidth : x, Chase_Group_All, ColorScheme, Number_Chases, AutoReverse, width, chaseSize, Chase_Fade3d1, bool(ChaseDirection) != DoubleEnd, Mirror); // Turn pixel on
        if (Dual_Chases)
        {
            draw_chase(buffer, DoubleEnd ? x - 1 * scaledChaseWidth : x, Chase_Group_All, ColorScheme, Number_Chases, AutoReverse, width, chaseSize, Chase_Fade3d1, bool(ChaseDirection) == DoubleEnd, Mirror);
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
    size_t colorcnt = buffer.GetColorCount();

    int max_chase_width = width * Chase_Width / 100.0;
    if (max_chase_width < 1) max_chase_width = 1;

    wxASSERT(Number_Chases != 0);

    int pixels_per_chase = width / Number_Chases;
    if (pixels_per_chase < 1) {
        pixels_per_chase = 1;
    }

    HSVValue hsv;
    buffer.palette.GetHSV(0, hsv);
    // if colour scheme is rainbow then we have to set colour to non-white
    if (ColorScheme == 0) {
        hsv = xlRED.asHSV();
    }
    float orig_v = hsv.value;

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
            int dif;
            if (firstX < 0) {
                firstX = -firstX - 1;
                direction = -1;
                dif = 0;
            }
            else {
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

    if (max_chase_width >= 1)
    {
        for (int i = 0; i < max_chase_width; i++)
        {
            xlColor color;
            if (ColorScheme == 0) {
                if (max_chase_width) hsv.hue = 1.0 - (i*1.0 / max_chase_width); // rainbow hue
                color = hsv;
            }

            int new_x;
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
            }
            else if (Number_Chases > 1) {
                new_x = x + i;

                while (new_x < 0) {
                    new_x += width;
                }
                while (new_x >= width) {
                    new_x -= width;
                }
            }
            else {
                new_x = x + i;
            }

            if (i < pixels_per_chase) // as long as the chase fits, keep drawing it
            {
                if (ChaseDirection == 0) {// are we going R-L?
                    new_x = width - new_x - 1;
                }

                if (ColorScheme != 0) {
                    int colorIdx;
                    if (colorcnt == 1) {
                        colorIdx = 0;
                    } else {
                        colorIdx = std::ceil(((double)((max_chase_width - i)*colorcnt)) / (double)max_chase_width) - 1;
                    }
                    if (colorIdx >= colorcnt) colorIdx = colorcnt - 1;

                    buffer.palette.GetColor(colorIdx, color);

                    if (buffer.palette.IsSpatial(colorIdx)) {
                        float x2 = 0;
                        if (colorcnt < max_chase_width) {
                            x2 = ((float)(i % (int)((float)max_chase_width / (float)colorcnt)));
                        }
                        float x3 = ((float)max_chase_width / (float)colorcnt);
                        buffer.palette.GetSpatialColor(colorIdx,  x2 / x3, 0.0, color);
                    }
                }

                if (Chase_Fade3d1) {
                    if (buffer.allowAlpha) {
                        color.alpha = 255.0 * (i + 1.0) / max_chase_width;
                    } else {
                        HSVValue hsv1 = color.asHSV();
                        hsv1.value = orig_v - ((max_chase_width - (i + 1.0)) / max_chase_width); // fades data down over chase width
                        if (hsv1.value < 0.0) hsv1.value = 0.0;
                        color = hsv1;
                    }
                }

                if (new_x >= 0 && new_x <= width) {
                    if (GroupAll) {
                        int y = 0;
                        int mirrorx = buffer.BufferWi*buffer.BufferHt - new_x - 1;
                        int mirrory = 0;
                        
                        y += new_x / buffer.BufferWi;
                        new_x = new_x % buffer.BufferWi;
                        mirrory += mirrorx / buffer.BufferWi;
                        mirrorx = mirrorx % buffer.BufferWi;
                        if (Chase_Fade3d1) {
                            xlColor c;
                            buffer.GetPixel(new_x, y, c);
                            if (c != xlBLACK) {
                                int a = color.alpha;
                                color = color.AlphaBlend(c);
                                color.alpha = c.alpha > a ? c.alpha : a;
                            }
                        }

                        buffer.SetPixel(new_x, y, color); // Turn pixel on
                        if (mirror) {
                            buffer.SetPixel(mirrorx, mirrory, color); // Turn pixel on
                        }
                    }
                    else {
                        if (Chase_Fade3d1) {
                            xlColor c;
                            buffer.GetPixel(new_x, 0, c);
                            if (c != xlBLACK) {
                                int a = color.alpha;
                                color = color.AlphaBlend(c);
                                color.alpha = c.alpha > a ? c.alpha : a;
                            }
                        }
                        for (int y = 0; y < buffer.BufferHt; y++) {
                            buffer.SetPixel(new_x, y, color); // Turn pixel on
                            if (mirror) {
                                buffer.SetPixel(buffer.BufferWi - new_x - 1, y, color); // Turn pixel on
                            }
                        }
                    }
                }
            }
        }
    }
}

