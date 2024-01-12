/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/sstream.h>

#include "MhChannel.h"
#include <glm/gtc/type_ptr.hpp>

MhChannel::MhChannel(wxXmlNode* node, wxString pretty_name)
: node_xml(node), name(pretty_name)
{
}

void MhChannel::Init() {
    channel_coarse = wxAtoi(node_xml->GetAttribute("ChannelCoarse", "0"));
    channel_fine = wxAtoi(node_xml->GetAttribute("ChannelFine", "0"));
    
    wxXmlNode* n = node_xml->GetChildren();
    while (n != nullptr) {
        std::string node_name = n->GetName();
        std::string range_name = n->GetAttribute("Name", node_name);
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
        n = n->GetNext();
    }
}

void MhChannel::SetChannelCoarse(std::string& val) {
    node_xml->DeleteAttribute("ChannelCoarse");
    node_xml->AddAttribute("ChannelCoarse", val);
}

void MhChannel::SetChannelFine(std::string& val) {
    node_xml->DeleteAttribute("ChannelFine");
    node_xml->AddAttribute("ChannelFine", val);
}

void MhChannel::AddRange(std::string& name)
{
    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "range");
    new_node->AddAttribute("Name", name);
    node_xml->AddChild(new_node);
    std::unique_ptr<MhChannel::MhRange> newRange(new MhChannel::MhRange(new_node, name));
    ranges.push_back(std::move(newRange));
}

MhChannel::MhRange::MhRange(wxXmlNode* node, wxString pretty_name)
    : range_node(node), name(pretty_name)
{
}

void MhChannel::MhRange::SetRangeMin(std::string& val) {
    range_node->DeleteAttribute("Min");
    range_node->AddAttribute("Min", val);
}

void MhChannel::MhRange::SetRangeMax(std::string& val) {
    range_node->DeleteAttribute("Max");
    range_node->AddAttribute("Max", val);
}

void MhChannel::MhRange::Init() {
    min = wxAtoi(range_node->GetAttribute("Min", "0"));
    max = wxAtoi(range_node->GetAttribute("Max", "255"));
}
