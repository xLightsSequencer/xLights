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

struct Controller {
    wxString Name;
    wxString Type;
    wxString IPCom;
    wxString Protocol;

    Controller() {}
    Controller(wxXmlNode* node)
    {
        ParseXML(node);
    }

    void ParseXML(wxXmlNode* node)
    {
        Name = node->GetAttribute("Name");
        Type = node->GetAttribute("Type", "");
        Protocol = node->GetAttribute("Protocol");
        if (node->HasAttribute("IP")) {
            IPCom = node->GetAttribute("IP");
        } 
        if (node->HasAttribute("Port")) {
            IPCom = node->GetAttribute("Port");
        }
    }

    [[nodiscard]] wxString ToString() const
    {
        return wxString::Format("%s:%s:%s:%s\n", Name, Type, IPCom, Protocol);
    }
//private:

};