
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

// Based on the protocol as described at http://www.3waylabs.com/ddp/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "DDPOutput.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"
#include "../OutputModelManager.h"

#include <log4cpp/Category.hh>

#pragma region Static Variables
bool DDPOutput::__initialised = false;
#pragma endregion

#pragma region Private Functions
void DDPOutput::OpenDatagram() {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_datagram != nullptr) return;

    wxIPV4address localaddr;
    if (IPOutput::__localIP == "") {
        localaddr.AnyAddress();
    }
    else {
        localaddr.Hostname(IPOutput::__localIP);
    }

    _datagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);
    if (_datagram == nullptr) {
        logger_base.error("Error initialising DDP datagram for %s. %s", (const char*)_ip.c_str(), (const char*)localaddr.IPAddress().c_str());
        _ok = false;
    }
    else if (!_datagram->IsOk()) {
        logger_base.error("Error initialising DDP datagram for %s. %s OK: FALSE", (const char*)_ip.c_str(), (const char*)localaddr.IPAddress().c_str());
        delete _datagram;
        _datagram = nullptr;
        _ok = false;
    }
    else if (_datagram->Error() != wxSOCKET_NOERROR) {
        logger_base.error("Error creating DDP datagram => %d : %s. %s", _datagram->LastError(), (const char*)DecodeIPError(_datagram->LastError()).c_str(), (const char*)localaddr.IPAddress().c_str());
        delete _datagram;
        _datagram = nullptr;
        _ok = false;
    }
}
#pragma endregion

#pragma region Constructors and Destructors
DDPOutput::DDPOutput(wxXmlNode* node) : IPOutput(node) {

    _fulldata = nullptr;
    _channelsPerPacket = wxAtoi(node->GetAttribute("ChannelsPerPacket"));
    _keepChannelNumbers = wxAtoi(node->GetAttribute("KeepChannelNumbers"));
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

DDPOutput::DDPOutput() : IPOutput() {

    _universe = 64001;
    _fulldata = nullptr;
    _channelsPerPacket = 1440;
    _channels = 512;
    _sequenceNum = 0;
    _datagram = nullptr;
    _keepChannelNumbers = true;
    memset(_data, 0, sizeof(_data));
}

DDPOutput::~DDPOutput() {

    if (_datagram != nullptr) delete _datagram;
    if (_fulldata != nullptr) delete _fulldata;
}

wxXmlNode* DDPOutput::Save() {

    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");

    node->AddAttribute("ChannelsPerPacket", wxString::Format("%i", _channelsPerPacket));
    node->AddAttribute("KeepChannelNumbers", _keepChannelNumbers ? "1" : "0");
    IPOutput::Save(node);

    return node;
}
#pragma endregion

#pragma region Static Functions
void DDPOutput::SendSync() {

    log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static uint8_t syncdata[DDP_SYNCPACKET_LEN];
    static wxIPV4address syncremoteAddr;
    static wxDatagramSocket *syncdatagram = nullptr;

    if (!__initialised) {
        logger_base.debug("Initialising DDP Sync.");

        __initialised = true;
        memset(syncdata, 0x00, sizeof(syncdata));
        syncdata[0] = DDP_FLAGS1_VER1 | DDP_FLAGS1_PUSH;
        syncdata[2] = 0x80;
        syncdata[3] = DDP_ID_DISPLAY;

        wxIPV4address localaddr;
        if (IPOutput::__localIP == "") {
            localaddr.AnyAddress();
        }
        else {
            localaddr.Hostname(IPOutput::__localIP);
        }

        if (syncdatagram != nullptr) {
            delete syncdatagram;
        }

        syncdatagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT | wxSOCKET_BROADCAST);
        if (syncdatagram == nullptr) {
            logger_base.error("Error initialising DDP sync datagram. %s", (const char *)localaddr.IPAddress().c_str());
            return;
        } else if (!syncdatagram->IsOk()) {
            logger_base.error("Error initialising DDP sync datagram ... is network connected? OK: FALSE %s", (const char *)localaddr.IPAddress().c_str());
            delete syncdatagram;
            syncdatagram = nullptr;
            return;
        }
        else if (syncdatagram->Error() != wxSOCKET_NOERROR) {
            logger_base.error("Error creating DDP sync datagram => %d : %s. %s", syncdatagram->LastError(), (const char *)DecodeIPError(syncdatagram->LastError()).c_str(), (const char *)localaddr.IPAddress().c_str());
            delete syncdatagram;
            syncdatagram = nullptr;
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

    if (syncdatagram != nullptr) {
        syncdatagram->SendTo(syncremoteAddr, syncdata, DDP_SYNCPACKET_LEN);
    }
}
#pragma endregion

#pragma region Getters and Setters
std::string DDPOutput::GetLongDescription() const {
    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "DDP {" + GetUniverseString() + "} ";
    res += "(" + std::string(wxString::Format(wxT("%i"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ") ";

    return res;
}
#pragma endregion

#pragma region Start and Stop
bool DDPOutput::Open() {

    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_enabled) return true;
    if (_ip == "") return false;

    if (_fulldata != nullptr) delete _fulldata;
    _fulldata = (uint8_t*)malloc(_channels);
    if (_fulldata == nullptr) {
        logger_base.error("Problem allocating %d memory for DDP output '%s'.", _channels, (const char *)GetIP().c_str());
        _ok = false;
        return false;
    }
    AllOff();

    _ok = IPOutput::Open();
    
    if (_fppProxyOutput) {
        return _ok;
    }

    memset(_data, 0x00, sizeof(_data));

    _data[2] = 1;
    _data[3] = DDP_ID_DISPLAY;
    _sequenceNum = 1;

    OpenDatagram();

    _remoteAddr.Hostname(_ip.c_str());
    _remoteAddr.Service(DDP_PORT);

    return _ok;
}

void DDPOutput::Close() {

    if (_datagram != nullptr) {
        delete _datagram;
        _datagram = nullptr;
    }
    if (_fulldata != nullptr) {
        free(_fulldata);
        _fulldata = nullptr;
    }
    IPOutput::Close();
}
#pragma endregion

#pragma region Frame Handling
void DDPOutput::StartFrame(long msec) {

    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_enabled) return;
    if (_fppProxyOutput) {
        _fppProxyOutput->StartFrame(msec);
    } else if (_datagram == nullptr && OutputManager::IsRetryOpen()) {
        OpenDatagram();
        if (_ok) {
            logger_base.debug("DDPOutput: Open retry successful");
        }
    }

    _timer_msec = msec;
}

void DDPOutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend) return;
    if (_fppProxyOutput) {
        _fppProxyOutput->EndFrame(suppressFrames);
        return;
    }
    if (_datagram == nullptr) return;

    if (_changed || NeedToOutput(suppressFrames)) {
        int32_t index = 0;
        int32_t chan = _keepChannelNumbers ? (_startChannel - 1) : 0;
        int32_t tosend = _channels;

        while (tosend > 0) {
            int32_t thissend = (tosend < _channelsPerPacket) ? tosend : _channelsPerPacket;

            if (__initialised) {
                // sync packet will boadcast later
                _data[0] = DDP_FLAGS1_VER1;
            }
            else {
                if (tosend == thissend) {
                    _data[0] = DDP_FLAGS1_VER1 | DDP_FLAGS1_PUSH;
                }
                else {
                    _data[0] = DDP_FLAGS1_VER1;
                }
            }

            _data[1] = (_data[1] & 0xF0) + _sequenceNum;

            _data[4] = (chan & 0xFF000000) >> 24;
            _data[5] = (chan & 0xFF0000) >> 16;
            _data[6] = (chan & 0xFF00) >> 8;
            _data[7] = (chan & 0xFF);

            _data[8] = (thissend & 0xFF00) >> 8;
            _data[9] = thissend & 0x00FF;

            memcpy(&_data[10], _fulldata + index, thissend);

            _datagram->SendTo(_remoteAddr, &_data[0], DDP_PACKET_LEN - (1440 - thissend));
            _sequenceNum = _sequenceNum == 15 ? 1 : _sequenceNum + 1;

            tosend -= thissend;
            index += thissend;
            chan += thissend;
        }
        FrameOutput();
    } else {
        SkipFrame();
    }
}
#pragma endregion

#pragma region Data Setting
void DDPOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (_fppProxyOutput) {
        _fppProxyOutput->SetOneChannel(channel, data);
        return;
    }
    if (_fulldata == nullptr) return;

    if ((channel < _channels) && (*(_fulldata + channel) != data)) {
        *(_fulldata + channel) = data;
        _changed = true;
    }
}

void DDPOutput::SetManyChannels(int32_t channel, unsigned char* data, size_t size) {

    if (_fppProxyOutput) {
        _fppProxyOutput->SetManyChannels(channel, data, size);
        return;
    }
    if (_fulldata == nullptr) return;

    size_t chs = (std::min)((int32_t)size, _channels - channel);

    if (memcmp(_fulldata + channel, data, chs) == 0) {
        // nothing changed
    } else {
        memcpy(_fulldata + channel, data, chs);
        _changed = true;
    }
}

void DDPOutput::AllOff() {

    if (_fppProxyOutput) {
        _fppProxyOutput->AllOff();
        return;
    }
    if (_fulldata == nullptr) return;
    memset(_fulldata, 0x00, _channels);
    _changed = true;
}
#pragma endregion

#pragma region UI
#ifndef EXCLUDENETWORKUI
void DDPOutput::AddProperties(wxPropertyGrid* propertyGrid, bool allSameSize)
{
    auto p = propertyGrid->Append(new wxUIntProperty("Channels Per Packet", "ChannelsPerPacket", GetChannelsPerPacket()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1440);
    p->SetEditor("SpinCtrl");

    p = propertyGrid->Append(new wxBoolProperty("Keep Channel Numbers", "KeepChannelNumbers", IsKeepChannelNumbers()));
    p->SetEditor("CheckBox");
}

bool DDPOutput::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) {

    wxString const name = event.GetPropertyName();

    if (name == "ChannelsPerPacket") {
        SetChannelsPerPacket(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "DDPOutput::HandlePropertyEvent::ChannelsPerPacket");
        return true;
    } else if (name == "KeepChannelNumbers") {
        SetKeepChannelNumber(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "DDPOutput::HandlePropertyEvent::KeepChannelNumbers");
        return true;
    }

    return false;
}
#endif
#pragma endregion UI
