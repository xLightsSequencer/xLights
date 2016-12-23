#ifndef OUTPUTMANAGER_H
#define OUTPUTMANAGER_H

#include <list>
#include <wx/thread.h>

class Output;
class Controller;
class TestPreset;

class OutputManager
{
    const std::string NETWORKSFILE = "xlights_networks.xml";
    std::string _filename;
    std::list<Output*> _outputs;
    std::list<TestPreset*> _testPresets;
    int _syncUniverse;
    bool _syncEnabled;
    bool _dirty;
    bool _outputting;
    wxCriticalSection _outputCriticalSection;

    void ResetOutputNumbers() const;

public:
    OutputManager(const std::string& showdir, bool syncEnabled = false);
    OutputManager();
    ~OutputManager();

    void Load(const std::string& showdir, bool syncEnabled = false);
    void SomethingChanged() const;
    Output* GetOutput(int outputNumber) const;
    Output* GetOutput(int absoluteChannel, int& startChannel) const;
    Output* GetOutput(int universe, const std::string& ip) const;
    int GetTotalChannels() const;
    int GetAbsoluteChannel(int outputNumber, int startChannel = 1) const;
    int GetAbsoluteChannel(const std::string& ip, int universe, int startChannel = 1) const;
    void AddOutput(Output* output, int pos);
    void DeleteOutput(Output* output);
    void DeleteAllOutputs();
    void Save();
    bool IsDirty() const;
    std::list<int> GetIPUniverses(const std::string& ip = "") const;
    std::list<std::string> GetIps() const;
    void StartFrame(long msec);
    void EndFrame();
    bool StartOutput();
    bool IsSyncEnabled() { return _syncEnabled; }
    void SetSyncEnabled(bool syncEnabled) { _syncEnabled = syncEnabled; }
    void StopOutput();
    bool IsOutputting() const { return _outputting; }
    int GetSyncUniverse() const { return _syncUniverse; }
    void SetSyncUniverse(int syncUniverse) { _syncUniverse = syncUniverse; }
    void AllOff();
    void SendHeartbeat();
    void MoveOutput(Output* output, int toOutputNumber);
    bool AreAllIPOutputs(std::list<int> outputNumbers);
    std::list<Output*> GetOutputs(const std::string& ip) const;
    std::list<Output*> GetOutputs(std::list<int>& outputNumbers) const;
    std::list<Output*> GetOutputs() const { return _outputs; }
    bool UseE131() const;
    bool UseArtnet() const;
    std::list<std::string> GetTestPresets();
    TestPreset* GetTestPreset(std::string preset);
    TestPreset* CreateTestPreset(std::string preset);
    void SetOneChannel(int channel, unsigned char data);
};
 
#endif