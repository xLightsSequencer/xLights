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

#include <string>
#include <wx/wx.h>

class wxXmlNode;
class SMSMessage;

class MagicWord
{
    protected:
        static int __nextId;
        int _id;
        std::string _command;
        std::string _parm1;
        std::string _parm2;
        std::string _parm3;
        std::string _name;
        int _changeCount;
        int _lastSavedChangeCount;

        void Fire();

	public:
        MagicWord();
		MagicWord(wxXmlNode* node);
		virtual ~MagicWord() {}
		void Save(wxXmlNode* node);
        int GetId() const { return _id; }
        bool operator==(const MagicWord& eb) const { return _id == eb._id; }
        bool IsDirty() const { return _lastSavedChangeCount != _changeCount; }
        void ClearDirty() { _lastSavedChangeCount = _changeCount; }
        std::string GetMagicWord() const { return _name; }
        std::string GetCommand() const { return _command; }
        std::string GetP1() const { return _parm1; }
        std::string GetP2() const { return _parm2; }
        std::string GetP3() const { return _parm3; }
        void SetCommand(std::string command) { if (command != _command) { _command = command; _changeCount++; } }
        void SetP1(std::string value) { if (value != _parm1) { _parm1 = value; _changeCount++; } }
        void SetP2(std::string value) { if (value != _parm2) { _parm2 = value; _changeCount++; } }
        void SetP3(std::string value) { if (value != _parm3) { _parm3 = value; _changeCount++; } }
        void SetMagicWord(std::string value) { if (value != _name) { _name = value; _changeCount++; } }
        bool CheckMessage(const SMSMessage& msg);
};
