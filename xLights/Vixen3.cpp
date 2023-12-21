/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "Vixen3.h"

#include <list>
#include <math.h>

#include <wx/wx.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <log4cpp/Category.hh>
#include "ExternalHooks.h"

double CorrectForGamma(double value)
{
    if (value > 0.0031308)
        return 1.055 * pow(value, 1.0 / 2.4) - 0.055;
    else
        return 12.92 * value;
}

wxColour ConvertXYZToColour(double x, double y, double z)
{
    return wxColour(std::max(0.0, std::min(255.0, 255.0 * CorrectForGamma(x * +0.032406 + y * -0.015372 + z * -0.004986))),
                    std::max(0.0, std::min(255.0, 255.0 * CorrectForGamma(x * -0.009689 + y * +0.018758 + z * +0.000415))),
                    std::max(0.0, std::min(255.0, 255.0 * CorrectForGamma(x * +0.000557 + y * -0.002040 + z * +0.010570)))
                   );
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

    for (int i = 0; i < palette.size(); i++) {
        wxString n = wxString::Format("%d", i + 1);

        if (res != "")
            res += ",";

        if (palette[i].size() == 1) {
            res += "C_BUTTON_Palette" + n + "=#" + wxString::Format("%02x%02x%02x",
                    palette[i][0].color.Red(), palette[i][0].color.Green(), palette[i][0].color.Blue());
            res += ",C_CHECKBOX_Palette" + n + "=1";
        // Break buttefly gradient into individual colors
        } else if (type == "ButterflyData") {
            for (int ii = 0; ii < palette[i].size(); ii++) {
                wxString nn = wxString::Format("%d", ii + 1);
                if (res != "")
                    res += ",";
                res += "C_BUTTON_Palette" + nn + "=#" + wxString::Format("%02x%02x%02x", palette[i][ii].color.Red(), palette[i][ii].color.Green(), palette[i][ii].color.Blue());
                res += ",C_CHECKBOX_Palette" + nn + "=1";
            }
        } else {
            res += "C_BUTTON_Palette" + n + "=Active=TRUE|Id=ID_BUTTON_Palette" + n + "|Values=";
            for (int ii = 0; ii < palette[i].size(); ii++) {
                res += wxString::Format("x=%.3f^c=#%02x%02x%02x",
                       palette[i][ii].position, palette[i][ii].color.Red(), palette[i][ii].color.Green(), palette[i][ii].color.Blue());
                if (ii != palette[i].size() - 1)
                    res += ";";
            }
            res += "|,C_CHECKBOX_Palette" + n + "=1";
        }

    }

    // More than two points for the Vixen level curve, generate a custom brightness value curve
    if (type == "PulseData" && levelCurve.size() > 2) {
        res += ",C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Custom|Min=0.00|Max=400.00|RV=TRUE|Values=";
        for (int i = 0; i < levelCurve.size(); i++) {
            res += wxString::Format("%.2f:", levelCurve[i].x / 100) + wxString::Format("%.2f", levelCurve[i].y / 100 / 4);
            if (i != levelCurve.size() - 1)
                res += ";";
        }
    }

    return res;
}

std::string VixenEffect::GetSettings() const
{
    std::string res;
    if (type == "AlternatingData") {
        res = "B_CHOICE_BufferStyle=Single Line,E_SLIDER_Marquee_Band_Size=" + settings.at("GroupLevel");       
    }
    else if (type == "AudioData") {}
    else if (type == "BarsData") {
        // Up/Down and Left/Right are backwards in V3
        auto s = settings.at("Direction");
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

        if (settings.at("TargetPositioning") == "Strings") {
            if (settings.at("Orientation") == "Vertical") {
                res = "B_CHOICE_BufferStyle=Horizontal Per Model/Strand,";
            } else {
                res = "B_CHOICE_BufferStyle=Vertical Per Model/Strand,";
            }
        }

        res += "E_CHECKBOX_Bars_3D=" + settings.at("Show3D") +
               ",E_CHECKBOX_Bars_Highlight=" + settings.at("Highlight") +
               ",E_CHOICE_Bars_Direction=" + dir +
               ",E_SLIDER_Bars_BarCount=" + wxString::Format("%d", std::min(wxAtoi(settings.at("Repeat")), 5)) +
               ",E_TEXTCTRL_Bars_Cycles=" + settings.at("Speed");
    }
    else if (type == "ButterflyData") {
        res = "E_SLIDER_Butterfly_Chunks=" + settings.at("BackgroundChunks") +
              ",E_SLIDER_Butterfly_Skip=" + settings.at("BackgroundSkips") +
              ",E_SLIDER_Butterfly_Style=" + wxString(settings.at("ButterflyType")).AfterFirst('e').ToStdString() +
              ",E_CHOICE_Butterfly_Direction=" + (settings.at("Direction") == "Forward" ? "0" : "1") +
              ",E_CHOICE_Butterfly_Colors=" + (settings.at("ColorScheme") == "Gradient" ? "Palette" : "Rainbow");
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
            res = "E_TEXTCTRL_Eff_On_Start=" + wxString::Format("%d", wxRound(levelCurve[0].y)) +
                  ",E_TEXTCTRL_Eff_On_End=" + wxString::Format("%d", wxRound(levelCurve[1].y));
        }
    }
    else if (type == "TwinkleData") {
        res = "E_SLIDER_Twinkle_Steps=" + settings.at("AveragePulseTime") +
              ",E_SLIDER_Twinkle_Count=" + settings.at("AverageCoverage");
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
        auto d = settings.at("Direction");
        auto r = settings.at("ReverseDirection");
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
                
        if (settings.at("WipeMovement") == "Count")
            res = res + ",E_TEXTCTRL_Chase_Rotations=" + settings.at("PassCount");
    }
    else if (type == "NutcrackerModuleData") {
        std::string nc = settings.at("CurrentEffect");
        std::string fit = BoolToIntStr(settings.at("FitToTime"));  // Not sure this can be mapped
        int speed = wxAtoi(settings.at("Speed")); // Max of 20

        // V3 Nutcracker wasn't spatially aware so switch render style
        if (settings.at("StringOrienation") == "Vertical") {
            res = "B_CHOICE_BufferStyle=Horizontal Per Model/Strand";
        } else {
            res = "B_CHOICE_BufferStyle=Vertical Per Model/Strand";
        }

        if (nc == "Bars") {
            int i = wxAtoi(settings.at("Bars_Direction"));
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
            res += ",E_CHECKBOX_Bars_3D=" + BoolToIntStr(settings.at("Bars_3D")) +
                   ",E_CHECKBOX_Bars_Highlight=" + BoolToIntStr(settings.at("Bars_Highlight")) +
                   ",E_CHOICE_Bars_Direction=" + dir +
                   ",E_TEXTCTRL_Bars_BarCount=" + settings.at("Bars_PaletteRepeat");
        } else if (nc == "Butterfly") {
            res += ",E_Butterfly_Chunks=" + settings.at("Butterfly_BkgrdChunks") +
                   ",E_Butterfly_Skip=" + settings.at("Butterfly_BkgrdSkip") +
                   ",E_SLIDER_Butterfly_Speed=" + wxString::Format("%d", speed * 5) + // max 100
                   ",E_SLIDER_Butterfly_Style=" + settings.at("Butterfly_Style") +
                   ",E_CHOICE_Butterfly_Direction=" + settings.at("Butterfly_Direction") +
                   ",E_CHOICE_Butterfly_Colors=" + (settings.at("Butterfly_Colors") == "0" ? std::string("Rainbow") : std::string("Palette"));
        } else if (nc == "ColorWash") {
            res += ",E_TEXTCTRL_ColorWash_Cycles=" + settings.at("ColorWash_Count") +
                   ",E_CHECKBOX_ColorWash_HFade=" + BoolToIntStr(settings.at("ColorWash_FadeHorizontal")) +
                   ",E_CHECKBOX_ColorWash_VFade=" + BoolToIntStr(settings.at("ColorWash_FadeVertical"));
        } else if (nc == "Curtain") {
            int i = wxAtoi(settings.at("Curtain_Edge"));
            std::string edge = "left";
            if (i == 1) edge = "center";
            else if (i == 2) edge = "right";
            else if (i == 3) edge = "bottom";
            else if (i == 4) edge = "middle";
            else if (i == 5) edge = "top";

            i = wxAtoi(settings.at("Curtain_Effect"));
            std::string effect = "open";
            if (i == 1) effect = "close";
            else if (i == 2) effect = "open then close";
            else if (i == 3) effect = "close then open";
            res += ",E_CHECKBOX_Curtain_Repeat=" + BoolToIntStr(settings.at("Curtain_Repeat")) +
                   ",E_CHOICE_Curtain_Edge=" + edge +
                   ",E_CHOICE_Curtain_Effect=" + effect +
                   ",E_SLIDER_Curtain_Swag=" + settings.at("Curtain_SwagWidth");
        } else if (nc == "Fire") {
            res += ",E_SLIDER_Fire_Height=" + settings.at("Fire_Height") +
                   ",E_SLIDER_Fire_HueShift=" + settings.at("Fire_Hue");
        } else if (nc == "Fireworks") {
            res += ",E_SLIDER_Fireworks_Count=" + settings.at("Fireworks_Particles") +
                   ",E_SLIDER_Fireworks_Explosions=" + wxString::Format("%d", (wxAtoi(settings.at("Fireworks_Explosions")) / 2)) +
                   ",E_SLIDER_Fireworks_Fade=" + settings.at("Fireworks_Fade") +
                   ",E_SLIDER_Fireworks_Velocity=" + settings.at("Fireworks_Fade");
        } else if (nc == "Garlands") {
            res += ",E_SLIDER_Garlands_Spacing=" + settings.at("Garland_Spacing") +
                   ",E_SLIDER_Garlands_Type=" + settings.at("Garland_Type");
        } else if (nc == "Glediator") {
            res += ",E_FILEPICKERCTRL_Glediator_Filename=" + settings.at("Glediator_Filename");
        } else if (nc == "Life") {
            res += ",E_SLIDER_Life_Count=" + settings.at("Life_CellsToStart") +
                   ",E_SLIDER_Life_Seed=" + settings.at("Life_Type") +
                   ",E_SLIDER_Life_Speed=" + wxString::Format("%d", wxRound(speed * 1.5)); // max 30
        } else if (nc == "Meteors") {
            int i = wxAtoi(settings.at("Meteor_Colors"));
            std::string colors = "Rainbow";
            if (i == 1) colors = "Range";
            else if (i == 2) colors = "Palette";
            res += ",E_CHOICE_Meteors_Type=" + colors +
                   ",E_SLIDER_Meteors_Count=" + settings.at("Meteor_Count") +
                   ",E_SLIDER_Meteors_Length=" + settings.at("Meteor_TrailLength") +
                   ",E_SLIDER_Meteors_Speed=" + wxString::Format("%d", wxRound(speed * 2.5)); // max 50
        } else if (nc == "Movie") {
            res += ",E_FILEPICKERCTRL_Video_Filename=" + settings.at("Movie_DataPath");
        } else if (nc == "Picture") {
            int i = wxAtoi(settings.at("Picture_Direction"));
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
                   ",E_FILEPICKER_Pictures_Filename=" + settings.at("Picture_FileName") +
                   ",E_TEXTCTRL_Pictures_Speed=" + settings.at("Picture_GifSpeed");
        } else if (nc == "PictureTile") {
            res += ",E_FILEPICKER_Pictures_Filename=" + settings.at("PictureTile_FileName");
        } else if (nc == "Snowflakes") {
            res += ",E_SLIDER_Snowflakes_Count=" + wxString::Format("%d", wxAtoi(settings.at("Snowflakes_Max")) * 5) + // max 20 in V3 / 100 in xLights
                   ",E_SLIDER_Snowflakes_Speed=" + wxString::Format("%d", wxRound(speed * 2.5)) + // max 50
                   ",E_SLIDER_Snowflakes_Type=" + settings.at("Snowflakes_Type");
        } else if (nc == "Snowstorm") {
            res += ",E_SLIDER_Snowstorm_Count=" + settings.at("Snowstorm_MaxFlakes") +
                   ",E_SLIDER_Snowstorm_Length=" + settings.at("Snowstorm_TrailLength") +
                   ",E_SLIDER_Snowstorm_Speed=" + wxString::Format("%d", wxRound(speed * 2.5)); // max 50
        } else if (nc == "Spirals") {
            res += ",E_CHECKBOX_Spirals_3D=" + BoolToIntStr(settings.at("Spirals_3D")) +
                   ",E_CHECKBOX_Spirals_Blend=" + BoolToIntStr(settings.at("Spirals_Blend")) +
                   ",E_SLIDER_Spirals_Thickness=" + settings.at("Spirals_Thickness");
        } else if (nc == "Spirograph") {
            res += ",E_SLIDER_Spirograph_R=" + settings.at("Spirograph_ROuter") +
                   ",E_SLIDER_Spirograph_d=" + settings.at("Spirograph_Distance") +
                   ",E_SLIDER_Spirograph_r=" + settings.at("Spirograph_RInner") +
                   ",E_TEXTCTRL_Spirograph_Speed=" + wxString::Format("%d", wxRound(speed * 2.5)); // max 50
        } else if (nc == "Text") {
            int i = wxAtoi(settings.at("Picture_Direction"));
            std::string dir = "left";
            if (i == 1) dir = "right";
            else if (i == 2) dir = "up";
            else if (i == 3) dir = "down";
            else if (i == 4) dir = "none";
            res += ",E_CHECKBOX_TextToCenter=" + BoolToIntStr(settings.at("Text_CenterStop")) +
                   ",E_CHOICE_Text_Dir=" + dir +
                   ",E_TEXTCTRL_Text=" + settings.at("Text_Line1") + "\n" + settings.at("Text_Line2") + "\n" + settings.at("Text_Line3") + "\n" + settings.at("Text_Line4") +
                   ",E_TEXTCTRL_Text_Speed=" + wxString::Format("%d", wxRound(speed * 2.5)); // max 50
        } else if (nc == "Tree") {
            res += ",E_SLIDER_Tree_Branches=" + settings.at("Tree_Branches") +
                   ",E_SLIDER_Tree_Speed=" + wxString::Format("%d", wxRound(speed * 2.5)); // max 50
        } else if (nc == "Twinkles") {
            res += ",E_CHECKBOX_Twinkle_Strobe=1" + BoolToIntStr(settings.at("Twinkles_Strobe")) +
                   ",E_CHOICE_Twinkle_Style=Old Render Method" +
                   ",E_SLIDER_Twinkle_Count=" + settings.at("Twinkles_Count") +
                   ",E_SLIDER_Twinkle_Steps=" + settings.at("Twinkles_Steps");
        }
    }

    return res;
}

std::string VixenEffect::GetXLightsType() const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

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
        logger_base.warn("Vixen3: Unable to convert Data effect ... inserting an On effect.");
        return "On"; // this should go to timing
    }
    if (type == "DissolveData") {
        logger_base.warn("Vixen3: Unable to convert DissolveData effect ... inserting an off effect.");
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
        logger_base.warn("Vixen3: Morph color import not supported.");
        return "Morph";
    }
    if (type == "NutcrackerModuleData") {
        auto nc = settings.at("CurrentEffect");
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
        logger_base.warn("Vixen3: Shapes color import not supported.");
        return "Shape";
    }
    if (type == "ShockwaveData") return "Shockwave";
    if (type == "SnowflakesData") {
        logger_base.warn("Vixen3: Snowflakes color import not supported.");
        return "Snowflakes";
    }
    if (type == "SnowStormData") return "Snowstorm";
    if (type == "SpinData") {
        logger_base.warn("Vixen3: Unable to convert SpinData effect ... inserting an Pinwheel effect.");
        return "Pinwheel";
    }
    if (type == "SpiralData") return "Spirals";
    if (type == "SpirographData") return "Spirograph";
    if (type == "StrobeData") return "Strobe";
    if (type == "TextData") return "Text";
    if (type == "TreeData") {
        logger_base.warn("Vixen3: Tree color import not supported.");
        return "Tree";
    }
    if (type == "TwinkleData") return "Twinkle";
    if (type == "VideoData") return "Video";
    if (type == "VUMeterData") return "VU Meter";
    if (type == "WaveData") return "Wave";
    if (type == "WipeData") return "Single Strand";

    logger_base.warn("Vixen3: Unknown effect %s ... inserting an off effect.", (const char*)type.c_str());

    return "Off";
}

void Vixen3::ProcessNode(wxXmlNode* n, std::map<std::string, std::string>& models)
{
    auto name = n->GetAttribute("name");
    auto id = n->GetAttribute("id");
    models[id] = name;

    for (wxXmlNode *m = n->GetChildren(); m != nullptr; m = m->GetNext())
    {
        if (m->GetName() == "Node")
        {
            ProcessNode(m, models);
        }
    }
}

Vixen3::Vixen3(const std::string& filename, const std::string& system)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _filename = filename;
    _systemFile = system;

    _systemFound = true;
    if (system == "" || !FileExists(_systemFile))
    {
        wxFileName seq(_filename);
        _systemFile = seq.GetPath() + wxFileName::GetPathSeparator() + "SystemConfig.xml";
        logger_base.debug("Looking for Vixen SystemConfig in %s", (const char*)_systemFile.c_str());

        if (!FileExists(_systemFile))
        {
            _systemFile = seq.GetPath() + wxFileName::GetPathSeparator() + "SystemData" + wxFileName::GetPathSeparator() + "SystemConfig.xml";
            logger_base.debug("Looking for Vixen SystemConfig in %s", (const char*)_systemFile.c_str());
        }

        if (!FileExists(_systemFile))
        {
            int lastFolder = seq.GetPath().Last(wxFileName::GetPathSeparator());
            _systemFile = seq.GetPath().Left(lastFolder) + wxFileName::GetPathSeparator() + "SystemData" + wxFileName::GetPathSeparator() + "SystemConfig.xml";
            logger_base.debug("Looking for Vixen SystemConfig in %s", (const char*)_systemFile.c_str());
        }

        if (!FileExists(_systemFile))
        {
            logger_base.debug("Looking for Vixen SystemConfig ... FAILED ... NOT FOUND");
            _systemFound = false;
        }
    }

    std::map<std::string, std::string> models;

    if (FileExists(_systemFile))
    {
        wxXmlDocument sysDoc(_systemFile);

        for (wxXmlNode *m = sysDoc.GetRoot(); m != nullptr; m = m->GetNext())
        {
            if (m->GetName() == "SystemConfig")
            {
                for (wxXmlNode *mm = m->GetChildren(); mm != nullptr; mm = mm->GetNext())
                {
                    if (mm->GetName() == "Nodes")
                    {
                        for (wxXmlNode *mmm = mm->GetChildren(); mmm != nullptr; mmm = mmm->GetNext())
                        {
                            if (mmm->GetName() == "Node")
                            {
                                ProcessNode(mmm, models);
                            }
                        }
                    }
                }
            }
        }
    }

    wxXmlDocument doc(filename);

    std::map<std::string, wxXmlNode*> effectSettings;
    wxArrayString markNames;
    int unnamed = 1;
    for (wxXmlNode *n = doc.GetRoot(); n != nullptr; n = n->GetNext())
    {
        if (n->GetName() == "TimedSequenceData")
        {
            for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext())
            {
                if (nn->GetName() == "MarkCollections")
                {
                    for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext())
                    {
                        if (nnn->GetName() == "MarkCollection")
                        {
                            std::string name;
                            std::list<VixenTiming> timing;
                            for (wxXmlNode* nnnn = nnn->GetChildren(); nnnn != nullptr; nnnn = nnnn->GetNext())
                            {
                                if (nnnn->GetName() == "Name")
                                {
                                    name = nnnn->GetChildren()->GetContent();
                                }
                                else if (nnnn->GetName() == "Marks")
                                {
                                    float last = 0;
                                    for (wxXmlNode* nnnnn = nnnn->GetChildren(); nnnnn != nullptr; nnnnn = nnnnn->GetNext())
                                    {
                                        if (nnnnn->GetName() == "d3p1:duration")
                                        {
                                            auto markTime = nnnnn->GetChildren()->GetContent();
                                            if (markTime.StartsWith("PT"))
                                            {
                                                markTime = markTime.AfterFirst('T');
                                            }
                                            float mins = 0;
                                            if (markTime.Contains('M'))
                                            {
                                                mins = wxAtof(markTime.BeforeFirst('M'));
                                                markTime = markTime.AfterFirst('M');
                                            }
                                            float secs = 0;
                                            if (markTime.EndsWith("S"))
                                            {
                                                secs = wxAtof(markTime.BeforeLast('S'));
                                            }
                                            float mt = mins * 60.0 + secs;
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
                else if (nn->GetName() == "LabeledMarkCollections")
                {
                    for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext())
                    {
                        if (nnn->GetName() == "d1p1:anyType")
                        {
                            std::list<VixenTiming> timing;
                            std::string name = ""; 
                            std::string type = "Generic";

                            for (wxXmlNode* nnnn = nnn->GetChildren(); nnnn != nullptr; nnnn = nnnn->GetNext())
                            {
                                if (nnnn->GetName() == "d2p1:Name")
                                {
                                    if (nnnn->GetChildren() != nullptr)
                                    {
                                        name = nnnn->GetChildren()->GetContent().ToStdString();
                                    }
                                }
                                else if (nnnn->GetName() == "d2p1:CollectionType")
                                {
                                    if (nnnn->GetChildren() != nullptr)
                                    {
                                        type = nnnn->GetChildren()->GetContent().ToStdString();
                                    }
                                }
                                else if (nnnn->GetName() == "d2p1:Marks")
                                {
                                    float last = 0;
                                    for (wxXmlNode* nnnnn = nnnn->GetChildren(); nnnnn != nullptr; nnnnn = nnnnn->GetNext())
                                    {
                                        float duration = 0;
                                        float end = 0;
                                        std::string label = "";
                                        if (nnnnn->GetName() == "d1p1:anyType")
                                        {
                                            for (wxXmlNode* nnnnnn = nnnnn->GetChildren(); nnnnnn != nullptr; nnnnnn = nnnnnn->GetNext())
                                            {
                                                if (nnnnnn->GetName() == "d2p1:StartTime")
                                                {
                                                    wxString markTime = nnnnnn->GetChildren()->GetContent();
                                                    if (markTime.StartsWith("PT"))
                                                    {
                                                        markTime = markTime.AfterFirst('T');
                                                    }

                                                    float mins = 0;
                                                    if (markTime.Contains("M"))
                                                    {
                                                        mins = wxAtof(markTime.BeforeFirst('M'));
                                                        markTime = markTime.AfterFirst('M');
                                                    }

                                                    float secs = 0;
                                                    if (markTime.EndsWith("S"))
                                                    {
                                                        markTime = markTime.BeforeLast('S');
                                                        secs = wxAtof(markTime);
                                                    }

                                                    end = mins * 60 + secs;
                                                }
                                                else if (nnnnnn->GetName() == "d2p1:Duration")
                                                {
                                                    wxString markTime = nnnnnn->GetChildren()->GetContent();
                                                    if (markTime.StartsWith("PT"))
                                                    {
                                                        markTime = markTime.AfterFirst('T');
                                                    }

                                                    float mins = 0;
                                                    if (markTime.Contains("M"))
                                                    {
                                                        mins = wxAtof(markTime.BeforeFirst('M'));
                                                        markTime = markTime.AfterFirst('M');
                                                    }

                                                    float secs = 0;
                                                    if (markTime.EndsWith("S"))
                                                    {
                                                        markTime = markTime.BeforeLast('S');
                                                        secs = wxAtof(markTime);
                                                    }

                                                    duration = mins * 60 + secs;
                                                }
                                                else if (nnnnnn->GetName() == "d2p1:Text")
                                                {
                                                    if (nnnnnn->GetChildren() != nullptr)
                                                    {
                                                        label = nnnnnn->GetChildren()->GetContent().ToStdString();
                                                    }
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
                                                else
                                                {
                                                    //wxASSERT(false);
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
                                                    else
                                                    {
                                                        //wxASSERT(false);
                                                    }
                                                }
                                                else
                                                {
                                                    //wxASSERT(false);
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            if (name == "")
                            {
                                name = wxString::Format("Unnamed %d", unnamed++).ToStdString();
                            }
                            _timingData[name] = timing;
                            _timingType[name] = type;
                        }
                    }
                }
                else if (nn->GetName() == "_effectNodeSurrogates" && models.size() > 0)
                {
                    for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext())
                    {
                        if (nnn->GetName() == "EffectNodeSurrogate")
                        {
                            wxString effectSettingId;
                            wxString modelId;
                            float start = 0.0;
                            float duration = 0.0;
                            for (wxXmlNode* nnnn = nnn->GetChildren(); nnnn != nullptr; nnnn = nnnn->GetNext())
                            {
                                if (nnnn->GetName() == "InstanceId")
                                {
                                    effectSettingId = nnnn->GetChildren()->GetContent();
                                }
                                else if (nnnn->GetName() == "StartTime")
                                {
                                    auto markTime = nnnn->GetChildren()->GetContent();
                                    if (markTime.StartsWith("PT"))
                                    {
                                        markTime = markTime.AfterFirst('T');
                                    }
                                    float mins = 0;
                                    if (markTime.Contains('M'))
                                    {
                                        mins = wxAtof(markTime.BeforeFirst('M'));
                                        markTime = markTime.AfterFirst('M');
                                    }
                                    float secs = 0;
                                    if (markTime.EndsWith("S"))
                                    {
                                        secs = wxAtof(markTime.BeforeLast('S'));
                                    }
                                    start = mins * 60.0 + secs;
                                }
                                else if (nnnn->GetName() == "TimeSpan")
                                {
                                    auto markTime = nnnn->GetChildren()->GetContent();
                                    if (markTime.StartsWith("PT"))
                                    {
                                        markTime = markTime.AfterFirst('T');
                                    }
                                    float mins = 0;
                                    if (markTime.Contains('M'))
                                    {
                                        mins = wxAtof(markTime.BeforeFirst('M'));
                                        markTime = markTime.AfterFirst('M');
                                    }
                                    float secs = 0;
                                    if (markTime.EndsWith("S"))
                                    {
                                        secs = wxAtof(markTime.BeforeLast('S'));
                                    }
                                    duration = mins * 60.0 + secs;
                                }
                                else if (nnnn->GetName() == "TargetNodes")
                                {
                                    for (wxXmlNode* nnnnn = nnnn->GetChildren(); nnnnn != nullptr; nnnnn = nnnnn->GetNext())
                                    {
                                        if (nnnnn->GetName() == "ChannelNodeReferenceSurrogate")
                                        {
                                            for (wxXmlNode* nnnnnn = nnnnn->GetChildren(); nnnnnn != nullptr; nnnnnn = nnnnnn->GetNext())
                                            {
                                                if (nnnnnn->GetName() == "NodeId")
                                                {
                                                    modelId = nnnnnn->GetChildren()->GetContent();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            VixenEffect ve(start, start + duration, effectSettingId.ToStdString());

                            auto m = models.find(modelId.ToStdString());

                            if (m != models.end())
                            {
                                _effectData[m->second].push_back(ve);
                            }
                            else
                            {
                                logger_base.warn("Vixen3: model not found for effect. %s", (const char*)modelId.c_str());
                                wxASSERT(false);
                            }
                        }
                    }
                }
                else if (nn->GetName() == "_dataModels" && models.size() > 0)
                {
                    for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext())
                    {
                        if (nnn->GetName() == "d1p1:anyType")
                        {
                            auto type = nnn->GetAttribute("i:type", "");
                            if (type != "")
                            {
                                wxString id;
                                for (wxXmlNode* nnnn = nnn->GetChildren(); id == "" && nnnn != nullptr; nnnn = nnnn->GetNext())
                                {
                                    if (nnnn->GetName() == "ModuleInstanceId")
                                    {
                                        id = nnnn->GetChildren()->GetContent();
                                    }
                                }
                                effectSettings[id.ToStdString()] = nnn;
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
                wxString colorHandling;
                std::vector<std::vector<VixenColor>> palleteStatic;
                std::vector<std::vector<VixenColor>> palleteDefault;
                
                e->type = es->second->GetAttribute("i:type").AfterFirst(':').ToStdString();
                for (auto n = es->second->GetChildren(); n != nullptr; n = n->GetNext())
                {
                    if (n->GetName() == "TargetPositioning" && n->GetChildren() != nullptr) {
                        e->settings[n->GetName().ToStdString()] = n->GetChildren()->GetContent().ToStdString();
                    } else if (n->GetName().StartsWith("d2p1:") && n->GetChildren() != nullptr) {
                        auto nm = n->GetName().AfterFirst(':');
                        
                        if (nm == "NutcrackerData") {
                            for (auto nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                                auto nm2 = nn->GetName().AfterFirst(':');
                                if (nn->GetChildren() != nullptr) {
                                    // Process color palette
                                    if (nm2 == "Palette") {
                                        std::vector<int> knownColor;
                                        for (auto nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext()) {
                                            auto nm3 = nnn->GetName().AfterFirst(':');
                                            if (nnn->GetChildren() != nullptr) {
                                                if (nm3 == "_colors") {
                                                    for (auto nnnn = nnn->GetChildren(); nnnn != nullptr; nnnn = nnnn->GetNext()) {
                                                        auto nm4 = nnnn->GetName().AfterFirst(':');
                                                        if (nm4 == "Color") {
                                                            for (auto nnnnn = nnnn->GetChildren(); nnnnn != nullptr; nnnnn = nnnnn->GetNext()) {
                                                                auto nm5 = nnnnn->GetName().AfterFirst(':');
                                                                if (nm5 == "knownColor") {
                                                                    // Finally, got the color
                                                                    knownColor.push_back(wxAtoi(nnnnn->GetChildren()->GetContent()));
                                                                }
                                                            }
                                                        }
                                                    }
                                                } else if (nm3 == "_colorsActive") {
                                                    auto index = 0;
                                                    for (auto nnnn = nnn->GetChildren(); nnnn != nullptr; nnnn = nnnn->GetNext()) {
                                                        auto nm4 = nnnn->GetName().AfterFirst(':');
                                                        if (nm4 == "boolean") {
                                                            if (nnnn->GetChildren()->GetContent() == "true") {
                                                                uint32_t argb = VixenEffect::KNOWN_COLOR[knownColor.at(index)];
                                                                char r = (argb & 0x00FF0000) >> 16;
                                                                char g = (argb & 0x0000FF00) >> 8;
                                                                char b = (argb & 0x000000FF);
                                                                palleteDefault.push_back({ VixenColor(wxColor(r, g, b), 0) });
                                                            }
                                                        }
                                                        index++;
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        e->settings[nm2.ToStdString()] = nn->GetChildren()->GetContent().ToStdString();
                                    }
                                }
                            }
                        } else {
                            e->settings[nm.ToStdString()] = n->GetChildren()->GetContent().ToStdString();
                        }

                        // Color for SetLevel
                        if (nm == "color") {
                            int r = 0;
                            int g = 0;
                            int b = 0;
                            for (auto nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                                auto nm2 = nn->GetName().AfterFirst(':');
                                if (nm2 == "_r") {
                                    r = 255.0 * wxAtof(nn->GetChildren()->GetContent());
                                }
                                else if (nm2 == "_g") {
                                    g = 255.0 * wxAtof(nn->GetChildren()->GetContent());
                                }
                                else if (nm2 == "_b") {
                                    b = 255.0 * wxAtof(nn->GetChildren()->GetContent());
                                }
                            }
                            palleteDefault.push_back({ VixenColor(wxColor(r, g, b), 0) });
                        }

                        // ColorHandling known values:
                        // - StaticColor
                        // - GradientThroughWHoleEffect
                        else if (nm == "ColorHandling") {
                            colorHandling = n->GetChildren()->GetContent();
                        }

                        // Colors for the following:
                        // ColorGradient:      Chase Pulse Spin Twinkle Wipe
                        // Gradient:           ColorWash Border Butterfly Curtain Shockwave
                        // Head/Fill:          Morph
                        // MeterColorGradient: VetricalMeter VUMeter Waveform
                        else if (nm == "ColorGradient" || nm == "Gradient" || nm == "HeadColor" || nm == "FillColor" || nm == "MeterColorGradient") {
                            for (auto nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                                auto nm2 = nn->GetName().AfterFirst(':');
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
                            for (auto nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                                auto nm2 = nn->GetName().AfterFirst(':');
                                if (nm2 == "_colors") {
                                    palleteDefault.push_back(ProcessColorData(nn));
                                } else if (nm2 == "ColorGradient" || "GradientLevelPair") { 
                                    for (auto nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext()) {
                                        auto nm3 = nnn->GetName().AfterFirst(':');
                                        if (nm3 == "_colors") {
                                            palleteDefault.push_back(ProcessColorData(nnn));
                                        } else if (nm3 == "ColorGradient") {
                                            for (auto nnnn = nnn->GetChildren(); nnnn != nullptr; nnnn = nnnn->GetNext()) {
                                                auto nm4 = nnnn->GetName().AfterFirst(':');
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
                            for (auto nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                                auto nm2 = nn->GetName().AfterFirst(':');
                                if (nm2 == "value") {
                                    unsigned long argb;
                                    nn->GetChildren()->GetContent().ToULong(&argb);
                                    char r = (argb & 0x00FF0000) >> 16;
                                    char g = (argb & 0x0000FF00) >> 8;
                                    char b = (argb & 0x000000FF);
                                    palleteStatic.push_back({ VixenColor(wxColor(r, g, b), 0) });
                                }
                            }
                        }

                        // Brightness curve
                        else if (nm == "LevelCurve") {
                            for (auto nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                                auto nm2 = nn->GetName().AfterFirst(':');
                                if (nm2 == "Points") {
                                    for (auto nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext()) {
                                        auto nm3 = nnn->GetName().AfterFirst(':');
                                        if (nm3 == "PointPair") {
                                            double x = 0;
                                            double y = 0;
                                            for (auto nnnn = nnn->GetChildren(); nnnn != nullptr; nnnn = nnnn->GetNext()) {
                                                auto nm4 = nnnn->GetName();
                                                if (nm4 == "X") {
                                                    x = wxAtoi(nnnn->GetChildren()->GetContent());
                                                } else if (nm4 == "Y") {
                                                    y = wxAtoi(nnnn->GetChildren()->GetContent());
                                                }
                                            }
                                            e->levelCurve.push_back(wxRealPoint(x, y));
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

std::vector<VixenColor> Vixen3::ProcessColorData(wxXmlNode* n)
{
    std::vector<VixenColor> vColor;

    for (auto nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
        auto nm3 = nn->GetName().AfterFirst(':');
        if (nm3 == "ColorPoint") {
            double x = 0;
            double y = 0;
            double z = 0;
            double position = 0;

            for (auto nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext()) {
                auto nm4 = nnn->GetName().AfterFirst(':');
                if (nm4 == "_color") {
                    for (auto nnnn = nnn->GetChildren(); nnnn != nullptr; nnnn = nnnn->GetNext()) {
                        auto nm5 = nnnn->GetName().AfterFirst(':');
                        if (nm5 == "_x") {
                            x = wxAtof(nnnn->GetChildren()->GetContent());
                        } else if (nm5 == "_y") {
                            y = wxAtof(nnnn->GetChildren()->GetContent());
                        } else if (nm5 == "_z") {
                            z = wxAtof(nnnn->GetChildren()->GetContent());
                        }
                    }
                } else if (nm4 == "_position") {
                    position = wxAtof(nnn->GetChildren()->GetContent());
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
