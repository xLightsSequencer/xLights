#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/string.h>
#include <glm/glm.hpp>

#include "SerializedObject.h"

class BaseObject;
class MhChannel;
class wxXmlNode;

class MhFeature : public SerializedObject
{
    public:
        MhFeature(wxXmlNode* node, wxString _name);
        virtual ~MhFeature();

        void Init(BaseObject* base);

        std::string GetBaseName() { return base_name; }
        std::string GetXmlName() { return xml_name; }
        wxXmlNode* GetXmlNode() { return node_xml; }

        std::vector<std::unique_ptr<MhChannel>>& GetChannels() { return channels; }

    protected:

    private:
        wxXmlNode* node_xml;
        wxString base_name;
        wxString xml_name;
        std::vector<std::unique_ptr<MhChannel>> channels;

        BaseObject* base;
};

