
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ArtNetOutput.h"
#include "OutputManager.h"
#include "UtilFunctions.h"
#include "../ui/wxUtilities.h"
#include "ControllerEthernet.h"
#include "../OutputModelManager.h"
#include "../SpecialOptions.h"
#include "../utils/ip_utils.h"
#include <format>

#include <log.h>

#include "../Discovery.h"

#pragma region Static Variables
int ArtNetOutput::__ip1 = -1;
int ArtNetOutput::__ip2 = -1;
int ArtNetOutput::__ip3 = -1;
bool ArtNetOutput::__initialised = false;
#pragma endregion

#pragma region Private Functions
void ArtNetOutput::OpenDatagram() {

    

    if (_datagram != nullptr) return;

    wxIPV4address localaddr;
    if (GetForceLocalIPToUse() == "") {
        localaddr.AnyAddress();
    }
    else {
        localaddr.Hostname(GetForceLocalIPToUse());
    }

    wxSocketFlags flags = wxSOCKET_BLOCK; // dont use NOWAIT as it can result in dropped packets
    if (_forceSourcePort) {
        flags |= wxSOCKET_REUSEADDR;
        localaddr.Service(ARTNET_PORT);
    }
    _datagram = new wxDatagramSocket(localaddr, flags);
    if (_datagram == nullptr) {
        spdlog::error("Error initialising Artnet datagram for {} {}:{}:{}. {}", (const char*)_ip.c_str(), GetArtNetNet(), GetArtNetSubnet(), GetArtNetUniverse(), (const char*)localaddr.IPAddress().c_str());
        _ok = false;
    }
    else if (!_datagram->IsOk()) {
        spdlog::error("Error initialising Artnet datagram for {} {}:{}:{}. {} OK : FALSE", (const char*)_ip.c_str(), GetArtNetNet(), GetArtNetSubnet(), GetArtNetUniverse(), (const char*)localaddr.IPAddress().c_str());
        delete _datagram;
        _datagram = nullptr;
        _ok = false;
    }
    else if (_datagram->Error()) {
        spdlog::error("Error creating Artnet datagram => {} : {}. {}", (int)_datagram->LastError(), (const char*)DecodeIPError(_datagram->LastError()).c_str(), (const char*)localaddr.IPAddress().c_str());
        delete _datagram;
        _datagram = nullptr;
        _ok = false;
    }
}
#pragma endregion

#pragma region Constructors and Destructors
ArtNetOutput::ArtNetOutput(pugi::xml_node node, bool isActive) : IPOutput(node, isActive) {

    if (_channels > 512) SetChannels(512);
    if (_autoSize_CONVERT) _autoSize_CONVERT = false;
    _sequenceNum = 0;
    _datagram = nullptr;
    _forceSourcePort = false;
    memset(_data, 0, sizeof(_data));

    _forceSourcePort = std::string_view(node.attribute("ForceSourcePort").as_string("FALSE")) == "TRUE";
}

ArtNetOutput::ArtNetOutput() : IPOutput() {

    _channels = 510;
    _sequenceNum = 0;
    _datagram = nullptr;
    _forceSourcePort = false;
    memset(_data, 0, sizeof(_data));
}

ArtNetOutput::ArtNetOutput(const ArtNetOutput& from) :
    IPOutput(from)
{
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
    _forceSourcePort = from._forceSourcePort;
}

ArtNetOutput::~ArtNetOutput() {
    if (_datagram != nullptr) delete _datagram;
}
#pragma endregion

#pragma region Static Functions
void ArtNetOutput::SendSync(const std::string& localIP) {

    
    static uint8_t syncdata[ARTNET_SYNCPACKET_LEN];
    static wxIPV4address syncremoteAddr;
    static wxDatagramSocket* syncdatagram = nullptr;

    if (!__initialised) {
        spdlog::debug("Initialising artNet Sync.");

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
        if (localIP == "") {
            localaddr.AnyAddress();
        }
        else {
            localaddr.Hostname(localIP);
        }

        if (syncdatagram != nullptr) {
            delete syncdatagram;
        }

        // using the forced source port for sync packets is buried in a special options as its use is very very rare. Few devices implement this restriction and few users use artnet sync
        wxSocketFlags flags = wxSOCKET_BLOCK; // dont use NOWAIT as it can result in dropped packets
        if (SpecialOptions::GetOption("ForceArtNetSourcePort", "false") == "true") {
            flags |= wxSOCKET_REUSEADDR;
            localaddr.Service(ARTNET_PORT);
        }

        syncdatagram = new wxDatagramSocket(localaddr, flags);
        if (syncdatagram == nullptr) {
            spdlog::error("Error initialising Artnet sync datagram. {}", (const char*)localaddr.IPAddress().c_str());
            return;
        }
        else if (!syncdatagram->IsOk()) {
            spdlog::error("Error initialising Artnet sync datagram ... is network connected? {} OK : FALSE", (const char*)localaddr.IPAddress().c_str());
            delete syncdatagram;
            syncdatagram = nullptr;
            return;
        }
        else if (syncdatagram->Error()) {
            spdlog::error("Error creating Artnet sync datagram => {} : {}. {}", (int)syncdatagram->LastError(), (const char*)DecodeIPError(syncdatagram->LastError()).c_str(), (const char*)localaddr.IPAddress().c_str());
            delete syncdatagram;
            syncdatagram = nullptr;
            return;
        }

        // broadcast ... this is not really in line with the spec
        // I should use the net mask but i cant find a good way to do that
        //syncremoteAddr.BroadcastAddress();
        wxString ipaddrWithUniv = wxString::Format("%d.%d.%d.%d", __ip1, __ip2, __ip3, 255);
        spdlog::debug("artNet Sync broadcasting to {}.", (const char*)ipaddrWithUniv.c_str());
        syncremoteAddr.Hostname(ipaddrWithUniv);
        syncremoteAddr.Service(ARTNET_PORT);
    }

    if (syncdatagram != nullptr) {
        syncdatagram->SendTo(syncremoteAddr, syncdata, ARTNET_SYNCPACKET_LEN);
    }
}

void ArtNetOutput::PrepareDiscovery(Discovery &discovery) {
    wxByte packet[14];
    memset(packet, 0x00, sizeof(packet));
    packet[0] = 'A';
    packet[1] = 'r';
    packet[2] = 't';
    packet[3] = '-';
    packet[4] = 'N';
    packet[5] = 'e';
    packet[6] = 't';
    packet[9] = 0x20;
    packet[11] = 0x0E; // Protocol version Low
    packet[12] = 0x10;
    packet[13] = 0x00; //Critical messages Only

    discovery.AddBroadcast(ARTNET_PORT, [&discovery](wxDatagramSocket* socket, uint8_t *buffer, int len) {
        
        if (buffer[0] == 'A' && buffer[1] == 'r' && buffer[2] == 't' && buffer[3] == '-' && buffer[9] == 0x21) {
            spdlog::debug(" ArtNET Valid response.");
            uint32_t channels = 510;

            auto ip = wxString::Format("%d.%d.%d.%d", (int)buffer[10], (int)buffer[11], (int)buffer[12], (int)buffer[13]);
            spdlog::debug("     From {}.", (const char *)ip.c_str());

            // We cant use Get IP as controller may have responded to multiple discovery requests
            ControllerEthernet* existing = nullptr;
            for (const auto& it : discovery.GetResults()) {
                if (it->ip == ip && it->controller != nullptr && it->controller->GetProtocol() == OUTPUT_ARTNET && it->controller->GetName() == std::string((char*)&buffer[26])) {
                    existing = it->controller;
                }
            }

            if (existing != nullptr) {
                spdlog::debug("        Existing.");
                // a second response from this controller
                for (uint8_t i = 0; i < 4; i++) {
                    if ((buffer[174 + i] & 0x80) != 0) {
                        // we only add universes if sequential ... while this wont always be true it will most of the time and it prevents us having issues
                        // where due to network pathing we see controllers more than once.
                        int u = GetArtNetCombinedUniverse(buffer[18], buffer[19], buffer[190]);
                        if (u == existing->GetOutputs().back()->GetUniverse() + 1) {
                            existing->AddOutput();
                            spdlog::info("        ArtNet adding extra universe ({}) to {} : {}.", GetArtNetCombinedUniverse(buffer[18], buffer[19], buffer[190 + i]), (const char*)ip.c_str(), existing->GetOutputCount());
                        }
                        else                             {
                            spdlog::info("        ArtNet ignoring universe {} as it was not sequential.", u);
                        }
                    }
                }
            }
            else {
                spdlog::info("        ArtNET Discovery adding controller {}.", (const char*)ip.c_str());
                ControllerEthernet* c = new ControllerEthernet(discovery.GetOutputManager(), false);
                c->SetProtocol(OUTPUT_ARTNET);
                c->SetName(std::string((char*)&buffer[26]));
                ArtNetOutput* o = dynamic_cast<ArtNetOutput*>(c->GetOutputs().front());
                if (o != nullptr) {
                    o->SetUniverse(GetArtNetCombinedUniverse(buffer[18], buffer[19], buffer[190]));
                    for (uint8_t i = 1; i < 4; i++) {
                        if ((buffer[174 + i] & 0x80) != 0) {
                            // we only add universes if sequential ... while this wont always be true it will most of the time and it prevents us having issues
                            // where due to network pathing we see controllers more than once.
                            int u = GetArtNetCombinedUniverse(buffer[18], buffer[19], buffer[190]);
                            if (u == c->GetOutputs().back()->GetUniverse() + 1) {
                                c->AddOutput();
                                spdlog::info("    ArtNet adding extra universe ({}) to {} : {}.", GetArtNetCombinedUniverse(buffer[18], buffer[19], buffer[190 + i]), (const char*)ip.c_str(), c->GetOutputCount());
                            }
                            else {
                                spdlog::info("        ArtNet ignoring universe {} as it was not sequential.", u);
                            }
                        }
                    }
                    o->SetChannels(channels);
                }
                c->SetIP(ip.ToStdString());

                discovery.AddController(c);
            }
        } else {
            // non discovery response packet
            spdlog::info("ArtNet Discovery strange packet received.");
        }
    });
    discovery.SendBroadcastData(ARTNET_PORT, packet, sizeof(packet));
}
#pragma endregion

#pragma region Getters and Setters
std::string ArtNetOutput::GetLongDescription() const {

    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "ArtNet {" + GetUniverseString() + "} ";
    res += "[1-" + std::to_string(_channels) + "] ";
    res += "(" + std::to_string(GetStartChannel()) + "-" + std::to_string(GetEndChannel()) + ") ";

    return res;
}

std::string ArtNetOutput::GetUniverseString() const {

    return std::format("{}:{}:{} or {}", GetArtNetNet(), GetArtNetSubnet(), GetArtNetUniverse(), GetUniverse());
}

std::string ArtNetOutput::GetExport() const {

    return std::format(",{},{},,{},{},,,,{},{}",
        GetStartChannel(),
        GetEndChannel(),
        GetType(),
        GetIP(),
        GetUniverse(),
        GetChannels());
}
#pragma endregion

#pragma region Start and Stop
bool ArtNetOutput::Open() {

    

    if (!_enabled) return true;
    if (!ip_utils::IsIPValid(GetResolvedIP())) return false;

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

    OpenDatagram();

    _remoteAddr.Hostname(_ip.c_str());
    _remoteAddr.Service(ARTNET_PORT);

    wxString ipAddr = _remoteAddr.IPAddress();

    // work out our broascast address
    wxArrayString ipc = wxSplit(ipAddr, '.');
    if (__ip1 == -1) {
        __ip1 = wxAtoi(ipc[0]);
        __ip2 = wxAtoi(ipc[1]);
        __ip3 = wxAtoi(ipc[2]);
    }
    else if (wxAtoi(ipc[0]) != __ip1) {
        __ip1 = 255;
        __ip2 = 255;
        __ip3 = 255;
    }
    else {
        if (wxAtoi(ipc[1]) != __ip2) {
            __ip2 = 255;
            __ip3 = 255;
        }
        else {
            if (wxAtoi(ipc[2]) != __ip3) {
                __ip3 = 255;
            }
        }
    }

    __initialised = false;
    spdlog::debug("Artnet broadcast address {}.{}.{}.255", __ip1, __ip2, __ip3);

    _data[16] = (uint8_t)(_channels >> 8);  // Property value count (high)
    _data[17] = (uint8_t)(_channels & 0xff);  // Property value count (low)

    return _ok;
}

void ArtNetOutput::Close() {

    if (_datagram != nullptr) {
        delete _datagram;
        _datagram = nullptr;
    }
}
#pragma endregion

#pragma region Frame Handling
void ArtNetOutput::StartFrame(long msec) {

    

    if (!_enabled) return;

    if (_datagram == nullptr && OutputManager::IsRetryOpen()) {
        OpenDatagram();
        if (_ok) {
            spdlog::debug("ArtNetOutput: Open retry successful");
        }
    }

    _timer_msec = msec;
}

void ArtNetOutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend || _datagram == nullptr) return;

    if (_changed || NeedToOutput(suppressFrames)) {
        _data[12] = _sequenceNum;
        _datagram->SendTo(_remoteAddr, _data, ARTNET_PACKET_LEN - (512 - _channels));
        _sequenceNum = _sequenceNum == 255 ? 0 : _sequenceNum + 1;
        FrameOutput();
        _changed = false;
    }
    else {
        SkipFrame();
    }
}
#pragma endregion

#pragma region Data Setting
void ArtNetOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled) return;
    wxASSERT(channel < _channels);

    if (_data[channel + ARTNET_PACKET_HEADERLEN] != data) {
        _data[channel + ARTNET_PACKET_HEADERLEN] = data;
        _changed = true;
    }
}

void ArtNetOutput::SetManyChannels(int32_t channel, unsigned char* data, size_t size) {

    if (!_enabled) return;
    wxASSERT((size_t)channel + size <= (size_t)_channels);

    size_t chs = (std::min)((int32_t)size, _channels - channel);

    if (memcmp(&_data[channel + ARTNET_PACKET_HEADERLEN], data, chs) == 0) {
        // nothing has changed
    }
    else {
        memcpy(&_data[channel + ARTNET_PACKET_HEADERLEN], data, chs);
        _changed = true;
    }
}

void ArtNetOutput::AllOff() {

    if (!_enabled) return;
    memset(&_data[ARTNET_PACKET_HEADERLEN], 0x00, _channels);
    _changed = true;
}
#pragma endregion


pugi::xml_node ArtNetOutput::Save(pugi::xml_node parent)
{
    pugi::xml_node node = parent.append_child("network");
    IPOutput::SaveAttr(node);
    if (_forceSourcePort) {
        node.append_attribute("ForceSourcePort") = "TRUE";
    }
    return node;
}
