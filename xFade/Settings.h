#ifndef SETTINGS_H
#define SETTINGS_H
#include <string>
#include <map>
#include <list>

class Settings
{
public:

    int _frameMS;
    std::string _leftIP;
    std::string _rightIP;
    std::string _localInputIP;
    std::string _localOutputIP;
    std::string _defaultIP;
    std::map<int, std::string> _targetIP;
    std::map<int, std::string> _targetDesc;
    std::map<int, std::string> _targetProtocol;
    bool _ArtNET;
    bool _E131;
    std::map<std::string, int> _midiDevice;
    std::map<std::string, int> _midiStatus;
    std::map<std::string, int> _midiChannel;
    std::map<std::string, int> _midiData1;
    std::map<std::string, int> _midiData2;
    std::list<std::string> _fadeExclude;
    std::string _defaultMIDIDevice = "";

    static std::list<std::string> GetMIDIDevices();
    static std::string GetMIDIDeviceName(int device);
    static int GetMIDIDeviceId(const std::string& deviceName);
    Settings(std::string settings = "");
	virtual ~Settings() {}
    std::string Serialise();
    void Load(std::string settings);
    bool IsFadeExclude(std::string ch);
    static void DecodeUSC(std::string ch, int & u, int & sc);
    void AddFadeExclude(std::string ch);
    void DeleteFadeExclude(std::string ch);
    std::list<int> GetExcludeChannels(int u);
    std::list<int> GetUsedMIDIDevices() const;
    std::string Safe(std::string s);
    void SetMIDIControl(const std::string& midiDevice, const std::string& controlName, int status, int channel, int data1, int data2);
    std::string LookupMIDI(int midiDevice, int status, int channel, int data1, int data2) const;
    void LookupMIDI(std::string controlName, int& midiDevice, int& status, int& channel, int& data1, int& data2);
    const std::list<std::string>& GetFadeExclude() const { return _fadeExclude; }
};

#endif 
