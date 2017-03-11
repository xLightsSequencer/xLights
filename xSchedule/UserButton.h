#ifndef USERBUTTON_H
#define USERBUTTON_H

#include <string>
#include <list>
#include <wx/wx.h>

class wxXmlNode;

class UserButton
{
    wxUint32 _id;
	std::string _label;
    std::string _command;
    std::string _parameters;
	char _hotkey;
	int _changeCount;
	int _lastSavedChangeCount;

    void Load(wxXmlNode* node);

    public:

		UserButton(wxXmlNode* node);
		UserButton();
		virtual ~UserButton() {}
		wxXmlNode* Save();
        wxUint32 GetId() const { return _id; }
        bool IsDirty() const { return _changeCount != _lastSavedChangeCount; }
        void ClearDirty() { _lastSavedChangeCount = _changeCount; }
		void SetLabel(const std::string& label) { if (_label != label) { _label = label; _changeCount++; } }
        void SetCommand(const std::string& command) { if (_command != command) { _command = command; _changeCount++; } }
        void SetParameters(const std::string& parameters) { if (_parameters != parameters) { _parameters = parameters; _changeCount++; } }
		void SetHotkey(const char hotkey) { if (_hotkey != hotkey) { _hotkey = hotkey; _changeCount++; } }
		std::string GetLabel() const { return _label; }
        std::string GetCommand() const { return _command; }
        std::string GetParameters() const { return _parameters; }
        char GetHotkey() const { return _hotkey; }
};

#endif 