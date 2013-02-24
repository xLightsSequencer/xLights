#include "NetInfo.h"

void NetInfoClass::Clear()
{
    NetMaxChannel.clear();
}

void NetInfoClass::AddNetwork(size_t NumChannels)
{
    NetMaxChannel.push_back(NumChannels);
}

size_t NetInfoClass::GetNumNetworks()
{
    return NetMaxChannel.size();
}

// returns -1 if NetNum is invalid
int NetInfoClass::GetNumChannels(size_t NetNum)
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
int NetInfoClass::CalcAbsChannel(int NetNum, int NetCh)
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

int NetInfoClass::GetTotChannels()
{
    int tot=0;
    for(size_t NetNum=0; NetNum < NetMaxChannel.size(); NetNum++)
    {
        tot+=NetMaxChannel[NetNum];
    }
    return tot;
}

void NetInfoClass::GetAllChannelNames(wxArrayString& names)
{
    int ChNum=0;
    for(size_t NetNum=0; NetNum < NetMaxChannel.size(); NetNum++)
    {
        for(size_t NetCh=0; NetCh < NetMaxChannel[NetNum]; NetCh++)
        {
            names.Add(wxString::Format(wxT("Ch %d: Net %d #%d"),int(ChNum+1),int(NetNum+1),int(NetCh+1)));
            ChNum++;
        }
    }
}

// first ChNum is 0
wxString NetInfoClass::GetChannelName(int ChNum)
{
    int NetCh=ChNum;
    for (size_t NetNum=0; NetNum < NetMaxChannel.size(); NetNum++)
    {
        if (NetCh < NetMaxChannel[NetNum])
        {
            return wxString::Format(wxT("Ch %d: Net %d #%d"),int(ChNum+1),int(NetNum+1),int(NetCh+1));
        }
        NetCh-=NetMaxChannel[NetNum];
    }
    return wxString::Format(wxT("Ch %d: invalid"),ChNum);
}
