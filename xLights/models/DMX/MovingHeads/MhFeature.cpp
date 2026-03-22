/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MhFeature.h"
#include "MhChannel.h"
#include <glm/gtc/type_ptr.hpp>

MhFeature::MhFeature(pugi::xml_node node, const std::string& _xml_name, const std::string& pretty_name)
    : /*SerializedObject(_xml_name), */ node_xml(node), name(pretty_name), xml_name(_xml_name)
{
}

MhFeature::~MhFeature()
{
}

void MhFeature::Init() {
    pugi::xml_node n = node_xml.first_child();
    while (n) {
        std::string node_name = n.name();
        std::string channel_name = n.attribute("Name").as_string(node_name.c_str());
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
        n = n.next_sibling();
    }
}
