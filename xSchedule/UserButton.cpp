/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "UserButton.h"
#include <wx/xml/xml.h>

int __buttonid = 0;

UserButton::UserButton(wxXmlNode* node, CommandManager* commandManager)
{
    _commandObj = nullptr;
    _color = "default";
    _id = __buttonid++;
    Load(node, commandManager);
    _changeCount = 0;
    _lastSavedChangeCount = 0;
}

void UserButton::SetCommand(const std::string& command, CommandManager* commandManager)
{
    if (_command != command)
    {
        _command = command; 
        _changeCount++;
    } 
    _commandObj = commandManager->GetCommand(_command);
}

void UserButton::Load(wxXmlNode* node, CommandManager* commandManager)
{
    SetLabel(node->GetAttribute("Label", "").ToStdString());
    SetCommand(node->GetAttribute("Command", "").ToStdString(), commandManager);
    _color = node->GetAttribute("Color", "default").ToStdString();
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
    _commandObj = nullptr;
    _id = __buttonid++;
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    SetLabel("");
    _command = "";
    _hotkey = '~';
    _parameters = "";
    _color = "default";
}

wxXmlNode* UserButton::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Button");

    res->AddAttribute("Label", _label);
    res->AddAttribute("Command", _command);
    res->AddAttribute("Parameters", _parameters);
    res->AddAttribute("Hotkey", _hotkey);
    res->AddAttribute("Color", _color);

    return res;
}

wxColor UserButton::GetColor() const
{
    if (_color == "red")
    {
        return wxColor(0xd9, 0x53, 0x4f);
    }
    else if (_color == "green")
    {
        return wxColor(0x5c, 0xb8, 0x5c);
    }
    else if (_color == "blue")
    {
        return wxColor(0x33, 0x7a, 0xb7);
    }
    else if (_color == "cyan")
    {
        return wxColor(0x5b, 0xc0, 0xde);
    }
    else if (_color == "orange")
    {
        return wxColor(0xf0, 0xad, 0x4e);
    }

    return *wxBLACK;
}