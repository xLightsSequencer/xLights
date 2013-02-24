#ifndef XLIGHTSNETINFO_H
#define XLIGHTSNETINFO_H

#include <vector>
#include <wx/string.h>
#include <wx/arrstr.h>

class NetInfoClass
{
protected:
    std::vector<size_t> NetMaxChannel;

public:
    void Clear();
    void AddNetwork(size_t NumChannels);
    size_t GetNumNetworks();
    int GetNumChannels(size_t NetNum);
    int CalcAbsChannel(int NetNum, int NetCh);
    int GetTotChannels();
    void GetAllChannelNames(wxArrayString& names);
    wxString GetChannelName(int ChNum);
};

#endif // XLIGHTSNETINFO_H
