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
class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class SerializedObject
{
    public:
        SerializedObject(wxString _name);
        virtual ~SerializedObject() = default;

        void Serialise(wxXmlNode* root, wxFile& f, const wxString& show_dir) const;
        void Serialise(wxXmlNode* root, wxXmlNode* model_xml, const wxString& show_dir) const;
    
    private:
        wxString base_name;
};

