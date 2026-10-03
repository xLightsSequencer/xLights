/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LOREdit.h"

#include <algorithm>
#include <cmath>
#include <regex>
#include <set>
#include <cstdlib>
#include <cctype>

#include <spdlog/fmt/fmt.h>

#include "render/RenderUtils.h"
#include "utils/string_utils.h"
#include "models/Model.h"

#include "effects/SpiralsEffect.h"
#include "effects/ButterflyEffect.h"
#include "effects/BarsEffect.h"
#include "effects/CurtainEffect.h"
#include "effects/FireEffect.h"
#include "effects/GarlandsEffect.h"
#include "effects/MarqueeEffect.h"
#include "effects/MeteorsEffect.h"
#include "effects/PinwheelEffect.h"
#include "effects/SnowflakesEffect.h"
#include "effects/RippleEffect.h"

#include <spdlog/spdlog.h>

// Current working assumptions
//
// A prop can be sequenced using channels or tracks ... not both
// There can be multiple tracks
// Each effect on a track can have a left/right side
// Channels have rows and columns but i dont know how they work

namespace {
    int loreAtoi(const std::string& s) {
        return (int)std::strtol(s.c_str(), nullptr, 10);
    }

    double loreAtof(const std::string& s) {
        return std::strtod(s.c_str(), nullptr);
    }

    // Mirrors wxString::IsNumber — true for an optionally-signed integer
    // or decimal, with no other characters.
    bool isNumber(const std::string& s) {
        if (s.empty()) return false;
        size_t i = 0;
        if (s[i] == '+' || s[i] == '-') ++i;
        bool digits = false;
        bool dot = false;
        for (; i < s.size(); ++i) {
            char c = s[i];
            if (c == '.') {
                if (dot) return false;
                dot = true;
            } else if (c >= '0' && c <= '9') {
                digits = true;
            } else {
                return false;
            }
        }
        return digits;
    }

    // LOR text is HTML (<P>, <SPAN>, <FONT color=...>). Returns the plain text
    // with paragraph/line breaks as newlines, and the first font colour.
    std::string StripLORHtml(const std::string& html, std::string& colour) {
        colour.clear();
        static const std::regex colourRe(R"((?:^|[^-a-zA-Z])color\s*[=:]\s*["']?(#[0-9a-fA-F]{6}|[a-zA-Z]+))", std::regex::icase);
        std::smatch m;
        if (std::regex_search(html, m, colourRe)) {
            colour = m[1].str();
        }
        std::string out;
        out.reserve(html.size());
        for (size_t i = 0; i < html.size(); ++i) {
            if (html[i] == '<') {
                size_t e = html.find('>', i);
                if (e == std::string::npos) {
                    break;
                }
                std::string tag = Lower(html.substr(i + 1, e - i - 1));
                if (StartsWith(tag, "/p") || StartsWith(tag, "br")) {
                    out += '\n';
                }
                i = e;
            } else if (html[i] != '\r') {
                out += html[i];
            }
        }
        Replace(out, "&gt;", ">");
        Replace(out, "&lt;", "<");
        Replace(out, "&nbsp;", " ");
        Replace(out, "&amp;", "&");
        // drop blank lines LOR leaves around paragraphs
        std::string res;
        for (auto& line : Split(out, '\n')) {
            std::string t = Trim(line);
            if (t.empty()) {
                continue;
            }
            if (!res.empty()) {
                res += '\n';
            }
            res += t;
        }
        return res;
    }

    std::string ColourNameToHex(const std::string& c) {
        if (c.empty() || c[0] == '#') {
            return c;
        }
        static const std::map<std::string, std::string> names = {
            { "white", "#FFFFFF" }, { "black", "#000000" }, { "red", "#FF0000" }, { "lime", "#00FF00" },
            { "green", "#008000" }, { "blue", "#0000FF" }, { "yellow", "#FFFF00" }, { "cyan", "#00FFFF" },
            { "aqua", "#00FFFF" }, { "magenta", "#FF00FF" }, { "fuchsia", "#FF00FF" }, { "orange", "#FFA500" },
            { "purple", "#800080" }, { "silver", "#C0C0C0" }, { "gray", "#808080" }, { "grey", "#808080" },
            { "gold", "#FFD700" }, { "pink", "#FFC0CB" }
        };
        auto it = names.find(Lower(c));
        return it == names.end() ? "#FFFFFF" : it->second;
    }

    // LOR numeric parameters may be plain ("10"), a ramp ("R0R100R1.00R2.00R0.00",
    // start then end) or an oscillation ("O5O0O1.00O2.30O0.00")
    void ParamRange(const std::string& v, double& start, double& end) {
        if (!v.empty() && (v[0] == 'R' || v[0] == 'O' || v[0] == 'T')) {
            auto parts = Split(v.substr(1), v[0]);
            start = parts.size() > 0 ? loreAtof(parts[0]) : 0.0;
            end = parts.size() > 1 ? loreAtof(parts[1]) : start;
            if (v[0] == 'O') {
                // oscillates between the two; settle on the middle
                start = end = (start + end) / 2.0;
            }
            return;
        }
        start = end = loreAtof(v);
    }

    // The part of a sketch's left/top position baked into its SVG: the ramp
    // end nearer the centre, so the drawing is clipped least. Any remaining
    // movement is done by the Pictures vector offset.
    void SketchBakedOffset(const std::vector<std::string>& parms, double& left, double& top) {
        double l0, l1, t0, t1;
        ParamRange(parms.size() > 3 ? parms[3] : std::string(), l0, l1);
        ParamRange(parms.size() > 4 ? parms[4] : std::string(), t0, t1);
        left = std::abs(l1) < std::abs(l0) ? l1 : l0;
        top = std::abs(t1) < std::abs(t0) ? t1 : t0;
    }

    // LOR sketch paths are SVG-like (M, L, C, Z) except "A", which LOR uses for
    // conic segments: control x y, end x y, weight (0.7071 = a quarter
    // circle). Rewrite those as cubics so any SVG renderer draws them.
    std::string ConvertLORSketchPath(const std::string& d) {
        auto tokens = Split(d, ' ');
        std::string out;
        char cmd = 0;
        double cx = 0, cy = 0, sx = 0, sy = 0;
        std::vector<double> nums;
        auto emit = [&](const std::string& t) {
            if (!out.empty()) out += ' ';
            out += t;
        };
        auto num = [](double v) { return fmt::format("{:.5g}", v); };
        auto flush = [&]() {
            size_t i = 0;
            switch (cmd) {
            case 'M':
            case 'L':
                for (; i + 1 < nums.size(); i += 2) {
                    emit(std::string(1, (cmd == 'M' && i == 0) ? 'M' : 'L') + " " + num(nums[i]) + " " + num(nums[i + 1]));
                    cx = nums[i];
                    cy = nums[i + 1];
                    if (cmd == 'M' && i == 0) {
                        sx = cx;
                        sy = cy;
                    }
                }
                break;
            case 'C':
                for (; i + 5 < nums.size(); i += 6) {
                    emit("C " + num(nums[i]) + " " + num(nums[i + 1]) + " " + num(nums[i + 2]) + " " + num(nums[i + 3]) + " " + num(nums[i + 4]) + " " + num(nums[i + 5]));
                    cx = nums[i + 4];
                    cy = nums[i + 5];
                }
                break;
            case 'A':
                for (; i + 4 < nums.size(); i += 5) {
                    double px = nums[i], py = nums[i + 1], ex = nums[i + 2], ey = nums[i + 3], w = nums[i + 4];
                    double k = 4.0 * w / (3.0 * (1.0 + w));
                    emit("C " + num(cx + k * (px - cx)) + " " + num(cy + k * (py - cy)) + " " +
                         num(ex + k * (px - ex)) + " " + num(ey + k * (py - ey)) + " " + num(ex) + " " + num(ey));
                    cx = ex;
                    cy = ey;
                }
                break;
            case 'Z':
                emit("Z");
                cx = sx;
                cy = sy;
                break;
            default:
                break;
            }
            nums.clear();
        };
        for (const auto& t : tokens) {
            if (t.empty()) {
                continue;
            }
            char c = t[0];
            if (t.size() == 1 && std::isalpha((unsigned char)c)) {
                flush();
                cmd = (char)std::toupper((unsigned char)c);
                if (cmd == 'Z') {
                    flush();
                    cmd = 0;
                }
            } else {
                nums.push_back(loreAtof(t));
            }
        }
        flush();
        return out;
    }

    // LOR measures sketch left/top in steps of 3% of the prop (fitted
    // against LOR renders)
    constexpr double kSketchOffsetStep = 0.03;

    // LOR bar direction -> xLights Bars direction settings; diagonals use the
    // Custom direction's angle (0 right, 90 up)
    std::string BarsDirectionSettings(std::string direction) {
        static const std::map<std::string, int> diagonals = {
            { "down_right", -45 }, { "down_left", -135 }, { "up_right", 45 }, { "up_left", 135 }
        };
        auto it = diagonals.find(direction);
        if (it != diagonals.end()) {
            return fmt::format(",E_CHOICE_Bars_Direction=Custom,E_SLIDER_Bars_Angle={}", it->second);
        }
        if (direction == "V_expand") direction = "expand";
        if (direction == "V_compress") direction = "compress";
        if (direction == "H_expand") direction = "H-expand";
        if (direction == "H_compress") direction = "H-compress";
        if (direction == "left") direction = "Left";
        if (direction == "right") direction = "Right";
        if (direction == "block_up") direction = "Alternate Up";
        if (direction == "block_down") direction = "Alternate Down";
        if (direction == "block_left") direction = "Alternate Left";
        if (direction == "block_right") direction = "Alternate Right";
        return ",E_CHOICE_Bars_Direction=" + direction;
    }

    std::string PercentDecode(std::string file) {
        size_t pos;
        while ((pos = file.find('%')) != std::string::npos) {
            if (pos + 2 < file.size()) {
                char c = HexToChar(file[pos + 1], file[pos + 2]);
                file.replace(pos, 3, std::string(1, c));
            } else {
                break;
            }
        }
        return file;
    }

    // A picture embedded in the .loredit is referenced as *name*guid*.ext
    std::string PictureReference(const std::string& encoded) {
        std::string file = PercentDecode(encoded);
        if (StartsWith(file, "*")) {
            return LOREdit::EmbeddedPictureName(file);
        }
        return file;
    }

    // Minimal URL percent-decode, matching the wxURI::Unescape that the
    // text effect's encoded label needs.
    std::string unescapeURI(const std::string& in) {
        std::string out;
        out.reserve(in.size());
        for (size_t i = 0; i < in.size(); ++i) {
            if (in[i] == '%' && i + 2 < in.size() && isHexChar(in[i + 1]) && isHexChar(in[i + 2])) {
                out += HexToChar(in[i + 1], in[i + 2]);
                i += 2;
            } else {
                out += in[i];
            }
        }
        return out;
    }
}

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

    int cnum = 0;
    std::vector<std::string> c = Split(effectSettings[0], ';');
    for (int i = 0; i < (int)c.size(); i++)
    {
        std::string n = fmt::format("{}", cnum + 1);

        std::vector<std::string> cc = Split(c[i], ',');
        if (cc.size() == 2)
        {
            std::string c1 = cc[0].substr(2); // drop transparency
            std::string active = cc[1];

            palette += ",C_BUTTON_Palette" + n + "=#" + c1;
            if (active == "1")
            {
                palette += ",C_CHECKBOX_Palette" + n + "=" + active;
            }
            cnum++;
        }
        else if (cc.size() == 3)
        {
            std::string c1 = cc[0].substr(2); // drop transparency
            std::string c2 = cc[1].substr(2); // drop transparency
            std::string active = cc[2];

            if (c1 == c2)
            {
                palette += ",C_BUTTON_Palette" + n + "=#" + c1;
                if (active == "1")
                {
                    palette += ",C_CHECKBOX_Palette" + n + "=" + active;
                }
            }
            else
            {
                palette += ",C_BUTTON_Palette" + n + "=Active=TRUE|Id=ID_BUTTON_Palette" + n + "|Values=x=0.000^c=#" + c1 + ";x=1.000^c=#" + c2 + "|";
                if (active == "1")
                {
                    palette += ",C_CHECKBOX_Palette" + n + "=" + active;
                }
            }
            cnum++;
        }
        else
        {
            // Not sure what the last value is so ignoring it
            //int unknown1 = loreAtoi(c[i]);
            break;
        }
    }

    int sparkle = loreAtoi(otherSettings[2]);

    if (sparkle > 0)
    {
        palette += ",C_SLIDER_SparkleFrequency=" + fmt::format("{}", sparkle);
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
            palette += ",C_SLIDER_Brightness=" + fmt::format("{}", startIntensity);
        }
        else
        {
            palette += ",C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=" + fmt::format("{}", startIntensity) + "|P2=" + fmt::format("{}", endIntensity) + "|RV=TRUE|";
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
    if (effectType == "picturexy") return "Pictures";
    if (effectType == "lineshorizontal") return "Lines";
    if (effectType == "linesvertical") return "Lines";
    if (effectType == "straightlines") return "Lines";
    if (effectType == "garland") return "Garlands";
    if (effectType == "spinner") return "Pinwheel";
    if (effectType == "spinfade") return "Pinwheel";
    if (effectType == "blendedbars") return "Bars";
    if (effectType == "singleblock") return "Morph";
    if (effectType == "countdown") return "Text"; // we dont support countdown
    if (effectType == "starfield") return "Shape";
    if (effectType == "sketch") return GetSketchSVG().empty() ? "" : "Pictures";
    if (effectType == "movingshapes") return "Shape";
    if (effectType == "simpleshape") return "Shape";

    return Capitalise(effectType);
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

std::string LOREditEffect::RescaleWithRangeI(const std::string& r, const std::string& vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, std::string& vc, float targetRealMin, float targetRealMax)
{
    if (Contains(r, "R"))
    {
        // it is a range
        std::vector<std::string> rr = Split(r, 'R');
        vc = "," + vcName + "=Active=TRUE|Id=ID_" + vcName.substr(2) + "|Type=Ramp|Min=" + fmt::format("{:.2f}", targetRealMin) +
            "|Max=" + fmt::format("{:.2f}", targetRealMax) +
            "|P1=" + fmt::format("{:.2f}", Rescale(loreAtof(rr[0]), sourceMin, sourceMax, targetMin, targetMax)) +
            "|P2=" + fmt::format("{:.2f}", Rescale(loreAtof(rr[1]), sourceMin, sourceMax, targetMin, targetMax)) +
            "|RV=TRUE|";
        return fmt::format("{}", (int)Rescale(loreAtof(rr[0]), sourceMin, sourceMax, targetMin, targetMax));
    }
    else
    {
        vc = "";
        return fmt::format("{}", (int)Rescale(loreAtof(r), sourceMin, sourceMax, targetMin, targetMax));
    }
}

std::string LOREditEffect::RescaleWithRangeF(const std::string& r, const std::string& vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, std::string& vc, float targetRealMin, float targetRealMax)
{
    if (Contains(r, "R"))
    {
        // it is a range
        std::vector<std::string> rr = Split(r, 'R');
        vc = "," + vcName + "=Active=TRUE|Id=ID_" + vcName.substr(2) + "|Type=Ramp|Min=" + fmt::format("{:.2f}", targetRealMin) +
            "|Max=" + fmt::format("{:.2f}", targetRealMax) +
            "|P1=" + fmt::format("{:.2f}", Rescale(loreAtof(rr[0]), sourceMin, sourceMax, targetMin, targetMax)) +
            "|P2=" + fmt::format("{:.2f}", Rescale(loreAtof(rr[1]), sourceMin, sourceMax, targetMin, targetMax)) +
            "|RV=TRUE|";
    }
    else
    {
        vc = "";
    }
    return fmt::format("{:.1f}", Rescale(loreAtof(r), sourceMin, sourceMax, targetMin, targetMax));
}

std::string LOREditEffect::GetSketchSVG() const
{
    if (effectType != "sketch" || effectSettings.size() < 2) {
        return "";
    }
    auto parms = Split(effectSettings[1], ',');
    if (parms.empty() || parms[0].empty()) {
        return "";
    }
    parms.resize(std::max<size_t>(parms.size(), 7));

    // groups are joined by '~': a "GSVG..." header (fill colour, pen, fill
    // flag) followed by its "PPath..." paths in SVG path syntax, 0-1 and y down
    std::string body;
    std::string colour = "#FFFFFF";
    std::string opacity;
    bool fill = true;
    float pen = 1.0f;
    std::string d;
    auto flush = [&]() {
        if (d.empty()) {
            return;
        }
        if (fill) {
            // a group is one compound path: its later paths cut holes
            body += "<path fill-rule=\"evenodd\" fill=\"" + colour + "\"" + opacity + " stroke=\"none\" d=\"" + d + "\"/>";
        } else {
            body += "<path fill=\"none\" stroke=\"" + colour + "\"" + opacity + fmt::format(" stroke-width=\"{:.4f}\"", std::max(pen, 1.0f) / 100.0f) + " d=\"" + d + "\"/>";
        }
        d.clear();
    };
    for (const auto& part : Split(parms[0], '~')) {
        if (StartsWith(part, "G")) {
            flush();
            auto f = Split(part, ' ');
            f.resize(std::max<size_t>(f.size(), 12));
            // colours: AARRGGBB-<1 if selected>[;...]
            auto cols = Split(f[9], ';');
            std::string chosen = cols.empty() ? std::string() : cols[0];
            for (const auto& c : cols) {
                if (EndsWith(c, "-1")) {
                    chosen = c;
                    break;
                }
            }
            if (chosen.size() >= 8) {
                colour = "#" + chosen.substr(2, 6);
                int alpha = (int)std::strtol(chosen.substr(0, 2).c_str(), nullptr, 16);
                opacity = alpha < 255 ? fmt::format(" fill-opacity=\"{:.3f}\" stroke-opacity=\"{:.3f}\"", alpha / 255.0, alpha / 255.0) : std::string();
            }
            pen = (float)loreAtof(f[10]);
            fill = f[11] != "False";
        } else if (StartsWith(part, "P")) {
            auto pos = part.find(" M ");
            if (pos != std::string::npos) {
                if (!d.empty()) {
                    d += ' ';
                }
                d += ConvertLORSketchPath(Trim(part.substr(pos + 1)));
            }
        }
    }
    flush();
    if (body.empty()) {
        return "";
    }

    double w0, w1, h0, h1, r0, r1;
    ParamRange(parms[1].empty() ? std::string("100") : parms[1], w0, w1);
    ParamRange(parms[2].empty() ? std::string("100") : parms[2], h0, h1);
    ParamRange(parms[5], r0, r1);
    // a zoom ramp has no Pictures equivalent here; draw it at the middle size
    double sx = (w0 + w1) / 200.0;
    double sy = (h0 + h1) / 200.0;
    double rot = std::fmod((r0 + r1) / 2.0, 360.0);
    // 100% stretches the 0-1 canvas over the whole prop. Other sizes scale
    // about the anchor: the flags are Left/Right/Top/Bottom ("FFTF" = top),
    // a lone anchored edge stays put, otherwise the prop's centre does.
    std::string anchor = parms[6];
    anchor.resize(4, 'F');
    double px = (anchor[0] == 'T') == (anchor[1] == 'T') ? 0.5 : (anchor[0] == 'T' ? 0.0 : 1.0);
    double py = (anchor[2] == 'T') == (anchor[3] == 'T') ? 0.5 : (anchor[2] == 'T' ? 0.0 : 1.0);
    double left, top;
    SketchBakedOffset(parms, left, top);
    std::string xf = fmt::format("translate({:.4f} {:.4f}) scale({:.4f} {:.4f})",
                                 px - px * sx + left * kSketchOffsetStep, py - py * sy + top * kSketchOffsetStep, sx, sy);
    if (std::abs(rot) > 0.01) {
        xf = fmt::format("rotate({:.2f} {:.4f} {:.4f}) ", rot, px + left * kSketchOffsetStep, py + top * kSketchOffsetStep) + xf;
    }
    return "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"1000\" height=\"1000\" viewBox=\"0 0 1 1\" preserveAspectRatio=\"none\">"
           "<g transform=\"" + xf + "\">" + body + "</g></svg>";
}

std::string LOREditEffect::GetSketchPictureName() const
{
    std::string svg = GetSketchSVG();
    uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : svg) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    return fmt::format("sketch-{:012x}.svg", h & 0xFFFFFFFFFFFFULL);
}

std::string LOREditEffect::GetBlend() const
{
    if (!left) return "Normal";
    if (otherSettings.size() == 0) return "Normal";

    std::string blend = otherSettings[0];

    if (blend == "Mix_Average") return "Average";
    // LOR overlay draws the right side over the left wherever it is lit; the
    // left side is the upper xLights layer, so show it only where the lower
    // (right) layer is black
    if (blend == "Mix_Overlay") return "Layered";
    if (blend == "Mix_Maximum") return "Max";
    if (blend == "Mix_Bottom_Top") return "Bottom-Top";
    if (blend == "Mix_Left-Right") return "Left-Right";
    if (blend == "Mix_Rt_Hides_Lt") return "1 is Mask";
    if (blend == "Mix_Rt_Reveals_Lt") return "2 reveals 1";

    return "Normal";
}

std::string LOREditEffect::GetLayerSettings() const
{
    // The mix only combines the two sides of one effect. A side on its own
    // renders fully, whatever mix and position it carries.
    if (!left || !otherSidePresent || otherSettings.size() < 2) {
        return ",T_CHOICE_LayerMethod=Normal";
    }
    std::string const& mix = otherSettings[0];
    if (StartsWith(mix, "Dissolve_")) {
        // A pixel dissolve from the left side to the right across the effect
        // (R100R0... runs the other way). The left side is the upper layer, so
        // it dissolves out to reveal the right, or dissolves in over it.
        auto ramp = Split(otherSettings[1], 'R');
        bool reverse = ramp.size() > 2 && loreAtof(ramp[1]) > loreAtof(ramp[2]);
        double secs = (double)(endMS - startMS) / 1000.0;
        if (reverse) {
            return fmt::format(",T_CHOICE_LayerMethod=Normal,T_CHOICE_In_Transition_Type=Dissolve,T_TEXTCTRL_Fadein={:.2f}", secs);
        }
        return fmt::format(",T_CHOICE_LayerMethod=Normal,T_CHOICE_Out_Transition_Type=Dissolve,T_TEXTCTRL_Fadeout={:.2f}", secs);
    }
    return ",T_CHOICE_LayerMethod=" + GetBlend();
}

std::string LOREditEffect::GetSubBuffer() const
{
    if (trackType == "custom_horizontal_buffer") {
        // LOR lays this row's nodes out as one horizontal line
        return ",B_CHOICE_BufferStyle=Single Line";
    }
    if (trackType != "rectangle") {
        return "";
    }
    // LOR measures y from the top, xLights sub-buffers from the bottom
    auto pct = [](float v) { return std::clamp(v * 100.0f, 0.0f, 100.0f); };
    float x1 = pct(subx);
    float x2 = pct(subx + subw);
    float y1 = pct(1.0f - suby - subh);
    float y2 = pct(1.0f - suby);
    if (x1 <= 0.0f && y1 <= 0.0f && x2 >= 100.0f && y2 >= 100.0f) {
        return "";
    }
    return fmt::format(",B_CUSTOM_SubBuffer={:.2f}x{:.2f}x{:.2f}x{:.2f}", x1, y1, x2, y2);
}

std::string LOREditEffect::GetSettings(std::string& palette) const
{


    if (effectType == "INTENSITY") {
        // intensity ramp 0-100
        return fmt::format("E_TEXTCTRL_Eff_On_End={},E_TEXTCTRL_Eff_On_Start={}", endIntensity, startIntensity);
    }
    if (effectType == "DMX_INTENSITY") {
        return fmt::format("E_VALUECURVE_DMX1=Active=TRUE|Id=ID_VALUECURVE_DMX1|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", startIntensity, endIntensity);
    }
    if (effectType == "SHIMMER") {
        return fmt::format("E_CHECKBOX_On_Shimmer=1,E_TEXTCTRL_Eff_On_End={},E_TEXTCTRL_Eff_On_Start={}", endIntensity, startIntensity);
    }
    if (effectType == "TWINKLE") {
        return "E_SLIDER_Twinkle_Count=48";
    }

    if (effectSettings.size() == 0) {
        return "";
    }

    std::string et = effectType;
    if (StartsWith(et, "lightorama_")) {
        et = AfterFirst(et, '_');
    }

    std::string settings;
    auto parms = Split(effectSettings.size() > 1 ? effectSettings[1] : std::string(), ',');
    // parameter lists vary between LOR versions; never index past the end
    if (parms.size() < 24) {
        parms.resize(24);
    }
    if (et == "butterfly") {
        std::string style = parms[0];
        std::string chunks = parms[1];
        std::string vcChunks;
        // VC bounds come from the effect's statics populated via Butterfly.json at startup.
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
        }
        else if (style == "radial") {
            settings += ",E_SLIDER_Butterfly_Style=2";
        }
        else if (style == "blocks") {
            settings += ",E_SLIDER_Butterfly_Style=3";
        }
        else if (style == "corner") {
            settings += ",E_SLIDER_Butterfly_Style=5";
        }
        settings += ",E_CHOICE_Butterfly_Colors=" + Capitalise(colours);
        settings += ",E_CHOICE_Butterfly_Direction=" + Capitalise(direction);
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
    }
    else if (et == "colorwash") {
        //full, full, none, 12
        // LOR 6: full,full,single_color|dither|..._gradient
        std::string horizontalFade = parms[0];
        std::string verticalFade = parms[1];

        if (otherSettings.size() > 3 && otherSettings[3] == "blink_in_unison") {
            // a whole-effect on/off strobe, measured at 10Hz for rate 20; Color
            // Wash's shimmer blanks every other frame, the same at 50ms frames
            settings += ",E_CHECKBOX_ColorWash_Shimmer=1";
        }
        if (parms[2] == "single_color") {
            // single colour mode shows only the first selected colour, where
            // xLights would cycle through every checked palette entry
            static const std::regex checkedRe(R"(,?C_CHECKBOX_Palette(\d)=1)");
            std::smatch m;
            if (std::regex_search(palette, m, checkedRe)) {
                std::string first = m[1].str();
                std::string res;
                std::string rest = palette;
                while (std::regex_search(rest, m, checkedRe)) {
                    res += m.prefix().str();
                    if (m[1].str() == first) {
                        res += m.str();
                    }
                    rest = m.suffix().str();
                }
                palette = res + rest;
            }
        }

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
    else if (et == "spirals") {
        // 1, left_to_right, 20, 50, 0, False, none, 12
        std::string repeat = parms[0];
        std::string vcRepeat;
        repeat = RescaleWithRangeI(repeat, "E_VALUECURVE_Spirals_Count", 1, 5, 1, 5, vcRepeat, SpiralsEffect::sCountMin, SpiralsEffect::sCountMax);
        std::string direction = parms[1];
        std::string rotation = parms[2];
        rotation = fmt::format("{:.2f}", loreAtof(rotation) / 60.0);
        std::string vcRotation;
        rotation = RescaleWithRangeF(rotation, "E_VALUECURVE_Spirals_Rotation", 0, 50, 0, 50, vcRotation, SpiralsEffect::sRotationMin, SpiralsEffect::sRotationMax);
        rotation = fmt::format("{}", (int)(loreAtof(rotation) * 10.0));
        std::string thickness = parms[3];
        std::string vcThickness;
        thickness = RescaleWithRangeI(thickness, "E_VALUECURVE_Spirals_Thickness", 0, 100, 0, 100, vcThickness, SpiralsEffect::sThicknessMin, SpiralsEffect::sThicknessMax);
        // std::string thicknessChange = parms[4]; //unused
        std::string blend = parms[5];
        std::string show3d = parms[6];
        std::string speed = parms[7];
        speed = fmt::format("{:.2f}", loreAtof(speed) / (20.0 / ((float)(endMS - startMS) / 1000.0)));
        std::string vcSpeed;
        if (direction == "right_to_left") {
            speed = RescaleWithRangeF(speed, "E_VALUECURVE_Spirals_Movement", 0, 50, 0, -50, vcSpeed, SpiralsEffect::sMovementMin, SpiralsEffect::sMovementMax);
        }
        else {
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

        // dont know what to do with thickness change

        if (blend == "True") {
            settings += ",E_CHECKBOX_Spirals_Blend=1";
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
    else if (et == "bars") {
        // down,2,False,False,8,0
        std::string direction = parms[0];
        std::string repeat = parms[1];
        std::string vcRepeat;
        // VC bounds come from the effect's statics populated via Bars.json at startup.
        repeat = RescaleWithRangeI(repeat, "E_VALUECURVE_Bars_BarCount", 1, 5, 1, 5, vcRepeat, BarsEffect::sBarCountMin, BarsEffect::sBarCountMax);
        std::string highlight = parms[2];
        std::string show3d = parms[3];
        // LOR moves the bars speed/20 prop widths a second (measured against LOR
        // renders); one xLights cycle is also one full width of the pattern
        std::string speed = parms[4];
        speed = fmt::format("{:.2f}", loreAtof(speed) / (20.0 / ((float)(endMS - startMS) / 1000.0)));
        std::string vcSpeed;
        speed = RescaleWithRangeF(speed, "E_VALUECURVE_Bars_Cycles", 0, 30, 0, 30, vcSpeed, BarsEffect::sCyclesMin, BarsEffect::sCyclesMax);
        std::string centre = parms[5];
        std::string vcCentre;
        centre = RescaleWithRangeI(centre, "E_VALUECURVE_Bars_Center", -50, 50, -100, 100, vcCentre, BarsEffect::sCenterMin, BarsEffect::sCenterMax);

        settings += ",E_SLIDER_Bars_BarCount=" + repeat;
        settings += vcRepeat;

        settings += BarsDirectionSettings(direction);

        if (show3d == "True") {
            settings += ",E_CHECKBOX_Bars_3D=1";
        }

        // older files store True/False, LOR 6 none/white/last color
        if (highlight == "True" || highlight == "white" || highlight == "last_color") {
            settings += ",E_CHECKBOX_Bars_Highlight=1";
        }

        settings += ",E_TEXTCTRL_Bars_Cycles=" + speed;
        settings += vcSpeed;

        settings += ",E_TEXTCTRL_Bars_Center=" + centre;
        settings += vcCentre;
    }
    else if (et == "countdown") {
        // 0,Arial,75,7
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
    }
    else if (et == "lineshorizontal") {
        // Bottom_to_Top,8,38

        // No xLights equivalent

        spdlog::warn("LPE conversion for Lines Horizontal does not exist.");
    }
    else if (et == "linesvertical") {
        // Left_to_Right,4,32

        // No xLights equivalent

        spdlog::warn("LPE conversion for Lines Vertical does not exist.");
    }
    else if (et == "curtain") {
        // center,open,0,once_at_speed,12
        std::string edge = parms[0];
        std::string movement = parms[1];
        std::string swag = parms[2];
        std::string vcSwag;
        swag = RescaleWithRangeF(swag, "E_VALUECURVE_Curtain_Swag", 0, 10, 0, 10, vcSwag, CurtainEffect::sSwagMin, CurtainEffect::sSwagMax);
        std::string repeat = parms[3];
        std::string speed = parms[4];
        std::string vcSpeed;
        // Curtain_Speed's pre-migration range was (0, 10) post-divisor. Write the VC in
        // that legacy form — UpgradeValueCurve rescales it to the new (0, 100) pre-divisor
        // form on the first sequence load. Keeping the literal here avoids a second rescale
        // (post-divisor slider vs pre-divisor VC) in the import path.
        speed = RescaleWithRangeF(speed, "E_VALUECURVE_Curtain_Speed", 0, 50, 0, 10, vcSpeed, 0, 10);

        if (repeat == "once_fit_to_duration") {
            // xLights' non-repeating curtain finishes at speed x the effect's
            // length, so 1.0 ends exactly with it. LOR 6 adds a progress ramp
            // (R0R100... = the whole movement).
            double p0 = 0, p1 = 100;
            if (!parms[5].empty()) {
                ParamRange(parms[5], p0, p1);
            }
            double span = (p1 - p0) / 100.0;
            speed = fmt::format("{:.2f}", p0 == 0 && span > 0 ? span : 1.0);
            vcSpeed.clear();
        }

        settings += ",E_CHOICE_Curtain_Edge=" + edge;
        Replace(movement, "_", " ");
        if (movement == "chase") {
            movement = "open then close"; // no chase in xLights
        }
        settings += ",E_CHOICE_Curtain_Effect=" + movement;
        settings += ",E_SLIDER_Curtain_Swag=" + swag;
        settings += vcSwag;

        if (repeat == "once_at_speed") {
            settings += ",E_CHECKBOX_Curtain_Repeat=0";
        }
        else if (repeat == "once_fit_to_duration") {
            settings += ",E_CHECKBOX_Curtain_Repeat=0";
        }
        else if (repeat == "repeat_at_speed_rotate_colors") {
            settings += ",E_CHECKBOX_Curtain_Repeat=1";
        }
        else if (repeat == "repeat_at_speed_blend_colors") {
            settings += ",E_CHECKBOX_Curtain_Repeat=1";
        }
        settings += ",E_TEXTCTRL_Curtain_Speed=" + speed;
        settings += vcSpeed;
    }
    else if (et == "fire") {
        //50,0
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
    }
    else if (et == "fireworks") {
        // 10,50,2,30,normal,continuous
        std::string explosionRate = parms[0];
        std::string vcCrap;
        explosionRate = RescaleWithRangeI(explosionRate, "IGNORE", 1, 95, 1, 50, vcCrap, -1, -1);
        std::string particles = parms[1];
        particles = RescaleWithRangeI(particles, "IGNORE", 1, 100, 1, 100, vcCrap, -1, -1);
        std::string velocity = parms[2];
        velocity = RescaleWithRangeI(velocity, "IGNORE", 1, 10, 1, 10, vcCrap, -1, -1);
        std::string fade = parms[3];
        fade = RescaleWithRangeI(fade, "IGNORE", 1, 100, 1, 100, vcCrap, -1, -1);
        // std::string pattern = parms[4]; // not used
        // std::string rateChange = parms[5]; // not used
        settings += ",E_SLIDER_Fireworks_Explosions=" + explosionRate;
        settings += ",E_SLIDER_Fireworks_Count=" + particles;
        settings += ",E_SLIDER_Fireworks_Fade=" + fade;
        settings += ",E_SLIDER_Fireworks_Velocity=" + velocity;
    }
    else if (et == "garland") {
        // 3,34,once_at_speed,12,bottom_to_top
        std::string type = parms[0];
        std::string vcCrap;
        type = RescaleWithRangeI(type, "IGNORE", 0, 4, 0, 4, vcCrap, -1, -1);
        std::string spacing = parms[1];
        std::string vcSpacing;
        spacing = RescaleWithRangeI(spacing, "E_VALUECURVE_Garlands_Spacing", 0, 100, 1, 100, vcSpacing, GarlandsEffect::sSpacingMin, GarlandsEffect::sSpacingMax);
        std::string repeat = parms[2];
        std::string speed = parms[3];
        std::string vcSpeed;
        // Garlands_Cycles pre-migration range was (0, 20) post-divisor. Write the VC in
        // that legacy form — UpgradeValueCurve rescales it to the new (0, 200) pre-divisor
        // form with divisor 10 on the first sequence load.
        speed = RescaleWithRangeF(speed, "E_VALUECURVE_Garlands_Cycles", 0, 50, 0, 20, vcSpeed, 0, 20);
        std::string fill = parms[4];

        settings += ",E_SLIDER_Garlands_Type=" + type;

        if (fill == "bottom_to_top") {
            settings += ",E_CHOICE_Garlands_Direction=Up";
        }
        else if (fill == "top_to_bottom") {
            settings += ",E_CHOICE_Garlands_Direction=Down";
        }
        else if (fill == "left_to_right") {
            settings += ",E_CHOICE_Garlands_Direction=Right";
        }
        else if (fill == "right_to_left") {
            settings += ",E_CHOICE_Garlands_Direction=Left";
        }

        settings += ",E_SLIDER_Garlands_Spacing=" + spacing;
        settings += vcSpacing;

        if (repeat == "repeat_at_speed") {
            settings += ",E_TEXTCTRL_Garlands_Cycles=" + speed;
            settings += vcSpeed;
        }
        else if (repeat == "once_at_speed") {
            settings += ",E_TEXTCTRL_Garlands_Cycles=1.0";
        }
        else if (repeat == "once_fit_to_duration") {
            settings += ",E_TEXTCTRL_Garlands_Cycles=1.0";
        }
    }
    else if (et == "marquee") {
        //4,1,1,12
        //spacing,filled space,size,speed
        std::string spacing = parms[0];
        std::string filledspace = parms[1];
        std::string size = parms[2];
        std::string speed = parms[4];
        std::string vcSpacing;
        spacing = RescaleWithRangeI(spacing, "E_SLIDER_Marquee_Skip_Size", 1, 20, 1, 20, vcSpacing, MarqueeEffect::sSkipSizeMin, MarqueeEffect::sSkipSizeMax);
        settings += ",E_SLIDER_Marquee_Skip_Size=" + spacing;
        settings += vcSpacing;

        std::string vcBandSize;
        filledspace = RescaleWithRangeI(filledspace, "E_SLIDER_Marquee_Band_Size", 1, 100, 1, 100, vcBandSize, MarqueeEffect::sBandSizeMin, MarqueeEffect::sBandSizeMax);
        settings += ",E_SLIDER_Marquee_Band_Size=" + filledspace;
        settings += vcBandSize;

        std::string vcSize;
        size = RescaleWithRangeI(size, "E_SLIDER_Marquee_Thickness", 1, 20, 1, 20, vcSize, MarqueeEffect::sThicknessMin, MarqueeEffect::sThicknessMax);
        settings += ",E_SLIDER_Marquee_Thickness=" + size;
        settings += vcSize;

        std::string vcSpeed;
        speed = RescaleWithRangeI(speed, "E_SLIDER_Marquee_Speed", 1, 50, 1, 50, vcSpeed, MarqueeEffect::sSpeedMin, MarqueeEffect::sSpeedMax);
        settings += ",E_SLIDER_Marquee_Speed=" + speed;
        settings += vcSpeed;
    }
    else if (et == "meteors") {
        // rainbow,10,25,down,0,12
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

        settings += ",E_CHOICE_Meteors_Type=" + Capitalise(Lower(colourScheme));
        settings += ",E_SLIDER_Meteors_Count=" + count;
        settings += vcCount;
        settings += ",E_SLIDER_Meteors_Length=" + length;
        settings += vcLength;
        settings += ",E_CHOICE_Meteors_Effect=" + Capitalise(effect);
        settings += ",E_SLIDER_Meteors_Swirl_Intensity=" + swirl;
        settings += vcSwirl;
        settings += ",E_SLIDER_Meteors_Speed=" + speed;
        settings += vcSpeed;
    }
    else if (et == "movie") {
        // xxx.avi,True,False
        std::string file = parms[0];
        std::string scale = parms[1];
        std::string fullLength = parms[2];

        settings += ",E_FILEPICKERCTRL_Video_Filename=" + file;

        if (scale == "True") {
            settings += ",E_CHECKBOX_Video_AspectRatio=0";
        }
        else {
            settings += ",E_CHECKBOX_Video_AspectRatio=1";
        }
        if (fullLength == "True") {
            settings += ",E_CHOICE_Video_DurationTreatment=Slow/Accelerate";
        }
        else {
            settings += ",E_CHOICE_Video_DurationTreatment=Normal";
        }
    }
    else if (et == "picture") {
        // file.jpg,True,none,0,10,19,12
        // LOR 6: *name*guid*.gif,fit_both,none,0,10,,12
        std::string file = PictureReference(parms[0]);
        std::string scale = parms[1];
        std::string movement = parms[2];
        std::string x = parms[3];
        std::string vcCrap;
        x = RescaleWithRangeI(x, "IGNORE", -50, 50, -100, 100, vcCrap, -1, -1);
        std::string speed = parms[6];
        speed = RescaleWithRangeF(speed, "IGNORE", 0, 50, 0, 20, vcCrap, -1, -1);

        settings += ",E_TEXTCTRL_Pictures_Filename=" + file;
        if (scale == "True" || scale == "fit_both") {
            settings += ",E_CHOICE_Scaling=Scale To Fit";
        } else if (StartsWith(scale, "fit_")) {
            settings += ",E_CHOICE_Scaling=Scale Keep Aspect Ratio";
        } else {
            settings += ",E_CHOICE_Scaling=No Scaling";
        }
        if (EndsWith(Lower(file), ".gif")) {
            settings += ",E_CHECKBOX_LoopGIF=1";
        }

        Replace(movement, "_", "-");
        if (movement == "peekaboo-bottom") {
            movement = "peekaboo";
        }
        else if (movement == "peekaboo-top") {
            movement = "peekaboo 180";
        }
        else if (movement == "peekaboo-left") {
            movement = "peekaboo 90";
        }
        else if (movement == "peekaboo-right") {
            movement = "peekaboo 270";
        }
        settings += ",E_CHOICE_Pictures_Direction=" + movement;
        settings += ",E_SLIDER_PicturesXC=" + x;
        settings += ",E_TEXTCTRL_Pictures_Speed=" + speed;
    } else if (et == "picturexy") {
        // file.jpg,71,104,0,R32R0R1.00,100,100,100,100,FFFF
        // file.jpg,58,136,0,0         ,100,100,100,100,FFFF,0

        std::string file = PictureReference(parms[0]);
        std::string scaleX = parms[1];
        std::string scaleY = parms[2];
        std::string movementLeft = parms[3];
        std::string movementTop = parms[4];

        settings += ",E_TEXTCTRL_Pictures_Filename=" + file;
        if (EndsWith(Lower(file), ".gif")) {
            settings += ",E_CHECKBOX_LoopGIF=1";
        }
        // width/height are percentages of the prop; 100x100 stretches the
        // picture over the whole prop
        auto isZero = [](const std::string& v) { return v.empty() || v == "0"; };
        if (scaleX == "100" && scaleY == "100" && isZero(movementLeft) && isZero(movementTop)) {
            settings += ",E_CHOICE_Scaling=Scale To Fit";
            settings += ",E_CHOICE_Pictures_Direction=none";
            return settings + GetLayerSettings() + GetSubBuffer();
        }
        settings += ",E_CHOICE_Scaling=No Scaling";

        settings += ",E_CHOICE_Pictures_Direction=vector";
        if (Contains(movementLeft, "R"))
        {
            std::vector<std::string> rr = Split(movementLeft, 'R');
            settings += ",E_SLIDER_PicturesXC=" + rr[1];
            settings += ",E_SLIDER_PicturesEndXC=" + rr[2];
        } else if (isNumber(movementLeft)) {
            settings += ",E_SLIDER_PicturesXC=" + movementLeft;
            settings += ",E_SLIDER_PicturesEndXC=" + movementLeft;
        }
        if (Contains(movementTop, "R")) {
            std::vector<std::string> rr = Split(movementTop, 'R');
            settings += ",E_SLIDER_PicturesYC=" + rr[1];
            settings += ",E_SLIDER_PicturesEndYC=" + rr[2];
        } else if (isNumber(movementTop)) {
            settings += ",E_SLIDER_PicturesYC=" + movementTop;
            settings += ",E_SLIDER_PicturesEndYC=" + movementTop;
        }
        if (Contains(scaleX, "R")) {
            std::vector<std::string> rr = Split(scaleX, 'R');
            settings += ",E_SLIDER_Pictures_StartScale=" + rr[1];
            settings += ",E_SLIDER_Pictures_EndScale=" + rr[2];
        } else if (isNumber(scaleX)) {
            settings += ",E_SLIDER_Pictures_StartScale=" + scaleX;
            settings += ",E_SLIDER_Pictures_EndScale=" + scaleX;
        }

        settings += ",E_TEXTCTRL_Pictures_Speed=1.0";
    }
    else if (et == "spinner") {
        //           style,   colour_mode, arms, arm width, inner_radius, bend, curvature, speed, width, height,   x,   y
        //               0,             1,    2,         3,            4,    5,         6,     7,     8,      9,  10,  11
        // MIN: pinwheel_1, color_per_arm,    1,         0,          -32,    0,         0,   -32,     1,      1, -50, -50
        // MAX: pinwheel_1, color_per_arm,   10,       100,           32,   50,        10,    32,   200,    200,  50,  50
        // pinwheel_1,color_per_arm,10,15,0,0,5,15,200,200,-24,0

        // std::string style = parms[0]; // unused
        // std::string colour_mode = parms[1]; //unused
        std::string arms = parms[2];
        std::string vcCrap;
        arms = RescaleWithRangeI(arms, "IGNORE", 1, 10, 1, 10, vcCrap, -1, -1);
        std::string armwidth = parms[3];
        std::string vcArmWidth;
        armwidth = RescaleWithRangeI(armwidth, "E_VALUECURVE_Pinwheel_Thickness", 0, 100, 0, 100, vcArmWidth, PinwheelEffect::sThicknessMin, PinwheelEffect::sThicknessMax);
        // std::string innerRadius = parms[4]; // not used
        std::string bend = parms[5];
        std::string vcBend;
        bend = RescaleWithRangeI(bend, "E_VALUECURVE_Pinwheel_Twist", 0, 50, -360, 360, vcBend, PinwheelEffect::sTwistMin, PinwheelEffect::sTwistMax);
        // std::string curvature = parms[6]; // not used
        std::string speed = parms[7];
        std::string vcSpeed;
        bool ccw = false;
        // need to do some funkiness with the ranges as we dont support negative numbers
        if (loreAtoi(speed) < 0)
        {
            ccw = true;
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Pinwheel_Speed", -32, 32, 0, 100, vcSpeed, PinwheelEffect::sSpeedMin, PinwheelEffect::sSpeedMax);
            speed = fmt::format("{}", 50 - loreAtoi(speed));
        }
        else             {
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Pinwheel_Speed", -32, 32, -50, 50, vcSpeed, PinwheelEffect::sSpeedMin, PinwheelEffect::sSpeedMax);
        }

        std::string length = parms[8];
        std::string vcLength;
        length = RescaleWithRangeI(length, "E_VALUECURVE_Pinwheel_ArmSize", 1, 100, 0, 400, vcLength, PinwheelEffect::sArmSizeMin, PinwheelEffect::sArmSizeMax);

        // std::string height = parms[9]; //unused

        std::string x = parms[10];
        std::string vcX;
        x = RescaleWithRangeI(x, "E_VALUECURVE_PinwheelXC", -50, 50, -100, 100, vcX, PinwheelEffect::sXCMin, PinwheelEffect::sXCMax);
        std::string y = parms[11];
        std::string vcY;
        y = RescaleWithRangeI(y, "E_VALUECURVE_PinwheelYC", -50, 50, -100, 100, vcY, PinwheelEffect::sYCMin, PinwheelEffect::sYCMax);

        settings += ",E_SLIDER_Pinwheel_Arms=" + arms;
        settings += ",E_SLIDER_Pinwheel_Thickness=" + armwidth;
        settings += vcArmWidth;
        settings += ",E_SLIDER_Pinwheel_Twist=" + bend;
        settings += vcBend;
        if (ccw) {
            settings += ",E_CHECKBOX_Pinwheel_Rotation=1";
        }
        else {
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
    else if (et == "pinwheel") {
        // 3,1,6,color_per_arm,True,12,100,10,-23

        std::string arms = parms[0];
        std::string vcCrap;
        arms = RescaleWithRangeI(arms, "IGNORE", 1, 10, 1, 10, vcCrap, -1, -1);
        std::string width = parms[1];
        std::string vcWidth;
        width = RescaleWithRangeI(width, "E_VALUECURVE_Pinwheel_Thickness", 1, 10, 0, 100, vcWidth, PinwheelEffect::sThicknessMin, PinwheelEffect::sThicknessMax);
        std::string bend = parms[2];
        std::string vcBend;
        bend = RescaleWithRangeI(bend, "E_VALUECURVE_Pinwheel_Twist", -10, 10, -360, 360, vcBend, PinwheelEffect::sTwistMin, PinwheelEffect::sTwistMax);
        // std::string colour = parms[3]; // not used
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
        }
        else {
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
    else if (et == "snowflakes") {
        //5,1,0,12,60
        std::string count = parms[0];
        std::string vcCount;
        count = RescaleWithRangeI(count, "E_VALUECURVE_Snowflakes_Count", 1, 20, 1, 20, vcCount, SnowflakesEffect::sCountMin, SnowflakesEffect::sCountMax);
        std::string type = parms[1];
        std::string vcCrap;
        type = RescaleWithRangeI(type, "IGNORE", 0, 5, 0, 5, vcCrap, -1, -1);
        std::string direction = parms[2]; // not used
        direction = RescaleWithRangeI(direction, "IGNORE", -8, 8, -8, 8, vcCrap, -1, -1);
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
            settings += vcCount;
            settings += vcSpeed;
        }
        else {
            settings += ",E_CHOICE_Falling=Falling & Accumulating";
            settings += vcCount;
            settings += vcSpeed;
        }
    }
    else if (et == "text") {
        //Hello%26nbsp%3B%20Keith,50,left,0,10,0,4,True
        // LOR 6: <html>,size,movement,position,hold,,speed,repeat,wrap,exit,smooth
        std::string colour;
        std::string text = StripLORHtml(unescapeURI(parms[0]), colour);
        Replace(text, ",", "&comma;");
        Replace(text, "\n", "\\n"); // the Text effect expands a literal \n
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

        // LOR colours text from its markup, not the palette; unstyled text is white
        static const std::regex paletteRe(R"((^|,)C_(BUTTON|CHECKBOX)_Palette\d=[^,]*)");
        palette = std::regex_replace(palette, paletteRe, "");
        palette = "C_BUTTON_Palette1=" + ColourNameToHex(colour.empty() ? std::string("white") : colour) +
                  ",C_CHECKBOX_Palette1=1" + (StartsWith(palette, ",") || palette.empty() ? "" : ",") + palette;
    }
    else if (et == "twinkle") {
        // 50,25,twinkle,random
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
        }
        else if (layout == "random") {
            settings += ",E_CHECKBOX_Twinkle_ReRandom=1";
        }

        if (mode == "twinkle") {
            settings += ",E_CHECKBOX_Twinkle_Strobe=0";
        }
        else // pulse/flash
        {
            settings += ",E_CHECKBOX_Twinkle_Strobe=1";
        }
    }
    else if (et == "straightlines") {
    }
    else if (et == "blendedbars") {
        // direction,count,speed,?: right,10,36,0
        settings += BarsDirectionSettings(parms[0]);
        settings += fmt::format(",E_SLIDER_Bars_BarCount={}", std::clamp(loreAtoi(parms[1]) / 2, 1, 5));
        settings += fmt::format(",E_TEXTCTRL_Bars_Cycles={:.2f}", loreAtof(parms[2]) / (20.0 / ((double)(endMS - startMS) / 1000.0)));
        settings += ",E_CHECKBOX_Bars_Gradient=1";
    }
    else if (et == "singleblock") {
        // direction,head,body,tail,position,size,offset,colourMode
        // down,0,0,50,50,100,50,single_color
        // A block crossing the prop at "fit to duration" speed: the whole block,
        // tail included, has left the prop when the effect ends, so the head
        // travels the width plus the block length. Morph's tail follows its
        // head-travel duration d as travel * (1/d - 1).
        std::string direction = parms[0];
        float head = (float)loreAtoi(parms[1]);
        float body = (float)loreAtoi(parms[2]);
        float tail = (float)loreAtoi(parms[3]);
        float size = parms[5].empty() ? 100.0f : (float)loreAtoi(parms[5]);
        float offset = parms[6].empty() ? 50.0f : (float)loreAtoi(parms[6]);
        int lo = (int)std::clamp(offset - size / 2.0f, 0.0f, 100.0f);
        int hi = (int)std::clamp(offset + size / 2.0f, 0.0f, 100.0f);
        int sx1 = 0, sy1 = 0, sx2 = 0, sy2 = 0, ex1 = 0, ey1 = 0, ex2 = 0, ey2 = 0;
        if (direction == "up") {
            sx1 = lo; sx2 = hi; sy1 = sy2 = 0;
            ex1 = lo; ex2 = hi; ey1 = ey2 = 100;
        } else if (direction == "left") {
            sy1 = lo; sy2 = hi; sx1 = sx2 = 100;
            ey1 = lo; ey2 = hi; ex1 = ex2 = 0;
        } else if (direction == "right") {
            sy1 = lo; sy2 = hi; sx1 = sx2 = 0;
            ey1 = lo; ey2 = hi; ex1 = ex2 = 100;
        } else { // down
            sx1 = lo; sx2 = hi; sy1 = sy2 = 100;
            ex1 = lo; ex2 = hi; ey1 = ey2 = 0;
        }
        int headLen = std::clamp((int)(head + body), 1, 100);
        int duration = std::clamp((int)std::lround(100.0f / (1.0f + tail / 100.0f)), 1, 100);
        settings += fmt::format(",E_SLIDER_Morph_Start_X1={},E_SLIDER_Morph_Start_Y1={},E_SLIDER_Morph_Start_X2={},E_SLIDER_Morph_Start_Y2={}", sx1, sy1, sx2, sy2);
        settings += fmt::format(",E_SLIDER_Morph_End_X1={},E_SLIDER_Morph_End_Y1={},E_SLIDER_Morph_End_X2={},E_SLIDER_Morph_End_Y2={}", ex1, ey1, ex2, ey2);
        settings += fmt::format(",E_SLIDER_MorphStartLength={},E_SLIDER_MorphEndLength={}", headLen, headLen);
        settings += fmt::format(",E_SLIDER_MorphDuration={},E_SLIDER_MorphAccel=0", duration);
        settings += ",E_CHECKBOX_Morph_Start_Link=0,E_CHECKBOX_Morph_End_Link=0,E_CHECKBOX_ShowHeadAtStart=0";
    } else if (et == "ripple") {
        // I actually think some of these should be shockwaves
        // Mix_Average|0|0|full|20|lightorama_ripple:FFFF8000,1;FF800080,1;FF0000FF,0;FFFF0000,1;FFFFFFFF,0;FF00FF00,1:circle,21,47,46,50,0,0,37,False,50|lightorama_none::
        std::string vcCrap;
        auto type = SafeGetStringParm(parms, 0);
        auto repeatcount = SafeGetStringParm(parms, 1);    // 1-20 -> 1-30
        repeatcount = RescaleWithRangeI(repeatcount, "IGNORE", 1, 20, 1 /*RIPPLE_CYCLES_MIN*/, RippleEffect::sCyclesMax, vcCrap, -1, -1); // not using min because i dont want it to scale to 0
        auto ringwidth = SafeGetIntParm(parms, 2);      // narrow is less
        //auto spacing = SafeGetIntParm(parms, 3);        // narrow is less
        //auto speed = SafeGetIntParm(parms, 4);          // slow is less
        auto leftright = SafeGetStringParm(parms, 5);      // left is negative ... zero centre
        leftright = RescaleWithRangeI(leftright, "IGNORE", -50, 50, RippleEffect::sXCMin, RippleEffect::sXCMax, vcCrap, -1, -1);
        auto topbottom = SafeGetStringParm(parms, 6);      // top is negative ... zero centre
        topbottom = RescaleWithRangeI(topbottom, "IGNORE", -50, 50, RippleEffect::sYCMin, RippleEffect::sYCMax, vcCrap, -1, -1);
        //auto highlightangle = SafeGetIntParm(parms, 7); // 0 = none
        auto inward = SafeGetBoolParm(parms, 8);
        //auto outerlimit = SafeGetIntParm(parms, 9); // 0 = small
        settings += ",E_CHOICE_Ripple_Object_To_Draw=" + ProperCase(type);
        if (inward) {
            settings += ",E_CHOICE_Ripple_Movement=Implode";
        } else {
            settings += ",E_CHOICE_Ripple_Movement=Explode";
        }
        settings += ",E_SLIDER_Ripple_XC=" + leftright;
        settings += ",E_SLIDER_Ripple_YC=" + topbottom;
        settings += ",E_TEXTCTRL_Ripple_Cycles=" + repeatcount;
        settings += ",E_SLIDER_Ripple_Thickness=" + std::to_string(ringwidth);
    }
    else if (et == "sketch") {
        // drawing,width,height,left,top,rotation,anchor
        // The drawing, with its size, rotation and position baked in, is
        // embedded as an SVG; a position ramp moves it with the Pictures
        // vector offset relative to the baked position.
        double l0, l1, t0, t1, lb, tb;
        ParamRange(parms[3], l0, l1);
        ParamRange(parms[4], t0, t1);
        SketchBakedOffset(parms, lb, tb);
        settings += ",E_TEXTCTRL_Pictures_Filename=" + GetSketchPictureName();
        settings += ",E_CHOICE_Scaling=Scale To Fit";
        if (l0 == l1 && t0 == t1) {
            settings += ",E_CHOICE_Pictures_Direction=none";
        } else {
            auto pct = [](double v) { return (int)std::lround(std::clamp(v * kSketchOffsetStep * 100.0, -100.0, 100.0)); };
            // xLights y is up, LOR's down
            settings += fmt::format(",E_CHOICE_Pictures_Direction=vector,E_SLIDER_PicturesXC={},E_SLIDER_PicturesEndXC={},E_SLIDER_PicturesYC={},E_SLIDER_PicturesEndYC={},E_TEXTCTRL_Pictures_Speed=1.0",
                                    pct(l0 - lb), pct(l1 - lb), -pct(t0 - tb), -pct(t1 - tb));
        }
    }
    else if (et == "wave") {
        // left,along_wave_scrolling,rainbow,triple,50,23,88,A3A50A1.00,A-14A50A1.00
        // LOR 6: up,across_wave_scrolling,palette,double,25,10,50,12,0,sine,repeat_at_speed,0
        if (parms[0] == "left") {
            settings += ",E_CHOICE_Wave_Direction=Right to Left";
        } else {
            settings += ",E_CHOICE_Wave_Direction=Left to Right";
        }
        if (parms[2] == "rainbow") {
            settings += ",E_CHOICE_Fill_Colors=Rainbow";
        } else if (parms[2] == "none") {
            settings += ",E_CHOICE_Fill_Colors=None";
        } else {
            settings += ",E_CHOICE_Fill_Colors=Palette";
        }
        int waves = parms[3] == "single" ? 1 : parms[3] == "double" ? 2 : parms[3] == "triple" ? 3 : 0;
        if (waves > 0) {
            settings += fmt::format(",E_TEXTCTRL_Number_Waves={:.2f}", (double)waves);
        }
        std::string type = Lower(parms[9]);
        if (type == "sine") settings += ",E_CHOICE_Wave_Type=Sine";
        else if (type == "triangle") settings += ",E_CHOICE_Wave_Type=Triangle";
        else if (type == "square") settings += ",E_CHOICE_Wave_Type=Square";
    }
    else if (et == "plasma") {
        // style,density,speed,colourMode: 1,3,12,blended
        settings += ",E_SLIDER_Plasma_Style=" + fmt::format("{}", std::clamp(loreAtoi(parms[0]), 1, 10));
        settings += ",E_SLIDER_Plasma_Line_Density=" + fmt::format("{}", std::clamp(loreAtoi(parms[1]), 1, 10));
        // 12 is LOR's default speed; xLights' default is 10
        settings += ",E_SLIDER_Plasma_Speed=" + fmt::format("{}", std::clamp(loreAtoi(parms[2]) * 10 / 12, 0, 100));
        settings += ",E_CHOICE_Plasma_Color=Normal";
    }
    else if (et == "starfield") {
        // shape,colourMode,style,density,growth,speed,tail,arms,rotation,newStarLocation,pattern
        // tree,palette,solid,50,50,50,1,3,0,25,random
        std::string shape = Capitalise(Lower(parms[0]));
        if (shape != "Heart" && shape != "Tree" && shape != "Star" && shape != "Snowflake" && shape != "Circle") {
            shape = "Star";
        }
        settings += ",E_CHOICE_Shape_ObjectToDraw=" + shape;
        settings += fmt::format(",E_SLIDER_Shape_Count={}", std::clamp(loreAtoi(parms[3]) / 5, 1, 100));
        settings += ",E_SLIDER_Shape_StartSize=1";
        settings += fmt::format(",E_SLIDER_Shape_Growth={}", std::clamp(loreAtoi(parms[4]) / 2, 1, 100));
        settings += fmt::format(",E_SLIDER_Shape_Lifetime={}", std::clamp(60 - loreAtoi(parms[5]) / 2, 5, 100));
        settings += ",E_CHECKBOX_Shape_RandomLocation=1,E_CHECKBOX_Shape_FadeAway=1";
    }
    else if (et == "movingshapes") {
        // shape,count,size,speed,colourMode,style,movement,direction,rotationMode,rotation,rotationSpeed,...
        // star5,8,73,10,palette,solid,random_wrap,90,continuous_rotation,20,12,False,0,0,1
        std::string shape = Lower(parms[0]);
        int points = 5;
        std::string object = "Circle";
        if (StartsWith(shape, "star")) {
            object = "Star";
            int p = loreAtoi(shape.substr(4));
            if (p >= 2) {
                points = std::min(p, 9);
            }
        } else if (shape == "heart") object = "Heart";
        else if (shape == "tree") object = "Tree";
        else if (shape == "snowflake") object = "Snowflake";
        else if (shape == "square") object = "Square";
        else if (shape == "triangle") object = "Triangle";
        settings += ",E_CHOICE_Shape_ObjectToDraw=" + object;
        settings += fmt::format(",E_SLIDER_Shape_Points={}", points);
        settings += fmt::format(",E_SLIDER_Shape_Count={}", std::clamp(loreAtoi(parms[1]), 1, 100));
        settings += fmt::format(",E_SLIDER_Shape_StartSize={}", std::clamp(loreAtoi(parms[2]), 1, 100));
        // LOR draws solid shapes; a thick outline is the closest xLights gets
        settings += fmt::format(",E_SLIDER_Shape_Thickness={}", parms[5] == "solid" ? 50 : 3);
        settings += ",E_SLIDER_Shape_Growth=0,E_SLIDER_Shape_Lifetime=100,E_CHECKBOX_Shape_FadeAway=0";
        settings += ",E_CHECKBOX_Shape_RandomLocation=1";
        settings += fmt::format(",E_SLIDER_Shapes_Velocity={}", std::clamp(loreAtoi(parms[3]), 0, 20));
        settings += fmt::format(",E_SLIDER_Shapes_Direction={}", std::clamp(loreAtoi(parms[7]), 0, 359));
        if (StartsWith(parms[6], "random")) {
            settings += ",E_CHECKBOX_Shapes_RandomMovement=1";
        }
    }
    else if (et == "simpleshape") {
        // shape,width,height,...,style: circle,O100O174O1.00O2.00O0.00,O100O174...,3,3,0,0,14,fade_inner_out
        std::string shape = Capitalise(Lower(parms[0]));
        settings += ",E_CHOICE_Shape_ObjectToDraw=" + (shape.empty() ? std::string("Circle") : shape);
        settings += ",E_SLIDER_Shape_Count=1,E_SLIDER_Shape_StartSize=50,E_SLIDER_Shape_Growth=0,E_SLIDER_Shape_Lifetime=100";
        settings += ",E_CHECKBOX_Shape_RandomLocation=0,E_CHECKBOX_Shape_FadeAway=0,E_SLIDER_Shape_Thickness=5";
    }
    else if (et == "spinfade") {
        // style,arms,width,?,speed,...: arc,4,50,0,12,100,100,0,0,False
        settings += fmt::format(",E_SLIDER_Pinwheel_Arms={}", std::clamp(loreAtoi(parms[1]), 1, 20));
        settings += fmt::format(",E_SLIDER_Pinwheel_Thickness={}", std::clamp(loreAtoi(parms[2]), 0, 100));
        settings += fmt::format(",E_SLIDER_Pinwheel_Speed={}", std::clamp(loreAtoi(parms[4]) * 10 / 12, 0, 50));
        settings += ",E_CHOICE_Pinwheel_Style=New Render Method,E_CHOICE_Pinwheel_3D=Sweep";
    }
    else {
        spdlog::warn("S5 conversion for {} not created yet.", et);
    }

    settings += GetLayerSettings();
    settings += GetSubBuffer();

    return settings;
}

LOREdit::LOREdit(pugi::xml_document& input_xml, int frequency) : _input_xml(input_xml), _frequency(frequency)
{

}

// gets a list of all the free timing tracks
std::vector<std::string> LOREdit::GetTimingTracks() const
{
    std::vector<std::string> res;

    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "TimingGrids") {
            for (pugi::xml_node timing = e.first_child(); timing; timing = timing.next_sibling()) {
                if (std::string_view(timing.name()) == "TimingGridFree") {
                    res.push_back(timing.attribute("name").as_string());
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
    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "TimingGrids") {
            for (pugi::xml_node timing = e.first_child(); timing; timing = timing.next_sibling()) {
                if (std::string_view(timing.name()) == "TimingGridFree") {
                    if (std::string_view(timing.attribute("name").as_string()) == timingTrackName) {
                        int lastMS = offset;
                        if (lastMS < 0) lastMS = 0;
                        for (pugi::xml_node t = timing.first_child(); t; t = t.next_sibling())
                        {
                            if (std::string_view(t.name()) == "timing")
                            {
                                int time = t.attribute("centisecond").as_int() * 10 + offset;
                                int adjTime = RoundToMultipleOfPeriod(time, _frequency);
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
std::map<int, std::string> LOREdit::GetModelStrands(const std::string& model) const
{
    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "PreviewClass") {
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                if (std::string_view(prop.name()) == "PropClass") {
                    if (std::string_view(prop.attribute("Name").as_string()) == model) {
                        std::string const grid = prop.attribute("ChannelGrid").as_string();
                        if(grid.empty()) return { { 1, "" } };
                        std::vector<std::string> strands = Split(grid, ';');
                        int strandCnts = 1;
                        std::map<int, std::string> strandMap;
                        for (std::string const& strand: strands) {
                            strandMap[strandCnts] = GetColor(strand);
                            strandCnts++;
                        }
                        return strandMap;
                    }
                }
            }
        }
    }
    return std::map<int, std::string>();
}

// Calculate the number of layers necessary for pixel effects on this model
// basically one per track * whether or not there are effects on left and right
std::string LOREdit::PropName(pugi::xml_node prop)
{
    std::string name = prop.attribute("name").as_string();
    if (name.empty()) {
        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
            if (std::string_view(ap.name()) == "PropClass") {
                name = ap.attribute("Name").as_string();
            }
        }
    }
    return name;
}

bool LOREdit::IsSubRowTrack(pugi::xml_node track)
{
    std::string_view type = track.attribute("type").as_string("none");
    return type == "custom" || type == "custom_horizontal_buffer";
}

std::vector<std::pair<pugi::xml_node, bool>> LOREdit::GetSourceLayers(const std::string& source) const
{
    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        std::string eName = e.name();
        if (eName != "SequenceProps" && eName != "ArchivedProps") {
            continue;
        }
        for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
            std::string propName = prop.name();
            if (propName != "SeqProp" && propName != "ArchiveProp") {
                continue;
            }
            std::string name = PropName(prop);
            if (name.empty()) {
                continue;
            }
            // prop names may themselves contain '/', so match the whole prop
            // name and then look for the row
            std::string row;
            if (source != name) {
                if (source.size() <= name.size() + 1 || !StartsWith(source, name + "/")) {
                    continue;
                }
                row = source.substr(name.size() + 1);
            }
            std::vector<pugi::xml_node> tracks;
            for (pugi::xml_node tc = prop.first_child(); tc; tc = tc.next_sibling()) {
                if (std::string_view(tc.name()) != "track" || !tc.first_child()) {
                    continue;
                }
                if (row.empty() ? IsSubRowTrack(tc) : (!IsSubRowTrack(tc) || row != tc.attribute("name").as_string())) {
                    continue;
                }
                tracks.push_back(tc);
            }
            if (!row.empty() && tracks.empty()) {
                continue; // a longer prop name may still match
            }
            // LOR draws later motion rows over earlier ones, so the last track
            // becomes xLights layer 0. Within a track the left side stays
            // above the right; the mix settings emitted for the left side
            // account for that.
            std::vector<std::pair<pugi::xml_node, bool>> res;
            for (auto it = tracks.rbegin(); it != tracks.rend(); ++it) {
                int l1 = 0;
                int l2 = 0;
                for (pugi::xml_node ef = it->first_child(); (l1 == 0 || l2 == 0) && ef; ef = ef.next_sibling()) {
                    int ll1, ll2;
                    GetLayers(ef.attribute("settings").as_string(), ll1, ll2);
                    if (ll1 == 1) l1 = 1;
                    if (ll2 == 1) l2 = 1;
                }
                if (l1 == 1) {
                    res.emplace_back(*it, true);
                }
                if (l2 == 1) {
                    res.emplace_back(*it, false);
                }
            }
            return res;
        }
    }
    return {};
}

// Calculate the number of layers necessary for pixel effects on this model
// basically one per track * whether or not there are effects on left and right
int LOREdit::GetModelLayers(const std::string& model) const
{
    return (int)GetSourceLayers(model).size();
}

int LOREdit::GetModelChannels(const std::string& model, int& rows, int& cols) const
{
    bool match = false;
    rows = 0;
    cols = 0;
    int count = 0;
    for (pugi::xml_node e = _input_xml.document_element().first_child(); e && !match; e = e.next_sibling()) {
        std::string eName = e.name();
        if (eName == "SequenceProps" || eName == "ArchivedProps") {
            // look for a match first
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string propName = prop.name();
                if (propName == "SeqProp" || propName == "ArchiveProp") {
                    std::string name = prop.attribute("name").as_string();
                    if (name == "") {
                        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                            if (std::string_view(ap.name()) == "PropClass") {
                                name = ap.attribute("Name").as_string();
                            }
                        }
                    }
                    if (name == model) {
                        for (pugi::xml_node tc = prop.first_child(); tc; tc = tc.next_sibling()) {
                            if (std::string_view(tc.name()) == "channel") {
                                if (tc.first_child()) {
                                    rows = std::max(rows, tc.attribute("row").as_int(0) + 1);
                                    cols = std::max(cols, tc.attribute("col").as_int(0) + 1);
                                    count++;
                                    match = true;
                                }
                            }
                        }
                        break;
                    }
                }
            }
            if (!match) {
                // no match so try a starts with
                for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                    std::string propName = prop.name();
                    if (propName == "SeqProp" || propName == "ArchiveProp") {
                        std::string name = prop.attribute("name").as_string();
                        if (name == "") {
                            for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                                if (std::string_view(ap.name()) == "PropClass") {
                                    name = ap.attribute("Name").as_string();
                                }
                            }
                        }
                        if (StartsWith(model, name)) {
                            for (pugi::xml_node tc = prop.first_child(); tc; tc = tc.next_sibling()) {
                                if (std::string_view(tc.name()) == "channel") {
                                    if (tc.first_child()) {
                                        rows = std::max(rows, tc.attribute("row").as_int(0) + 1);
                                        cols = std::max(cols, tc.attribute("col").as_int(0) + 1);
                                        count++;
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    // I only think I want to use count when nothing matched perfectly
    if (!match && count > 1 && rows == 1 && cols == 1) {
        cols = count;
    }

    return count;
}

// returns the type of sequencing on the named model
// assumes you cant have both channel and track sequencing on the same model ... this may not be true
loreditType LOREdit::GetSequencingType(const std::string& model) const
{
    if (IsFaceSource(model)) {
        return loreditType::NONE; // see MapS5Face
    }
    if (model.find('/') != std::string::npos && !GetSourceLayers(model).empty()) {
        return loreditType::TRACKS;
    }
    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        std::string eName = e.name();
        if (eName == "SequenceProps" || eName == "ArchivedProps") {
            // check first for exact name matches
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string propName = prop.name();
                if (propName == "SeqProp" || propName == "ArchiveProp") {
                    std::string name = prop.attribute("name").as_string();
                    if (name == "")
                    {
                        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                            if (std::string_view(ap.name()) == "PropClass")
                            {
                                name = ap.attribute("Name").as_string();
                            }
                        }
                    }
                    if (model == name)
                    {
                        for (pugi::xml_node tc = prop.first_child(); tc; tc = tc.next_sibling()) {
                            if (std::string_view(tc.name()) == "channel" && tc.first_child()) {
                                return loreditType::CHANNELS;
                            }
                            if (std::string_view(tc.name()) == "track" && tc.first_child())
                            {
                                return loreditType::TRACKS;
                            }
                        }
                    }
                }
            }
            // now check for starts with (for some decorated names)
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string propName = prop.name();
                if (propName == "SeqProp" || propName == "ArchiveProp") {
                    std::string name = prop.attribute("name").as_string();
                    if (name == "") {
                        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                            if (std::string_view(ap.name()) == "PropClass") {
                                name = ap.attribute("Name").as_string();
                            }
                        }
                    }
                    if (StartsWith(model, name)) {
                        for (pugi::xml_node tc = prop.first_child(); tc; tc = tc.next_sibling()) {
                            if (std::string_view(tc.name()) == "channel" && tc.first_child()) {
                                return loreditType::CHANNELS;
                            }
                            if (std::string_view(tc.name()) == "track" && tc.first_child()) {
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
    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        std::string eName = e.name();
        if (eName == "SequenceProps" || eName == "ArchivedProps") {
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string propName = prop.name();
                if (propName == "SeqProp" || propName == "ArchiveProp") {
                    std::string name = PropName(prop);
                    if (name.empty()) {
                        continue;
                    }
                    bool whole = false;
                    std::vector<std::string> rows;
                    for (pugi::xml_node tc = prop.first_child(); tc; tc = tc.next_sibling()) {
                        std::string tcName = tc.name();
                        if (!tc.first_child() || (tcName != "channel" && tcName != "track")) {
                            continue;
                        }
                        if (tcName == "track" && IsSubRowTrack(tc)) {
                            std::string row = name + "/" + tc.attribute("name").as_string();
                            if (std::find(rows.begin(), rows.end(), row) == rows.end()) {
                                rows.push_back(row);
                            }
                        } else {
                            whole = true;
                        }
                    }
                    if (whole) {
                        res.push_back(name);
                    }
                    res.insert(res.end(), rows.begin(), rows.end());
                }
            }
        }
    }
    auto faces = GetFaceSources();
    res.insert(res.end(), faces.begin(), faces.end());

    return res;
}

namespace {
    const std::string kFaceSuffix = " (Singing Face)";

    // "<face> Mouth <shape>"
    bool SplitMouthName(const std::string& name, std::string& face, std::string& shape) {
        auto pos = name.rfind(" Mouth ");
        if (pos == std::string::npos || pos == 0) {
            return false;
        }
        face = name.substr(0, pos);
        shape = name.substr(pos + 7);
        return !shape.empty();
    }
}

std::string LOREdit::MouthShapeToPhoneme(const std::string& shapeName)
{
    std::string shape = Lower(Trim(shapeName));
    Replace(shape, "\"", "");
    // "AI (Full Open)", "E (Half Open)", "L(th)"
    auto paren = shape.find('(');
    std::string base = Trim(paren == std::string::npos ? shape : shape.substr(0, paren));
    if (base == "closed" || base == "rest") return "rest";
    if (base == "ai" || base == "full open" || base == "ah") return "AI";
    if (base == "e" || base == "half open") return "E";
    if (base == "o" || base == "oh" || base == "ou") return "O";
    if (base == "u") return "U";
    if (base == "wq") return "WQ";
    if (base == "l") return "L";
    if (base == "fv") return "FV";
    if (base == "mbp") return "MBP";
    if (base == "etc") return "etc";
    return "";
}

void LOREdit::ScanFaces() const
{
    if (_facesScanned) {
        return;
    }
    _facesScanned = true;
    std::map<std::string, Face> faces;
    std::vector<std::string> order;
    auto faceFor = [&](const std::string& name) -> Face& {
        auto it = faces.find(name);
        if (it == faces.end()) {
            order.push_back(name);
            it = faces.emplace(name, Face()).first;
            it->second.name = name;
        }
        return it->second;
    };
    auto addEffect = [&](Face& f, pugi::xml_node ef, const std::string& phoneme) {
        LORLipSyncMark m;
        m.startMS = ef.attribute("startCentisecond").as_uint();
        m.endMS = ef.attribute("endCentisecond").as_uint();
        m.label = phoneme;
        if (m.endMS > m.startMS) {
            f.shapes.push_back(m);
        }
    };

    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) != "SequenceProps") {
            continue;
        }
        for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
            if (std::string_view(prop.name()) != "SeqProp") {
                continue;
            }
            std::string name = PropName(prop);
            std::string faceName, shape;
            // a whole channel prop per mouth shape
            if (SplitMouthName(name, faceName, shape)) {
                std::string phoneme = MouthShapeToPhoneme(shape);
                if (!phoneme.empty()) {
                    for (pugi::xml_node ch = prop.child("channel"); ch; ch = ch.next_sibling("channel")) {
                        for (pugi::xml_node ef = ch.first_child(); ef; ef = ef.next_sibling()) {
                            Face& f = faceFor(faceName);
                            if (!f.colourSet && phoneme != "rest") {
                                // RGB props store the colour as a negative intensity
                                int si = ef.attribute("intensity").as_int(ef.attribute("startIntensity").as_int(0));
                                if (si < 0) {
                                    f.colour = xlColor((si & 0xFF0000) >> 16, (si & 0xFF00) >> 8, si & 0xFF);
                                    f.colourSet = true;
                                }
                            }
                            addEffect(f, ef, phoneme);
                        }
                    }
                }
                continue;
            }
            // or a motion row per mouth shape on one prop (matrix faces)
            for (pugi::xml_node tc = prop.child("track"); tc; tc = tc.next_sibling("track")) {
                if (!SplitMouthName(tc.attribute("name").as_string(), faceName, shape)) {
                    continue;
                }
                std::string phoneme = MouthShapeToPhoneme(shape);
                if (phoneme.empty() || !tc.first_child()) {
                    continue;
                }
                Face& f = faceFor(name + "/" + faceName);
                for (pugi::xml_node ef = tc.first_child(); ef; ef = ef.next_sibling()) {
                    addEffect(f, ef, phoneme);
                }
            }
        }
    }

    for (const auto& n : order) {
        Face f = std::move(faces[n]);
        std::sort(f.shapes.begin(), f.shapes.end(), [](const LORLipSyncMark& a, const LORLipSyncMark& b) { return a.startMS < b.startMS; });
        // drop duplicates (two sides / overlapping rows) and merge runs of a shape
        std::vector<LORLipSyncMark> merged;
        for (const auto& m : f.shapes) {
            if (!merged.empty() && merged.back().label == m.label && m.startMS <= merged.back().endMS) {
                merged.back().endMS = std::max(merged.back().endMS, m.endMS);
            } else if (!merged.empty() && m.startMS < merged.back().endMS) {
                merged.back().endMS = m.startMS;
                merged.push_back(m);
            } else {
                merged.push_back(m);
            }
        }
        f.shapes = std::move(merged);
        int spoken = 0;
        for (const auto& m : f.shapes) {
            if (m.label != "rest") spoken++;
        }
        if (spoken == 0) {
            continue; // never sings in this sequence
        }
        _faces.push_back(std::move(f));
    }

    // Faces singing the same part share a timing track: nearly every shape
    // change of the coarser face (a 4-shape face, a backing face differing by
    // a few frames) is also a change of the other. The track is built from the
    // face with the most distinct shapes and named after it.
    auto changeTimes = [](const Face& f) {
        std::set<uint32_t> t;
        for (const auto& m : f.shapes) {
            if (m.label != "rest") t.insert(m.startMS);
        }
        return t;
    };
    auto vocabulary = [](const Face& f) {
        std::set<std::string> v;
        for (const auto& m : f.shapes) v.insert(m.label);
        return v.size();
    };
    std::vector<std::set<uint32_t>> times;
    std::vector<size_t> byRichness(_faces.size());
    for (size_t i = 0; i < _faces.size(); ++i) {
        times.push_back(changeTimes(_faces[i]));
        byRichness[i] = i;
    }
    // richest faces first, so each group is represented by its most detailed face
    std::stable_sort(byRichness.begin(), byRichness.end(), [&](size_t a, size_t b) {
        size_t va = vocabulary(_faces[a]), vb = vocabulary(_faces[b]);
        return va != vb ? va > vb : times[a].size() > times[b].size();
    });
    std::vector<size_t> reps;
    for (size_t i : byRichness) {
        size_t chosen = i;
        for (size_t r : reps) {
            const auto& t = times[i];
            size_t common = 0;
            for (auto x : t) {
                common += times[r].count(x);
            }
            if (!t.empty() && common * 100 >= t.size() * 95) {
                chosen = r;
                break;
            }
        }
        if (chosen == i) {
            reps.push_back(i);
            std::string trackName = _faces[i].name;
            Replace(trackName, "/", " - ");
            _faces[i].trackName = "Lip Sync " + trackName;
        }
        _faces[i].trackSource = chosen;
        _faces[i].trackName = _faces[chosen].trackName;
    }
}

const LOREdit::Face* LOREdit::FindFace(const std::string& faceSource) const
{
    ScanFaces();
    std::string name = EndsWith(faceSource, kFaceSuffix) ? faceSource.substr(0, faceSource.size() - kFaceSuffix.size()) : faceSource;
    for (const auto& f : _faces) {
        if (f.name == name) {
            return &f;
        }
    }
    return nullptr;
}

bool LOREdit::IsFaceSource(const std::string& source)
{
    return EndsWith(source, kFaceSuffix);
}

std::vector<std::string> LOREdit::GetFaceSources() const
{
    ScanFaces();
    std::vector<std::string> res;
    for (const auto& f : _faces) {
        res.push_back(f.name + kFaceSuffix);
    }
    return res;
}

std::vector<std::string> LOREdit::GetLipSyncTracks() const
{
    ScanFaces();
    std::vector<std::string> res;
    for (const auto& f : _faces) {
        if (std::find(res.begin(), res.end(), f.trackName) == res.end()) {
            res.push_back(f.trackName);
        }
    }
    return res;
}

bool LOREdit::IsLipSyncTrack(const std::string& name) const
{
    auto tracks = GetLipSyncTracks();
    return std::find(tracks.begin(), tracks.end(), name) != tracks.end();
}

std::string LOREdit::GetLipSyncTrackForFace(const std::string& faceSource) const
{
    const Face* f = FindFace(faceSource);
    return f == nullptr ? std::string() : f->trackName;
}

std::vector<std::vector<LORLipSyncMark>> LOREdit::GetLipSync(const std::string& trackName, int offset) const
{
    ScanFaces();
    std::vector<std::vector<LORLipSyncMark>> layers(3);
    const Face* face = nullptr;
    for (const auto& f : _faces) {
        if (f.trackName == trackName) {
            face = &_faces[f.trackSource];
            break;
        }
    }
    if (face == nullptr) {
        return layers;
    }
    auto toMS = [&](uint32_t cs) {
        int t = (int)cs * 10 + offset;
        return (uint32_t)std::max(0, RoundToMultipleOfPeriod(t, _frequency));
    };
    // phonemes; a closed mouth is the Faces effect's default between them
    for (const auto& m : face->shapes) {
        if (m.label == "rest") {
            continue;
        }
        uint32_t s = toMS(m.startMS);
        uint32_t e = toMS(m.endMS);
        if (!layers[2].empty() && s < layers[2].back().endMS) {
            s = layers[2].back().endMS;
        }
        if (e <= s) {
            continue;
        }
        if (!layers[2].empty() && layers[2].back().label == m.label && layers[2].back().endMS == s) {
            layers[2].back().endMS = e;
        } else {
            layers[2].push_back({ s, e, m.label });
        }
    }
    // phrases: runs of phonemes split where the mouth rests for 300ms or more
    constexpr uint32_t kPhraseGapMS = 300;
    for (const auto& p : layers[2]) {
        if (layers[0].empty() || p.startMS >= layers[0].back().endMS + kPhraseGapMS) {
            layers[0].push_back({ p.startMS, p.endMS, "" });
        } else {
            layers[0].back().endMS = p.endMS;
        }
    }
    layers[1] = layers[0];
    return layers;
}

bool LOREdit::GetFaceSpan(const std::string& faceSource, int offset, uint32_t& startMS, uint32_t& endMS) const
{
    const Face* f = FindFace(faceSource);
    if (f == nullptr || f->shapes.empty()) {
        return false;
    }
    auto toMS = [&](uint32_t cs) {
        int t = (int)cs * 10 + offset;
        return (uint32_t)std::max(0, RoundToMultipleOfPeriod(t, _frequency));
    };
    startMS = toMS(f->shapes.front().startMS);
    endMS = startMS;
    for (const auto& m : f->shapes) {
        endMS = std::max(endMS, toMS(m.endMS));
    }
    return endMS > startMS;
}

xlColor LOREdit::GetFaceColour(const std::string& faceSource) const
{
    const Face* f = FindFace(faceSource);
    return f == nullptr ? xlWHITE : f->colour;
}

std::vector<std::string> LOREdit::GetNodesWithEffects() const
{
    std::string lastName;
    int standIndex = 1;;
    std::vector<std::string> res;
    std::map<int, std::string> strands;
    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        std::string eName = e.name();
        if (eName == "SequenceProps" || eName == "ArchivedProps") {
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string propName = prop.name();
                if (propName == "SeqProp" || propName == "ArchiveProp") {
                    std::string name = prop.attribute("name").as_string();
                    if (name == "") {
                        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                            if (std::string_view(ap.name()) == "PropClass") {
                                name = ap.attribute("Name").as_string();
                            }
                        }
                    }
                    if (name != "") {
                        for (pugi::xml_node tc = prop.first_child(); tc; tc = tc.next_sibling()) {
                            if (std::string_view(tc.name()) == "channel" && tc.first_child()) {
                                if (lastName != name) {
                                    standIndex = 0;
                                    strands = GetModelStrands(name);
                                    lastName = name;
                                }
                                standIndex++;
                                {
                                    int row = tc.attribute("row").as_int(0);
                                    int col = tc.attribute("col").as_int(0);
                                    int colour = tc.attribute("color").as_int(0);
                                    res.push_back(name + "[" + std::to_string(row) + "," + std::to_string(col) + ","
                                        + std::to_string(colour) + "][" + strands[standIndex] + "]");
                                }
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
    auto ss = Split(settings, '|');

    if (ss.size() >= 7)
    {
        if (StartsWith(ss[5], "lightorama_") && !StartsWith(ss[5], "lightorama_none"))
        {
            ll1 = 1;
        }
        if (StartsWith(ss[6], "lightorama_") && !StartsWith(ss[6], "lightorama_none"))
        {
            ll2 = 1;
        }
    }
}

std::vector<LOREditEffect> LOREdit::AddEffects(pugi::xml_node track, bool left, int offset) const
{
    std::vector<LOREditEffect> res;

    std::string trackType = track.attribute("type").as_string("none");
    float subx = track.attribute("subx").as_float(0.0f);
    float suby = track.attribute("suby").as_float(0.0f);
    float subw = track.attribute("subw").as_float(1.0f);
    float subh = track.attribute("subh").as_float(1.0f);

    for (pugi::xml_node ef = track.first_child(); ef; ef = ef.next_sibling()) {
        LOREditEffect effect;
        effect.left = left;
        effect.trackType = trackType;
        effect.subx = subx;
        effect.suby = suby;
        effect.subw = subw;
        effect.subh = subh;
        effect.startMS = ef.attribute("startCentisecond").as_int() * 10 + offset;
        effect.endMS = ef.attribute("endCentisecond").as_int() * 10 + offset;
        int si = ef.attribute("intensity").as_int(9999);
        if (si != 9999)
        {
            effect.startColour = xlWHITE;
            effect.endColour = xlWHITE;
            effect.startIntensity = si;
            effect.endIntensity = si;
        }
        else
        {
            effect.startIntensity = ef.attribute("startIntensity").as_int(100);
            effect.endIntensity = ef.attribute("endIntensity").as_int(100);
            effect.startColour = xlWHITE;
            effect.endColour = xlWHITE;
        }
        effect.type = loreditType::TRACKS;

        std::string s = ef.attribute("settings").as_string();
        auto ss = Split(s, '|');

        if (ss.size() >= 7)
        {
            std::string es;
            if (left)
            {
                es = ss[5];
            }
            else
            {
                es = ss[6];
            }
            std::string const& other = left ? ss[6] : ss[5];
            effect.otherSidePresent = StartsWith(other, "lightorama_") && !StartsWith(other, "lightorama_none");
            auto ees = Split(es, ':');

            if (ees.size() > 0)
            {
                if (StartsWith(ees[0], "lightorama_"))
                {
                    if (!StartsWith(ees[0], "lightorama_none"))
                    {
                        effect.effectType = AfterFirst(ees[0], '_');
                        for (auto it2 : ees)
                        {
                            if (!StartsWith(it2, "lightorama_"))
                            {
                                effect.effectSettings.push_back(it2);
                            }
                        }
                    }
                }
            }
            effect.otherSettings.push_back(ss[0]); // blend
            effect.otherSettings.push_back(ss[1]);
            effect.otherSettings.push_back(ss[2]);
            effect.otherSettings.push_back(ss[3]);
            effect.otherSettings.push_back(ss[4]);
            if (ss.size() == 8) {
                effect.otherSettings.push_back(ss[7]);
            }
            else                 {
                effect.otherSettings.push_back("");
            }
        }

        res.push_back(effect);
    }

    return res;
}

std::vector<LOREditEffect> LOREdit::GetTrackEffects(const std::string& model, int layer, int offset) const
{
    auto layers = GetSourceLayers(model);
    if (layer < 0 || layer >= (int)layers.size()) {
        return {};
    }
    return AddEffects(layers[layer].first, layers[layer].second, offset);
}

std::vector<LOREditEffect> LOREdit::GetChannelEffectsForNode(int targetRow, int targetCol, int targetColor, pugi::xml_node prop, int offset) const
{
    std::vector<LOREditEffect> res;

    for (pugi::xml_node tc = prop.first_child(); tc; tc = tc.next_sibling()) {
        if (std::string_view(tc.name()) == "channel") {
            int row = tc.attribute("row").as_int(0);
            int col = tc.attribute("col").as_int(0);
            int colour = tc.attribute("color").as_int(0);

            if ((targetRow == -1 && targetCol == -1 && targetColor == -1) || // map regardless
                (row == targetRow && col == targetCol && targetColor == -1) || // map because the node matches
                (row == 0 && col == 0 && colour == targetCol && targetRow == 0 && targetColor == -1) ||
                (row == targetRow && col == targetCol && colour == targetColor)) {//match stand/color
                for (pugi::xml_node ef = tc.first_child(); ef; ef = ef.next_sibling()) {
                    LOREditEffect effect;
                    effect.pixelChannels = std::string_view(prop.attribute("EnablePixelChannels").as_string("0")) == "1";
                    effect.startMS = ef.attribute("startCentisecond").as_int() * 10 + offset;
                    effect.endMS = ef.attribute("endCentisecond").as_int() * 10 + offset;
                    int si = ef.attribute("intensity").as_int(9999);
                    if (si != 9999) {
                        if (si < 0) {
                            if (std::string_view(ef.attribute("settings").as_string()) == "DMX_INTENSITY") {
                                effect.startIntensity = 255;
                                effect.endIntensity = 255;
                            }
                            else {
                                effect.startIntensity = 100;
                                effect.endIntensity = 100;
                            }
                            effect.startColour = xlColor((si & 0xFF0000) >> 16, (si & 0xFF00) >> 8, si & 0xFF);
                            effect.endColour = effect.startColour;
                        }
                        else {
                            effect.startIntensity = si;
                            effect.endIntensity = si;
                            effect.startColour = xlWHITE;
                            effect.endColour = xlWHITE;
                        }
                    }
                    else {
                        si = ef.attribute("startIntensity").as_int(9999);
                        if (si != 9999) {
                            if (si < 0) {
                                if (std::string_view(ef.attribute("settings").as_string()) == "DMX_INTENSITY") {
                                    effect.startIntensity = 255;
                                    effect.endIntensity = 255;
                                }
                                else {
                                    effect.startIntensity = 100;
                                    effect.endIntensity = 100;
                                }
                                effect.startColour = xlColor((si & 0xFF0000) >> 16, (si & 0xFF00) >> 8, si & 0xFF);
                                int ei = ef.attribute("endIntensity").as_int(-1);
                                effect.endColour = xlColor((ei & 0xFF0000) >> 16, (ei & 0xFF00) >> 8, ei & 0xFF);
                            }
                            else {
                                effect.startIntensity = si;
                                effect.endIntensity = ef.attribute("endIntensity").as_int(100);
                                effect.startColour = xlWHITE;
                                effect.endColour = xlWHITE;
                            }
                        }
                    }
                    effect.type = loreditType::CHANNELS;
                    effect.effectType = ef.attribute("settings").as_string();
                    res.push_back(effect);
                }
                return res;
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
        return res;
    }

    int rows = 0;
    int cols = 0;
    int channels = GetModelChannels(model, rows, cols);

    if (channel >= channels && channels > 1)
    {
        // Tried to map a model with less channels than the target model so stop once we run out
        return res;
        //channel = channels - 1;
    }

    int mw = m->GetDefaultBufferWi();
    int mh = m->GetDefaultBufferHt();

    std::vector<xlPoint> coords;
    m->GetNodeCoords(channel, coords);
    int bufx = -1;
    int bufy = -1;
    if (channels != 1) {
        (void)mw;
        (void)mh;

        if (coords.size() != 0) {
            bufx = coords[0].x;
            bufy = coords[0].y;
        }
        else {
            // this is not encouraging
        }
    }

    int targetRow = bufy;
    int targetCol = bufx;

    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        std::string eName = e.name();
        if (eName == "SequenceProps" || eName == "ArchivedProps") {
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string propName = prop.name();
                if (propName == "SeqProp" || propName == "ArchiveProp") {
                    std::string name = prop.attribute("name").as_string();
                    if (name == "")
                    {
                        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                            if (std::string_view(ap.name()) == "PropClass")
                            {
                                name = ap.attribute("Name").as_string();
                            }
                        }
                    }
                    if (name == model)
                    {
                        res = GetChannelEffectsForNode(targetRow, targetCol, -1, prop, offset);
                        if (res.size() != 0) {
                            return res;
                        }

                        // if we got here and the source only has one node just map it regardless
                        if (rows == 1 && cols == 1)
                        {
                            return GetChannelEffectsForNode(-1, -1, -1, prop, offset);
                        }

                        // still no match

                        return res;
                    }
                }
            }
        }
    }

    return res;
}

std::vector<LOREditEffect> LOREdit::GetChannelEffects(const std::string& model, int targetRow, int targetCol, int targetColor, int offset) const
{
    std::vector<LOREditEffect> res;
    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        std::string eName = e.name();
        if (eName == "SequenceProps" || eName == "ArchivedProps") {
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string propName = prop.name();
                if (propName == "SeqProp" || propName == "ArchiveProp") {
                    std::string name = prop.attribute("name").as_string();
                    if (name == "") {
                        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                            if (std::string_view(ap.name()) == "PropClass") {
                                name = ap.attribute("Name").as_string();
                            }
                        }
                    }
                    if (name == model) {
                        res = GetChannelEffectsForNode(targetRow, targetCol, targetColor, prop, offset);
                        if (res.size() != 0) {
                            return res;
                        }
                        return res;
                    }
                }
            }
        }
    }

    return res;
}

std::vector<LOREditEffect> LOREdit::GetChannelEffects(const std::string& model, int channel, int nodes, int offset) const
{
    std::vector<LOREditEffect> res;

    int rows = 0;
    int cols = 0;
    int channels = GetModelChannels(model, rows, cols);

    if (channel >= channels)
    {
        channel = channels - 1;
    }

    int targetRow = 0;
    int targetCol = 0;

    if (rows > 1)
    {
        targetRow = channel;
        if (channel >= rows) return res;
    }
    else
    {
        targetCol = channel;
        if (channel >= cols) return res;
    }

    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        std::string eName = e.name();
        if (eName == "SequenceProps" || eName == "ArchivedProps") {
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string propName = prop.name();
                if (propName == "SeqProp" || propName == "ArchiveProp") {
                    std::string name = prop.attribute("name").as_string();
                    if (name == "")
                    {
                        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                            if (std::string_view(ap.name()) == "PropClass")
                            {
                                name = ap.attribute("Name").as_string();
                            }
                        }
                    }
                    if (name == model)
                    {
                        for (pugi::xml_node tc = prop.first_child(); tc; tc = tc.next_sibling()) {
                            if (std::string_view(tc.name()) == "channel") {
                                int row = tc.attribute("row").as_int(0);
                                int col = tc.attribute("col").as_int(0);

                                if (row == targetRow && col == targetCol)
                                {
                                    for (pugi::xml_node ef = tc.first_child(); ef; ef = ef.next_sibling()) {
                                        LOREditEffect effect;
                                        effect.pixelChannels = std::string_view(prop.attribute("EnablePixelChannels").as_string("0")) == "1";
                                        effect.startMS = ef.attribute("startCentisecond").as_int() * 10 + offset;
                                        effect.endMS = ef.attribute("endCentisecond").as_int() * 10 + offset;
                                        int si = ef.attribute("intensity").as_int(9999);
                                        if (si != 9999)
                                        {
                                            if (si < 0)
                                            {
                                                if (std::string_view(ef.attribute("settings").as_string()) == "DMX_INTENSITY")
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
                                            si = ef.attribute("startIntensity").as_int(9999);
                                            if (si != 9999)
                                            {
                                                if (si < 0)
                                                {
                                                    if (std::string_view(ef.attribute("settings").as_string()) == "DMX_INTENSITY")
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
                                                    int ei = ef.attribute("endIntensity").as_int(-1);
                                                    effect.endColour = xlColor((ei & 0xFF0000) >> 16, (ei & 0xFF00) >> 8, ei & 0xFF);
                                                }
                                                else
                                                {
                                                    effect.startIntensity = si;
                                                    effect.endIntensity = ef.attribute("endIntensity").as_int(100);
                                                    effect.startColour = xlWHITE;
                                                    effect.endColour = xlWHITE;
                                                }
                                            }
                                        }
                                        effect.type = loreditType::CHANNELS;
                                        effect.effectType = ef.attribute("settings").as_string();
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

std::map<std::string, std::string> LOREdit::GetEmbeddedPictures() const
{
    std::map<std::string, std::string> res;
    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "pictures") {
            for (pugi::xml_node p = e.first_child(); p; p = p.next_sibling()) {
                if (std::string_view(p.name()) == "picture") {
                    std::string data = p.text().as_string();
                    data.erase(std::remove_if(data.begin(), data.end(), [](char c) { return std::isspace((unsigned char)c); }), data.end());
                    res[p.attribute("name").as_string()] = data;
                }
            }
        }
    }
    return res;
}

std::string LOREdit::EmbeddedPictureName(const std::string& lorName)
{
    // "*guitar*f526c123-50dc-48ea-8238-5c2a4855f32e*.png" -> "guitar-f526c123.png"
    auto parts = Split(lorName, '*');
    std::vector<std::string> nonEmpty;
    for (auto& p : parts) {
        if (!p.empty()) nonEmpty.push_back(p);
    }
    if (nonEmpty.size() < 3) {
        std::string res = lorName;
        Replace(res, "*", "");
        return res;
    }
    std::string ext = nonEmpty.back();
    std::string guid = nonEmpty[nonEmpty.size() - 2];
    std::string stem;
    for (size_t i = 0; i + 2 < nonEmpty.size(); ++i) {
        stem += nonEmpty[i];
    }
    return stem + "-" + guid.substr(0, 8) + ext;
}

std::string LOREdit::GetColor(const std::string& settings)
{
    std::vector<std::string> const savedUploadItems = Split(settings, ',');
    if (savedUploadItems.size() == 6)
        return savedUploadItems[5];
    return "";
}

bool LOREdit::IsNodeStrandMapping(const std::string& mapping)
{
    static const std::regex regex("\\[([0-9]+),([0-9]+),([0-9]+)\\]");
    return std::regex_search(mapping, regex);
}

void LOREdit::setNodeColor(const std::string& color, LOREditEffect & effect)
{
    if (color.empty())
        return;

    if (color.find("Multi") != std::string::npos)
        return;

    effect.startColour.SetFromString(color);
    effect.endColour.SetFromString(color);
}
