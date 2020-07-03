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

#include <wx/thread.h>

#include <list>
#include <string>
#include <map>

class wxWindow;
class wxXmlNode;

class Output;
class Controller;
class TestPreset;
class Controller;
class ControllerEthernet;

#define NETWORKSFILE "xlights_networks.xml";

class OutputManager
{
    #pragma region Member Variables
    std::string _filename;
    std::list<Controller*> _controllers;
    std::list<std::pair<Output*, Controller*>> _conversionOutputs;
    std::list<TestPreset*> _testPresets;
    int _syncUniverse = 0;
    bool _syncEnabled = false;
    bool _dirty = false;
    int _suppressFrames = 0;
    bool _parallelTransmission = false;
    bool _outputting = false; // true if we are currently sending out data
    bool _didConvert = false;
    std::string _globalFPPProxy;
    wxCriticalSection _outputCriticalSection; // used to protect areas that must be single threaded
    #pragma endregion 

    #pragma region Static Variables
    static bool __isSync; // copied here so it can be accessed statically
    static int _lastSecond;
    static int _currentSecond;
    static int _lastSecondCount;
    static int _currentSecondCount;
    static bool _isRetryOpen;
    static bool _isInteractive;
    #pragma endregion 

    #pragma region Private Functions
    bool SetGlobalOutputtingFlag(bool state, bool force = false);
    bool ConvertStartChannel(const std::string sc, std::string& newsc) const;
    void AsyncPingAll();
    #pragma endregion 

public:

    #pragma region Constructors and Destructors
    OutputManager();
    ~OutputManager();
    #pragma endregion 

    #pragma region Save and Load
    bool Load(const std::string& showdir, bool syncEnabled = false);
    bool Save();
    bool DidConvert() const { return _didConvert; }
    bool ConvertModelStartChannels(wxXmlNode* modelsNode) const;
    #pragma endregion 

    #pragma region Static Functions
    static std::string GetNetworksFileName() { return NETWORKSFILE; }
    static void RegisterSentPacket();
    static bool IsRetryOpen() { return _isRetryOpen; }
    static void SetRetryOpen(bool retryOpen) { _isRetryOpen = retryOpen; }
    static bool IsInteractive() { return _isInteractive; }
    static void SetInteractive(bool interactive) { _isInteractive = interactive; }
    static std::string GetExportHeader();
    #pragma endregion 

    #pragma region Controller Management
    std::list<Controller*> GetControllers() const { return _controllers; }
    std::list<ControllerEthernet*> GetControllers(const std::string& ip);
    std::list<ControllerEthernet*> GetControllers(const std::string& ip, const std::string hostname);
    void AddController(Controller* controller, int pos = -1);
    void DeleteController(const std::string& controllerName);
    void DeleteAllControllers();
    void MoveController(Controller* controller, int toControllerNumber);
    Controller* GetController(const std::string& name) const;
    Controller* GetController(int32_t absoluteChannel, int32_t& startChannel) const; // returns the controller - equivalent to the old level 1
    Controller* GetControllerWithIP(const std::string& ip);
    Controller* GetControllerIndex(int index) const;
    int GetControllerIndex(Controller* c);
    int GetControllerCount() const { return _controllers.size(); }
    int GetControllerCount(const std::string& type, const std::string& ip) const;
    std::list<std::string> GetControllerNames() const;
    std::list<std::string> GetAutoLayoutControllerNames() const;
    #pragma endregion 

    #pragma region Output Management
    int GetOutputCount() const;
    std::list<Output*> GetAllOutputs(const std::string& ip, const std::string& hostName = std::string()) const;
    std::list<Output*> GetAllOutputs() const;
    std::list<Output*> GetOutputs() const { return GetAllOutputs(); }
    Output* GetOutput_CONVERT(int outputNumber) const;
    Output* GetOutput(int32_t absoluteChannel, int32_t& startChannel) const; // returns the output ... even if it is in a collection
    Output* GetOutput(int universe, const std::string& ip) const;
    #pragma endregion 

    #pragma region Channel Mapping
    int32_t GetTotalChannels() const;

    //both outputNumber and startChannel are 0 based
    int32_t GetAbsoluteChannel(int controllerIndex, int32_t startChannel) const;
    int32_t GetAbsoluteChannel(const std::string& ip, int universe, int32_t startChannel) const;
    int32_t DecodeStartChannel(const std::string& startChannelString);
    #pragma endregion 

    #pragma region Getters and Setters
    std::string GetFirstUnusedCommPort() const;

    std::list<int> GetIPUniverses(const std::string& ip = std::string()) const;
    std::list<std::string> GetIps() const;

    std::string GetGlobalFPPProxy() const { return _globalFPPProxy; }
    void SetGlobalFPPProxy(const std::string& globalFPPProxy);
    
    bool Discover(wxWindow* parent, std::map<std::string, std::string>& renames); // discover controllers and add them to the list if they are not already there
    
    void SetShowDir(const std::string& showDir);
    
    void SuspendAll(bool suspend);
    
    void SetParallelTransmission(bool parallel) { _parallelTransmission = parallel; }
    bool GetParallelTransmission() const { return _parallelTransmission; }
    
    int GetPacketsPerSecond() const;
    
    void UpdateUnmanaged();
    
    std::string UniqueName(const std::string& prefix);
    
    int UniqueId();

    void SomethingChanged() const;
    bool IsDirty() const;

    void SetForceFromIP(const std::string& forceFromIP);

    bool AtLeastOneOutputUsingProtocol(const std::string& protocol) const;

    void SetSuppressFrames(int suppressFrames) { _suppressFrames = suppressFrames; _dirty = true; }
    int GetSuppressFrames() const { return _suppressFrames; }
    
    std::string GetChannelName(int32_t channel);

    bool IsOutputOpenInAnotherProcess();
    #pragma endregion 

    #pragma region Start and Stop
    bool StartOutput();
    void StopOutput();
    bool IsOutputting() const { return _outputting; }
    size_t TxNonEmptyCount();
    bool TxEmpty();
    #pragma endregion 

    #pragma region Frame Handling
    void StartFrame(long msec);
    void EndFrame();
    void ResetFrame();
    void SendHeartbeat();
    #pragma endregion 

    #pragma region Packet Sync
    bool IsSyncEnabled() const { return _syncEnabled; }
    static bool IsSyncEnabled_() { return __isSync; }
    void SetSyncEnabled(bool syncEnabled) {
        _syncEnabled = syncEnabled;
        OutputManager::__isSync = syncEnabled;
        _dirty = true;
        if (!_syncEnabled) SetSyncUniverse(0);
    }
    int GetSyncUniverse() const { return _syncUniverse; }
    void SetSyncUniverse(int syncUniverse) { _syncUniverse = syncUniverse; _dirty = true;}
    #pragma endregion 

    #pragma region Data Setting
    void SetOneChannel(int32_t channel, unsigned char data);
    void SetManyChannels(int32_t channel, unsigned char* data, size_t size);
    void AllOff(bool send = true);
    #pragma endregion 

    #pragma region Test Presets
    std::list<std::string> GetTestPresets();
    TestPreset* GetTestPreset(std::string preset);
    TestPreset* CreateTestPreset(std::string preset);
    #pragma endregion 
};
