#include "UserButton.h"
#include <wx/xml/xml.h>

UserButton::UserButton(wxXmlNode* node)
{
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    Load(node);
}

void UserButton::Load(wxXmlNode* node)
{
    _label = node->GetAttribute("Label", "").ToStdString();
    _command = node->GetAttribute("Command", "").ToStdString();
    _parameters = node->GetAttribute("Parameters", "").ToStdString();
    auto hk = node->GetAttribute("Hotkey", "~");
    if (hk == "")
    {
        _hotkey = '~';
    }
    else
    {
        _hotkey = hk[0];
    }
}

UserButton::UserButton()
{
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    _label = "";
    _command = "";
    _hotkey = '~';
    _parameters = "";
}

wxXmlNode* UserButton::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Button");

    res->AddAttribute("Label", _label);
    res->AddAttribute("Command", _command);
    res->AddAttribute("Parameters", _parameters);
    res->AddAttribute("Hotkey", _hotkey);

    return res;
}
