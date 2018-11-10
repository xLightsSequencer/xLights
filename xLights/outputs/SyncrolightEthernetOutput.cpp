#include "SyncrolightEthernetOutput.h"

#include <wx/xml/xml.h>
#include <wx/process.h>

#include <log4cpp/Category.hh>
#ifndef EXCLUDENETWORKUI
#include "SyncrolightDialog.h"
#endif
#include "OutputManager.h"
#include "../UtilFunctions.h"

#pragma region Constructors and Destructors
SyncrolightEthernetOutput::SyncrolightEthernetOutput(wxXmlNode* node) : IPOutput(node) {
    _universe = 64001;
    _port = wxAtoi(node->GetAttribute("Port", "1"));
    SetId(wxAtoi(node->GetAttribute("Id", "0")));
    _data = (uint8_t*)malloc(_channels + SYNCROLIGHTETHERNET_PACKET_HEADERLEN + SYNCROLIGHTETHERNET_PACKET_FOOTERLEN);
    memset(_data, 0, _channels + SYNCROLIGHTETHERNET_PACKET_HEADERLEN + SYNCROLIGHTETHERNET_PACKET_FOOTERLEN);
}

SyncrolightEthernetOutput::SyncrolightEthernetOutput() : IPOutput() {
    _universe = 64001;
    _data = (uint8_t*)malloc(_channels + SYNCROLIGHTETHERNET_PACKET_HEADERLEN + SYNCROLIGHTETHERNET_PACKET_FOOTERLEN);
    memset(_data, 0, _channels + SYNCROLIGHTETHERNET_PACKET_HEADERLEN + SYNCROLIGHTETHERNET_PACKET_FOOTERLEN);
}

SyncrolightEthernetOutput::~SyncrolightEthernetOutput() {
    if (_data != nullptr)
    {
        free(_data);
        _data = nullptr;
    }
}

SyncrolightEthernetOutput::SyncrolightEthernetOutput(SyncrolightEthernetOutput* output) : IPOutput(output) {
    _port = output->_port;
    _data = (uint8_t*)malloc(_channels);
    memset(_data, 0, _channels);
}
#pragma endregion Constructors and Destructors

std::string SyncrolightEthernetOutput::GetExport() const {
    std::string enabled = _enabled ? _("Y") : _("N");
    std::string suppress = _suppressDuplicateFrames ? _("Y") : _("N");

    // "Output Number,Start Absolute,End Absolute,Type,IP,Multicast,Universe/Id,Comm Port,Baud Rate,Description,Channels,Active,Suppress Duplicates,Auto Size,
    // FPP Proxy,Keep Channel Numbers,Channels Per Packet,Port,Dont Configure,Priority,Vendor,Model,Supports Virtual Strings,Supports Smart Remotes";
    return wxString::Format("%d,%ld,%ld,%s,%s,,,,,%s,%i,%s,%s,,,,,%i,,,,,,",
        _outputNumber, GetStartChannel(), GetEndChannel(), GetType(), GetIP(), _description, _channels,
        enabled, suppress, _port).ToStdString();
}

wxXmlNode* SyncrolightEthernetOutput::Save() {
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");

    node->AddAttribute("Id", wxString::Format(wxT("%i"), GetId()));
    node->AddAttribute("Port",wxString::Format(wxT("%i"), _port));
    
    IPOutput::Save(node);

    return node;
}

#pragma region Start and Stop
void SyncrolightEthernetOutput::OpenDatagram() {
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
        logger_base.error("SyncrolightEthernetOutput: %s Error opening datagram.", (const char *)localaddr.IPAddress().c_str());
    }
    else if (!_datagram->IsOk())
    {
        logger_base.error("SyncrolightEthernetOutput: %s Error opening datagram. Network may not be connected? OK : FALSE", (const char *)localaddr.IPAddress().c_str());
        delete _datagram;
        _datagram = nullptr;
    }
    else if (_datagram->Error() != wxSOCKET_NOERROR)
    {
        logger_base.error("SyncrolightEthernetOutput: %s Error creating SyncrolightEthernet datagram => %d : %s.", (const char *)localaddr.IPAddress().c_str(), _datagram->LastError(), (const char *)DecodeIPError(_datagram->LastError()).c_str());
        delete _datagram;
        _datagram = nullptr;
    }
}

bool SyncrolightEthernetOutput::Open() {
    if (!_enabled) return true;

    _ok = IPOutput::Open();

    memset(_packet, 0, sizeof(_packet));

    OpenDatagram();
    _remoteAddr.Hostname(_ip.c_str());
    _remoteAddr.Service(SYNCROLIGHT_PORT);

    Heartbeat(0);

    return _ok && _datagram != nullptr;
}

void SyncrolightEthernetOutput::Close() {
    Heartbeat(9);
    if (_datagram != nullptr)
    {
        delete _datagram;
        _datagram = nullptr;
    }
    IPOutput::Close();
}
#pragma endregion Start and Stop

#pragma region Frame Handling
void SyncrolightEthernetOutput::StartFrame(long msec) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_enabled) return;

    if (_datagram == nullptr && OutputManager::IsRetryOpen())
    {
        OpenDatagram();
        if (_ok)
        {
            logger_base.debug("SyncrolightEthernetOutput: Open retry successful");
        }
    }

    _timer_msec = msec;
}

void SyncrolightEthernetOutput::Heartbeat(int mode) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static wxLongLong __lastTime = 0;
    static wxIPV4address __remoteAddr;
    static uint8_t __pkt[] = { 0x80, 0x01, 0x00, 0x81 };
    static wxDatagramSocket* __datagram = nullptr;

    if (mode == 1)
    {
        // output
        if (__datagram == nullptr) return;

        wxLongLong now = wxGetUTCTimeMillis();
        if (__lastTime + SYNCROLIGHT_HEARTBEATINTERVAL < now)
        {
            __datagram->SendTo(__remoteAddr, __pkt, sizeof(__pkt));
            __lastTime = now;
        }
    }
    else if (mode == 0)
    {
        // initialise
        if (__datagram != nullptr) return;

        wxIPV4address localaddr;
        if (IPOutput::__localIP == "")
        {
            localaddr.AnyAddress();
        }
        else
        {
            localaddr.Hostname(IPOutput::__localIP.c_str());
        }
        __remoteAddr.Hostname("224.0.0.0");
        __remoteAddr.Service(SYNCROLIGHT_PORT);

        __datagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);

        if (__datagram != nullptr)
        {
            if (!__datagram->IsOk() || __datagram->Error() != wxSOCKET_NOERROR)
            {
                logger_base.error("SyncrolightEthernetOutput: %s Error creating SyncrolightEthernet heartbeat datagram => %d : %s.",
                    (const char*)localaddr.IPAddress().c_str(),
                    __datagram->LastError(),
                    (const char*)DecodeIPError(__datagram->LastError()).c_str());
                delete __datagram;
                __datagram = nullptr;
            }
        }
        else
        {
            logger_base.error("SyncrolightEthernetOutput: %s Error creating SyncrolightEthernet heartbeat datagram.",
                (const char*)localaddr.IPAddress().c_str());
        }
    }
    else if (mode == 9)
    {
        // close
        if (__datagram == nullptr) return;
        __datagram->Close();
        delete __datagram;
        __datagram = nullptr;
    }
}

void SyncrolightEthernetOutput::EndFrame(int suppressFrames) {
    if (!_enabled || _suspend) return;

        if (_datagram == nullptr) return;

        if (_changed || NeedToOutput(suppressFrames))
        {
            int current = 0;
            for (int i = 0; i < (_channels - 1) / 900 + 1; i++)
            {
                _packet[0] = 0x80;
                _packet[1] = _port;
                _packet[2] = 0x1c;
                _packet[3] = (uint8_t)(((current / 3) >> 8) & 0xFF); // high start pixel
                _packet[4] = (uint8_t)((current / 3) & 0xFF); // low start pixel
                int ch = (std::min)((int)_channels - current, 900);
                _packet[5] = (uint8_t)((ch >> 8) & 0xFF); // high pixels per packet
                _packet[6] = (uint8_t)(ch & 0xFF); // low pixels per packet
                memcpy(&_packet[SYNCROLIGHTETHERNET_PACKET_HEADERLEN], &_data[current], ch);
                _packet[SYNCROLIGHTETHERNET_PACKET_HEADERLEN + ch] = 0x81;
                _datagram->SendTo(_remoteAddr, _packet, SYNCROLIGHTETHERNET_PACKET_HEADERLEN + ch + SYNCROLIGHTETHERNET_PACKET_FOOTERLEN);
                current += 900;
            }
            FrameOutput();
            Heartbeat(1);
        }
        else
        {
            SkipFrame();
        }
}
#pragma endregion Frame Handling

#pragma region Data Setting
void SyncrolightEthernetOutput::SetOneChannel(int32_t channel, unsigned char data)
{
    if (!_enabled) return;

        if (_data[channel] != data) {
            _data[channel] = data;
            _changed = true;
        }
}

void SyncrolightEthernetOutput::SetManyChannels(int32_t channel, unsigned char data[], size_t size) {
    size_t chs = (std::min)(size, (size_t)(GetMaxChannels() - channel));
    if (memcmp(&_data[channel], data, chs) == 0) {
        // nothing changed
    } else {
        memcpy(&_data[channel], data, chs);
        _changed = true;
    }
}

void SyncrolightEthernetOutput::AllOff()
{
    memset(_data, 0x00, _channels);
    _changed = true;
}
#pragma endregion Data Setting

#pragma region Getters and Setters
bool SyncrolightEthernetOutput::IsLookedUpByControllerName() const {
    return GetDescription() != "";
}

std::string SyncrolightEthernetOutput::GetLongDescription() const
{
    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "Syncrolight Ethernet " + _ip + " {" + wxString::Format(wxT("%i"), _port).ToStdString() + "} ";
    res += "[1-" + std::string(wxString::Format(wxT("%i"), _channels)) + "] ";
    res += "(" + std::string(wxString::Format(wxT("%i"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), GetActualEndChannel())) + ") ";
    res += _description;

    return res;
}

std::string SyncrolightEthernetOutput::GetChannelMapping(int32_t ch) const
{
    std::string res = "";

    res = "Channel " + std::string(wxString::Format(wxT("%i"), ch)) + " maps to ...\n";

    res += "Type: Syncrolight Ethernet\n";
    int32_t channeloffset = ch - GetStartChannel() + 1;
    res += "IP: " + _ip + "\n";
    res += "Port: " + wxString::Format("%d", _port).ToStdString() + "\n";
    res += "Channel: " + std::string(wxString::Format(wxT("%i"), channeloffset)) + "\n";

    if (!_enabled) res += " INACTIVE";
    return res;
}
#pragma endregion Getters and Setters

#pragma region UI
#ifndef EXCLUDENETWORKUI
Output* SyncrolightEthernetOutput::Configure(wxWindow* parent, OutputManager* outputManager, ModelManager* modelManager)
{
    SyncrolightDialog dlg(parent, this, outputManager);

    int res = dlg.ShowModal();

    if (res == wxID_CANCEL)
    {
        return nullptr;
    }

    return this;
}
#endif
#pragma endregion UI
