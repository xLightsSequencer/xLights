/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MagicWord.h"
#include "SMSMessage.h"
#include "xSMSDaemonApp.h"

#include <wx/wx.h>
#include <wx/xml/xml.h>
#include "spdlog/spdlog.h"

#include "../../xLights/UtilFunctions.h"

int MagicWord::__nextId = 0;

MagicWord::MagicWord()
{
    _id = __nextId++;
    _command = "";
    _parm1 = "";
    _parm2 = "";
    _parm3 = "";
    _name = "";
    _changeCount = 0;
    _lastSavedChangeCount = 0;
}

MagicWord::MagicWord(wxXmlNode* node)
{
    _id = __nextId++;
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    _command = node->GetAttribute("Command", "");
    _parm1 = node->GetAttribute("CommandParm1", "");
    _parm2 = node->GetAttribute("CommandParm2", "");
    _parm3 = node->GetAttribute("CommandParm3", "");
    _name = node->GetAttribute("Name", "");
}

void MagicWord::Save(wxXmlNode* mws)
{
    wxXmlNode* node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "MagicWord");
    node->AddAttribute("Command", _command);
    node->AddAttribute("Name", _name);
    node->AddAttribute("CommandParm1", _parm1);
    node->AddAttribute("CommandParm2", _parm2);
    node->AddAttribute("CommandParm3", _parm3);
    mws->AddChild(node);
}

void MagicWord::Fire()
{
    char result[4096];

    std::wstring s;
    if (_parm1 != "") {
        s = std::wstring(_parm1.begin(), _parm1.end());
    }
    if (_parm2 != "") {
        s += _(",") + std::wstring(_parm2.begin(), _parm2.end());
    }
    if (_parm3 != "") {
        s += _(",") + std::wstring(_parm3.begin(), _parm3.end());
    }

    xSMSDaemonApp::GetAction()(_command.c_str(), (const wchar_t*)s.c_str(), "", result, sizeof(result));
    std::string res(result);

    if (Contains(res, _("result\":\"ok"))) {
        spdlog::debug("Command {}:{},{},{} fired", (const char*)_command.c_str(), (const char*)_parm1.c_str(), (const char*)_parm2.c_str(), (const char*)_parm3.c_str());
    }
    else {
        spdlog::debug("Command {}:{},{},{} failed to fire", (const char*)_command.c_str(), (const char*)_parm1.c_str(), (const char*)_parm2.c_str(), (const char*)_parm3.c_str());
        spdlog::debug("   res: {}", (const char*)res.c_str());
    }

}

bool MagicWord::CheckMessage(const SMSMessage& msg)
{
    auto m = Lower(msg._message);
    if (m == _name) {
        spdlog::debug("Magic word {} found.", (const char*)_name.c_str());
        Fire();
        return true;
    }
    return false;
}

