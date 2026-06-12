/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LORPixelEditor.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string_view>

#include <spdlog/fmt/fmt.h>

#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/RenderUtils.h"
#include "utils/string_utils.h"

#include "effects/BarsEffect.h"
#include "effects/ButterflyEffect.h"
#include "effects/CurtainEffect.h"
#include "effects/FireEffect.h"
#include "effects/GarlandsEffect.h"
#include "effects/MeteorsEffect.h"
#include "effects/PinwheelEffect.h"
#include "effects/SnowflakesEffect.h"
#include "effects/SpiralsEffect.h"

namespace {

// wxString::Capitalize — upper-case first char, lower-case the rest.
std::string Capitalize(const std::string& s)
{
    if (s.empty()) return s;
    std::string r = ::Lower(s);
    r[0] = (char)std::toupper((unsigned char)r[0]);
    return r;
}

double Atof(const std::string& s)
{
    return std::strtod(s.c_str(), nullptr);
}

int Atoi(const std::string& s)
{
    return (int)std::strtol(s.c_str(), nullptr, 10);
}

// Decode the %XX / + escaping LPE applies to text-effect strings. Mirrors the
// wxURI::Unescape used by the desktop text-effect branch.
std::string UriUnescape(const std::string& in)
{
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%' && i + 2 < in.size() && isxdigit((unsigned char)in[i + 1]) && isxdigit((unsigned char)in[i + 2])) {
            int hi = std::strtol(in.substr(i + 1, 2).c_str(), nullptr, 16);
            out += (char)hi;
            i += 2;
        } else {
            out += in[i];
        }
    }
    return out;
}

float Rescale(float original, float sourceMin, float sourceMax, float targetMin, float targetMax)
{
    if (original < sourceMin)
        original = sourceMin;
    if (original > sourceMax)
        original = sourceMax;

    return ((original - sourceMin) / (sourceMax - sourceMin)) * (targetMax - targetMin) + targetMin;
}

std::string RescaleWithRangeI(std::string r, std::string vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, std::string& vc, float targetRealMin, float targetRealMax)
{
    if (::Contains(r, "R")) {
        auto rr = ::Split(r, 'R');
        vc = "," + vcName + "=Active=TRUE|Id=ID_" + vcName.substr(2) + "|Type=Ramp|Min=" + fmt::format("{:.2f}", targetRealMin) +
             "|Max=" + fmt::format("{:.2f}", targetRealMax) +
             "|P1=" + fmt::format("{:.2f}", Rescale(Atof(rr[0]), sourceMin, sourceMax, targetMin, targetMax)) +
             "|P2=" + fmt::format("{:.2f}", Rescale(Atof(rr[1]), sourceMin, sourceMax, targetMin, targetMax)) +
             "|RV=TRUE|";
        return fmt::format("{}", (int)Rescale(Atof(rr[0]), sourceMin, sourceMax, targetMin, targetMax));
    } else {
        vc = "";
        return fmt::format("{}", (int)Rescale(Atof(r), sourceMin, sourceMax, targetMin, targetMax));
    }
}

std::string RescaleWithRangeF(std::string r, std::string vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, std::string& vc, float targetRealMin, float targetRealMax)
{
    if (::Contains(r, "R")) {
        auto rr = ::Split(r, 'R');
        vc = "," + vcName + "=Active=TRUE|Id=ID_" + vcName.substr(2) + "|Type=Ramp|Min=" + fmt::format("{:.2f}", targetRealMin) +
             "|Max=" + fmt::format("{:.2f}", targetRealMax) +
             "|P1=" + fmt::format("{:.2f}", Rescale(Atof(rr[0]), sourceMin, sourceMax, targetMin, targetMax)) +
             "|P2=" + fmt::format("{:.2f}", Rescale(Atof(rr[1]), sourceMin, sourceMax, targetMin, targetMax)) +
             "|RV=TRUE|";
    } else {
        vc = "";
    }
    return fmt::format("{:.1f}", Rescale(Atof(r), sourceMin, sourceMax, targetMin, targetMax));
}

std::string MapLPEEffectType(const std::string& effect)
{
    if (effect == "colorwash")
        return "Color Wash";
    if (effect == "picture")
        return "Pictures";
    if (effect == "lineshorizontal")
        return "";
    if (effect == "linesvertical")
        return "";
    if (effect == "countdown")
        return "Text";

    return Capitalize(effect);
}

std::string MapLPEBlend(const std::string& blend, bool left)
{
    if (!left)
        return "Normal";

    if (blend == "Mix_Average")
        return "Average";
    if (blend == "Mix_Overlay")
        return "Normal";
    if (blend == "Mix_Maximum")
        return "Max";
    if (blend == "Mix_Bottom_Top")
        return "Bottom-Top";
    if (blend == "Mix_Left-Right")
        return "Left-Right";
    if (blend == "Mix_Rt_Hides_Lt")
        return "1 is Mask";
    if (blend == "Mix_Rt_Reveals_Lt")
        return "2 reveals 1";

    return "Normal";
}

std::string ExtractLPEPallette(const std::vector<std::string>& ps)
{
    std::string palette;

    int cnum = 0;
    auto c = ::Split(ps[1], ';');
    for (int i = 0; i < (int)c.size(); i++) {
        std::string n = fmt::format("{}", cnum + 1);

        auto cc = ::Split(c[i], ',');
        if (cc.size() == 2) {
            std::string c1 = cc[0].substr(2); // drop transparency
            std::string active = cc[1];

            palette += ",C_BUTTON_Palette" + n + "=#" + c1;
            if (active == "1") {
                palette += ",C_CHECKBOX_Palette" + n + "=" + active;
            }
            cnum++;
        } else if (cc.size() == 3) {
            std::string c1 = cc[0].substr(2); // drop transparency
            std::string c2 = cc[1].substr(2); // drop transparency
            std::string active = cc[2];

            if (c1 == c2) {
                palette += ",C_BUTTON_Palette" + n + "=#" + c1;
                if (active == "1") {
                    palette += ",C_CHECKBOX_Palette" + n + "=" + active;
                }
            } else {
                palette += "C_BUTTON_Palette" + n + "=Active=TRUE|Id=ID_BUTTON_Palette" + n + "|Values=x=0.000^c=#" + c1 + ";x=1.000^c=#" + c2 + "|";
                if (active == "1") {
                    palette += ",C_CHECKBOX_Palette" + n + "=" + active;
                }
            }
            cnum++;
        } else {
            break;
        }
    }

    return palette;
}

pugi::xml_node FindLastLPEEffectNode(pugi::xml_node effect, int start_centisecond, int /*end_centisecond*/, int end_intensity, int intensityChange, const std::string& settings, int& fadeInCS, int& fadeOutCS)
{
    int originalIntensity = end_intensity - intensityChange;

    fadeInCS = 0;
    fadeOutCS = 0;

    pugi::xml_node res = effect;
    pugi::xml_node n = effect.next_sibling();
    while (n) {
        int newstartCentisecond = n.attribute("startCentisecond").as_int();
        int newstartIntensity = n.attribute("startIntensity").as_int(100);
        int newendIntensity = n.attribute("endIntensity").as_int(100);
        std::string newsettings = n.attribute("pixelEffect").as_string();
        if (settings != newsettings) {
            break;
        }

        int newintensityChange = newendIntensity - newstartIntensity;
        if (std::abs(intensityChange - newintensityChange) > 1) {
            if (intensityChange > 0 && newintensityChange == 0 && fadeInCS == 0) {
                fadeInCS = newstartCentisecond;
                intensityChange = newintensityChange;
            } else if (intensityChange == 0 && newintensityChange < 0 && fadeOutCS == 0) {
                fadeOutCS = newstartCentisecond;
                intensityChange = newintensityChange;
            } else {
                break;
            }
        }

        res = n;
        n = n.next_sibling();
    }

    int newendCentisecond = res.attribute("endCentisecond").as_int();
    int newendIntensity = res.attribute("endIntensity").as_int(100);
    if (fadeOutCS == 0 && fadeInCS == 0 && originalIntensity < newendIntensity && newendIntensity > 95 && originalIntensity < 5) {
        fadeInCS = newendCentisecond;
    } else if (fadeOutCS == 0 && fadeInCS == 0 && originalIntensity > newendIntensity && newendIntensity < 5 && originalIntensity > 95) {
        fadeOutCS = start_centisecond;
    }

    return res;
}

std::string LPEParseEffectSettings(const std::string& effectType, const std::vector<std::string>& arrSettings, std::string& palette, int durationMS)
{
    std::string settings;

    if (arrSettings.size() > 1) {
        auto parms = ::Split(arrSettings[2], ',');
        if (effectType == "butterfly") {
            std::string style = parms[0];
            std::string chunks = parms[1];
            std::string vcChunks;
            chunks = RescaleWithRangeI(chunks, "E_VALUECURVE_Butterfly_Chunks", 1, 10, 1, 10, vcChunks, ButterflyEffect::sChunksMin, ButterflyEffect::sChunksMax);
            std::string skip = parms[2];
            std::string vcSkip;
            skip = RescaleWithRangeI(skip, "E_VALUECURVE_Butterfly_Skip", 2, 10, 2, 10, vcSkip, ButterflyEffect::sSkipMin, ButterflyEffect::sSkipMax);
            std::string direction = parms[3];
            std::string hue = parms[4];
            std::string vcHue;
            hue = RescaleWithRangeI(hue, "C_VALUECURVE_Color_HueAdjust", 0, 359, -100, 100, vcHue, -100, 100);
            std::string speed = parms[5];
            std::string vcSpeed;
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Butterfly_Speed", 0, 50, 0, 50, vcSpeed, ButterflyEffect::sSpeedMin, ButterflyEffect::sSpeedMax);
            std::string colours = parms[6];

            if (style == "linear") {
                settings += ",E_SLIDER_Butterfly_Style=1";
            } else if (style == "radial") {
                settings += ",E_SLIDER_Butterfly_Style=2";
            } else if (style == "blocks") {
                settings += ",E_SLIDER_Butterfly_Style=3";
            } else if (style == "corner") {
                settings += ",E_SLIDER_Butterfly_Style=5";
            }
            settings += ",E_CHOICE_Butterfly_Colors=" + Capitalize(colours);
            settings += ",E_CHOICE_Butterfly_Direction=" + Capitalize(direction);
            settings += ",E_SLIDER_Butterfly_Chunks=" + chunks;
            settings += vcChunks;
            settings += ",E_SLIDER_Butterfly_Skip=" + skip;
            settings += vcSkip;
            settings += ",E_SLIDER_Butterfly_Speed=" + speed;
            settings += vcSpeed;

            if (hue != "0") {
                palette += ",C_SLIDER_Color_HueAdjust=" + hue;
                palette += vcHue;
            }
        } else if (effectType == "colorwash") {
            std::string horizontalFade = parms[0];
            std::string verticalFade = parms[1];

            if (horizontalFade != "full") {
                settings += ",E_CHECKBOX_ColorWash_HFade=1";
            }
            if (verticalFade != "full") {
                settings += ",E_CHECKBOX_ColorWash_VFade=1";
            }
        } else if (effectType == "spirals") {
            std::string repeat = parms[0];
            std::string vcRepeat;
            repeat = RescaleWithRangeI(repeat, "E_VALUECURVE_Spirals_Count", 1, 5, 1, 5, vcRepeat, SpiralsEffect::sCountMin, SpiralsEffect::sCountMax);
            std::string direction = parms[1];
            std::string rotation = parms[2];
            rotation = fmt::format("{:.2f}", Atof(rotation) / 60.0);
            std::string vcRotation;
            rotation = RescaleWithRangeF(rotation, "E_VALUECURVE_Spirals_Rotation", 0, 50, 0, 50, vcRotation, SpiralsEffect::sRotationMin, SpiralsEffect::sRotationMax);
            rotation = fmt::format("{}", (int)(Atof(rotation) * 10.0));
            std::string thickness = parms[3];
            std::string vcThickness;
            thickness = RescaleWithRangeI(thickness, "E_VALUECURVE_Spirals_Thickness", 0, 100, 0, 100, vcThickness, SpiralsEffect::sThicknessMin, SpiralsEffect::sThicknessMax);
            std::string blend = parms[5];
            std::string show3d = parms[6];
            std::string speed = parms[7];
            speed = fmt::format("{}", (int)(Atof(speed) / (20.0 / ((float)durationMS / 1000.0))));
            std::string vcSpeed;
            if (direction == "right_to_left") {
                speed = RescaleWithRangeF(speed, "E_VALUECURVE_Spirals_Movement", 0, 50, 0, -50, vcSpeed, SpiralsEffect::sMovementMin, SpiralsEffect::sMovementMax);
            } else {
                speed = RescaleWithRangeF(speed, "E_VALUECURVE_Spirals_Movement", 0, 50, 0, 50, vcSpeed, SpiralsEffect::sMovementMin, SpiralsEffect::sMovementMax);
            }

            settings += ",E_SLIDER_Spirals_Count=" + repeat;
            settings += vcRepeat;
            settings += ",E_TEXTCTRL_Spirals_Movement=" + speed;
            settings += vcSpeed;
            settings += ",E_SLIDER_Spirals_Rotation=" + rotation;
            settings += vcRotation;
            settings += ",E_SLIDER_Spirals_Thickness=" + thickness;
            settings += vcThickness;

            if (blend == "True") {
                settings += ",E_CHECKBOX_Spirals_Blend=1,";
            }

            if (show3d == "trail_left" || show3d == "trail_right") {
                settings += ",E_CHECKBOX_Spirals_3D=1";
            }
        } else if (effectType == "bars") {
            std::string direction = parms[0];
            std::string repeat = parms[1];
            std::string vcRepeat;
            repeat = RescaleWithRangeI(repeat, "E_VALUECURVE_Bars_BarCount", 1, 5, 1, 5, vcRepeat, BarsEffect::sBarCountMin, BarsEffect::sBarCountMax);
            std::string highlight = parms[2];
            std::string show3d = parms[3];
            std::string speed = parms[4];
            speed = fmt::format("{}", (int)(Atof(speed) / (20.0 / ((float)durationMS / 1000.0))));
            std::string vcSpeed;
            speed = RescaleWithRangeF(speed, "E_VALUECURVE_Bars_Cycles", 0, 50, 0, 30, vcSpeed, BarsEffect::sCyclesMin, BarsEffect::sCyclesMax);
            std::string centre = parms[5];
            std::string vcCentre;
            centre = RescaleWithRangeI(centre, "E_VALUECURVE_Bars_Center", -50, 50, -100, 100, vcCentre, BarsEffect::sCenterMin, BarsEffect::sCenterMax);

            settings += ",E_SLIDER_Bars_BarCount=" + repeat;
            settings += vcRepeat;

            if (direction == "V_expand")
                direction = "expand";
            if (direction == "V_compress")
                direction = "compress";
            if (direction == "H_expand")
                direction = "H-expand";
            if (direction == "H_compress")
                direction = "H-compress";
            if (direction == "left")
                direction = "Left";
            if (direction == "right")
                direction = "Right";
            if (direction == "block_up")
                direction = "Alternate Up";
            if (direction == "block_down")
                direction = "Alternate Down";
            if (direction == "block_left")
                direction = "Alternate Right";
            if (direction == "block_right")
                direction = "Alternate Right";
            settings += ",E_CHOICE_Bars_Direction=" + direction;

            if (show3d == "True") {
                settings += ",E_CHECKBOX_Bars_3D=1";
            }

            if (highlight == "True") {
                settings += "E_CHECKBOX_Bars_Highlight=1";
            }

            settings += ",E_TEXTCTRL_Bars_Cycles=" + speed;
            settings += vcSpeed;
            settings += ",E_TEXTCTRL_Bars_Center=" + centre;
            settings += vcCentre;
        } else if (effectType == "countdown") {
            std::string seconds = parms[0];
            std::string font = parms[1];
            std::string fontSize = parms[2];
            std::string vcCrap;
            fontSize = RescaleWithRangeI(fontSize, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
            std::string position = parms[3];
            position = RescaleWithRangeI(position, "IGNORE", -50, 50, -200, 200, vcCrap, -1, -1);

            settings += ",E_TEXTCTRL_Text=" + seconds;
            settings += ",E_CHOICE_Text_Count=seconds";
            settings += ",E_CHOICE_Text_Font=Use OS Fonts";
            settings += ",E_FONTPICKER_Text_Font='" + font + "' " + fontSize;
            settings += ",E_SLIDER_Text_XStart=" + position;
        } else if (effectType == "lineshorizontal") {
            // No xLights equivalent
        } else if (effectType == "linesvertical") {
            // No xLights equivalent
        } else if (effectType == "curtain") {
            std::string edge = parms[0];
            std::string movement = parms[1];
            std::string swag = parms[2];
            std::string vcSwag;
            swag = RescaleWithRangeF(swag, "E_VALUECURVE_Curtain_Swag", 0, 10, 0, 10, vcSwag, CurtainEffect::sSwagMin, CurtainEffect::sSwagMax);
            std::string repeat = parms[3];
            std::string speed = parms[4];
            std::string vcSpeed;
            speed = RescaleWithRangeF(speed, "E_VALUECURVE_Curtain_Speed", 0, 50, 0, 10, vcSpeed, 0, 10);

            settings += ",E_CHOICE_Curtain_Edge=" + edge;
            ::Replace(movement, "_", " ");
            settings += ",E_CHOICE_Curtain_Effect=" + movement;
            settings += ",E_SLIDER_Curtain_Swag=" + swag;
            settings += vcSwag;

            if (repeat == "once_at_speed" || repeat == "once_fit_to_duration") {
                settings += ",E_CHECKBOX_Curtain_Repeat=0";
            } else if (repeat == "repeat_at_speed_rotate_colors" || repeat == "repeat_at_speed_blend_colors") {
                settings += ",E_CHECKBOX_Curtain_Repeat=1";
            }
            settings += ",E_TEXTCTRL_Curtain_Speed=" + speed;
            settings += vcSpeed;
        } else if (effectType == "fire") {
            std::string height = parms[0];
            std::string vcHeight;
            height = RescaleWithRangeI(height, "E_VALUECURVE_Fire_Height", 10, 100, 0, 100, vcHeight, FireEffect::sHeightMin, FireEffect::sHeightMax);
            std::string hueShift = parms[1];
            std::string vcHueShift;
            hueShift = RescaleWithRangeI(hueShift, "E_VALUECURVE_Fire_HueShift", 0, 359, 0, 100, vcHueShift, FireEffect::sHueShiftMin, FireEffect::sHueShiftMax);

            settings += ",E_SLIDER_Fire_Height=" + height;
            settings += vcHeight;
            settings += ",E_SLIDER_Fire_HueShift=" + hueShift;
            settings += vcHueShift;
        } else if (effectType == "fireworks") {
            std::string explosionRate = parms[0];
            std::string vcCrap;
            explosionRate = RescaleWithRangeI(explosionRate, "IGNORE", 1, 95, 1, 50, vcCrap, -1, -1);
            std::string particles = parms[1];
            particles = RescaleWithRangeI(particles, "IGNORE", 1, 100, 1, 100, vcCrap, -1, -1);
            std::string velocity = parms[2];
            velocity = RescaleWithRangeI(velocity, "IGNORE", 1, 10, 1, 10, vcCrap, -1, -1);
            std::string fade = parms[3];
            fade = RescaleWithRangeI(fade, "IGNORE", 1, 100, 1, 100, vcCrap, -1, -1);
            settings += ",E_SLIDER_Fireworks_Explosions=" + explosionRate;
            settings += ",E_SLIDER_Fireworks_Count=" + particles;
            settings += ",E_SLIDER_Fireworks_Fade=" + fade;
            settings += ",E_SLIDER_Fireworks_Velocity=" + velocity;
        } else if (effectType == "garland") {
            std::string type = parms[0];
            std::string vcCrap;
            type = RescaleWithRangeI(type, "IGNORE", 0, 4, 0, 4, vcCrap, -1, -1);
            std::string spacing = parms[1];
            std::string vcSpacing;
            spacing = RescaleWithRangeI(spacing, "E_VALUECURVE_Garlands_Spacing", 0, 100, 1, 100, vcSpacing, GarlandsEffect::sSpacingMin, GarlandsEffect::sSpacingMax);
            std::string repeat = parms[2];
            std::string speed = parms[3];
            std::string vcSpeed;
            speed = RescaleWithRangeF(speed, "E_VALUECURVE_Garlands_Cycles", 0, 50, 0, 20, vcSpeed, 0, 20);
            std::string fill = parms[4];

            settings += ",E_SLIDER_Garlands_Type=" + type;

            if (fill == "bottom_to_top") {
                settings += ",E_CHOICE_Garlands_Direction=Up";
            } else if (fill == "top_to_bottom") {
                settings += ",E_CHOICE_Garlands_Direction=Down";
            } else if (fill == "left_to_right") {
                settings += ",E_CHOICE_Garlands_Direction=Right";
            } else if (fill == "right_to_left") {
                settings += ",E_CHOICE_Garlands_Direction=Left";
            }

            settings += ",E_SLIDER_Garlands_Spacing=" + spacing;
            settings += vcSpacing;

            if (repeat == "repeat_at_speed") {
                settings += ",E_TEXTCTRL_Garlands_Cycles=" + speed;
                settings += vcSpeed;
            } else if (repeat == "once_at_speed" || repeat == "once_fit_to_duration") {
                settings += ",E_TEXTCTRL_Garlands_Cycles=1.0";
            }
        } else if (effectType == "meteors") {
            std::string colourScheme = parms[0];
            std::string count = parms[1];
            std::string vcCount;
            count = RescaleWithRangeI(count, "E_VALUECURVE_Meteors_Count", 1, 100, 1, 100, vcCount, MeteorsEffect::sCountMin, MeteorsEffect::sCountMax);
            std::string length = parms[2];
            std::string vcLength;
            length = RescaleWithRangeI(length, "E_VALUECURVE_Meteors_Length", 1, 100, 1, 100, vcLength, MeteorsEffect::sLengthMin, MeteorsEffect::sLengthMax);
            std::string effect = parms[3];
            std::string swirl = parms[4];
            std::string vcSwirl;
            swirl = RescaleWithRangeI(swirl, "E_VALUECURVE_Meteors_Swirl_Intensity", 0, 20, 0, 20, vcSwirl, MeteorsEffect::sSwirlMin, MeteorsEffect::sSwirlMax);
            std::string speed = parms[5];
            std::string vcSpeed;
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Meteors_Speed", 1, 50, 1, 50, vcSpeed, MeteorsEffect::sSpeedMin, MeteorsEffect::sSpeedMax);

            settings += ",E_CHOICE_Meteors_Type=" + ::Lower(colourScheme);
            settings += ",E_SLIDER_Meteors_Count=" + count;
            settings += vcCount;
            settings += ",E_SLIDER_Meteors_Length=" + length;
            settings += vcLength;
            settings += ",E_CHOICE_Meteors_Effect=" + Capitalize(effect);
            settings += ",E_SLIDER_Meteors_Swirl_Intensity=" + swirl;
            settings += vcSwirl;
            settings += ",E_SLIDER_Meteors_Speed=" + speed;
            settings += vcSpeed;
        } else if (effectType == "movie") {
            std::string file = parms[0];
            std::string scale = parms[1];
            std::string fullLength = parms[2];

            settings += ",E_FILEPICKERCTRL_Video_Filename=" + file;

            if (scale == "True") {
                settings += ",E_CHECKBOX_Video_AspectRatio=0";
            } else {
                settings += ",E_CHECKBOX_Video_AspectRatio=1";
            }
            if (fullLength == "True") {
                settings += ",E_CHOICE_Video_DurationTreatment=Slow/Accelerate";
            } else {
                settings += ",E_CHOICE_Video_DurationTreatment=Normal";
            }
        } else if (effectType == "picture") {
            std::string file = parms[0];
            std::string scale = parms[1];
            std::string movement = parms[2];
            std::string x = parms[3];
            std::string vcCrap;
            x = RescaleWithRangeI(x, "IGNORE", -50, 50, -100, 100, vcCrap, -1, -1);
            std::string speed = parms[6];
            speed = RescaleWithRangeF(speed, "IGNORE", 0, 50, 0, 20, vcCrap, -1, -1);

            settings += ",E_TEXTCTRL_Pictures_Filename=" + file;
            if (scale == "True") {
                settings += ",E_CHOICE_Scaling=Scale To Fit";
            } else {
                settings += ",E_CHOICE_Scaling=No Scaling";
            }

            ::Replace(movement, "_", "-");
            if (movement == "peekaboo-bottom") {
                movement = "peekaboo";
            } else if (movement == "peekaboo-top") {
                movement = "peekaboo 180";
            } else if (movement == "peekaboo-left") {
                movement = "peekaboo 90";
            } else if (movement == "peekaboo-right") {
                movement = "peekaboo 270";
            }
            settings += ",E_CHOICE_Pictures_Direction=" + movement;
            settings += ",E_SLIDER_PicturesXC=" + x;
            settings += ",E_TEXTCTRL_Pictures_Speed=" + speed;
        } else if (effectType == "pinwheel") {
            std::string arms = parms[0];
            std::string vcCrap;
            arms = RescaleWithRangeI(arms, "IGNORE", 1, 10, 1, 10, vcCrap, -1, -1);
            std::string width = parms[1];
            std::string vcWidth;
            width = RescaleWithRangeI(width, "E_VALUECURVE_Pinwheel_Thickness", 1, 10, 0, 100, vcWidth, PinwheelEffect::sThicknessMin, PinwheelEffect::sThicknessMax);
            std::string bend = parms[2];
            std::string vcBend;
            bend = RescaleWithRangeI(bend, "E_VALUECURVE_Pinwheel_Twist", -10, 10, -360, 360, vcBend, PinwheelEffect::sTwistMin, PinwheelEffect::sTwistMax);
            std::string CCW = parms[4];
            std::string speed = parms[5];
            std::string vcSpeed;
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Pinwheel_Speed", 0, 50, 0, 50, vcSpeed, PinwheelEffect::sSpeedMin, PinwheelEffect::sSpeedMax);
            std::string length = parms[6];
            std::string vcLength;
            length = RescaleWithRangeI(length, "E_VALUECURVE_Pinwheel_ArmSize", 1, 100, 0, 400, vcLength, PinwheelEffect::sArmSizeMin, PinwheelEffect::sArmSizeMax);
            std::string x = parms[7];
            std::string vcX;
            x = RescaleWithRangeI(x, "E_VALUECURVE_PinwheelXC", -50, 50, -100, 100, vcX, PinwheelEffect::sXCMin, PinwheelEffect::sXCMax);
            std::string y = parms[8];
            std::string vcY;
            y = RescaleWithRangeI(y, "E_VALUECURVE_PinwheelYC", -50, 50, -100, 100, vcY, PinwheelEffect::sYCMin, PinwheelEffect::sYCMax);

            settings += ",E_SLIDER_Pinwheel_Arms=" + arms;
            settings += ",E_SLIDER_Pinwheel_Thickness=" + width;
            settings += vcWidth;
            settings += ",E_SLIDER_Pinwheel_Twist=" + bend;
            settings += vcBend;
            if (CCW == "True") {
                settings += ",E_CHECKBOX_Pinwheel_Rotation=1";
            } else {
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
        } else if (effectType == "snowflakes") {
            std::string count = parms[0];
            std::string vcCount;
            count = RescaleWithRangeI(count, "E_VALUECURVE_Snowflakes_Count", 1, 20, 1, 20, vcCount, SnowflakesEffect::sCountMin, SnowflakesEffect::sCountMax);
            std::string type = parms[1];
            std::string vcCrap;
            type = RescaleWithRangeI(type, "IGNORE", 0, 5, 0, 5, vcCrap, -1, -1);
            std::string speed = parms[3];
            std::string vcSpeed;
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Snowflakes_Speed", 0, 50, 0, 50, vcSpeed, SnowflakesEffect::sSpeedMin, SnowflakesEffect::sSpeedMax);
            std::string accumulation = parms[4];
            accumulation = RescaleWithRangeI(accumulation, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);

            settings += ",E_SLIDER_Snowflakes_Count=" + count;
            settings += ",E_SLIDER_Snowflakes_Type=" + type;
            settings += ",E_SLIDER_Snowflakes_Speed=" + speed;
            if (accumulation == "0") {
                settings += ",E_CHOICE_Falling=Falling";
            } else {
                settings += ",E_CHOICE_Falling=Falling & Accumulating";
            }
            settings += vcCount;
            settings += vcSpeed;
        } else if (effectType == "text") {
            std::string text = UriUnescape(parms[0]);
            ::Replace(text, "&gt;", ">");
            ::Replace(text, "&lt;", "<");
            ::Replace(text, "&nbsp;", " ");
            ::Replace(text, "&amp;", "&");
            std::string fontSize = parms[1];
            std::string vcCrap;
            fontSize = RescaleWithRangeI(fontSize, "IGNORE", 0, 149, 0, 149, vcCrap, -1, -1);
            std::string movement = parms[2];
            std::string position = parms[3];
            position = RescaleWithRangeI(position, "IGNORE", -50, 49, -200, 200, vcCrap, -1, -1);
            std::string speed = parms[6];
            speed = RescaleWithRangeI(speed, "IGNORE", 0, 50, 0, 50, vcCrap, -1, -1);

            settings += ",E_TEXTCTRL_Text=" + text;
            settings += ",E_CHOICE_Text_Font=Use OS Fonts";
            settings += ",E_FONTPICKER_Text_Font='segoe ui' " + fontSize;

            if (movement == "peekaboo_bottom") {
                settings += ",E_CHECKBOX_TextToCenter=1";
                movement = "up";
            }
            if (movement == "peekaboo_top") {
                settings += ",E_CHECKBOX_TextToCenter=1";
                movement = "down";
            }
            if (movement == "peekaboo_left") {
                settings += ",E_CHECKBOX_TextToCenter=1";
                movement = "left";
            }
            if (movement == "peekaboo_right") {
                settings += ",E_CHECKBOX_TextToCenter=1";
                movement = "right";
            }
            settings += ",E_CHOICE_Text_Dir=" + movement;
            settings += ",E_SLIDER_Text_XStart=" + position;
            settings += ",E_TEXTCTRL_Text_Speed=" + speed;
        } else if (effectType == "twinkle") {
            std::string rate = parms[0];
            std::string vcCrap;
            rate = RescaleWithRangeI(rate, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
            std::string density = parms[1];
            density = RescaleWithRangeI(density, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
            std::string mode = parms[2];
            std::string layout = parms[3];

            settings += ",E_SLIDER_Twinkle_Count=" + density;
            settings += ",E_SLIDER_Twinkle_Steps=" + rate;
            if (layout == "interval") {
                settings += ",E_CHECKBOX_Twinkle_ReRandom=0";
            } else if (layout == "random") {
                settings += ",E_CHECKBOX_Twinkle_ReRandom=1";
            }

            if (mode == "twinkle") {
                settings += ",E_CHECKBOX_Twinkle_Strobe=0";
            } else {
                settings += ",E_CHECKBOX_Twinkle_Strobe=1";
            }
        }
    }

    return settings;
}

} // namespace

LORPixelEditor::LORPixelEditor(pugi::xml_document& input_xml, int frequency) :
    _input_xml(input_xml), _frequency(frequency)
{
    Index();
}

void LORPixelEditor::Index()
{
    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        std::string ename = e.name();
        if (ename == "SequenceProps" || ename == "ArchivedProps") {
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string pname = prop.name();
                if (pname == "SeqProp" || pname == "ArchiveProp") {
                    std::string name = prop.attribute("name").as_string();
                    if (name == "") {
                        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                            if (std::string_view(ap.name()) == "PropClass") {
                                name = ap.attribute("Name").as_string();
                            }
                        }
                    }
                    if (name != "") {
                        _channelNames.push_back(name);
                    }
                }
            }
        }
    }
    std::sort(_channelNames.begin(), _channelNames.end(), stdlistNumberAwareStringCompare);
}

bool LORPixelEditor::HasEffects(const std::string& model, int layer, bool left) const
{
    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        std::string ename = e.name();
        if (ename == "SequenceProps" || ename == "ArchivedProps") {
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string pname = prop.name();
                if (pname == "SeqProp" || pname == "ArchiveProp") {
                    std::string name = prop.attribute("name").as_string();
                    if (name == "") {
                        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                            if (std::string_view(ap.name()) == "PropClass") {
                                name = ap.attribute("Name").as_string();
                            }
                        }
                    }
                    if (name == model) {
                        for (pugi::xml_node track = prop.first_child(); track; track = track.next_sibling()) {
                            int id = track.attribute("id").as_int();
                            if (id == layer) {
                                for (pugi::xml_node eff = track.first_child(); eff; eff = eff.next_sibling()) {
                                    if (std::string_view(eff.name()) == "effect" && std::string_view(eff.attribute("type").as_string()) == "pixelEffect") {
                                        std::string settings = eff.attribute("pixelEffect").as_string();
                                        auto as = ::Split(settings, '|');
                                        if (as.size() == 7) {
                                            std::string s = left ? as[5] : as[6];
                                            auto ss = ::Split(s, ':');
                                            if (ss[0] != "none")
                                                return true;
                                        } else if (as.size() == 5) {
                                            std::string s = left ? as[3] : as[4];
                                            auto ss = ::Split(s, ':');
                                            if (ss[0] != "none")
                                                return true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

void LORPixelEditor::MapLPE(EffectLayer* layer, int lpeLayer, const std::string& model, bool left, bool eraseExisting) const
{
    if (eraseExisting)
        layer->DeleteAllEffects();

    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        std::string ename = e.name();
        if (ename == "SequenceProps" || ename == "ArchivedProps") {
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string pname = prop.name();
                if (pname == "SeqProp" || pname == "ArchiveProp") {
                    std::string name = prop.attribute("name").as_string();
                    if (name == "") {
                        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                            if (std::string_view(ap.name()) == "PropClass") {
                                name = ap.attribute("Name").as_string();
                            }
                        }
                    }
                    if (name == model) {
                        for (pugi::xml_node track = prop.first_child(); track; track = track.next_sibling()) {
                            int id = track.attribute("id").as_int();
                            if (id == lpeLayer) {
                                for (pugi::xml_node effect = track.first_child(); effect; effect = effect.next_sibling()) {
                                    std::string type = effect.attribute("type").as_string();
                                    if (std::string_view(effect.name()) != "effect" || type != "pixelEffect") {
                                        continue;
                                    }
                                    int startCentisecond = effect.attribute("startCentisecond").as_int();
                                    int endCentisecond = effect.attribute("endCentisecond").as_int();
                                    int startIntensity = effect.attribute("startIntensity").as_int(100);
                                    int endIntensity = effect.attribute("endIntensity").as_int(100);
                                    std::string settings = effect.attribute("pixelEffect").as_string();
                                    auto settingsArray = ::Split(settings, '|');
                                    std::string sideSettings;
                                    if (left) {
                                        sideSettings = (settingsArray.size() == 7) ? settingsArray[5] : settingsArray[3];
                                    } else {
                                        sideSettings = (settingsArray.size() == 7) ? settingsArray[6] : settingsArray[4];
                                    }
                                    auto effSettings = ::Split(sideSettings, ':');
                                    std::string effectType = effSettings[0];
                                    if (effectType == "none") {
                                        continue;
                                    }
                                    std::string ourEffectType = MapLPEEffectType(effectType);
                                    if (ourEffectType == "") {
                                        continue;
                                    }

                                    int fadeInCS, fadeOutCS;
                                    pugi::xml_node lastnode = FindLastLPEEffectNode(effect, startCentisecond, endCentisecond, endIntensity, endIntensity - startIntensity, settings, fadeInCS, fadeOutCS);
                                    if (lastnode != effect) {
                                        endCentisecond = lastnode.attribute("endCentisecond").as_int();
                                        endIntensity = lastnode.attribute("endIntensity").as_int(100);
                                        effect = lastnode;
                                    }

                                    if (!layer->HasEffectsInTimeRange(RoundToMultipleOfPeriod(startCentisecond * 10, _frequency), RoundToMultipleOfPeriod(endCentisecond * 10, _frequency))) {
                                        std::string blend = MapLPEBlend(settingsArray[0], left);
                                        int blendPos = Atoi(settingsArray[1]);
                                        int sparkle = Atoi(settingsArray[2]);

                                        std::string newpalette = ExtractLPEPallette(effSettings);
                                        std::string newsettings = "T_CHOICE_LayerMethod=" + blend;

                                        if (sparkle > 0) {
                                            newpalette += ",C_SLIDER_SparkleFrequency=" + fmt::format("{}", sparkle);
                                        }
                                        if (left && blendPos > 0) {
                                            newsettings += ",T_SLIDER_EffectLayerMix=" + fmt::format("{}", blendPos);
                                        }

                                        if (startIntensity == 100 && endIntensity == 100 && fadeInCS == 0 && fadeOutCS == 0) {
                                            // nothing to do
                                        } else if (startIntensity == endIntensity && fadeInCS == 0 && fadeOutCS == 0) {
                                            newpalette += ",C_SLIDER_Brightness=" + fmt::format("{}", startIntensity);
                                        } else {
                                            if (fadeInCS > 0) {
                                                newsettings += ",T_TEXTCTRL_Fadein=" + fmt::format("{:.2f}", (float)(fadeInCS - startCentisecond) / 100.0);
                                            }
                                            if (fadeOutCS > 0) {
                                                newsettings += ",T_TEXTCTRL_Fadeout=" + fmt::format("{:.2f}", (float)(endCentisecond - fadeOutCS) / 100.0);
                                            }
                                            if (fadeInCS == 0 && fadeOutCS == 0) {
                                                newpalette += ",C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=" + fmt::format("{}", startIntensity) + "|P2=" + fmt::format("{}", endIntensity) + "|RV=TRUE|";
                                            }
                                        }

                                        newsettings += LPEParseEffectSettings(effectType, effSettings, newpalette, (endCentisecond - startCentisecond) * 10);

                                        layer->AddEffect(0, ourEffectType, newsettings, newpalette, RoundToMultipleOfPeriod(startCentisecond * 10, _frequency), RoundToMultipleOfPeriod(endCentisecond * 10, _frequency), false, false);
                                    }
                                }
                            }
                        }
                        return;
                    }
                }
            }
        }
    }
}

void LORPixelEditor::MapPropNodeEffects(EffectLayer* layer, const std::string& mapping, int lpeLayer, bool eraseExisting) const
{
    if (layer == nullptr) {
        return;
    }
    MapLPE(layer, lpeLayer, mapping, true, eraseExisting);
}

void LORPixelEditor::MapPropEffects(Element* model, const std::string& mapping, bool eraseExisting, int startLayer) const
{
    if (model == nullptr) {
        return;
    }
    int layer = startLayer;
    if (HasEffects(mapping, 0, true)) {
        if ((int)model->GetEffectLayerCount() < layer + 1)
            model->AddEffectLayer();
        MapLPE(model->GetEffectLayer(layer), 0, mapping, true, eraseExisting);
    }
    if (HasEffects(mapping, 0, false)) {
        layer++;
        if ((int)model->GetEffectLayerCount() < layer + 1) {
            model->AddEffectLayer();
        }
        MapLPE(model->GetEffectLayer(layer), 0, mapping, false, eraseExisting);
    }
    if (HasEffects(mapping, 1, true)) {
        layer++;
        if ((int)model->GetEffectLayerCount() < layer + 1) {
            model->AddEffectLayer();
        }
        MapLPE(model->GetEffectLayer(layer), 1, mapping, true, eraseExisting);
    }
    if (HasEffects(mapping, 1, false)) {
        layer++;
        if ((int)model->GetEffectLayerCount() < layer + 1) {
            model->AddEffectLayer();
        }
        MapLPE(model->GetEffectLayer(layer), 1, mapping, false, eraseExisting);
    }
}
