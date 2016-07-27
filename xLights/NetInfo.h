#ifndef XLIGHTSNETINFO_H
#define XLIGHTSNETINFO_H

#include <vector>
#ifdef FPP
#include <string>
typedef std::string wxString;
typedef std::vector<int> wxArrayInt;
typedef std::vector<std::string> wxArrayString;
#else
#include <wx/string.h>
#include <wx/arrstr.h>
#endif

class NetInfoClass
{
protected:
    std::vector<size_t> NetMaxChannel;
    std::vector<size_t> NetUniverse;
    std::vector<wxString> NetIP;
    std::vector<size_t> NetUniverseChannel;

public:
    void Clear();
    void AddNetwork(size_t NumChannels);
    void AddUniverseNetwork(wxString ip, size_t Universe, size_t NumChannels);
    size_t GetNumNetworks() const;
    int GetNumChannels(size_t NetNum) const;
    int CalcAbsChannel(int NetNum, int NetCh) const;
    int CalcUniverseChannel(size_t universe, int Ch) const;
    int CalcUniverseChannel(wxString ip, size_t universe, int Ch) const;
    unsigned int GetTotChannels() const;
    void GetAllChannelNames(wxArrayString& names) const;
    wxString GetChannelName(int ChNum) const;
    bool GetEndNetworkAndChannel(int netNum,int startChannel, int totalChannels,int& endNetNum, int& endChannel) const;
    bool GetStartNetworkAndChannelFromEndInfo(int endNetNum, int endChannel,int& netNum,int& startChannel) const;
};

#endif // XLIGHTSNETINFO_H
