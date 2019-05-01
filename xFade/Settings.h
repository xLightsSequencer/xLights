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
    std::string _midiDevice;
    std::map<std::string, int> _midiStatus;
    std::map<std::string, int> _midiChannel;
    std::map<std::string, int> _midiData1;
    std::list<std::string> _fadeExclude;

	Settings(std::string settings = "");
	virtual ~Settings() {}
    std::string Serialise();
    void Load(std::string settings);
    int GetMIDIDeviceId();
    bool IsFadeExclude(std::string ch);
    static void DecodeUSC(std::string ch, int & u, int & sc);
    void AddFadeExclude(std::string ch);
    void DeleteFadeExclude(std::string ch);
    std::list<int> GetExcludeChannels(int u);
    std::string Safe(std::string s);
    void SetMIDIControl(std::string controlName, int status, int channel, int data1);
    std::string LookupMIDI(int status, int channel, int data1) const;
    void LookupMIDI(std::string controlName, int& status, int& channel, int& data1);
    const std::list<std::string>& GetFadeExclude() const { return _fadeExclude; }
};

#endif 
