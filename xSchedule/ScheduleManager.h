#ifndef SCHEDULEMANAGER_H
#define SCHEDULEMANAGER_H
#include <list>
#include "PlayList/PlayList.h"

class ScheduleManager
{
    std::string _showDir;
    bool _dirty;
	std::list<PlayList*> _playLists;
	
    public:

        bool IsDirty();
        void ClearDirty();
        ScheduleManager(const std::string& showDir);
        virtual ~ScheduleManager();
		static std::string GetScheduleFile() { return "xlights.xschedule"; }
		void Save();
        void AddPlayList(PlayList* playlist) { _playLists.push_back(playlist); _dirty = true; }
        std::list<PlayList*> GetPlayLists() const { return _playLists; }
};

#endif 