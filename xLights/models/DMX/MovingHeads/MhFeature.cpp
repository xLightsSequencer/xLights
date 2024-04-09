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

#include "MhFeature.h"
#include "MhChannel.h"
#include <glm/gtc/type_ptr.hpp>

MhFeature::MhFeature(wxXmlNode* node, wxString _xml_name, wxString pretty_name)
    : /*SerializedObject(_xml_name), */ node_xml(node), name(pretty_name), xml_name(_xml_name)
{
}

MhFeature::~MhFeature()
{
}

void MhFeature::Init() {
    wxXmlNode* n = node_xml->GetChildren();
    while (n != nullptr) {
        std::string node_name = n->GetName();
        std::string channel_name = n->GetAttribute("Name", node_name);
        bool channel_found {false};
        for (auto it = channels.begin(); it != channels.end(); ++it) {
            if( (*it)->GetName() == channel_name ) {
                channel_found = true;
                (*it)->Init();
                break;
            }
        }
        if( !channel_found ) {
            std::unique_ptr<MhChannel> newChannel(new MhChannel(n, channel_name));
            newChannel->Init();
            channels.push_back(std::move(newChannel));
        }
        n = n->GetNext();
    }
}
