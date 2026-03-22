/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cstdlib>

#include "MhChannel.h"
#include <glm/gtc/type_ptr.hpp>

MhChannel::MhChannel(pugi::xml_node node, const std::string& pretty_name)
: node_xml(node), name(pretty_name)
{
}

void MhChannel::Init() {
    channel_coarse = (int)std::strtol(node_xml.attribute("ChannelCoarse").as_string("0"), nullptr, 10);
    channel_fine = (int)std::strtol(node_xml.attribute("ChannelFine").as_string("0"), nullptr, 10);

    pugi::xml_node n = node_xml.first_child();
    while (n) {
        std::string node_name = n.name();
        std::string range_name = n.attribute("Name").as_string(node_name.c_str());
        bool range_found {false};
        for (auto it = ranges.begin(); it != ranges.end(); ++it) {
            if( (*it)->GetName() == range_name ) {
                range_found = true;
                (*it)->Init();
                break;
            }
        }
        if( !range_found ) {
            std::unique_ptr<MhRange> newRange(new MhRange(n, range_name));
            newRange->Init();
            ranges.push_back(std::move(newRange));
        }
        n = n.next_sibling();
    }
}

void MhChannel::SetChannelCoarse(std::string& val) {
    node_xml.remove_attribute("ChannelCoarse");
    node_xml.append_attribute("ChannelCoarse") = val;
}

void MhChannel::SetChannelFine(std::string& val) {
    node_xml.remove_attribute("ChannelFine");
    node_xml.append_attribute("ChannelFine") = val;
}

void MhChannel::AddRange(std::string& name)
{
    pugi::xml_node new_node = node_xml.append_child("range");
    new_node.append_attribute("Name") = name;
    std::unique_ptr<MhChannel::MhRange> newRange(new MhChannel::MhRange(new_node, name));
    ranges.push_back(std::move(newRange));
}

MhChannel::MhRange::MhRange(pugi::xml_node node, const std::string& pretty_name)
    : range_node(node), name(pretty_name)
{
}

void MhChannel::MhRange::SetRangeMin(std::string& val) {
    range_node.remove_attribute("Min");
    range_node.append_attribute("Min") = val;
}

void MhChannel::MhRange::SetRangeMax(std::string& val) {
    range_node.remove_attribute("Max");
    range_node.append_attribute("Max") = val;
}

void MhChannel::MhRange::Init() {
    min = (int)std::strtol(range_node.attribute("Min").as_string("0"), nullptr, 10);
    max = (int)std::strtol(range_node.attribute("Max").as_string("255"), nullptr, 10);
}
