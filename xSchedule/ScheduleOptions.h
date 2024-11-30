#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <list>
#include <string>
#include <vector>

#include "MatrixMapper.h"
#include "SyncManager.h"
#include "VirtualMatrix.h"

class CommandManager;
class wxXmlNode;
class UserButton;
class OutputManager;
class EventBase;
class Command;

enum class OSCTIME {
    TIME_SECONDS,
    TIME_MILLISECONDS
};

enum class OSCFRAME {
    FRAME_DEFAULT,
    FRAME_24,
    FRAME_25,
    FRAME_2997,
    FRAME_30,
    FRAME_60,
    FRAME_PROGRESS
};

class ExtraIP {
    std::string _ip;
    std::string _description;
    int _changeCount;
    int _lastSavedChangeCount;

public:
    ExtraIP(const std::string& ip, const std::string& description);
    ExtraIP(wxXmlNode* node);
    void Load(wxXmlNode* node);
    virtual ~ExtraIP() {
    }
    wxXmlNode* Save() const;
    const std::string& GetDescription() const {
        return _description;
    }
    const std::string& GetIP() const {
        return _ip;
    }
    bool IsDirty() const {
        return _lastSavedChangeCount != _changeCount;
    }
    void SetDescription(const std::string description) {
        _description = description;
        _changeCount++;
    }
    void SetIP(const std::string ip) {
        _ip = ip;
        _changeCount++;
    }
    void ClearDirty() {
        _lastSavedChangeCount = _changeCount;
    }
    bool operator==(const std::string& ip) const {
        return _ip == ip;
    }
};

class OSCOptions {
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
    virtual ~OSCOptions() {
    }
    void SetMasterPath(std::string path) {
        if (path != _masterPath) {
            _masterPath = path;
            _changeCount++;
        }
    }
    void SetRemotePath(std::string path) {
        if (path != _remotePath) {
            _remotePath = path;
            _changeCount++;
        }
    }
    void SetIPAddress(std::string ip) {
        if (ip != _ipAddress) {
            _ipAddress = ip;
            _changeCount++;
        }
    }
    void SetTime() {
        if (!_time_not_frames) {
            _time_not_frames = true;
            _changeCount++;
        }
    }
    void SetFrames() {
        if (_time_not_frames) {
            _time_not_frames = false;
            _changeCount++;
        }
    }
    void SetFrame(std::string frame) {
        if (EncodeFrame(frame) != _frame) {
            _frame = EncodeFrame(frame);
            _changeCount++;
        }
    }
    void SetTime(std::string time) {
        if (EncodeTime(time) != _time) {
            _time = EncodeTime(time);
            _changeCount++;
        }
    }
    void SetServerPort(int serverPort) {
        if (serverPort != _serverport) {
            _serverport = serverPort;
            _changeCount++;
        }
    }
    void SetClientPort(int clientPort) {
        if (clientPort != _clientport) {
            _clientport = clientPort;
            _changeCount++;
        }
    }
    std::string GetMasterPath() const {
        return _masterPath;
    }
    std::string GetRemotePath() const {
        return _remotePath;
    }
    std::string GetIPAddress() const {
        return _ipAddress;
    }
    bool IsTime() const {
        return _time_not_frames;
    }
    bool IsFrames() const {
        return !_time_not_frames;
    }
    std::string GetFrame() const {
        return DecodeFrame(_frame);
    }
    std::string GetTime() const {
        return DecodeTime(_time);
    }
    OSCFRAME GetFrameCode() const {
        return _frame;
    }
    OSCTIME GetTimeCode() const {
        return _time;
    }
    int GetServerPort() const {
        return _serverport;
    }
    int GetClientPort() const {
        return _clientport;
    }
    bool IsDirty() const {
        return _changeCount != _lastSavedChangeCount;
    }
    void ClearDirty() {
        _lastSavedChangeCount = _changeCount;
    }
    wxXmlNode* Save();
    bool IsBroadcast() const {
        return _ipAddress == "255.255.255.255";
    }
    void SetBroadcast() {
        SetIPAddress("255.255.255.255");
    }
};

typedef enum {
    TEST_ALTERNATE,
    TEST_LEVEL1,
    TEST_A_B_C,
    TEST_A_B_C_ALL,
    TEST_A_B_C_ALL_NONE,
    TEST_A,
    TEST_B,
    TEST_C
} TESTMODE;

class TestOptions {
    TESTMODE _mode;
    uint8_t _level1;
    uint8_t _level2;
    int _interval;
    int _changeCount;
    int _lastSavedChangeCount;
    long _start;
    long _end;

    static std::string DecodeMode(TESTMODE mode);
    TESTMODE EncodeMode(std::string mode) const;
    void Load(wxXmlNode* node);

public:
    TestOptions();
    TestOptions(wxXmlNode* node);
    virtual ~TestOptions() {
    }
    void SetMode(std::string mode) {
        if (EncodeMode(mode) != _mode) {
            _mode = EncodeMode(mode);
            _changeCount++;
        }
    }
    void SetLevel1(uint8_t level1) {
        if (level1 != _level1) {
            _level1 = level1;
            _changeCount++;
        }
    }
    void SetLevel2(uint8_t level2) {
        if (level2 != _level2) {
            _level2 = level2;
            _changeCount++;
        }
    }
    void SetInterval(int interval) {
        if (interval != _interval) {
            _interval = interval;
            _changeCount++;
        }
    }
    void SetBounds(long start, long end) {
        _start = start;
        _end = end;
    }
    void ClearBounds() {
        _start = -1;
        _end = -1;
    }
    std::string GetMode() const {
        return DecodeMode(_mode);
    }
    TESTMODE GetModeCode() const {
        return _mode;
    }
    uint8_t GetLevel1() const {
        return _level1;
    }
    uint8_t GetLevel2() const {
        return _level2;
    }
    void GetBounds(long& start, long& end) const {
        start = _start;
        end = _end;
    }
    bool HasBounds() const {
        return _start != -1 && _end != -1;
    }
    int GetInterval() const {
        return _interval;
    }
    bool IsDirty() const {
        return _changeCount != _lastSavedChangeCount;
    }
    void ClearDirty() {
        _lastSavedChangeCount = _changeCount;
    }
    wxXmlNode* Save();
};

class ScheduleOptions {
    bool _advancedMode = false;
    bool _sync = false;
    int _changeCount = 0;
    int _lastSavedChangeCount = 0;
    bool _sendOffWhenNotRunning = false;
    bool _sendBackgroundWhenNotRunning = false;
    bool _webAPIOnly = false;
    int _port = 80;
    int _remoteLatency = 0;
    int _remoteAcceptableJitter = 20;
    std::string _wwwRoot;
    std::string _password;
    std::string _defaultPage = "index.html";
    bool _allowUnauth = false;
    std::string _crashBehaviour;
    int _passwordTimeout = 0;
    std::vector<UserButton*> _buttons;
    std::list<MatrixMapper*> _matrices;
    std::list<VirtualMatrix*> _virtualMatrices;
    std::list<std::string> _fppRemotes;
    std::string _audioDevice;
    std::string _inputAudioDevice;
    OSCOptions* _oscOptions;
    TestOptions* _testOptions;
    std::list<EventBase*> _events;
    TIMECODEFORMAT _artNetTimeCodeFormat;
    std::string _city;
    std::string _MIDITimecodeDevice;
    TIMECODEFORMAT _MIDITimecodeFormat;
    size_t _MIDITimecodeOffset = 0;
    std::list<ExtraIP*> _extraIPs;
    wxSize _defaultVideoSize = { 300, 300 };
    wxPoint _defaultVideoPos = { 0, 0 };
    bool _parallelTransmission;
    bool _remoteAllOff;
    bool _keepScreenOn;
    bool _retryOutputOpen;
    bool _suppressAudioOnRemotes;
    bool _hardwareAcceleratedVideo = false;
    bool _lateStartingScheduleUsesTime = false;
    bool _disableOutputOnPingFailure = false;
    int _SMPTEMode = 3;
    std::string _SMPTEDevice = "";
    bool _minimiseUIUpdates = false;
    bool _useStepMMSSTimecodeFormat = false;
    bool _remoteTimecodeStepAdvance = false;

    std::pair<int, int> ParsePair(const std::string& value, const std::pair<int, int>& def);
    std::string SerialisePair(int a, int b);

public:
    static int EncodeSMPTEMode(const std::string& mode);
    static wxArrayString GetSPMTEModes();
    static std::string DecodeSMPTEMode(int mode);

    bool IsDirty() const;
    void ClearDirty();
    ScheduleOptions(OutputManager* outputManager, wxXmlNode* node, CommandManager* commandManager);
    ScheduleOptions();
    virtual ~ScheduleOptions();
    wxXmlNode* Save();
    UserButton* GetButton(const std::string& label) const;
    UserButton* GetButton(wxUint32 id) const;
    bool IsSync() const {
        return _sync;
    }
    bool IsAdvancedMode() const {
        return _advancedMode;
    }
    wxSize GetDefaultVideoSize() const {
        return _defaultVideoSize;
    }
    wxPoint GetDefaultVideoPos() const {
        return _defaultVideoPos;
    }
    std::list<ExtraIP*>* GetExtraIPs() {
        return &_extraIPs;
    }
    std::list<MatrixMapper*>* GetMatrices() {
        return &_matrices;
    }
    std::list<VirtualMatrix*>* GetVirtualMatrices() {
        return &_virtualMatrices;
    }
    std::list<EventBase*>* GetEvents() {
        return &_events;
    }
    std::string GetMIDITimecodeDevice() const {
        return _MIDITimecodeDevice;
    }
    std::string GetOurURL() const;
    void SetMIDITimecodeDevice(std::string midi) {
        if (midi != _MIDITimecodeDevice) {
            _MIDITimecodeDevice = midi;
            _changeCount++;
        }
    }
    TIMECODEFORMAT GetMIDITimecodeFormat() const {
        return _MIDITimecodeFormat;
    }
    size_t GetMIDITimecodeOffset() const {
        return _MIDITimecodeOffset;
    }
    void SetMIDITimecodeFormat(TIMECODEFORMAT format) {
        if (format != _MIDITimecodeFormat) {
            _MIDITimecodeFormat = format;
            _changeCount++;
        }
    }
    void SetMIDITimecodeOffset(size_t offset) {
        if (offset != _MIDITimecodeOffset) {
            _MIDITimecodeOffset = offset;
            _changeCount++;
        }
    }
    void SetAdvancedMode(bool advancedMode) {
        if (_advancedMode != advancedMode) {
            _advancedMode = advancedMode;
            _changeCount++;
        }
    }
    void SetParallelTransmission(bool parallel) {
        if (_parallelTransmission != parallel) {
            _parallelTransmission = parallel;
            _changeCount++;
        }
    }
    void SetRemoteAllOff(bool remoteAllOff) {
        if (_remoteAllOff != remoteAllOff) {
            _remoteAllOff = remoteAllOff;
            _changeCount++;
        }
    }
    void SetMinimiseUIUpdates(bool minimiseUIUpdates) {
        if (_minimiseUIUpdates != minimiseUIUpdates) {
            _minimiseUIUpdates = minimiseUIUpdates;
            _changeCount++;
        }
    }
    void SetKeepScreenOn(bool keepScreenOn) {
        if (_keepScreenOn != keepScreenOn) {
            _keepScreenOn = keepScreenOn;
            _changeCount++;
        }
    }
    void SetRetryOutputOpen(bool retryOpen) {
        if (_retryOutputOpen != retryOpen) {
            _retryOutputOpen = retryOpen;
            _changeCount++;
        }
    }
    void SetSMPTEMode(int mode) {
        if (_SMPTEMode != mode) {
            _SMPTEMode = mode;
            _changeCount++;
        }
    }
    void SetSMPTEDevice(const std::string& device) {
        auto d = device;
        if (device == "(Default)")
            d = "";
        if (_SMPTEDevice != d) {
            _SMPTEDevice = d;
            _changeCount++;
        }
    }
    void SetStepMMSSTimecodeFormat(bool useStepMMSS) {
        if (_useStepMMSSTimecodeFormat != useStepMMSS) {
            _useStepMMSSTimecodeFormat = useStepMMSS;
            _changeCount++;
        }
    }
    void SetRemoteTimecodeStepAdvance(bool remoteTimecodeStepAdvance) {
        if (_remoteTimecodeStepAdvance != remoteTimecodeStepAdvance) {
            _remoteTimecodeStepAdvance = remoteTimecodeStepAdvance;
            _changeCount++;
        }
    }
    void SetSuppressAudioOnRemotes(bool suppressAudio) {
        if (_suppressAudioOnRemotes != suppressAudio) {
            _suppressAudioOnRemotes = suppressAudio;
            _changeCount++;
        }
    }
    void SetSync(bool sync) {
        if (_sync != sync) {
            _sync = sync;
            _changeCount++;
        }
    }
    void SetSendOffWhenNotRunning(bool send) {
        if (_sendOffWhenNotRunning != send) {
            _sendOffWhenNotRunning = send;
            _changeCount++;
        }
    }
    bool IsSendOffWhenNotRunning() const {
        return _sendOffWhenNotRunning;
    }
    bool IsParallelTransmission() const {
        return _parallelTransmission;
    }
    bool IsRemoteAllOff() const {
        return _remoteAllOff;
    }
    bool IsKeepScreenOn() const {
        return _keepScreenOn;
    }
    bool IsMinimiseUIUpdates() const {
        return _minimiseUIUpdates;
    }
    bool IsRetryOpen() const {
        return _retryOutputOpen;
    }
    bool IsUseStepMMSSTimecodeFormat() const {
        return _useStepMMSSTimecodeFormat;
    }
    bool IsRemoteTimecodeStepAdvance() const {
        return _remoteTimecodeStepAdvance;
    }
    int GetSMPTEMode() const {
        return _SMPTEMode;
    }
    std::string GetSMPTEDevice() const {
        return _SMPTEDevice;
    }
    bool IsSuppressAudioOnRemotes() const {
        return _suppressAudioOnRemotes;
    }
    void SetSendBackgroundWhenNotRunning(bool send) {
        if (_sendBackgroundWhenNotRunning != send) {
            _sendBackgroundWhenNotRunning = send;
            _changeCount++;
        }
    }
    bool IsSendBackgroundWhenNotRunning() const {
        return _sendBackgroundWhenNotRunning;
    }
    void SetHardwareAcceleratedVideo(bool hardwareAcceleratedVideo) {
        if (_hardwareAcceleratedVideo != hardwareAcceleratedVideo) {
            _hardwareAcceleratedVideo = hardwareAcceleratedVideo;
            _changeCount++;
        }
    }
    bool IsHardwareAcceleratedVideo() const {
        return _hardwareAcceleratedVideo;
    }
    void SetLateStartingScheduleUsesTime(bool lateStartingScheduleUsesTime) {
        if (_lateStartingScheduleUsesTime != lateStartingScheduleUsesTime) {
            _lateStartingScheduleUsesTime = lateStartingScheduleUsesTime;
            _changeCount++;
        }
    }
    bool IsLateStartingScheduleUsesTime() const {
        return _lateStartingScheduleUsesTime;
    }
    void SetDisableOutputOnPingFailure(bool disableOutputOnPingFailure) {
        if (_disableOutputOnPingFailure != disableOutputOnPingFailure) {
            _disableOutputOnPingFailure = disableOutputOnPingFailure;
            _changeCount++;
        }
    }
    bool IsDisableOutputOnPingFailure() const {
        return _disableOutputOnPingFailure;
    }
    void SetArtNetTimeCodeFormat(TIMECODEFORMAT artNetTimeCodeFormat) {
        if (artNetTimeCodeFormat != _artNetTimeCodeFormat) {
            _artNetTimeCodeFormat = artNetTimeCodeFormat;
            _changeCount++;
        }
    }
    void SetDefaultVideoSize(const wxSize& size) {
        if (size != _defaultVideoSize) {
            _defaultVideoSize = size;
            _changeCount++;
        }
    }
    void SetDefaultVideoPos(const wxPoint& pos) {
        if (pos != _defaultVideoPos) {
            _defaultVideoPos = pos;
            _changeCount++;
        }
    }
    TIMECODEFORMAT GetARTNetTimeCodeFormat() const {
        return _artNetTimeCodeFormat;
    }
    std::string GetCrashBehaviour() const {
        return _crashBehaviour;
    }
    void SetCrashBehaviour(std::string crashBehaviour) {
        if (crashBehaviour != _crashBehaviour) {
            _crashBehaviour = crashBehaviour;
            _changeCount++;
        }
    }
    std::vector<UserButton*> GetButtons() const;
    void ClearButtons();
    std::string GetButtonsJSON(const CommandManager& cmdMgr, const std::string& reference) const;
    int GetWebServerPort() const {
        return _port;
    }
    std::list<std::string> GetFPPRemotes() const {
        return _fppRemotes;
    }
    void SetWebServerPort(int port) {
        if (_port != port) {
            _port = port;
            _changeCount++;
        }
    }
    void SetFPPRemotes(std::list<std::string> remotes) {
        _fppRemotes = remotes;
        _changeCount++;
    }
    std::string GetWWWRoot() const {
        return _wwwRoot;
    }
    std::string GetAudioDevice() const {
        return _audioDevice;
    }
    std::string GetInputAudioDevice() const {
        return _inputAudioDevice;
    }
    std::string GetDefaultRoot() const;
    void SetWWWRoot(const std::string& wwwRoot) {
        if (_wwwRoot != wwwRoot) {
            _wwwRoot = wwwRoot;
            _changeCount++;
        }
    }
    void SetDirty() {
        _changeCount++;
    }
    void SetAudioDevice(const std::string& audioDevice);
    void SetInputAudioDevice(const std::string& inputAudioDevice);
    void AddButton(const std::string& label, const std::string& command, const std::string& parms, char hotkey, const std::string& color, CommandManager* commandManager);
    bool GetAPIOnly() const {
        return _webAPIOnly;
    }
    int GetRemoteLatency() const {
        return _remoteLatency;
    }
    int GetRemoteAcceptableJitter() const {
        return _remoteAcceptableJitter;
    }
    std::string GetPassword() const {
        return _password;
    }
    std::string GetDefaultPage() const {
        return _defaultPage;
    }
    bool GetAllowUnauth() const {
        return _allowUnauth;
    }
    std::string GetCity() const {
        return _city;
    }
    int GetPasswordTimeout() const {
        return _passwordTimeout;
    }
    void SetAPIOnly(bool apiOnly) {
        if (_webAPIOnly != apiOnly) {
            _webAPIOnly = apiOnly;
            _changeCount++;
        }
    }
    void SetRemoteLatency(int remoteLatency) {
        if (remoteLatency != _remoteLatency) {
            _remoteLatency = remoteLatency;
            _changeCount++;
        }
    }
    void SetRemoteAcceptableJitter(int remoteAcceptableJitter) {
        if (remoteAcceptableJitter != _remoteAcceptableJitter) {
            _remoteAcceptableJitter = remoteAcceptableJitter;
            _changeCount++;
        }
    }
    void SetPasswordTimeout(int passwordTimeout) {
        if (_passwordTimeout != passwordTimeout) {
            _passwordTimeout = passwordTimeout;
            _changeCount++;
        }
    }
    void SetPassword(const std::string& password) {
        if (_password != password) {
            _password = password;
            _changeCount++;
        }
    }
    void SetDefaultPage(const std::string& defaultPage) {
        if (_defaultPage != defaultPage) {
            _defaultPage = defaultPage;
            _changeCount++;
        }
    }
    void SetAllowUnauth(bool allowUnauth) {
        if (_allowUnauth != allowUnauth) {
            _allowUnauth = allowUnauth;
            _changeCount++;
        }
    }
    void SetCity(const std::string& city) {
        if (_city != city) {
            _city = city;
            _changeCount++;
        }
    }
    OSCOptions* GetOSCOptions() const {
        return _oscOptions;
    }
    TestOptions* GetTestOptions() const {
        return _testOptions;
    }
};
