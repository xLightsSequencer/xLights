
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>

#include "ArtNetOutput.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"
#include "ControllerEthernet.h"
#include "../OutputModelManager.h"
#include "../SpecialOptions.h"
#include "../utils/ip_utils.h"
#ifndef EXCLUDENETWORKUI
#include "../models/ModelManager.h"
#endif
#include <log4cpp/Category.hh>

#include "../Discovery.h"

#pragma region Static Variables
int ArtNetOutput::__ip1 = -1;
int ArtNetOutput::__ip2 = -1;
int ArtNetOutput::__ip3 = -1;
bool ArtNetOutput::__initialised = false;
#pragma endregion

#pragma region Private Functions
void ArtNetOutput::OpenDatagram() {

    log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

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
        logger_base.error("Error initialising Artnet datagram for %s %d:%d:%d. %s", (const char*)_ip.c_str(), GetArtNetNet(), GetArtNetSubnet(), GetArtNetUniverse(), (const char*)localaddr.IPAddress().c_str());
        _ok = false;
    }
    else if (!_datagram->IsOk()) {
        logger_base.error("Error initialising Artnet datagram for %s %d:%d:%d. %s OK : FALSE", (const char*)_ip.c_str(), GetArtNetNet(), GetArtNetSubnet(), GetArtNetUniverse(), (const char*)localaddr.IPAddress().c_str());
        delete _datagram;
        _datagram = nullptr;
        _ok = false;
    }
    else if (_datagram->Error()) {
        logger_base.error("Error creating Artnet datagram => %d : %s. %s", _datagram->LastError(), (const char*)DecodeIPError(_datagram->LastError()).c_str(), (const char*)localaddr.IPAddress().c_str());
        delete _datagram;
        _datagram = nullptr;
        _ok = false;
    }
}
#pragma endregion

#pragma region Constructors and Destructors
ArtNetOutput::ArtNetOutput(wxXmlNode* node, bool isActive) : IPOutput(node, isActive) {

    if (_channels > 512) SetChannels(512);
    if (_autoSize_CONVERT) _autoSize_CONVERT = false;
    _sequenceNum = 0;
    _datagram = nullptr;
    _forceSourcePort = false;
    memset(_data, 0, sizeof(_data));

    _forceSourcePort = node->GetAttribute("ForceSourcePort", "FALSE") == "TRUE";
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

    log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static uint8_t syncdata[ARTNET_SYNCPACKET_LEN];
    static wxIPV4address syncremoteAddr;
    static wxDatagramSocket* syncdatagram = nullptr;

    if (!__initialised) {
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
            logger_base.error("Error initialising Artnet sync datagram. %s", (const char*)localaddr.IPAddress().c_str());
            return;
        }
        else if (!syncdatagram->IsOk()) {
            logger_base.error("Error initialising Artnet sync datagram ... is network connected? %s OK : FALSE", (const char*)localaddr.IPAddress().c_str());
            delete syncdatagram;
            syncdatagram = nullptr;
            return;
        }
        else if (syncdatagram->Error()) {
            logger_base.error("Error creating Artnet sync datagram => %d : %s. %s", syncdatagram->LastError(), (const char*)DecodeIPError(syncdatagram->LastError()).c_str(), (const char*)localaddr.IPAddress().c_str());
            delete syncdatagram;
            syncdatagram = nullptr;
            return;
        }

        // broadcast ... this is not really in line with the spec
        // I should use the net mask but i cant find a good way to do that
        //syncremoteAddr.BroadcastAddress();
        wxString ipaddrWithUniv = wxString::Format("%d.%d.%d.%d", __ip1, __ip2, __ip3, 255);
        logger_base.debug("artNet Sync broadcasting to %s.", (const char*)ipaddrWithUniv.c_str());
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
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        if (buffer[0] == 'A' && buffer[1] == 'r' && buffer[2] == 't' && buffer[3] == '-' && buffer[9] == 0x21) {
            logger_base.debug(" ArtNET Valid response.");
            uint32_t channels = 510;

            auto ip = wxString::Format("%d.%d.%d.%d", (int)buffer[10], (int)buffer[11], (int)buffer[12], (int)buffer[13]);
            logger_base.debug("     From %s.", (const char *)ip.c_str());

            // We cant use Get IP as controller may have responded to multiple discovery requests
            ControllerEthernet* existing = nullptr;
            for (const auto& it : discovery.GetResults()) {
                if (it->ip == ip && it->controller != nullptr && it->controller->GetProtocol() == OUTPUT_ARTNET && it->controller->GetName() == std::string((char*)&buffer[26])) {
                    existing = it->controller;
                }
            }

            if (existing != nullptr) {
                logger_base.debug("        Existing.");
                // a second response from this controller
                for (uint8_t i = 0; i < 4; i++) {
                    if ((buffer[174 + i] & 0x80) != 0) {
                        // we only add universes if sequential ... while this wont always be true it will most of the time and it prevents us having issues
                        // where due to network pathing we see controllers more than once.
                        int u = GetArtNetCombinedUniverse(buffer[18], buffer[19], buffer[190]);
                        if (u == existing->GetOutputs().back()->GetUniverse() + 1) {
                            existing->AddOutput();
                            logger_base.info("        ArtNet adding extra universe (%d) to %s : %d.", GetArtNetCombinedUniverse(buffer[18], buffer[19], buffer[190 + i]), (const char*)ip.c_str(), existing->GetOutputCount());
                        }
                        else                             {
                            logger_base.info("        ArtNet ignoring universe %d as it was not sequential.", u);
                        }
                    }
                }
            }
            else {
                logger_base.info("        ArtNET Discovery adding controller %s.", (const char*)ip.c_str());
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
                                logger_base.info("    ArtNet adding extra universe (%d) to %s : %d.", GetArtNetCombinedUniverse(buffer[18], buffer[19], buffer[190 + i]), (const char*)ip.c_str(), c->GetOutputCount());
                            }
                            else {
                                logger_base.info("        ArtNet ignoring universe %d as it was not sequential.", u);
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
            logger_base.info("ArtNet Discovery strange packet received.");
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
    res += "[1-" + std::string(wxString::Format(wxT("%i"), _channels)) + "] ";
    res += "(" + std::string(wxString::Format(wxT("%i"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ") ";

    return res;
}

std::string ArtNetOutput::GetUniverseString() const {

    return wxString::Format(wxT("%i:%i:%i or %d"), GetArtNetNet(), GetArtNetSubnet(), GetArtNetUniverse(), GetUniverse()).ToStdString();
}

std::string ArtNetOutput::GetExport() const {

    return wxString::Format(",%ld,%ld,,%s,%s,,,,%d,%i",
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

    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_enabled) return true;
    if (!ip_utils::IsIPValid(_resolvedIp)) return false;

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
    logger_base.debug("Artnet broadcast address %d.%d.%d.255", __ip1, __ip2, __ip3);

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

    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_enabled) return;

    if (_datagram == nullptr && OutputManager::IsRetryOpen()) {
        OpenDatagram();
        if (_ok) {
            logger_base.debug("ArtNetOutput: Open retry successful");
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
    wxASSERT(channel + size <= _channels);

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

#pragma region UI
#ifndef EXCLUDENETWORKUI
void ArtNetOutput::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    IPOutput::UpdateProperties(propertyGrid, c, modelManager, expandProperties);
    ControllerEthernet *ce = dynamic_cast<ControllerEthernet*>(c);

    auto p = propertyGrid->GetProperty("Universes");
    if (p) {
        p->SetValue((int)c->GetOutputs().size());
        if (c->IsAutoSize()) {
            p->ChangeFlag(wxPG_PROP_READONLY, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Universes Count cannot be changed when an output is set to Auto Size.");
        } else {
            p->SetEditor("SpinCtrl");
            p->ChangeFlag(wxPG_PROP_READONLY, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
    p = propertyGrid->GetProperty("UniversePerString");
    if (p) {
        p->SetValue(ce->IsUniversePerString());
        if (ce->IsAutoSize() && ce->SupportsUniversePerString()) {
            p->Hide(false);
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("UniversesDisplay");
    if (p) {
        if (ce->GetOutputs().size() > 1) {
            p->SetValue(ce->GetOutputs().front()->GetUniverseString() + " - " + ce->GetOutputs().back()->GetUniverseString());
            p->Hide(false);
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("IndivSizes");
    if (p) {
        if (ce->IsAutoSize()) {
            p->ChangeFlag(wxPG_PROP_READONLY, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Individual Sizes cannot be changed when an output is set to Auto Size.");
        } else {
            bool v = !ce->AllSameSize() || ce->IsForcingSizes() || ce->IsUniversePerString();
            p->SetValue(v);
            p->ChangeFlag(wxPG_PROP_READONLY, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
    if (!ce->AllSameSize() || ce->IsForcingSizes()) {
        p = propertyGrid->GetProperty("Channels");
        if (p) {
            p->Hide(true);
        }
        auto p2 = propertyGrid->GetProperty("Sizes");
        if (p2) {
            p2->Hide(false);
            if (ce->IsExpanded()) {
                expandProperties.push_back(p2);
            }
            while (propertyGrid->GetFirstChild(p2)) {
                propertyGrid->RemoveProperty(propertyGrid->GetFirstChild(p2));
            }
            for (const auto& it : ce->GetOutputs()) {
                p = propertyGrid->AppendIn(p2, new wxUIntProperty(it->GetUniverseString(), "Channels/" + it->GetUniverseString(), it->GetChannels()));
                p->SetAttribute("Min", 1);
                p->SetAttribute("Max", it->GetMaxChannels());
                p->SetEditor("SpinCtrl");
                auto modelsOnUniverse = modelManager->GetModelsOnChannels(it->GetStartChannel(), it->GetEndChannel(), 4);
                p->SetHelpString(wxString::Format("[%d-%d]\n", it->GetStartChannel(), it->GetEndChannel()) + modelsOnUniverse);
                if (modelsOnUniverse != "") {
                    if (IsDarkMode()) {
                        p->SetBackgroundColour(wxColour(104, 128, 79));
                    } else {
                        p->SetBackgroundColour(wxColour(208, 255, 158));
                    }
                }
            }
        }
    } else {
        p = propertyGrid->GetProperty("Channels");
        if (p) {
            p->Hide(false);
            p->SetValue(GetChannels());
            if (ce->IsAutoSize()) {
                p->ChangeFlag(wxPG_PROP_READONLY, true);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
            } else {
                p->SetEditor("SpinCtrl");
                p->ChangeFlag(wxPG_PROP_READONLY, false);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
                p->SetHelpString("");
            }
        }
        auto p2 = propertyGrid->GetProperty("Sizes");
        if (p2) {
            p2->Hide(true);
        }
    }
}

void ArtNetOutput::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty *before , Controller *c, bool allSameSize, std::list<wxPGProperty*>& expandProperties)
{
    IPOutput::AddProperties(propertyGrid, before, c, allSameSize, expandProperties);
    auto p = propertyGrid->Insert(before, new wxBoolProperty("Force source port", "ForceSourcePort", _forceSourcePort));
    p->SetEditor("CheckBox");
    p->SetHelpString("You should only set this option if your ArtNet device is not seeing the ArtNet packets because it strictly requires that packets come from port 0x1936.");
    
    p = propertyGrid->Insert(before, new wxUIntProperty("Start Universe", "Universe", GetUniverse()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 32767);
    p->SetEditor("SpinCtrl");
    
    
    p = propertyGrid->Insert(before, new wxUIntProperty("Universe Count", "Universes", c->GetOutputs().size()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1000);

    p = propertyGrid->Insert(before, new wxBoolProperty("Universe Per String", "UniversePerString", false));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxStringProperty("Universes", "UniversesDisplay", ""));
    p->ChangeFlag(wxPG_PROP_READONLY, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));


    p = propertyGrid->Insert(before, new wxBoolProperty("Individual Sizes", "IndivSizes", false));
    p->SetEditor("CheckBox");
    
    p = propertyGrid->Insert(before, new wxUIntProperty("Channels per Universe", "Channels", GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", GetMaxChannels());
    
    propertyGrid->Insert(before, new wxPropertyCategory("Sizes", "Sizes"));
}

bool ArtNetOutput::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller *c)
{
    if (IPOutput::HandlePropertyEvent(event, outputModelManager, c)) return true;
    
    wxString const name = event.GetPropertyName();
    if (name == "ForceSourcePort") {
        _forceSourcePort = event.GetValue().GetBool();
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ArtNetOutput::HandlePropertyEvent::ForceSourcePort");
        return true;
    }

    return false;
}
void ArtNetOutput::RemoveProperties(wxPropertyGrid* propertyGrid) {
    IPOutput::RemoveProperties(propertyGrid);
    propertyGrid->DeleteProperty("ForceSourcePort");
    propertyGrid->DeleteProperty("Universe");
    propertyGrid->DeleteProperty("Universes");
    propertyGrid->DeleteProperty("UniversePerString");
    propertyGrid->DeleteProperty("UniversesDisplay");
    propertyGrid->DeleteProperty("IndivSizes");
    propertyGrid->DeleteProperty("Channels");
    propertyGrid->DeleteProperty("Sizes");
}

#endif
#pragma endregion

wxXmlNode* ArtNetOutput::Save()
{
    auto node = IPOutput::Save();
    if (_forceSourcePort) {
        node->AddAttribute("ForceSourcePort", "TRUE");
    }
    return node;
}
