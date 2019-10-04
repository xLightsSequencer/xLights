#include "LOREdit.h"

#include <wx/wx.h>
#include <wx/uri.h>

#include "sequencer/TimeLine.h"
#include "UtilFunctions.h"
#include "models/Model.h"

#include "effects/SpiralsEffect.h"
#include "effects/ButterflyEffect.h"
#include "effects/BarsEffect.h"
#include "effects/CurtainEffect.h"
#include "effects/FireEffect.h"
#include "effects/GarlandsEffect.h"
#include "effects/MeteorsEffect.h"
#include "effects/PinwheelEffect.h"
#include "effects/SnowflakesEffect.h"

#include <log4cpp/Category.hh>

// Current working assumptions
//
// A prop can be sequenced using channels or tracks ... not both
// There can be multiple tracks
// Each effect on a track can have a left/right side
// Channels have rows and columns but i dont know how they work

std::string LOREditEffect::GetPalette() const
{
    if (type == loreditType::CHANNELS)
    {
        if (startColour != endColour)
        {
            // colour ramp - so a colour curve
            return "C_BUTTON_Palette1=Active=TRUE|Id=ID_BUTTON_Palette1|Values=x=0.000^c=" + (std::string)startColour + ";x=1.000^c=" + (std::string)endColour + "|,C_CHECKBOX_Palette1=1,"
                + "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0";
        }

        return "C_BUTTON_Palette1=" + (std::string)startColour + ",C_CHECKBOX_Palette1=1,"
            + "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0";
    }

    if (effectSettings.size() == 0) return "";

    std::string palette;

    wxArrayString c = wxSplit(effectSettings[0], ';');
    for (int i = 0; i < c.size(); i++)
    {
        wxArrayString cc = wxSplit(c[i], ',');

        if (cc.size() >= 2)
        {
            wxString c1 = cc[0].substr(2); // drop transparency
            wxString active = cc[1];

            palette += ",C_BUTTON_Palette" + wxString::Format("%d", i + 1) + "=#" + c1;
            if (active == "1")
            {
                palette += ",C_CHECKBOX_Palette" + wxString::Format("%d", i + 1) + "=" + active;
            }
        }
        else
        {
            // Not sure what the last value is so ignoring it
            //int unknown1 = wxAtoi(c[i]);
            break;
        }
    }

    int sparkle = wxAtoi(otherSettings[2]);

    if (sparkle > 0)
    {
        palette += ",C_SLIDER_SparkleFrequency=" + wxString::Format("%d", sparkle);
    }

    if (type == loreditType::TRACKS)
    {
        if (startIntensity == 100 && endIntensity == 100)
        {
            // dont need to do anything
        }
        else if (startIntensity == endIntensity)
        {
            // need to set brightness
            palette += ",C_SLIDER_Brightness=" + wxString::Format("%d", startIntensity);
        }
        else
        {
            palette += ",C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=" + wxString::Format("%d", startIntensity) + "|P2=" + wxString::Format("%d", endIntensity) + "|RV=TRUE|";
        }
    }

    return palette;
}

std::string LOREditEffect::GetxLightsEffect() const
{
    if (effectType == "INTENSITY") return "On";
    if (effectType == "DMX_INTENSITY") return "DMX";
    if (effectType == "SHIMMER") return "On";

    if (effectType == "colorwash") return "Color Wash";
    if (effectType == "picture") return "Pictures";
    if (effectType == "lineshorizontal") return "";
    if (effectType == "linesvertical") return "";
    if (effectType == "countdown") return "Text"; // we dont support countdown

    return wxString(effectType).Capitalize();

    return "";
}

// Used to rescale a parameter to a broader scale.
// Assumes the range is different but the translation is direct.
// If this is not the case then set the source Min/Max to the range that does map to the targetMin/Max and the conversion will
// clamp original values outside the supported range to the largest practical in the target
float LOREditEffect::Rescale(float original, float sourceMin, float sourceMax, float targetMin, float targetMax)

{
    if (original < sourceMin) original = sourceMin;
    if (original > sourceMax) original = sourceMax;

    return ((original - sourceMin) / (sourceMax - sourceMin))*(targetMax - targetMin) + targetMin;
}

wxString LOREditEffect::RescaleWithRangeI(wxString r, wxString vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, wxString& vc, float targetRealMin, float targetRealMax)
{
    if (r.Contains("R"))
    {
        // it is a range
        wxArrayString rr = wxSplit(r, 'R');
        vc = "," + vcName + "=Active=TRUE|Id=ID_" + vcName.substr(2) + "|Type=Ramp|Min=" + wxString::Format("%.2f", targetRealMin) +
            "|Max=" + wxString::Format("%.2f", targetRealMax) +
            "|P1=" + wxString::Format("%.2f", Rescale(wxAtof(rr[0]), sourceMin, sourceMax, targetMin, targetMax)) +
            "|P2=" + wxString::Format("%.2f", Rescale(wxAtof(rr[1]), sourceMin, sourceMax, targetMin, targetMax)) +
            "|RV=TRUE|";
        return wxString::Format("%d", (int)Rescale(wxAtof(rr[0]), sourceMin, sourceMax, targetMin, targetMax));
    }
    else
    {
        vc = "";
        return wxString::Format("%d", (int)Rescale(wxAtof(r), sourceMin, sourceMax, targetMin, targetMax));
    }
}

wxString LOREditEffect::RescaleWithRangeF(wxString r, wxString vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, wxString& vc, float targetRealMin, float targetRealMax)
{
    if (r.Contains("R"))
    {
        // it is a range
        wxArrayString rr = wxSplit(r, 'R');
        vc = "," + vcName + "=Active=TRUE|Id=ID_" + vcName.substr(2) + "|Type=Ramp|Min=" + wxString::Format("%.2f", targetRealMin) +
            "|Max=" + wxString::Format("%.2f", targetRealMax) +
            "|P1=" + wxString::Format("%.2f", Rescale(wxAtof(rr[0]), sourceMin, sourceMax, targetMin, targetMax)) +
            "|P2=" + wxString::Format("%.2f", Rescale(wxAtof(rr[1]), sourceMin, sourceMax, targetMin, targetMax)) +
            "|RV=TRUE|";
    }
    else
    {
        vc = "";
    }
    return wxString::Format("%.1f", Rescale(wxAtof(r), sourceMin, sourceMax, targetMin, targetMax));
}

std::string LOREditEffect::GetBlend() const
{
    if (!left) return "Normal";
    if (otherSettings.size() == 0) return "Normal";

    std::string blend = otherSettings[0];

    if (blend == "Mix_Average") return "Average";
    if (blend == "Mix_Overlay") return "Normal";
    if (blend == "Mix_Maximum") return "Max";
    if (blend == "Mix_Bottom_Top") return "Bottom-Top";
    if (blend == "Mix_Left-Right") return "Left-Right";
    if (blend == "Mix_Rt_Hides_Lt") return "1 is Mask";
    if (blend == "Mix_Rt_Reveals_Lt") return "2 reveals 1";

    return "Normal";
}

std::string LOREditEffect::GetSettings(std::string& palette) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (effectType == "INTENSITY")
    {
        // intensity ramp 0-100
        return wxString::Format("E_TEXTCTRL_Eff_On_End=%d,E_TEXTCTRL_Eff_On_Start=%d", endIntensity, startIntensity).ToStdString();
    }
    if (effectType == "DMX_INTENSITY")
    {
        return wxString::Format("E_VALUECURVE_DMX1=Active=TRUE|Id=ID_VALUECURVE_DMX1|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", startIntensity, endIntensity).ToStdString();
    }
    if (effectType == "SHIMMER")
    {
        return wxString::Format("E_CHECKBOX_On_Shimmer=1,E_TEXTCTRL_Eff_On_End=%d,E_TEXTCTRL_Eff_On_Start=%d", endIntensity, startIntensity).ToStdString();
    }
    if (effectType == "TWINKLE")
    {
        return "E_SLIDER_Twinkle_Count=48";
    }

    if (effectSettings.size() == 0)
    {
        wxASSERT(false);
        return "";
    }

    std::string et = effectType;
    if (StartsWith(et, "lightorama_"))
    {
        et = AfterFirst(et, '_');
    }

    std::string settings;
    auto parms = wxSplit(effectSettings[1], ',');
    if (et == "butterfly")
    {
        wxString style = parms[0];
        wxString chunks = parms[1];
        wxString vcChunks;
        chunks = RescaleWithRangeI(chunks, "E_VALUECURVE_Butterfly_Chunks", 1, 10, 1, 10, vcChunks, BUTTERFLY_CHUNKS_MIN, BUTTERFLY_CHUNKS_MAX);
        wxString skip = parms[2];
        wxString vcSkip;
        skip = RescaleWithRangeI(skip, "E_VALUECURVE_Butterfly_Skip", 2, 10, 2, 10, vcSkip, BUTTERFLY_SKIP_MIN, BUTTERFLY_SKIP_MAX);
        wxString direction = parms[3];
        wxString hue = parms[4];
        wxString vcHue;
        hue = RescaleWithRangeI(hue, "C_VALUECURVE_Color_HueAdjust", 0, 359, -100, 100, vcHue, -100, 100);
        wxString speed = parms[5];
        wxString vcSpeed;
        speed = RescaleWithRangeI(speed, "E_VALUECURVE_Butterfly_Speed", 0, 50, 0, 50, vcSpeed, BUTTERFLY_SPEED_MIN, BUTTERFLY_SPEED_MAX);
        wxString colours = parms[6];

        if (style == "linear")
        {
            settings += ",E_SLIDER_Butterfly_Style=1";
        }
        else if (style == "radial")
        {
            settings += ",E_SLIDER_Butterfly_Style=2";
        }
        else if (style == "blocks")
        {
            settings += ",E_SLIDER_Butterfly_Style=3";
        }
        else if (style == "corner")
        {
            settings += ",E_SLIDER_Butterfly_Style=5";
        }
        settings += ",E_CHOICE_Butterfly_Colors=" + colours.Capitalize();
        settings += ",E_CHOICE_Butterfly_Direction=" + direction.Capitalize();
        settings += ",E_SLIDER_Butterfly_Chunks=" + chunks;
        settings += vcChunks;
        settings += ",E_SLIDER_Butterfly_Skip=" + skip;
        settings += vcSkip;
        settings += ",E_SLIDER_Butterfly_Speed=" + speed;
        settings += vcSpeed;

        if (hue != "0")
        {
            palette += ",C_SLIDER_Color_HueAdjust=" + hue;
            palette += vcHue;
        }
    }
    else if (et == "colorwash")
    {
        //full, full, none, 12
        wxString horizontalFade = parms[0];
        wxString verticalFade = parms[1];

        if (horizontalFade == "full") {
        }
        else if (horizontalFade == "left_to_right") {
            settings += ",E_CHECKBOX_ColorWash_HFade=1";
        }
        else if (horizontalFade == "right_to_left") {
            settings += ",E_CHECKBOX_ColorWash_HFade=1";
        }
        else if (horizontalFade == "center_on") {
            settings += ",E_CHECKBOX_ColorWash_HFade=1";
        }
        else if (horizontalFade == "center_off") {
            settings += ",E_CHECKBOX_ColorWash_HFade=1";
        }

        if (verticalFade == "full") {
        }
        else if (verticalFade == "top_to_bottom") {
            settings += ",E_CHECKBOX_ColorWash_VFade=1";
        }
        else if (verticalFade == "bottom_to_top") {
            settings += ",E_CHECKBOX_ColorWash_VFade=1";
        }
        else if (verticalFade == "center_on") {
            settings += ",E_CHECKBOX_ColorWash_VFade=1";
        }
        else if (verticalFade == "center_off") {
            settings += ",E_CHECKBOX_ColorWash_VFade=1";
        }
    }
    else if (et == "spirals")
    {
        // 1, left_to_right, 20, 50, 0, False, none, 12
        wxString repeat = parms[0];
        wxString vcRepeat;
        repeat = RescaleWithRangeI(repeat, "E_VALUECURVE_Spirals_Count", 1, 5, 1, 5, vcRepeat, SPIRALS_COUNT_MIN, SPIRALS_COUNT_MAX);
        wxString direction = parms[1];
        wxString rotation = parms[2];
        rotation = wxString::Format("%.2f", wxAtof(rotation) / 60.0);
        wxString vcRotation;
        rotation = RescaleWithRangeF(rotation, "E_VALUECURVE_Spirals_Rotation", 0, 50, 0, 50, vcRotation, SPIRALS_ROTATION_MIN, SPIRALS_ROTATION_MAX);
        rotation = wxString::Format("%d", (int)(wxAtof(rotation) * 10.0));
        wxString thickness = parms[3];
        wxString vcThickness;
        thickness = RescaleWithRangeI(thickness, "E_VALUECURVE_Spirals_Thickness", 0, 100, 0, 100, vcThickness, SPIRALS_THICKNESS_MIN, SPIRALS_THICKNESS_MAX);
        wxString thicknessChange = parms[4];
        wxString blend = parms[5];
        wxString show3d = parms[6];
        wxString speed = parms[7];
        speed = wxString::Format("%d", (int)(wxAtof(speed) / (20.0 / ((float)(endMS - startMS) / 1000.0))));
        wxString vcSpeed;
        if (direction == "right_to_left")
        {
            speed = RescaleWithRangeF(speed, "E_VALUECURVE_Spirals_Movement", 0, 50, 0, -50, vcSpeed, SPIRALS_MOVEMENT_MIN, SPIRALS_MOVEMENT_MAX);
        }
        else
        {
            speed = RescaleWithRangeF(speed, "E_VALUECURVE_Spirals_Movement", 0, 50, 0, 50, vcSpeed, SPIRALS_MOVEMENT_MIN, SPIRALS_MOVEMENT_MAX);
        }

        settings += ",E_SLIDER_Spirals_Count=" + repeat;
        settings += vcRepeat;

        settings += ",E_TEXTCTRL_Spirals_Movement=" + speed;
        settings += vcSpeed;

        settings += ",E_SLIDER_Spirals_Rotation=" + rotation;
        settings += vcRotation;

        settings += ",E_SLIDER_Spirals_Thickness=" + thickness;
        settings += vcThickness;

        // dont know what to do with thickness change

        if (blend == "True")
        {
            settings += ",E_CHECKBOX_Spirals_Blend=1,";
        }

        if (show3d == "none") {
        }
        else if (show3d == "trail_left") {
            settings += ",E_CHECKBOX_Spirals_3D=1";
        }
        else if (show3d == "trail_right") {
            settings += ",E_CHECKBOX_Spirals_3D=1";
        }
    }
    else if (et == "bars")
    {
        // down,2,False,False,8,0
        wxString direction = parms[0];
        wxString repeat = parms[1];
        wxString vcRepeat;
        repeat = RescaleWithRangeI(repeat, "E_VALUECURVE_Bars_BarCount", 1, 5, 1, 5, vcRepeat, BARCOUNT_MIN, BARCOUNT_MAX);
        wxString highlight = parms[2];
        wxString show3d = parms[3];
        wxString speed = parms[4];
        speed = wxString::Format("%d", (int)(wxAtof(speed) / (20.0 / ((float)(endMS - startMS) / 1000.0))));
        wxString vcSpeed;
        speed = RescaleWithRangeF(speed, "E_VALUECURVE_Bars_Cycles", 0, 50, 0, 30, vcSpeed, BARCYCLES_MIN, BARCYCLES_MAX);
        wxString centre = parms[5];
        wxString vcCentre;
        centre = RescaleWithRangeI(centre, "E_VALUECURVE_Bars_Center", -50, 50, -100, 100, vcCentre, BARCENTER_MIN, BARCENTER_MAX);

        settings += ",E_SLIDER_Bars_BarCount=" + repeat;
        settings += vcRepeat;

        if (direction == "V_expand") direction = "expand";
        if (direction == "V_compress") direction = "compress";
        if (direction == "H_expand") direction = "H-expand";
        if (direction == "H_compress") direction = "H-compress";
        if (direction == "left") direction = "Left";
        if (direction == "right") direction = "Right";
        if (direction == "block_up") direction = "Alternate Up";
        if (direction == "block_down") direction = "Alternate Down";
        if (direction == "block_left") direction = "Alternate Right";
        if (direction == "block_right") direction = "Alternate Right";
        settings += ",E_CHOICE_Bars_Direction=" + direction;

        if (show3d == "True")
        {
            settings += ",E_CHECKBOX_Bars_3D=1";
        }

        if (highlight == "True")
        {
            settings += "E_CHECKBOX_Bars_Highlight=1";
        }

        settings += ",E_TEXTCTRL_Bars_Cycles=" + speed;
        settings += vcSpeed;

        settings += ",E_TEXTCTRL_Bars_Center=" + centre;
        settings += vcCentre;
    }
    else if (et == "countdown")
    {
        // 0,Arial,75,7
        wxString seconds = parms[0];
        wxString font = parms[1];
        wxString fontSize = parms[2];
        wxString vcCrap;
        fontSize = RescaleWithRangeI(fontSize, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
        wxString position = parms[3];
        position = RescaleWithRangeI(position, "IGNORE", -50, 50, -200, 200, vcCrap, -1, -1);

        settings += ",E_TEXTCTRL_Text=" + seconds;
        settings += ",E_CHOICE_Text_Count=seconds";
        settings += ",E_CHOICE_Text_Font=Use OS Fonts";
        settings += ",E_FONTPICKER_Text_Font='" + font + "' " + fontSize;
        settings += ",E_SLIDER_Text_XStart=" + position;
    }
    else if (et == "lineshorizontal")
    {
        // Bottom_to_Top,8,38

        // No xLights equivalent

        logger_base.warn("LPE conversion for Lines Horizontal does not exist.");
    }
    else if (et == "linesvertical")
    {
        // Left_to_Right,4,32

        // No xLights equivalent

        logger_base.warn("LPE conversion for Lines Vertical does not exist.");
    }
    else if (et == "curtain")
    {
        // center,open,0,once_at_speed,12
        wxString edge = parms[0];
        wxString movement = parms[1];
        wxString swag = parms[2];
        wxString vcSwag;
        swag = RescaleWithRangeF(swag, "E_VALUECURVE_Curtain_Swag", 0, 10, 0, 10, vcSwag, CURTAIN_SWAG_MIN, CURTAIN_SWAG_MAX);
        wxString repeat = parms[3];
        wxString speed = parms[4];
        wxString vcSpeed;
        speed = RescaleWithRangeF(speed, "E_VALUECURVE_Curtain_Speed", 0, 50, 0, 10, vcSpeed, CURTAIN_SPEED_MIN, CURTAIN_SPEED_MAX);

        settings += ",E_CHOICE_Curtain_Edge=" + edge;
        movement.Replace("_", " ");
        settings += ",E_CHOICE_Curtain_Effect=" + movement;
        settings += ",E_SLIDER_Curtain_Swag=" + swag;
        settings += vcSwag;

        if (repeat == "once_at_speed")
        {
            settings += ",E_CHECKBOX_Curtain_Repeat=0";
        }
        else if (repeat == "once_fit_to_duration")
        {
            settings += ",E_CHECKBOX_Curtain_Repeat=0";
        }
        else if (repeat == "repeat_at_speed_rotate_colors")
        {
            settings += ",E_CHECKBOX_Curtain_Repeat=1";
        }
        else if (repeat == "repeat_at_speed_blend_colors")
        {
            settings += ",E_CHECKBOX_Curtain_Repeat=1";
        }
        settings += ",E_TEXTCTRL_Curtain_Speed=" + speed;
        settings += vcSpeed;
    }
    else if (et == "fire")
    {
        //50,0
        wxString height = parms[0];
        wxString vcHeight;
        height = RescaleWithRangeI(height, "E_VALUECURVE_Fire_Height", 10, 100, 0, 100, vcHeight, FIRE_HEIGHT_MIN, FIRE_HEIGHT_MAX);
        wxString hueShift = parms[1];
        wxString vcHueShift;
        hueShift = RescaleWithRangeI(hueShift, "E_VALUECURVE_Fire_HueShift", 0, 359, 0, 100, vcHueShift, FIRE_HUE_MIN, FIRE_HUE_MAX);

        settings += ",E_SLIDER_Fire_Height=" + height;
        settings += vcHeight;
        settings += ",E_SLIDER_Fire_HueShift=" + hueShift;
        settings += vcHueShift;
    }
    else if (et == "fireworks")
    {
        // 10,50,2,30,normal,continuous
        wxString explosionRate = parms[0];
        wxString vcCrap;
        explosionRate = RescaleWithRangeI(explosionRate, "IGNORE", 1, 95, 1, 50, vcCrap, -1, -1);
        wxString particles = parms[1];
        particles = RescaleWithRangeI(particles, "IGNORE", 1, 100, 1, 100, vcCrap, -1, -1);
        wxString velocity = parms[2];
        velocity = RescaleWithRangeI(velocity, "IGNORE", 1, 10, 1, 10, vcCrap, -1, -1);
        wxString fade = parms[3];
        fade = RescaleWithRangeI(fade, "IGNORE", 1, 100, 1, 100, vcCrap, -1, -1);
        wxString pattern = parms[4]; // not used
        wxString rateChange = parms[5]; // not used
        settings += ",E_SLIDER_Fireworks_Explosions=" + explosionRate;
        settings += ",E_SLIDER_Fireworks_Count=" + particles;
        settings += ",E_SLIDER_Fireworks_Fade=" + fade;
        settings += ",E_SLIDER_Fireworks_Velocity=" + velocity;
    }
    else if (et == "garland")
    {
        // 3,34,once_at_speed,12,bottom_to_top
        wxString type = parms[0];
        wxString vcCrap;
        type = RescaleWithRangeI(type, "IGNORE", 0, 4, 0, 4, vcCrap, -1, -1);
        wxString spacing = parms[1];
        wxString vcSpacing;
        spacing = RescaleWithRangeI(spacing, "E_VALUECURVE_Garlands_Spacing", 0, 100, 1, 100, vcSpacing, GARLANDS_SPACING_MIN, GARLANDS_SPACING_MAX);
        wxString repeat = parms[2];
        wxString speed = parms[3];
        wxString vcSpeed;
        speed = RescaleWithRangeF(speed, "E_VALUECURVE_Garlands_Cycles", 0, 50, 0, 20, vcSpeed, GARLANDS_CYCLES_MIN, GARLANDS_CYCLES_MAX);
        wxString fill = parms[4];

        settings += ",E_SLIDER_Garlands_Type=" + type;

        if (fill == "bottom_to_top")
        {
            settings += ",E_CHOICE_Garlands_Direction=Up";
        }
        else if (fill == "top_to_bottom")
        {
            settings += ",E_CHOICE_Garlands_Direction=Down";
        }
        else if (fill == "left_to_right")
        {
            settings += ",E_CHOICE_Garlands_Direction=Right";
        }
        else if (fill == "right_to_left")
        {
            settings += ",E_CHOICE_Garlands_Direction=Left";
        }

        settings += ",E_SLIDER_Garlands_Spacing=" + spacing;
        settings += vcSpacing;

        if (repeat == "repeat_at_speed")
        {
            settings += ",E_TEXTCTRL_Garlands_Cycles=" + speed;
            settings += vcSpeed;
        }
        else if (repeat == "once_at_speed")
        {
            settings += ",E_TEXTCTRL_Garlands_Cycles=1.0";
        }
        else if (repeat == "once_fit_to_duration")
        {
            settings += ",E_TEXTCTRL_Garlands_Cycles=1.0";
        }
    }
    else if (et == "meteors")
    {
        // rainbow,10,25,down,0,12
        wxString colourScheme = parms[0];
        wxString count = parms[1];
        wxString vcCount;
        count = RescaleWithRangeI(count, "E_VALUECURVE_Meteors_Count", 1, 100, 1, 100, vcCount, METEORS_COUNT_MIN, METEORS_COUNT_MAX);
        wxString length = parms[2];
        wxString vcLength;
        length = RescaleWithRangeI(length, "E_VALUECURVE_Meteors_Length", 1, 100, 1, 100, vcLength, METEORS_LENGTH_MIN, METEORS_LENGTH_MAX);
        wxString effect = parms[3];
        wxString swirl = parms[4];
        wxString vcSwirl;
        swirl = RescaleWithRangeI(swirl, "E_VALUECURVE_Meteors_Swirl_Intensity", 0, 20, 0, 20, vcSwirl, METEORS_SWIRL_MIN, METEORS_SWIRL_MAX);
        wxString speed = parms[5];
        wxString vcSpeed;
        speed = RescaleWithRangeI(speed, "E_VALUECURVE_Meteors_Speed", 1, 50, 1, 50, vcSpeed, METEORS_SPEED_MIN, METEORS_SPEED_MAX);

        settings += ",E_CHOICE_Meteors_Type=" + colourScheme.Lower();
        settings += ",E_SLIDER_Meteors_Count=" + count;
        settings += vcCount;
        settings += ",E_SLIDER_Meteors_Length=" + length;
        settings += vcLength;
        settings += ",E_CHOICE_Meteors_Effect=" + effect.Capitalize();
        settings += ",E_SLIDER_Meteors_Swirl_Intensity=" + swirl;
        settings += vcSwirl;
        settings += ",E_SLIDER_Meteors_Speed=" + speed;
        settings += vcSpeed;
    }
    else if (et == "movie")
    {
        // xxx.avi,True,False
        wxString file = parms[0];
        wxString scale = parms[1];
        wxString fullLength = parms[2];

        settings += ",E_FILEPICKERCTRL_Video_Filename=" + file;

        if (scale == "True")
        {
            settings += ",E_CHECKBOX_Video_AspectRatio=0";
        }
        else
        {
            settings += ",E_CHECKBOX_Video_AspectRatio=1";
        }
        if (fullLength == "True")
        {
            settings += ",E_CHOICE_Video_DurationTreatment=Slow/Accelerate";
        }
        else
        {
            settings += ",E_CHOICE_Video_DurationTreatment=Normal";
        }
    }
    else if (et == "picture")
    {
        // file.jpg,True,none,0,10,19,12
        wxString file = parms[0];
        wxString scale = parms[1];
        wxString movement = parms[2];
        wxString x = parms[3];
        wxString vcCrap;
        x = RescaleWithRangeI(x, "IGNORE", -50, 50, -100, 100, vcCrap, -1, -1);
        wxString peekabooHoldTime = parms[4]; // not used
        peekabooHoldTime = RescaleWithRangeI(peekabooHoldTime, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
        wxString wiggle = parms[5]; // not used
        wiggle = RescaleWithRangeI(wiggle, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
        wxString speed = parms[6];
        speed = RescaleWithRangeF(speed, "IGNORE", 0, 50, 0, 20, vcCrap, -1, -1);

        while (file.Contains("%"))
        {
            int pos = file.Find("%");
            if (pos < file.Length() - 2)
            {
                char c = HexToChar(file[pos+1], file[pos + 2]);
                file.Replace(file.substr(pos, 3), wxString(c));
            }
        }

        settings += ",E_FILEPICKER_Pictures_Filename=" + file;
        if (scale == "True")
        {
            settings += ",E_CHOICE_Scaling=Scale To Fit";
        }
        else
        {
            settings += ",E_CHOICE_Scaling=No Scaling";
        }

        movement.Replace("_", "-");
        if (movement == "peekaboo-bottom")
        {
            movement = "peekaboo";
        }
        else if (movement == "peekaboo-top")
        {
            movement = "peekaboo 180";
        }
        else if (movement == "peekaboo-left")
        {
            movement = "peekaboo 90";
        }
        else if (movement == "peekaboo-right")
        {
            movement = "peekaboo 270";
        }
        settings += ",E_CHOICE_Pictures_Direction=" + movement;
        settings += ",E_SLIDER_PicturesXC=" + x;
        settings += ",E_TEXTCTRL_Pictures_Speed=" + speed;
    }
    else if (et == "pinwheel")
    {
        // 3,1,6,color_per_arm,True,12,100,10,-23

        wxString arms = parms[0];
        wxString vcCrap;
        arms = RescaleWithRangeI(arms, "IGNORE", 1, 10, 1, 10, vcCrap, -1, -1);
        wxString width = parms[1];
        wxString vcWidth;
        width = RescaleWithRangeI(width, "E_VALUECURVE_Pinwheel_Thickness", 1, 10, 0, 100, vcWidth, PINWHEEL_THICKNESS_MIN, PINWHEEL_THICKNESS_MAX);
        wxString bend = parms[2];
        wxString vcBend;
        bend = RescaleWithRangeI(bend, "E_VALUECURVE_Pinwheel_Twist", -10, 10, -360, 360, vcBend, PINWHEEL_TWIST_MIN, PINWHEEL_TWIST_MAX);
        wxString colour = parms[3]; // not used
        wxString CCW = parms[4];
        wxString speed = parms[5];
        wxString vcSpeed;
        speed = RescaleWithRangeI(speed, "E_VALUECURVE_Pinwheel_Speed", 0, 50, 0, 50, vcSpeed, PINWHEEL_SPEED_MIN, PINWHEEL_SPEED_MAX);
        wxString length = parms[6];
        wxString vcLength;
        length = RescaleWithRangeI(length, "E_VALUECURVE_Pinwheel_ArmSize", 1, 100, 0, 400, vcLength, PINWHEEL_ARMSIZE_MIN, PINWHEEL_ARMSIZE_MAX);
        wxString x = parms[7];
        wxString vcX;
        x = RescaleWithRangeI(x, "E_VALUECURVE_PinwheelXC", -50, 50, -100, 100, vcX, PINWHEEL_X_MIN, PINWHEEL_X_MAX);
        wxString y = parms[8];
        wxString vcY;
        y = RescaleWithRangeI(y, "E_VALUECURVE_PinwheelYC", -50, 50, -100, 100, vcY, PINWHEEL_Y_MIN, PINWHEEL_Y_MAX);

        settings += ",E_SLIDER_Pinwheel_Arms=" + arms;
        settings += ",E_SLIDER_Pinwheel_Thickness=" + width;
        settings += vcWidth;
        settings += ",E_SLIDER_Pinwheel_Twist=" + bend;
        settings += vcBend;
        if (CCW == "True")
        {
            settings += ",E_CHECKBOX_Pinwheel_Rotation=1";
        }
        else
        {
            settings += ",E_CHECKBOX_Pinwheel_Rotation=0";
        }
        settings += ",E_SLIDER_Pinwheel_Speed=" + speed;
        settings += vcSpeed;
        settings += ",E_SLIDER_Pinwheel_ArmSize=" + length;
        settings += vcLength;
        settings += ",E_CHOICE_Pinwheel_Style=New Render Method";
        settings += ",E_SLIDER_PinwheelXC=" + x;
        settings += vcX;
        settings += ",E_SLIDER_PinwheelYC=" + y;
        settings += vcY;
    }
    else if (et == "snowflakes")
    {
        //5,1,0,12,60
        wxString count = parms[0];
        wxString vcCount;
        count = RescaleWithRangeI(count, "E_VALUECURVE_Snowflakes_Count", 1, 20, 1, 20, vcCount, SNOWFLAKES_COUNT_MIN, SNOWFLAKES_COUNT_MAX);
        wxString type = parms[1];
        wxString vcCrap;
        type = RescaleWithRangeI(type, "IGNORE", 0, 5, 0, 5, vcCrap, -1, -1);
        wxString direction = parms[2]; // not used
        direction = RescaleWithRangeI(direction, "IGNORE", -8, 8, -8, 8, vcCrap, -1, -1);
        wxString speed = parms[3];
        wxString vcSpeed;
        speed = RescaleWithRangeI(speed, "E_VALUECURVE_Snowflakes_Speed", 0, 50, 0, 50, vcSpeed, SNOWFLAKES_SPEED_MIN, SNOWFLAKES_SPEED_MAX);
        wxString accumulation = parms[4];
        accumulation = RescaleWithRangeI(accumulation, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);

        settings += ",E_SLIDER_Snowflakes_Count=" + count;
        settings += ",E_SLIDER_Snowflakes_Type=" + type;
        settings += ",E_SLIDER_Snowflakes_Speed=" + speed;
        if (accumulation == "0")
        {
            settings += ",E_CHOICE_Falling=Falling";
            settings += vcCount;
            settings += vcSpeed;
        }
        else
        {
            settings += ",E_CHOICE_Falling=Falling & Accumulating";
            settings += vcCount;
            settings += vcSpeed;
        }
    }
    else if (et == "text")
    {
        //Hello%26nbsp%3B%20Keith,50,left,0,10,0,4,True
        wxString text = wxURI::Unescape(parms[0]);
        text.Replace("&gt;", ">");
        text.Replace("&lt;", "<");
        text.Replace("&nbsp;", " ");
        text.Replace("&amp;", "&");
        wxString fontSize = parms[1];
        wxString vcCrap;
        fontSize = RescaleWithRangeI(fontSize, "IGNORE", 0, 149, 0, 149, vcCrap, -1, -1);
        wxString movement = parms[2];
        wxString position = parms[3];
        position = RescaleWithRangeI(position, "IGNORE", -50, 49, -200, 200, vcCrap, -1, -1);
        wxString peekabooHoldTime = parms[4]; // unused
        peekabooHoldTime = RescaleWithRangeI(peekabooHoldTime, "IGNORE", 0, 99, 0, 99, vcCrap, -1, -1);
        wxString bounce = parms[5]; // unused
        bounce = RescaleWithRangeI(bounce, "IGNORE", 0, 99, 0, 99, vcCrap, -1, -1);
        wxString speed = parms[6];
        speed = RescaleWithRangeI(speed, "IGNORE", 0, 50, 0, 50, vcCrap, -1, -1);
        if (parms.size() > 7)
        {
            wxString unknown1 = parms[7]; // unused
        }

        settings += ",E_TEXTCTRL_Text=" + text;
        settings += ",E_CHOICE_Text_Font=Use OS Fonts";
        settings += ",E_FONTPICKER_Text_Font='segoe ui' " + fontSize;

        if (movement == "peekaboo_bottom")
        {
            settings += ",E_CHECKBOX_TextToCenter=1";
            movement = "up";
        }
        if (movement == "peekaboo_top")
        {
            settings += ",E_CHECKBOX_TextToCenter=1";
            movement = "down";
        }
        if (movement == "peekaboo_left")
        {
            settings += ",E_CHECKBOX_TextToCenter=1";
            movement = "left";
        }
        if (movement == "peekaboo_right")
        {
            settings += ",E_CHECKBOX_TextToCenter=1";
            movement = "right";
        }
        settings += ",E_CHOICE_Text_Dir=" + movement;
        settings += ",E_SLIDER_Text_XStart=" + position;
        settings += ",E_TEXTCTRL_Text_Speed=" + speed;
    }
    else if (et == "twinkle")
    {
        // 50,25,twinkle,random
        wxString rate = parms[0];
        wxString vcCrap;
        rate = RescaleWithRangeI(rate, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
        wxString density = parms[1];
        density = RescaleWithRangeI(density, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
        wxString mode = parms[2];
        wxString layout = parms[3];

        settings += ",E_SLIDER_Twinkle_Count=" + density;
        settings += ",E_SLIDER_Twinkle_Steps=" + rate;
        if (layout == "interval")
        {
            settings += ",E_CHECKBOX_Twinkle_ReRandom=0";
        }
        else if (layout == "random")
        {
            settings += ",E_CHECKBOX_Twinkle_ReRandom=1";
        }

        if (mode == "twinkle")
        {
            settings += ",E_CHECKBOX_Twinkle_Strobe=0";
        }
        else // pulse/flash
        {
            settings += ",E_CHECKBOX_Twinkle_Strobe=1";
        }
    }
    else
    {
        logger_base.warn("S5 conversion for %s not created yet.", (const char *)et.c_str());
        wxASSERT(false);
    }

    wxString blend = GetBlend();
    settings += ",T_CHOICE_LayerMethod=" + blend;

    int blendPos = wxAtoi(otherSettings[1]);
    if (left && blendPos > 0)
    {
        settings += ",T_SLIDER_EffectLayerMix=" + wxString::Format("%d", blendPos);
    }

    return settings;
}

LOREdit::LOREdit(wxXmlDocument& input_xml, int frequency) : _input_xml(input_xml), _frequency(frequency)
{

}

// gets a list of all the free timing tracks
std::vector<std::string> LOREdit::GetTimingTracks() const
{
    std::vector<std::string> res;

    for (wxXmlNode* e = _input_xml.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "TimingGrids") {
            for (wxXmlNode* timing = e->GetChildren(); timing != nullptr; timing = timing->GetNext()) {
                if (timing->GetName() == "TimingGridFree") {
                    res.push_back(timing->GetAttribute("name", "").ToStdString());
                }
            }
        }
    }

    return res;
}

// returns the timing begin/end for the named timing track
std::vector<std::pair<uint32_t, uint32_t>> LOREdit::GetTimings(const std::string& timingTrackName, int offset) const
{
    std::vector<std::pair<uint32_t, uint32_t>> res;
    for (wxXmlNode* e = _input_xml.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "TimingGrids") {
            for (wxXmlNode* timing = e->GetChildren(); timing != nullptr; timing = timing->GetNext()) {
                if (timing->GetName() == "TimingGridFree") {
                    if (timing->GetAttribute("name", "") == timingTrackName) {
                        int lastMS = offset;
                        if (lastMS < 0) lastMS = 0;
                        for (wxXmlNode* t = timing->GetChildren(); t != nullptr; t = t->GetNext())
                        {
                            if (t->GetName() == "timing")
                            {
                                int time = wxAtoi(t->GetAttribute("centisecond")) * 10 + offset;
                                int adjTime = TimeLine::RoundToMultipleOfPeriod(time, _frequency);
                                if (adjTime > lastMS)
                                {
                                    res.push_back({ lastMS, adjTime });
                                }
                                lastMS = adjTime;
                            }
                        }
                    }
                }
            }
        }
    }
    return res;
}

// Uses prop definition to work out how many strands a model has
// that can then be used out to work out channel sequencing mapping
int LOREdit::GetModelStrands(const std::string& model) const
{
    for (wxXmlNode* e = _input_xml.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "PreviewClass") {
            for (wxXmlNode* prop = e->GetChildren(); prop != nullptr; prop = prop->GetNext()) {
                if (prop->GetName() == "PropClass") {
                    if (prop->GetAttribute("name") == model) {
                        wxString grid = prop->GetAttribute("ChannelGrid");
                        int strands = 1;
                        for (const auto it : grid) {
                            if (it == ';') strands++;
                        }
                        return strands;
                    }
                }
            }
        }
    }
    return 0;
}

// Calculate the number of layers necessary for pixel effects on this model
// basically one per track * whether or not there are effects on left and right
int LOREdit::GetModelLayers(const std::string& model) const
{
    int count = 0;
    for (wxXmlNode* e = _input_xml.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "SequenceProps" || e->GetName() == "ArchivedProps") {
            for (wxXmlNode* prop = e->GetChildren(); prop != nullptr; prop = prop->GetNext()) {
                if (prop->GetName() == "SeqProp" || prop->GetName() == "ArchiveProp") {
                    std::string name = prop->GetAttribute("name").ToStdString();
                    if (name == "")
                    {
                        for (wxXmlNode* ap = prop->GetChildren(); ap != nullptr; ap = ap->GetNext()) {
                            if (ap->GetName() == "PropClass")
                            {
                                name = ap->GetAttribute("Name");
                            }
                        }
                    }
                    if (name == model) {
                        for (wxXmlNode* tc = prop->GetChildren(); tc != nullptr; tc = tc->GetNext()) {
                            if (tc->GetName() == "track") {
                                int l1 = 0;
                                int l2 = 0;
                                for (wxXmlNode* ef = tc->GetChildren(); (l1 == 0 || l2 == 0) && ef != nullptr; ef = ef->GetNext()) {
                                    int ll1 = 0;
                                    int ll2 = 0;
                                    GetLayers(ef->GetAttribute("settings"), ll1, ll2);
                                    if (ll1 == 1) l1 = 1;
                                    if (ll2 == 1) l2 = 1;
                                }
                                count += l1 + l2;
                            }
                        }
                    }
                }
            }
        }
    }

    return count;
}

// If a prop has sequenced channels this calculates how many there are
int LOREdit::GetModelChannels(const std::string& model, int& rows, int& cols) const
{
    rows = 0;
    cols = 0;
    int count = 0;
    for (wxXmlNode* e = _input_xml.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "SequenceProps" || e->GetName() == "ArchivedProps") {
            for (wxXmlNode* prop = e->GetChildren(); prop != nullptr; prop = prop->GetNext()) {
                if (prop->GetName() == "SeqProp" || prop->GetName() == "ArchiveProp") {
                    std::string name = prop->GetAttribute("name").ToStdString();
                    if (name == "")
                    {
                        for (wxXmlNode* ap = prop->GetChildren(); ap != nullptr; ap = ap->GetNext()) {
                            if (ap->GetName() == "PropClass")
                            {
                                name = ap->GetAttribute("Name");
                            }
                        }
                    }
                    if (name == model)
                    {
                        for (wxXmlNode* tc = prop->GetChildren(); tc != nullptr; tc = tc->GetNext()) {
                            if (tc->GetName() == "channel") {
                                if (tc->GetChildren() != nullptr)
                                {
                                    rows = std::max(rows, wxAtoi(tc->GetAttribute("row", "0")) + 1);
                                    cols = std::max(cols, wxAtoi(tc->GetAttribute("col", "0")) + 1);
                                    count++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return count;
}

// returns the type of sequencing on the named model
// assumes you cant have both channel and track sequencing on the same model ... this may not be true
loreditType LOREdit::GetSequencingType(const std::string& model) const
{
    for (wxXmlNode* e = _input_xml.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "SequenceProps" || e->GetName() == "ArchivedProps") {
            for (wxXmlNode* prop = e->GetChildren(); prop != nullptr; prop = prop->GetNext()) {
                if (prop->GetName() == "SeqProp" || prop->GetName() == "ArchiveProp") {
                    std::string name = prop->GetAttribute("name").ToStdString();
                    if (name == "")
                    {
                        for (wxXmlNode* ap = prop->GetChildren(); ap != nullptr; ap = ap->GetNext()) {
                            if (ap->GetName() == "PropClass")
                            {
                                name = ap->GetAttribute("Name");
                            }
                        }
                    }
                    if (name == model)
                    {
                        for (wxXmlNode* tc = prop->GetChildren(); tc != nullptr; tc = tc->GetNext()) {
                            if (tc->GetName() == "channel" && tc->GetChildren() != nullptr) {
                                return loreditType::CHANNELS;
                            }
                            if (tc->GetName() == "track" && tc->GetChildren() != nullptr)
                            {
                                return loreditType::TRACKS;
                            }
                        }
                    }
                }
            }
        }
    }
    return loreditType::NONE;
}

std::vector<std::string> LOREdit::GetModelsWithEffects() const
{
    std::vector<std::string> res;
    for (wxXmlNode* e = _input_xml.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "SequenceProps" || e->GetName() == "ArchivedProps") {
            for (wxXmlNode* prop = e->GetChildren(); prop != nullptr; prop = prop->GetNext()) {
                if (prop->GetName() == "SeqProp" || prop->GetName() == "ArchiveProp") {
                    std::string name = prop->GetAttribute("name").ToStdString();
                    if (name == "")
                    {
                        for (wxXmlNode* ap = prop->GetChildren(); ap != nullptr; ap = ap->GetNext()) {
                            if (ap->GetName() == "PropClass")
                            {
                                name = ap->GetAttribute("Name");
                            }
                        }
                    }
                    if (name != "")
                    {
                        for (wxXmlNode* tc = prop->GetChildren(); tc != nullptr; tc = tc->GetNext()) {
                            if ((tc->GetName() == "channel" || tc->GetName() == "track") && tc->GetChildren() != nullptr) {
                                res.push_back(name);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    return res;
}

void LOREdit::GetLayers(const std::string& settings, int& ll1, int& ll2)
{
    ll1 = 0;
    ll2 = 0;
    auto ss = wxSplit(settings, '|');

    if (ss.size() == 8)
    {
        if (ss[5].StartsWith("lightorama_") && !ss[5].StartsWith("lightorama_none"))
        {
            ll1 = 1;
        }
        if (ss[6].StartsWith("lightorama_") && !ss[6].StartsWith("lightorama_none"))
        {
            ll2 = 1;
        }
    }
}

std::vector<LOREditEffect> LOREdit::AddEffects(wxXmlNode* track, bool left, int offset) const
{
    std::vector<LOREditEffect> res;

    for (wxXmlNode* ef = track->GetChildren(); ef != nullptr; ef = ef->GetNext()) {
        LOREditEffect effect;
        effect.left = left;
        effect.startMS = wxAtoi(ef->GetAttribute("startCentisecond")) * 10 + offset;
        effect.endMS = wxAtoi(ef->GetAttribute("endCentisecond")) * 10 + offset;
        int si = wxAtoi(ef->GetAttribute("intensity", "9999"));
        if (si != 9999)
        {
            effect.startColour = xlWHITE;
            effect.endColour = xlWHITE;
            effect.startIntensity = si;
            effect.endIntensity = si;
        }
        else
        {
            effect.startIntensity = wxAtoi(ef->GetAttribute("startIntensity", "100"));
            effect.endIntensity = wxAtoi(ef->GetAttribute("endIntensity", "100"));
            effect.startColour = xlWHITE;
            effect.endColour = xlWHITE;
        }
        effect.type = loreditType::TRACKS;

        auto s = ef->GetAttribute("settings");
        auto ss = wxSplit(s, '|');
        
        if (ss.size() == 8)
        {
            wxString es;
            if (left)
            {
                es = ss[5];
            }
            else
            {
                es = ss[6];
            }
            auto ees = wxSplit(es, ':');

            if (ees.size() > 0)
            {
                if (ees[0].StartsWith("lightorama_"))
                {
                    if (!ees[0].StartsWith("lightorama_none"))
                    {
                        effect.effectType = ees[0].AfterFirst('_');
                        for (auto it2 : ees)
                        {
                            if (!it2.StartsWith("lightorama_"))
                            {
                                effect.effectSettings.push_back(it2.ToStdString());
                            }
                        }
                    }
                }
            }
            effect.otherSettings.push_back(ss[0].ToStdString()); // blend
            effect.otherSettings.push_back(ss[1].ToStdString());
            effect.otherSettings.push_back(ss[2].ToStdString());
            effect.otherSettings.push_back(ss[3].ToStdString());
            effect.otherSettings.push_back(ss[4].ToStdString());
            effect.otherSettings.push_back(ss[7].ToStdString());
        }

        res.push_back(effect);
    }

    return res;
}

std::vector<LOREditEffect> LOREdit::GetTrackEffects(const std::string& model, int layer, int offset) const
{
    std::vector<LOREditEffect> res;

    for (wxXmlNode* e = _input_xml.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "SequenceProps" || e->GetName() == "ArchivedProps") {
            for (wxXmlNode* prop = e->GetChildren(); prop != nullptr; prop = prop->GetNext()) {
                if (prop->GetName() == "SeqProp" || prop->GetName() == "ArchiveProp") {
                    std::string name = prop->GetAttribute("name").ToStdString();
                    if (name == "")
                    {
                        for (wxXmlNode* ap = prop->GetChildren(); ap != nullptr; ap = ap->GetNext()) {
                            if (ap->GetName() == "PropClass")
                            {
                                name = ap->GetAttribute("Name");
                            }
                        }
                    }
                    if (name == model)
                    {
                        int tcount = 0;
                        for (wxXmlNode* tc = prop->GetChildren(); tc != nullptr; tc = tc->GetNext()) {
                            if ((tc->GetName() == "track")) {
                                if (tc->GetChildren() != nullptr)
                                {
                                    int l1 = 0;
                                    int l2 = 0;
                                    for (wxXmlNode* ef = tc->GetChildren(); (l1 == 0 || l2 == 0) && ef != nullptr; ef = ef->GetNext()) {
                                        int ll1, ll2;
                                        GetLayers(ef->GetAttribute("settings"), ll1, ll2);
                                        if (ll1 == 1) l1 = 1;
                                        if (ll2 == 1) l2 = 1;
                                    }

                                    if (tcount == layer && l1 == 1)
                                    {
                                        return AddEffects(tc, true, offset);
                                    }
                                    if (l1 == 1) tcount++;

                                    if (l2 == 1 && tcount == layer)
                                    {
                                        return AddEffects(tc, false, offset);
                                    }
                                    if (l2 == 1) tcount++;
                                }
                            }
                        }
                        return res;
                    }
                }
            }
        }
    }

    return res;
}

std::vector<LOREditEffect> LOREdit::GetChannelEffects(const std::string& model, int channel, Model* m, int offset) const
{
    std::vector<LOREditEffect> res;

    if (m == nullptr)
    {
        wxASSERT(false); return res;
    }

    int rows = 0;
    int cols = 0;
    int channels = GetModelChannels(model, rows, cols);

    if (channel >= channels)
    {
        channel = channels - 1;
    }

    int mw = m->GetDefaultBufferWi();
    int mh = m->GetDefaultBufferHt();

    wxASSERT(rows == mh);
    wxASSERT(cols == mw);

    std::vector<wxPoint> coords;
    m->GetNodeCoords(channel, coords);
    int bufx = 0;
    int bufy = 0;
    if (coords.size() != 0)
    {
        bufx = coords[0].x;
        bufy = coords[0].y;
    }
    else
    {
        // this is not encouraging
    }

    int targetRow = bufy;
    int targetCol = bufx;

    for (wxXmlNode* e = _input_xml.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "SequenceProps" || e->GetName() == "ArchivedProps") {
            for (wxXmlNode* prop = e->GetChildren(); prop != nullptr; prop = prop->GetNext()) {
                if (prop->GetName() == "SeqProp" || prop->GetName() == "ArchiveProp") {
                    std::string name = prop->GetAttribute("name").ToStdString();
                    if (name == "")
                    {
                        for (wxXmlNode* ap = prop->GetChildren(); ap != nullptr; ap = ap->GetNext()) {
                            if (ap->GetName() == "PropClass")
                            {
                                name = ap->GetAttribute("Name");
                            }
                        }
                    }
                    if (name == model)
                    {
                        for (wxXmlNode* tc = prop->GetChildren(); tc != nullptr; tc = tc->GetNext()) {
                            if ((tc->GetName() == "channel")) {
                                int row = wxAtoi(tc->GetAttribute("row", "0"));
                                int col = wxAtoi(tc->GetAttribute("col", "0"));

                                if (row == targetRow && col == targetCol)
                                {
                                    for (wxXmlNode* ef = tc->GetChildren(); ef != nullptr; ef = ef->GetNext()) {
                                        LOREditEffect effect;
                                        effect.pixelChannels = prop->GetAttribute("EnablePixelChannels", "0") == "1";
                                        effect.startMS = wxAtoi(ef->GetAttribute("startCentisecond")) * 10 + offset;
                                        effect.endMS = wxAtoi(ef->GetAttribute("endCentisecond")) * 10 + offset;
                                        int si = wxAtoi(ef->GetAttribute("intensity", "9999"));
                                        if (si != 9999)
                                        {
                                            if (si < 0)
                                            {
                                                if (ef->GetAttribute("settings") == "DMX_INTENSITY")
                                                {
                                                    effect.startIntensity = 255;
                                                    effect.endIntensity = 255;
                                                }
                                                else
                                                {
                                                    effect.startIntensity = 100;
                                                    effect.endIntensity = 100;
                                                }
                                                effect.startColour = xlColor((si & 0xFF0000) >> 16,(si & 0xFF00) >> 8, si & 0xFF);
                                                effect.endColour = effect.startColour;
                                            }
                                            else
                                            {
                                                effect.startIntensity = si;
                                                effect.endIntensity = si;
                                                effect.startColour = xlWHITE;
                                                effect.endColour = xlWHITE;
                                            }
                                        }
                                        else
                                        {
                                            si = wxAtoi(ef->GetAttribute("startIntensity", "9999"));
                                            if (si != 9999)
                                            {
                                                if (si < 0)
                                                {
                                                    if (ef->GetAttribute("settings") == "DMX_INTENSITY")
                                                    {
                                                        effect.startIntensity = 255;
                                                        effect.endIntensity = 255;
                                                    }
                                                    else
                                                    {
                                                        effect.startIntensity = 100;
                                                        effect.endIntensity = 100;
                                                    }
                                                    effect.startColour = xlColor((si & 0xFF0000) >> 16, (si & 0xFF00) >> 8, si & 0xFF);
                                                    int ei = wxAtoi(ef->GetAttribute("endIntensity", "-1"));
                                                    effect.endColour = xlColor((ei & 0xFF0000) >> 16, (ei & 0xFF00) >> 8, ei & 0xFF);
                                                }
                                                else
                                                {
                                                    effect.startIntensity = si;
                                                    effect.endIntensity = wxAtoi(ef->GetAttribute("endIntensity", "100"));
                                                    effect.startColour = xlWHITE;
                                                    effect.endColour = xlWHITE;
                                                }
                                            }
                                        }
                                        effect.type = loreditType::CHANNELS;
                                        effect.effectType = ef->GetAttribute("settings");
                                        res.push_back(effect);
                                    }
                                    return res;
                                }
                            }
                        }
                        return res;
                    }
                }
            }
        }
    }

    return res;
}
