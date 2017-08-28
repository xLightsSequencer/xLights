// http://www.3waylabs.com/ddp/

#include "DDPOutput.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "DDPDialog.h"
#include "OutputManager.h"

#pragma region Static Variables
bool DDPOutput::__initialised = false;
#pragma endregion Static Variables

#pragma region Constructors and Destructors
DDPOutput::DDPOutput(wxXmlNode* node) : IPOutput(node)
{
    _fulldata = nullptr;
    _channelsPerPacket = wxAtoi(node->GetAttribute("ChannelsPerPacket"));
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

DDPOutput::DDPOutput() : IPOutput()
{
    _fulldata = nullptr;
    _channelsPerPacket = 512;
    _channels = 512;
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

DDPOutput::~DDPOutput()
{
    if (_datagram != nullptr) delete _datagram;
    if (_fulldata != nullptr) delete _fulldata;
}
#pragma endregion Constructors and Destructors

#pragma region Static Functions
void DDPOutput::SendSync()
{
    static wxByte syncdata[DDP_SYNCPACKET_LEN];
    static wxIPV4address syncremoteAddr;
    static wxDatagramSocket *syncdatagram = nullptr;

    if (!__initialised)
    {
        log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("Initialising DDP Sync.");

        __initialised = true;
        memset(syncdata, 0x00, sizeof(syncdata));
        syncdata[0] = DDP_FLAGS1_VER1 | DDP_FLAGS1_PUSH;
        syncdata[2] = 1;
        syncdata[3] = DDP_ID_DISPLAY;

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

        syncdatagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT | wxSOCKET_BROADCAST);
        if (syncdatagram == nullptr)
        {
            logger_base.error("Error initialising DDP sync datagram.");
            return;
        }

        if (!syncdatagram->IsOk())
        {
            delete syncdatagram;
            syncdatagram = nullptr;
            logger_base.error("Error initialising DDP sync datagram ... is network connected.");
            return;
        }

        // broadcast ... this is not really in line with the spec
        // I should use the net mask but i cant find a good way to do that
        //syncremoteAddr.BroadcastAddress();
        wxString broadcast = "255.255.255.255";
        logger_base.debug("DDP Sync broadcasting to %s.", (const char *)broadcast.c_str());
        syncremoteAddr.Hostname(broadcast);
        syncremoteAddr.Service(DDP_PORT);
    }

    if (syncdatagram != nullptr)
    {
        syncdatagram->SendTo(syncremoteAddr, syncdata, DDP_SYNCPACKET_LEN);
    }
}
#pragma endregion Static Functions

wxXmlNode* DDPOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    node->AddAttribute("ChannelsPerPacket", wxString::Format("%i", _channelsPerPacket));
    IPOutput::Save(node);

    return node;
}

#pragma region Start and Stop
bool DDPOutput::Open()
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_enabled) return true;

    if (_fulldata != nullptr) delete _fulldata;
    _fulldata = (wxByte*)malloc(_channels);
    if (_fulldata == nullptr)
    {
        logger_base.error("Problem allocating %ld memory for DDP output '%s'.", _channels, (const char *)_description.c_str());
        _ok = false;
        return false;
    }
    AllOff();

    _ok = IPOutput::Open();

    memset(_data, 0x00, sizeof(_data));

    _data[2] = 1;
    _data[3] = DDP_ID_DISPLAY;
    _sequenceNum = 1;

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
        logger_base.error("Error initialising DDP datagram for %s.", (const char *)_ip.c_str());
        _ok = false;
        return _ok;
    }

    if (!_datagram->IsOk())
    {
        delete _datagram;
        _datagram = nullptr;
        logger_base.error("Error initialising DDP datagram for %s.", (const char *)_ip.c_str());
        _ok = false;
        return _ok;
    }

    _remoteAddr.Hostname(_ip.c_str());
    _remoteAddr.Service(DDP_PORT);

    __initialised = false;

    return _ok;
}
#pragma endregion Start and Stop

#pragma region Frame Handling
void DDPOutput::EndFrame(int suppressFrames)
{
    if (!_enabled || _datagram == nullptr) return;

    if (_changed || NeedToOutput(suppressFrames))
    {
        long index = 0;
        long tosend = _channels;

        while (tosend > 0)
        {
            long thissend = (tosend < _channelsPerPacket) ? tosend : _channelsPerPacket;

            if (__initialised)
            {
                _data[0] = DDP_FLAGS1_VER1;
            }
            else
            {
                if (tosend == thissend)
                {
                    _data[0] = DDP_FLAGS1_VER1 | DDP_FLAGS1_PUSH;
                }
                else
                {
                    _data[0] = DDP_FLAGS1_VER1;
                }
            }

            _data[1] = (_data[1] & 0xF0) + _sequenceNum;
            
            _data[4] = (index & 0xFF000000) >> 24;
            _data[5] = (index & 0xFF0000) >> 16;
            _data[6] = (index & 0xFF00) >> 8;
            _data[7] = (index & 0xFF);

            _data[8] = (thissend & 0xFF00) >> 8;
            _data[9] = thissend & 0x00FF;

            memcpy(&_data[10], _fulldata + index, thissend);

            _datagram->SendTo(_remoteAddr, &_data[0], DDP_PACKET_LEN - (1440 - thissend));
            _sequenceNum = _sequenceNum == 15 ? 1 : _sequenceNum + 1;

            tosend -= thissend;
            index += thissend;
            FrameOutput();
        }
    }
    else
    {
        SkipFrame();
    }
}
#pragma endregion Frame Handling

#pragma region Data Setting
void DDPOutput::SetOneChannel(long channel, unsigned char data)
{
    wxASSERT(channel < _channels);

    if (*(_fulldata + channel) != data) 
    {
        *(_fulldata + channel) = data;
        _changed = true;
    }
}

void DDPOutput::SetManyChannels(long channel, unsigned char data[], long size)
{
    if (_fulldata == nullptr) return;

    wxASSERT(channel + size <= _channels);

#ifdef _MSC_VER
    long chs = min(size, _channels - channel);
#else
    long chs = std::min(size, _channels - channel);
#endif

    if (memcmp(_fulldata + channel, data, chs) == 0)
    {
        // nothing changed
    }
    else
    {
        memcpy(_fulldata + channel, data, chs);
        _changed = true;
    }
}

void DDPOutput::AllOff()
{
    memset(_fulldata, 0x00, _channels);
    _changed = true;
}
#pragma endregion Data Setting

#pragma region Getters and Setters
std::string DDPOutput::GetLongDescription() const
{
    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "DDP " + _ip + " {" + GetUniverseString() + "} ";
    res += "[1-" + std::string(wxString::Format(wxT("%i"), (long)_channels)) + "] ";
    res += "(" + std::string(wxString::Format(wxT("%i"), (long)GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), (long)GetEndChannel())) + ") ";
    res += _description;

    return res;
}

std::string DDPOutput::GetChannelMapping(long ch) const
{
    std::string res = "Channel " + std::string(wxString::Format(wxT("%i"), ch)) + " maps to ...\n";

    long channeloffset = ch - GetStartChannel() + 1;

    res += "Type: DDP\n";
    res += "IP: " + _ip + "\n";
    res += "Channel: " + std::string(wxString::Format(wxT("%i"), channeloffset)) + "\n";

    if (!_enabled) res += " INACTIVE";

    return res;
}
#pragma endregion Getters and Setters

#pragma region UI
#ifndef EXCLUDENETWORKUI
Output* DDPOutput::Configure(wxWindow* parent, OutputManager* outputManager)
{
    DDPDialog dlg(parent, this, outputManager);

    int res = dlg.ShowModal();

    if (res == wxID_CANCEL)
    {
        return nullptr;
    }

    return this;
}
#endif
#pragma endregion UI
