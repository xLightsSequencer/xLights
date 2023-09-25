
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#ifdef LINUX
#include <arpa/inet.h>
#endif
#include <list>

#include <wx/xml/xml.h>
#include <wx/file.h>
#include <wx/filename.h>

#include "../controllers/Falcon.h"
#include "ZCPPOutput.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"
#include "ControllerEthernet.h"
#include "../OutputModelManager.h"
#include "../ExternalHooks.h"
#include "../utils/ip_utils.h"

#ifndef EXCLUDENETWORKUI
#include "../controllers/Falcon.h"
#endif

#ifndef EXCLUDEDISCOVERY
#include "../Discovery.h"
#endif

#include <log4cpp/Category.hh>

#pragma region Private Functions
void ZCPPOutput::ExtractUsedChannelsFromModelData(Controller* c) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Extracting used channels from model data. %s", (const char*)c->GetName().c_str());

    _usedChannels = 1;

    for (const auto& it : _modelData) {
        int ports = it->Configuration.ports;
        wxASSERT(ports <= ZCPP_CONFIG_MAX_PORT_PER_PACKET);
        if (ports > ZCPP_CONFIG_MAX_PORT_PER_PACKET) {
            logger_base.warn("ZCPP file corrupt. Abandoning read.");
            _usedChannels = 1;
            if (c->IsAutoSize()) SetChannels(1);
            return;
        }
        ZCPP_PortConfig* port = it->Configuration.PortConfig;
        for (int i = 0; i < ports; i++) {
            long start = htonl(port->startChannel);
            long len = htonl(port->channels);
            if (start + len - 1 > _usedChannels) {
                _usedChannels = start + len;
            }
            port++;
        }
        logger_base.debug("    End of config packet ... channels %ld.", (long)_usedChannels);
    }

    wxASSERT(_channels < 100000); // catch weird numbers
    if (_usedChannels != _channels && c->IsAutoSize()) {
        logger_base.debug("    usedChannels %ld != _channels %ld and autosize.", (long)_usedChannels, (long)_channels);
        SetChannels(_usedChannels);
        wxASSERT(_channels < 100000); // catch weird numbers
    }
    else if (_usedChannels > _channels) {
        logger_base.debug("    usedChannels %ld > _channels %ld and NO autosize.", (long)_usedChannels, (long)_channels);
        // cant use more channels than there are
        _usedChannels = _channels;
    }
    logger_base.debug("    usedChannels %ld, _channels %ld.", (long)_usedChannels, (long)_channels);
}

void ZCPPOutput::DeserialiseProtocols(const std::string& protocols) {

    auto ps = wxSplit(protocols, '|');

    for (const auto& it : ps) {
        AddProtocol(it);
    }
}

std::string ZCPPOutput::SerialiseProtocols() {

    std::string res;
    for (const auto& it : _protocols) {
        if (res != "") res += "|";
        res += it;
    }
    return res;
}
#pragma endregion

#pragma region Constructors and Destructors
ZCPPOutput::ZCPPOutput(Controller* c, wxXmlNode* node, std::string showdir) : IPOutput(node, c->IsActive()) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _usedChannels = _channels;
    if (_channels == 0) {
        _data = nullptr;
    }
    else {
        _data = (wxByte*)malloc(_channels);
        if (_data != nullptr) memset(_data, 0x00, _channels);
    }
    memset(&_packet, 0, sizeof(_packet));
    _vendor = wxAtoi(node->GetAttribute("Vendor", "65535"));
    _model = wxAtoi(node->GetAttribute("Model", "65535"));
    _priority = wxAtoi(node->GetAttribute("Priority", "100"));
    _supportsVirtualStrings = node->GetAttribute("SupportsVirtualStrings", "FALSE") == "TRUE";
    _supportsSmartRemotes = node->GetAttribute("SupportsSmartRemotes", "FALSE") == "TRUE";
    _multicast = node->GetAttribute("Multicast", "FALSE") == "TRUE";
    _dontConfigure = node->GetAttribute("DontConfigure", "FALSE") == "TRUE";
    DeserialiseProtocols(node->GetAttribute("Protocols", ""));

    if (!_dontConfigure) {
        wxString fileName = GetIP();
        fileName.Replace(".", "_");
        fileName += ".zcpp";
        fileName = showdir + wxFileName::GetPathSeparator() + fileName;

        if (FileExists(fileName)) {
            wxFile zf;
            if (zf.Open(fileName)) {
                uint8_t tag[4];
                zf.Read(tag, sizeof(tag));
                if (tag[0] != 'Z' || tag[1] != 'C' || tag[2] != 'P' || tag[3] != 'P') {
                    logger_base.warn("ZCPP Model data file %s did not contain opening tag.", (const char*)fileName.c_str());
                }
                else {
                    while (!zf.Eof()) {
                        uint8_t type;
                        zf.Read(&type, sizeof(type));
                        switch (type) {
                        case 0x00:
                        {
                            uint8_t b1;
                            uint8_t b2;
                            zf.Read(&b1, sizeof(b1));
                            zf.Read(&b2, sizeof(b2));
                            uint16_t size = (b1 << 8) + b2;
                            if (size == sizeof(ZCPP_packet_t)) {
                                ZCPP_packet_t* modelPacket = (ZCPP_packet_t*)malloc(sizeof(ZCPP_packet_t));
                                zf.Read(modelPacket, sizeof(ZCPP_packet_t));
                                _modelData.push_back(modelPacket);
                            }
                            else {
                                logger_base.warn("ZCPP Model data file %s unrecognised model data size.", (const char*)fileName.c_str());
                                zf.SeekEnd();
                            }
                        }
                        break;
                        case 0x01:
                        {
                            uint8_t b1;
                            uint8_t b2;
                            zf.Read(&b1, sizeof(b1));
                            zf.Read(&b2, sizeof(b2));
                            uint16_t size = (b1 << 8) + b2;
                            if (size == sizeof(ZCPP_packet_t)) {
                                ZCPP_packet_t* descPacket = (ZCPP_packet_t*)malloc(sizeof(ZCPP_packet_t));
                                zf.Read(descPacket, sizeof(ZCPP_packet_t));
                                _extraConfig.push_back(descPacket);
                            }
                            else {
                                logger_base.warn("ZCPP Model data file %s unrecognised extra config size.", (const char*)fileName.c_str());
                                zf.SeekEnd();
                            }
                        }
                        break;
                        case 0xFF:
                            zf.SeekEnd();
                            break;
                        default:
                            wxASSERT(false);
                            logger_base.warn("ZCPP Model data file %s unrecognised type %d.", (const char*)fileName.c_str(), type);
                            break;
                        }
                    }
                    logger_base.debug("ZCPP Model data file %s loaded.", (const char*)fileName.c_str());
                }
                zf.Close();
            }
            else {
                logger_base.warn("ZCPP Model data file %s could not be opened.", (const char*)fileName.c_str());
            }
            if (c != nullptr) ExtractUsedChannelsFromModelData(c);
        }
        else {
             logger_base.warn("ZCPP Model data file %s not found.", (const char*)fileName.c_str());
        }
    }
    else {
        logger_base.warn("ZCPP set to not configure the controller.");
    }
}

ZCPPOutput::ZCPPOutput() : IPOutput() {

    _channels = 1;
    _usedChannels = 1;
    _universe = -1;
    _autoSize_CONVERT = true;
    _data = (wxByte*)malloc(_channels);
    if (_data != nullptr) memset(_data, 0x00, _channels);
    memset(&_packet, 0, sizeof(_packet));
}

ZCPPOutput::ZCPPOutput(const ZCPPOutput& from) : IPOutput(from) {

    _channels = from._channels;
    _universe = from._universe;
    _autoSize_CONVERT = from._autoSize_CONVERT;
    _usedChannels = from._usedChannels;
    if (_channels == 0){
        _data = nullptr;
    }
    else {
        _data = (wxByte*)malloc(_channels);
        if (_data != nullptr) memset(_data, 0x00, _channels);
    }
    memset(&_packet, 0, sizeof(_packet));
    _sequenceNum = from._sequenceNum;
    _remoteAddr = from._remoteAddr;
    _datagram = nullptr;
    _lastSecond = -1;
    _vendor = from._vendor;
    _model = from._model;
    _priority = from._priority;
    _supportsVirtualStrings = from._supportsVirtualStrings;
    _supportsSmartRemotes = from._supportsSmartRemotes;
    _multicast = from._multicast;
    _dontConfigure = from._dontConfigure;
    _protocols = from._protocols;
}

ZCPPOutput::~ZCPPOutput() {
    while (_modelData.size() > 0) {
        delete _modelData.front();
        _modelData.pop_front();
    }
    while (_extraConfig.size() > 0) {
        delete _extraConfig.front();
        _extraConfig.pop_front();
    }
    if (_datagram != nullptr) delete _datagram;
    if (_data != nullptr) free(_data);
}

wxXmlNode* ZCPPOutput::Save() {

    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");

    node->AddAttribute("Vendor", wxString::Format("%d", _vendor));
    node->AddAttribute("Model", wxString::Format("%d", _model));
    node->AddAttribute("Priority", wxString::Format("%d", _priority));
    if (_supportsVirtualStrings) {
        node->AddAttribute("SupportsVirtualStrings", "TRUE");
    }
    else {
        node->AddAttribute("SupportsVirtualStrings", "FALSE");
    }
    if (_supportsSmartRemotes) {
        node->AddAttribute("SupportsSmartRemotes", "TRUE");
    }
    else {
        node->AddAttribute("SupportsSmartRemotes", "FALSE");
    }
    if (_dontConfigure)node->AddAttribute("DontConfigure", "TRUE");
    if (_multicast)node->AddAttribute("Multicast", "TRUE");
    node->AddAttribute("Protocols", SerialiseProtocols());
    IPOutput::Save(node);

    return node;
}
#pragma endregion

#pragma region Static Functions
void ZCPPOutput::SendSync(const std::string& localIP) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    static ZCPP_packet_t syncdata;
    static uint8_t syncSequenceNum = 0;
    static bool initialised = false;
    static wxIPV4address syncremoteAddr;
    static wxDatagramSocket* syncdatagram = nullptr;

    if (!initialised) {
        logger_base.debug("Initialising ZCPP Sync.");

        initialised = true;

        memset(&syncdata, 0x00, sizeof(syncdata));
        memcpy(syncdata.Sync.Header.token, ZCPP_token, sizeof(ZCPP_token));
        syncdata.Sync.Header.type = ZCPP_TYPE_SYNC;
        syncdata.Sync.Header.protocolVersion = ZCPP_CURRENT_PROTOCOL_VERSION;

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

        syncdatagram = new wxDatagramSocket(localaddr, wxSOCKET_BLOCK); // dont use NOWAIT as it can result in dropped packets

        if (syncdatagram == nullptr) {
            logger_base.error("Error initialising ZCPP sync datagram.");
        }
        else if (!syncdatagram->IsOk()) {
            logger_base.error("Error initialising ZCPP sync datagram ... is network connected? OK : FALSE");
            delete syncdatagram;
            syncdatagram = nullptr;
        }
        else if (syncdatagram->Error()) {
            logger_base.error("Error creating ZCPP sync datagram => %d : %s.", syncdatagram->LastError(), (const char*)DecodeIPError(syncdatagram->LastError()).c_str());
            delete syncdatagram;
            syncdatagram = nullptr;
        }

        // multicast - universe number must be in lower 2 bytes
        wxString ipaddrWithUniv = ZCPP_MULTICAST_ADDRESS;
        syncremoteAddr.Hostname(ipaddrWithUniv);
        syncremoteAddr.Service(ZCPP_PORT);
    }

    syncdata.Sync.sequenceNumber = syncSequenceNum++;   // sequence number

    // bail if we dont have a datagram to use
    if (syncdatagram != nullptr) {
        syncdatagram->SendTo(syncremoteAddr, &syncdata, ZCPP_GetPacketActualSize(syncdata));
    }
}

void ZCPPOutput::InitialiseExtraConfigPacket(ZCPP_packet_t* packet, int seq, uint8_t priority) {

    memset(packet, 0x00, sizeof(ZCPP_packet_t));
    memcpy(packet->ExtraData.Header.token, ZCPP_token, sizeof(ZCPP_token));
    packet->ExtraData.Header.type = ZCPP_TYPE_EXTRA_DATA;
    packet->ExtraData.Header.protocolVersion = ZCPP_CURRENT_PROTOCOL_VERSION;
    packet->ExtraData.sequenceNumber = ntohs(seq);
    packet->ExtraData.priority = priority;
}

void ZCPPOutput::InitialiseModelDataPacket(ZCPP_packet_t* packet, int seq, uint8_t priority, const std::string& description) {

    memset(packet, 0x00, sizeof(ZCPP_packet_t));
    memcpy(packet->Configuration.Header.token, ZCPP_token, sizeof(ZCPP_token));
    packet->Configuration.Header.type = ZCPP_TYPE_CONFIG;
    packet->Configuration.Header.protocolVersion = ZCPP_CURRENT_PROTOCOL_VERSION;
    packet->Configuration.sequenceNumber = ntohs(seq);
    packet->Configuration.priority = priority;
    strncpy(packet->Configuration.userControllerName, description.c_str(), sizeof(packet->Configuration.userControllerName));
}

std::string ZCPPOutput::DecodeProtocol(int protocol) {

    switch (protocol) {
    case 0x00:
        return "ws2811";
    case 0x01:
        return "gece";
    case 0x02:
        return "dmx";
    case 0x03:
        return "lx1203";
    case 0x04:
        return "tls3001";
    case 0x05:
        return "lpd6803";
    case 0x06:
        return "ws2801";
    case 0x07:
        return "sm16716";
    case 0x08:
        return "mb16020";
    case 0x09:
        return "my9231";
    case 0x0a:
        return "apa102";
    case 0x0b:
        return "my9221";
    case 0x0c:
        return "sk6812";
    case 0x0d:
        return "ucs1903";
    case 0x0e:
        return "tm18xx";
    case 0x0f:
        return "renard";
    case 0x10:
        return "lpd8806";
    case 0x11:
        return "dm412";
    case 0x12:
        return "p9813";
    case 0x13:
        return "lor";
    default:
        return "unknown";
    }
}

int ZCPPOutput::EncodeProtocol(const std::string& protocol) {

    auto p = wxString(protocol).Lower();
    if (p == "ws2811") return 0x00;
    if (p == "gece") return 0x01;
    if (p == "dmx") return 0x02;
    if (p == "lx1203") return 0x03;
    if (p == "tls3001") return 0x04;
    if (p == "lpd6803") return 0x05;
    if (p == "ws2801") return 0x06;
    if (p == "sm16716") return 0x07;
    if (p == "mb16020") return 0x08;
    if (p == "my9231") return 0x09;
    if (p == "apa102") return 0x0a;
    if (p == "my9221") return 0x0b;
    if (p == "sk6812") return 0x0c;
    if (p == "ucs1903") return 0x0d;
    if (p == "tm18xx") return 0x0e;
    if (p == "renard") return 0x0f;
    if (p == "lpd8806") return 0x10;
    if (p == "dm412") return 0x11;
    if (p == "p9813") return 0x12;
    if (p == "lor") return 0x13;
    return 0xFE;
}

int ZCPPOutput::EncodeColourOrder(const std::string& colourOrder) {

    if (colourOrder == "RGB") return 0;
    if (colourOrder == "RBG") return 1;
    if (colourOrder == "GRB") return 2;
    if (colourOrder == "GBR") return 3;
    if (colourOrder == "BRG") return 4;
    if (colourOrder == "BGR") return 5;
    return 0;
}

#ifndef EXCLUDEDISCOVERY
void ZCPPOutput::PrepareDiscovery(Discovery &discovery) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    ZCPP_packet_t packet;
    memset(&packet, 0x00, sizeof(packet));
    memcpy(packet.Discovery.Header.token, ZCPP_token, sizeof(ZCPP_token));
    packet.Discovery.Header.type = ZCPP_TYPE_DISCOVERY;
    packet.Discovery.Header.protocolVersion = ZCPP_CURRENT_PROTOCOL_VERSION;

    discovery.AddBroadcast(ZCPP_PORT, [&discovery] (wxDatagramSocket* socket, uint8_t *buffer, int len) {
        ZCPP_packet_t response;
        memcpy(&response, buffer, len);

        if (memcmp(&response, ZCPP_token, sizeof(ZCPP_token)) == 0 && response.DiscoveryResponse.Header.type == ZCPP_TYPE_DISCOVERY_RESPONSE) {
            logger_base.debug(" Valid response.");

            ControllerEthernet* controller = new ControllerEthernet(discovery.GetOutputManager(), false);
            controller->SetProtocol(OUTPUT_ZCPP);

            int vendor = ZCPP_FromWire16(response.DiscoveryResponse.vendor);
            logger_base.debug("   Vendor %d", vendor);
            int model = ZCPP_FromWire16(response.DiscoveryResponse.model);
            logger_base.debug("   Model %d", model);
            switch (vendor) {
            case ZCPP_VENDOR_FALCON: {
                controller->SetVendor("Falcon");
                int m, v;
                std::string mod = "";
                Falcon::DecodeModelVersion(model, m, v);
                if (m == 48) {
                    mod = "F48";
                } else {
                    mod = wxString::Format("F%dV%d", m, v);
                }
                controller->SetModel(mod);
            }
            break;
            case ZCPP_VENDOR_ESPIXELSTICK:
                controller->SetVendor("ESPixelStick");
                break;
            default:
                break;
            }

            ZCPPOutput* o = dynamic_cast<ZCPPOutput*>(controller->GetOutputs().front());

            logger_base.debug("   Firmware %s", response.DiscoveryResponse.firmwareVersion);

            auto ip = wxString::Format("%d.%d.%d.%d",
                (int)(uint8_t)(response.DiscoveryResponse.ipv4Address & 0xFF),
                (int)(uint8_t)((response.DiscoveryResponse.ipv4Address & 0xFF00) >> 8),
                (int)(uint8_t)((response.DiscoveryResponse.ipv4Address & 0xFF0000) >> 16),
                (int)(uint8_t)((response.DiscoveryResponse.ipv4Address & 0xFF000000) >> 24));
            controller->SetIP(ip.ToStdString());
            logger_base.debug("   IP %s", (const char*)ip.c_str());

            controller->SetName(response.DiscoveryResponse.userControllerName);
            logger_base.debug("   Name %s", (const char*)controller->GetName().c_str());

            uint32_t channels = ZCPP_FromWire32(response.DiscoveryResponse.maxTotalChannels);
            logger_base.debug("   Channels %ld", channels);

            bool supportsVirtualStrings = (ZCPP_FromWire16(response.DiscoveryResponse.flags) & ZCPP_DISCOVERY_FLAG_SUPPORTS_VIRTUAL_STRINGS) != 0;
            logger_base.debug("   Supports Virtual Strings %d", supportsVirtualStrings);
            o->SetSupportsVirtualStrings(supportsVirtualStrings);

            bool supportsSmartRemotes = (ZCPP_FromWire16(response.DiscoveryResponse.flags) & ZCPP_DISCOVERY_FLAG_SUPPORTS_SMART_REMOTES) != 0;
            logger_base.debug("   Supports Smart Remotes %d", supportsSmartRemotes);
            o->SetSupportsSmartRemotes(supportsSmartRemotes);

            bool dontConfigure = (ZCPP_FromWire16(response.DiscoveryResponse.flags) & ZCPP_DISCOVERY_FLAG_CONFIGURATION_LOCKED) != 0;
            logger_base.debug("   Doesn't want to receive configuration %d", dontConfigure);
            o->SetDontConfigure(dontConfigure);

            bool multicast = (ZCPP_FromWire16(response.DiscoveryResponse.flags) & ZCPP_DISCOVERY_FLAG_SEND_DATA_AS_MULTICAST) != 0;
            logger_base.debug("   Wants to receive data multicast %d", multicast);
            o->SetMulticast(multicast);

            logger_base.info("ZCPP Discovery found a new controller %s.", (const char*)controller->GetIP().c_str());

            uint32_t mask = 0x00000001;
            uint32_t dp = ZCPP_FromWire32(response.DiscoveryResponse.protocolsSupported);
            for (int i = 0; i < 32; i++) {
                if ((dp & mask) != 0) {
                    o->AddProtocol(ZCPPOutput::DecodeProtocol(ZCPP_ConvertDiscoveryProtocolToProtocol(dp & mask)));
                    logger_base.debug("   Supports Protocol %s", (const char*)ZCPPOutput::DecodeProtocol(ZCPP_ConvertDiscoveryProtocolToProtocol(dp & mask)).c_str());
                }
                mask = mask << 1;
            }

            controller->SetAutoSize(true, nullptr);
            controller->SetAutoLayout(true);
            o->SetChannels(1 /*channels*/); // Set this to one as it defaults to auto size

            logger_base.info("ZCPP Discovery adding controller %s.", (const char*)controller->GetIP().c_str());
            discovery.AddController(controller);
        } else {
            // non discovery response packet
            logger_base.info("ZCPP Discovery strange packet received.");
        }
    });

    logger_base.info("ZCPP sending discovery packet.");
    discovery.SendData(ZCPP_PORT, ZCPP_MULTICAST_ADDRESS, (uint8_t*)&packet, ZCPP_GetPacketActualSize(packet));
}
#endif

#ifndef EXCLUDENETWORKUI
#endif

wxArrayString ZCPPOutput::GetVendors() {

    wxArrayString res;
    res.push_back("Falcon");
    res.push_back("ESP Pixel Stick");
    res.push_back("Unknown");
    return res;
}

int ZCPPOutput::EncodeVendor(const std::string& vendor) {

    if (vendor == "Falcon") return ZCPP_VENDOR_FALCON;
    if (vendor == "FPP") return ZCPP_VENDOR_FPP;
    if (vendor == "ESP Pixel Stick") return ZCPP_VENDOR_ESPIXELSTICK;
    return ZCPP_VENDOR_UNKNOWN;
}

std::string ZCPPOutput::DecodeVendor(int vendor) {

    switch (vendor) {
    case ZCPP_VENDOR_FALCON:
        return "Falcon";
    case ZCPP_VENDOR_FPP:
        return "FPP";
    case ZCPP_VENDOR_ESPIXELSTICK:
        return "ESP Pixel Stick";
    default:
        return "Unknown";
    }
}
#pragma endregion

#pragma region Getters and Setters
std::string ZCPPOutput::GetLongDescription() const {

    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "ZCPP ";
    res += "[1-" + std::string(wxString::Format(wxT("%d"), _channels)) + "] ";
    res += "(" + std::string(wxString::Format(wxT("%d"), GetStartChannel())) + "-" +
        std::string(wxString::Format(wxT("%d"), GetEndChannel())) + ") ";

    return res;
}

void ZCPPOutput::SetChannels(int32_t channels) {

    if (channels != _channels) {
        _channels = channels;
        _dirty = true;
        if (_data != nullptr) {
            free(_data);
        }
        _data = (wxByte*)malloc(_channels);
        if (_data != nullptr) memset(_data, 0x00, _channels);
        if (_usedChannels > _channels) _usedChannels = _channels;
    }
}

void ZCPPOutput::AllOn() {

    // turn everything to a dim white
    memset(_data, 0x20, _channels);
    _changed = true;
}

bool ZCPPOutput::SetModelData(Controller* c, std::list<ZCPP_packet_t*> modelData, std::list<ZCPP_packet_t*> extraConfig, std::string showDir) {

    if (_dontConfigure) return false;

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("ZCPP setting the model data %s.", (const char*)c->GetName().c_str());

    wxString fileName = GetIP();

    if (_multicast) {
        fileName = ZCPP_MULTICAST_DATA_ADDRESS + wxString(GetIP()).AfterLast('.');
    }

    fileName.Replace(".", "_");
    fileName += ".zcpp";
    fileName = showDir + wxFileName::GetPathSeparator() + fileName;

    if (_modelData.size() != modelData.size() || _extraConfig.size() != extraConfig.size() || !FileExists(fileName)) {
        // different size so must be different
    }
    else {
        bool modelDataSame = true;

        auto oldit = _modelData.begin();
        auto newit = modelData.begin();

        while (modelDataSame && oldit != _modelData.end()) {
            if (ZCPP_GetPacketActualSize(**oldit) != ZCPP_GetPacketActualSize(**newit) ||
                memcmp(&(*oldit)->raw[ZCPP_CONFIGURATION_HEADER_MUTABLE_SIZE], &(*newit)->raw[ZCPP_CONFIGURATION_HEADER_MUTABLE_SIZE], ZCPP_GetPacketActualSize(**oldit) - ZCPP_CONFIGURATION_HEADER_MUTABLE_SIZE) != 0) {
                modelDataSame = false;
            }
            ++oldit;
            ++newit;
        }

        bool extraConfigSame = modelDataSame;
        auto it1 = _extraConfig.begin();
        auto it2 = extraConfig.begin();

        while (extraConfigSame && it1 != _extraConfig.end()) {
            if (ZCPP_GetPacketActualSize(**it1) != ZCPP_GetPacketActualSize(**it2) ||
                memcmp(&(*it1)->raw[ZCPP_EXTRADATA_HEADER_MUTABLE_SIZE], &(*it2)->raw[ZCPP_EXTRADATA_HEADER_MUTABLE_SIZE], ZCPP_GetPacketActualSize(**it1) - ZCPP_EXTRADATA_HEADER_MUTABLE_SIZE) != 0) {
                extraConfigSame = false;
            }
            ++it1;
            ++it2;
        }

        if (extraConfigSame) {
            // nothing has changed
            logger_base.debug("    No change.");
            return false;
        }
    }

    logger_base.debug("    Model data is different ... we need to save.");

    AllOff();
    EndFrame(0);

    // dont let any outputting happen while we play with things
    bool oldSuspend = _suspend;
    _suspend = true;

    // erase any existing data
    while (_modelData.size() > 0) {
        delete _modelData.front();
        _modelData.pop_front();
    }
    while (_extraConfig.size() > 0) {
        delete _extraConfig.front();
        _extraConfig.pop_front();
    }

    // and replace it with our new data
    for (const auto& it : modelData) {
        _modelData.push_back(it);
    }

    for (const auto& it : extraConfig) {
        _extraConfig.push_back(it);
    }

    wxFile zf;
    if (zf.Create(fileName, true)) {
        zf.Write(ZCPP_token, sizeof(ZCPP_token));

        for (const auto& it : _modelData) {
            wxASSERT(it->Configuration.ports <= ZCPP_CONFIG_MAX_PORT_PER_PACKET);
            uint8_t type = 0x00;
            zf.Write(&type, sizeof(type));

            uint8_t b = (sizeof(ZCPP_packet_t) & 0xFF00) >> 8;
            zf.Write(&b, sizeof(b));
            b = sizeof(ZCPP_packet_t) & 0xFF;
            zf.Write(&b, sizeof(b));

            zf.Write(it, sizeof(ZCPP_packet_t));
        }

        for (const auto& it : _extraConfig) {
            uint8_t type = 0x01;
            zf.Write(&type, sizeof(type));

            uint8_t b = (sizeof(ZCPP_packet_t) & 0xFF00) >> 8;
            zf.Write(&b, sizeof(b));
            b = sizeof(ZCPP_packet_t) & 0xFF;
            zf.Write(&b, sizeof(b));

            zf.Write(it, sizeof(ZCPP_packet_t));
        }

        uint8_t type = 0xFF;
        zf.Write(&type, sizeof(type));

        zf.Close();
    }

    _lastSecond = -1;

    if (c != nullptr) {
        ExtractUsedChannelsFromModelData(c);
    }

    _suspend = oldSuspend;
    return true;
}
#pragma endregion

#pragma region Start and Stop
bool ZCPPOutput::Open() {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_enabled) return true;
    if (!ip_utils::IsIPValid(_resolvedIp)) return false;

    _lastSecond = -1;

    _ok = IPOutput::Open();

    memset(&_packet, 0x00, sizeof(_packet));
    _sequenceNum = 0;

    memcpy(_packet.Data.Header.token, ZCPP_token, sizeof(ZCPP_token));
    _packet.Data.Header.type = ZCPP_TYPE_DATA;
    _packet.Data.Header.protocolVersion = ZCPP_CURRENT_PROTOCOL_VERSION;
    _packet.Data.priority = _priority;

    wxIPV4address localaddr;
    if (GetForceLocalIPToUse() == "") {
        localaddr.AnyAddress();
    }
    else {
        localaddr.Hostname(GetForceLocalIPToUse());
    }

    _datagram = new wxDatagramSocket(localaddr, wxSOCKET_BLOCK); // dont use NOWAIT as it can result in dropped packets
    if (_datagram == nullptr) {
        logger_base.error("ZCPPOutput: Error opening datagram.");
    }
    else if (!_datagram->IsOk()) {
        logger_base.error("ZCPPOutput: Error opening datagram. Network may not be connected? OK : FALSE");
        delete _datagram;
        _datagram = nullptr;
    }
    else if (_datagram->Error()) {
        logger_base.error("Error creating ZCPP datagram => %d : %s.", _datagram->LastError(), (const char *)DecodeIPError(_datagram->LastError()).c_str());
        delete _datagram;
        _datagram = nullptr;
    }

    if (_multicast) {
        _remoteAddr.Hostname(ZCPP_MULTICAST_DATA_ADDRESS + wxString(_ip).AfterLast('.'));
    }
    else {
        _remoteAddr.Hostname(_ip.c_str());
    }
    _remoteAddr.Service(ZCPP_PORT);

    return _ok && _datagram != nullptr;
}

void ZCPPOutput::Close() {

    if (_datagram != nullptr) {
        delete _datagram;
        _datagram = nullptr;
    }
    _ok = false;
}
#pragma endregion

#pragma region Frame Handling
void ZCPPOutput::StartFrame(long msec) {

    if (!_enabled) return;

    _timer_msec = msec;
}

void ZCPPOutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend || _tempDisable) return;

    if (_datagram == nullptr || _usedChannels == 0) return;

    if (!IsDontConfigure()) {
        long second = wxGetLocalTime();
        if (_lastSecond == -1 || (second != _lastSecond && (second - _lastSecond) % ZCPP_SEND_CONFIG_EVERY_N_SECONDS == 0)) {
            bool sendExtra = false;
            if (_lastSecond == -1 || second % ZCPP_SEND_CONFIG_EXTRADATA_EVERY_N_SECONDS == 0) {
                sendExtra = true;
            }

            for (auto it = _modelData.begin(); it != _modelData.end(); ++it) {
                auto it2 = it;
                ++it2;
                if (it == _modelData.begin()) {
                    (*it)->Configuration.flags |= ZCPP_CONFIG_FLAG_FIRST;
                }
                else                     {
                    (*it)->Configuration.flags &= ~ZCPP_CONFIG_FLAG_FIRST;
                }
                if (it2 == _modelData.end()) {
                    (*it)->Configuration.flags |= ZCPP_CONFIG_FLAG_LAST;
                }
                else {
                    (*it)->Configuration.flags &= ~ZCPP_CONFIG_FLAG_LAST;
                }

                if (sendExtra) {
                    (*it)->Configuration.flags |= ZCPP_CONFIG_FLAG_EXTRA_DATA_WILL_FOLLOW;
                }
                else                     {
                    (*it)->Configuration.flags &= ~ZCPP_CONFIG_FLAG_EXTRA_DATA_WILL_FOLLOW;
                }

                _datagram->SendTo(_remoteAddr, *it, ZCPP_GetPacketActualSize(**it));
            }

            if (sendExtra) {
                // Send descriptions every 10 mins
                for (auto it = _extraConfig.begin(); it != _extraConfig.end(); ++it) {
                    auto it2 = it;
                    ++it2;
                    if (it == _extraConfig.begin()) {
                        (*it)->ExtraData.flags |= ZCPP_CONFIG_FLAG_FIRST;
                    }
                    else                         {
                        (*it)->ExtraData.flags &= ~ZCPP_CONFIG_FLAG_FIRST;
                    }
                    if (it2 == _extraConfig.end()) {
                        (*it)->ExtraData.flags |= ZCPP_CONFIG_FLAG_LAST;
                    }
                    else                         {
                        (*it)->ExtraData.flags &= ~ZCPP_CONFIG_FLAG_LAST;
                    }
                    _datagram->SendTo(_remoteAddr, *it, ZCPP_GetPacketActualSize(**it));
                }
            }
            _lastSecond = second;
        }
    }

    if (_changed || NeedToOutput(suppressFrames)) {
        long i = 0;
        while (i < _usedChannels) {
            _packet.Data.sequenceNumber = _sequenceNum;
            uint32_t startAddress = i;
            _packet.Data.frameAddress = ntohl(startAddress);
            uint16_t packetlen = _usedChannels - i > sizeof(ZCPP_packet_t) - ZCPP_DATA_HEADER_SIZE ? sizeof(ZCPP_packet_t) - ZCPP_DATA_HEADER_SIZE : _usedChannels - i;
            _packet.Data.flags = (OutputManager::IsSyncEnabled_() ? ZCPP_DATA_FLAG_SYNC_WILL_BE_SENT : 0x00) +
                          (i + packetlen == _usedChannels ? ZCPP_DATA_FLAG_LAST : 0x00) +
                          (i == 0 ? ZCPP_DATA_FLAG_FIRST : 0x00);
            _packet.Data.packetDataLength = ntohs(packetlen);
            memcpy(_packet.Data.data, &_data[i], packetlen);
            _datagram->SendTo(_remoteAddr, &_packet, ZCPP_GetPacketActualSize(_packet));
            i += packetlen;
        }
        _sequenceNum++;

        FrameOutput();
    }
    else {
        SkipFrame();
    }
}

void ZCPPOutput::ResetFrame() {

    if (!_enabled) return;
}
#pragma endregion

#pragma region Data Setting
void ZCPPOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled) return;

    if (_data[channel] != data) {
        _data[channel] = data;
        _changed = true;
    }
}

void ZCPPOutput::SetManyChannels(int32_t channel, unsigned char* data, size_t size) {

    if (!_enabled) return;

    size_t chs = std::min(size, (size_t)(_channels - channel));

    if (memcmp(&_data[channel], data, chs) != 0)
    {
        memcpy(&_data[channel], data, chs);
        _changed = true;
    }
}

void ZCPPOutput::AllOff() {

    if (!_enabled) return;
    memset(_data, 0x00, _channels);
    _changed = true;
}
#pragma endregion

#pragma region UI
#ifndef EXCLUDENETWORKUI
void ZCPPOutput::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    IPOutput::UpdateProperties(propertyGrid, c, modelManager, expandProperties);
    auto p = propertyGrid->GetProperty("Channels");
    if (p) {
        p->SetValue(GetChannels());
        if (c->IsAutoSize()) {
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
}
void ZCPPOutput::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty *before, Controller *c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    IPOutput::AddProperties(propertyGrid, before, c, allSameSize, expandProperties);
    
    auto p = propertyGrid->Insert(before, new wxStringProperty("Multicast Address", "MulticastAddressDisplay", ZCPP_GetDataMulticastAddress(_ip)));
    p->ChangeFlag(wxPG_PROP_READONLY, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    p = propertyGrid->Insert(before, new wxBoolProperty("Supports Virtual Strings", "SupportsVirtualStrings", IsSupportsVirtualStrings()));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxBoolProperty("Supports Smart Remotes", "SupportsSmartRemotes", IsSupportsSmartRemotes()));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxBoolProperty("Send Data Multicast", "SendDataMulticast", IsMulticast()));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxBoolProperty("Suppress Sending Configuration", "DontSendConfig", IsDontConfigure()));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxUIntProperty("Channels", "Channels", GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", GetMaxChannels());
}

bool ZCPPOutput::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller *c) {
    if (IPOutput::HandlePropertyEvent(event, outputModelManager, c)) return true;

    wxString const name = event.GetPropertyName();

    if (name == "SupportsVirtualStrings") {
        SetSupportsVirtualStrings(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ZCPPOutput::HandlePropertyEvent::SupportsVirtualStrings");
        return true;
    } else if (name == "SupportsSmartRemotes") {
        SetSupportsSmartRemotes(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ZCPPOutput::HandlePropertyEvent::SupportsSmartRemotes");
        return true;
    } else if (name == "SendDataMulticast") {
        SetMulticast(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ZCPPOutput::HandlePropertyEvent::SendDataMulticast");
        return true;
    } else if (name == "DontSendConfig") {
        SetDontConfigure(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ZCPPOutput::HandlePropertyEvent::DontSendConfig");
        return true;
    } else if (name == "Channels") {
        SetChannels(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ZCPPOutput::HandlePropertyEvent::Channels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ZCPPOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ZCPPOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ZCPPOutput::HandlePropertyEvent::Channels", nullptr);
        return true;
    }
    return false;
}
void ZCPPOutput::RemoveProperties(wxPropertyGrid* propertyGrid) {
    IPOutput::RemoveProperties(propertyGrid);
    propertyGrid->DeleteProperty("DontSendConfig");
    propertyGrid->DeleteProperty("SendDataMulticast");
    propertyGrid->DeleteProperty("SupportsSmartRemotes");
    propertyGrid->DeleteProperty("SupportsVirtualStrings");
    propertyGrid->DeleteProperty("Channels");
}

#endif
#pragma endregion

