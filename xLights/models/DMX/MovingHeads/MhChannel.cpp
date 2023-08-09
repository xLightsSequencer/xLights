/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/sstream.h>

#include "MhChannel.h"
#include "../../BaseObject.h"
#include <glm/gtc/type_ptr.hpp>

MhChannel::MhChannel(wxXmlNode* node, wxString _name)
    : node_xml(node), base_name(_name)
{
}

void MhChannel::Init(BaseObject* base) {
    this->base = base;
    channel_coarse = wxAtoi(node_xml->GetAttribute("ChannelCoarse", "0"));
    channel_fine = wxAtoi(node_xml->GetAttribute("ChannelFine", "0"));
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
    wxString node_name = wxString::Format("Range%d", ranges.size()+1);
    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, node_name);
    node_xml->AddChild(new_node);
    new_node->AddAttribute("RangeLabel", name);
    std::unique_ptr<MhChannel::MhRange> newRange(new MhChannel::MhRange(new_node, node_name));
    ranges.push_back(std::move(newRange));
}

MhChannel::MhRange::MhRange(wxXmlNode* node, wxString _name)
    : node_xml(node), name(_name)
{
}

void MhChannel::MhRange::SetRangeMin(std::string& val) {
    node_xml->DeleteAttribute("MinValue");
    node_xml->AddAttribute("MinValue", val);
}

void MhChannel::MhRange::SetRangeMax(std::string& val) {
    node_xml->DeleteAttribute("MaxValue");
    node_xml->AddAttribute("MaxValue", val);
}
