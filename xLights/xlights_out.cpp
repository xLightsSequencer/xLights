/***************************************************************
 * Name:      xlights_out.cpp
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

#include "xlights_out.h"
#include <wx/socket.h>
#include "serial.h"

bool xNetwork_E131_changed = false;

// ***********************
// * xNetwork (base class)
// ***********************
xNetwork::xNetwork()
{
    timer_msec=0;
    num_channels=0;
}

xNetwork::~xNetwork()
{
}

void xNetwork::SetNetworkDesc(wxString& NetworkDesc)
{
    netdesc=NetworkDesc;
}

wxString xNetwork::GetNetworkDesc()
{
    return netdesc;
}

size_t xNetwork::GetChannelCount()
{
    return num_channels;
}

void xNetwork::InitSerialPort(const wxString& portname, int baudrate)
{
}

void xNetwork::InitNetwork(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum)
{
}

void xNetwork::TimerStart(long msec)
{
    timer_msec=msec;
}

long xNetwork::GetTimer()
{
    return timer_msec;
}

void xNetwork::ResetTimer()
{
}


// ***************************************************************************************
// * This is the base class for all serial port (and virtual com port) driven protocols
// ***************************************************************************************
class xNetwork_Serial: public xNetwork
{
protected:
    SerialPort* serptr;
    char SerialConfig[4];
    bool changed;

public:
    xNetwork_Serial()
    {
        serptr=0;
        strcpy(SerialConfig,"8N1");
    };

    virtual ~xNetwork_Serial()
    {
        if (serptr) delete serptr;
    };

    void InitSerialPort(const wxString& portname, int baudrate)
    {
        static char errmsg[100];
        if (portname == wxT("NotConnected")) return;
        serptr=new SerialPort();
        int errcode=serptr->Open(portname, baudrate, SerialConfig);
        if (errcode < 0)
        {
            sprintf(errmsg,"unable to open serial port, error code=%d",errcode);
            throw errmsg;
        }
    };

    bool TxEmpty()
    {
        if (serptr) return (serptr->WaitingToWrite() == 0);
        return true;
    };

};


// ***************************************************************************************
// * This class represents a single DMX universe
// * Compatible with Entec Pro, Lynx DMX, DIYC RPM, DMXking.com, and DIYblinky.com dongles
// * Universes are exactly 512 bytes, except for DIYblinky, where they can be up to 3036 bytes
// * Methods should be called with: 0 <= chindex < 3036
// ***************************************************************************************
class xNetwork_DMXpro: public xNetwork_Serial
{
protected:
    wxByte data[3036+6];

    void SetIntensity(size_t chindex, wxByte intensity)
    {
        data[chindex+5]=intensity;
        changed=true;
    };

public:
    void SetChannelCount(size_t numchannels)
    {
        if (numchannels > 3036)
        {
            throw "max channels on DMX is 3036";
        }
        int len=numchannels < 512 ? 513 : numchannels+1;
        datalen=len+5;
        data[0]=0x7E;               // start of message
        data[1]=6;                  // dmx send
        data[2]=len & 0xFF;         // length LSB
        data[3]=(len >> 8) & 0xFF;  // length MSB
        data[4]=0;                  // DMX start
        data[datalen-1]=0xE7;       // end of message
        changed=false;
        num_channels=numchannels;
    };

    void TimerEnd()
    {
        if (changed && serptr)
        {
            serptr->Write((char *)data,datalen);
            changed=false;
        }
    };
};


// ******************************************************
// * This class represents a single DMX universe
// * Compatible with Entec Open DMX, LOR dongle, D-Light dongle,
// * and any other FTDI-based USB to RS-485 converter
// * Methods should be called with: 0 <= chindex <= 511
// ******************************************************
class xNetwork_DMXopen: public xNetwork_Serial
{
protected:
    wxByte data[513];

    void SetIntensity(size_t chindex, wxByte intensity)
    {
        data[chindex+1]=intensity;
        changed=true;
    };

public:
    void SetChannelCount(size_t numchannels)
    {
        if (numchannels > 512)
        {
            throw "max channels on DMX is 512";
        }
        data[0]=0;   // dmx start code
        changed=false;
        num_channels=numchannels;
        SerialConfig[2]='2'; // use 2 stop bits so padding chars are not required
    };

    void TimerEnd()
    {
        if (serptr)
        {
            serptr->SendBreak();  // sends a 1 millisecond break
            wxMilliSleep(1);      // mark after break (MAB) - 1 millisecond is overkill (8 microseconds is the minimum dmx requirement)
            serptr->Write((char *)data,513);
            changed=false;
        }
    };
};


#define E131_PACKET_LEN 638
#define E131_PORT 5568
#define XLIGHTS_UUID "c0de0080-c69b-11e0-9572-0800200c9a66"

// ******************************************************
// * This class represents a single universe for E1.31
// * Methods should be called with: 0 <= chindex <= 511
// ******************************************************
class xNetwork_E131: public xNetwork
{
protected:
    wxByte data[E131_PACKET_LEN];
    wxByte SequenceNum;
    int SkipCount;
    wxIPV4address remoteAddr;
    wxDatagramSocket *datagram;

    void SetIntensity(size_t chindex, wxByte intensity)
    {
        data[chindex+126]=intensity;
        xNetwork_E131_changed=true;
    };

public:
    xNetwork_E131()
    {
        datagram=0;
        memset(data,0,sizeof(data));
    };

    ~xNetwork_E131()
    {
        if (datagram) delete datagram;
    };

    void InitNetwork(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum)
    {
        if (UniverseNumber == 0 || UniverseNumber >= 64000)
        {
            throw "universe number must be between 1 and 63999";
        }
        SequenceNum=0;
        SkipCount=0;
        wxByte UnivHi = UniverseNumber >> 8;   // Universe Number (high)
        wxByte UnivLo = UniverseNumber & 0xff; // Universe Number (low)

        data[0]=0x00;   // RLP preamble size (high)
        data[1]=0x10;   // RLP preamble size (low)
        data[2]=0x00;   // RLP postamble size (high)
        data[3]=0x00;   // RLP postamble size (low)
        data[4]=0x41;   // ACN Packet Identifier (12 bytes)
        data[5]=0x53;
        data[6]=0x43;
        data[7]=0x2d;
        data[8]=0x45;
        data[9]=0x31;
        data[10]=0x2e;
        data[11]=0x31;
        data[12]=0x37;
        data[13]=0x00;
        data[14]=0x00;
        data[15]=0x00;
        data[16]=0x72;  // RLP Protocol flags and length (high)
        data[17]=0x6e;  // 0x26e = 638 - 16
        data[18]=0x00;  // RLP Vector (Identifies RLP Data as 1.31 Protocol PDU)
        data[19]=0x00;
        data[20]=0x00;
        data[21]=0x04;

        // CID/UUID

        wxChar msb,lsb;
        wxString id=wxT(XLIGHTS_UUID);
        id.Replace(wxT("-"), wxT(""));
        id.MakeLower();
        if (id.Len() != 32) throw "invalid CID";
        for (int i=0,j=22; i < 32; i+=2)
        {
            msb=id.GetChar(i);
            lsb=id.GetChar(i+1);
            msb -= isdigit(msb) ? 0x30 : 0x57;
            lsb -= isdigit(lsb) ? 0x30 : 0x57;
            data[j++] = (wxByte)((msb << 4) | lsb);
        }

        data[38]=0x72;  // Framing Protocol flags and length (high)
        data[39]=0x58;  // 0x258 = 638 - 38
        data[40]=0x00;  // Framing Vector (indicates that the E1.31 framing layer is wrapping a DMP PDU)
        data[41]=0x00;
        data[42]=0x00;
        data[43]=0x02;
        data[44]='x';   // Source Name (64 bytes)
        data[45]='L';
        data[46]='i';
        data[47]='g';
        data[48]='h';
        data[49]='t';
        data[50]='s';
        data[51]=0x00;
        data[52]=0x00;
        data[53]=0x00;
        data[54]=0x00;
        data[55]=0x00;
        data[56]=0x00;
        data[57]=0x00;
        data[58]=0x00;
        data[59]=0x00;
        data[60]=0x00;
        data[61]=0x00;
        data[61]=0x00;
        data[62]=0x00;
        data[63]=0x00;
        data[64]=0x00;
        data[65]=0x00;
        data[66]=0x00;
        data[67]=0x00;
        data[68]=0x00;
        data[69]=0x00;
        data[70]=0x00;
        data[71]=0x00;
        data[71]=0x00;
        data[72]=0x00;
        data[73]=0x00;
        data[74]=0x00;
        data[75]=0x00;
        data[76]=0x00;
        data[77]=0x00;
        data[78]=0x00;
        data[79]=0x00;
        data[80]=0x00;
        data[81]=0x00;
        data[81]=0x00;
        data[82]=0x00;
        data[83]=0x00;
        data[84]=0x00;
        data[85]=0x00;
        data[86]=0x00;
        data[87]=0x00;
        data[88]=0x00;
        data[89]=0x00;
        data[90]=0x00;
        data[91]=0x00;
        data[91]=0x00;
        data[92]=0x00;
        data[93]=0x00;
        data[94]=0x00;
        data[95]=0x00;
        data[96]=0x00;
        data[97]=0x00;
        data[98]=0x00;
        data[99]=0x00;
        data[100]=0x00;
        data[101]=0x00;
        data[101]=0x00;
        data[102]=0x00;
        data[103]=0x00;
        data[104]=0x00;
        data[105]=0x00;
        data[106]=0x00;
        data[107]=0x00;
        data[108]=100;  // Priority
        data[109]=0x00; // Reserved
        data[110]=0x00; // Reserved
        data[111]=0x00; // Sequence Number
        data[112]=0x00; // Framing Options Flags
        data[113]=UnivHi;  // Universe Number (high)
        data[114]=UnivLo;  // Universe Number (low)

        data[115]=0x72;  // DMP Protocol flags and length (high)
        data[116]=0x0b;  // 0x20b = 638 - 115
        data[117]=0x02;  // DMP Vector (Identifies DMP Set Property Message PDU)
        data[118]=0xa1;  // DMP Address Type & Data Type
        data[119]=0x00;  // First Property Address (high)
        data[120]=0x00;  // First Property Address (low)
        data[121]=0x00;  // Address Increment (high)
        data[122]=0x01;  // Address Increment (low)
        data[123]=0x02;  // Property value count (high)
        data[124]=0x01;  // Property value count (low)
        data[125]=0x00;  // DMX512-A START Code

        wxIPV4address localaddr;
        localaddr.AnyAddress();
        //localaddr.Hostname(wxT("192.168.2.100"));
        localaddr.Service(0x8000 | NetNum);
        datagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);
        //datagram = new wxDatagramSocket(localaddr, wxSOCKET_WAITALL);

        if (ipaddr.StartsWith(wxT("239.255.")) || ipaddr == wxT("MULTICAST"))
        {
            // multicast - universe number must be in lower 2 bytes
            wxString ipaddrWithUniv = wxString::Format(wxT("%d.%d.%d.%d"),239,255,(int)UnivHi,(int)UnivLo);
            remoteAddr.Hostname (ipaddrWithUniv);
        }
        else
        {
            remoteAddr.Hostname (ipaddr);
        }
        remoteAddr.Service (E131_PORT);
    };

    void SetChannelCount(size_t numchannels)
    {
        if (numchannels > 512)
        {
            throw "max channels on E1.31 is 512";
        }
        num_channels=numchannels;
    };

    void TimerEnd()
    {
        // skipping would cause ECG-DR4 (firmware version 1.30) to timeout
        if (xNetwork_E131_changed || SkipCount > 10)
        {
            data[111]=SequenceNum;
            datagram->SendTo(remoteAddr, data, E131_PACKET_LEN);
            SequenceNum= SequenceNum==255 ? 0 : SequenceNum+1;
            SkipCount=0;
        }
        else
        {
            SkipCount++;
        }
    };

    bool TxEmpty()
    {
        return true;
    }
};



// Should be called with: 0 <= chindex <= 1015 (max channels=127*8)
class xNetwork_Renard: public xNetwork_Serial
{
protected:
    wxByte data[1024];

    void SetIntensity (size_t chindex, wxByte intensity)
    {
        wxByte RenIntensity;
        switch (intensity)
        {
        case 0x7D:
        case 0x7E:
            RenIntensity=0x7C;
            break;
        case 0x7F:
            RenIntensity=0x80;
            break;
        default:
            RenIntensity=intensity;
        }
        data[chindex+2]=RenIntensity;
        changed=true;
    };

public:
    void SetChannelCount(size_t numchannels)
    {
        if (numchannels > 1016)
        {
            throw "max channels on a Renard network is 1016";
        }
        if (numchannels % 8 != 0)
        {
            throw "Number of Renard channels must be a multiple of 8";
        }
        datalen=numchannels+2;
        data[0]=0x7E;               // start of message
        data[1]=0x80;               // start address
        changed=false;
        num_channels=numchannels;
        SerialConfig[2]='2'; // use 2 stop bits so padding chars are not required
    };

    void TimerEnd()
    {
        if (changed && serptr)
        {
            serptr->Write((char *)data,datalen);
            changed=false;
        }
    };
};



// Should be called with: 0 <= chindex <= 4095
class xNetwork_Pixelnet: public xNetwork_Serial
{
protected:
    wxByte data[4096];
    wxByte SerialBuffer[4097];

    void SetIntensity (size_t chindex, wxByte intensity)
    {
        data[chindex]=intensity==170 ? 171 : intensity;
    };

public:
    void SetChannelCount(size_t numchannels)
    {
        if (numchannels > 4096)
        {
            throw "max channels on a Pixelnet network is 4096";
        }
        datalen=numchannels;
        num_channels=numchannels;
        memset(data,0,sizeof(data));
    };

    void TimerEnd()
    {
        if (serptr && serptr->WaitingToWrite()==0)
        {
            memcpy(&SerialBuffer[1],data,sizeof(data));
            SerialBuffer[0]=170;    // start of message
            serptr->Write((char *)SerialBuffer,sizeof(SerialBuffer));
        }
    };
};



// Should be called with: 0 <= chindex <= 3839 (max channels=240*16)
class xNetwork_LOR: public xNetwork_Serial
{
protected:
    long lastheartbeat;
    wxByte IntensityMap[256];

    // set intensity to a value that has already been mapped
    void SetIntensity (size_t chindex, wxByte intensity)
    {
        wxByte d[6];
        d[0]=0;
        d[1]=chindex >> 4;
        if (d[1] < 0xF0) d[1]++;
        d[2]=3;
        d[3]=IntensityMap[intensity];
        d[4]=0x80 | (chindex % 16);
        d[5]=0;
        //printf("LOR SetIntensity 1=%02X 2=%02X 3=%02X 4=%02X\n",d[1],d[2],d[3],d[4]);
        if (serptr) serptr->Write((char *)d,6);
    };

public:
    void SendHeartbeat ()
    {
        wxByte d[5];
        d[0]=0;
        d[1]=0xFF;
        d[2]=0x81;
        d[3]=0x56;
        d[4]=0;
        if (serptr) serptr->Write((char *)d,5);
    };

    void TimerEnd()
    {
        // send heartbeat
        if (timer_msec > lastheartbeat+300 || timer_msec < lastheartbeat || lastheartbeat < 0)
        {
            SendHeartbeat();
            lastheartbeat=timer_msec;
        }
    };

    void SetChannelCount(size_t numchannels)
    {
        num_channels=numchannels;
        SetMaxIntensity(255);
    };

    void ResetTimer()
    {
        lastheartbeat=-1;
    };

    // maxintensity is usually 100 (LOR) or 255 (Vixen)
    void SetMaxIntensity(wxByte maxintensity)
    {
        int temp;
        for (int i=0; i<=maxintensity; i++)
        {
            temp=(int)(100.0*(double)i/(double)maxintensity+0.5);
            switch (temp)
            {
            case 0:
                IntensityMap[i]=0xF0;
                break;
            case 100:
                IntensityMap[i]=0x01;
                break;
            default:
                IntensityMap[i]=228-temp*2;
                break;
            }
        }
    };

};


// xOutput should be a singleton
// It contains references to all of the networks

xOutput::xOutput()
{
    srand((unsigned)time(NULL));
}

xOutput::~xOutput()
{
    WX_CLEAR_ARRAY(networks);
}

size_t xOutput::NetworkCount()
{
    return networks.GetCount();
}

// returns the network index
size_t xOutput::addnetwork (const wxString& NetworkType, int chcount, const wxString& portname, int baudrate)
{
    xNetwork* netobj;
    wxString nettype3 = NetworkType.Upper().Left(3);
    if (nettype3 == wxT("LOR"))
    {
        netobj = new xNetwork_LOR();
    }
    else if (nettype3 == wxT("D-L"))
    {
        netobj = new xNetwork_LOR();
    }
    else if (nettype3 == wxT("REN"))
    {
        netobj = new xNetwork_Renard();
    }
    else if (nettype3 == wxT("DMX"))
    {
        netobj = new xNetwork_DMXpro();
    }
    else if (nettype3 == wxT("OPE"))
    {
        netobj = new xNetwork_DMXopen();
    }
    else if (nettype3 == wxT("PIX"))
    {
        netobj = new xNetwork_Pixelnet();
    }
    else if (nettype3 == wxT("E13"))
    {
        netobj = new xNetwork_E131();
    }
    else
    {
        throw "unknown network type";
    }
    size_t netnum = networks.GetCount();
    networks.Add(netobj);
    netobj->SetChannelCount(chcount);
    for (int ch=0; ch<chcount; ch++)
    {
        channels.push_back(std::make_pair(netnum, ch));
    }
    wxString description = NetworkType + wxT(" on ") + portname;
    netobj->SetNetworkDesc(description);
    if (nettype3 == wxT("E13"))
    {
        netobj->InitNetwork(portname, baudrate, (wxUint16) netnum);  // portname is ip address and baudrate is universe number
    }
    else
    {
        netobj->InitSerialPort(portname, baudrate);
    }
    return netnum;
}

int xOutput::GetChannelCount(size_t netnum)
{
    if (netnum >= networks.GetCount()) return 0;
    return networks[netnum]->GetChannelCount();
}

wxString xOutput::GetNetworkDesc(size_t netnum)
{
    if (netnum >= networks.GetCount()) return wxT("");
    return networks[netnum]->GetNetworkDesc();
}

// absChNum starts at 0
// intensity is 0-255
void xOutput::SetIntensity (size_t absChNum, wxByte intensity)
{
    if (absChNum < channels.size())
        networks[channels[absChNum].first]->SetIntensity(channels[absChNum].second, intensity);
}

// convenience function to turn a single channel off
void xOutput::off (size_t absChNum)
{
    if (absChNum < channels.size())
        networks[channels[absChNum].first]->SetIntensity(channels[absChNum].second, 0);
}

// turns all channels off on all networks
void xOutput::alloff ()
{
    for(size_t absChNum=0; absChNum < channels.size(); ++absChNum)
    {
        networks[channels[absChNum].first]->SetIntensity(channels[absChNum].second, 0);
    }
}

// returns total number of channels across all networks
size_t xOutput::TotalChannelCount()
{
    return channels.size();
}

size_t xOutput::AbsChannel2NetNum(size_t absChNum)
{
    return channels[absChNum].first;
}

size_t xOutput::AbsChannel2NetCh(size_t absChNum)
{
    return channels[absChNum].second;
}

ChannelPair xOutput::AbsChannelPair(size_t absChNum)
{
    return channels[absChNum];
}

void xOutput::ResetTimer()
{
    for(size_t i=0; i < networks.GetCount(); ++i)
    {
        networks[i]->ResetTimer();
    }
}

void xOutput::TimerStart(long msec)
{
    xNetwork_E131_changed=false;
    for(size_t i=0; i < networks.GetCount(); ++i)
    {
        networks[i]->TimerStart(msec);
    }
}

void xOutput::TimerEnd()
{
    for(size_t i=0; i < networks.GetCount(); ++i)
    {
        networks[i]->TimerEnd();
    }
}

bool xOutput::TxEmpty()
{
    for(size_t i=0; i < networks.GetCount(); ++i)
    {
        if (!networks[i]->TxEmpty()) return false;
    }
    return true;
}
