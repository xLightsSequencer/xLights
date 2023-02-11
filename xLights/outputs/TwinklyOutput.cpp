
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "TwinklyOutput.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"
#include "../xSchedule/wxJSON/jsonreader.h"
#include "../utils/Curl.h"
#include <log4cpp/Category.hh>
#include <wx/base64.h>
#include <wx/protocol/http.h>
#include <wx/sstream.h>

#ifndef EXCLUDEDISCOVERY
#include "../Discovery.h"
#endif

#pragma region Constructors and Destructors
TwinklyOutput::TwinklyOutput(wxXmlNode* node) :
    IPOutput(node)
{
}

TwinklyOutput::TwinklyOutput()
{
}

TwinklyOutput::~TwinklyOutput()
{
    if (_datagram != nullptr) {
        delete _datagram;
        _datagram = nullptr;
    }
}
wxXmlNode* TwinklyOutput::Save()
{
    return IPOutput::Save();
}
#pragma endregion

#pragma region Output implementation
inline std::string TwinklyOutput::GetType() const
{
    return OUTPUT_TWINKLY;
}

inline int TwinklyOutput::GetMaxChannels() const
{
    return MAX_CHANNELS;
}

inline bool TwinklyOutput::IsValidChannelCount(int32_t channelCount) const
{
    return channelCount > 0 && channelCount <= MAX_CHANNELS;
}

std::string TwinklyOutput::GetLongDescription() const
{
    std::string res = "Twinkly ";
    res += "[" + std::string(wxString::Format(wxT("%i"), _channels)) + "] ";
    return res;
}

void TwinklyOutput::SetTransientData(int32_t& startChannel, int nullnumber)
{
    if (_fppProxyOutput) {
        _fppProxyOutput->SetTransientData(startChannel, nullnumber);
    }

    wxASSERT(startChannel != -1);
    _startChannel = startChannel;
    startChannel += GetChannels();
}
#pragma endregion

#pragma region Start and Stop
bool TwinklyOutput::Open()
{
    if (!IPOutput::Open()) {
        return false;
    }

    if (_fppProxyOutput) {
        return _ok;
    }

    // ensure the token is fresh
    if (!ReloadToken()) {
        return false;
    }

    OpenDatagram();

    // set real time mode
    wxJSONValue result;
    if (!MakeCall("POST", "/xled/v1/led/mode", result, "{\"mode\": \"rt\"}")) {
        return false;
    }

    return _datagram != nullptr;
}

void TwinklyOutput::Close()
{
    if (_datagram != nullptr) {
        delete _datagram;
        _datagram = nullptr;
    }

    if (!_fppProxyOutput) {
        // turn off
        wxJSONValue result;
        MakeCall("POST", "/xled/v1/led/mode", result, "{\"mode\": \"off\"}");
    }

    IPOutput::Close();
}
#pragma endregion

#pragma region Frame Handling
void TwinklyOutput::StartFrame(long msec)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_enabled) {
        return;
    }

    if (_fppProxyOutput) {
        return _fppProxyOutput->StartFrame(msec);
    }

    if (_datagram == nullptr && OutputManager::IsRetryOpen()) {
        OpenDatagram();
        if (_ok) {
            logger_base.debug("TwinklyOutput: Open retry successful");
        }
    }

    _timer_msec = msec;
}

void TwinklyOutput::EndFrame(int suppressFrames)
{
    if (!_enabled || _suspend || _tempDisable || _datagram == nullptr) {
        return;
    }

    if (_fppProxyOutput) {
        _fppProxyOutput->EndFrame(suppressFrames);
        return;
    }

    if (_channels > m_channelData.size()) {
        m_channelData.resize(_channels);
    }

    const int PACKET_SIZE = 1 + TOKEN_SIZE + 2 + 1 + 900;
    unsigned char packet[PACKET_SIZE];

    wxIPV4address remoteAddr;
    remoteAddr.Hostname(_ip.c_str());
    remoteAddr.Service(UDP_PORT);

    int offset = 0;
    int fragmentNumber = 0;
    while (offset < _channels) {
        // prepare the packet
        int i = 0;

        // version 3
        packet[i++] = 3;

        // 8 bytes token
        memcpy(&packet[i], m_decodedToken.data(), TOKEN_SIZE);
        i += TOKEN_SIZE;

        // 2 zeros
        packet[i++] = 0;
        packet[i++] = 0;

        // 1 byte fragment number
        packet[i++] = fragmentNumber++;

        // send 900 channels
        int payloadSize = std::min(900, (int)_channels);
        memcpy(&packet[i], &m_channelData[offset], payloadSize);
        offset += payloadSize;
        i += payloadSize;

        wxASSERT(i <= PACKET_SIZE);
        _datagram->SendTo(remoteAddr, packet, PACKET_SIZE);
    }

    FrameOutput();
}

void TwinklyOutput::ResetFrame()
{
    if (!_enabled)
        return;
    if (_fppProxyOutput) {
        _fppProxyOutput->ResetFrame();
        return;
    }
}
#pragma endregion

#pragma region Frame Handling
void TwinklyOutput::SetOneChannel(int32_t channel, unsigned char data)
{
    if (!_enabled)
        return;

    if (_fppProxyOutput) {
        _fppProxyOutput->SetOneChannel(channel, data);
    } else {
        if (_channels > m_channelData.size()) {
            m_channelData.resize(_channels);
        }
        m_channelData[channel] = data;
    }
}
void TwinklyOutput::SetManyChannels(int32_t channel, unsigned char* data, size_t size)
{
    if (!_enabled)
        return;

    if (_fppProxyOutput) {
        _fppProxyOutput->SetManyChannels(channel, data, size);
    } else {
        if (_channels > m_channelData.size()) {
            m_channelData.resize(_channels);
        }
        std::copy(data, data + size, m_channelData.data());
    }
}
void TwinklyOutput::AllOff()
{
    if (!_enabled)
        return;

    if (_fppProxyOutput) {
        _fppProxyOutput->AllOff();
    } else {
        memset(m_channelData.data(), 0x00, m_channelData.size());
    }
}
#pragma endregion

bool TwinklyOutput::MakeCall(const std::string& method, const std::string& path, wxJSONValue& result, const char* body)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Twinkly: Invoke " + method + " http://" + _ip + path);
    if (body != nullptr)
        logger_base.debug("         '%s'", body);

    wxString bod;
    if (body != nullptr) {
        bod = wxString(body);
    }

    std::vector<std::pair<std::string, std::string>> customHeaders;
    if (!m_token.empty()) {
        // assign authentication token if present
        customHeaders.push_back(std::pair("X-Auth-Token", m_token));
    }

    int responseCode;
    std::string httpResponse = Curl::HTTPSPost("http://" + _ip + path, bod, "", "", "JSON", HTTP_TIMEOUT, customHeaders, &responseCode);

    if (responseCode != 200) {
        logger_base.error("Twinkly: Error %d : %s", responseCode, (const char*)httpResponse.c_str());
    }

    wxJSONReader reader;
    wxString str(httpResponse);

    if (reader.Parse(str, &result)) {
        logger_base.debug("DX");
        wxString err;
        auto errors = reader.GetErrors();
        for (int i = 0; i < errors.GetCount(); ++i) {
            err.Append(errors.Item(i)).Append(", ");
        }
        logger_base.error("Twinkly: Returned json is not valid: " + err + " : '" + str + "'");
        return false;
    }

    int32_t code;
    if (!result.Get("code", wxJSONValue(0)).AsInt32(code) || code != 1000) {
        logger_base.error("Twinkly: Server returned: " + std::to_string(code));
        return false;
    }

    return true;
}

bool TwinklyOutput::ReloadToken()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    m_token = "";

    // get token
    wxJSONValue reply;
    if (!MakeCall("POST", "/xled/v1/login", reply, "{\"challenge\": \"AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8=\"}")) {
        return false;
    }
    wxString token;
    if (!reply.Get("authentication_token", "").AsString(token)) {
        logger_base.error("Invalid authentication token");
        return false;
    }
    auto decoded = wxBase64Decode(token);
    if (decoded.GetDataLen() != TOKEN_SIZE) {
        logger_base.error("Invalid authentication token");
        return false;
    }

    // all token needs to be verified
    m_token = token;
    if (!MakeCall("POST", "/xled/v1/verify", reply)) {
        m_token = "";
        return false;
    }

    std::copy((char*)decoded.GetData(), (char*)decoded.GetData() + 8, m_decodedToken.data());
    return true;
}

#pragma region UDP
void TwinklyOutput::OpenDatagram()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_datagram != nullptr)
        return;

    wxIPV4address localaddr;
    if (GetForceLocalIPToUse() == "") {
        localaddr.AnyAddress();
    } else {
        localaddr.Hostname(GetForceLocalIPToUse());
    }

    _datagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);
    if (_datagram == nullptr) {
        logger_base.error("Twinkly: %s Error opening datagram.", (const char*)localaddr.IPAddress().c_str());
    } else if (!_datagram->IsOk()) {
        logger_base.error("Twinkly: %s Error opening datagram. Network may not be connected? OK : FALSE", (const char*)localaddr.IPAddress().c_str());
        delete _datagram;
        _datagram = nullptr;
    } else if (_datagram->Error() != wxSOCKET_NOERROR) {
        logger_base.error("Twinkly: %s Error creating Twinkly datagram => %d : %s.", (const char*)localaddr.IPAddress().c_str(), _datagram->LastError(), (const char*)DecodeIPError(_datagram->LastError()).c_str());
        delete _datagram;
        _datagram = nullptr;
    }
}
#pragma endregion

bool TwinklyOutput::GetLayout(const std::string& ip, std::vector<std::tuple<float, float, float>>& result)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::vector<std::pair<std::string, std::string>> customHeaders = {};
    int responseCode;
    std::string httpResponse = Curl::HTTPSGet("http://" + ip + "/xled/v1/led/layout/full", "", "", HTTP_TIMEOUT, customHeaders, &responseCode);

    if (responseCode != 200) {
        logger_base.error("Twinkly: Error %d : %s", responseCode, (const char*)httpResponse.c_str());
        return false;
    }

    logger_base.debug("%s", (const char*)httpResponse.c_str());

    wxJSONReader reader;
    wxJSONValue jsonDoc;
    wxString str(httpResponse);
    if (reader.Parse(str, &jsonDoc)) {
        wxString result;
        auto errors = reader.GetErrors();
        for (int i = 0; i < errors.GetCount(); i++) {
            result.Append(errors.Item(i)).Append(", ");
        }
        logger_base.error("Twinkly: Returned json is not valid: " + result);
        return false;
    }

    int32_t code;
    if (!jsonDoc.Get("code", "").AsInt32(code) || code != 1000) {
        logger_base.error("Twinkly: Server returned: " + std::to_string(code));
        return false;
    }

    auto coords = jsonDoc.Get("coordinates", "").AsArray();

    for (uint32_t i = 0; i < coords->Count(); i++) {
        auto v = coords->Item(i);
        // we invert Y as that is how it comes from Twinkly
        result.push_back(std::tuple<float, float, float>(v["x"].AsDouble(), 1.0 - v["y"].AsDouble(), v["z"].AsDouble()));
    }

    return true;
}

bool TwinklyOutput::GetLayout(std::vector<std::tuple<float,float,float>>& result)
{
    return GetLayout(_ip, result);
}

#ifndef EXCLUDENETWORKUI
wxJSONValue TwinklyOutput::Query(const std::string& ip, uint8_t type, const std::string& localIP)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxJSONValue val;

    uint8_t packet[9];
    memset(&packet, 0x00, sizeof(packet));

    packet[0] = 0x01;
    packet[1] = 'd';
    packet[2] = 'i';
    packet[3] = 's';
    packet[4] = 'c';
    packet[5] = 'o';
    packet[6] = 'v';
    packet[7] = 'e';
    packet[8] = 'r';

    wxIPV4address localaddr;
    if (localIP == "") {
        localaddr.AnyAddress();
    } else {
        localaddr.Hostname(localIP);
    }

    logger_base.debug(" Twinkly query using %s", (const char*)localaddr.IPAddress().c_str());
    wxDatagramSocket* datagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);

    if (datagram == nullptr) {
        logger_base.error("Error initialising Twinkly query datagram.");
    } else if (!datagram->IsOk()) {
        logger_base.error("Error initialising Twinkly query datagram ... is network connected? OK : FALSE");
        delete datagram;
        datagram = nullptr;
    } else if (datagram->Error() != wxSOCKET_NOERROR) {
        logger_base.error("Error creating Twinkly query datagram => %d : %s.", datagram->LastError(), (const char*)DecodeIPError(datagram->LastError()).c_str());
        delete datagram;
        datagram = nullptr;
    } else {
        logger_base.info("Twinkly query datagram opened successfully.");
    }

    wxIPV4address remoteaddr;
    remoteaddr.Hostname(ip);
    remoteaddr.Service(DISCOVERY_PORT);

    // bail if we dont have a datagram to use
    if (datagram != nullptr) {
        logger_base.info("Twinkly sending query packet.");
        datagram->SendTo(remoteaddr, &packet, sizeof(packet));
        if (datagram->Error() != wxSOCKET_NOERROR) {
            logger_base.error("Error sending Twinkly query datagram => %d : %s.", datagram->LastError(), (const char*)DecodeIPError(datagram->LastError()).c_str());
        } else {
            logger_base.info("Twinkly sent query packet. Sleeping for 1 second.");

            // give the controllers 2 seconds to respond
            wxMilliSleep(1000);

            uint8_t response[1024];

            int lastread = 1;

            while (lastread > 0) {
                wxStopWatch sw;
                logger_base.debug("Trying to read Twinkly query response packet.");
                memset(&response, 0x00, sizeof(response));
                datagram->Read(&response, sizeof(response));
                lastread = datagram->LastReadCount();

                if (lastread > 0) {
                    logger_base.debug(" Read done. %d bytes %ldms", lastread, sw.Time());

                    if (response[0] == 0x01 && response[1] == 'd' && response[2] == 'i') {
                        // getting my own QUERY request, ignore
                    } else if (response[4] == 'O' && response[5] == 'K') {
                        logger_base.debug(" Valid response.");
                        logger_base.debug((const char*)&response[6]);
                    }
                }
                logger_base.info("Twinkly Query Done looking for response.");
            }
        }
        datagram->Close();
        delete datagram;
    }
    logger_base.info("Twinkly Query Finished.");

    return val;
}
#endif

void TwinklyOutput::PrepareDiscovery(Discovery& discovery)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    discovery.AddBroadcast(DISCOVERY_PORT, [&discovery](wxDatagramSocket* socket, uint8_t* response, int len) {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        if (response[0] == 0x01 && response[1] == 'd' && response[2] == 'i') {
            // getting my own QUERY request, ignore
            return;
        }
        // discovery response format:
        // DD CC BB AA - // AA.BB.CC.DD ip address of responder
        // OK
        // Name of the controller
        if (response[4] == 'O' && response[5] == 'K') {
            logger_base.debug(" Valid Twinkly Status Response.");
            logger_base.debug((const char*)&response[6]);

            wxIPV4address add;
            socket->GetPeer(add);
            std::string ip = add.IPAddress();
            DiscoveredData* dd = discovery.FindByIp(ip);
            if (dd == nullptr) {
                ControllerEthernet* controller = new ControllerEthernet(discovery.GetOutputManager(), false);
                controller->SetProtocol(OUTPUT_TWINKLY);
                logger_base.debug("   IP %s", (const char*)ip.c_str());
                controller->SetAutoSize(false, nullptr); // output model manager not required when setting it to false
                controller->SetIP(ip);
                controller->SetId(1);
                controller->EnsureUniqueId();
                controller->SetName((char*)&response[6]);

                std::vector<std::tuple<float, float, float>> pixels;
                if (GetLayout(ip, pixels)) {
                    controller->SetChannelSize(pixels.size() * 3);
                }

                dd = discovery.AddController(controller);
            }
        }
    });

    logger_base.info("Sending Twinkly Discovery.");
    uint8_t packet[9];
    memset(&packet, 0x00, sizeof(packet));
    packet[0] = 0x01;
    packet[1] = 'd';
    packet[2] = 'i';
    packet[3] = 's';
    packet[4] = 'c';
    packet[5] = 'o';
    packet[6] = 'v';
    packet[7] = 'e';
    packet[8] = 'r';
    discovery.SendBroadcastData(DISCOVERY_PORT, packet, sizeof(packet));
}
#pragma endregion
