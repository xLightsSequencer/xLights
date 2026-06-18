/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LORMusic.h"

#include <algorithm>
#include <cstdlib>
#include <string_view>

#include "render/EffectLayer.h"
#include "render/RenderUtils.h"
#include "utils/UtilFunctions.h"

namespace {

xlColor LMSGetColor(const std::string& rgb)
{
    int i = (int)std::strtol(rgb.c_str(), nullptr, 10);
    xlColor cl;
    cl.red = (i & 0xff);
    cl.green = ((i >> 8) & 0xFF);
    cl.blue = ((i >> 16) & 0xff);
    return cl;
}

std::string Itoa(int v)
{
    return std::to_string(v);
}

bool findRGB(pugi::xml_node e, pugi::xml_node chan, pugi::xml_node& rchannel, pugi::xml_node& gchannel, pugi::xml_node& bchannel)
{
    std::string idxs[3];
    int cnt = 0;
    for (pugi::xml_node n = chan.first_child(); n; n = n.next_sibling()) {
        if (std::string_view(n.name()) == "channels") {
            for (pugi::xml_node n2 = n.first_child(); n2; n2 = n2.next_sibling()) {
                if (std::string_view(n2.name()) == "channel" && cnt < 3) {
                    idxs[cnt] = n2.attribute("savedIndex").as_string();
                    cnt++;
                }
            }
        }
    }
    for (pugi::xml_node ch = e.first_child(); ch; ch = ch.next_sibling()) {
        if (std::string_view(ch.name()) == "channel") {
            std::string idx = ch.attribute("savedIndex").as_string();
            if (idx == idxs[0]) {
                rchannel = ch;
            }
            if (idx == idxs[1]) {
                gchannel = ch;
            }
            if (idx == idxs[2]) {
                bchannel = ch;
            }
        }
    }
    return true;
}

void GetRGBTimes(pugi::xml_node re, int& startms, int& endms)
{
    if (re) {
        startms = re.attribute("startCentisecond").as_int() * 10;
        endms = re.attribute("endCentisecond").as_int() * 10;
    } else {
        startms = 9999999;
        endms = 9999999;
    }
}

void GetIntensities(pugi::xml_node re, int& starti, int& endi)
{
    int intensity = re.attribute("intensity").as_int(-1);
    if (intensity == -1) {
        starti = re.attribute("startIntensity").as_int();
        endi = re.attribute("endIntensity").as_int();
    } else {
        starti = endi = intensity;
    }
}

struct RGBData
{
    int startms, endms;
    int starti, endi;
    bool shimmer;
};

void FillData(pugi::xml_node nd, RGBData& data)
{
    GetIntensities(nd, data.starti, data.endi);
    GetRGBTimes(nd, data.startms, data.endms);
    data.shimmer = std::string_view(nd.attribute("type").as_string()) == "shimmer";
}

void Insert(int x, std::vector<RGBData>& v, int startms)
{
    v.insert(v.begin() + x, 1, RGBData());
    v[x].startms = startms;
    v[x].endms = v[x + 1].startms;
    v[x].endi = v[x].starti = 0;
    v[x].shimmer = false;
}

void Split(int x, std::vector<RGBData>& v, int endms)
{
    v.insert(v.begin() + x, 1, RGBData());
    v[x].startms = v[x + 1].startms;
    v[x].endms = endms;
    v[x + 1].startms = endms;
    v[x].shimmer = v[x + 1].shimmer;
    v[x].starti = v[x + 1].starti;
    double d = endms - v[x].startms;
    d = d / double(v[x + 1].endms - v[x].startms);
    double newi = (v[x + 1].endi - v[x].starti) * d + v[x].starti;
    v[x].endi = v[x + 1].starti = newi;
}

#define MAXMS 99999999
int GetStartMS(int x, std::vector<RGBData>& v)
{
    if (x < (int)v.size()) {
        return v[x].startms;
    }
    return MAXMS;
}
int GetEndMS(int x, std::vector<RGBData>& v)
{
    if (x < (int)v.size()) {
        return v[x].endms;
    }
    return MAXMS;
}
void Resize(int x, std::vector<RGBData>& v, int startms)
{
    while (x >= (int)v.size()) {
        int i = v.size();
        v.push_back(RGBData());
        v[i].endms = MAXMS;
        v[i].startms = startms;
        v[i].starti = v[i].endi = 0;
        v[i].shimmer = false;
    }
}

void UnifyData(int x, std::vector<RGBData>& red, std::vector<RGBData>& green, std::vector<RGBData>& blue)
{
    int min = std::min(GetStartMS(x, red), std::min(GetStartMS(x, green), GetStartMS(x, blue)));
    Resize(x, red, min);
    Resize(x, green, min);
    Resize(x, blue, min);
    if (red[x].startms != min) {
        Insert(x, red, min);
    }
    if (green[x].startms != min) {
        Insert(x, green, min);
    }
    if (blue[x].startms != min) {
        Insert(x, blue, min);
    }
    min = std::min(GetEndMS(x, red), std::min(GetEndMS(x, green), GetEndMS(x, blue)));
    if (min == MAXMS) {
        return;
    }
    if (red[x].endms != min) {
        Split(x, red, min);
    }
    if (green[x].endms != min) {
        Split(x, green, min);
    }
    if (blue[x].endms != min) {
        Split(x, blue, min);
    }
}

bool GetRGBEffectData(RGBData& red, RGBData& green, RGBData& blue, xlColor& sc, xlColor& ec)
{
    sc.red = red.starti * 255 / 100;
    sc.green = green.starti * 255 / 100;
    sc.blue = blue.starti * 255 / 100;

    ec.red = red.endi * 255 / 100;
    ec.green = green.endi * 255 / 100;
    ec.blue = blue.endi * 255 / 100;

    return red.shimmer | blue.shimmer | green.shimmer;
}

void LoadRGBData(EffectLayer* layer, pugi::xml_node rchannel, pugi::xml_node gchannel, pugi::xml_node bchannel)
{
    std::vector<RGBData> red, green, blue;
    while (rchannel) {
        int startms, endms;
        GetRGBTimes(rchannel, startms, endms);
        if (startms < endms) {
            red.resize(red.size() + 1);
            FillData(rchannel, red[red.size() - 1]);
        }
        rchannel = rchannel.next_sibling();
    }
    while (gchannel) {
        int startms, endms;
        GetRGBTimes(gchannel, startms, endms);
        if (startms < endms) {
            green.resize(green.size() + 1);
            FillData(gchannel, green[green.size() - 1]);
        }
        gchannel = gchannel.next_sibling();
    }
    while (bchannel) {
        int startms, endms;
        GetRGBTimes(bchannel, startms, endms);
        if (startms < endms) {
            blue.resize(blue.size() + 1);
            FillData(bchannel, blue[blue.size() - 1]);
        }
        bchannel = bchannel.next_sibling();
    }
    // have the data, now need to split it so common start/end times
    for (size_t x = 0; x < red.size() || x < green.size() || x < blue.size(); x++) {
        UnifyData(x, red, green, blue);
    }

    for (size_t x = 0; x < red.size() || x < green.size() || x < blue.size(); x++) {
        xlColor sc, ec;
        bool isShimmer = GetRGBEffectData(red[x], green[x], blue[x], sc, ec);

        int starttime = red[x].startms;
        int endtime = red[x].endms;

        if (ec == sc) {
            if (ec != xlBLACK) {
                std::string palette = "C_BUTTON_Palette1=" + (std::string)sc + ",C_CHECKBOX_Palette1=1," + "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0";
                std::string settings = (isShimmer ? "E_CHECKBOX_On_Shimmer=1" : "");
                layer->AddEffect(0, "On", settings, palette, starttime, endtime, false, false);
            }
        } else if (sc == xlBLACK) {
            std::string palette = "C_BUTTON_Palette1=" + (std::string)ec + ",C_CHECKBOX_Palette1=1,"
                                                                           "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0";
            std::string settings = "E_TEXTCTRL_Eff_On_Start=0";
            if (isShimmer) {
                settings += ",E_CHECKBOX_On_Shimmer=1";
            }
            layer->AddEffect(0, "On", settings, palette, starttime, endtime, false, false);
        } else if (ec == xlBLACK) {
            std::string palette = "C_BUTTON_Palette1=" + (std::string)sc + ",C_CHECKBOX_Palette1=1,"
                                                                           "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0";
            std::string settings = "E_TEXTCTRL_Eff_On_End=0";
            if (isShimmer) {
                settings += ",E_CHECKBOX_On_Shimmer=1";
            }
            layer->AddEffect(0, "On", settings, palette, starttime, endtime, false, false);
        } else {
            std::string palette = "C_BUTTON_Palette1=" + (std::string)sc + ",C_CHECKBOX_Palette1=1,"
                                                                           "C_BUTTON_Palette2=" +
                                  (std::string)ec + ",C_CHECKBOX_Palette2=1";
            std::string settings = (isShimmer ? "E_CHECKBOX_ColorWash_Shimmer=1," : "");
            layer->AddEffect(0, "Color Wash", settings, palette, starttime, endtime, false, false);
        }
    }
}

void MapRGBEffects(EffectLayer* layer, pugi::xml_node rchannel, pugi::xml_node gchannel, pugi::xml_node bchannel)
{
    pugi::xml_node re = rchannel.first_child();
    while (re && std::string_view(re.name()) != "effect")
        re = re.next_sibling();
    pugi::xml_node ge = gchannel.first_child();
    while (ge && std::string_view(ge.name()) != "effect")
        ge = ge.next_sibling();
    pugi::xml_node be = bchannel.first_child();
    while (be && std::string_view(be.name()) != "effect")
        be = be.next_sibling();
    LoadRGBData(layer, re, ge, be);
}

std::string Scale255To100(const std::string& s, bool doscale)
{
    if (doscale) {
        int v = (int)std::strtol(s.c_str(), nullptr, 10);
        return Itoa(v * 100 / 255);
    }
    return s;
}

void MapOnEffects(EffectLayer* layer, pugi::xml_node channel, int chancountpernode, const xlColor& color)
{
    std::string palette = "C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1";
    if (chancountpernode > 1) {
        palette = "C_BUTTON_Palette1=" + (std::string)color + ",C_CHECKBOX_Palette1=1";
    }

    for (pugi::xml_node ch = channel.first_child(); ch; ch = ch.next_sibling()) {
        if (std::string_view(ch.name()) == "effect") {
            std::string type = ch.attribute("type").as_string();
            bool doscale = (type == "DMX intensity");
            int starttime = ch.attribute("startCentisecond").as_int() * 10;
            int endtime = ch.attribute("endCentisecond").as_int() * 10;
            std::string intensity = ch.attribute("intensity").as_string("-1");
            std::string starti, endi;
            if (intensity == "-1") {
                starti = Scale255To100(ch.attribute("startIntensity").as_string(), doscale);
                endi = Scale255To100(ch.attribute("endIntensity").as_string(), doscale);
            } else {
                starti = endi = Scale255To100(intensity, doscale);
            }

            if (type == "twinkle") {
                std::string efpalette = palette;
                int steps = std::max((float)(endtime - starttime - 1000) / 100.0f, 0.0f) + (rand01() * 10.0 - 5.0);
                steps = std::max(steps, 2);
                steps = std::min(steps, 200);
                std::string settings = "E_CHECKBOX_Twinkle_Strobe=1,E_SLIDER_Twinkle_Count=2,E_SLIDER_Twinkle_Steps=" + Itoa(steps);

                if (starti == endi) {
                    if (starti != "100") {
                        efpalette += ",C_SLIDER_Brightness=" + starti;
                    }
                } else {
                    efpalette += ",C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=" + starti + ".00|P2=" + endi + ".00|RV=TRUE|";
                }

                layer->AddEffect(0, "Twinkle", settings, efpalette, starttime, endtime, false, false);
            } else {
                std::string settings;
                if ("100" != starti) {
                    settings += "E_TEXTCTRL_Eff_On_Start=" + starti;
                }
                if ("100" != endi) {
                    if (!settings.empty()) {
                        settings += ",";
                    }
                    settings += "E_TEXTCTRL_Eff_On_End=" + endi;
                }
                if (type != "intensity" && type != "DMX intensity") {
                    if (!settings.empty()) {
                        settings += ",";
                    }
                    settings += "E_CHECKBOX_On_Shimmer=1";
                }
                layer->AddEffect(0, "On", settings, palette, starttime, endtime, false, false);
            }
        }
    }
}

} // namespace

LORMusic::LORMusic(pugi::xml_document& input_xml, int frequency) :
    _input_xml(input_xml), _frequency(frequency)
{
    Index();
}

void LORMusic::Index()
{
    for (pugi::xml_node e = _input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "channels") {
            for (pugi::xml_node chan = e.first_child(); chan; chan = chan.next_sibling()) {
                std::string chanName = chan.name();
                if (chanName == "channel" || chanName == "rgbChannel") {
                    std::string name = chan.attribute("name").as_string();
                    if (chanName == "rgbChannel") {
                        _channelColors[name] = xlBLACK;
                    } else {
                        std::string color = chan.attribute("color").as_string();
                        std::string unit = chan.attribute("unit").as_string();
                        std::string circuit = chan.attribute("circuit").as_string();
                        if (std::find(_channelNames.begin(), _channelNames.end(), name) != _channelNames.end()) {
                            name += "_Unit_" + unit + "_Circuit_" + circuit;
                        }
                        _channelColors[name] = LMSGetColor(color);
                    }

                    bool ccr = false;
                    if (chanName == "rgbChannel") {
                        int idxDP = name.find("-P");
                        int idxUP = name.find(" P");
                        int idxSP = name.find(" p");
                        if (idxUP > idxSP) {
                            idxSP = idxUP;
                        }
                        if (idxDP > idxSP) {
                            idxSP = idxDP;
                        }
                        if (idxSP != (int)std::string::npos) {
                            int i = (int)std::strtol(name.substr(idxSP + 2).c_str(), nullptr, 10);
                            if (i > 0 && name != "") {
                                ccr = true;
                                _channelNames.push_back(name);
                                if (name.substr(0, idxSP) != "" && std::find(_ccrNames.begin(), _ccrNames.end(), name.substr(0, idxSP)) == _ccrNames.end()) {
                                    _ccrNames.push_back(name.substr(0, idxSP));
                                }
                            }
                        }
                    }

                    if (!ccr && name != "") {
                        _channelNames.push_back(name);
                    }
                }
            }
        } else if (std::string_view(e.name()) == "timingGrids") {
            for (pugi::xml_node timing = e.first_child(); timing; timing = timing.next_sibling()) {
                if (std::string_view(timing.name()) == "timingGrid") {
                    std::string type = timing.attribute("type").as_string();
                    if (type != "fixed") {
                        std::string name = timing.attribute("name").as_string();
                        if (name != "") {
                            _timingTrackNames.push_back(name);
                            _timingTracks[name] = timing;
                        }
                    }
                }
            }
        }
    }

    std::sort(_channelNames.begin(), _channelNames.end(), stdlistNumberAwareStringCompare);
    std::sort(_ccrNames.begin(), _ccrNames.end(), stdlistNumberAwareStringCompare);
    // CCR prefix names live in _ccrNames; remove the raw per-pixel channels
    // from the flat available list so only the collapsed prop name is offered.
    _channelNames.erase(std::remove_if(_channelNames.begin(), _channelNames.end(),
                                       [this](const std::string& n) { return IsCCRName(n); }),
                        _channelNames.end());
}

bool LORMusic::IsCCRName(const std::string& name) const
{
    for (const auto& prefix : _ccrNames) {
        if (name.size() > prefix.size() && name.compare(0, prefix.size(), prefix) == 0) {
            return true;
        }
    }
    return false;
}

xlColor LORMusic::GetChannelColor(const std::string& name) const
{
    auto it = _channelColors.find(name);
    if (it != _channelColors.end()) {
        return it->second;
    }
    return xlBLACK;
}

std::vector<std::string> LORMusic::GetTimingTracks() const
{
    return _timingTrackNames;
}

std::vector<std::pair<uint32_t, uint32_t>> LORMusic::GetTimings(const std::string& timingTrackName, int offset) const
{
    std::vector<std::pair<uint32_t, uint32_t>> res;
    auto it = _timingTracks.find(timingTrackName);
    if (it == _timingTracks.end()) {
        return res;
    }
    long last = offset;
    for (pugi::xml_node t = it->second.first_child(); t; t = t.next_sibling()) {
        if (std::string_view(t.name()) == "timing") {
            int time = t.attribute("centisecond").as_int() * 10 + offset;
            int adjTime = RoundToMultipleOfPeriod(time, _frequency);
            if (adjTime > last) {
                res.push_back({ (uint32_t)last, (uint32_t)adjTime });
                last = adjTime;
            }
        }
    }
    return res;
}

bool LORMusic::ChannelExists(const std::string& name) const
{
    return std::find(_channelNames.begin(), _channelNames.end(), name) != _channelNames.end();
}

std::string LORMusic::ResolveCCRNodeName(const std::string& ccrName, int node) const
{
    char buf[64];
    snprintf(buf, sizeof(buf), "-P%02d", node + 1);
    std::string nm = ccrName + buf;
    if (ChannelExists(nm)) return nm;
    snprintf(buf, sizeof(buf), " p%02d", node + 1);
    nm = ccrName + buf;
    if (ChannelExists(nm)) return nm;
    snprintf(buf, sizeof(buf), "-P%d", node + 1);
    nm = ccrName + buf;
    if (ChannelExists(nm)) return nm;
    snprintf(buf, sizeof(buf), " p%d", node + 1);
    nm = ccrName + buf;
    if (ChannelExists(nm)) return nm;
    snprintf(buf, sizeof(buf), " P %02d", node + 1);
    nm = ccrName + buf;
    if (ChannelExists(nm)) return nm;
    snprintf(buf, sizeof(buf), "-P%02d", node + 1);
    return ccrName + buf;
}

bool LORMusic::MapChannelEffects(EffectLayer* layer, const std::string& name, const xlColor& color, int chanCountPerNode, bool eraseExisting) const
{
    if (name.empty() || layer == nullptr) {
        return false;
    }

    if (eraseExisting) {
        layer->DeleteAllEffects();
    }

    pugi::xml_node channel;
    pugi::xml_node rchannel;
    pugi::xml_node gchannel;
    pugi::xml_node bchannel;
    for (pugi::xml_node e = _input_xml.document_element().first_child(); !channel && e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "channels") {
            for (pugi::xml_node chan = e.first_child(); !channel && chan; chan = chan.next_sibling()) {
                std::string unit = chan.attribute("unit").as_string();
                std::string circuit = chan.attribute("circuit").as_string();
                std::string dedupname = std::string(chan.attribute("name").as_string()) + "_Unit_" + unit + "_Circuit_" + circuit;
                std::string chanName = chan.name();
                if ((chanName == "channel" || chanName == "rgbChannel") && (name == chan.attribute("name").as_string() || name == dedupname)) {
                    channel = chan;
                    if (chanName == "rgbChannel" && !findRGB(e, chan, rchannel, gchannel, bchannel)) {
                        return false;
                    }
                    break;
                }
            }
        }
    }
    if (!channel) {
        return false;
    }
    if (std::string_view(channel.name()) == "rgbChannel") {
        MapRGBEffects(layer, rchannel, gchannel, bchannel);
    } else {
        MapOnEffects(layer, channel, chanCountPerNode, color);
    }
    return true;
}
