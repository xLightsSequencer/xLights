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

#include <wx/string.h>
#include <glm/glm.hpp>

//#include "SerializedObject.h"

class MhChannel;
class wxXmlNode;

class MhFeature// : public SerializedObject
{
    public:
        MhFeature(wxXmlNode* node, wxString _xml_name, wxString pretty_name);
        virtual ~MhFeature();

        void Init();

        std::string GetName() { return name; }
        std::string GetXmlName() { return xml_name; }
        wxXmlNode* GetXmlNode() { return node_xml; }

        std::vector<std::unique_ptr<MhChannel>>& GetChannels() { return channels; }

    protected:

    private:
        wxXmlNode* node_xml;
        wxString name;
        wxString xml_name;
        std::vector<std::unique_ptr<MhChannel>> channels;
};

