#ifndef SCHEDULEMANAGER_H
#define SCHEDULEMANAGER_H
#include <list>
#include "PlayList/PlayList.h"
//#include "Schedule/Schedule.h"

class ScheduleManager
{
    std::string _showDir;
    bool _dirty;
	std::list<PlayList*> _playLists;
	//std::list<Schedule*> _schedules;
	
    public:

        bool IsDirty();
        ScheduleManager(const std::string& showDir);
        virtual ~ScheduleManager();
		static std::string GetScheduleFile() { return "xlights.xschedule"; }
		void Save();
        void AddPlayList(PlayList* playlist) { _playLists.push_back(playlist); _dirty = true; }
};

#endif 