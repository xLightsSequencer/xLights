#ifndef SCHEDULEOPTIONS_H
#define SCHEDULEOPTIONS_H
#include <list>
#include <string>
#include <vector>
#include "MatrixMapper.h"
#include "VirtualMatrix.h"

class CommandManager;
class wxXmlNode;
class UserButton;
class OutputManager;
class EventBase;
class Command;

typedef enum
{
    TIME_SECONDS,
    TIME_MILLISECONDS
} OSCTIME;

typedef enum
{
    FRAME_DEFAULT,
    FRAME_24,
    FRAME_25,
    FRAME_2997,
    FRAME_30,
    FRAME_60,
    FRAME_PROGRESS
} OSCFRAME;

class ExtraIP
{
    std::string _ip;
    std::string _description;
    int _changeCount;
    int _lastSavedChangeCount;

    public:
    ExtraIP(const std::string& ip, const std::string& description);
    ExtraIP(wxXmlNode* node);
    void Load(wxXmlNode* node);
    virtual ~ExtraIP() {}
    wxXmlNode* Save() const;
    const std::string& GetDescription() const { return _description; }
    const std::string& GetIP() const { return _ip; }
    bool IsDirty() const { return _lastSavedChangeCount != _changeCount; }
    void SetDescription(const std::string description) { _description = description; _changeCount++; }
    void SetIP(const std::string ip) { _ip = ip; _changeCount++; }
    void ClearDirty() { _lastSavedChangeCount = _changeCount; }
    bool operator==(const std::string&ip) const { return _ip == ip; }
};

class OSCOptions
{
    std::string _masterPath;
    std::string _remotePath;
    bool _time_not_frames;
    OSCFRAME _frame;
    OSCTIME _time;
    int _changeCount;
    int _lastSavedChangeCount;
    int _serverport;
    int _clientport;
    std::string _ipAddress;

    static std::string DecodeFrame(OSCFRAME frame);
    static std::string DecodeTime(OSCTIME time);
    OSCFRAME EncodeFrame(std::string frame) const;
    OSCTIME EncodeTime(std::string time) const;
    void Load(wxXmlNode* node);

public:
    OSCOptions();
    OSCOptions(wxXmlNode* node);
    virtual ~OSCOptions() {}
    void SetMasterPath(std::string path) { if (path != _masterPath) { _masterPath = path; _changeCount++; } }
    void SetRemotePath(std::string path) { if (path != _remotePath) { _remotePath = path; _changeCount++; } }
    void SetIPAddress(std::string ip) { if (ip != _ipAddress) { _ipAddress = ip; _changeCount++; } }
    void SetTime() { if (!_time_not_frames) { _time_not_frames = true; _changeCount++; } }
    void SetFrames() { if (_time_not_frames) { _time_not_frames = false; _changeCount++; } }
    void SetFrame(std::string frame) { if (EncodeFrame(frame) != _frame) { _frame = EncodeFrame(frame); _changeCount++; } }
    void SetTime(std::string time) { if (EncodeTime(time) != _time) { _time = EncodeTime(time); _changeCount++; } }
    void SetServerPort(int serverPort) { if (serverPort != _serverport) { _serverport = serverPort; _changeCount++; } }
    void SetClientPort(int clientPort) { if (clientPort != _clientport) { _clientport = clientPort; _changeCount++; } }
    std::string GetMasterPath() const { return _masterPath; }
    std::string GetRemotePath() const { return _remotePath; }
    std::string GetIPAddress() const { return _ipAddress; }
    bool IsTime() const { return _time_not_frames; }
    bool IsFrames() const { return !_time_not_frames; }
    std::string GetFrame() const { return DecodeFrame(_frame); }
    std::string GetTime() const { return DecodeTime(_time); }
    OSCFRAME GetFrameCode() const { return _frame; }
    OSCTIME GetTimeCode() const { return _time; }
    int GetServerPort() const { return _serverport; }
    int GetClientPort() const { return _clientport; }
    bool IsDirty() const { return _changeCount != _lastSavedChangeCount; }
    void ClearDirty() { _lastSavedChangeCount = _changeCount; }
    wxXmlNode* Save();
    bool IsBroadcast() const { return _ipAddress == "255.255.255.255"; }
    void SetBroadcast() { SetIPAddress("255.255.255.255"); }
};

class ScheduleOptions
{
    bool _advancedMode;
	bool _sync;
    int _changeCount;
    int _lastSavedChangeCount;
    bool _sendOffWhenNotRunning;
    bool _sendBackgroundWhenNotRunning;
    bool _webAPIOnly;
    int _port;
    std::string _wwwRoot;
    std::string _password;
    std::string _crashBehaviour;
    int _passwordTimeout;
    std::vector<UserButton*> _buttons;
    std::list<MatrixMapper*> _matrices;
    std::list<VirtualMatrix*> _virtualMatrices;
    std::list<std::string> _fppRemotes;
    std::string _audioDevice;
    OSCOptions* _oscOptions;
    std::list<EventBase*> _events;
    int _artNetTimeCodeFormat;
    std::string _city;
    std::string _MIDITimecodeDevice;
    int _MIDITimecodeFormat;
    size_t _MIDITimecodeOffset;
    std::list<ExtraIP*> _extraIPs;
    bool _parallelTransmission;
    bool _remoteAllOff;
    bool _retryOutputOpen;

    public:

        bool IsDirty() const;
        void ClearDirty();
        ScheduleOptions(OutputManager* outputManager, wxXmlNode* node, CommandManager* commandManager);
        ScheduleOptions();
        virtual ~ScheduleOptions();
        wxXmlNode* Save();
        UserButton* GetButton(const std::string& label) const;
        UserButton* GetButton(wxUint32 id) const;
        bool IsSync() const { return _sync; }
        bool IsAdvancedMode() const { return _advancedMode; }
        std::list<ExtraIP*>* GetExtraIPs() { return &_extraIPs; }
        std::list<MatrixMapper*>* GetMatrices() { return &_matrices; }
        std::list<VirtualMatrix*>* GetVirtualMatrices() { return &_virtualMatrices; }
        std::list<EventBase*>* GetEvents() { return &_events; }
        std::string GetMIDITimecodeDevice() const { return _MIDITimecodeDevice; }
        void SetMIDITimecodeDevice(std::string midi) { if (midi != _MIDITimecodeDevice) { _MIDITimecodeDevice = midi; _changeCount++; } }
        int GetMIDITimecodeFormat() const { return _MIDITimecodeFormat; }
        size_t GetMIDITimecodeOffset() const { return _MIDITimecodeOffset; }
        void SetMIDITimecodeFormat(int format) { if (format != _MIDITimecodeFormat) { _MIDITimecodeFormat = format; _changeCount++; } }
        void SetMIDITimecodeOffset(size_t offset) { if (offset != _MIDITimecodeOffset) { _MIDITimecodeOffset = offset; _changeCount++; } }
        void SetAdvancedMode(bool advancedMode) { if (_advancedMode != advancedMode) { _advancedMode = advancedMode; _changeCount++; } }
        void SetParallelTransmission(bool parallel) { if (_parallelTransmission != parallel) { _parallelTransmission = parallel; _changeCount++; } }
        void SetRemoteAllOff(bool remoteAllOff) { if (_remoteAllOff != remoteAllOff) { _remoteAllOff = remoteAllOff; _changeCount++; } }
        void SetRetryOutputOpen(bool retryOpen) { if (_retryOutputOpen != retryOpen) { _retryOutputOpen = retryOpen; _changeCount++; } }
        void SetSync(bool sync) { if (_sync != sync) { _sync = sync; _changeCount++; } }
        void SetSendOffWhenNotRunning(bool send) { if (_sendOffWhenNotRunning != send) { _sendOffWhenNotRunning = send; _changeCount++; } }
        bool IsSendOffWhenNotRunning() const { return _sendOffWhenNotRunning; }
        bool IsParallelTransmission() const { return _parallelTransmission; }
        bool IsRemoteAllOff() const { return _remoteAllOff; }
        bool IsRetryOpen() const { return _retryOutputOpen; }
        void SetSendBackgroundWhenNotRunning(bool send) { if (_sendBackgroundWhenNotRunning != send) { _sendBackgroundWhenNotRunning = send; _changeCount++; } }
        bool IsSendBackgroundWhenNotRunning() const { return _sendBackgroundWhenNotRunning; }
        void SetArtNetTimeCodeFormat(int artNetTimeCodeFormat) { if (artNetTimeCodeFormat != _artNetTimeCodeFormat) { _artNetTimeCodeFormat = artNetTimeCodeFormat; _changeCount++; } }
        int GetARTNetTimeCodeFormat() const { return _artNetTimeCodeFormat; }
        std::string GetCrashBehaviour() const { return _crashBehaviour; }
        void SetCrashBehaviour(std::string crashBehaviour) { if (crashBehaviour != _crashBehaviour) { _crashBehaviour = crashBehaviour; _changeCount++; } }
        std::vector<UserButton*> GetButtons() const;
        void ClearButtons();
        std::string GetButtonsJSON(const CommandManager &cmdMgr, const std::string& reference) const;
        int GetWebServerPort() const { return _port; }
        std::list<std::string> GetFPPRemotes() const { return _fppRemotes; }
        void SetWebServerPort(int port) { if (_port != port) { _port = port; _changeCount++; } }
        void SetFPPRemotes(std::list<std::string> remotes) { _fppRemotes = remotes; _changeCount++; } 
        std::string GetWWWRoot() const { return _wwwRoot; }
        std::string GetAudioDevice() const { return _audioDevice; }
        std::string GetDefaultRoot() const;
        void SetWWWRoot(const std::string& wwwRoot) { if (_wwwRoot != wwwRoot) { _wwwRoot = wwwRoot; _changeCount++; } }
        void SetDirty() { _changeCount++; }
        void SetAudioDevice(const std::string& audioDevice);
        void AddButton(const std::string& label, const std::string& command, const std::string& parms, char hotkey, const std::string& color, CommandManager* commandManager);
        bool GetAPIOnly() const { return _webAPIOnly; }
        std::string GetPassword() const { return _password; }
        std::string GetCity() const { return _city; }
        int GetPasswordTimeout() const { return _passwordTimeout; }
        void SetAPIOnly(bool apiOnly) { if (_webAPIOnly != apiOnly) { _webAPIOnly = apiOnly; _changeCount++; } }
        void SetPasswordTimeout(int passwordTimeout) { if (_passwordTimeout != passwordTimeout) { _passwordTimeout = passwordTimeout; _changeCount++; } }
        void SetPassword(const std::string& password) { if (_password != password) { _password = password; _changeCount++; } }
        void SetCity(const std::string& city) { if (_city != city) { _city = city; _changeCount++; } }
        OSCOptions* GetOSCOptions() const { return _oscOptions; }
};

#endif
