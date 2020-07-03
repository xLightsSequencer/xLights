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

#include <string>
#include <wx/wx.h>
#include "CommandManager.h"

class wxXmlNode;
class Command;

class UserButton
{
    wxUint32 _id;
	std::string _label;
	wxString _labelLower;
    std::string _command;
    std::string _parameters;
	char _hotkey;
	int _changeCount;
	int _lastSavedChangeCount;
    std::string _color;
    Command* _commandObj;

    void Load(wxXmlNode* node, CommandManager* commandManager);

    public:

		UserButton(wxXmlNode* node, CommandManager* commandManager);
		UserButton();
		virtual ~UserButton() {}
		wxXmlNode* Save();
        wxUint32 GetId() const { return _id; }
        bool IsDirty() const { return _changeCount != _lastSavedChangeCount; }
        void ClearDirty() { _lastSavedChangeCount = _changeCount; }
        void SetLabel(const std::string& label) { if (_label != label) { _label = label; _labelLower = wxString(_label).Lower(); _changeCount++; } }
        void SetCommand(const std::string& command, CommandManager* commandManager);
        void SetParameters(const std::string& parameters) { if (_parameters != parameters) { _parameters = parameters; _changeCount++; } }
		void SetHotkey(const char hotkey) { if (_hotkey != hotkey) { _hotkey = hotkey; _changeCount++; } }
        Command* GetCommandObj() const { return _commandObj; }
		std::string GetLabel() const { return _label; }
		wxString GetLabelLower() const { return _labelLower; }
        std::string GetCommand() const { return _command; }
        std::string GetParameters() const { return _parameters; }
        std::string GetColorName() const { return _color; }
        wxColor GetColor() const;
        void SetColor(const std::string& color) { if (_color != color) { _color = color; _changeCount++; } }
        char GetHotkey() const { return _hotkey; }
};

