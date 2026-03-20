#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <vector>
#include <memory>

//#include "SerializedObject.h"

class MhChannel;
class wxXmlNode;

class MhFeature// : public SerializedObject
{
    public:
        MhFeature(wxXmlNode* node, const std::string& _xml_name, const std::string& pretty_name);
        virtual ~MhFeature();

        void Init();

        std::string GetName() { return name; }
        std::string GetXmlName() { return xml_name; }
        wxXmlNode* GetXmlNode() { return node_xml; }

        std::vector<std::unique_ptr<MhChannel>>& GetChannels() { return channels; }

    protected:

    private:
        wxXmlNode* node_xml;
        std::string name;
        std::string xml_name;
        std::vector<std::unique_ptr<MhChannel>> channels;
};

