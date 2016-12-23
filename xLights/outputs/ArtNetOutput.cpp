#include "ArtNetOutput.h"

#include <wx/xml/xml.h>

#include <log4cpp/Category.hh>
#include "ArtNetDialog.h"
#include "OutputManager.h"

int ArtNetOutput::__ip1 = -1;
int ArtNetOutput::__ip2 = -1;
int ArtNetOutput::__ip3 = -1;
bool ArtNetOutput::__initialised = false;

wxXmlNode* ArtNetOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    IPOutput::Save(node);

    return node;
}

ArtNetOutput::ArtNetOutput(wxXmlNode* node) : IPOutput(node)
{
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

ArtNetOutput::ArtNetOutput() : IPOutput()
{
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

ArtNetOutput::~ArtNetOutput()
{
    if (_datagram != nullptr) delete _datagram;
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
        localaddr.AnyAddress();

        if (syncdatagram != nullptr)
        {
            delete syncdatagram;
            syncdatagram = nullptr;
        }

        syncdatagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);
        if (syncdatagram == nullptr)
        {
            logger_base.error("Error initialising Artnet sync datagram.");
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

bool ArtNetOutput::Open()
{
    _ok = IPOutput::Open();

    memset(_data, 0x00, sizeof(_data));

    _sequenceNum = 0;
    #ifdef USECHANGEDETECTION
        _skipCount = 0;
    #endif

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

    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxIPV4address localaddr;
    localaddr.AnyAddress();
    _datagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);
    if (_datagram == nullptr)
    {
        logger_base.error("Error initialising Artnet datagram for %s %d:%d:%d.", (const char *)_ip.c_str(), GetArtNetNet(), GetArtNetSubnet(), GetArtNetUniverse());
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

    int i = _channels;
    wxByte NumHi = i >> 8;   // Channels (high)
    wxByte NumLo = i & 0xff; // Channels (low)

    _data[16] = NumHi;  // Property value count (high)
    _data[17] = NumLo;  // Property value count (low)

    return _ok && _datagram != nullptr;
}

void ArtNetOutput::Close()
{
}

void ArtNetOutput::StartFrame(long msec)
{
    _timer_msec = msec;
}

void ArtNetOutput::EndFrame()
{
    if (!_enabled || _datagram == nullptr) return;

#ifdef USECHANGEDETECTION
    if (changed || SkipCount > 10)
    {
#endif
        _data[12] = _sequenceNum;
        _datagram->SendTo(_remoteAddr, _data, ARTNET_PACKET_LEN - (512 - _channels));
        _sequenceNum = _sequenceNum == 255 ? 0 : _sequenceNum + 1;
#ifdef USECHANGEDETECTION
        _skipCount = 0;
        changed = false;
    }
    else
    {
        _skipCount++;
    }
#endif
}

void ArtNetOutput::ResetFrame()
{
}

void ArtNetOutput::SetOneChannel(int channel, unsigned char data)
{
#ifdef USECHANGEDETECTION
    if (_data[channel + ARTNET_PACKET_HEADERLEN] != data) {
#endif
        _data[channel + ARTNET_PACKET_HEADERLEN] = data;
#ifdef USECHANGEDETECTION
        _changed = true;
    }
#endif
}

void ArtNetOutput::SetManyChannels(int channel, unsigned char data[], size_t size)
{
#ifdef _MSC_VER
    int chs = min((int)size, (int)(_channels - channel + 1));
#else
    int chs = std::min((int)size, (int)(GetMaxChannels() - channel + 1));
#endif
    memcpy(&_data[channel + ARTNET_PACKET_HEADERLEN], data, chs);

#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}

void ArtNetOutput::AllOff()
{
    memset(&_data[1 + ARTNET_PACKET_HEADERLEN], 0x00, _channels);
#ifdef USECHANGEDETECTION
    _changed = true;
#endif
}

std::string ArtNetOutput::GetLongDescription() const
{
    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "ArtNet " + _ip + " {" + wxString::Format("%d", GetArtNetNet()).ToStdString() + ":" + wxString::Format("%d", GetArtNetSubnet()).ToStdString() + ":" + wxString::Format("%d", GetArtNetUniverse()).ToStdString() + "} ";
    res += "[1-" + std::string(wxString::Format(wxT("%i"), _channels)) + "] ";
    res += "(" + std::string(wxString::Format(wxT("%i"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ") ";
    res += _description;

    return res;
}

std::string ArtNetOutput::GetChannelMapping(int ch) const
{
    std::string res = "Channel " + std::string(wxString::Format(wxT("%i"), ch)) + " maps to ...\n";

    int channeloffset = ch - GetStartChannel() + 1;

    res += "Type: ArtNet\n";
    res += "IP: " + _ip + "\n";
    res += "Net: " + wxString::Format("%d", GetArtNetNet()).ToStdString() + "\n";
    res += "Subnet: " + wxString::Format("%d", GetArtNetSubnet()).ToStdString() + "\n";
    res += "Universe: " + wxString::Format("%d", GetArtNetUniverse()).ToStdString() + "\n";
    res += "Channel: " + std::string(wxString::Format(wxT("%i"), channeloffset)) + "\n";

    if (!_enabled) res += " INACTIVE";

    return res;
}

#ifndef EXCLUDENETWORKUI
int ArtNetOutput::Configure(wxWindow* parent, OutputManager& outputManager)
{
    ArtNetDialog dlg(parent);

    dlg.SpinCtrlNet->SetValue(GetArtNetNet());
    dlg.SpinCtrlSubnet->SetValue(GetArtNetSubnet());
    dlg.SpinCtrlUniverse->SetValue(GetArtNetUniverse());
    dlg.SpinCtrlChannels->SetValue(_channels);
    dlg.TextCtrlDescription->SetValue(_description.c_str());
    dlg.TextCtrlIPAddress->SetValue(_ip.c_str());
    dlg.SpinCtrlUniverseOnly->SetValue(_universe);
    dlg.SpinCtrl_NumUniv->SetValue(1);
    if (_ip == "")
    {
        dlg.SpinCtrl_NumUniv->Enable(true);
    }
    else
    {
        dlg.SpinCtrl_NumUniv->Enable(false);
    }

    int res = dlg.ShowModal();

    if (res == wxID_OK)
    {
        SetIP(dlg.TextCtrlIPAddress->GetValue().ToStdString());
        SetArtNetUniverse(dlg.SpinCtrlNet->GetValue(), dlg.SpinCtrlSubnet->GetValue(), dlg.SpinCtrlUniverse->GetValue());
        _channels = dlg.SpinCtrlChannels->GetValue();
        _description = dlg.TextCtrlDescription->GetValue().ToStdString();

        if (dlg.SpinCtrl_NumUniv->GetValue() > 1)
        {
            for (int i = 1; i < dlg.SpinCtrl_NumUniv->GetValue(); i++)
            {
                ArtNetOutput* a = new ArtNetOutput();
                a->SetIP(_ip);
                a->SetUniverse(_universe + i);
                a->SetDescription(_description);
                a->SetChannels(_channels);
                outputManager.AddOutput(a, -1);
            }
        }
    }

    return res;
}
#endif
