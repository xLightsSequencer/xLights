#ifndef SCHEDULEMANAGER_H
#define SCHEDULEMANAGER_H
#include <list>
#include <string>
#include <wx/wx.h>
#include "Schedule.h"
#include "CommandManager.h"
#include "OSCPacket.h"
#include "FSEQFile.h"
#include <wx/socket.h>

class PlayListItemText;
class ScheduleOptions;
class PlayList;
class OutputManager;
class RunningSchedule;
class PlayListStep;
class OutputProcess;
class Xyzzy;
class PlayListItem;
class xScheduleFrame;
class Pinger;
class ListenerManager;

typedef enum
{
    STANDALONE,
    FPPMASTER,
    FPPSLAVE,
    FPPUNICASTSLAVE,
    ARTNETMASTER,
    ARTNETSLAVE,
    OSCMASTER,
    FPPOSCMASTER,
    OSCSLAVE
} SYNCMODE;

class PixelData
{
    size_t _startChannel;
    size_t _size;
    wxByte* _data;
    APPLYMETHOD _blendMode;

    void ExtractData(const std::string& data);

public:
    PixelData(size_t startChannel, const std::string& data, APPLYMETHOD blendMode);
    PixelData(size_t startChannel, size_t channels, const wxColor& c, APPLYMETHOD blendMode);
    virtual ~PixelData();
    void Set(wxByte* buffer, size_t size);
    void SetColor(const wxColor& c, APPLYMETHOD blendMode);
    void SetData(const std::string& data, APPLYMETHOD blendMode);
    long GetSize() const { return _size; }
    size_t GetStartChannel() const { return _startChannel; }
};

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
    PlayList* _backgroundPlayList;
    std::list<PlayList*> _eventPlayLists;
    std::list<PixelData*> _overlayData;
    CommandManager _commandManager;
    PlayList* _queuedSongs;
    std::list<RunningSchedule*> _activeSchedules;
    int _brightness;
    int _lastBrightness;
    wxByte _brightnessArray[255];
    wxDatagramSocket* _fppSyncMaster;
    wxDatagramSocket* _artNetSyncMaster;
    wxDatagramSocket* _oscSyncMaster;
    wxDatagramSocket* _fppSyncMasterUnicast;
    wxDatagramSocket* _oscSyncSlave;
    std::list<OutputProcess*> _outputProcessing;
    ListenerManager* _listenerManager;
    Xyzzy* _xyzzy;
    wxDateTime _lastXyzzyCommand;
    int _timerAdjustment;
    bool _webRequestToggle;
    Pinger* _pinger;

    std::string GetPingStatus();
    void SendOSC(const OSCPacket& osc);
    std::string FormatTime(size_t timems);
    void CreateBrightnessArray();
    void SendFPPSync(const std::string& syncItem, size_t msec, size_t frameMS);
    void SendARTNetSync(size_t msec, size_t frameMS);
    void SendOSCSync(PlayListStep* step, size_t msec, size_t frameMS);
    void SendUnicastSync(const std::string& ip, const std::string& syncItem, size_t msec, size_t frameMS, int action);
    void CloseFPPSyncSendSocket();
    void CloseARTNetSyncSendSocket();
    void CloseOSCSyncSendSocket();
    void ManageBackground();
    bool DoText(PlayListItemText* pliText, const std::string& text, const std::string& properties);
    void StartVirtualMatrices();
    void StopVirtualMatrices();
    void StartFSEQ(const std::string fseq);
    void StartStep(const std::string stepName);
    void StartTiming(const std::string timgingName);
    PlayListItem* FindRunProcessNamed(const std::string& item) const;

    public:

        void SetPinger(Pinger* pinger) { _pinger = pinger; }
        void SetMode(SYNCMODE mode);
        SYNCMODE GetMode() const { return _mode; }
        void ToggleMute();
        void SetVolume(int volume);
        void AdjustVolumeBy(int volume);
        int GetVolume() const;
        void AllOff();
        ListenerManager* GetListenerManager() const { return _listenerManager; }
        int GetManualOutputToLights() const { return _manualOTL; }
        void ManualOutputToLightsClick(xScheduleFrame* frame);
        bool IsScheduleActive(Schedule* schedue);
        std::list<RunningSchedule*> GetRunningSchedules() const { return _activeSchedules; }
        void OpenFPPSyncSendSocket();
        void OpenARTNetSyncSendSocket();
        void OpenOSCSyncSendSocket();
        int GetTimerAdjustment() const { return _timerAdjustment; }
        std::string GetOurIP() const;
        void SetTimerAdjustment(int timerAdjustment) { _timerAdjustment = timerAdjustment; }
        PlayList* GetPlayList(int  id) const;
        PlayList* GetBackgroundPlayList() const { return _backgroundPlayList; }
        std::list<PlayList*> GetEventPlayLists() const { return _eventPlayLists; }
        void SetBackgroundPlayList(PlayList* playlist);
        OutputManager* GetOutputManager() const { return _outputManager; }
        RunningSchedule* GetRunningSchedule() const;
        RunningSchedule* GetRunningSchedule(const std::string& schedulename) const;
        RunningSchedule* GetRunningSchedule(Schedule* schedule) const;
        ScheduleOptions* GetOptions() const { return _scheduleOptions; }
        std::list<OutputProcess*>* GetOutputProcessing() { return &_outputProcessing; }
        void WebRequestReceived() { _webRequestToggle = !_webRequestToggle; }
        std::list<PlayListItem*> GetPlayListIps() const;
        bool GetWebRequestToggle();
        bool IsDirty();
        void SetDirty();
        void ClearDirty();
        size_t GetTotalChannels() const;
        bool IsXyzzy() const { return _xyzzy != nullptr; }
        ScheduleManager(xScheduleFrame* frame, const std::string& showDir);
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
        CommandManager* GetCommandManager() { return &_commandManager; };
        bool IsRunning() const { return GetRunningPlayList() != nullptr; }
        int GetBrightness() const { return _brightness; }
        void AdjustBrightness(int by) { _brightness += by; if (_brightness < 0) _brightness = 0; else if (_brightness > 100) _brightness = 100; }
        void SetBrightness(int brightness) { if (brightness < 0) _brightness = 0; else if (brightness > 100) _brightness = 100; else _brightness = brightness; }
        int Frame(bool outputframe); // called when a frame needs to be displayed ... returns desired frame rate
        int CheckSchedule();
        std::string GetShowDir() const { return _showDir; }
        bool PlayPlayList(PlayList* playlist, size_t& rate, bool loop = false, const std::string& step = "", bool forcelast = false, int loops = -1, bool random = false, int steploops = -1);
        bool IsSomethingPlaying() const { return GetRunningPlayList() != nullptr; }
        void OptionsChanged() { _changeCount++; };
        void OutputProcessingChanged() { _changeCount++; };
        bool Action(const std::string label, PlayList* selplaylist, Schedule* selschedule, size_t& rate, std::string& msg);
        bool Action(const std::string command, const std::string parameters, const std::string& data, PlayList* selplaylist, Schedule* selschedule, size_t& rate, std::string& msg);
        bool Query(const std::string command, const std::string parameters, std::string& data, std::string& msg, const std::string& ip, const std::string& reference);
        PlayList * GetPlayList(const std::string& playlist) const;
        void StopPlayList(PlayList* playlist, bool atendofcurrentstep);
        bool StoreData(const std::string& key, const std::string& data, std::string& msg) const;
        bool RetrieveData(const std::string& key, std::string& data, std::string& msg) const;
        bool ToggleOutputToLights(xScheduleFrame* frame, std::string& msg, bool interactive);
        void SuppressVM(bool suppress);
        bool ToggleCurrentPlayListRandom(std::string& msg);
        bool ToggleCurrentPlayListPause(std::string& msg);
        bool ToggleCurrentPlayListLoop(std::string& msg);
        bool ToggleCurrentPlayListStepLoop(std::string& msg);
        bool IsOutputToLights() const;
        bool IsCurrentPlayListScheduled() const { return _immediatePlay == nullptr && GetRunningPlayList() != _queuedSongs; }
        void SetOutputToLights(xScheduleFrame* frame, bool otl, bool interactive);
        void CheckScheduleIntegrity(bool display);
        void ImportxLightsSchedule(const std::string& filename);
        bool DoXyzzy(const std::string& command, const std::string& parameters, std::string& result, const std::string& reference);
        PlayListStep* GetStepContainingPlayListItem(wxUint32 id) const;
        std::string DecodePlayList(const std::string& playlistparameter);
        std::string DecodeStep(const std::string& stepparameter);
        std::string DecodeSchedule(const std::string& scheduleparameter);
        std::string DecodeItem(const std::string& itemparameter);
        std::string DecodeButton(const std::string& buttonlabelparameter);
        static std::string xLightsShowDir();
        static std::string xScheduleShowDir();
        bool ShowDirectoriesMatch() const;
        int GetPPS() const;
        void StartListeners();
        int Sync(const std::string& filename, long ms);
};

#endif
