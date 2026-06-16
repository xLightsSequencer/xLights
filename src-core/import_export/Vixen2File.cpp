/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "Vixen2File.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <string_view>

#include "ImportDataRow.h"
#include "render/EffectLayer.h"
#include "utils/ExternalHooks.h"
#include "utils/string_utils.h"

namespace {

bool VecContains(const std::vector<std::string>& v, const std::string& s)
{
    return std::find(v.begin(), v.end(), s) != v.end();
}

// Mirrors desktop CheckForVixenRGB — given a channel name that ends in a colour
// leg and whose two siblings exist, return the collapsed base name (else empty)
// and the leg colour.
std::string VixenRGBBase(const std::string& name, const std::vector<std::string>& all, xlColor& legColor)
{
    if (EndsWith(name, "Red") || EndsWith(name, "-R")) {
        legColor = xlRED;
        std::string base = EndsWith(name, "-R") ? name.substr(0, name.size() - 2) : name.substr(0, name.size() - 3);
        if ((VecContains(all, base + "Blue") && VecContains(all, base + "Green")) || (VecContains(all, base + "-B") && VecContains(all, base + "-G"))) {
            return base;
        }
    } else if (EndsWith(name, "Blue") || EndsWith(name, "-B")) {
        legColor = xlBLUE;
        std::string base = EndsWith(name, "-B") ? name.substr(0, name.size() - 2) : name.substr(0, name.size() - 4);
        if ((VecContains(all, base + "Red") && VecContains(all, base + "Green")) || (VecContains(all, base + "-R") && VecContains(all, base + "-G"))) {
            return base;
        }
    } else if (EndsWith(name, "Green") || EndsWith(name, "-G")) {
        legColor = xlGREEN;
        std::string base = EndsWith(name, "-G") ? name.substr(0, name.size() - 2) : name.substr(0, name.size() - 5);
        if ((VecContains(all, base + "Blue") && VecContains(all, base + "Red")) || (VecContains(all, base + "-B") && VecContains(all, base + "-R"))) {
            return base;
        }
    }
    return std::string();
}

} // namespace

Vixen2File::Vixen2File(pugi::xml_document& input_xml, const std::string& vixDir, const std::string& profileDir)
{
    pugi::xml_node root = input_xml.document_element();
    if (!root) {
        return;
    }

    pugi::xml_node periodNode = root.child("EventPeriodInMilliseconds");
    if (periodNode) {
        _frameTime = (int)std::strtol(periodNode.child_value(), nullptr, 10);
    }
    if (_frameTime <= 0) {
        _frameTime = 50;
    }

    int totalMs = 0;
    pugi::xml_node timeNode = root.child("Time");
    if (timeNode) {
        totalMs = (int)std::strtol(timeNode.child_value(), nullptr, 10);
    }
    _frames = (int)std::ceil((float)totalMs / (float)_frameTime);

    std::vector<std::string> rawNames;
    std::vector<xlColor> rawColors;

    pugi::xml_node profileNode = root.child("Profile");
    if (profileNode) {
        std::string profileName = profileNode.child_value();
        LoadProfile(profileName, vixDir, profileDir, rawNames, rawColors);
    }

    pugi::xml_node channelsNode = root.child("Channels");
    if (channelsNode) {
        for (pugi::xml_node channel : channelsNode.children("Channel")) {
            int chanColor = (int)std::strtol(channel.attribute("color").value(), nullptr, 10) & 0xFFFFFF;
            std::string nameAttr = channel.attribute("name").value();
            if (nameAttr.empty()) {
                nameAttr = channel.child_value();
            }
            if (!nameAttr.empty()) {
                rawNames.push_back(nameAttr);
                rawColors.push_back(xlColor(chanColor, false));
            }
        }
    }

    _rawChannels = rawNames;

    pugi::xml_node eventValuesNode = root.child("EventValues");
    if (eventValuesNode) {
        std::string ev = eventValuesNode.child_value();
        Base64Decode(ev, _eventData);
    }

    // Build the collapsed discovery list — RGB legs fold into one base name.
    for (size_t i = 0; i < rawNames.size(); ++i) {
        const std::string& name = rawNames[i];
        xlColor legColor = rawColors[i];
        std::string base = VixenRGBBase(name, rawNames, legColor);
        if (!base.empty()) {
            RegisterChannel(base, xlBLACK);
        } else {
            RegisterChannel(name, rawColors[i]);
        }
    }

    std::sort(_channelNames.begin(), _channelNames.end(), stdlistNumberAwareStringCompare);

    _valid = !_channelNames.empty() && _frames > 0 && !_eventData.empty();
}

void Vixen2File::RegisterChannel(const std::string& name, const xlColor& color)
{
    if (!VecContains(_channelNames, name)) {
        _channelNames.push_back(name);
        _channelColors[name] = color;
    }
}

int Vixen2File::RawChannelIndex(const std::string& name) const
{
    auto it = std::find(_rawChannels.begin(), _rawChannels.end(), name);
    if (it == _rawChannels.end()) {
        return -1;
    }
    return (int)std::distance(_rawChannels.begin(), it);
}

xlColor Vixen2File::GetChannelColor(const std::string& name) const
{
    auto it = _channelColors.find(name);
    if (it != _channelColors.end()) {
        return it->second;
    }
    return xlWHITE;
}

bool Vixen2File::LoadProfile(const std::string& profileName, const std::string& vixDir, const std::string& profileDir,
                             std::vector<std::string>& names, std::vector<xlColor>& colors) const
{
    std::filesystem::path fn = std::filesystem::path(profileDir) / (profileName + ".pro");
    if (!FileExists(fn.string())) {
        fn = std::filesystem::path(vixDir) / (profileName + ".pro");
    }
    if (!FileExists(fn.string())) {
        return false;
    }
    pugi::xml_document doc;
    if (!doc.load_file(fn.string().c_str())) {
        return false;
    }
    pugi::xml_node root = doc.document_element();
    for (pugi::xml_node e = root.first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "ChannelObjects") {
            for (pugi::xml_node p = e.first_child(); p; p = p.next_sibling()) {
                if (std::string_view(p.name()) == "Channel") {
                    if (p.attribute("name")) {
                        names.push_back(p.attribute("name").as_string());
                    } else {
                        names.push_back(p.text().get());
                    }
                    if (p.attribute("color")) {
                        int chanColor = p.attribute("color").as_int() & 0xFFFFFF;
                        colors.push_back(xlColor(chanColor, false));
                    } else {
                        colors.push_back(xlWHITE);
                    }
                }
            }
        }
    }
    return true;
}

bool Vixen2File::MapChannelEffects(EffectLayer* layer, const std::string& name, const xlColor& color, bool eraseExisting) const
{
    if (name.empty() || layer == nullptr || _frames <= 0) {
        return false;
    }

    xlColorVector colors(_frames);

    int channel = RawChannelIndex(name);
    if (channel < 0) {
        int rchannel = RawChannelIndex(name + "Red");
        if (rchannel < 0) rchannel = RawChannelIndex(name + "-R");
        int gchannel = RawChannelIndex(name + "Green");
        if (gchannel < 0) gchannel = RawChannelIndex(name + "-G");
        int bchannel = RawChannelIndex(name + "Blue");
        if (bchannel < 0) bchannel = RawChannelIndex(name + "-B");
        if (rchannel < 0 || gchannel < 0 || bchannel < 0) {
            return false;
        }
        for (int x = 0; x < _frames; x++) {
            int ri = x + _frames * rchannel;
            int gi = x + _frames * gchannel;
            int bi = x + _frames * bchannel;
            unsigned char r = ri < (int)_eventData.size() ? _eventData[ri] : 0;
            unsigned char g = gi < (int)_eventData.size() ? _eventData[gi] : 0;
            unsigned char b = bi < (int)_eventData.size() ? _eventData[bi] : 0;
            colors[x].Set(r, g, b);
        }
    } else {
        xlColor c(color.Red(), color.Green(), color.Blue());
        HSVValue hsv = c.asHSV();
        for (int x = 0; x < _frames; x++) {
            int idx = x + _frames * channel;
            unsigned char v = idx < (int)_eventData.size() ? _eventData[idx] : 0;
            hsv.value = ((double)v) / 255.0;
            colors[x] = hsv;
        }
    }

    ConvertDataRowToEffects(layer, colors, _frameTime, eraseExisting);
    return true;
}
