#ifndef SCHEDULEMANAGER_H
#define SCHEDULEMANAGER_H
#include <list>
#include <string>
#include <wx/wx.h>
#include "Schedule.h"
#include "CommandManager.h"

class ScheduleOptions;
class PlayList;
class OutputManager;
class RunningSchedule;
class wxSocketClient;
class PlayListStep;

typedef enum
{
    STANDALONE,
    FPPMASTER,
    FPPSLAVE,
    ARTNETMASTER,
    ARTNETSLAVE
} SYNCMODE;

class ScheduleManager
{
    SYNCMODE _mode;
    int _manualOTL;
    std::string _showDir;
    int _lastSavedChangeCount;
    int _changeCount;
	std::list<PlayList*> _playLists;
    ScheduleOptions* _scheduleOptions;
    OutputManager* _outputManager;
    wxByte* _buffer;
    wxUint32 _startTime;
    PlayList* _immediatePlay;
    CommandManager _commandManager;
    PlayList* _queuedSongs;
    std::list<RunningSchedule*> _activeSchedules;
    int _brightness;
    int _lastBrightness;
    wxByte _brightnessArray[255];
    wxSocketClient* _fppSync;

    std::string FormatTime(size_t timems);
    void CreateBrightnessArray();
    void SendFPPSync(const std::string& syncItem, size_t msec);
    void OpenFPPSyncSendSocket();
    void CloseFPPSyncSendSocket();
    void EnqueueSong(PlayListStep* step);

    public:

        void SetMode(SYNCMODE mode);
        SYNCMODE GetMode() const { return _mode; }
        void ToggleMute();
        void SetVolume(int volume);
        void AdjustVolumeBy(int volume);
        int GetVolume() const;
        int GetManualOutputToLights() const { return _manualOTL; }
        void ManualOutputToLightsClick();
        bool IsScheduleActive(Schedule* schedue);
        std::list<RunningSchedule*> GetRunningSchedules() const { return _activeSchedules; }
        RunningSchedule* GetRunningSchedule() const;
        RunningSchedule* GetRunningSchedule(const std::string& schedulename) const;
        RunningSchedule* GetRunningSchedule(Schedule* schedule) const;
        ScheduleOptions* GetOptions() const { return _scheduleOptions; }
        bool IsDirty();
        void ClearDirty();
        size_t GetTotalChannels() const;
        ScheduleManager(const std::string& showDir);
        virtual ~ScheduleManager();
        std::string GetStatus() const;
		static std::string GetScheduleFile() { return "xlights.xschedule"; }
		void Save();
        void StopAll();
        void AddPlayList(PlayList* playlist);
        bool IsQueuedPlaylistRunning() const;
        void RemovePlayList(PlayList* playlist);
        PlayList* GetRunningPlayList() const;
        std::list<PlayList*> GetPlayLists();
        std::list<Command*> GetCommands() const { return _commandManager.GetCommands(); }
        Command* GetCommand(std::string command) const { return _commandManager.GetCommand(command); }
        bool IsRunning() const { return GetRunningPlayList() != nullptr; }
        int GetBrightness() const { return _brightness; }
        void AdjustBrightness(int by) { _brightness += by; if (_brightness < 0) _brightness = 0; else if (_brightness > 100) _brightness = 100; }
        void SetBrightness(int brightness) { if (brightness < 0) _brightness = 0; else if (brightness > 100) _brightness = 100; else _brightness = brightness; }
        void Frame(bool outputframe); // called when a frame needs to be displayed ... returns desired frame rate
        int CheckSchedule();
        std::string GetShowDir() const { return _showDir; }
        bool PlayPlayList(PlayList* playlist, size_t& rate, bool loop = false, const std::string& step = "", bool forcelast = false, int loops = -1, bool random = false, int steploops = -1);
        bool IsSomethingPlaying() const { return GetRunningPlayList() != nullptr; }
        void OptionsChanged() {};
        bool Action(const std::string label, PlayList* selplaylist, Schedule* selschedule, size_t& rate, std::string& msg);
        bool Action(const std::string command, const std::string parameters, PlayList* selplaylist, Schedule* selschedule, size_t& rate, std::string& msg);
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
        void SetOutputToLights(bool otl);
};

#endif
