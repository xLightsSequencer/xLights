/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "HLSFile.h"

#include <algorithm>
#include <cstdlib>
#include <string_view>

#include <spdlog/fmt/fmt.h>

#include "ImportDataRow.h"
#include "render/EffectLayer.h"
#include "utils/UtilFunctions.h"

namespace {

// HLS strand names look like "Name_0001" or "Name_S###P###" — collapse the
// per-pixel channels to a single strand prefix. Mirrors desktop
// MapToStrandName.
void MapToStrandName(const std::string& name, std::vector<std::string>& strands)
{
    if (name.find("_") != std::string::npos) {
        int idx = (int)name.find("_") + 1;

        int ppos = -1;
        int spos = -1;
        for (int x = idx; x < (int)name.size(); x++) {
            if (name[x] == 'P') {
                ppos = x;
            } else if (name[x] == 'S') {
                spos = x;
            } else if (name[x] < '0' || name[x] > '9') {
                return;
            }
        }
        std::string strandName;
        if (spos == -1 && ppos == -1) {
            strandName = name.substr(0, name.find("_") - 1);
        } else if (spos >= 0 && ppos > spos) {
            strandName = name.substr(0, ppos - 1);
        }
        if (strandName != "" && std::find(strands.begin(), strands.end(), strandName) == strands.end()) {
            strands.push_back(strandName);
        }
    }
}

void ReadHLSData(pugi::xml_node chand, std::vector<unsigned char>& data)
{
    for (pugi::xml_node chani = chand.first_child(); chani; chani = chani.next_sibling()) {
        if (std::string_view(chani.name()) == "IlluminationData") {
            for (pugi::xml_node block = chani.first_child(); block; block = block.next_sibling()) {
                std::string vals = block.text().get();
                auto dash = vals.find('-');
                int offset = (int)std::strtol(vals.substr(0, dash).c_str(), nullptr, 10);
                vals = (dash == std::string::npos) ? std::string() : vals.substr(dash + 1);
                while (!vals.empty()) {
                    auto comma = vals.find(',');
                    std::string v = vals.substr(0, comma);
                    vals = (comma == std::string::npos) ? std::string() : vals.substr(comma + 1);
                    long iv = std::strtol(v.c_str(), nullptr, 16);
                    if (offset >= 0 && offset < (int)data.size()) {
                        data[offset] = (unsigned char)iv;
                    }
                    offset++;
                }
            }
        }
    }
}

} // namespace

HLSFile::HLSFile(pugi::xml_document& input_xml) :
    _input_xml(input_xml)
{
    Index();
}

void HLSFile::Index()
{
    for (pugi::xml_node tuniv = _input_xml.document_element().first_child(); tuniv; tuniv = tuniv.next_sibling()) {
        if (std::string_view(tuniv.name()) == "NumberOfTimeCells") {
            _frames = tuniv.text().as_int();
        } else if (std::string_view(tuniv.name()) == "MilliSecPerTimeUnit") {
            _frameTime = tuniv.text().as_int();
        } else if (std::string_view(tuniv.name()) == "TotalUniverses") {
            _totalUniverses = tuniv;
            for (pugi::xml_node univ = tuniv.first_child(); univ; univ = univ.next_sibling()) {
                if (std::string_view(univ.name()) == "Universe") {
                    for (pugi::xml_node channels = univ.first_child(); channels; channels = channels.next_sibling()) {
                        if (std::string_view(channels.name()) == "Channels") {
                            for (pugi::xml_node chand = channels.first_child(); chand; chand = chand.next_sibling()) {
                                if (std::string_view(chand.name()) == "ChannelData") {
                                    for (pugi::xml_node chani = chand.first_child(); chani; chani = chani.next_sibling()) {
                                        if (std::string_view(chani.name()) == "ChanInfo") {
                                            std::string info = chani.text().get();
                                            if (info.find(", Normal") != std::string::npos) {
                                                std::string name = info.substr(0, info.find(", Normal"));
                                                _channelNames.push_back(name);
                                                _channelColors[name] = xlWHITE;
                                                MapToStrandName(name, _ccrNames);
                                            } else if (info.find(", RGB-") != std::string::npos) {
                                                std::string name = info.substr(0, info.find(", RGB-"));
                                                std::string color = info.substr(info.size() - 1, 1);
                                                if (color == "R") {
                                                    _channelNames.push_back(name);
                                                    _channelColors[name] = xlBLACK;
                                                }
                                                _channelNames.push_back(info);
                                                if (color == "R") {
                                                    _channelColors[info] = xlRED;
                                                } else if (color == "G") {
                                                    _channelColors[info] = xlGREEN;
                                                } else {
                                                    _channelColors[info] = xlBLUE;
                                                }
                                                MapToStrandName(name, _ccrNames);
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

    std::sort(_channelNames.begin(), _channelNames.end(), stdlistNumberAwareStringCompare);
    std::sort(_ccrNames.begin(), _ccrNames.end(), stdlistNumberAwareStringCompare);
}

bool HLSFile::IsCCRName(const std::string& name) const
{
    return std::find(_ccrNames.begin(), _ccrNames.end(), name) != _ccrNames.end();
}

xlColor HLSFile::GetChannelColor(const std::string& name) const
{
    auto it = _channelColors.find(name);
    if (it != _channelColors.end()) {
        return it->second;
    }
    return xlWHITE;
}

bool HLSFile::ChannelExists(const std::string& name) const
{
    return std::find(_channelNames.begin(), _channelNames.end(), name) != _channelNames.end();
}

std::string HLSFile::FindStrandName(const std::string& ccrName, int node) const
{
    std::string r = ccrName + fmt::format("P{:03d}", node);
    if (ChannelExists(r)) return r;
    r = ccrName + fmt::format("P{:04d}", node);
    if (ChannelExists(r)) return r;
    r = ccrName + fmt::format("P{:02d}", node);
    if (ChannelExists(r)) return r;
    r = ccrName + fmt::format("_{:04d}", node);
    if (ChannelExists(r)) return r;
    r = ccrName + fmt::format("_{:03d}", node);
    return r;
}

bool HLSFile::MapChannelEffects(EffectLayer* layer, const std::string& name, const xlColor& color, bool eraseExisting) const
{
    if (name.empty() || layer == nullptr || !_totalUniverses) {
        return false;
    }

    pugi::xml_node redNode;
    pugi::xml_node greenNode;
    pugi::xml_node blueNode;

    for (pugi::xml_node univ = _totalUniverses.first_child(); univ; univ = univ.next_sibling()) {
        if (std::string_view(univ.name()) == "Universe") {
            for (pugi::xml_node channels = univ.first_child(); channels; channels = channels.next_sibling()) {
                if (std::string_view(channels.name()) == "Channels") {
                    for (pugi::xml_node chand = channels.first_child(); chand; chand = chand.next_sibling()) {
                        if (std::string_view(chand.name()) == "ChannelData") {
                            for (pugi::xml_node chani = chand.first_child(); chani; chani = chani.next_sibling()) {
                                if (std::string_view(chani.name()) == "ChanInfo") {
                                    std::string info = chani.text().get();
                                    if (info == name + ", Normal") {
                                        redNode = chand;
                                    } else if (info == name + ", RGB-R") {
                                        redNode = chand;
                                    } else if (info == name + ", RGB-G") {
                                        greenNode = chand;
                                    } else if (info == name + ", RGB-B") {
                                        blueNode = chand;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (!redNode) {
        return false;
    }

    std::vector<unsigned char> redData(_frames);
    std::vector<unsigned char> greenData(_frames);
    std::vector<unsigned char> blueData(_frames);
    xlColorVector colors(_frames);
    ReadHLSData(redNode, redData);
    if (greenNode) {
        ReadHLSData(greenNode, greenData);
        ReadHLSData(blueNode, blueData);
        for (int x = 0; x < _frames; x++) {
            colors[x].Set(redData[x], greenData[x], blueData[x]);
        }
    } else {
        xlColor c(color.Red(), color.Green(), color.Blue());
        HSVValue hsv = c.asHSV();
        for (int x = 0; x < _frames; x++) {
            int i = redData[x];
            // HLS ramps go 1%-100%, so the first cell of a ramp isn't linear and
            // breaks ramp detection — clamp those leading/trailing 1-3 values to 0.
            if (i <= 3 && i > 0) {
                if (x < (_frames - 4)) {
                    if (i < redData[x + 1] && redData[x + 1] < redData[x + 2] && redData[x + 2] < redData[x + 3]) {
                        i = 0;
                    }
                }
                if (x > 4) {
                    if (i < redData[x - 1] && redData[x - 1] < redData[x - 2] && redData[x - 2] < redData[x - 3]) {
                        i = 0;
                    }
                }
            }
            hsv.value = ((double)i) / 255.0;
            colors[x] = hsv;
        }
    }

    ConvertDataRowToEffects(layer, colors, _frameTime, eraseExisting);
    return true;
}
