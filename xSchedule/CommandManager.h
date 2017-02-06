#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H
#include <list>
#include <vector>
#include <string>

typedef enum { PLAYLIST, STEP, SCHEDULE, INTEGER, STRING, COMMAND, ANY} PARMTYPE;

class PlayList;
class Schedule;
class ScheduleManager;

class Command
{
	public:
	std::string _command;
	int _parms;
	bool _requiresSelectedPlaylist;
	bool _requiresSelectedSchedule;
	bool _requiresPlayingPlaylist;
	bool _requiresPlayingSchedule;
    bool _worksInSlaveMode;
    bool _worksInQueuedMode;
    bool _userSelectable;
    bool _uiOnly;
	std::vector<PARMTYPE> _parmtype;
	Command(const std::string& name, int parms, const PARMTYPE *parmtypes, bool reqSelPL, bool reqSelSch, bool reqPlayPL, bool reqPlaySch, bool worksInSlaveMode, bool worksInQueuedMode, bool userSelectable, bool uiOnly);
    bool IsValid(std::string parms, PlayList* selectedPlayList, Schedule* selectedSchedule, ScheduleManager* scheduleManager, std::string& msg, bool queueMode);
    bool IsUserSelectable() const { return _userSelectable; }
    bool IsUIOnly() const { return _uiOnly; }
};

class CommandManager
{
	std::list<Command*> _commands;

    public:

		CommandManager();
		virtual ~CommandManager();
		std::list<Command*> GetCommands() const { return _commands; }
		Command* GetCommand(std::string name) const;
};

#endif
