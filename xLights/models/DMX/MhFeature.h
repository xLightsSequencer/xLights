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

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class MhChannel;
class wxXmlNode;

class MhFeature : public SerializedObject
{
    public:
        MhFeature(wxXmlNode* node, wxString _name);
        virtual ~MhFeature();

        void Init(BaseObject* base);

        void AddTypeProperties(wxPropertyGridInterface* grid);

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

        std::string GetBaseName() { return base_name; }
        std::string GetXmlName() { return xml_name; }
        wxXmlNode* GetXmlNode() { return node_xml; }
    
    protected:

    private:
        wxXmlNode* node_xml;
        wxString base_name;
        wxString xml_name;
        std::vector<MhChannel*> channels;

        BaseObject* base;
};

