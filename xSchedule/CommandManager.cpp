/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>

#include "CommandManager.h"
#include "ScheduleManager.h"
#include "PlayList/PlayList.h"
#include "PlayList/PlayListStep.h"
#include "Schedule.h"
#include "PlayList/PlayListItem.h"

Command::Command(const std::string& name, int parms, const PARMTYPE* parmtypes, bool reqSelPL, bool reqSelPLS, bool reqSelSch, bool reqPlayPL, bool reqPlaySch, bool worksinslavemode, bool worksInQueuedMode, bool userSelectable, bool uiOnly)
{
    // req sel pl step must also req sel PL
    wxASSERT(!reqSelPLS || (reqSelPLS && reqSelPL));

    SetCommand(name);
    _parms = parms;
    for (int i = 0; i < parms; i++)
    {
        _parmtype.push_back(parmtypes[i]);
    }
    _requiresSelectedPlaylist = reqSelPL;
    _requiresSelectedPlaylistStep = reqSelPLS;
    _requiresSelectedSchedule = reqSelSch;
    _requiresPlayingPlaylist = reqPlayPL;
    _requiresPlayingSchedule = reqPlaySch;
    _worksInSlaveMode = worksinslavemode;
    _worksInQueuedMode = worksInQueuedMode;
    _userSelectable = userSelectable;
    _uiOnly = uiOnly;
}

std::string Command::GetParametersTip() const
{
    if (_parms <= 0) return "";

    std::string tip = "";

    for (int i = 0; i < _parms; i++)
    {
        if (tip != "")
        {
            tip += ",";
        }

        switch (_parmtype[i])
        {
        case PARMTYPE::INTEGER:
            tip += "<number>";
            break;
        case PARMTYPE::PLAYLIST:
            tip += "<playlist name>";
            break;
        case PARMTYPE::STEP:
            tip += "<playlist step name>";
            break;
        case PARMTYPE::SCHEDULE:
            tip += "<schedule name>";
            break;
        case PARMTYPE::STRING:
            tip += "<text>";
            break;
        case PARMTYPE::COMMAND:
            tip += "<command>";
            break;
        case PARMTYPE::ANY:
            tip += "<?>";
            break;
        case PARMTYPE::ITEM:
            tip += "<playlist item name>";
            break;
        }
    }

    return tip;
}

bool Command::IsValid(wxString parms, PlayList* selectedPlayList, PlayListStep* selectedPlayListStep, Schedule* selectedSchedule, ScheduleManager* scheduleManager, wxString& msg, bool queuedMode) const
{
    auto components = wxSplit(parms, ',');

    if (_parms != -1 && components.Count() != _parms)
    {
        msg = wxString::Format("Invalid number of parameters. Found %d when there should be %d.", (int)components.Count(), _parms).ToStdString();
        return false;
    }

    if (queuedMode && !_worksInQueuedMode)
    {
        msg = "Command not valid when running in queued mode.";
        return false;
    }

    if (!_worksInSlaveMode && scheduleManager->IsSlave())
    {
        msg = "Command not valid when running in FPP Remote mode.";
        return false;
    }

    if (_requiresSelectedPlaylist && selectedPlayList == nullptr)
    {
        msg = "Playlist not selected.";
        return false;
    }

    if (_requiresSelectedPlaylistStep && selectedPlayListStep == nullptr)         {
        msg = "Playlist step not selected.";
        return false;
    }

    if (_requiresSelectedSchedule && selectedSchedule == nullptr)
    {
        msg = "Schedule not selected.";
        return false;
    }

    PlayList* pl = scheduleManager->GetRunningPlayList();
    if (_requiresPlayingPlaylist && pl == nullptr && scheduleManager->GetEventPlayLists().size() == 0 && scheduleManager->GetNonStoppedCount() == 0)
    {
        msg = "Playlist not playing.";
        return false;
    }

    RunningSchedule* sch = scheduleManager->GetRunningSchedule();
    if (_requiresPlayingSchedule && sch == nullptr && scheduleManager->GetNonStoppedCount() == 0)
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
        else if (_parmtype[i] == PARMTYPE::PLAYLIST && scheduleManager->GetPlayList(components[i].ToStdString()) == nullptr && scheduleManager->GetPlayList(scheduleManager->DecodePlayList(components[i].ToStdString())) == nullptr)
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
                    pl = scheduleManager->GetPlayList(scheduleManager->DecodePlayList(components[i-1].ToStdString()));

                    if (pl == nullptr)
                    {
                        msg = wxString::Format("Parameter %d: '%s' is not a known playlist.", i, components[i - 1].c_str()).ToStdString();
                        return false;
                    }
                }
            }

            if (pl->GetStep(components[i].ToStdString()) == nullptr && pl->GetStep(scheduleManager->DecodeStep(components[i].ToStdString())) == nullptr)
            {
                msg = wxString::Format("Parameter %d: '%s' is not a known step in playlist '%s'.", i+1, components[i].c_str(), pl->GetNameNoTime().c_str()).ToStdString();
                return false;
            }
        }
        else if (_parmtype[i] == PARMTYPE::ITEM)
        {
            // assume the prior parameters are playlist and step ... this is important
            if (components.Count() < 3)
            {
                    msg = "Not enough parameters.";
                    return false;
            }

            pl = scheduleManager->GetPlayList(components[i - 2].ToStdString());
            if (pl == nullptr)
            {
                pl = scheduleManager->GetPlayList(scheduleManager->DecodePlayList(components[i - 2].ToStdString()));

                if (pl == nullptr)
                {
                    msg = wxString::Format("Parameter %d: '%s' is not a known playlist.", i - 1, components[i - 2].c_str()).ToStdString();
                    return false;
                }
            }

            PlayListStep* pls = pl->GetStep(components[i - 1].ToStdString());
            if (pls == nullptr)
            {
                pls = pl->GetStep(scheduleManager->DecodeStep(components[i - 1].ToStdString()));
                if (pls == nullptr)
                {
                    msg = wxString::Format("Parameter %d: '%s' is not a known step in playlist '%s'.", i, components[i - 1].c_str(), pl->GetNameNoTime().c_str()).ToStdString();
                    return false;
                }
            }

            PlayListItem* pli = pls->GetItem(components[i].ToStdString());
            if (pli == nullptr)
            {
                pli = pls->GetItem(scheduleManager->DecodeItem(components[i].ToStdString()));
                if (pli == nullptr)
                {
                    msg = wxString::Format("Parameter %d: '%s' is not a known item in step '%s' in playlist '%s'.", i + 1, components[i].c_str(), pls->GetNameNoTime().c_str(), pl->GetNameNoTime().c_str()).ToStdString();
                    return false;
                }
            }

            if (pli->GetTitle() != "Run Process")
            {
                msg = wxString::Format("Parameter %d: '%s' is not a run process item in step '%s' in playlist '%s'.", i + 1, components[i].c_str(), pls->GetNameNoTime().c_str(), pl->GetNameNoTime().c_str()).ToStdString();
                return false;
            }
        }
    }

    return true;
}

Command* CommandManager::GetCommand(std::string name) const
{
    auto n = wxString(name).Lower();
    for (auto it : _commands)
    {
        if (it->_commandLower == n)
        {
            return it;
        }
    }

    return nullptr;
}

std::string CommandManager::GetCommandParametersTip(const std::string command) const
{
    Command* c = GetCommand(command);

    if (c != nullptr)
    {
        return c->GetParametersTip();
    }

    return "";
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
    PARMTYPE ss[] = { PARMTYPE::STRING, PARMTYPE::STRING };
    PARMTYPE sss[] = { PARMTYPE::STRING, PARMTYPE::STRING, PARMTYPE::STRING };
    PARMTYPE iiss[] = { PARMTYPE::INTEGER, PARMTYPE::INTEGER, PARMTYPE::STRING, PARMTYPE::STRING };
    PARMTYPE sch[] = { PARMTYPE::SCHEDULE };
    // PARMTYPE c[] = { PARMTYPE::COMMAND };
    PARMTYPE plstit[] = { PARMTYPE::PLAYLIST, PARMTYPE::STEP, PARMTYPE::ITEM };

    _commands.push_back(new Command("Stop all now", 0, {}, false, false, false, true, false, false, true, true, false));
    _commands.push_back(new Command("Stop", 0,{}, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Play selected playlist", 0, {}, true, false, false, false, false, false, true, true, true));
    _commands.push_back(new Command("Play selected playlist looped", 0, {}, true, false, false, false, false, false, true, true, true));
    _commands.push_back(new Command("Play selected playlist step", 0, {}, true, true, false, false, false, false, true, true, true));
    _commands.push_back(new Command("Play selected playlist step looped", 0, {}, true, true, false, false, false, false, true, true, true));
    _commands.push_back(new Command("Play specified playlist", 1, pl, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Play specified playlist if not running", 1, pl, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Play specified playlist if nothing running", 1, pl, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Play specified playlist looped", 1, pl, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Stop specified playlist", 1, pl, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Stop specified playlist at end of current step", 1, pl, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Stop specified playlist at end of current loop", 1, pl, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Stop playlist at end of current step", 0, {}, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Stop playlist at end of current loop", 0 ,{}, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Jump to play once at end at end of current step and then stop", 0, {}, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Pause", 0, {}, false, false, false, true, false, false, true, true, false));
    _commands.push_back(new Command("Next step in current playlist", 0, {}, false, false, false, true, false, false, true, true, false));
    _commands.push_back(new Command("Restart step in current playlist", 0, {}, false, false, false, true, false, false, true, true, false));
    _commands.push_back(new Command("Prior step in current playlist", 0, {}, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Jump to random step in current playlist", 0, {}, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Jump to random step in specified playlist", 1, pl, false, false, false, false, false, false, false, true, false));
    _commands.push_back(new Command("Play one random step in specified playlist", 1, pl, false, false, false, false, false, false, false, true, false));
    _commands.push_back(new Command("Jump to specified step in current playlist", 1, st, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Jump to specified step in current playlist at the end of current step", 1, st, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Play playlist starting at step", 2, plst, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Play playlist step", 2, plst, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Play playlist starting at step looped", 2, plst, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Toggle loop current step", 0, {}, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Play specified step in specified playlist looped", 2, plst, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Add to the current schedule n minutes", 1, i, false, false, false, true, true, false, true, true, false));
    _commands.push_back(new Command("Set volume to", 1, i, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Adjust volume by", 1, i, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Save schedule", 0, {}, false, false, false, false, false, true, true, true, true));
    _commands.push_back(new Command("Toggle output to lights", 0, {}, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Toggle current playlist random", 0, {}, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Toggle current playlist loop", 0, {}, false, false, false, true, false, false, false, true, false));
    _commands.push_back(new Command("Play specified playlist step once only", 2, plst, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Play specified playlist n times", 2, pli, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Play specified playlist step n times", 3, plsti, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Increase brightness by n%", 1, i, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Set brightness to n%", 1, i, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("PressButton", 1, s, false, false, false, false, false, true, true, false, true));
    _commands.push_back(new Command("Restart selected schedule", 0, {}, false, false, true, false, false, false, true, true, true));
    _commands.push_back(new Command("Restart all schedules", 0, {}, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Restart playlist schedules", 1, pl, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Restart named schedule", 1, sch, false, false, false, false, true, false, true, true, false));
    _commands.push_back(new Command("Toggle mute", 0, {}, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Enqueue playlist step", 2, plst, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Clear playlist queue", 0, {}, false, false, false, true, false, false, true, true, false));
    _commands.push_back(new Command("Refresh current playlist", 0, {}, false, false, false, true, false, false, false, true, false)); // this is called to load a changed playlist that is currently playing
    _commands.push_back(new Command("Run command at end of current step", -1, {}, false, false, false, true, false, false, true, true, false)); // this is called to run a command but only at the end of the current step
    _commands.push_back(new Command("Bring to foreground", 0, {}, false, false, false, false, false, true, true, false, false)); // this is useful from the web UI to force the scheduler into the foreground on the PC
    _commands.push_back(new Command("Set current text", 3, sss, false, false, false, true, false, true, true, false, true)); // <text name>,<text>, <properties>
    _commands.push_back(new Command("Set pixels", 2, ss, false, false, false, false, false, true, true, false, true)); // <set channels name>,<base64 encoded data>, <properties>
    _commands.push_back(new Command("Set pixel range", 4, iiss, false, false, false, false, false, true, true, false, true)); // <startchannel>,<channels>,<color>,<blendmode>
    _commands.push_back(new Command("Run process", 3, plstit, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Run event playlist step", 2, plst, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Run event playlist step unique", 2, plst, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Run event playlist step if idle", 2, plst, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Run event playlist step looped", 2, plst, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Run event playlist step unique looped", 2, plst, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Run event playlist step if idle looped", 2, plst, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Stop event playlist", 1, pl, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Stop event playlist if playing step", 2, plst, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Activate specified schedule", 1, sch, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Deactivate specified schedule", 1, sch, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Activate all schedules", 0, {}, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Deactivate all schedules", 0, {}, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Set playlist as background", 1, pl, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Clear background playlist", 0, {}, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Close xSchedule", 0, {}, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Add n Seconds To Current Step Position", 1, i, false, false, false, true, false, false, true, true, false));
    _commands.push_back(new Command("Start test mode", 1, s, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Stop test mode", 0, {}, false, false, false, false, false, false, true, true, false));
    _commands.push_back(new Command("Change show folder", 1, s, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Set mode", 1, s, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Fire plugin event", 1, s, false, false, false, false, false, true, true, true, false));
    _commands.push_back(new Command("Set step position", 1, i, false, false, false, true, false, false, true, true, false));
    _commands.push_back(new Command("Set step position ms", 1, i, false, false, false, true, false, false, true, true, false));
    _commands.push_back(new Command("Adjust frame interval by ms", 1, i, false, false, false, true, false, false, true, true, false));
    _commands.push_back(new Command("Set frame interval to ms", 1, i, false, false, false, true, false, false, true, true, false));
}
