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
#include <wx/xml/xml.h>

struct Model {
    wxString Name;
    wxString Type;
    wxString Layout;
    wxString StartChanel;

    Model() {}
    Model(wxXmlNode* node)
    {
        ParseXML(node);
    }

    void ParseXML(wxXmlNode* node)
    {
        Name = node->GetAttribute("name");
        Type = node->GetAttribute("DisplayAs", "");
        Layout = node->GetAttribute("LayoutGroup");
        StartChanel = node->GetAttribute("StartChannel");
    }

    [[nodiscard]] wxString ToString() const
    {
        return wxString::Format("%s:%s:%s:%s\n", Name, Type, Layout, StartChanel);
    }
    //private:

};