#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <list>
#include <vector>
#include <string>
#include <wx/wx.h>

enum class PARMTYPE { PLAYLIST, STEP, SCHEDULE, INTEGER, STRING, COMMAND, ANY, ITEM};

class PlayList;
class PlayListStep;
class Schedule;
class ScheduleManager;

class Command
{
	public:
	std::string _command;
	wxString _commandLower;
	int _parms;
    bool _requiresSelectedPlaylist;
    bool _requiresSelectedPlaylistStep;
    bool _requiresSelectedSchedule;
	bool _requiresPlayingPlaylist;
	bool _requiresPlayingSchedule;
    bool _worksInSlaveMode;
    bool _worksInQueuedMode;
    bool _userSelectable;
    bool _uiOnly;
	std::vector<PARMTYPE> _parmtype;
	Command(const std::string& name, int parms, const PARMTYPE *parmtypes, bool reqSelPL, bool reqSelPLS, bool reqSelSch, bool reqPlayPL, bool reqPlaySch, bool worksInSlaveMode, bool worksInQueuedMode, bool userSelectable, bool uiOnly);
    bool IsValid(wxString parms, PlayList* selectedPlayList, PlayListStep* selectedPlayListStep, Schedule* selectedSchedule, ScheduleManager* scheduleManager, wxString& msg, bool queueMode) const;
    bool IsUserSelectable() const { return _userSelectable; }
    bool IsUIOnly() const { return _uiOnly; }
    std::string GetParametersTip() const;
    void SetCommand(std::string command) { _command = command; _commandLower = wxString(_command).Lower(); }
};

class CommandManager
{
	std::list<Command*> _commands;

    public:

		CommandManager();
		virtual ~CommandManager();
		std::list<Command*> GetCommands() const { return _commands; }
		Command* GetCommand(std::string name) const;
        std::string GetCommandParametersTip(const std::string command) const;
};
