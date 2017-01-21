#include "PlayListItemRunProcess.h"
#include "PlayListItemRunProcessPanel.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "PlayList.h"
#include "PlayListStep.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <log4cpp/Category.hh>
#include "../RunningSchedule.h"

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

void PlayListItemRunProcess::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
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
        PlayList* pl = xScheduleFrame::GetScheduleManager()->GetRunningPlayList();
        if (pl != nullptr)
        {
            if (cmd.Contains("%RUNNING_PLAYLIST%"))
            {
                cmd.Replace("%RUNNING_PLAYLIST%", pl->GetNameNoTime(), true);
            }
            PlayListStep* pls = pl->GetRunningStep();
            if (pls != nullptr)
            {
                if (cmd.Contains("%RUNNING_PLAYLISTSTEP%"))
                {
                    cmd.Replace("%RUNNING_PLAYLISTSTEP%", pls->GetNameNoTime(), true);
                }
                if (cmd.Contains("%RUNNING_PLAYLISTSTEPMS%"))
                {
                    cmd.Replace("%RUNNING_PLAYLISTSTEPMS%", wxString::Format(wxT("%i"), pls->GetLengthMS()), true);
                }
                if (cmd.Contains("%RUNNING_PLAYLISTSTEPMSLEFT%"))
                {
                    cmd.Replace("%RUNNING_PLAYLISTSTEPMSLEFT%", wxString::Format(wxT("%i"), pls->GetLengthMS() - pls->GetPosition()), true);
                }
            }
        }
        if (cmd.Contains("%RUNNING_SCHEDULE%"))
        {
            RunningSchedule* rs = xScheduleFrame::GetScheduleManager()->GetRunningSchedule();
            if (rs != nullptr && rs->GetPlayList()->IsRunning())
            {
                cmd.Replace("%RUNNING_SCHEDULE%", rs->GetSchedule()->GetName(), true);
            }
        }

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Launching command %s wait %d.", (const char *)cmd.c_str(), (int)_waitForCompletion);
        wxExecute(cmd, flags);
        logger_base.info("Command launched.");
    }
}

void PlayListItemRunProcess::Start()
{
    _started = false;
}
