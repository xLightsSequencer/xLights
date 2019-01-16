#include "E131Output.h"

#include <wx/xml/xml.h>
#include <wx/process.h>

#include <log4cpp/Category.hh>
#include "E131Dialog.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"

#pragma region Constructors and Destructors
E131Output::E131Output(wxXmlNode* node) : IPOutput(node)
{
    _numUniverses = wxAtoi(node->GetAttribute("NumUniverses", "1"));
    CreateMultiUniverses(_numUniverses);
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

E131Output::E131Output() : IPOutput()
{
    _channels = 510;
    _universe = 1;
    _sequenceNum = 0;
    _numUniverses = 1;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

E131Output::~E131Output()
{
    if (_datagram != nullptr) delete _datagram;
    for (auto i : _outputs) {
        delete i;
    }
}
#pragma endregion Constructors and Destructors

wxXmlNode* E131Output::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    IPOutput::Save(node);

    if (_numUniverses > 1)
    {
        node->AddAttribute("NumUniverses", wxString::Format(wxT("%i"), _numUniverses));
    }

    return node;
}

void E131Output::CreateMultiUniverses(int num)
{
    _numUniverses = num;
    
    for (auto i : _outputs) {
        delete i;
    }
    
    _outputs.clear();

    for (int i = 0; i < _numUniverses; i++)
    {
        E131Output* e = new E131Output();
        e->SetIP(_ip);
        e->SetUniverse(_universe + i);
        e->SetChannels(_channels);
        e->SetDescription(_description);
        e->SetSuppressDuplicateFrames(_suppressDuplicateFrames);
        _outputs.push_back(e);
    }
}

#pragma region Static Functions
void E131Output::SendSync(int syncUniverse)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static uint8_t syncdata[E131_SYNCPACKET_LEN];
    static uint8_t syncSequenceNum = 0;
    static bool initialised = false;
    static wxUint16 _lastsyncuniverse = 0;
    static wxIPV4address syncremoteAddr;
    static wxDatagramSocket *syncdatagram = nullptr;

    if (!initialised)
    {
        logger_base.debug("Initialising e131 Sync.");

        initialised = true;

        memset(syncdata, 0x00, sizeof(syncdata));

        syncdata[1] = 0x10;   // RLP preamble size (low)
        syncdata[4] = 0x41;   // ACN Packet Identifier (12 bytes)
        syncdata[5] = 0x53;
        syncdata[6] = 0x43;
        syncdata[7] = 0x2d;
        syncdata[8] = 0x45;
        syncdata[9] = 0x31;
        syncdata[10] = 0x2e;
        syncdata[11] = 0x31;
        syncdata[12] = 0x37;
        syncdata[16] = 0x70;  // RLP Protocol flags and length (high)
        syncdata[17] = 0x21;  // 0x021 = 49 - 16
        syncdata[21] = 0x08;

        wxString id = XLIGHTS_UUID;
        id.Replace("-", "");
        id.MakeLower();
        if (id.Len() != 32) throw "invalid CID";
        for (int i = 0, j = 22; i < 32; i += 2)
        {
            wxChar msb = id.GetChar(i);
            wxChar lsb = id.GetChar(i + 1);
            msb -= isdigit(msb) ? 0x30 : 0x57;
            lsb -= isdigit(lsb) ? 0x30 : 0x57;
            syncdata[j++] = (uint8_t)((msb << 4) | lsb);
        }

        syncdata[38] = 0x70;  // Framing Protocol flags and length (high)
        syncdata[39] = 0x0b;  // 0x00B = 49 - 38
        syncdata[43] = 0x01;
    }

    if (syncUniverse > 0)
    {
        if (syncUniverse != _lastsyncuniverse)
        {
            _lastsyncuniverse = syncUniverse;
            syncSequenceNum = 0;   // sequence number
            syncdata[45] = syncUniverse >> 8;
            syncdata[46] = syncUniverse & 0xff;

            wxIPV4address localaddr;
            if (IPOutput::__localIP == "")
            {
                localaddr.AnyAddress();
            }
            else
            {
                localaddr.Hostname(IPOutput::__localIP);
            }

            if (syncdatagram != nullptr)
            {
                delete syncdatagram;
            }

            syncdatagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);

            if (syncdatagram == nullptr)
            {
                logger_base.error("Error initialising E131 sync datagram. %s", (const char *)localaddr.IPAddress().c_str());
            }
            else if (!syncdatagram->IsOk())
            {
                logger_base.error("Error initialising E131 sync datagram ... is network connected? OK : FALSE %s", (const char *)localaddr.IPAddress().c_str());
                delete syncdatagram;
                syncdatagram = nullptr;
            }
            else if (syncdatagram->Error() != wxSOCKET_NOERROR)
            {
                logger_base.error("Error creating E131 sync datagram => %d : %s. %s", syncdatagram->LastError(), (const char *)DecodeIPError(syncdatagram->LastError()).c_str(), (const char *)localaddr.IPAddress().c_str());
                delete syncdatagram;
                syncdatagram = nullptr;
            }

            // multicast - universe number must be in lower 2 bytes
            wxString ipaddrWithUniv = wxString::Format("%d.%d.%d.%d", 239, 255, syncdata[45], syncdata[46]);
            syncremoteAddr.Hostname(ipaddrWithUniv);
            syncremoteAddr.Service(E131_PORT);

            logger_base.debug("e131 Sync sync universe changed to %d => %s:%d.", syncUniverse, (const char *)ipaddrWithUniv.c_str(), E131_PORT);
        }

        syncdata[44] = syncSequenceNum++;   // sequence number
        syncdata[45] = syncUniverse >> 8;
        syncdata[46] = syncUniverse & 0xff;

        // bail if we dont have a datagram to use
        if (syncdatagram != nullptr)
        {
            syncdatagram->SendTo(syncremoteAddr, syncdata, E131_SYNCPACKET_LEN);
        }
    }
}

std::string E131Output::GetTag()
{
    // creates a unique tag per running instance of xLights on this machine
    return "xLights " + wxString::Format("%ld", wxGetProcessId());
}
#pragma endregion Static Functions

#pragma region Start and Stop
void E131Output::OpenDatagram()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_datagram != nullptr) return;

    wxIPV4address localaddr;
    if (IPOutput::__localIP == "")
    {
        localaddr.AnyAddress();
    }
    else
    {
        localaddr.Hostname(IPOutput::__localIP);
    }

    _datagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);
    if (_datagram == nullptr)
    {
        logger_base.error("E131Output: %s Error opening datagram.", (const char *)localaddr.IPAddress().c_str());
    }
    else if (!_datagram->IsOk())
    {
        logger_base.error("E131Output: %s Error opening datagram. Network may not be connected? OK : FALSE", (const char *)localaddr.IPAddress().c_str());
        delete _datagram;
        _datagram = nullptr;
    }
    else if (_datagram->Error() != wxSOCKET_NOERROR)
    {
        logger_base.error("E131Output: %s Error creating E131 datagram => %d : %s.", (const char *)localaddr.IPAddress().c_str(), _datagram->LastError(), (const char *)DecodeIPError(_datagram->LastError()).c_str());
        delete _datagram;
        _datagram = nullptr;
    }
}

bool E131Output::Open()
{
    if (!_enabled) return true;

    if (IsOutputCollection())
    {
        if (IsOutputCollection())
        {
            for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
            {
                _ok = (*it)->Open() && _ok;
            }
        }
        return _ok;
    }
    else
    {
        _ok = IPOutput::Open();

        memset(_data, 0x00, sizeof(_data));
        _sequenceNum = 0;
        uint8_t UnivHi = _universe >> 8;   // Universe Number (high)
        uint8_t UnivLo = _universe & 0xff; // Universe Number (low)

        _data[1] = 0x10;   // RLP preamble size (low)
        _data[4] = 0x41;   // ACN Packet Identifier (12 bytes)
        _data[5] = 0x53;
        _data[6] = 0x43;
        _data[7] = 0x2d;
        _data[8] = 0x45;
        _data[9] = 0x31;
        _data[10] = 0x2e;
        _data[11] = 0x31;
        _data[12] = 0x37;
        _data[16] = 0x72;  // RLP Protocol flags and length (high)
        _data[17] = 0x6e;  // 0x26e = 638 - 16
        _data[21] = 0x04;

        // CID/UUID

        wxString id = XLIGHTS_UUID;
        id.Replace("-", "");
        id.MakeLower();
        if (id.Len() != 32) throw "invalid CID";
        for (int i = 0, j = 22; i < 32; i += 2)
        {
            wxChar msb = id.GetChar(i);
            wxChar lsb = id.GetChar(i + 1);
            msb -= isdigit(msb) ? 0x30 : 0x57;
            lsb -= isdigit(lsb) ? 0x30 : 0x57;
            _data[j++] = (uint8_t)((msb << 4) | lsb);
        }

        _data[38] = 0x72;  // Framing Protocol flags and length (high)
        _data[39] = 0x58;  // 0x258 = 638 - 38
        _data[43] = 0x02;
        // Source Name (64 bytes)
        strcpy((char*)&_data[44], GetTag().c_str());
        _data[108] = 100;  // Priority
        _data[113] = UnivHi;  // Universe Number (high)
        _data[114] = UnivLo;  // Universe Number (low)
        _data[115] = 0x72;  // DMP Protocol flags and length (high)
        _data[116] = 0x0b;  // 0x20b = 638 - 115
        _data[117] = 0x02;  // DMP Vector (Identifies DMP Set Property Message PDU)
        _data[118] = 0xa1;  // DMP Address Type & Data Type
        _data[122] = 0x01;  // Address Increment (low)
        _data[123] = 0x02;  // Property value count (high)
        _data[124] = 0x01;  // Property value count (low)

        OpenDatagram();

        if (wxString(_ip).StartsWith("239.255.") || _ip == "MULTICAST")
        {
            // multicast - universe number must be in lower 2 bytes
            wxString ipaddrWithUniv = wxString::Format("%d.%d.%d.%d", 239, 255, (int)UnivHi, (int)UnivLo);
            _remoteAddr.Hostname(ipaddrWithUniv);
        }
        else
        {
            _remoteAddr.Hostname(_ip.c_str());
        }
        _remoteAddr.Service(E131_PORT);

        int i = _channels;
        uint8_t NumHi = (_channels + 1) >> 8;   // Channels (high)
        uint8_t NumLo = (_channels + 1) & 0xff; // Channels (low)

        _data[123] = NumHi;  // Property value count (high)
        _data[124] = NumLo;  // Property value count (low)

        i = E131_PACKET_LEN - 16 - (512 - _channels);
        uint8_t hi = i >> 8;   // (high)
        uint8_t lo = i & 0xff; // (low)

        _data[16] = hi + 0x70;  // RLP Protocol flags and length (high)
        _data[17] = lo;  // 0x26e = E131_PACKET_LEN - 16

        i = E131_PACKET_LEN - 38 - (512 - _channels);
        hi = i >> 8;   // (high)
        lo = i & 0xff; // (low)
        _data[38] = hi + 0x70;  // Framing Protocol flags and length (high)
        _data[39] = lo;  // 0x258 = E131_PACKET_LEN - 38

        i = E131_PACKET_LEN - 115 - (512 - _channels);
        hi = i >> 8;   // (high)
        lo = i & 0xff; // (low)
        _data[115] = hi + 0x70;  // DMP Protocol flags and length (high)
        _data[116] = lo;  // 0x20b = E131_PACKET_LEN - 115
    }

    return _ok && _datagram != nullptr;
}

void E131Output::Close()
{
    if (IsOutputCollection())
    {
        for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
        {
            (*it)->Close();
        }
    }
    else
    {
        if (_datagram != nullptr)
        {
            delete _datagram;
            _datagram = nullptr;
        }
    }
}
#pragma endregion Start and Stop

void E131Output::SetTransientData(int on, long startChannel, int nullnumber)
{
    if (IsOutputCollection())
    {
        _outputNumber = on;
        _startChannel = startChannel;
        if (nullnumber > 0) _nullNumber = nullnumber;
        long nextstartchannel = startChannel;
        for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
        {
            (*it)->SetTransientData(on, nextstartchannel, nullnumber);
            nextstartchannel += _channels;
        }
    }
    else
    {
        _outputNumber = on;
        _startChannel = startChannel;
        if (nullnumber > 0) _nullNumber = nullnumber;
    }
}

#pragma region Frame Handling
void E131Output::StartFrame(long msec)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_enabled) return;

    if (IsOutputCollection())
    {
        for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
        {
            (*it)->StartFrame(msec);
        }
    }
    else
    {
        if (_datagram == nullptr && OutputManager::IsRetryOpen())
        {
            OpenDatagram();
            if (_ok)
            {
                logger_base.debug("E131Output: Open retry successful");
            }
        }

        _timer_msec = msec;
    }
}

void E131Output::EndFrame(int suppressFrames)
{
    if (!_enabled || _suspend) return;

    if (IsOutputCollection())
    {
        for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
        {
            (*it)->EndFrame(suppressFrames);
        }
    }
    else
    {
        if (_datagram == nullptr) return;

        if (_changed || NeedToOutput(suppressFrames))
        {
            _data[111] = _sequenceNum;
            _datagram->SendTo(_remoteAddr, _data, E131_PACKET_LEN - (512 - _channels));
            _sequenceNum = _sequenceNum == 255 ? 0 : _sequenceNum + 1;
            FrameOutput();
        }
        else
        {
            SkipFrame();
        }
    }
}

void E131Output::ResetFrame()
{
    if (!_enabled) return;

    if (IsOutputCollection())
    {
        for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
        {
            (*it)->ResetFrame();
        }
    }
}
#pragma endregion Frame Handling

#pragma region Data Setting
void E131Output::SetOneChannel(long channel, unsigned char data)
{
    if (!_enabled) return;

    if (IsOutputCollection())
    {
        long unum = channel / _channels;
        auto it = _outputs.begin();
        for (long i = 0; i < unum && it != _outputs.end(); i++)
        {
            ++it;
        }

        if (it != _outputs.end())
        {
            (*it)->SetOneChannel(channel % _channels, data);
        }
    }
    else
    {
        if (_data[channel + E131_PACKET_HEADERLEN] != data) {
            _data[channel + E131_PACKET_HEADERLEN] = data;
            _changed = true;
        }
    }
}

void E131Output::SetManyChannels(long channel, unsigned char data[], long size)
{
    if (IsOutputCollection())
    {
        long startu = (channel) / _channels;
        long startc = (channel) % _channels;

        auto o = _outputs.begin();
        for (long i = 0; i < startu; i++)
        {
            ++o;
        }

        long left = size;
        while (left > 0 && o != _outputs.end())
        {
#ifdef _MSC_VER
            long send = min(left, _channels);
#else
            long send = std::min(left, _channels);
#endif
            (*o)->SetManyChannels(startc, &data[size - left], send);
            left -= send;
            ++o;
            startc = 0;
        }
    }
    else
    {
#ifdef _MSC_VER
        long chs = min(size, _channels - channel);
#else
        long chs = std::min(size, GetMaxChannels() - channel);
#endif

        if (memcmp(&_data[channel + E131_PACKET_HEADERLEN], data, chs) == 0)
        {
            // nothing changed
        }
        else
        {
            memcpy(&_data[channel + E131_PACKET_HEADERLEN], data, chs);
            _changed = true;
        }
    }
}

void E131Output::AllOff()
{
    if (IsOutputCollection())
    {
        for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
        {
            (*it)->AllOff();
        }
    }
    else
    {
        memset(&_data[E131_PACKET_HEADERLEN], 0x00, _channels);
        _changed = true;
    }
}
#pragma endregion Data Setting

#pragma region Getters and Setters
long E131Output::GetEndChannel() const
{
    if (IsOutputCollection())
    {
        return _outputs.back()->GetEndChannel();
    }
    else
    {
        return _startChannel + _channels - 1;
    }
}

Output* E131Output::GetActualOutput(long startChannel)
{
    if (!IsOutputCollection())
    {
        return this;
    }

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if ((*it)->GetStartChannel() <= startChannel && (*it)->GetEndChannel() >= startChannel)
        {
            return *it;
        }
    }

    return this;
}

std::string E131Output::GetLongDescription() const
{
    std::string res = "";

    if (IsOutputCollection())
    {
        res = "Multi E131";
    }
    else
    {
        if (!_enabled) res += "INACTIVE ";
        res += "E1.31 " + _ip + " {" + wxString::Format(wxT("%i"), _universe).ToStdString() + "} ";
        res += "[1-" + std::string(wxString::Format(wxT("%li"), (long)_channels)) + "] ";
        res += "(" + std::string(wxString::Format(wxT("%li"), (long)GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%li"), (long)GetActualEndChannel())) + ") ";
        res += _description;
    }

    return res;
}

std::string E131Output::GetChannelMapping(long ch) const
{
    std::string res = "";

    if (IsOutputCollection())
    {
        long unum = (ch - GetStartChannel()) / _channels;

        auto o = _outputs.begin();
        for (long i = 0; i < unum; i++)
        {
            ++o;
        }

        res = (*o)->GetChannelMapping(ch);
    }
    else
    {
        res = "Channel " + std::string(wxString::Format(wxT("%li"), ch)) + " maps to ...\n";

        res += "Type: E1.31\n";
        // int u = _universe;
        long channeloffset = ch - GetStartChannel() + 1;
        if (_numUniverses > 1)
        {
            // u += (ch - GetStartChannel()) / _channels;
            channeloffset -= (ch - GetStartChannel()) / _channels * _channels;
        }
        res += "IP: " + _ip + "\n";
        res += "Universe: " + GetUniverseString() + "\n";
        res += "Channel: " + std::string(wxString::Format(wxT("%li"), channeloffset)) + "\n";

        if (!_enabled) res += " INACTIVE";
    }
        return res;
}

std::string E131Output::GetUniverseString() const
{
    if (IsOutputCollection())
    {
        return wxString::Format(wxT("%i-%i"), _outputs.front()->GetUniverse(), _outputs.back()->GetUniverse()).ToStdString();
    }
    else
    {
        return IPOutput::GetUniverseString();
    }
}
#pragma endregion Getters and Setters

#pragma region UI
#ifndef EXCLUDENETWORKUI
Output* E131Output::Configure(wxWindow* parent, OutputManager* outputManager)
{
    E131Dialog dlg(parent, this, outputManager);

    int res = dlg.ShowModal();

    if (res == wxID_CANCEL)
    {
        return nullptr;
    }

    return this;
}
#endif
#pragma endregion UI
