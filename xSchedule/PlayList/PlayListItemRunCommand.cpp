#include "PlayListItemRunCommand.h"
#include "PlayListItemRunCommandPanel.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <log4cpp/Category.hh>
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"

PlayListItemRunCommand::PlayListItemRunCommand(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _command = "";
    _parm1 = "";
    _parm2 = "";
    _parm3 = "";
    PlayListItemRunCommand::Load(node);
}

void PlayListItemRunCommand::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _command = node->GetAttribute("Command", "");
    _parm1 = node->GetAttribute("Parm1", "");
    _parm2 = node->GetAttribute("Parm2", "");
    _parm3 = node->GetAttribute("Parm3", "");
}

PlayListItemRunCommand::PlayListItemRunCommand() : PlayListItem()
{
    _started = false;
    _command = "";
    _parm1 = "";
    _parm2 = "";
    _parm3 = "";
}

PlayListItem* PlayListItemRunCommand::Copy() const
{
    PlayListItemRunCommand* res = new PlayListItemRunCommand();
    res->_command = _command;
    res->_parm1 = _parm1;
    res->_parm2 = _parm2;
    res->_parm3 = _parm3;
    res->_started = false;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemRunCommand::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLICommand");

    node->AddAttribute("Command", _command);
    node->AddAttribute("Parm1", _parm1);
    node->AddAttribute("Parm2", _parm2);
    node->AddAttribute("Parm3", _parm3);

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemRunCommand::GetTitle() const
{
    return "Run Command";
}

void PlayListItemRunCommand::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemRunCommandPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemRunCommand::GetNameNoTime() const
{
    if (_name != "") return _name;

    return "Run Command";
}

void PlayListItemRunCommand::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (ms >= _delay && !_started)
    {
        _started = true;

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Launching command %s %s,%s,%s.", (const char *)_command.c_str(), (const char*)_parm1.c_str(), (const char*)_parm2.c_str(), (const char*)_parm3.c_str());

        std::string parms = _parm1;
        if (_parm2 != "") parms += "," + _parm2;
        if (_parm3 != "") parms += "," + _parm3;

        size_t rate;
        wxString msg;
        if (!xScheduleFrame::GetScheduleManager()->Action(_command, parms, "", nullptr, nullptr, rate, msg))
        {
            logger_base.info("Command failed: %s.", (const char *)msg.c_str());
        }
    }
}

std::list<std::string> PlayListItemRunCommand::GetCommands() const
{
    std::list<std::string> res;

    auto commands = xScheduleFrame::GetScheduleManager()->GetCommands();

    for (auto it = commands.begin(); it != commands.end(); ++it)
    {
        res.push_back((*it)->_command);
    }

    return res;
}

void PlayListItemRunCommand::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}
