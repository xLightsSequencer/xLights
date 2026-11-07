/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "Vixen3.h"

#include <list>
#include <cstdlib>
#include <cmath>
#include <filesystem>
#include <algorithm>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include "utils/string_utils.h"
#include "utils/ExternalHooks.h"

double CorrectForGamma(double value)
{
    if (value > 0.0031308)
        return 1.055 * pow(value, 1.0 / 2.4) - 0.055;
    else
        return 12.92 * value;
}

xlColor ConvertXYZToColour(double x, double y, double z)
{
    return xlColor((uint8_t)std::max(0.0, std::min(255.0, 255.0 * CorrectForGamma(x * +0.032406 + y * -0.015372 + z * -0.004986))),
                   (uint8_t)std::max(0.0, std::min(255.0, 255.0 * CorrectForGamma(x * -0.009689 + y * +0.018758 + z * +0.000415))),
                   (uint8_t)std::max(0.0, std::min(255.0, 255.0 * CorrectForGamma(x * +0.000557 + y * -0.002040 + z * +0.010570)))
                  );
}

static float ParseVixenDuration(std::string t)
{
    if (StartsWith(t, "PT")) {
        t = AfterFirst(t, 'T');
    }
    float hours = 0;
    if (Contains(t, "H")) {
        hours = std::strtod(BeforeFirst(t, 'H').c_str(), nullptr);
        t = AfterFirst(t, 'H');
    }
    float mins = 0;
    if (Contains(t, "M")) {
        mins = std::strtod(BeforeFirst(t, 'M').c_str(), nullptr);
        t = AfterFirst(t, 'M');
    }
    float secs = 0;
    if (EndsWith(t, "S")) {
        secs = std::strtod(BeforeLast(t, 'S').c_str(), nullptr);
    }
    return hours * 3600.0 + mins * 60.0 + secs;
}

std::string BoolToIntStr (std::string str)
{
    if (str == "true")
        return "1";
    else
        return "0";
}

std::string VixenEffect::GetPalette() const
{
    std::string res;

    for (int i = 0; i < (int)palette.size(); i++) {
        std::string n = std::to_string(i + 1);

        if (res != "")
            res += ",";

        if (palette[i].size() == 1) {
            res += "C_BUTTON_Palette" + n + "=#" + fmt::format("{:02x}{:02x}{:02x}",
                    (int)palette[i][0].color.Red(), (int)palette[i][0].color.Green(), (int)palette[i][0].color.Blue());
            res += ",C_CHECKBOX_Palette" + n + "=1";
        // Break buttefly gradient into individual colors
        } else if (type == "ButterflyData") {
            for (int ii = 0; ii < (int)palette[i].size(); ii++) {
                std::string nn = std::to_string(ii + 1);
                if (res != "")
                    res += ",";
                res += "C_BUTTON_Palette" + nn + "=#" + fmt::format("{:02x}{:02x}{:02x}", (int)palette[i][ii].color.Red(), (int)palette[i][ii].color.Green(), (int)palette[i][ii].color.Blue());
                res += ",C_CHECKBOX_Palette" + nn + "=1";
            }
        } else {
            res += "C_BUTTON_Palette" + n + "=Active=TRUE|Id=ID_BUTTON_Palette" + n + "|Values=";
            for (int ii = 0; ii < (int)palette[i].size(); ii++) {
                res += fmt::format("x={:.3f}^c=#{:02x}{:02x}{:02x}",
                       palette[i][ii].position, (int)palette[i][ii].color.Red(), (int)palette[i][ii].color.Green(), (int)palette[i][ii].color.Blue());
                if (ii != (int)palette[i].size() - 1)
                    res += ";";
            }
            res += "|,C_CHECKBOX_Palette" + n + "=1";
        }

    }

    // More than two points for the Vixen level curve, generate a custom brightness value curve
    if (type == "PulseData" && levelCurve.size() > 2) {
        res += ",C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Custom|Min=0.00|Max=400.00|RV=TRUE|Values=";
        for (int i = 0; i < (int)levelCurve.size(); i++) {
            res += fmt::format("{:.2f}:", levelCurve[i].x / 100) + fmt::format("{:.2f}", levelCurve[i].y / 100 / 4);
            if (i != (int)levelCurve.size() - 1)
                res += ";";
        }
    }

    return res;
}

std::string VixenEffect::Setting(const std::string& key) const
{
    auto it = settings.find(key);
    return it == settings.end() ? std::string() : it->second;
}

std::string VixenEffect::GetSettings() const
{
    std::string res;
    if (type == "AlternatingData") {
        res = "B_CHOICE_BufferStyle=Single Line,E_SLIDER_Marquee_Band_Size=" + Setting("GroupLevel");
    }
    else if (type == "AudioData") {}
    else if (type == "BarsData") {
        // Up/Down and Left/Right are backwards in V3
        auto s = Setting("Direction");
        std::string dir = "down";
        if (s == "Moves Down") dir = "up";
        else if (s == "Expands") dir = "expand";
        else if (s == "Compresses") dir = "compress";
        else if (s == "Moves Left") dir = "Right";
        else if (s == "Moves Right") dir = "Left";
        else if (s == "Horizontal Expand") dir = "H-expand";
        else if (s == "Horizontal Compress") dir = "H-compress";
        else if (s == "Alternate Up") dir = "Alternate Down";
        else if (s == "Alternate Down") dir = "Alternate Up";
        else if (s == "Alternate Left") dir = "Alternate Right";
        else if (s == "Alternate Right") dir = "Alternate Left";

        if (Setting("TargetPositioning") == "Strings") {
            if (Setting("Orientation") == "Vertical") {
                res = "B_CHOICE_BufferStyle=Horizontal Per Model/Strand,";
            } else {
                res = "B_CHOICE_BufferStyle=Vertical Per Model/Strand,";
            }
        }

        res += "E_CHECKBOX_Bars_3D=" + Setting("Show3D") +
               ",E_CHECKBOX_Bars_Highlight=" + Setting("Highlight") +
               ",E_CHOICE_Bars_Direction=" + dir +
               ",E_SLIDER_Bars_BarCount=" + std::to_string(std::min((int)std::strtol(Setting("Repeat").c_str(), nullptr, 10), 5)) +
               ",E_TEXTCTRL_Bars_Cycles=" + Setting("Speed");
    }
    else if (type == "ButterflyData") {
        res = "E_SLIDER_Butterfly_Chunks=" + Setting("BackgroundChunks") +
              ",E_SLIDER_Butterfly_Skip=" + Setting("BackgroundSkips") +
              ",E_SLIDER_Butterfly_Style=" + AfterFirst(Setting("ButterflyType"), 'e') +
              ",E_CHOICE_Butterfly_Direction=" + (Setting("Direction") == "Forward" ? "0" : "1") +
              ",E_CHOICE_Butterfly_Colors=" + (Setting("ColorScheme") == "Gradient" ? "Palette" : "Rainbow");
    }
    else if (type == "ChaseData") {}
    else if (type == "CirclesData") {}
    else if (type == "Data") {}
    else if (type == "FireData") {}
    else if (type == "GarlandsData") {}
    else if (type == "MeteorsData") {}
    else if (type == "PictureData") {}
    else if (type == "PinWheelData") {}
    else if (type == "PlasmaData") {}
    else if (type == "PulseData") {
        // Only two points, set them as start and end intensity.  If more than two, a brightness curve will be generated
        if (levelCurve.size() == 2) {
            res = "E_TEXTCTRL_Eff_On_Start=" + std::to_string((int)std::lround(levelCurve[0].y)) +
                  ",E_TEXTCTRL_Eff_On_End=" + std::to_string((int)std::lround(levelCurve[1].y));
        }
    }
    else if (type == "TwinkleData") {
        res = "E_SLIDER_Twinkle_Steps=" + Setting("AveragePulseTime") +
              ",E_SLIDER_Twinkle_Count=" + Setting("AverageCoverage");
    }
    else if (type == "SetLevelData") {}
    else if (type == "SpinData") {}
    else if (type == "SpiralData") {}
    else if (type == "SpirographData") {}
    else if (type == "TextData") {}
    else if (type == "VUMeterData") {
        res = "E_CHOICE_VUMeter_Type=On";
    }

    else if (type == "WipeData") {
        auto d = Setting("Direction");
        auto r = Setting("ReverseDirection");
        std::string chase = "From Middle";
        std::string transform = "";

        if (d == "Vertical") {
            chase = "Left-Right";
            if (r == "true")
                transform = "Rotate CC 90";
            else
                transform = "Rotate CW 90";
        } else if (d == "Horizontal") {
            if (r == "true")
                chase = "Right-Left";
            else
                chase = "Left-Right";
        } else {
            chase = "From Middle";
        }

        res = "E_CHOICE_Chase_Type1=" + chase;
        if (transform != "")
            res = res + ",B_CHOICE_BufferTransform=" + transform;

        if (Setting("WipeMovement") == "Count")
            res = res + ",E_TEXTCTRL_Chase_Rotations=" + Setting("PassCount");
    }
    else if (type == "NutcrackerModuleData") {
        std::string nc = Setting("CurrentEffect");
        std::string fit = BoolToIntStr(Setting("FitToTime"));  // Not sure this can be mapped
        int speed = (int)std::strtol(Setting("Speed").c_str(), nullptr, 10); // Max of 20

        // V3 Nutcracker wasn't spatially aware so switch render style
        if (Setting("StringOrienation") == "Vertical") {
            res = "B_CHOICE_BufferStyle=Horizontal Per Model/Strand";
        } else {
            res = "B_CHOICE_BufferStyle=Vertical Per Model/Strand";
        }

        if (nc == "Bars") {
            int i = (int)std::strtol(Setting("Bars_Direction").c_str(), nullptr, 10);
            std::string dir = "up";
            if (i == 1) dir = "down";
            else if (i == 2) dir = "expand";
            else if (i == 3) dir = "compress";
            else if (i == 4) dir = "Left";
            else if (i == 5) dir = "Right";
            else if (i == 6) dir = "H-expand";
            else if (i == 7) dir = "H-compress";
            else if (i == 8) dir = "Alternate Up";
            else if (i == 9) dir = "Alternate Down";
            else if (i == 10) dir = "Alternate Left";
            else if (i == 11) dir = "Alternate Right";
            res += ",E_CHECKBOX_Bars_3D=" + BoolToIntStr(Setting("Bars_3D")) +
                   ",E_CHECKBOX_Bars_Highlight=" + BoolToIntStr(Setting("Bars_Highlight")) +
                   ",E_CHOICE_Bars_Direction=" + dir +
                   ",E_TEXTCTRL_Bars_BarCount=" + Setting("Bars_PaletteRepeat");
        } else if (nc == "Butterfly") {
            res += ",E_SLIDER_Butterfly_Chunks=" + Setting("Butterfly_BkgrdChunks") +
                   ",E_SLIDER_Butterfly_Skip=" + Setting("Butterfly_BkgrdSkip") +
                   ",E_SLIDER_Butterfly_Speed=" + std::to_string(speed * 5) + // max 100
                   ",E_SLIDER_Butterfly_Style=" + Setting("Butterfly_Style") +
                   ",E_CHOICE_Butterfly_Direction=" + Setting("Butterfly_Direction") +
                   ",E_CHOICE_Butterfly_Colors=" + (Setting("Butterfly_Colors") == "0" ? std::string("Rainbow") : std::string("Palette"));
        } else if (nc == "ColorWash") {
            res += ",E_TEXTCTRL_ColorWash_Cycles=" + Setting("ColorWash_Count") +
                   ",E_CHECKBOX_ColorWash_HFade=" + BoolToIntStr(Setting("ColorWash_FadeHorizontal")) +
                   ",E_CHECKBOX_ColorWash_VFade=" + BoolToIntStr(Setting("ColorWash_FadeVertical"));
        } else if (nc == "Curtain") {
            int i = (int)std::strtol(Setting("Curtain_Edge").c_str(), nullptr, 10);
            std::string edge = "left";
            if (i == 1) edge = "center";
            else if (i == 2) edge = "right";
            else if (i == 3) edge = "bottom";
            else if (i == 4) edge = "middle";
            else if (i == 5) edge = "top";

            i = (int)std::strtol(Setting("Curtain_Effect").c_str(), nullptr, 10);
            std::string effect = "open";
            if (i == 1) effect = "close";
            else if (i == 2) effect = "open then close";
            else if (i == 3) effect = "close then open";
            res += ",E_CHECKBOX_Curtain_Repeat=" + BoolToIntStr(Setting("Curtain_Repeat")) +
                   ",E_CHOICE_Curtain_Edge=" + edge +
                   ",E_CHOICE_Curtain_Effect=" + effect +
                   ",E_SLIDER_Curtain_Swag=" + Setting("Curtain_SwagWidth");
        } else if (nc == "Fire") {
            res += ",E_SLIDER_Fire_Height=" + Setting("Fire_Height") +
                   ",E_SLIDER_Fire_HueShift=" + Setting("Fire_Hue");
        } else if (nc == "Fireworks") {
            res += ",E_SLIDER_Fireworks_Count=" + Setting("Fireworks_Particles") +
                   ",E_SLIDER_Fireworks_Explosions=" + std::to_string((int)std::strtol(Setting("Fireworks_Explosions").c_str(), nullptr, 10) / 2) +
                   ",E_SLIDER_Fireworks_Fade=" + Setting("Fireworks_Fade") +
                   ",E_SLIDER_Fireworks_Velocity=" + Setting("Fireworks_Velocity");
        } else if (nc == "Garlands") {
            res += ",E_SLIDER_Garlands_Spacing=" + Setting("Garland_Spacing") +
                   ",E_SLIDER_Garlands_Type=" + Setting("Garland_Type");
        } else if (nc == "Glediator") {
            res += ",E_FILEPICKERCTRL_Glediator_Filename=" + Setting("Glediator_Filename");
        } else if (nc == "Life") {
            res += ",E_SLIDER_Life_Count=" + Setting("Life_CellsToStart") +
                   ",E_SLIDER_Life_Seed=" + Setting("Life_Type") +
                   ",E_SLIDER_Life_Speed=" + std::to_string((int)std::lround(speed * 1.5)); // max 30
        } else if (nc == "Meteors") {
            int i = (int)std::strtol(Setting("Meteor_Colors").c_str(), nullptr, 10);
            std::string colors = "Rainbow";
            if (i == 1) colors = "Range";
            else if (i == 2) colors = "Palette";
            res += ",E_CHOICE_Meteors_Type=" + colors +
                   ",E_SLIDER_Meteors_Count=" + Setting("Meteor_Count") +
                   ",E_SLIDER_Meteors_Length=" + Setting("Meteor_TrailLength") +
                   ",E_SLIDER_Meteors_Speed=" + std::to_string((int)std::lround(speed * 2.5)); // max 50
        } else if (nc == "Movie") {
            res += ",E_FILEPICKERCTRL_Video_Filename=" + Setting("Movie_DataPath");
        } else if (nc == "Picture") {
            int i = (int)std::strtol(Setting("Picture_Direction").c_str(), nullptr, 10);
            std::string dir = "left";
            if (i == 1) dir = "right";
            else if (i == 2) dir = "up";
            else if (i == 3) dir = "down";
            else if (i == 4) dir = "none";
            else if (i == 5) dir = "up-left";
            else if (i == 6) dir = "down-left";
            else if (i == 7) dir = "up-right";
            else if (i == 8) dir = "down-right";
            else if (i == 9) dir = "peekaboo";
            else if (i == 10) dir = "peekaboo 90";
            else if (i == 11) dir = "peekaboo 180";
            else if (i == 12) dir = "peekaboo 270";
            else if (i == 13) dir = "wiggle";
            res += ",E_CHOICE_Pictures_Direction=" + dir +
                   ",E_TEXTCTRL_Pictures_Filename=" + Setting("Picture_FileName") +
                   ",E_TEXTCTRL_Pictures_Speed=" + Setting("Picture_GifSpeed");
        } else if (nc == "PictureTile") {
            res += ",E_TEXTCTRL_Pictures_Filename=" + Setting("PictureTile_FileName");
        } else if (nc == "Snowflakes") {
            res += ",E_SLIDER_Snowflakes_Count=" + std::to_string((int)std::strtol(Setting("Snowflakes_Max").c_str(), nullptr, 10) * 5) + // max 20 in V3 / 100 in xLights
                   ",E_SLIDER_Snowflakes_Speed=" + std::to_string((int)std::lround(speed * 2.5)) + // max 50
                   ",E_SLIDER_Snowflakes_Type=" + Setting("Snowflakes_Type");
        } else if (nc == "Snowstorm") {
            res += ",E_SLIDER_Snowstorm_Count=" + Setting("Snowstorm_MaxFlakes") +
                   ",E_SLIDER_Snowstorm_Length=" + Setting("Snowstorm_TrailLength") +
                   ",E_SLIDER_Snowstorm_Speed=" + std::to_string((int)std::lround(speed * 2.5)); // max 50
        } else if (nc == "Spirals") {
            res += ",E_CHECKBOX_Spirals_3D=" + BoolToIntStr(Setting("Spirals_3D")) +
                   ",E_CHECKBOX_Spirals_Blend=" + BoolToIntStr(Setting("Spirals_Blend")) +
                   ",E_SLIDER_Spirals_Thickness=" + Setting("Spirals_Thickness");
        } else if (nc == "Spirograph") {
            res += ",E_SLIDER_Spirograph_R=" + Setting("Spirograph_ROuter") +
                   ",E_SLIDER_Spirograph_d=" + Setting("Spirograph_Distance") +
                   ",E_SLIDER_Spirograph_r=" + Setting("Spirograph_RInner") +
                   ",E_TEXTCTRL_Spirograph_Speed=" + std::to_string((int)std::lround(speed * 2.5)); // max 50
        } else if (nc == "Text") {
            int i = (int)std::strtol(Setting("Picture_Direction").c_str(), nullptr, 10);
            std::string dir = "left";
            if (i == 1) dir = "right";
            else if (i == 2) dir = "up";
            else if (i == 3) dir = "down";
            else if (i == 4) dir = "none";
            res += ",E_CHECKBOX_TextToCenter=" + BoolToIntStr(Setting("Text_CenterStop")) +
                   ",E_CHOICE_Text_Dir=" + dir +
                   ",E_TEXTCTRL_Text=" + Setting("Text_Line1") + "\n" + Setting("Text_Line2") + "\n" + Setting("Text_Line3") + "\n" + Setting("Text_Line4") +
                   ",E_TEXTCTRL_Text_Speed=" + std::to_string((int)std::lround(speed * 2.5)); // max 50
        } else if (nc == "Tree") {
            res += ",E_SLIDER_Tree_Branches=" + Setting("Tree_Branches") +
                   ",E_SLIDER_Tree_Speed=" + std::to_string((int)std::lround(speed * 2.5)); // max 50
        } else if (nc == "Twinkles") {
            res += ",E_CHECKBOX_Twinkle_Strobe=" + BoolToIntStr(Setting("Twinkles_Strobe")) +
                   ",E_CHOICE_Twinkle_Style=Old Render Method" +
                   ",E_SLIDER_Twinkle_Count=" + Setting("Twinkles_Count") +
                   ",E_SLIDER_Twinkle_Steps=" + Setting("Twinkles_Steps");
        }
    }

    return res;
}

std::string VixenEffect::GetXLightsType() const
{


    if (type == "AlternatingData") return "Marquee";
    if (type == "BarsData") return "Bars";
    if (type == "BorderData") return "Marquee";
    if (type == "ButterflyData") return "Butterfly";
    if (type == "CandleData") return "Candle";
    if (type == "ChaseData") return "Single Strand";
    if (type == "CirclesData") return "Circles";
    if (type == "ColorWashData") return "Color Wash";
    if (type == "CurtainData") return "Curtain";
    if (type == "Data") {
        spdlog::warn("Vixen3: Unable to convert Data effect ... inserting an On effect.");
        return "On"; // this should go to timing
    }
    if (type == "DissolveData") {
        spdlog::warn("Vixen3: Unable to convert DissolveData effect ... inserting an off effect.");
        return "Off"; // not sure what to do with this
    }
    if (type == "FireData") return "Fire";
    if (type == "FireworksData") return "Fireworks";
    if (type == "GarlandsData") return "Garlands";
    if (type == "GlediatorData") return "Glediator";
    if (type == "LifeData") return "Life";
    if (type == "LipSyncData") return "Faces";
    if (type == "LiquidData") return "Liquid";
    if (type == "MeteorsData") return "Meteors";
    if (type == "MorphData") {
        spdlog::warn("Vixen3: Morph color import not supported.");
        return "Morph";
    }
    if (type == "NutcrackerModuleData") {
        auto nc = Setting("CurrentEffect");
        if (nc == "ColorWash") nc = "Color Wash";
        else if (nc == "Movie") nc = "Video";
        else if (nc == "Picture") nc = "Pictures";
        else if (nc == "PictureTile") nc = "Pictures";
        else if (nc == "Twinkles") nc = "Twinkle";
        return nc;
    }

    if (type == "PictureData") return "Pictures";
    if (type == "PinWheelData") return "Pinwheel";
    if (type == "PlasmaData") return "Plasma";
    if (type == "PulseData") return "On";
    if (type == "SetLevelData") return "On";
    if (type == "ShapesData") {
        spdlog::warn("Vixen3: Shapes color import not supported.");
        return "Shape";
    }
    if (type == "ShockwaveData") return "Shockwave";
    if (type == "SnowflakesData") {
        spdlog::warn("Vixen3: Snowflakes color import not supported.");
        return "Snowflakes";
    }
    if (type == "SnowStormData") return "Snowstorm";
    if (type == "SpinData") {
        spdlog::warn("Vixen3: Unable to convert SpinData effect ... inserting an Pinwheel effect.");
        return "Pinwheel";
    }
    if (type == "SpiralData") return "Spirals";
    if (type == "SpirographData") return "Spirograph";
    if (type == "StrobeData") return "Strobe";
    if (type == "TextData") return "Text";
    if (type == "TreeData") {
        spdlog::warn("Vixen3: Tree color import not supported.");
        return "Tree";
    }
    if (type == "TwinkleData") return "Twinkle";
    if (type == "VideoData") return "Video";
    if (type == "VUMeterData") return "VU Meter";
    if (type == "WaveData") return "Wave";
    if (type == "WipeData") return "Single Strand";

    spdlog::warn("Vixen3: Unknown effect {} ... inserting an off effect.", type);

    return "Off";
}

void Vixen3::ProcessNode(pugi::xml_node n, std::map<std::string, std::string>& models)
{
    std::string name = n.attribute("name").as_string();
    std::string id = n.attribute("id").as_string();
    models[id] = name;

    for (pugi::xml_node m = n.first_child(); m; m = m.next_sibling())
    {
        if (std::string_view(m.name()) == "Node")
        {
            ProcessNode(m, models);
        }
    }
}

Vixen3::Vixen3(const std::string& filename, const std::string& system)
{


    _filename = filename;
    _systemFile = system;

    _systemFound = true;
    if (system == "" || !FileExists(_systemFile))
    {
        std::filesystem::path seqPath = std::filesystem::path(_filename).parent_path();
        _systemFile = (seqPath / "SystemConfig.xml").string();
        spdlog::debug("Looking for Vixen SystemConfig in {}", _systemFile);

        if (!FileExists(_systemFile))
        {
            _systemFile = (seqPath / "SystemData" / "SystemConfig.xml").string();
            spdlog::debug("Looking for Vixen SystemConfig in {}", _systemFile);
        }

        if (!FileExists(_systemFile))
        {
            _systemFile = (seqPath.parent_path() / "SystemData" / "SystemConfig.xml").string();
            spdlog::debug("Looking for Vixen SystemConfig in {}", _systemFile);
        }

        if (!FileExists(_systemFile))
        {
            spdlog::debug("Looking for Vixen SystemConfig ... FAILED ... NOT FOUND");
            _systemFound = false;
        }
    }

    std::map<std::string, std::string> models;

    if (FileExists(_systemFile))
    {
        pugi::xml_document sysDoc;
        sysDoc.load_file(_systemFile.c_str());

        pugi::xml_node sysRoot = sysDoc.document_element();
        if (sysRoot && std::string_view(sysRoot.name()) == "SystemConfig")
        {
            for (pugi::xml_node mm = sysRoot.first_child(); mm; mm = mm.next_sibling())
            {
                if (std::string_view(mm.name()) == "Nodes")
                {
                    for (pugi::xml_node mmm = mm.first_child(); mmm; mmm = mmm.next_sibling())
                    {
                        if (std::string_view(mmm.name()) == "Node")
                        {
                            ProcessNode(mmm, models);
                        }
                    }
                }
            }
        }
    }

    pugi::xml_document doc;
    doc.load_file(filename.c_str());

    std::map<std::string, pugi::xml_node> effectSettings;
    int unnamed = 1;
    pugi::xml_node docRoot = doc.document_element();
    if (docRoot && std::string_view(docRoot.name()) == "TimedSequenceData")
    {
        {
            for (pugi::xml_node nn = docRoot.first_child(); nn; nn = nn.next_sibling())
            {
                if (std::string_view(nn.name()) == "MarkCollections")
                {
                    for (pugi::xml_node nnn = nn.first_child(); nnn; nnn = nnn.next_sibling())
                    {
                        if (std::string_view(nnn.name()) == "MarkCollection")
                        {
                            std::string name;
                            std::list<VixenTiming> timing;
                            for (pugi::xml_node nnnn = nnn.first_child(); nnnn; nnnn = nnnn.next_sibling())
                            {
                                if (std::string_view(nnnn.name()) == "Name")
                                {
                                    name = nnnn.text().get();
                                }
                                else if (std::string_view(nnnn.name()) == "Marks")
                                {
                                    float last = 0;
                                    for (pugi::xml_node nnnnn = nnnn.first_child(); nnnnn; nnnnn = nnnnn.next_sibling())
                                    {
                                        if (std::string_view(nnnnn.name()) == "d3p1:duration")
                                        {
                                            float mt = ParseVixenDuration(nnnnn.text().get());
                                            timing.push_back(VixenTiming(last, mt, ""));
                                            last = mt;
                                        }
                                    }
                                }
                            }
                            _timingData[name] = timing;
                            _timingType[name] = "Generic";
                        }
                    }
                }
                else if (std::string_view(nn.name()) == "LabeledMarkCollections")
                {
                    for (pugi::xml_node nnn = nn.first_child(); nnn; nnn = nnn.next_sibling())
                    {
                        if (std::string_view(nnn.name()) == "d1p1:anyType")
                        {
                            std::list<VixenTiming> timing;
                            std::string name = "";
                            std::string type = "Generic";

                            for (pugi::xml_node nnnn = nnn.first_child(); nnnn; nnnn = nnnn.next_sibling())
                            {
                                if (std::string_view(nnnn.name()) == "d2p1:Name")
                                {
                                    name = nnnn.text().get();
                                }
                                else if (std::string_view(nnnn.name()) == "d2p1:CollectionType")
                                {
                                    type = nnnn.text().get();
                                }
                                else if (std::string_view(nnnn.name()) == "d2p1:Marks")
                                {
                                    float last = 0;
                                    for (pugi::xml_node nnnnn = nnnn.first_child(); nnnnn; nnnnn = nnnnn.next_sibling())
                                    {
                                        float duration = 0;
                                        float end = 0;
                                        std::string label = "";
                                        if (std::string_view(nnnnn.name()) == "d1p1:anyType")
                                        {
                                            for (pugi::xml_node nnnnnn = nnnnn.first_child(); nnnnnn; nnnnnn = nnnnnn.next_sibling())
                                            {
                                                if (std::string_view(nnnnnn.name()) == "d2p1:StartTime")
                                                {
                                                    end = ParseVixenDuration(nnnnnn.text().get());
                                                }
                                                else if (std::string_view(nnnnnn.name()) == "d2p1:Duration")
                                                {
                                                    duration = ParseVixenDuration(nnnnnn.text().get());
                                                }
                                                else if (std::string_view(nnnnnn.name()) == "d2p1:Text")
                                                {
                                                    label = nnnnnn.text().get();
                                                }
                                            }
                                            if (label == "")
                                            {
                                                // if labels are blank then we ignore duration
                                                if (end != 0 && end > last)
                                                {
                                                    timing.push_back(VixenTiming(last, end, ""));
                                                    last = end;
                                                }
                                            }
                                            else
                                            {
                                                // end is actually the start and we trust the duration
                                                if (duration > 0)
                                                {
                                                    float s = std::max(last, end);
                                                    if (s < end + duration)
                                                    {
                                                        float e = s + duration;
                                                        if (last > end)
                                                        {
                                                            duration -= (last - end);
                                                        }
                                                        timing.push_back(VixenTiming(s, e, label));
                                                        last = e;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            if (name == "")
                            {
                                name = "Unnamed " + std::to_string(unnamed++);
                            }
                            _timingData[name] = timing;
                            _timingType[name] = type;
                        }
                    }
                }
                else if (std::string_view(nn.name()) == "_effectNodeSurrogates" && models.size() > 0)
                {
                    for (pugi::xml_node nnn = nn.first_child(); nnn; nnn = nnn.next_sibling())
                    {
                        if (std::string_view(nnn.name()) == "EffectNodeSurrogate")
                        {
                            std::string effectSettingId;
                            std::string modelId;
                            float start = 0.0;
                            float duration = 0.0;
                            for (pugi::xml_node nnnn = nnn.first_child(); nnnn; nnnn = nnnn.next_sibling())
                            {
                                if (std::string_view(nnnn.name()) == "InstanceId")
                                {
                                    effectSettingId = nnnn.text().get();
                                }
                                else if (std::string_view(nnnn.name()) == "StartTime")
                                {
                                    start = ParseVixenDuration(nnnn.text().get());
                                }
                                else if (std::string_view(nnnn.name()) == "TimeSpan")
                                {
                                    duration = ParseVixenDuration(nnnn.text().get());
                                }
                                else if (std::string_view(nnnn.name()) == "TargetNodes")
                                {
                                    for (pugi::xml_node nnnnn = nnnn.first_child(); nnnnn; nnnnn = nnnnn.next_sibling())
                                    {
                                        if (std::string_view(nnnnn.name()) == "ChannelNodeReferenceSurrogate")
                                        {
                                            for (pugi::xml_node nnnnnn = nnnnn.first_child(); nnnnnn; nnnnnn = nnnnnn.next_sibling())
                                            {
                                                if (std::string_view(nnnnnn.name()) == "NodeId")
                                                {
                                                    modelId = nnnnnn.text().get();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            VixenEffect ve(start, start + duration, effectSettingId);

                            auto m = models.find(modelId);

                            if (m != models.end())
                            {
                                _effectData[m->second].push_back(ve);
                            }
                            else
                            {
                                spdlog::warn("Vixen3: model not found for effect. {}", modelId);
                            }
                        }
                    }
                }
                else if (std::string_view(nn.name()) == "_dataModels" && models.size() > 0)
                {
                    for (pugi::xml_node nnn = nn.first_child(); nnn; nnn = nnn.next_sibling())
                    {
                        if (std::string_view(nnn.name()) == "d1p1:anyType")
                        {
                            std::string type = nnn.attribute("i:type").as_string();
                            if (type != "")
                            {
                                std::string id;
                                for (pugi::xml_node nnnn = nnn.first_child(); id.empty() && nnnn; nnnn = nnnn.next_sibling())
                                {
                                    if (std::string_view(nnnn.name()) == "ModuleInstanceId")
                                    {
                                        id = nnnn.text().get();
                                    }
                                }
                                effectSettings[id] = nnn;
                            }
                        }
                    }
                }
            }
        }
    }

    // hook up all the effect settings
    for (auto it = _effectData.begin(); it != _effectData.end(); ++it)
    {
        for (auto e = it->second.begin(); e != it->second.end(); ++e)
        {
            auto es = effectSettings.find(e->effectSettingsId);
            if (es != effectSettings.end())
            {
                std::string colorHandling;
                std::vector<std::vector<VixenColor>> palleteStatic;
                std::vector<std::vector<VixenColor>> palleteDefault;

                e->type = AfterFirst(es->second.attribute("i:type").as_string(), ':');
                for (pugi::xml_node n = es->second.first_child(); n; n = n.next_sibling())
                {
                    std::string nName = n.name();
                    if (nName == "TargetPositioning" && n.first_child()) {
                        e->settings[std::string(n.name())] = n.text().get();
                    } else if (StartsWith(nName, "d2p1:") && n.first_child()) {
                        std::string nm = AfterFirst(nName, ':');

                        if (nm == "NutcrackerData") {
                            for (pugi::xml_node nn = n.first_child(); nn; nn = nn.next_sibling()) {
                                std::string nm2 = AfterFirst(nn.name(), ':');
                                if (nn.first_child()) {
                                    // Process color palette
                                    if (nm2 == "Palette") {
                                        std::vector<int> knownColor;
                                        for (pugi::xml_node nnn = nn.first_child(); nnn; nnn = nnn.next_sibling()) {
                                            std::string nm3 = AfterFirst(nnn.name(), ':');
                                            if (nnn.first_child()) {
                                                if (nm3 == "_colors") {
                                                    for (pugi::xml_node nnnn = nnn.first_child(); nnnn; nnnn = nnnn.next_sibling()) {
                                                        std::string nm4 = AfterFirst(nnnn.name(), ':');
                                                        if (nm4 == "Color") {
                                                            for (pugi::xml_node nnnnn = nnnn.first_child(); nnnnn; nnnnn = nnnnn.next_sibling()) {
                                                                std::string nm5 = AfterFirst(nnnnn.name(), ':');
                                                                if (nm5 == "knownColor") {
                                                                    // Finally, got the color
                                                                    knownColor.push_back(nnnnn.text().as_int());
                                                                }
                                                            }
                                                        }
                                                    }
                                                } else if (nm3 == "_colorsActive") {
                                                    auto index = 0;
                                                    for (pugi::xml_node nnnn = nnn.first_child(); nnnn; nnnn = nnnn.next_sibling()) {
                                                        std::string nm4 = AfterFirst(nnnn.name(), ':');
                                                        if (nm4 == "boolean") {
                                                            if (std::string_view(nnnn.text().get()) == "true" && index < (int)knownColor.size() && knownColor[index] >= 0 && knownColor[index] < (int)(sizeof(VixenEffect::KNOWN_COLOR) / sizeof(VixenEffect::KNOWN_COLOR[0]))) {
                                                                uint32_t argb = VixenEffect::KNOWN_COLOR[knownColor[index]];
                                                                char r = (argb & 0x00FF0000) >> 16;
                                                                char g = (argb & 0x0000FF00) >> 8;
                                                                char b = (argb & 0x000000FF);
                                                                palleteDefault.push_back({ VixenColor(xlColor((uint8_t)r, (uint8_t)g, (uint8_t)b), 0) });
                                                            }
                                                        }
                                                        index++;
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        e->settings[nm2] = nn.text().get();
                                    }
                                }
                            }
                        } else {
                            e->settings[nm] = n.text().get();
                        }

                        // Color for SetLevel
                        if (nm == "color") {
                            int r = 0;
                            int g = 0;
                            int b = 0;
                            for (pugi::xml_node nn = n.first_child(); nn; nn = nn.next_sibling()) {
                                std::string nm2 = AfterFirst(nn.name(), ':');
                                if (nm2 == "_r") {
                                    r = (int)std::clamp(255.0 * std::strtod(nn.text().get(), nullptr), 0.0, 255.0);
                                }
                                else if (nm2 == "_g") {
                                    g = (int)std::clamp(255.0 * std::strtod(nn.text().get(), nullptr), 0.0, 255.0);
                                }
                                else if (nm2 == "_b") {
                                    b = (int)std::clamp(255.0 * std::strtod(nn.text().get(), nullptr), 0.0, 255.0);
                                }
                            }
                            palleteDefault.push_back({ VixenColor(xlColor((uint8_t)r, (uint8_t)g, (uint8_t)b), 0) });
                        }

                        // ColorHandling known values:
                        // - StaticColor
                        // - GradientThroughWHoleEffect
                        else if (nm == "ColorHandling") {
                            colorHandling = n.text().get();
                        }

                        // Colors for the following:
                        // ColorGradient:      Chase Pulse Spin Twinkle Wipe
                        // Gradient:           ColorWash Border Butterfly Curtain Shockwave
                        // Head/Fill:          Morph
                        // MeterColorGradient: VetricalMeter VUMeter Waveform
                        else if (nm == "ColorGradient" || nm == "Gradient" || nm == "HeadColor" || nm == "FillColor" || nm == "MeterColorGradient") {
                            for (pugi::xml_node nn = n.first_child(); nn; nn = nn.next_sibling()) {
                                std::string nm2 = AfterFirst(nn.name(), ':');
                                if (nm2 == "_colors") {
                                    palleteDefault.push_back(ProcessColorData(nn));
                                }
                            }
                        }

                        // Colors for the following:
                        // Colors:                                   Strobe Picture
                        // Colors->ColorGradient:                    Bars Balls Circles Garland Life Liquid Meteors Plasma Snowstorm Spiral Spirograph Wave
                        // Colors->GradientLevelPair->ColorGradient: Alternating Dissolve Pinwheel
                        // GradientColors->ColorGradient:            Countdown
                        // ColorGradients->ColorGradient:            Fireworks
                        else if (nm == "Colors" || nm == "GradientColors" || nm == "ColorGradients") {
                            for (pugi::xml_node nn = n.first_child(); nn; nn = nn.next_sibling()) {
                                std::string nm2 = AfterFirst(nn.name(), ':');
                                if (nm2 == "_colors") {
                                    palleteDefault.push_back(ProcessColorData(nn));
                                } else if (nm2 == "ColorGradient" || nm2 == "GradientLevelPair") {
                                    for (pugi::xml_node nnn = nn.first_child(); nnn; nnn = nnn.next_sibling()) {
                                        std::string nm3 = AfterFirst(nnn.name(), ':');
                                        if (nm3 == "_colors") {
                                            palleteDefault.push_back(ProcessColorData(nnn));
                                        } else if (nm3 == "ColorGradient") {
                                            for (pugi::xml_node nnnn = nnn.first_child(); nnnn; nnnn = nnnn.next_sibling()) {
                                                std::string nm4 = AfterFirst(nnnn.name(), ':');
                                                if (nm4 == "_colors") {
                                                    palleteDefault.push_back(ProcessColorData(nnnn));
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Colors for the following:
                        // Twinkle
                        else if (nm == "StaticColor") {
                            for (pugi::xml_node nn = n.first_child(); nn; nn = nn.next_sibling()) {
                                std::string nm2 = AfterFirst(nn.name(), ':');
                                if (nm2 == "value") {
                                    unsigned long argb;
                                    argb = std::strtoul(nn.text().get(), nullptr, 10);
                                    char r = (argb & 0x00FF0000) >> 16;
                                    char g = (argb & 0x0000FF00) >> 8;
                                    char b = (argb & 0x000000FF);
                                    palleteStatic.push_back({ VixenColor(xlColor((uint8_t)r, (uint8_t)g, (uint8_t)b), 0) });
                                }
                            }
                        }

                        // Brightness curve
                        else if (nm == "LevelCurve") {
                            for (pugi::xml_node nn = n.first_child(); nn; nn = nn.next_sibling()) {
                                std::string nm2 = AfterFirst(nn.name(), ':');
                                if (nm2 == "Points") {
                                    for (pugi::xml_node nnn = nn.first_child(); nnn; nnn = nnn.next_sibling()) {
                                        std::string nm3 = AfterFirst(nnn.name(), ':');
                                        if (nm3 == "PointPair") {
                                            double x = 0;
                                            double y = 0;
                                            for (pugi::xml_node nnnn = nnn.first_child(); nnnn; nnnn = nnnn.next_sibling()) {
                                                std::string nm4 = nnnn.name();
                                                if (nm4 == "X") {
                                                    x = nnnn.text().as_int();
                                                } else if (nm4 == "Y") {
                                                    y = nnnn.text().as_int();
                                                }
                                            }
                                            e->levelCurve.push_back(VixenPoint(x, y));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if (colorHandling == "StaticColor") {
                    e->palette = palleteStatic;
                } else {
                    e->palette = palleteDefault;
                }
            }
        }
    }
}

std::vector<VixenColor> Vixen3::ProcessColorData(pugi::xml_node n)
{
    std::vector<VixenColor> vColor;

    for (pugi::xml_node nn = n.first_child(); nn; nn = nn.next_sibling()) {
        std::string nm3 = AfterFirst(nn.name(), ':');
        if (nm3 == "ColorPoint") {
            double x = 0;
            double y = 0;
            double z = 0;
            double position = 0;

            for (pugi::xml_node nnn = nn.first_child(); nnn; nnn = nnn.next_sibling()) {
                std::string nm4 = AfterFirst(nnn.name(), ':');
                if (nm4 == "_color") {
                    for (pugi::xml_node nnnn = nnn.first_child(); nnnn; nnnn = nnnn.next_sibling()) {
                        std::string nm5 = AfterFirst(nnnn.name(), ':');
                        if (nm5 == "_x") {
                            x = std::strtod(nnnn.text().get(), nullptr);
                        } else if (nm5 == "_y") {
                            y = std::strtod(nnnn.text().get(), nullptr);
                        } else if (nm5 == "_z") {
                            z = std::strtod(nnnn.text().get(), nullptr);
                        }
                    }
                } else if (nm4 == "_position") {
                    position = std::strtod(nnn.text().get(), nullptr);
                }
            }
            vColor.push_back(VixenColor(ConvertXYZToColour(x, y, z), position));
        }
    }

    std::sort(vColor.begin(), vColor.end());
    return (vColor);
}

std::list<std::pair<std::string, int>> Vixen3::GetModelsWithEffects() const
{
    std::list<std::pair<std::string, int>> res;

    for (auto it: _effectData)
    {
        if (it.second.size() > 0)
        {
            res.emplace_back(it.first, it.second.size());
        }
    }

    return res;
}

std::list<std::string> Vixen3::GetTimings() const
{
    std::list<std::string> res;

    for (const auto& it : _timingData)
    {
        if (_timingType.at(it.first) == "Generic" || _timingType.at(it.first) == "Phrase")
        {
            res.push_back(it.first);
        }
    }

    return res;
}

std::string Vixen3::GetTimingType(const std::string& timing) const
{
    if (_timingType.find(timing) == _timingType.end()) return "";
    return _timingType.at(timing);
}

// type must be Phoneme or Word
std::list<VixenTiming> Vixen3::GetRelatedTiming(const std::string& timing, const std::string& type) const
{
    return GetTimings(timing + " " + type);
}

std::list<VixenTiming> Vixen3::GetTimings(const std::string& timing) const
{
    if (_timingData.find(timing) != _timingData.end())
    {
        return _timingData.at(timing);
    }

    std::list<VixenTiming> empty;
    return empty;
}

std::list<VixenEffect> Vixen3::GetEffects(const std::string& model) const
{
    if (_effectData.find(model) != _effectData.end())
    {
        return _effectData.at(model);
    }

    std::list<VixenEffect> empty;
    return empty;
}
