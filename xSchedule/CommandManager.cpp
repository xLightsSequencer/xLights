#include "CommandManager.h"
#include <wx/wx.h>
#include "ScheduleManager.h"
#include "PlayList/PlayList.h"
#include "Schedule.h"

Command::Command(const std::string& name, int parms, const PARMTYPE* parmtypes, bool reqSelPL, bool reqSelSch, bool reqPlayPL, bool reqPlaySch, bool worksinslavemode, bool worksInQueuedMode)
{
    _command = name;
    _parms = parms;
    for (int i = 0; i < parms; i++)
    {
        _parmtype.push_back(parmtypes[i]);
    }
    _requiresSelectedPlaylist = reqSelPL;
    _requiresSelectedSchedule = reqSelSch;
    _requiresPlayingPlaylist = reqPlayPL;
    _requiresPlayingSchedule = reqPlaySch;
    _worksInSlaveMode = worksinslavemode;
    _worksInQueuedMode = worksInQueuedMode;
}

bool Command::IsValid(std::string parms, PlayList* selectedPlayList, Schedule* selectedSchedule, ScheduleManager* scheduleManager, std::string& msg, bool queuedMode)
{
    auto components = wxSplit(parms, ',');

    if (components.Count() != _parms)
    {
        msg = wxString::Format("Invalid number of parameters. Found %d when there should be %d.", components.Count(), _parms).ToStdString();
        return false;
    }

    if (queuedMode && !_worksInQueuedMode)
    {
        msg = "Command not valid when running in queued mode.";
        return false;
    }

    if (!_worksInSlaveMode && scheduleManager->GetMode() == SYNCMODE::FPPSLAVE)
    {
        msg = "Command not valid when running in FPP Remote mode.";
        return false;
    }

    if (_requiresSelectedPlaylist && selectedPlayList == nullptr)
    {
        msg = "Playlist not selected.";
        return false;
    }

    if (_requiresSelectedSchedule && selectedSchedule == nullptr)
    {
        msg = "Schedule not selected.";
        return false;
    }

    PlayList* pl = scheduleManager->GetRunningPlayList();
    if (_requiresPlayingPlaylist && pl == nullptr)
    {
        msg = "Playlist not playing.";
        return false;
    }

    RunningSchedule* sch = scheduleManager->GetRunningSchedule();
    if (_requiresPlayingSchedule && sch == nullptr)
    {
        msg = "Schedule not playing.";
        return false;
    }

    for (int i = 0; i < _parms; i++)
    {
        if (_parmtype[i] == PARMTYPE::INTEGER && !components[i].IsNumber())
        {
            msg = wxString::Format("Parameter %d: '%s' is not a number.", i + 1, components[i].c_str()).ToStdString();
            return false;
        }
        else if (_parmtype[i] == PARMTYPE::PLAYLIST && scheduleManager->GetPlayList(components[i].ToStdString()) == nullptr)
        {
            msg = wxString::Format("Parameter %d: '%s' is not a known playlist.", i + 1, components[i].c_str()).ToStdString();
            return false;
        }
        else if (_parmtype[i] == PARMTYPE::STEP)
        {
            // assume the prior parameter is a playlist ... this is important
            if (components.Count() < 2)
            {
                pl = scheduleManager->GetRunningPlayList();
                if (pl == nullptr)
                {
                    msg = "No playlist running.";
                    return false;
                }
            }
            else
            {
                pl = scheduleManager->GetPlayList(components[i - 1].ToStdString());
                if (pl == nullptr)
                {
                    msg = wxString::Format("Parameter %d: '%s' is not a known playlist.", i, components[i - 1].c_str()).ToStdString();
                    return false;
                }
            }

            if (pl->GetStep(components[i].ToStdString()) == nullptr)
            {
                msg = wxString::Format("Parameter %d: '%s' is not a known step in playlist '%s'.", i+1, components[i].c_str(), pl->GetNameNoTime().c_str()).ToStdString();
                return false;
            }
        }
    }

    return true;
}

Command* CommandManager::GetCommand(std::string name) const
{
    for (auto it = _commands.begin(); it != _commands.end(); ++it)
    {
        if ((*it)->_command == name)
        {
            return *it;
        }
    }

    return nullptr;
}

CommandManager::~CommandManager()
{
    while (_commands.size() > 0)
    {
        auto toremove = _commands.front();
        _commands.remove(toremove);
        delete toremove;
    }
}

CommandManager::CommandManager()
{
    PARMTYPE pl[] = { PARMTYPE::PLAYLIST };
    PARMTYPE st[] = { PARMTYPE::STEP };
    PARMTYPE plst[] = { PARMTYPE::PLAYLIST, PARMTYPE::STEP };
    PARMTYPE pli[] = { PARMTYPE::PLAYLIST, PARMTYPE::INTEGER };
    PARMTYPE plsti[] = { PARMTYPE::PLAYLIST, PARMTYPE::STEP, PARMTYPE::INTEGER };
    PARMTYPE i[] = { PARMTYPE::INTEGER };
    PARMTYPE s[] = { PARMTYPE::STRING };
    PARMTYPE sch[] = { PARMTYPE::SCHEDULE };

    _commands.push_back(new Command("Stop all now", 0, {}, false, false, true, false, false, true));
    _commands.push_back(new Command("Stop", 0,{}, false, false, true, false, false, false));
    _commands.push_back(new Command("Play selected playlist", 0, {}, true, false, false, false, false, true));
    _commands.push_back(new Command("Play selected playlist looped", 0, {}, true, false, false, false, false, true));
    _commands.push_back(new Command("Play specified playlist", 1, pl, false, false, false, false, false, true));
    _commands.push_back(new Command("Play specified playlist looped", 1, pl, false, false, false, false, false, true));
    _commands.push_back(new Command("Stop specified playlist", 1, pl, false, false, true, false, false, false));
    _commands.push_back(new Command("Stop specified playlist at end of current step", 1, pl, false, false, true, false, false, false));
    _commands.push_back(new Command("Stop specified playlist at end of current loop", 1, pl, false, false, true, false, false, false));
    _commands.push_back(new Command("Stop playlist at end of current step", 0, {}, false, false, true, false, false, false));
    _commands.push_back(new Command("Stop playlist at end of current loop", 0 ,{}, false, false, true, false, false, false));
    _commands.push_back(new Command("Jump to play once at end at end of current step and then stop", 0, {}, false, false, true, false, false, false));
    _commands.push_back(new Command("Pause", 0, {}, false, false, true, false, false, true));
    _commands.push_back(new Command("Next step in current playlist", 0, {}, false, false, true, false, false, true));
    _commands.push_back(new Command("Restart step in current playlist", 0, {}, false, false, true, false, false, true));
    _commands.push_back(new Command("Prior step in current playlist", 0, {}, false, false, true, false, false, false));
    _commands.push_back(new Command("Jump to random step in current playlist", 0, {}, false, false, true, false, false, false));
    _commands.push_back(new Command("Jump to random step in specified playlist", 1, pl, false, false, false, false, false, false));
    _commands.push_back(new Command("Jump to specified step in current playlist", 1, st, false, false, true, false, false, false));
    _commands.push_back(new Command("Jump to specified step in current playlist at the end of current step", 1, st, false, false, true, false, false, false));
    _commands.push_back(new Command("Play playlist starting at step", 2, plst, false, false, false, false, false, true));
    _commands.push_back(new Command("Play playlist step", 2, plst, false, false, false, false, false, true));
    _commands.push_back(new Command("Play playlist starting at step looped", 2, plst, false, false, false, false, false, true));
    _commands.push_back(new Command("Toggle loop current step", 0, {}, false, false, true, false, false, false));
    _commands.push_back(new Command("Play specified step in specified playlist looped", 2, plst, false, false, false, false, false, true));
    _commands.push_back(new Command("Add to the current schedule n minutes", 1, i, false, false, true, true, false, true));
    _commands.push_back(new Command("Set volume to", 1, i, false, false, false, false, true, true));
    _commands.push_back(new Command("Adjust volume by", 1, i, false, false, false, false, true, true));
    _commands.push_back(new Command("Save schedule", 0, {}, false, false, false, false, true, true));
    _commands.push_back(new Command("Toggle output to lights", 0, {}, false, false, false, false, true, true));
    _commands.push_back(new Command("Toggle current playlist random", 0, {}, false, false, true, false, false, false));
    _commands.push_back(new Command("Toggle current playlist loop", 0, {}, false, false, true, false, false, false));
    _commands.push_back(new Command("Play specified playlist step once only", 2, plst, false, false, false, false, false, true));
    _commands.push_back(new Command("Play specified playlist n times", 2, pli, false, false, false, false, false, true));
    _commands.push_back(new Command("Play specified playlist step n times", 3, plsti, false, false, false, false, false, true));
    _commands.push_back(new Command("Increase brightness by n%", 1, i, false, false, false, false, true, true));
    _commands.push_back(new Command("Set brightness to n%", 1, i, false, false, false, false, true, true));
    _commands.push_back(new Command("PressButton", 1, s, false, false, false, false, true, true));
    _commands.push_back(new Command("Restart selected schedule", 0, {}, false, true, false, true, false, true));
    _commands.push_back(new Command("Restart named schedule", 1, sch, false, false, false, true, false, true));
    _commands.push_back(new Command("Toggle mute", 0, {}, false, false, false, false, true, true));
    _commands.push_back(new Command("Enqueue playlist step", 2, plst, false, false, false, false, false, true));
    _commands.push_back(new Command("Clear playlist queue", 0, {}, false, false, true, false, false, true));
}

