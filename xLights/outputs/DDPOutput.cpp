
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
#include "ControllerEthernet.h"
#include "../utils/ip_utils.h"

#include <log4cpp/Category.hh>

#ifndef EXCLUDEDISCOVERY
#include "../Discovery.h"
#endif

#pragma region Static Variables
bool DDPOutput::__initialised = false;
#pragma endregion

#pragma region Private Functions
void DDPOutput::OpenDatagram() {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_datagram != nullptr) return;

    wxIPV4address localaddr;
    if (GetForceLocalIP() == "") {
        localaddr.AnyAddress();
    }
    else {
        localaddr.Hostname(GetForceLocalIP());
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
void DDPOutput::SendSync(const std::string& localIP) {

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
        if (localIP == "") {
            localaddr.AnyAddress();
        }
        else {
            localaddr.Hostname(localIP);
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

#ifndef EXCLUDENETWORKUI
wxJSONValue DDPOutput::Query(const std::string& ip, uint8_t type, const std::string& localIP)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxJSONValue val;

    uint8_t packet[DDP_DISCOVERPACKET_LEN];
    memset(&packet, 0x00, sizeof(packet));

    packet[0] = DDP_FLAGS1_VER1 | DDP_FLAGS1_QUERY;
    packet[3] = type;

    wxIPV4address localaddr;
    if (localIP == "") {
        localaddr.AnyAddress();
    }
    else {
        localaddr.Hostname(localIP);
    }

    logger_base.debug(" DDP query using %s", (const char*)localaddr.IPAddress().c_str());
    wxDatagramSocket* datagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);

    if (datagram == nullptr) {
        logger_base.error("Error initialising DDP query datagram.");
    }
    else if (!datagram->IsOk()) {
        logger_base.error("Error initialising DDP query datagram ... is network connected? OK : FALSE");
        delete datagram;
        datagram = nullptr;
    }
    else if (datagram->Error() != wxSOCKET_NOERROR) {
        logger_base.error("Error creating DDP query datagram => %d : %s.", datagram->LastError(), (const char*)DecodeIPError(datagram->LastError()).c_str());
        delete datagram;
        datagram = nullptr;
    }
    else {
        logger_base.info("DDP query datagram opened successfully.");
    }

    wxIPV4address remoteaddr;
    remoteaddr.Hostname(ip);
    remoteaddr.Service(DDP_PORT);

    // bail if we dont have a datagram to use
    if (datagram != nullptr) {
        logger_base.info("DDP sending query packet.");
        datagram->SendTo(remoteaddr, &packet, DDP_DISCOVERPACKET_LEN);
        if (datagram->Error() != wxSOCKET_NOERROR) {
            logger_base.error("Error sending DDP query datagram => %d : %s.", datagram->LastError(), (const char*)DecodeIPError(datagram->LastError()).c_str());
        }
        else {
            logger_base.info("DDP sent query packet. Sleeping for 1 second.");

            // give the controllers 2 seconds to respond
            wxMilliSleep(1000);

            uint8_t response[1024];

            int lastread = 1;

            while (lastread > 0) {
                wxStopWatch sw;
                logger_base.debug("Trying to read DDP query response packet.");
                memset(&response, 0x00, sizeof(response));
                datagram->Read(&response, sizeof(response));
                lastread = datagram->LastReadCount();

                if (lastread > 10) {
                    logger_base.debug(" Read done. %d bytes %ldms", lastread, sw.Time());

                    if (response[3] == type) {
                        logger_base.debug(" Valid response.");
                        logger_base.debug((const char*)&response[10]);

                        wxJSONReader reader;
                        reader.Parse(wxString(&response[10]), &val);
                    }
                }
                logger_base.info("DDP Query Done looking for response.");
            }
        }
        datagram->Close();
        delete datagram;
    }
    logger_base.info("DDP Query Finished.");

    return val;
}
#endif

void DDPOutput::PrepareDiscovery(Discovery &discovery) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    discovery.AddBroadcast(DDP_PORT, [&discovery](wxDatagramSocket* socket, uint8_t *response, int len) {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        if (response[0] & DDP_FLAGS1_QUERY) {
            //getting my own QUERY request, ignore
            return;
        }
        if (response[3] == DDP_ID_STATUS) {
            logger_base.debug(" Valid DDP Status Response.");
            logger_base.debug((const char*)&response[10]);

            wxIPV4address add;
            socket->GetPeer(add);
            std::string ip = add.IPAddress();
            DiscoveredData *dd = discovery.FindByIp(ip);
            if (dd == nullptr) {
                ControllerEthernet *controller = new ControllerEthernet(discovery.GetOutputManager(), false);
                controller->SetProtocol(OUTPUT_DDP);
                logger_base.debug("   IP %s", (const char*)ip.c_str());
                controller->SetIP(ip);
                controller->SetId(1);
                controller->EnsureUniqueId();
                controller->SetName("DDP_" + ip);
                dd = discovery.AddController(controller);
            }
            ControllerEthernet* controller = dd->controller;

            if (controller == nullptr){
                logger_base.warn("Unsupported DDP controller");
            }
            else {
                wxJSONReader reader;
                wxJSONValue val;
                reader.Parse(wxString(&response[10]), &val);
                if (val.HasMember("status")) {
                    std::string name = "";
                    if (val["status"].HasMember("man")) {
                        name =
                                val["status"]["man"].AsString().ToStdString();
                        dd->SetVendor(name);
                    }
                    if (val["status"].HasMember("mod")) {
                        if (name != "") {
                            name += "-";
                        }
                        name +=
                                val["status"]["mod"].AsString().ToStdString();
                        dd->SetModel(
                                val["status"]["mod"].AsString().ToStdString());
                    }
                    if (val["status"].HasMember("ver")) {
                        if (name != "") {
                            name += "-";
                        }
                        name +=
                                val["status"]["ver"].AsString().ToStdString();
                        dd->version =
                                val["status"]["ver"].AsString().ToStdString();
                    }
                    dd->description = name;
                    controller->SetDescription(name);
                }
            }
            uint8_t packet[DDP_DISCOVERPACKET_LEN];
            memset(&packet, 0x00, sizeof(packet));
            packet[0] = DDP_FLAGS1_VER1 | DDP_FLAGS1_QUERY;
            packet[3] = DDP_ID_CONFIG;
            // send a unicast CONFIG request in case it doesn't respond to config
            // on the broadcast
            discovery.SendData(DDP_PORT, ip, packet, DDP_DISCOVERPACKET_LEN);
        } else if (response[3] == DDP_ID_CONFIG) {
            logger_base.debug(" Valid DDP Config Response.");
            logger_base.debug((const char*)&response[10]);

            wxIPV4address add;
            socket->GetPeer(add);
            std::string ip = add.IPAddress();
            DiscoveredData *dd = discovery.FindByIp(ip);
            if (dd == nullptr) {
                ControllerEthernet *controller = new ControllerEthernet(discovery.GetOutputManager(), false);
                controller->SetProtocol(OUTPUT_DDP);
                logger_base.debug("   IP %s", (const char*)ip.c_str());
                controller->SetIP(ip);
                controller->SetId(1);
                controller->EnsureUniqueId();
                controller->SetName("DDP_" + ip);
                dd = discovery.AddController(controller);
            }
            ControllerEthernet* controller = dd->controller;

            if (controller == nullptr){
                logger_base.warn("Unsupported DDP controller");
            }
            else {
                wxJSONReader reader;
                wxJSONValue val;
                reader.Parse(wxString(&response[10]), &val);

                if (val.HasMember("config")
                        && val["config"].HasMember("ports")) {
                    int channels = 0;
                    auto ports = val["config"]["ports"].AsArray();
                    for (int i = 0; i < ports->Count(); i++) {
                        auto ts =
                                wxAtoi(
                                        val["config"]["ports"][i]["ts"].AsString())
                                        + 1;
                        auto l =
                                wxAtoi(
                                        val["config"]["ports"][i]["l"].AsString());
                        channels += ts * l * 3;
                    }
                    controller->GetOutputs().front()->SetChannels(
                            channels);
                } else {
                    controller->GetOutputs().front()->SetChannels(512);
                }
            }
        } else {
            // non discovery response packet
            logger_base.info("DDP Discovery strange packet received.");
        }
    });

    logger_base.info("Sending DDP Discovery.");
    uint8_t packet[DDP_DISCOVERPACKET_LEN];
    memset(&packet, 0x00, sizeof(packet));
    packet[0] = DDP_FLAGS1_VER1 | DDP_FLAGS1_QUERY;
    packet[3] = DDP_ID_STATUS;
    discovery.SendBroadcastData(DDP_PORT, packet, DDP_DISCOVERPACKET_LEN);
    packet[0] = DDP_FLAGS1_VER1 | DDP_FLAGS1_QUERY;
    packet[3] = DDP_ID_CONFIG;
    discovery.SendBroadcastData(DDP_PORT, packet, DDP_DISCOVERPACKET_LEN);
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
    if (!ip_utils::IsIPValid(_resolvedIp)) return false;

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

    _data[2] = 0;
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

    if (!_enabled || _suspend || _tempDisable) return;
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

    if (!_enabled) return;

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

    if (!_enabled) return;

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

    if (!_enabled) return;

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
void DDPOutput::AddProperties(wxPropertyGrid* propertyGrid, bool allSameSize, std::list<wxPGProperty*>& expandProperties)
{
    auto p = propertyGrid->Append(new wxUIntProperty("Channels Per Packet", "ChannelsPerPacket", GetChannelsPerPacket()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1440);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("It would be very rare that you would ever want to change this from the default.");

    p = propertyGrid->Append(new wxBoolProperty("Keep Channel Numbers", "KeepChannelNumbers", IsKeepChannelNumbers()));
    p->SetEditor("CheckBox");
    p->SetHelpString("When not selected DDP data arrives at each controller looking like it starts at channel 1. When selected it arrives with the xLights channel number.");
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
