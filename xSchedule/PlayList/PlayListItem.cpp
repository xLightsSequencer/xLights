#include "PlayListItem.h"
#include <wx/xml/xml.h>
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../../xLights/AudioManager.h"
#include "../RunningSchedule.h"
#include "PlayList.h"
#include "PlayListStep.h"

int __playlistitemid = 0;

PlayListItem::PlayListItem(wxXmlNode* node)
{
    _currentFrame = 0;
    _id = __playlistitemid++;
    _priority = 0;
    _volume = -1;
    _lastSavedChangeCount = 0;
    _changeCount = 0;
    _frames = 0;
    _msPerFrame = 50;
    _delay = 0;
    _name = "";
    _restOfStep = false;
    PlayListItem::Load(node);
}

void PlayListItem::Load(wxXmlNode* node)
{
    _delay = wxAtoi(node->GetAttribute("Delay", "0"));
    _volume = wxAtoi(node->GetAttribute("Volume", "-1"));
    _priority = wxAtoi(node->GetAttribute("Priority", "0"));
    _name = node->GetAttribute("Name", "");
    if (node->GetAttribute("RestOfStep", "FALSE") == "TRUE")
    {
        _restOfStep = true;
    }
}

PlayListItem::PlayListItem()
{
    _currentFrame = 0;
    _name = "";
    _id = __playlistitemid++;
    _volume = -1;
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _frames = 0;
    _msPerFrame = 50;
    _delay = 0;
    _priority = 0;
    _restOfStep = false;
    _stepLengthMS = 0;
}

void PlayListItem::Save(wxXmlNode* node)
{
    node->AddAttribute("Delay", wxString::Format(wxT("%i"), (long)_delay));
    node->AddAttribute("Name", _name);
    if (_volume != -1)
    {
        node->AddAttribute("Volume", wxString::Format(wxT("%i"), _volume));
    }
    if (_priority != 0)
    {
        node->AddAttribute("Priority", wxString::Format(wxT("%i"), (int)_priority));
    }
    if (_restOfStep)
    {
        node->AddAttribute("RestOfStep", "TRUE");
    }
}

std::string PlayListItem::GetName() const
{
    std::string duration = "";
    if (GetDurationMS() != 0)
    {
        duration = " [" + wxString::Format(wxT("%.3f"), (float)GetDurationMS() / 1000.0).ToStdString() + "]";
    }

    return GetNameNoTime() + duration;
}

std::string PlayListItem::GetNameNoTime() const
{
    if (_name != "") return _name;

    return "<unnamed>";
}

void PlayListItem::Copy(PlayListItem* to) const
{
    to->_id = _id;
    to->_lastSavedChangeCount = _lastSavedChangeCount;
    to->_changeCount = _changeCount;
    to->_delay = _delay;
    to->_frames = _frames;
    to->_msPerFrame = _msPerFrame;
    to->_name = _name;
    to->_priority = _priority;
    to->_volume = _volume;
    to->_restOfStep = _restOfStep;
    to->_stepLengthMS = _stepLengthMS;
}

bool PlayListItem::IsInSlaveMode() const
{
    ScheduleManager* sm = xScheduleFrame::GetScheduleManager();

    if (sm == nullptr) return false;

    return sm->GetSyncManager()->IsSlave();
}

std::string PlayListItem::ReplaceTags(const std::string s) const
{
    wxString res = s;
    AudioManager* am = nullptr;

    PlayList* pl = xScheduleFrame::GetScheduleManager()->GetRunningPlayList();
    if (pl != nullptr)
    {
        if (res.Contains("%RUNNING_PLAYLIST%"))
        {
            res.Replace("%RUNNING_PLAYLIST%", pl->GetNameNoTime(), true);
        }

        PlayListStep* pls = pl->GetRunningStep();
        if (pls != nullptr)
        {
            if (am == nullptr) am = pls->GetAudioManager();

            if (res.Contains("%RUNNING_PLAYLISTSTEP%"))
            {
                res.Replace("%RUNNING_PLAYLISTSTEP%", pls->GetNameNoTime(), true);
            }
            if (res.Contains("%STEPNAME%"))
            {
                res.Replace("%STEPNAME%", pls->GetNameNoTime(), true);
            }
            if (res.Contains("%RUNNING_PLAYLISTSTEPMS%"))
            {
                res.Replace("%RUNNING_PLAYLISTSTEPMS%", wxString::Format(wxT("%i"), pls->GetLengthMS()), true);
            }
            if (res.Contains("%RUNNING_PLAYLISTSTEPMSLEFT%"))
            {
                res.Replace("%RUNNING_PLAYLISTSTEPMSLEFT%", wxString::Format(wxT("%i"), pls->GetLengthMS() - pls->GetPosition()), true);
            }

            if (pl != nullptr && !pl->IsRandom())
            {
                bool dummy;
                auto nextstep = pl->GetNextStep(dummy);
                if (nextstep != nullptr)
                {
                    res.Replace("%NEXTSTEPNAME%", nextstep->GetNameNoTime());
                }
            }
        }
    }
    if (res.Contains("%RUNNING_SCHEDULE%"))
    {
        RunningSchedule* rs = xScheduleFrame::GetScheduleManager()->GetRunningSchedule();
        if (rs != nullptr && rs->GetPlayList()->IsRunning())
        {
            res.Replace("%RUNNING_SCHEDULE%", rs->GetSchedule()->GetName(), true);
        }
    }
    res.Replace("%NEXTSTEPNAME%", "");

    if (am != nullptr)
    {
        res.Replace("%TITLE%", am->Title());
        res.Replace("%ARTIST%", am->Artist());
        res.Replace("%ALBUM%", am->Album());
    }

    return res.ToStdString();
}

std::string PlayListItem::GetTagHint()
{
    return "Available variables:\n    %RUNNING_PLAYLIST% - current playlist\n    %RUNNING_PLAYLISTSTEP% - step name\n    %RUNNING_PLAYLISTSTEPMS% - Position in current step\n    %RUNNING_PLAYLISTSTEPMSLEFT% - Time left in current step\n    %RUNNING_SCHEDULE% - Name of schedule\n    %STEPNAME% - Current step\n    %NEXTSTEPNAME% - Next step\n    %NEXTSTEPNAME% - Next step\n    %ALBUM% - from mp3\n    %TITLE% - from mp3\n    %ARTIST% - from mp3";
}

