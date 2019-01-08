#ifndef OUTPUTMANAGER_H
#define OUTPUTMANAGER_H

#include <list>
#include <string>
#include <wx/thread.h>

class Output;
class Controller;
class TestPreset;

#define NETWORKSFILE "xlights_networks.xml";

class OutputManager
{
    #pragma region Member Variables
    std::string _filename;
    std::list<Output*> _outputs;
    std::list<TestPreset*> _testPresets;
    int _syncUniverse;
    bool _syncEnabled;
    bool _dirty;
    int _suppressFrames;
    bool _parallelTransmission;
    bool _outputting; // true if we are currently sending out data
    wxCriticalSection _outputCriticalSection; // used to protect areas that must be single threaded
    #pragma endregion Member Variables

    static int _lastSecond;
    static int _currentSecond;
    static int _lastSecondCount;
    static int _currentSecondCount;
    static bool _isRetryOpen;
    static bool _isInteractive;

    bool SetGlobalOutputtingFlag(bool state, bool force = false);

public:

    #pragma region Constructors and Destructors
    OutputManager();
    ~OutputManager();
    #pragma endregion Constructors and Destructors

    #pragma region Static Functions
    static std::string GetNetworksFileName() { return NETWORKSFILE; }
    int GetPacketsPerSecond() const;
    static void RegisterSentPacket();
    static bool IsRetryOpen() { return _isRetryOpen; }
    static void SetRetryOpen(bool retryOpen) { _isRetryOpen = retryOpen; }
    static bool IsInteractive() { return _isInteractive; }
    static void SetInteractive(bool interactive) { _isInteractive = interactive; }
    #pragma endregion Static Functions

    #pragma region Save and Load
    bool Load(const std::string& showdir, bool syncEnabled = false);
    bool Save();
    #pragma endregion Save and Load

    #pragma region Output Management
    void AddOutput(Output* output, Output* after);
    void AddOutput(Output* output, int pos);
    void DeleteOutput(Output* output);
    void DeleteAllOutputs();
    void MoveOutput(Output* output, int toOutputNumber);
    bool AreAllIPOutputs(std::list<int> outputNumbers);
    std::list<Output*> GetAllOutputs(const std::string& ip, const std::list<int>& selected = std::list<int>()) const {
        return GetAllOutputs(ip, "", selected);
    }
    std::list<Output*> GetAllOutputs(const std::string& ip, const std::string &hostName, const std::list<int>& selected = std::list<int>()) const;
    std::list<Output*> GetAllOutputs(const std::list<int>& outputNumbers) const;
    std::list<Output*> GetAllOutputs() const;
    std::list<Output*> GetOutputs() const { return _outputs; } // returns a list like that on setup tab
    void Replace(Output* replacethis, Output* withthis);
    Output* GetOutput(int outputNumber) const;
    Output* GetOutput(long absoluteChannel, long& startChannel) const; // returns the output ... even if it is in a collection
    Output* GetLevel1Output(long absoluteChannel, long& startChannel) const; // returns the output ... but always level 1
    Output* GetOutput(int universe, const std::string& ip) const;
    std::list<int> GetIPUniverses(const std::string& ip = "") const;
    int GetOutputCount() const { return _outputs.size(); }
    bool Discover(); // discover controllers and add them to the list if they are not already there
    void SetShowDir(const std::string& showDir);
    void SuspendAll(bool suspend);
    void SetParallelTransmission(bool parallel) { _parallelTransmission = parallel; }
    bool GetParallelTransmission() const { return _parallelTransmission; }
    #pragma endregion Output Management

    void SomethingChanged() const;
    bool IsDirty() const;

    #pragma region Channel Mapping
    long GetTotalChannels() const;
    
    //both outputNumber and startChannel are 0 based
    long GetAbsoluteChannel(int outputNumber, int startChannel) const;
    long GetAbsoluteChannel(const std::string& ip, int universe, int startChannel) const;
    long DecodeStartChannel(const std::string& startChannelString);
    #pragma endregion Channel Mapping

    #pragma region Start and Stop
    bool StartOutput();
    void StopOutput();
    bool IsOutputting() const { return _outputting; }
    #pragma endregion Start and Stop

    #pragma region Frame Handling
    void StartFrame(long msec);
    void EndFrame();
    void ResetFrame();
    #pragma endregion Frame Handling

    #pragma region Packet Sync
    bool IsSyncEnabled() const { return _syncEnabled; }
    void SetSyncEnabled(bool syncEnabled) { _syncEnabled = syncEnabled; _dirty = true; }
    int GetSyncUniverse() const { return _syncUniverse; }
    void SetSyncUniverse(int syncUniverse) { _syncUniverse = syncUniverse; _dirty = true;}
    void SetForceFromIP(const std::string& forceFromIP);
    bool UseE131() const;
    bool UseArtnet() const;
    bool UseDDP() const;
    #pragma endregion Packet Sync

    #pragma region Data Setting
    void SetOneChannel(long channel, unsigned char data);
    void SetManyChannels(long channel, unsigned char* data, long size);
    void AllOff(bool send = true);
    #pragma endregion Data Setting

    #pragma region Test Presets
    std::list<std::string> GetTestPresets();
    TestPreset* GetTestPreset(std::string preset);
    TestPreset* CreateTestPreset(std::string preset);
    #pragma endregion Test Presets

    void SetSuppressFrames(int suppressFrames) { _suppressFrames = suppressFrames; _dirty = true; }
    int GetSuppressFrames() const { return _suppressFrames; }
    std::list<std::string> GetIps() const;
    size_t TxNonEmptyCount();
    bool TxEmpty();
    std::string GetChannelName(long channel);
    void SendHeartbeat();

    bool IsOutputOpenInAnotherProcess();
};

#endif
