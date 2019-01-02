#include <wx/xml/xml.h>
#include <wx/notebook.h>

#include "PlayListItemRunProcess.h"
#include "PlayListItemRunProcessPanel.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"

#include <log4cpp/Category.hh>

PlayListItemRunProcess::PlayListItemRunProcess(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _command = "";
    _waitForCompletion = false;
    PlayListItemRunProcess::Load(node);
}

void PlayListItemRunProcess::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _command = node->GetAttribute("Command", "");
    _waitForCompletion = (node->GetAttribute("Wait", "FALSE") == "TRUE");
}

PlayListItemRunProcess::PlayListItemRunProcess() : PlayListItem()
{
    _started = false;
    _command = "";
    _waitForCompletion = false;
}

PlayListItem* PlayListItemRunProcess::Copy() const
{
    PlayListItemRunProcess* res = new PlayListItemRunProcess();
    res->_command = _command;
    res->_started = false;
    res->_waitForCompletion = _waitForCompletion;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemRunProcess::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIProcess");

    node->AddAttribute("Command", _command);
    if (_waitForCompletion)
    {
        node->AddAttribute("Wait", "TRUE");
    }

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemRunProcess::GetTitle() const
{
    return "Run Process";
}

void PlayListItemRunProcess::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemRunProcessPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemRunProcess::GetNameNoTime() const
{
    if (_name != "") return _name;

    return "Run Process";
}

std::string PlayListItemRunProcess::GetTooltip()
{
    return GetTagHint() + "\n    %SHOWDIR% - the current show directory";
}

void PlayListItemRunProcess::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (ms >= _delay && !_started)
    {
        _started = true;

        // we need to run the process
        int flags;
        if (_waitForCompletion)
        {
            flags = wxEXEC_BLOCK;
        }
        else
        {
            flags = wxEXEC_ASYNC;
        }

        wxString cmd = _command;
        if (cmd.Contains("%SHOWDIR%"))
        {
            cmd.Replace("%SHOWDIR%", xScheduleFrame::GetScheduleManager()->GetShowDir(), true);
        }
        cmd = ReplaceTags(cmd.ToStdString());

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Launching command %s wait %d.", (const char *)cmd.c_str(), (int)_waitForCompletion);

        // ensure we start in the show directory
        wxExecuteEnv execEnv;
        execEnv.cwd = xScheduleFrame::GetScheduleManager()->GetShowDir();

        if (cmd != "")
        {
            wxExecute(cmd, flags, nullptr, &execEnv);
            logger_base.info("Command launched.");
        }
        else
        {
            logger_base.error("Command launched failed as it was blank.");
        }
    }
}

void PlayListItemRunProcess::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);
    _started = false;
}
