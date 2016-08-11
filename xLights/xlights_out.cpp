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
#include "xLightsMain.h"
#include <wx/msgdlg.h> //debug only -DJ
#include <wx/filename.h> //-DJ
#include <vector> //-DJ
#include "serial.h"

// ***********************
// * xNetwork (base class)
// ***********************
xNetwork::xNetwork()
{
    timer_msec=0;
    num_channels=0;
    enabled = true;
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

void xNetwork::InitNetwork(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum, wxUint16 syncuniverse)
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

class xNullNetwork : public xNetwork {
public:
    xNullNetwork() {};
    virtual ~xNullNetwork() {};
    
    virtual bool TxEmpty() {return true;};
    virtual size_t TxNonEmptyCount() {return 0;};
    virtual void SetChannelCount(size_t numchannels) {num_channels = numchannels;};
    virtual void TimerEnd() {}
    virtual void SetIntensity (size_t chindex, wxByte intensity) {}
    
};

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
        if (portname == "NotConnected") return;
        serptr=new SerialPort();
        int errcode=serptr->Open(portname, baudrate, SerialConfig);
        if (errcode < 0)
        {
            sprintf(errmsg,"unable to open serial port, error code=%d",errcode);
            throw errmsg;
        }
    };

    size_t TxNonEmptyCount(void)
    {
        return serptr? serptr->WaitingToWrite(): 0;
    }
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
        if (enabled && changed && serptr)
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
        if (enabled && serptr)
        {
            serptr->SendBreak();  // sends a 1 millisecond break
            wxMilliSleep(1);      // mark after break (MAB) - 1 millisecond is overkill (8 microseconds is the minimum dmx requirement)
            serptr->Write((char *)data,513);
            changed=false;
        }
    };
};


#define E131_SYNCPACKET_LEN 49
#define E131_PORT 5568
#define XLIGHTS_UUID "c0de0080-c69b-11e0-9572-0800200c9a66"

// ******************************************************
// * This class represents a single universe for E1.31
// * Methods should be called with: 0 <= chindex <= 511
// ******************************************************
    void xNetwork_E131::Sync()
    {
        static wxByte syncdata[E131_SYNCPACKET_LEN];
        static wxByte syncSequenceNum = 0;
        static bool initialised = false;
        static wxUint16 _lastsyncuniverse = 0;
        static wxIPV4address syncremoteAddr;
        static wxDatagramSocket *syncdatagram = nullptr;

        if (!initialised)
        {
            log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.debug("Initialising e131 Sync.");

            initialised = true;
            syncdata[0] = 0x00;   // RLP preamble size (high)
            syncdata[1] = 0x10;   // RLP preamble size (low)
            syncdata[2] = 0x00;   // RLP postamble size (high)
            syncdata[3] = 0x00;   // RLP postamble size (low)
            syncdata[4] = 0x41;   // ACN Packet Identifier (12 bytes)
            syncdata[5] = 0x53;
            syncdata[6] = 0x43;
            syncdata[7] = 0x2d;
            syncdata[8] = 0x45;
            syncdata[9] = 0x31;
            syncdata[10] = 0x2e;
            syncdata[11] = 0x31;
            syncdata[12] = 0x37;
            syncdata[13] = 0x00;
            syncdata[14] = 0x00;
            syncdata[15] = 0x00;
            syncdata[16] = 0x70;  // RLP Protocol flags and length (high)
            syncdata[17] = 0x21;  // 0x021 = 49 - 16
            syncdata[18] = 0x00;  // VECTOR_ROOT_E131_EXTENDED 
            syncdata[19] = 0x00;
            syncdata[20] = 0x00;
            syncdata[21] = 0x08;

            // CID/UUID

            wxChar msb, lsb;
            wxString id = XLIGHTS_UUID;
            id.Replace("-", "");
            id.MakeLower();
            if (id.Len() != 32) throw "invalid CID";
            for (int i = 0, j = 22; i < 32; i += 2)
            {
                msb = id.GetChar(i);
                lsb = id.GetChar(i + 1);
                msb -= isdigit(msb) ? 0x30 : 0x57;
                lsb -= isdigit(lsb) ? 0x30 : 0x57;
                syncdata[j++] = (wxByte)((msb << 4) | lsb);
            }

            syncdata[38] = 0x70;  // Framing Protocol flags and length (high)
            syncdata[39] = 0x0b;  // 0x00B = 49 - 38
            syncdata[40] = 0x00;  // VECTOR_E131_EXTENDED_SYNCHRONIZATION 
            syncdata[41] = 0x00;
            syncdata[42] = 0x00;
            syncdata[43] = 0x01;
            syncdata[44] = 0x00;   // sequence number
            syncdata[45] = 0x00;   // sync universe
            syncdata[46] = 0x00;
            syncdata[47] = 0x00;
            syncdata[48] = 0x00;
        }

        if (_syncuniverse > 0)
        {
            if (_syncuniverse != _lastsyncuniverse)
            {
                _lastsyncuniverse = _syncuniverse;
                syncSequenceNum = 0;   // sequence number
                syncdata[45] = _syncuniverse >> 8;
                syncdata[46] = _syncuniverse & 0xff;

                wxIPV4address localaddr;
                localaddr.AnyAddress();

                if (syncdatagram != nullptr)
                {
                    delete syncdatagram;
                    syncdatagram = nullptr;
                }
                syncdatagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);

                // multicast - universe number must be in lower 2 bytes
                wxString ipaddrWithUniv = wxString::Format("%d.%d.%d.%d", 239, 255, syncdata[45], syncdata[46]);
                syncremoteAddr.Hostname(ipaddrWithUniv);
                syncremoteAddr.Service(E131_PORT);

                log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                logger_base.debug("e131 Sync sync universe changed to %d => %s:%d.", _syncuniverse, (const char *)ipaddrWithUniv.c_str(), E131_PORT);
            }
            syncdata[44] = syncSequenceNum++;   // sequence number
            syncdata[45] = _syncuniverse >> 8;
            syncdata[46] = _syncuniverse & 0xff;
            syncdatagram->SendTo(syncremoteAddr, syncdata, E131_SYNCPACKET_LEN);
        }
    }

    void xNetwork_E131::SetIntensity(size_t chindex, wxByte intensity)
    {
        if (data[chindex+126] != intensity) {
            data[chindex+126] = intensity;
            changed=true;
        }
    };

    xNetwork_E131::xNetwork_E131()
    {
        datagram=0;
        memset(data,0,sizeof(data));
        changed = true;
    };

    xNetwork_E131::~xNetwork_E131()
    {
        if (datagram) delete datagram;
    };

    void xNetwork_E131::InitNetwork(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum, wxUint16 syncuniverse)
    {
        if (UniverseNumber == 0 || UniverseNumber >= 64000)
        {
            throw "universe number must be between 1 and 63999";
        }
        InitData(ipaddr, UniverseNumber, NetNum);
        SetSyncUniverse(syncuniverse);
        InitRemoteAddr(ipaddr, UniverseNumber, NetNum);
        SetChannelCount(num_channels);
    }
    void xNetwork_E131::SetSyncUniverseStatic(wxUint16 syncuniverse)
    {
        _syncuniverse = syncuniverse;
    }
    void xNetwork_E131::SetSyncUniverse(wxUint16 syncuniverse)
    {
        _syncuniverse = syncuniverse;
        if (syncuniverse > 0)
        {
            data[109] = syncuniverse >> 8;
            data[110] = syncuniverse & 0xff;
            data[112] |= 0x10;
        }
        else
        {
            data[109] = 0x00;
            data[110] = 0x00;
            data[112] &= 0xef;
        }
    }
    void xNetwork_E131::InitData(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum) {
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
        wxString id = XLIGHTS_UUID;
        id.Replace("-", "");
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
    }
    void xNetwork_E131::InitRemoteAddr(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum) {
        wxByte UnivHi = UniverseNumber >> 8;   // Universe Number (high)
        wxByte UnivLo = UniverseNumber & 0xff; // Universe Number (low)

        wxIPV4address localaddr;
        localaddr.AnyAddress();
        //localaddr.Hostname("192.168.2.100");
        //localaddr.Service(0x8000 | NetNum);
        datagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);
        //datagram = new wxDatagramSocket(localaddr, wxSOCKET_WAITALL);

        if (ipaddr.StartsWith("239.255.") || ipaddr == "MULTICAST")
        {
            // multicast - universe number must be in lower 2 bytes
            wxString ipaddrWithUniv = wxString::Format("%d.%d.%d.%d",239,255,(int)UnivHi,(int)UnivLo);
            remoteAddr.Hostname (ipaddrWithUniv);
        }
        else
        {
            remoteAddr.Hostname (ipaddr);
        }
        remoteAddr.Service (E131_PORT);
    };

    void xNetwork_E131::SetChannelCount(size_t numchannels)
    {
        if (numchannels > 512)
        {
            throw "max channels on E1.31 is 512";
        }
        num_channels=numchannels;

        int i = num_channels + 1;
        wxByte NumHi = i >> 8;   // Channels (high)
        wxByte NumLo = i & 0xff; // Channels (low)

        data[123]=NumHi;  // Property value count (high)
        data[124]=NumLo;  // Property value count (low)

        i = 638 - 16 - (512 - num_channels);
        wxByte hi = i >> 8;   // (high)
        wxByte lo = i & 0xff; // (low)

        data[16]=hi + 0x70;  // RLP Protocol flags and length (high)
        data[17]=lo;  // 0x26e = 638 - 16

        i = 638 - 38 - (512 - num_channels);
        hi = i >> 8;   // (high)
        lo = i & 0xff; // (low)
        data[38]=hi + 0x70;  // Framing Protocol flags and length (high)
        data[39]=lo;  // 0x258 = 638 - 38

        i = 638 - 115 - (512 - num_channels);
        hi = i >> 8;   // (high)
        lo = i & 0xff; // (low)
        data[115]=hi + 0x70;  // DMP Protocol flags and length (high)
        data[116]=lo;  // 0x20b = 638 - 115
    };

    void xNetwork_E131::TimerEnd()
    {
        if (!enabled) {
            return;
        }
        // skipping would cause ECG-DR4 (firmware version 1.30) to timeout
        if (changed || SkipCount > 10)
        {
            data[111]=SequenceNum;
            datagram->SendTo(remoteAddr, data, E131_PACKET_LEN - (512 - num_channels));
            SequenceNum= SequenceNum==255 ? 0 : SequenceNum+1;
            SkipCount=0;
            //changed = false;
        }
        else
        {
            SkipCount++;
        }
    };

    size_t xNetwork_E131::TxNonEmptyCount(void)
    {
        return 0;
    }
    bool xNetwork_E131::TxEmpty()
    {
        return true;
    }

wxUint16 xNetwork_E131::_syncuniverse = 0;

class xMultiE131Network : public xNetwork {
public:
    xMultiE131Network(int univCount) { ucount = univCount;};
    virtual ~xMultiE131Network() {};
    
    virtual bool TxEmpty() {return true;};
    virtual size_t TxNonEmptyCount() {return 0;};
    
    virtual void InitNetwork(const wxString& ipaddr, wxUint16 UniverseNumber, wxUint16 NetNum, wxUint16 syncuniverse) {
        for (int x = 0; x < ucount; x++) {
            xNetwork_E131 *ptr = new xNetwork_E131();
            ptr->SetChannelCount(chanPerNet);
            ptr->InitNetwork(ipaddr, UniverseNumber, NetNum, syncuniverse);
            UniverseNumber++;
            networks.push_back(std::unique_ptr<xNetwork_E131>(ptr));
        }
    }
    
    void SetSyncUniverse(wxUint16 syncuniverse)
    {
        for (auto it = networks.begin(); it != networks.end(); ++it) {
            it->get()->SetSyncUniverse(syncuniverse);
        }
    }

    virtual void SetChannelCount(size_t numchannels) {
        num_channels = ucount * numchannels;
        chanPerNet = numchannels;
    };
    
    virtual void TimerStart(long msec)
    {
        timer_msec=msec;
        for (std::vector<std::unique_ptr<xNetwork_E131>>::iterator it = networks.begin(); it != networks.end(); it++) {
            it->get()->TimerStart(msec);
        }
    }

    virtual void TimerEnd() {
        if (!enabled) {
            return;
        }
        for (std::vector<std::unique_ptr<xNetwork_E131>>::iterator it = networks.begin(); it != networks.end(); it++) {
            it->get()->TimerEnd();
        }
    }
    virtual void SetIntensity (size_t chindex, wxByte intensity) {
        int net = chindex / chanPerNet;
        int ch = chindex % chanPerNet;
        networks[net]->SetIntensity(ch, intensity);
    }
    
    int chanPerNet;
    int ucount;
    std::vector<std::unique_ptr<xNetwork_E131>> networks;
};

// Should be called with: 0 <= chindex <= 1015 (max channels=127*8)
class xNetwork_Renard: public xNetwork_Serial
{
public:
    xNetwork_Renard(): fmtout(0), incoming(0), seqnum(0) //check for plug-in DLL -DJ
    {
        data = std::vector<wxByte>(1024);
#ifdef __WXMSW__ //TODO: generalize this for dynamically loaded output plug-ins on all platforms -DJ
        hPlugin = NULL;
        wxString path;
        { //inner scope to destroy pathbuf
            wxStringBuffer pathbuf(path, MAX_PATH + 1);
//        path.Alloc(MAX_PATH + 1);
//        char* pathbuf = path.GetWriteBuf(MAX_PATH + 1);
            GetModuleFileName(GetModuleHandle(NULL), pathbuf, MAX_PATH); //my exe path
//        path.Truncate(strlen(pathbuf));
//        path.UngetWriteBuf(strlen(pathbuf));
        }
        path = path.BeforeLast(wxFileName::GetPathSeparator());
        path += wxFileName::GetPathSeparator();
        path += "Plugin.dll"; //TODO: enumerate DLLs in a Plugins subfolder; for now, just load one generic name
        SetErrorMode(SEM_FAILCRITICALERRORS); //don't display errors
        if ((hPlugin = LoadLibrary(path.c_str())) != NULL)
        {
            incoming = (plugin_entpt_in)GetProcAddress(hPlugin, "InputHook");
            fmtout = (plugin_entpt_out)GetProcAddress(hPlugin, "FormatOutput");
            plugin_entpt_cfg cfg = (plugin_entpt_cfg)GetProcAddress(hPlugin, "SetConfig");
            wxString cfgdir = xLightsFrame::xlightsFilename;
//            wxMessageBox(_(cfg? "found cfg ": "no cfg ") + _(cfgdir.empty()? "empty ": "!empty ") + xLightsFrame::CurrentDir);
            if (cfgdir.empty()) cfgdir = xLightsFrame::CurrentDir + "\\no_seq-use_folder.txt"; //default to current dir; add dummy file name so force parent dir
            if (cfg) (*cfg)(cfgdir); //use config from active folder
//            byte test[4] = {1,2,3,4};
//            if (incoming) (*incoming)(serptr->m_devname, 0, 0, test, 4);
        }
//        wxMessageBox(wxString::Format("loaded plug-in '%s'? %d, err %d, fo 0x%x, ih 0x%x", path, HasPlugin(), GetLastError(), fmtout, incoming), _("DEBUG"));
//        if (HasPlugin()) wxMessageBox("yes", _("DEBUG"));
//        else  wxMessageBox("no", _("DEBUG"));
#endif
    }
    ~xNetwork_Renard() //unload plug-in -DJ
    {
        fmtout = 0;
        incoming = 0;
#ifdef __WXMSW__ //TODO: generalize this for dynamically loaded output plug-ins on all platforms -DJ
        if (hPlugin != NULL) FreeLibrary(hPlugin);
        hPlugin = NULL;
#endif
    }
protected:
    int seqnum; //useful for debug or to notify plugin of start of sequence

#ifdef __WXMSW__ //TODO: generalize this for dynamically loaded output plug-ins on all platforms -DJ
    HINSTANCE hPlugin;
#endif
    typedef int (*plugin_entpt_cfg)(const char* cfgdir);
    typedef size_t (*plugin_entpt_in)(const char* netname, const char* frameset, int seqnum, const wxByte* inbuf, size_t inlen);
    typedef size_t (*plugin_entpt_out)(const char* netname, const char* frameset, int seqnum, const /*byte*/ void* inbuf, const /*byte*/ void* prev_inbuf, size_t inlen, wxByte* outbuf, size_t maxoutlen);
    plugin_entpt_in incoming;
    plugin_entpt_out fmtout;
//    wxByte data[1024];
    std::vector<wxByte> data; //(1024); //alloc initial size, but allow more channels (dynamic alloc) -DJ
    wxByte iobuf[250000 / (1+8+2) / 20]; //max data size @250k baud, 8N2, 20 fps -DJ
    bool HasPlugin(void) const { return fmtout != 0; } //allow additional formatting before send -DJ

    void SetIntensity (size_t chindex, wxByte intensity)
    {
        wxByte RenIntensity;
        if (HasPlugin()) data[chindex + 2] = intensity; //let plug-in handle esc codes -DJ
        else
        {
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
        }
        changed=true;
    };

public:
    void SetChannelCount(size_t numchannels)
    {
        if (!HasPlugin() && (numchannels > data.size() - 2)) //1016)
        {
            throw wxString::Format("max channels on a Renard network is %d", data.size());
        }
        /*
        // commented out by Matt - per request from lboucher on DIYC, 11/28/2014
        if (!HasPlugin() && ((numchannels % 8) != 0)) //restraint does not apply to plug-ins -DJ
        {
            throw "Number of Renard channels must be a multiple of 8";
        }
        */
        if (HasPlugin()) data.resize(numchannels * 2 + 2); //allow more channels, 2 copies for delta analysis
        datalen=numchannels+2;
        data[0]=0x7E;               // start of message
        data[1]=0x80;               // start address
        changed=false;
        num_channels=numchannels;
        SerialConfig[2]='2'; // use 2 stop bits so padding chars are not required
    };

    void TimerEnd()
    {
        if (!enabled) {
            return;
        }

        if (changed && serptr)
        {
            if (HasPlugin()) //call plug-in to process data before sending -DJ
            {
                int iolen = (*fmtout)(serptr->m_devname.c_str(), xLightsFrame::PlaybackMarker.c_str(), seqnum++, &data[2], &data[datalen], datalen - 2, iobuf, sizeof(iobuf)); //don't pre-fill first 2 bytes; plug-in might not need them; provide prev data in case plug-in needs to look back
//                if (seqnum < 5) wxMessageBox(wxString::Format("called plug-in: in %d -> out %d, got back 0x%x", datalen - 2, sizeof(iobuf), iolen), _("DEBUG"));
                if (iolen > 0) serptr->Write((char*)iobuf, iolen);
            }
            else serptr->Write((char *)&data[0],datalen);
            changed=false;
        }
        if (serptr && HasPlugin() && incoming) //check for incoming data (trigger or data echo) -DJ
        {
//            static int fido = 0;
////            int rdlen = serptr->AvailableToRead();
//            usleep(1000); //TODO: allow a little time to receive data?  OTOH, don't want to slow down xLights
            int rdlen = serptr->Read((char*)iobuf, sizeof(iobuf)); //NOTE: might be split up
//            if ((rdlen > 0) && (fido++ < 5)) wxMessageBox(wxString::Format("plugin: got %d bytes back, sent? %d", rdlen, incoming? 1: 0), _("DEBUG"));
            if (rdlen > 0) (*incoming)(serptr->m_devname.c_str(), xLightsFrame::PlaybackMarker.c_str(), seqnum, iobuf, rdlen);
        }
    }
};



// DIYLightAnimation.com Lynx USB dongle with Pixenet firmware
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
        if (!enabled) {
            return;
        }

        if (serptr && serptr->WaitingToWrite()==0)
        {
            memcpy(&SerialBuffer[1],data,sizeof(data));
            SerialBuffer[0]=170;    // start of message
            serptr->Write((char *)SerialBuffer,sizeof(SerialBuffer));
        }
    };
};


// Generic FTDI-based USB to RS485 dongle transmitting Pixelnet
class xNetwork_PixelnetOpen: public xNetwork_Serial
{
protected:
    wxByte data[4096];
    wxByte SerialBuffer[4104];

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

        SerialConfig[2]='2';

        SerialBuffer[0] = 0xAA;
        SerialBuffer[1] = 0x55;
        SerialBuffer[2] = 0x55;
        SerialBuffer[3] = 0xAA;
        SerialBuffer[4] = 0x15;
        SerialBuffer[5] = 0x5D;
    };

    void TimerEnd()
    {
        if (!enabled) {
            return;
        }

        if (serptr && serptr->WaitingToWrite()==0)
        {
            memcpy(&SerialBuffer[6],data,sizeof(data));
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
        if (!enabled) {
            return;
        }

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
xOutput::xOutput(wxUint16 syncuniverse)
{
    _syncuniverse = syncuniverse;
}

xOutput::~xOutput()
{
    WX_CLEAR_ARRAY(networks);
}

size_t xOutput::NetworkCount()
{
    return networks.GetCount();
}

void xOutput::EnableOutput(size_t network, bool en) {
    if (network < networks.GetCount()) {
        networks[network]->SetEnabled(en);
    }
}

// returns the network index
size_t xOutput::addnetwork(const wxString& NetworkType, int chcount, const wxString& portname,
                           int baudrate, int univCount, bool enabled)
{
    xNetwork* netobj;
    wxString nettype3 = NetworkType.Upper().Left(3);
    if (nettype3 == "LOR")
    {
        netobj = new xNetwork_LOR();
    }
    else if (nettype3 == "D-L")
    {
        netobj = new xNetwork_LOR();
    }
    else if (nettype3 == "REN")
    {
        netobj = new xNetwork_Renard();
    }
    else if (nettype3 == "DMX")
    {
        netobj = new xNetwork_DMXpro();
    }
    else if (nettype3 == "OPE")
    {
        netobj = new xNetwork_DMXopen();
    }
    else if (nettype3 == "PIX")
    {
        if (NetworkType == "Pixelnet-Open")
            netobj = new xNetwork_PixelnetOpen();
        else
            netobj = new xNetwork_Pixelnet();
    }
    else if (nettype3 == "E13")
    {
        if (univCount > 1) {
            netobj = new xMultiE131Network(univCount);
        } else {
            netobj = new xNetwork_E131();
        }
    }
    else if (nettype3 == "NUL")
    {
        netobj = new xNullNetwork();
    }
    else
    {
        throw "unknown network type";
    }
    size_t netnum = networks.GetCount();
    networks.Add(netobj);
    netobj->SetChannelCount(chcount);
    wxString description = NetworkType + " on " + portname;
    netobj->SetNetworkDesc(description);
    if (nettype3 == "E13")
    {
        netobj->InitNetwork(portname, baudrate, (wxUint16) netnum, _syncuniverse);  // portname is ip address and baudrate is universe number
    }
    else
    {
        netobj->InitSerialPort(portname, baudrate);
    }
    chcount = netobj->GetChannelCount();
    for (int ch=0; ch<chcount; ch++)
    {
        channels.push_back(std::make_pair(netnum, ch));
    }
    netobj->SetEnabled(enabled);
    return netnum;
}

int xOutput::GetChannelCount(size_t netnum)
{
    if (netnum >= networks.GetCount()) return 0;
    return networks[netnum]->GetChannelCount();
}

wxString xOutput::GetNetworkDesc(size_t netnum)
{
    if (netnum >= networks.GetCount()) return "";
    return networks[netnum]->GetNetworkDesc();
}

// absChNum starts at 0
// intensity is 0-255
void xOutput::SetIntensity (size_t absChNum, wxByte intensity)
{
    if (absChNum < channels.size())
        networks[channels[absChNum].first]->SetIntensity(channels[absChNum].second, intensity);
}

void xOutput::SetIntensities(size_t startChannel, unsigned char *buffer, size_t count) {
    for (size_t x = 0; x < count; x++) {
        SetIntensity(startChannel + x, buffer[x]);
    }
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
    for(size_t i=0; i < networks.GetCount(); ++i)
    {
        networks[i]->TimerStart(msec);
    }
}

void xOutput::SetSyncUniverse(wxUint16 syncuniverse)
{
    _syncuniverse = syncuniverse;
    for (size_t i = 0; i < networks.GetCount(); ++i)
    {
        if (xNetwork_E131* n = dynamic_cast<xNetwork_E131*>(networks[i]))
        {
            n->SetSyncUniverse(syncuniverse);
        }
        else if (xMultiE131Network* n = dynamic_cast<xMultiE131Network*>(networks[i]))
        {
            n->SetSyncUniverse(syncuniverse);
        }
    }
};

void xOutput::TimerEnd()
{
    for(size_t i=0; i < networks.GetCount(); ++i)
    {
        networks[i]->TimerEnd();
    }

    // send e131 sync if required
    if (_syncuniverse > 0)
    {
        for (size_t i = 0; i < networks.GetCount(); ++i)
        {
            if (dynamic_cast<xNetwork_E131*>(networks[i]) || dynamic_cast <xMultiE131Network*> (networks[i]))
            {
                xNetwork_E131::Sync();
                break;
            }
        }
    }
}

//tell how much: -DJ
size_t xOutput::TxNonEmptyCount(void)
{
    size_t total = 0;
    for(size_t i=0; i < networks.GetCount(); ++i)
    {
        total += networks[i]->TxNonEmptyCount();
    }
    return total;
}

bool xOutput::TxEmpty()
{
    for(size_t i=0; i < networks.GetCount(); ++i)
    {
        if (!networks[i]->TxEmpty()) return false;
    }
    return true;
}



