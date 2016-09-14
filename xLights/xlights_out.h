/***************************************************************
 * Name:      xlights_out.h
 * Purpose:
    Defines a common API for LOR, D-Light, DMX, Renard, Pixelnet, and E1.31 networks
    All calls should be made to xOutput class
 *
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2010-05-07
 * Copyright: 2010-2012 by Matt Brown
 * License:
     This file is part of xLights.

    xLights is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xLights is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xLights.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************/
#ifndef XLIGHTSOUT_H
#define XLIGHTSOUT_H


#include <wx/string.h>
#include <wx/dynarray.h>
#include <vector>
#include <wx/socket.h>

typedef std::pair<int, int> ChannelPair; // first is network #, second is channel #
typedef std::vector<ChannelPair> ChannelVector;

#define ARTNET_UNIVERSE(a) (a & 0x000F)
#define ARTNET_SUBNET(a) ((a & 0x00F0) >> 4)
#define ARTNET_NET(a) ((a & 0x7F00) >> 8)
#define ARTNET_MAKEU(n, s, u) (((n&0x007F) << 8) + ((s &0x000F) << 4) + (u & 0x000F))

class xNetwork
{
protected:
    long timer_msec;
    size_t num_channels;
    wxString netdesc;
    int datalen;
    bool enabled;

public:
    xNetwork();
    virtual ~xNetwork();
    void CloseSerialPort();
    virtual bool TxEmpty() = 0;
    virtual size_t TxNonEmptyCount() = 0;
    void SetNetworkDesc(wxString& NetworkDesc);
    wxString GetNetworkDesc();
    size_t GetChannelCount();
    virtual void SetChannelCount(size_t numchannels) = 0;
    virtual void InitSerialPort(const wxString& portname, int baudrate);
    virtual void InitNetwork(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum, wxUint16 syncuniverse);
    virtual void TimerStart(long msec);
    long GetTimer();
    virtual void TimerEnd() = 0;
    virtual void ResetTimer();
    virtual void SetIntensity (size_t chindex, wxByte intensity) = 0;
    void SetEnabled(bool b) {
        enabled = b;
    }
};


// xOutput should be a singleton
// It contains references to all of the networks
class xOutput
{
protected:
    WX_DEFINE_ARRAY_PTR(xNetwork*, xNetworkArray);
    xNetworkArray networks;
    ChannelVector channels;
    wxUint16 _syncuniverse;
    bool _sync;

public:
    xOutput(bool sync = false, wxUint16 syncuniverse = 0);
    ~xOutput();
    void EnableOutput(size_t network, bool en = true);
    size_t NetworkCount();
    size_t addnetwork (const wxString& NetworkType, int chcount, const wxString& portname, int baudrate, int count, bool enabled);
    int GetChannelCount(size_t netnum);
    wxString GetNetworkDesc(size_t netnum);
    void SetIntensity (size_t absChNum, wxByte intensity);
    void SetIntensities(size_t startChannel, unsigned char *buffer, size_t count);
    void off (size_t absChNum);
    void alloff ();
    size_t TotalChannelCount();
    size_t AbsChannel2NetNum(size_t absChNum);
    size_t AbsChannel2NetCh(size_t absChNum);
    ChannelPair AbsChannelPair(size_t absChNum);
    void ResetTimer();
    void TimerStart(long msec);
    void TimerEnd();
    void ClosePorts();
    bool TxEmpty();
    size_t TxNonEmptyCount(void);
    void SetSyncUniverse(wxUint16 syncuniverse);
};

#define ARTNET_PACKET_HEADER_LEN 18
#define ARTNET_PACKET_LEN (ARTNET_PACKET_HEADER_LEN + 512)
class xNetwork_ArtNET : public xNetwork
{
protected:
    wxByte data[ARTNET_PACKET_LEN];
    wxByte SequenceNum;
    int SkipCount;
    wxIPV4address remoteAddr;
    wxDatagramSocket *datagram;
    bool changed;
    static int _ip1;
    static int _ip2;
    static int _ip3;
    static bool _initialised;

public:
    void SetIntensity(size_t chindex, wxByte intensity) override;
    xNetwork_ArtNET();
    ~xNetwork_ArtNET();
    virtual void InitNetwork(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum, wxUint16 syncuniverse = 0) override;

private:
    void InitData(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum);
    void InitRemoteAddr(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum);

public:
    void SetChannelCount(size_t numchannels) override;
    void TimerEnd() override;
    static void Sync();
    size_t TxNonEmptyCount(void) override;
    bool TxEmpty() override;
};

#define E131_PACKET_LEN 638
class xNetwork_E131 : public xNetwork
{
protected:
    static wxUint16 _syncuniverse;
    wxByte data[E131_PACKET_LEN];
    wxByte SequenceNum;
    int SkipCount;
    wxIPV4address remoteAddr;
    wxDatagramSocket *datagram;
    bool changed;

public:

    static void Sync();
    void SetIntensity(size_t chindex, wxByte intensity) override;
    xNetwork_E131();
    ~xNetwork_E131();
    virtual void InitNetwork(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum, wxUint16 syncuniverse) override;
    static void SetSyncUniverseStatic(wxUint16 syncuniverse);
    void SetSyncUniverse(wxUint16 syncuniverse);
private:
    void InitData(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum);
    void InitRemoteAddr(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum);

public:
    void SetChannelCount(size_t numchannels) override;
    void TimerEnd() override;
    size_t TxNonEmptyCount(void) override;
    bool TxEmpty() override;
};

#endif // XLIGHTSOUT_H
