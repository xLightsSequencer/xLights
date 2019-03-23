#include "ZCPPOutput.h"

#include <wx/xml/xml.h>
#include <wx/file.h>
#include <wx/filename.h>

#include "ZCPPDialog.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"

#include <list>

#include <log4cpp/Category.hh>

#pragma region Constructors and Destructors
ZCPPOutput::ZCPPOutput(wxXmlNode* node, std::string showdir) : IPOutput(node)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    memset(&_modelData, 0x00, sizeof(_modelData));
    _multicast = false;
    _dontConfigure = false;
    _lastSecond = -1;
    _sequenceNum = 0;
    _usedChannels = _channels;
    _datagram = nullptr;
    _data = (wxByte*)malloc(_channels);
    memset(_data, 0, _channels);
    memset(&_packet, 0, sizeof(_packet));
    _vendor = wxAtoi(node->GetAttribute("Vendor", "65535"));
    _model = wxAtoi(node->GetAttribute("Model", "65535"));
    _supportsVirtualStrings = node->GetAttribute("SupportsVirtualStrings", "FALSE") == "TRUE";
    _supportsSmartRemotes = node->GetAttribute("SupportsSmartRemotes", "FALSE") == "TRUE";
    _multicast = node->GetAttribute("Multicast", "FALSE") == "TRUE";
    _dontConfigure = node->GetAttribute("DontConfigure", "FALSE") == "TRUE";
    DeserialiseProtocols(node->GetAttribute("Protocols", ""));

    if (!_dontConfigure)
    {
        wxString fileName = GetIP();
        fileName.Replace(".", "_");
        fileName += ".zcpp";
        fileName = showdir + wxFileName::GetPathSeparator() + fileName;

        if (wxFile::Exists(fileName))
        {
            wxFile zf;
            if (zf.Open(fileName))
            {
                uint8_t tag[4];
                zf.Read(tag, sizeof(tag));
                if (tag[0] != 'Z' || tag[1] != 'C' || tag[2] != 'P' || tag[3] != 'P')
                {
                    logger_base.warn("ZCPP Model data file %s did not contain opening tag.", (const char*)fileName.c_str());
                }
                else
                {
                    while (!zf.Eof())
                    {
                        uint8_t type;
                        zf.Read(&type, sizeof(type));
                        switch (type)
                        {
                        case 0x00:
                        {
                            uint8_t b1;
                            uint8_t b2;
                            zf.Read(&b1, sizeof(b1));
                            zf.Read(&b2, sizeof(b2));
                            uint16_t size = (b1 << 8) + b2;
                            if (size == sizeof(_modelData))
                            {
                                zf.Read(&_modelData, sizeof(_modelData));
                            }
                            else
                            {
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
                            if (size == sizeof(ZCPP_packet_t))
                            {
                                ZCPP_packet_t descPacket;
                                zf.Read(&descPacket, sizeof(descPacket));
                                _extraConfig.push_back(descPacket);
                            }
                            else
                            {
                                logger_base.warn("ZCPP Model data file %s unrecognised extra config size.", (const char*)fileName.c_str());
                                zf.SeekEnd();
                            }
                        }
                        break;
                        case 0xFF:
                            zf.SeekEnd();
                            break;
                        default:
                            logger_base.warn("ZCPP Model data file %s unrecognised type %d.", (const char*)fileName.c_str(), type);
                            break;
                        }
                    }
                    logger_base.debug("ZCPP Model data file %s loaded.", (const char*)fileName.c_str());
                }
                zf.Close();
            }
            else
            {
                logger_base.warn("ZCPP Model data file %s could not be opened.", (const char*)fileName.c_str());
            }
            ExtractUsedChannelsFromModelData();
        }
        else
        {
            logger_base.warn("ZCPP Model data file %s not found.", (const char*)fileName.c_str());
        }
    }
    else
    {
        logger_base.warn("ZCPP set to not configure the controller.");
    }
}

ZCPPOutput::ZCPPOutput() : IPOutput()
{
    memset(&_modelData, 0x00, sizeof(_modelData));
    _multicast = false;
    _dontConfigure = false;
    _lastSecond = -1;
    _channels = 1;
    _usedChannels = 1;
    _universe = -1;
    _sequenceNum = 0;
    _supportsVirtualStrings = false;
    _supportsSmartRemotes = false;
    _datagram = nullptr;
    _vendor = -1;
    _autoSize = true;
    _model = -1;
    _data = (wxByte*)malloc(_channels);
    memset(_data, 0, _channels);
    memset(&_packet, 0, sizeof(_packet));
}

ZCPPOutput::~ZCPPOutput()
{
    if (_datagram != nullptr) delete _datagram;
    if (_data != nullptr) free(_data);
}
#pragma endregion Constructors and Destructors

void ZCPPOutput::ExtractUsedChannelsFromModelData()
{
    int ports = _modelData.Configuration.ports;
    _usedChannels = 1;
    ZCPP_PortConfig* port = _modelData.Configuration.PortConfig;
    for (int i = 0; i < ports; i++)
    {
        int start = htonl(port->startChannel);
        int len = htons(port->channels);
        if (start + len - 1 > _usedChannels)
        {
            _usedChannels = start + len;
        }
        port++;
    }
}

bool ZCPPOutput::SetModelData(ZCPP_packet_t& modelData, std::list<ZCPP_packet_t> extraConfig, std::string showDir)
{
    if (_dontConfigure) return false;

    // before byte 9 there can be differences
    if (memcmp(&_modelData.raw[8], &modelData.raw[8], sizeof(_modelData) - 8) == 0 &&
        _extraConfig.size() == _extraConfig.size())
    {
        bool extraConfigSame = true;
        auto it1 = _extraConfig.begin();
        auto it2 = extraConfig.begin();

        while (extraConfigSame && it1 != _extraConfig.end())
        {
            if (memcmp(&(*it1).raw[9], &(*it2).raw[9], sizeof(extraConfig) - 9) != 0)
            {
                extraConfigSame = false;
            }
            ++it1;
            ++it2;
        }

        if (extraConfigSame)
        {
            // nothing has changed
            return false;
        }
    }

    while (_extraConfig.size() > 0)
    {
        _extraConfig.pop_front();
    }

    for (auto it : extraConfig)
    {
        _extraConfig.push_back(it);
    }

    wxString fileName = GetIP();

    if (_multicast)
    {
        fileName = ZCPP_MULTICAST_DATA_ADDRESS + wxString(GetIP()).AfterLast('.');
    }

    fileName.Replace(".", "_");
    fileName += ".zcpp";
    fileName = showDir + wxFileName::GetPathSeparator() + fileName;

    wxFile zf;
    if (zf.Create(fileName, true))
    {
        zf.Write(ZCPP_token, sizeof(ZCPP_token));
        
        uint8_t type = 0x00;
        zf.Write(&type, sizeof(type));
        
        uint8_t b = (sizeof(modelData) & 0xFF00) >> 8;
        zf.Write(&b, sizeof(b));
        b = sizeof(modelData) & 0xFF;
        zf.Write(&b, sizeof(b));
        
        zf.Write(&modelData, sizeof(modelData));
        
        for (auto it : _extraConfig)
        {
            type = 0x01;
            zf.Write(&type, sizeof(type));

            b = (sizeof(ZCPP_packet_t) & 0xFF00) >> 8;
            zf.Write(&b, sizeof(b));
            b = sizeof(ZCPP_packet_t) & 0xFF;
            zf.Write(&b, sizeof(b));

            zf.Write(&it, sizeof(ZCPP_packet_t));
        }

        type = 0xFF;
        zf.Write(&type, sizeof(type));

        zf.Close();
    }

    _modelData = modelData;
    _lastSecond = -1;

    ExtractUsedChannelsFromModelData();

    return true;
}


void ZCPPOutput::DeserialiseProtocols(const std::string& protocols)
{
    auto ps = wxSplit(protocols, '|');

    for (auto it : ps)
    {
        AddProtocol(it);
    }
}

std::string ZCPPOutput::SerialiseProtocols()
{
    std::string res;
    for (auto it : _protocols)
    {
        if (res != "") res += "|";
        res += it;
    }
    return res;
}

wxXmlNode* ZCPPOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    node->AddAttribute("Vendor", wxString::Format("%d", _vendor));
    node->AddAttribute("Model", wxString::Format("%d", _model));
    if (_supportsVirtualStrings)node->AddAttribute("SupportsVirtualStrings", "TRUE");
    if (_supportsSmartRemotes)node->AddAttribute("SupportsSmartRemotes", "TRUE");
    if (_dontConfigure)node->AddAttribute("DontConfigure", "TRUE");
    if (_multicast)node->AddAttribute("Multicast", "TRUE");
    node->AddAttribute("Protocols", SerialiseProtocols());
    IPOutput::Save(node);

    return node;
}

#pragma region Static Functions
void ZCPPOutput::SendSync()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static ZCPP_packet_t syncdata;
    static uint8_t syncSequenceNum = 0;
    static bool initialised = false;
    static wxIPV4address syncremoteAddr;
    static wxDatagramSocket *syncdatagram = nullptr;

    if (!initialised)
    {
        logger_base.debug("Initialising ZCPP Sync.");

        initialised = true;

        memset(&syncdata, 0x00, sizeof(syncdata));
        memcpy(syncdata.Sync.Header.token, ZCPP_token, sizeof(ZCPP_token));
        syncdata.Sync.Header.type = ZCPP_TYPE_SYNC;
        syncdata.Sync.Header.protocolVersion = ZCPP_CURRENT_PROTOCOL_VERSION;

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
            logger_base.error("Error initialising ZCPP sync datagram.");
        }
        else if (!syncdatagram->IsOk())
        {
            logger_base.error("Error initialising ZCPP sync datagram ... is network connected? OK : FALSE");
            delete syncdatagram;
            syncdatagram = nullptr;
        }
        else if (syncdatagram->Error() != wxSOCKET_NOERROR)
        {
            logger_base.error("Error creating ZCPP sync datagram => %d : %s.", syncdatagram->LastError(), (const char *)DecodeIPError(syncdatagram->LastError()).c_str());
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
    if (syncdatagram != nullptr)
    {
        syncdatagram->SendTo(syncremoteAddr, &syncdata, sizeof(ZCPP_Sync));
    }
}

void ZCPPOutput::InitialiseExtraConfigPacket(ZCPP_packet_t& packet, int seq)
{
    memset(&packet, 0x00, sizeof(packet));
    memcpy(packet.ExtraData.Header.token, ZCPP_token, sizeof(ZCPP_token));
    packet.ExtraData.Header.type = ZCPP_TYPE_EXTRA_DATA;
    packet.ExtraData.Header.protocolVersion = ZCPP_CURRENT_PROTOCOL_VERSION;
    packet.ExtraData.sequenceNumber = ntohs(seq);
}

std::list<Output*> ZCPPOutput::Discover(OutputManager* outputManager)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::list<Output*> res;

    ZCPP_packet_t packet;

    memset(&packet, 0x00, sizeof(packet));
    memcpy(packet.Discovery.Header.token, ZCPP_token, sizeof(ZCPP_token));
    packet.Discovery.Header.type = ZCPP_TYPE_DISCOVERY;
    packet.Discovery.Header.protocolVersion = ZCPP_CURRENT_PROTOCOL_VERSION;

    wxIPV4address sendlocaladdr;
    if (IPOutput::__localIP == "")
    {
        sendlocaladdr.AnyAddress();
    }
    else
    {
        sendlocaladdr.Hostname(IPOutput::__localIP);
    }

    wxDatagramSocket* datagram = new wxDatagramSocket(sendlocaladdr, wxSOCKET_NOWAIT | wxSOCKET_BROADCAST);

    if (datagram == nullptr)
    {
        logger_base.error("Error initialising ZCPP discovery datagram.");
    }
    else if (!datagram->IsOk())
    {
        logger_base.error("Error initialising ZCPP discovery datagram ... is network connected? OK : FALSE");
        delete datagram;
        datagram = nullptr;
    }
    else if (datagram->Error() != wxSOCKET_NOERROR)
    {
        logger_base.error("Error creating ZCPP discovery datagram => %d : %s.", datagram->LastError(), (const char *)DecodeIPError(datagram->LastError()).c_str());
        delete datagram;
        datagram = nullptr;
    }
    else
    {
        logger_base.info("ZCPP discovery datagram opened successfully.");
    }

    // multicast - universe number must be in lower 2 bytes
    wxIPV4address remoteaddr;
    wxString ipaddrWithUniv = ZCPP_MULTICAST_ADDRESS;
    remoteaddr.Hostname(ipaddrWithUniv);
    remoteaddr.Service(ZCPP_PORT);

    // bail if we dont have a datagram to use
    if (datagram != nullptr)
    {
        logger_base.info("ZCPP sending discovery packet.");
        datagram->SendTo(remoteaddr, &packet, sizeof(ZCPP_Discovery));
        if (datagram->Error() != wxSOCKET_NOERROR)
        {
            logger_base.error("Error sending ZCPP discovery datagram => %d : %s.", datagram->LastError(), (const char *)DecodeIPError(datagram->LastError()).c_str());
        }
        else
        {
            logger_base.info("ZCPP sent discovery packet. Sleeping for 2 seconds.");

            // give the controllers 2 seconds to respond
            wxMilliSleep(2000);

            ZCPP_packet_t response;

            int lastread = 1;

            while (lastread > 0)
            {
                wxStopWatch sw;
                logger_base.debug("Trying to read ZCPP discovery packet.");
                memset(&response, 0x00, sizeof(response));
                datagram->Read(&response, sizeof(response));
                lastread = datagram->LastReadCount();

                if (lastread > 0)
                {
                    logger_base.debug(" Read done. %d bytes %ldms", lastread, sw.Time());

                    if (memcmp(&response, ZCPP_token, sizeof(ZCPP_token)) == 0 && response.DiscoveryResponse.Header.type == ZCPP_TYPE_DISCOVERY_RESPONSE)
                    {
                        logger_base.debug(" Valid response.");

                        ZCPPOutput* output = new ZCPPOutput();

                        int vendor = ZCPP_FromWire16(response.DiscoveryResponse.vendor);
                        output->SetVendor(vendor);
                        logger_base.debug("   Vendor %d", vendor);

                        int model = ZCPP_FromWire16(response.DiscoveryResponse.model);
                        output->SetModel(model);
                        logger_base.debug("   Model %d", model);

                        logger_base.debug("   Firmware %s", response.DiscoveryResponse.firmwareVersion);

                        auto ip = wxString::Format("%d.%d.%d.%d",
                            (int)(response.DiscoveryResponse.ipv4Address & 0xFF),
                            (int)(response.DiscoveryResponse.ipv4Address & 0xFF00) >> 8,
                            (int)(response.DiscoveryResponse.ipv4Address & 0xFF0000) >> 16,
                            (int)(response.DiscoveryResponse.ipv4Address & 0xFF000000) >> 24);
                        output->SetIP(ip.ToStdString());
                        logger_base.debug("   IP %s", (const char *)ip.c_str());

                        output->SetDescription(response.DiscoveryResponse.userControllerName);
                        logger_base.debug("   Description %s", (const char *)output->GetDescription().c_str());

                        uint32_t channels = ZCPP_FromWire32(response.DiscoveryResponse.maxTotalChannels);
                        logger_base.debug("   Channels %ld", channels);

                        bool supportsVirtualStrings = response.DiscoveryResponse.flags & ZCPP_DISCOVERY_FLAG_SUPPORTS_VIRTUAL_STRINGS;
                        logger_base.debug("   Supports Virtual Strings %d", supportsVirtualStrings);

                        bool supportsSmartRemotes = response.DiscoveryResponse.flags & ZCPP_DISCOVERY_FLAG_SUPPORTS_SMART_REMOTES;
                        logger_base.debug("   Supports Smart Remotes %d", supportsSmartRemotes);

                        bool dontConfigure = (response.DiscoveryResponse.flags & ZCPP_DISCOVERY_FLAG_CONFIGURATION_LOCKED) != 0;
                        logger_base.debug("   Doesnt want to recieve configuration %d", dontConfigure);

                        bool multicast = (response.DiscoveryResponse.flags & ZCPP_DISCOVERY_FLAG_SEND_DATA_AS_MULTICAST) != 0;
                        logger_base.debug("   Wants to receive data multicast %d", multicast);

                        // now search for it in outputManager
                        auto outputs = outputManager->GetOutputs();
                        for (auto it = outputs.begin(); it != outputs.end(); ++it)
                        {
                            if ((*it)->GetIP() == output->GetIP())
                            {
                                // we already know about this controller
                                logger_base.info("ZCPP Discovery we already know about this controller %s.", (const char*)output->GetIP().c_str());
                                delete output;
                                output = nullptr;
                                break;
                            }
                        }

                        if (output != nullptr)
                        {
                            logger_base.info("ZCPP Discovery found a new controller %s.", (const char*)output->GetIP().c_str());
                            output->SetSupportsVirtualStrings(supportsVirtualStrings);
                            output->SetSupportsSmartRemotes(supportsSmartRemotes);

                            uint32_t mask = 0x00000001;
                            uint32_t dp = ZCPP_FromWire32(response.DiscoveryResponse.protocolsSupported);
                            for (int i = 0; i < 32; i++)
                            {
                                if ((dp & mask) != 0)
                                {
                                    output->AddProtocol(DecodeProtocol(ZCPP_ConvertDiscoveryProtocolToProtocol(dp & mask)));
                                    logger_base.debug("   Supports Protocol %s", (const char *)DecodeProtocol(ZCPP_ConvertDiscoveryProtocolToProtocol(dp & mask)).c_str());
                                }
                                mask = mask << 1;
                            }

                            output->SetAutoSize(true);
                            output->SetChannels(1 /*channels*/); // Set this to one as it defaults to auto size
                            output->SetDontConfigure(dontConfigure);

                            output->SetMulticast(multicast);

                            if (output != nullptr)
                            {
                                logger_base.info("ZCPP Discovery adding controller %s.", (const char*)output->GetIP().c_str());
                                res.push_back(output);
                            }
                        }
                    }
                    else
                    {
                        // non discovery response packet
                        logger_base.info("ZCPP Discovery strange packet received.");
                    }
                }
            }
            logger_base.info("ZCPP Discovery Done looking for response.");
        }
        datagram->Close();
        delete datagram;
    }

    logger_base.info("ZCPP Discovery Finished.");

    return res;
}

int ZCPPOutput::EncodeColourOrder(const std::string& colourOrder)
{
    if (colourOrder == "RGB") return 0;
    if (colourOrder == "RBG") return 1;
    if (colourOrder == "GRB") return 2;
    if (colourOrder == "GBR") return 3;
    if (colourOrder == "BRG") return 4;
    if (colourOrder == "BGR") return 5;
    return 0;
}

int ZCPPOutput::EncodeProtocol(const std::string& protocol)
{
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

    return 0xFE;
}

std::string ZCPPOutput::DecodeProtocol(int protocol)
{
    switch (protocol)
    {
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
    default:
        return "unknown";
    }
}
#pragma endregion Static Functions

#pragma region Start and Stop
bool ZCPPOutput::Open()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_enabled) return true;

    _lastSecond = -1;

    _ok = IPOutput::Open();

    memset(&_packet, 0x00, sizeof(_packet));
    _sequenceNum = 0;

    memcpy(_packet.Data.Header.token, ZCPP_token, sizeof(ZCPP_token));
    _packet.Data.Header.type = ZCPP_TYPE_DATA;
    _packet.Data.Header.protocolVersion = ZCPP_CURRENT_PROTOCOL_VERSION;

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
        logger_base.error("ZCPPOutput: Error opening datagram.");
    }
    else if (!_datagram->IsOk())
    {
        logger_base.error("ZCPPOutput: Error opening datagram. Network may not be connected? OK : FALSE");
        delete _datagram;
        _datagram = nullptr;
    }
    else if (_datagram->Error() != wxSOCKET_NOERROR)
    {
        logger_base.error("Error creating ZCPP datagram => %d : %s.", _datagram->LastError(), (const char *)DecodeIPError(_datagram->LastError()).c_str());
        delete _datagram;
        _datagram = nullptr;
    }

    if (_multicast)
    {
        _remoteAddr.Hostname(ZCPP_MULTICAST_DATA_ADDRESS + wxString(_ip).AfterLast('.'));
    }
    else
    {
        _remoteAddr.Hostname(_ip.c_str());
    }
    _remoteAddr.Service(ZCPP_PORT);

    return _ok && _datagram != nullptr;
}

void ZCPPOutput::Close()
{
    if (_datagram != nullptr)
    {
        delete _datagram;
        _datagram = nullptr;
    }
}
#pragma endregion Start and Stop

void ZCPPOutput::SetTransientData(int on, long startChannel, int nullnumber)
{
    _outputNumber = on;
    _startChannel = startChannel;
    if (nullnumber > 0) _nullNumber = nullnumber;
}

#pragma region Frame Handling
void ZCPPOutput::StartFrame(long msec)
{
    if (!_enabled) return;

    _timer_msec = msec;
}

void ZCPPOutput::EndFrame(int suppressFrames)
{
    if (!_enabled || _suspend) return;

    if (_datagram == nullptr || _usedChannels == 0) return;

    if (!IsDontConfigure())
    {
        long second = wxGetLocalTime();
        if (_lastSecond == -1 || (second != _lastSecond && (second - _lastSecond) % 10 == 0))
        {
            if (_lastSecond == -1 || second % 600 == 0)
            {
                // Send descriptions every 10 mins
                for (auto it = _extraConfig.begin(); it != _extraConfig.end(); ++it)
                {
                    auto it2 = it;
                    ++it2;
                    if (it == _extraConfig.begin())
                    {
                        (*it).ExtraData.flags |= ZCPP_CONFIG_FLAG_FIRST;
                    }
                    if (it2 == _extraConfig.end())
                    {
                        (*it).ExtraData.flags |= ZCPP_CONFIG_FLAG_LAST;
                    }
                    _datagram->SendTo(_remoteAddr, &(*it), sizeof(*it));
                }
            }

            if (_modelData.raw[0] != 0x00)
            {
                _lastSecond = second;
                _datagram->SendTo(_remoteAddr, &_modelData, sizeof(_modelData));
            }
        }
    }

    if (_changed || NeedToOutput(suppressFrames))
    {
        int i = 0;
        while (i < _usedChannels)
        {
            _packet.Data.sequenceNumber = _sequenceNum;
            uint16_t startAddress = i;
            _packet.Data.frameAddress = ntohs(startAddress);
            uint16_t packetlen = _usedChannels - i > sizeof(ZCPP_packet_t) - 14 ? sizeof(ZCPP_packet_t) - 14 : _usedChannels - i;
            _packet.Data.flags = (OutputManager::IsSyncEnabled_() ? ZCPP_DATA_FLAG_SYNC_WILL_BE_SENT : 0x00) +
                          (i + packetlen == _usedChannels ? ZCPP_DATA_FLAG_LAST : 0x00) +
                          (i == 0 ? ZCPP_DATA_FLAG_FIRST : 0x00);
            _packet.Data.packetDataLength = ntohs(packetlen);
            memcpy(_packet.Data.data, &_data[i], packetlen);
            _datagram->SendTo(_remoteAddr, &_packet, 14 + packetlen);
            i += packetlen;
        }
        _sequenceNum++;

        FrameOutput();
    }
    else
    {
        SkipFrame();
    }
}

void ZCPPOutput::ResetFrame()
{
    if (!_enabled) return;
}
#pragma endregion Frame Handling

#pragma region Data Setting
void ZCPPOutput::SetOneChannel(long channel, unsigned char data)
{
    if (!_enabled) return;

    if (_data[channel] != data) {
        _data[channel] = data;
        _changed = true;
    }
}

void ZCPPOutput::SetManyChannels(long channel, unsigned char data[], long size)
{
        long chs = std::min(size, _channels - channel);

        if (memcmp(&_data[channel], data, chs) == 0)
        {
            // nothing changed
        }
        else
        {
            memcpy(&_data[channel], data, chs);
            _changed = true;
        }
}

void ZCPPOutput::AllOff()
{
    memset(_data, 0x00, _channels);
    _changed = true;
}
#pragma endregion Data Setting

#pragma region Getters and Setters
long ZCPPOutput::GetEndChannel() const
{
    return _startChannel + _channels - 1;
}

std::string ZCPPOutput::GetLongDescription() const
{
    std::string res = "";

        if (!_enabled) res += "INACTIVE ";
        res += "ZCPP " + _ip;
        res += " [1-" + std::string(wxString::Format(wxT("%li"), (long)_channels)) + "] ";
        res += "(" + std::string(wxString::Format(wxT("%li"), (long)GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%li"), (long)GetActualEndChannel())) + ") ";
        res += _description;

    return res;
}

std::string ZCPPOutput::GetChannelMapping(long ch) const
{
    std::string res = "Channel " + std::string(wxString::Format(wxT("%li"), ch)) + " maps to ...\n";

    res += "Type: ZCPP\n";
    long channeloffset = ch - GetStartChannel() + 1;
    res += "IP: " + _ip + "\n";
    res += "Channel: " + std::string(wxString::Format(wxT("%li"), channeloffset)) + "\n";

    if (!_enabled) res += " INACTIVE";
    return res;
}
#pragma endregion Getters and Setters

#pragma region UI
#ifndef EXCLUDENETWORKUI
Output* ZCPPOutput::Configure(wxWindow* parent, OutputManager* outputManager, ModelManager* modelManager)
{
    ZCPPDialog dlg(parent, this, outputManager, modelManager);

    int res = dlg.ShowModal();

    if (res == wxID_CANCEL)
    {
        return nullptr;
    }

    return this;
}
#endif
#pragma endregion UI