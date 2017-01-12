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

    std::string FormatTime(size_t timems);

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
        void AddPlayList(PlayList* playlist);
        void RemovePlayList(PlayList* playlist);
        PlayList* GetRunningPlayList() const;
        std::list<PlayList*> GetPlayLists();
        std::list<std::string> GetCommands() const;
        bool IsRunning() const { return GetRunningPlayList() != nullptr; }
        void Frame(); // called when a frame needs to be displayed ... returns desired frame rate
        int CheckSchedule();
        std::string GetShowDir() const { return _showDir; }
        bool PlayPlayList(PlayList* playlist, size_t& rate, bool loop = false, const std::string& step = "", bool forcelast = false);
        bool IsSomethingPlaying() const { return GetRunningPlayList() != nullptr; }
        void OptionsChanged() {};
        bool Action(const std::string label, PlayList* playlist, size_t& rate, std::string& msg);
        bool Action(const std::string command, const std::string parameters, PlayList* playlist, size_t& rate, std::string& msg);
        bool Query(const std::string command, const std::string parameters, std::string& data, std::string& msg);
        PlayList * GetPlayList(const std::string& playlist) const;
        void StopPlayList(PlayList* playlist, bool atendofcurrentstep);
        bool StoreData(const std::string& key, const std::string& data, std::string& msg) const;
        bool RetrieveData(const std::string& key, std::string& data, std::string& msg) const;
        bool ToggleOutputToLights(std::string& msg);
        bool ToggleCurrentPlayListRandom(std::string& msg);
        bool ToggleCurrentPlayListPause(std::string& msg);
        bool ToggleCurrentPlayListLoop(std::string& msg);
        bool ToggleCurrentPlayListStepLoop(std::string& msg);
        bool IsOutputToLights() const;
        bool IsCurrentPlayListScheduled() const { return _immediatePlay == nullptr; }
};

#endif
