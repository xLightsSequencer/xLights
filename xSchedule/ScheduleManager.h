#ifndef SCHEDULEMANAGER_H
#define SCHEDULEMANAGER_H
#include <list>
#include <string>
#include <wx/wx.h>

class ScheduleOptions;
class PlayList;
class OutputManager;

class ScheduleManager
{
    std::string _showDir;
    bool _dirty;
	std::list<PlayList*> _playLists;
    ScheduleOptions* _scheduleOptions;
    OutputManager* _outputManager;
    wxByte* _buffer;
    wxLongLong _startTime;
    PlayList* _immediatePlay;

    public:

        ScheduleOptions* GetOptions() const { return _scheduleOptions; }
        bool IsDirty();
        void ClearDirty();
        ScheduleManager(const std::string& showDir);
        virtual ~ScheduleManager();
        std::string GetStatus() const;
		static std::string GetScheduleFile() { return "xlights.xschedule"; }
		void Save();
        void StopAll();
        void AddPlayList(PlayList* playlist) { _playLists.push_back(playlist); _dirty = true; }
        void RemovePlayList(PlayList* playlist);
        PlayList* GetRunningPlayList() const;
        std::list<PlayList*> GetPlayLists() const { return _playLists; }
        std::list<std::string> GetCommands() const;
        bool IsRunning() const { return GetRunningPlayList() != nullptr; }
        void Frame(); // called when a frame needs to be displayed ... returns desired frame rate
        int CheckSchedule();
        int PlayPlayList(PlayList* playlist);
        bool IsSomethingPlaying() const { return GetRunningPlayList() != nullptr; }
        void OptionsChanged() {};
        void Action(const std::string label, PlayList* playlist, size_t& rate);
        void Action(const std::string command, const std::string parameters, PlayList* playlist, size_t& rate);
        std::string Query(const std::string command, const std::string parameters);
        PlayList * GetPlayList(const std::string& playlist) const;
        void StopPlayList(PlayList* playlist, bool atendofcurrentstep);
};

#endif
