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
        } else {
            res += "C_BUTTON_Palette" + n + "=Active=TRUE|Id=ID_BUTTON_Palette" + n + "|Values=";
            for (int ii = 0; ii < palette[i].size(); ii++) {
                res += wxString::Format("x=%.3f^c=#%02x%02x%02x",
                       palette[i][ii].position, palette[i][ii].color.Red(), palette[i][ii].color.Green(), palette[i][ii].color.Blue());
                if (ii != palette[i].size() - 1)
                    res += ";";
            }
            res += "|";
        }
        res += ",C_CHECKBOX_Palette" + n + "=1";
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

    if (type == "PlasmaData")
    {
    }
    else if (type == "TwinkleData")
    {
        res = "E_SLIDER_Twinkle_Steps=" + settings.at("AveragePulseTime") +
              ",E_SLIDER_Twinkle_Count=" + settings.at("AverageCoverage");
    }
    else if (type == "PulseData") {
        // Only two points, set them as start and end intensity.  If more than two, a brightness curve will be generated
        if (levelCurve.size() == 2) {
            res = "E_TEXTCTRL_Eff_On_Start=" + wxString::Format("%d", wxRound(levelCurve[0].y)) +
                  ",E_TEXTCTRL_Eff_On_End=" + wxString::Format("%d", wxRound(levelCurve[1].y));
        }
    }
    else if (type == "Data") {}
    else if (type == "SetLevelData") {}
    else if (type == "WipeData") {}
    else if (type == "AlternatingData") {}
    else if (type == "AudioData") {}
    else if (type == "MeteorsData") {}
    else if (type == "GarlandsData") {}
    else if (type == "PictureData") {}
    else if (type == "SpiralData") {}
    else if (type == "PinWheelData") {}
    else if (type == "ChaseData") {}
    else if (type == "CirclesData") {}
    else if (type == "BarsData") {}
    else if (type == "TextData") {}
    else if (type == "SpirographData") {}
    else if (type == "FireData") {}
    else if (type == "NutcrackerModuleData") {}
    else if (type == "ButterflyData")
    {
        res = "E_Butterfly_Chunks=" + settings.at("BackgroundChunks") +
            ",E_Butterfly_Skip=" + settings.at("BackgroundSkips") +
            ",E_SLIDER_Butterfly_Style=" + wxString(settings.at("ButterflyType")).AfterFirst('e').ToStdString() +
            ",E_CHOICE_Butterfly_Direction=" + (settings.at("Direction") == "Forward" ? "0" : "1");
    }
    else if (type == "SpinData") {}

    return res;
}

std::string VixenEffect::GetXLightsType() const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (type == "AlternatingData") return "Marquee";
    if (type == "CandleData") return "Candle";
    if (type == "ChaseData") return "Single Strand";
    if (type == "DissolveData")
    {
        logger_base.warn("Vixen3: Unable to convert DissolveData effect ... inserting an off effect.");
        return "Off"; // not sure what to do with this
    }
    if (type == "LipSyncData") return "Faces";
    if (type == "PulseData") return "On";
    if (type == "SetLevelData") return "On";
    if (type == "SpinData")
    {
        logger_base.warn("Vixen3: Unable to convert SpinData effect ... inserting an Pinwheel effect.");
        return "Pinwheel";
    }
    if (type == "StrobeData") return "Strobe";
    if (type == "TwinkleData") return "Twinkle";
    if (type == "WipeData") return "Color Wash";
    if (type == "BarsData") return "Bars";
    if (type == "BorderData") return "Marquee";
    if (type == "ButterflyData") return "Butterfly";
    if (type == "CirclesData") return "Circles";
    if (type == "ColorWashData") return "Color Wash";
    if (type == "CurtainData") return "Curtain";
    if (type == "FireData") return "Fire";
    if (type == "FireworksData") return "Fireworks";
    if (type == "GarlandsData") return "Garlands";
    if (type == "GlediatorData") return "Glediator";
    if (type == "LifeData") return "Life";
    if (type == "LiquidData") return "Liquid";
    if (type == "MeteorsData") return "Meteors";
    if (type == "PictureData") return "Pictures";
    if (type == "PinWheelData") return "Pinwheel";
    if (type == "PlasmaData") return "Plasma";
    if (type == "ShapesData") return "Shape";
    if (type == "ShockwaveData") return "Shockwave";
    if (type == "SnowflakesData") return "Snowflakes";
    if (type == "SnowStormData") return "Snowstorm";
    if (type == "SpiralData") return "Spirals";
    if (type == "SpirographData") return "Spirograph";
    if (type == "TextData") return "Text";
    if (type == "TreeData") return "Tree";
    if (type == "VideoData") return "Video";
    if (type == "VUMeterData") return "VU Meter";
    if (type == "WaveData") return "Wave";
    
    if (type == "Data")
    {
        logger_base.warn("Vixen3: Unable to convert Data effect ... inserting an On effect.");
        return "On"; // this should go to timing
    }
    if (type == "NutcrackerModuleData")
    {
        logger_base.warn("Vixen3: Unable to convert NutcrackerModuleData effect ... inserting an off effect.");
        return "Off"; // not sure what to do with this
    }
    

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
    if (system == "" || !wxFile::Exists(_systemFile))
    {
        wxFileName seq(_filename);
        _systemFile = seq.GetPath() + wxFileName::GetPathSeparator() + "SystemConfig.xml";
        logger_base.debug("Looking for Vixen SystemConfig in %s", (const char*)_systemFile.c_str());

        if (!wxFile::Exists(_systemFile))
        {
            _systemFile = seq.GetPath() + wxFileName::GetPathSeparator() + "SystemData" + wxFileName::GetPathSeparator() + "SystemConfig.xml";
            logger_base.debug("Looking for Vixen SystemConfig in %s", (const char*)_systemFile.c_str());
        }

        if (!wxFile::Exists(_systemFile))
        {
            int lastFolder = seq.GetPath().Last(wxFileName::GetPathSeparator());
            _systemFile = seq.GetPath().Left(lastFolder) + wxFileName::GetPathSeparator() + "SystemData" + wxFileName::GetPathSeparator() + "SystemConfig.xml";
            logger_base.debug("Looking for Vixen SystemConfig in %s", (const char*)_systemFile.c_str());
        }

        if (!wxFile::Exists(_systemFile))
        {
            logger_base.debug("Looking for Vixen SystemConfig ... FAILED ... NOT FOUND");
            _systemFound = false;
        }
    }

    std::map<std::string, std::string> models;

    if (wxFile::Exists(_systemFile))
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
                e->type = es->second->GetAttribute("i:type").AfterFirst(':').ToStdString();
                for (auto n = es->second->GetChildren(); n != nullptr; n = n->GetNext())
                {
                    if (n->GetName().StartsWith("d2p1:") && n->GetChildren() != nullptr) {
                        auto nm = n->GetName().AfterFirst(':');
                        e->settings[nm.ToStdString()] = n->GetChildren()->GetContent().ToStdString();
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
                            e->palette.push_back({ VixenColor(wxColor(r, g, b), 0) });
                        }

                        // Single color gradient
                        else if (nm == "ColorGradient") {
                            for (auto nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                                auto nm2 = nn->GetName().AfterFirst(':');
                                if (nm2 == "_colors") {
                                    e->palette.push_back(GetColorData(nn));
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
            }
        }
    }
}

std::vector<VixenColor> Vixen3::GetColorData(wxXmlNode* n)
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

std::list<std::string> Vixen3::GetModelsWithEffects() const
{
    std::list<std::string> res;

    for (auto it: _effectData)
    {
        if (it.second.size() > 0)
        {
            res.push_back(it.first);
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
