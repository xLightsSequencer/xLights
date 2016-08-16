
#include "NetInfo.h"

#include <stdarg.h>

inline std::string string_format(const std::string fmt, ...) {
    int size=100;
    std::string str;
    va_list ap;
    while (1) {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {
            return str;
        }
        if (n > -1) {
            size=n+1;
        } else {
            size*=2;
        }
    }
}

void NetInfoClass::Clear()
{
    NetMaxChannel.clear();
    NetUniverse.clear();
    NetUniverseChannel.clear();
}

void NetInfoClass::AddUniverseNetwork(wxString ip, size_t Universe, size_t NumChannels)
{
    NetIP.push_back(ip);
    NetUniverse.push_back(Universe);
    NetUniverseChannel.push_back(NumChannels);
}

void NetInfoClass::AddNetwork(size_t NumChannels)
{
    NetMaxChannel.push_back(NumChannels);
}

size_t NetInfoClass::GetNumNetworks() const
{
    return NetMaxChannel.size();
}

// returns -1 if NetNum is invalid
int NetInfoClass::GetNumChannels(size_t NetNum) const
{
    if (NetNum < NetMaxChannel.size())
    {
        return NetMaxChannel[NetNum];
    }
    else
    {
        return -1;
    }
}

// first channel starts with 0
int NetInfoClass::CalcAbsChannel(int NetNum, int NetCh) const
{
    int AbsChannel = -1;
    if (NetNum>=0 && NetNum<NetMaxChannel.size() && NetCh>=0 && NetCh<NetMaxChannel[NetNum])
    {
        AbsChannel = NetCh;
        for (int i=0; i < NetNum; i++)
        {
            AbsChannel += NetMaxChannel[i];
        }
    }
    return AbsChannel;
}

// first channel starts with 0
int NetInfoClass::CalcUniverseChannel(size_t Universe, int Ch) const
{
    int AbsChannel = -1;
    for (size_t i = 0; i < NetUniverse.size(); i++)
    {
        if (NetUniverse[i] == Universe)
        {
            return AbsChannel + Ch + 1;
        }
        AbsChannel += NetUniverseChannel[i];
    }

    // return -1 if not found
    return -1;
}

int NetInfoClass::CalcUniverseChannel(wxString ip, size_t Universe, int Ch) const
{
    int AbsChannel = -1;
    for (size_t i = 0; i < NetUniverse.size(); i++)
    {
        if (NetIP[i] == ip && NetUniverse[i] == Universe)
        {
            return AbsChannel + Ch + 1;
        }
        AbsChannel += NetUniverseChannel[i];
    }

    // return 1 if not found
    return 1;
}

std::vector<size_t> NetInfoClass::GetUniversesForIP(wxString& ip) const
{
    std::vector<size_t> res;

    for (size_t i = 0; i < NetUniverse.size(); i++)
    {
        if (NetIP[i] == ip)
        {
            res.push_back(NetUniverse[i]);
        }
    }

    return res;
}

unsigned int NetInfoClass::GetTotChannels() const
{
    unsigned tot=0;
    for(size_t NetNum=0; NetNum < NetMaxChannel.size(); NetNum++)
    {
        tot+=NetMaxChannel[NetNum];
    }
    return tot;
}

void NetInfoClass::GetAllChannelNames(wxArrayString& names) const
{
    int ChNum=0;
    names.Clear();
    names.Alloc(GetTotChannels());
    names.reserve(GetTotChannels());
    for(size_t NetNum=0; NetNum < NetMaxChannel.size(); NetNum++)
    {
        for(size_t NetCh=0; NetCh < NetMaxChannel[NetNum]; NetCh++)
        {
            names.push_back(string_format("Ch %d: Net %d #%d",int(ChNum+1),int(NetNum+1),int(NetCh+1)));
            ChNum++;
        }
    }
}

// first ChNum is 0
wxString NetInfoClass::GetChannelName(int ChNum) const
{
    int NetCh=ChNum;
    for (size_t NetNum=0; NetNum < NetMaxChannel.size(); NetNum++)
    {
        if (NetCh < NetMaxChannel[NetNum])
        {
            return string_format("Ch %d: Net %d #%d",int(ChNum+1),int(NetNum+1),int(NetCh+1));
        }
        NetCh-=NetMaxChannel[NetNum];
    }
    return string_format("Ch %d: invalid",ChNum);
}

bool NetInfoClass::GetEndNetworkAndChannel(int netNum,int startChannel, int totalChannels,int& endNetNum, int& endChannel) const
{
    if(netNum >= GetNumNetworks())
    {
        return false;
    }

    if(startChannel > GetNumChannels(netNum))
    {
        return false;
    }

    int index = netNum;
    int start = startChannel-1;
    int countRemaining = totalChannels;
	int channelsLeftInNetwork = GetNumChannels(index) - start;
    while(countRemaining > channelsLeftInNetwork)
    {
        start = 0;
        countRemaining-=channelsLeftInNetwork;
        // Is there more networks defined
        if(index<GetNumNetworks()-1)
        {
            // Yes there are more networks defined, increment
            index++;
        }
        else
        {
            return false;
        }
        channelsLeftInNetwork = GetNumChannels(index);
    }
    endNetNum = index;
    endChannel = countRemaining+start;
    return true;
}

bool NetInfoClass::GetStartNetworkAndChannelFromEndInfo(int endNetNum, int endChannel,int& netNum,int& startChannel) const
{
    if(endChannel+1 < GetNumChannels(endNetNum))
    {
        netNum = endNetNum;
        startChannel = endChannel+1;
        return true;
    }
    else
    {
        netNum = endNetNum+1;
        startChannel = 1;
        if(netNum<GetNumNetworks())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}
