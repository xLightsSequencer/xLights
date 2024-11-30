/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItem.h"
#include "PlayList.h"
#include "PlayListStep.h"
#include "../../xLights/AudioManager.h"
#include "../RunningSchedule.h"
#include "../ScheduleManager.h"
#include "../ScheduleOptions.h"
#include "../xScheduleMain.h"
#include <wx/regex.h>
#include <wx/xml/xml.h>

int __playlistitemid = 0;

PlayListItem::PlayListItem(wxXmlNode* node) {
    _name = "";
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

bool PlayListItem::SetPosition(size_t frame, size_t ms) {
    // wxASSERT(abs((long)frame * _msPerFrame - (long)ms) < _msPerFrame);
    _currentFrame = frame;

    return true;
}

ScheduleOptions* PlayListItem::GetOptions() const {
    ScheduleManager* sm = xScheduleFrame::GetScheduleManager();
    if (sm == nullptr)
        return nullptr;
    return sm->GetOptions();
}

std::string PlayListItem::GetLocalIP() const {
    ScheduleManager* sm = xScheduleFrame::GetScheduleManager();
    if (sm == nullptr)
        return "";
    return sm->GetForceLocalIP();
}

void PlayListItem::Load(wxXmlNode* node) {
    _type = node->GetName();
    _delay = wxAtoi(node->GetAttribute("Delay", "0"));
    _volume = wxAtoi(node->GetAttribute("Volume", "-1"));
    _priority = wxAtoi(node->GetAttribute("Priority", "0"));
    _name = node->GetAttribute("Name", "");
    if (node->GetAttribute("RestOfStep", "FALSE") == "TRUE") {
        _restOfStep = true;
    }
}

PlayListItem::PlayListItem() {
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
    _type = "";
}

void PlayListItem::Save(wxXmlNode* node) {
    node->AddAttribute("Delay", wxString::Format(wxT("%i"), (long)_delay));
    node->AddAttribute("Name", _name);
    if (_volume != -1) {
        node->AddAttribute("Volume", wxString::Format(wxT("%i"), _volume));
    }
    if (_priority != 0) {
        node->AddAttribute("Priority", wxString::Format(wxT("%i"), (int)_priority));
    }
    if (_restOfStep) {
        node->AddAttribute("RestOfStep", "TRUE");
    }
}

std::string PlayListItem::GetName() const {
    std::string duration = "";
    if (GetDurationMS() != 0) {
        duration = " [" + wxString::Format(wxT("%.3f"), (float)GetDurationMS() / 1000.0).ToStdString() + "]";
    }

    return GetNameNoTime() + duration;
}

std::string PlayListItem::GetNameNoTime() const {
    if (_name != "")
        return _name;

    return "<unnamed>";
}

void PlayListItem::Copy(PlayListItem* to, const bool isClone) const {
    if (!isClone) {
        // During a clone the following members are not copied because we want the values set in the constructor.
        to->_id = _id;
        to->_lastSavedChangeCount = _lastSavedChangeCount;
        to->_changeCount = _changeCount;
    }
    to->_delay = _delay;
    to->_frames = _frames;
    to->_msPerFrame = _msPerFrame;
    to->_name = _name;
    to->_priority = _priority;
    to->_volume = _volume;
    to->_restOfStep = _restOfStep;
    to->_stepLengthMS = _stepLengthMS;
    to->_type = _type;
}

PlayListItem* PlayListItem::Clone() const {
    return Copy(true);
}

bool PlayListItem::IsInSlaveMode() const {
    ScheduleManager* sm = xScheduleFrame::GetScheduleManager();

    if (sm == nullptr)
        return false;

    return sm->GetSyncManager()->IsSlave();
}

bool PlayListItem::IsSuppressAudioOnSlaves() const {
    ScheduleManager* sm = xScheduleFrame::GetScheduleManager();

    if (sm == nullptr)
        return false;

    return sm->GetOptions()->IsSuppressAudioOnRemotes();
}

std::string PlayListItem::ReplaceTags(const std::string s) const {
    wxString res = s;
    AudioManager* am = nullptr;

    auto now = wxDateTime::Now();

    res.Replace("\\n", "\n");
    res.Replace("\\t", "\t");
    res.Replace("\\\\", "\\");

    res.Replace("%TIMESTAMP%", now.Format("%F %T"));
    res.Replace("%TIME%", now.Format("%T"));
    res.Replace("%DATE%", now.Format("%F"));
    res.Replace("%MACHINENAME%", wxGetHostName());

    PlayList* pl = xScheduleFrame::GetScheduleManager()->GetRunningPlayList();
    if (pl != nullptr) {
        if (res.Contains("%RUNNING_PLAYLIST%")) {
            res.Replace("%RUNNING_PLAYLIST%", pl->GetNameNoTime(), true);
        }

        PlayListStep* pls = pl->GetRunningStep();
        if (pls != nullptr) {
            if (am == nullptr)
                am = pls->GetAudioManager();

            if (res.Contains("%RUNNING_PLAYLISTSTEP%")) {
                res.Replace("%RUNNING_PLAYLISTSTEP%", pls->GetNameNoTime(), true);
            }
            if (res.Contains("%STEPNAME%")) {
                res.Replace("%STEPNAME%", pls->GetNameNoTime(), true);
            }
            if (res.Contains("%RUNNING_PLAYLISTSTEPMS%")) {
                res.Replace("%RUNNING_PLAYLISTSTEPMS%", wxString::Format(wxT("%i"), pls->GetLengthMS()), true);
            }
            if (res.Contains("%RUNNING_PLAYLISTSTEPMSLEFT%")) {
                res.Replace("%RUNNING_PLAYLISTSTEPMSLEFT%", wxString::Format(wxT("%i"), pls->GetLengthMS() - pls->GetPosition()), true);
            }

            if (pl != nullptr && !pl->IsRandom()) {
                bool dummy;
                auto nextstep = pl->GetNextStep(dummy);
                if (nextstep != nullptr) {
                    res.Replace("%NEXTSTEPNAME%", nextstep->GetNameNoTime());
                }
            }
        }
    }
    if (res.Contains("%RUNNING_SCHEDULE%")) {
        RunningSchedule* rs = xScheduleFrame::GetScheduleManager()->GetRunningSchedule();
        if (rs != nullptr && rs->GetPlayList()->IsRunning()) {
            res.Replace("%RUNNING_SCHEDULE%", rs->GetSchedule()->GetName(), true);
        }
    }
    res.Replace("%NEXTSTEPNAME%", "");

    if (am != nullptr) {
        res.Replace("%TITLE%", am->Title());
        res.Replace("%ARTIST%", am->Artist());
        res.Replace("%ALBUM%", am->Album());
    }

    return res.ToStdString();
}

std::string PlayListItem::GetTagHint() {
    return "Available variables:\n    %RUNNING_PLAYLIST% - current playlist\n    %RUNNING_PLAYLISTSTEP% - step name\n    %RUNNING_PLAYLISTSTEPMS% - Position in current step\n    %RUNNING_PLAYLISTSTEPMSLEFT% - Time left in current step\n    %RUNNING_SCHEDULE% - Name of schedule\n    %STEPNAME% - Current step\n    %NEXTSTEPNAME% - Next step\n    %NEXTSTEPNAME% - Next step\n    %ALBUM% - from mp3\n    %TITLE% - from mp3\n    %ARTIST% - from mp3\n    %TIMESTAMP% - timestamp\n    %TIME% - time now\n    %MACHINENAME% - computer name\n    %DATE% - date now";
}
