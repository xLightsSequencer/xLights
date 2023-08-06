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

MhChannel::MhRange::MhRange(wxString _name)
    : name(_name)
{
}
