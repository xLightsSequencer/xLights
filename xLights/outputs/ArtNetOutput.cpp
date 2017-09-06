#include "ArtNetOutput.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "ArtNetDialog.h"
#include "OutputManager.h"

#pragma region Static Variables
int ArtNetOutput::__ip1 = -1;
int ArtNetOutput::__ip2 = -1;
int ArtNetOutput::__ip3 = -1;
bool ArtNetOutput::__initialised = false;
#pragma endregion Static Variables

#pragma region Constructors and Destructors
ArtNetOutput::ArtNetOutput(wxXmlNode* node) : IPOutput(node)
{
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

ArtNetOutput::ArtNetOutput() : IPOutput()
{
    _channels = 510;
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

ArtNetOutput::~ArtNetOutput()
{
    if (_datagram != nullptr) delete _datagram;
}
#pragma endregion Constructors and Destructors

#pragma region Static Functions
int ArtNetOutput::GetArtNetNet(int u)
{
    return (u & 0x7F00) >> 8;
}

int ArtNetOutput::GetArtNetSubnet(int u)
{
    return (u & 0x00F0) >> 4;
}

int ArtNetOutput::GetArtNetUniverse(int u)
{
    return u & 0x000F;
}

int ArtNetOutput::GetArtNetCombinedUniverse(int net, int subnet, int universe)
{
    return ((net & 0x007F) << 8) + ((subnet & 0x000F) << 4) + (universe & 0x000F);
}

void ArtNetOutput::SendSync()
{
    static wxByte syncdata[ARTNET_SYNCPACKET_LEN];
    static wxIPV4address syncremoteAddr;
    static wxDatagramSocket *syncdatagram = nullptr;

    if (!__initialised)
    {
        log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("Initialising artNet Sync.");

        __initialised = true;
        memset(syncdata, 0x00, sizeof(syncdata));
        syncdata[0] = 'A';   // ARTNET flag
        syncdata[1] = 'r';
        syncdata[2] = 't';
        syncdata[3] = '-';
        syncdata[4] = 'N';
        syncdata[5] = 'e';
        syncdata[6] = 't';
        syncdata[9] = 0x52;
        syncdata[11] = 0x0E; // Protocol version Low

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
            logger_base.error("Error initialising Artnet sync datagram.");
            return;
        }

        if (!syncdatagram->IsOk())
        {
            logger_base.error("Error initialising Artnet sync datagram ... is network connected: %s", (const char *)IPOutput::DecodeError(syncdatagram->LastError()).c_str());
            delete syncdatagram;
            syncdatagram = nullptr;
            return;
        }

        // broadcast ... this is not really in line with the spec
        // I should use the net mask but i cant find a good way to do that
        //syncremoteAddr.BroadcastAddress();
        wxString ipaddrWithUniv = wxString::Format("%d.%d.%d.%d", __ip1, __ip2, __ip3, 255);
        logger_base.debug("artNet Sync broadcasting to %s.", (const char *)ipaddrWithUniv.c_str());
        syncremoteAddr.Hostname(ipaddrWithUniv);
        syncremoteAddr.Service(ARTNET_PORT);
    }

    if (syncdatagram != nullptr)
    {
        syncdatagram->SendTo(syncremoteAddr, syncdata, ARTNET_SYNCPACKET_LEN);
    }
}
#pragma endregion Static Functions

#pragma region Start and Stop
bool ArtNetOutput::Open()
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_enabled) return true;

    _ok = IPOutput::Open();

    memset(_data, 0x00, sizeof(_data));

    _sequenceNum = 0;

    _data[0] = 'A';   // ID[8]
    _data[1] = 'r';
    _data[2] = 't';
    _data[3] = '-';
    _data[4] = 'N';
    _data[5] = 'e';
    _data[6] = 't';
    _data[9] = 0x50;
    _data[11] = 0x0E; // Protocol version Low
    _data[14] = (_universe & 0xFF);
    _data[15] = ((_universe & 0xFF00) >> 8);
    _data[16] = 0x02; // we are going to send all 512 bytes

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
        logger_base.error("Error initialising Artnet datagram for %s %d:%d:%d.", (const char *)_ip.c_str(), GetArtNetNet(), GetArtNetSubnet(), GetArtNetUniverse());
        _ok = false;
        return _ok;
    }

    if (!_datagram->IsOk())
    {
        logger_base.error("Error initialising Artnet datagram for %s %d:%d:%d. %s", (const char *)_ip.c_str(), GetArtNetNet(), GetArtNetSubnet(), GetArtNetUniverse(), (const char *)IPOutput::DecodeError(_datagram->LastError()).c_str());
        delete _datagram;
        _datagram = nullptr;
        _ok = false;
        return _ok;
    }

    _remoteAddr.Hostname(_ip.c_str());
    _remoteAddr.Service(ARTNET_PORT);

    // work out our broascast address
    wxArrayString ipc = wxSplit(_ip.c_str(), '.');
    if (__ip1 == -1)
    {
        __ip1 = wxAtoi(ipc[0]);
        __ip2 = wxAtoi(ipc[1]);
        __ip3 = wxAtoi(ipc[2]);
    }
    else if (wxAtoi(ipc[0]) != __ip1)
    {
        __ip1 = 255;
        __ip2 = 255;
        __ip3 = 255;
    }
    else
    {
        if (wxAtoi(ipc[1]) != __ip2)
        {
            __ip2 = 255;
            __ip3 = 255;
        }
        else
        {
            if (wxAtoi(ipc[2]) != __ip3)
            {
                __ip3 = 255;
            }
        }
    }

    __initialised = false;
    logger_base.debug("Artnet broadcast address %d.%d.%d.255", __ip1, __ip2, __ip3);

    _data[16] = (wxByte)(_channels >> 8);  // Property value count (high)
    _data[17] = (wxByte)(_channels & 0xff);  // Property value count (low)

    return _ok;
}
#pragma endregion Start and Stop

#pragma region Frame Handling
void ArtNetOutput::EndFrame(int suppressFrames)
{
    if (!_enabled || _datagram == nullptr) return;

    if (_changed || NeedToOutput(suppressFrames))
    {
        _data[12] = _sequenceNum;
        _datagram->SendTo(_remoteAddr, _data, ARTNET_PACKET_LEN - (512 - _channels));
        _sequenceNum = _sequenceNum == 255 ? 0 : _sequenceNum + 1;
        FrameOutput();
        _changed = false;
    }
    else
    {
        SkipFrame();
    }
}
#pragma endregion Frame Handling

#pragma region Data Setting
void ArtNetOutput::SetOneChannel(long channel, unsigned char data)
{
    wxASSERT(channel < _channels);

    if (_data[channel + ARTNET_PACKET_HEADERLEN] != data) {
        _data[channel + ARTNET_PACKET_HEADERLEN] = data;
        _changed = true;
    }
}

void ArtNetOutput::SetManyChannels(long channel, unsigned char data[], long size)
{
    wxASSERT(channel + size < _channels);

#ifdef _MSC_VER
    long chs = min(size, _channels - channel);
#else
    long chs = std::min(size, _channels - channel);
#endif

    if (memcmp(&_data[channel + ARTNET_PACKET_HEADERLEN], data, chs) == 0)
    {
        // nothing has changed
    }
    else
    {
        memcpy(&_data[channel + ARTNET_PACKET_HEADERLEN], data, chs);
        _changed = true;
    }
}

void ArtNetOutput::AllOff()
{
    memset(&_data[ARTNET_PACKET_HEADERLEN], 0x00, _channels);
    _changed = true;
}
#pragma endregion Data Setting

#pragma region Getters and Setters
std::string ArtNetOutput::GetUniverseString() const
{
    return wxString::Format(wxT("%i:%i:%i"), GetArtNetNet(), GetArtNetSubnet(), GetArtNetUniverse()).ToStdString();
}

std::string ArtNetOutput::GetLongDescription() const
{
    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "ArtNet " + _ip + " {" + GetUniverseString() + "} ";
    res += "[1-" + std::string(wxString::Format(wxT("%i"), (long)_channels)) + "] ";
    res += "(" + std::string(wxString::Format(wxT("%i"), (long)GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), (long)GetEndChannel())) + ") ";
    res += _description;

    return res;
}

std::string ArtNetOutput::GetChannelMapping(long ch) const
{
    std::string res = "Channel " + std::string(wxString::Format(wxT("%i"), ch)) + " maps to ...\n";

    long channeloffset = ch - GetStartChannel() + 1;

    res += "Type: ArtNet\n";
    res += "IP: " + _ip + "\n";
    res += "Net: " + wxString::Format(wxT("%i"), GetArtNetNet()).ToStdString() + "\n";
    res += "Subnet: " + wxString::Format(wxT("%i"), GetArtNetSubnet()).ToStdString() + "\n";
    res += "Universe: " + wxString::Format(wxT("%i"), GetArtNetUniverse()).ToStdString() + "\n";
    res += "Channel: " + std::string(wxString::Format(wxT("%i"), channeloffset)) + "\n";

    if (!_enabled) res += " INACTIVE";

    return res;
}
#pragma endregion Getters and Setters

#pragma region UI
#ifndef EXCLUDENETWORKUI
Output* ArtNetOutput::Configure(wxWindow* parent, OutputManager* outputManager)
{
    ArtNetDialog dlg(parent, this, outputManager);

    int res = dlg.ShowModal();

    if (res == wxID_CANCEL)
    {
        return nullptr;
    }

    return this;
}
#endif
#pragma endregion UI
