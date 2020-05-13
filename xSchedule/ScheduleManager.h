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
#include <string>
#include <wx/wx.h>
#include "Schedule.h"
#include "CommandManager.h"
#include <wx/socket.h>
#include <wx/thread.h>
#include "wxMIDI/src/wxMidi.h"
#include "Blend.h"
#include "SyncManager.h"

class PlayListItemText;
class ScheduleOptions;
class PlayList;
class OutputManager;
class RunningSchedule;
class PlayListStep;
class OutputProcess;
class XyzzyBase;
class PlayListItem;
class xScheduleFrame;
class Pinger;
class ListenerManager;

class PixelData
{
    size_t _startChannel;
    size_t _size;
    uint8_t* _data;
    APPLYMETHOD _blendMode;

    void ExtractData(const wxString& data);

public:
    PixelData(size_t startChannel, const wxString& data, APPLYMETHOD blendMode);
    PixelData(size_t startChannel, size_t channels, const wxColor& c, APPLYMETHOD blendMode);
    virtual ~PixelData();
    void Set(uint8_t* buffer, size_t size);
    void SetColor(const wxColor& c, APPLYMETHOD blendMode);
    void SetData(const wxString& data, APPLYMETHOD blendMode);
    long GetSize() const { return _size; }
    size_t GetStartChannel() const { return _startChannel; }
};

class ActionMessageData
{
public:
    ActionMessageData(wxString command, wxString parameters, wxString data)
    {
        _command = command;
        _parameters = parameters;
        _data = data;
    }
    std::string _command;
    std::string _parameters;
    std::string _data;
};

class ScheduleManager
{
    int _mode = (int)SYNCMODE::STANDALONE;
    REMOTEMODE _remoteMode = REMOTEMODE::DISABLED;
    bool _testMode = false;
    int _manualOTL = 0;
    std::string _showDir;
    int _lastSavedChangeCount = 0;
    int _changeCount = 0;
	std::list<PlayList*> _playLists;
    ScheduleOptions* _scheduleOptions;
    OutputManager* _outputManager;
    uint8_t* _buffer = nullptr;
    wxUint32 _startTime = 0;
    PlayList* _immediatePlay = nullptr;
    PlayList* _backgroundPlayList = nullptr;
    std::list<PlayList*> _eventPlayLists;
    int _overrideMS = 0;
    std::list<PixelData*> _overlayData;
    CommandManager _commandManager;
    PlayList* _queuedSongs = nullptr;
    std::list<RunningSchedule*> _activeSchedules;
    wxThreadIdType _mainThread;
    int _brightness = 0;
    int _lastBrightness = 0;
    uint8_t _brightnessArray[256];
    wxMidiOutDevice* _midiMaster = nullptr;
    wxDatagramSocket* _fppSyncMaster = nullptr;
    wxDatagramSocket* _artNetSyncMaster = nullptr;
    wxDatagramSocket* _fppSyncMasterUnicast = nullptr;
    std::list<OutputProcess*> _outputProcessing;
    ListenerManager* _listenerManager = nullptr;
    XyzzyBase* _xyzzy = nullptr;
    wxDateTime _lastXyzzyCommand;
    int _timerAdjustment = 0;
    bool _webRequestToggle = false;
    Pinger* _pinger = nullptr;
    std::unique_ptr<SyncManager> _syncManager = nullptr;

    void DisableRemoteOutputs();
    std::string GetPingStatus();
    std::string FormatTime(size_t timems);
    void CreateBrightnessArray();
    void ManageBackground();
    bool DoText(PlayListItemText* pliText, const wxString& text, const wxString& properties);
    void StartVirtualMatrices();
    void StopVirtualMatrices();
    void StartStep(const std::string stepName);
    void StartTiming(const std::string timgingName);
    PlayListItem* FindRunProcessNamed(const std::string& item) const;
    void TestFrame(uint8_t* buffer, long totalChannels, long msec);

    public:

        void SetPinger(Pinger* pinger) { _pinger = pinger; }
        void SetMode(int mode, REMOTEMODE remote);
        void GetMode(int& mode, REMOTEMODE& remote) const { mode = _mode; remote = _remoteMode; }
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
        const SyncManager* GetSyncManager() const { return _syncManager.get(); }
        int GetTimerAdjustment() const { return _timerAdjustment; }
        std::string GetOurIP() const;
        void SetTimerAdjustment(int timerAdjustment) { _timerAdjustment = timerAdjustment; }
        PlayList* GetPlayList(int  id) const;
        PlayList* GetBackgroundPlayList() const { return _backgroundPlayList; }
        std::list<PlayList*> GetEventPlayLists() const { return _eventPlayLists; }
        void SetBackgroundPlayList(PlayList* playlist);
        OutputManager* GetOutputManager() const { return _outputManager; }
        int GetNonStoppedCount() const;
        void GetNextScheduledPlayList(PlayList** p, Schedule** s);
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
        void StopAll(bool sustain = false);
        void AddPlayList(PlayList* playlist);
        bool IsQueuedPlaylistRunning() const;
        void RemovePlayList(PlayList* playlist);
        PlayList* GetRunningPlayList() const;
        PlayList* GetRunningPlayList(int id) const;
        std::list<PlayList*> GetPlayLists();
        std::list<Command*> GetCommands() const { return _commandManager.GetCommands(); }
        Command* GetCommand(std::string command) const { return _commandManager.GetCommand(command); }
        CommandManager* GetCommandManager() { return &_commandManager; };
        bool IsRunning() const { return GetRunningPlayList() != nullptr; }
        int GetBrightness() const { return _brightness; }
        void AdjustBrightness(int by) { _brightness += by; if (_brightness < 0) _brightness = 0; else if (_brightness > 100) _brightness = 100; }
        void SetBrightness(int brightness) { if (brightness < 0) _brightness = 0; else if (brightness > 100) _brightness = 100; else _brightness = brightness; }
        int Frame(bool outputframe, xScheduleFrame* frame); // called when a frame needs to be displayed ... returns desired frame rate
        int CheckSchedule();
        std::string GetShowDir() const { return _showDir; }
        bool PlayPlayList(PlayList* playlist, size_t& rate, bool loop = false, const std::string& step = "", bool forcelast = false, int loops = -1, bool random = false, int steploops = -1);
        bool IsSomethingPlaying() const { return GetRunningPlayList() != nullptr; }
        void OptionsChanged() { _changeCount++; };
        void OutputProcessingChanged() { _changeCount++; };
        bool Action(const wxString& label, PlayList* selplaylist, PlayListStep* selplayliststep, Schedule* selschedule, size_t& rate, wxString& msg);
        bool Action(const wxString& command, const wxString& parameters, const wxString& data, PlayList* selplaylist, PlayListStep* selplayliststep, Schedule* selschedule, size_t& rate, wxString& msg);
        bool Query(const wxString& command, const wxString& parameters, wxString& data, wxString& msg, const wxString& ip, const wxString& reference);
        bool IsQuery(const wxString& command);
        PlayList * GetPlayList(const std::string& playlist) const;
        void StopPlayList(PlayList* playlist, bool atendofcurrentstep, bool sustain = false);
        bool StoreData(const wxString& key, const wxString& data, wxString& msg) const;
        bool RetrieveData(const wxString& key, wxString& data, wxString& msg) const;
        bool ToggleOutputToLights(xScheduleFrame* frame, wxString& msg, bool interactive);
        void SuppressVM(bool suppress);
        bool ToggleCurrentPlayListRandom(wxString& msg);
        bool ToggleCurrentPlayListPause(wxString& msg);
        bool ToggleCurrentPlayListLoop(wxString& msg);
        bool ToggleCurrentPlayListStepLoop(wxString& msg);
        bool IsOutputToLights() const;
        bool IsCurrentPlayListScheduled() const { return _immediatePlay == nullptr && GetRunningPlayList() != _queuedSongs; }
        void SetOutputToLights(xScheduleFrame* frame, bool otl, bool interactive);
        void CheckScheduleIntegrity(bool display);
        void ImportxLightsSchedule(const std::string& filename);
        bool DoXyzzy(const wxString& command, const wxString& parameters, wxString& result, const wxString& reference);
        PlayListStep* GetStepContainingPlayListItem(wxUint32 id) const;
        std::string FindStepForFSEQ(const std::string& fseq) const;
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
        int DoSync(const std::string& filename, long ms);
        bool IsSlave() const;
        bool IsFPPRemoteOrMaster() const;
        bool IsTest() const;
        void SetTestMode(bool test) { _testMode = test; if (!test) AllOff(); }
};
