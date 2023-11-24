
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

// this is how often we re-call the web service on the controller to put it in real time mode
// If the controller loses power this is how long it may take to start to accept xlights data
#define ENSURELEDMODE_SECS 60

#pragma region Constructors and Destructors
TwinklyOutput::TwinklyOutput(wxXmlNode* node, bool isActive) :
    IPOutput(node, isActive)
{
    _httpPort = wxAtoi(node->GetAttribute("HTTPPort", "80"));
}

TwinklyOutput::TwinklyOutput()
{
}

TwinklyOutput::TwinklyOutput(const TwinklyOutput& from) :
    IPOutput(from){
    _httpPort = from._httpPort;
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
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");

    node->AddAttribute("HTTPPort", wxString::Format("%d", _httpPort));

    IPOutput::Save(node);

    return node;
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
bool TwinklyOutput::SetLEDMode(bool rt)
{
    wxJSONValue result;
    if (rt) {
        if (!MakeCall("POST", "/xled/v1/led/mode", result, "{\"mode\": \"rt\"}")) {
            return false;
        }
        _lastLEDModeTime = wxGetLocalTimeMillis();
    }
    else
    {
        if (!_fppProxyOutput) {
            // turn off
            MakeCall("POST", "/xled/v1/led/mode", result, "{\"mode\": \"off\"}");
        }
    }
    return true;
}

bool TwinklyOutput::Open()
{
    if (!_enabled)
        return true;

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
    if (!SetLEDMode(true)) {
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

    if (_enabled)
        SetLEDMode(false);

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

    if (wxGetLocalTimeMillis() - _lastLEDModeTime > ENSURELEDMODE_SECS * 1000)
    {
        SetLEDMode(true);
    }
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
    logger_base.debug("Twinkly: Invoke " + method + " http://" + _ip + (_httpPort == 80
                          ? ""
                          : wxString::Format(":%d", _httpPort)) + path);
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
    std::string httpResponse = Curl::HTTPSPost("http://" + _ip + (_httpPort == 80 ? "" : wxString::Format(":%d", _httpPort)) + path, bod, "", "", "JSON", HTTP_TIMEOUT, customHeaders, &responseCode);

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

    _datagram = new wxDatagramSocket(localaddr, wxSOCKET_BLOCK); // dont use NOWAIT as it can result in dropped packets
    if (_datagram == nullptr) {
        logger_base.error("Twinkly: %s Error opening datagram.", (const char*)localaddr.IPAddress().c_str());
    } else if (!_datagram->IsOk()) {
        logger_base.error("Twinkly: %s Error opening datagram. Network may not be connected? OK : FALSE", (const char*)localaddr.IPAddress().c_str());
        delete _datagram;
        _datagram = nullptr;
    } else if (_datagram->Error()) {
        logger_base.error("Twinkly: %s Error creating Twinkly datagram => %d : %s.", (const char*)localaddr.IPAddress().c_str(), _datagram->LastError(), (const char*)DecodeIPError(_datagram->LastError()).c_str());
        delete _datagram;
        _datagram = nullptr;
    }
}
#pragma endregion

bool TwinklyOutput::GetLayout(const std::string& ip, std::vector<std::tuple<float, float, float>>& result, bool& is3D, uint16_t httpPort)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    #ifdef TEST_TWINKLY_FORMAT
    std::string httpResponse = "{\"source\":\"3d\",\"synthesized\":false,\"uuid\":\"18040665-6CCE-4402-F257-66BF76027922\",\"coordinates\":[{\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.930176,\"y\":0.055054,\"z\":-1.000000},\
                                                                                                                                {\"x\":0.931396,\"y\":0.054138,\"z\":-0.997070},\
                                                                                                                                {\"x\":0.932495,\"y\":0.053223,\"z\":-0.994019},\
                                                                                                                                {\"x\":0.933716,\"y\":0.052307,\"z\":-0.990967},\
                                                                                                                                {\"x\":0.934814,\"y\":0.051392,\"z\":-0.987915},\
                                                                                                                                {\"x\":0.936035,\"y\":0.050476,\"z\":-0.984985},\
                                                                                                                                {\"x\":0.937134,\"y\":0.049561,\"z\":-0.981934},\
                                                                                                                                {\"x\":0.938354,\"y\":0.048645,\"z\":-0.978882},\
                                                                                                                                {\"x\":0.939453,\"y\":0.047729,\"z\":-0.975830},\
                                                                                                                                {\"x\":0.940674,\"y\":0.046753,\"z\":-0.972900},\
                                                                                                                                {\"x\":0.941772,\"y\":0.045837,\"z\":-0.969849},\
                                                                                                                                {\"x\":0.942993,\"y\":0.044922,\"z\":-0.966797},\
                                                                                                                                {\"x\":0.944092,\"y\":0.044006,\"z\":-0.963745},\
                                                                                                                                {\"x\":0.945312,\"y\":0.043091,\"z\":-0.960693},\
                                                                                                                                {\"x\":0.946411,\"y\":0.042175,\"z\":-0.957764},\
                                                                                                                                {\"x\":0.947632,\"y\":0.041260,\"z\":-0.954712},\
                                                                                                                                {\"x\":0.948730,\"y\":0.040344,\"z\":-0.951660},\
                                                                                                                                {\"x\":0.949951,\"y\":0.039429,\"z\":-0.948608},\
                                                                                                                                {\"x\":0.951050,\"y\":0.038513,\"z\":-0.945679},\
                                                                                                                                {\"x\":0.952271,\"y\":0.037598,\"z\":-0.942627},\
                                                                                                                                {\"x\":0.953491,\"y\":0.036682,\"z\":-0.939575},\
                                                                                                                                {\"x\":0.954590,\"y\":0.035767,\"z\":-0.936523},\
                                                                                                                                {\"x\":0.955811,\"y\":0.034851,\"z\":-0.933472},\
                                                                                                                                {\"x\":0.956909,\"y\":0.033936,\"z\":-0.930542},\
                                                                                                                                {\"x\":0.958130,\"y\":0.033020,\"z\":-0.927490},\
                                                                                                                                {\"x\":0.959229,\"y\":0.032104,\"z\":-0.924438},\
                                                                                                                                {\"x\":0.960449,\"y\":0.031189,\"z\":-0.921387},\
                                                                                                                                {\"x\":0.961548,\"y\":0.030273,\"z\":-0.918457},\
                                                                                                                                {\"x\":0.962769,\"y\":0.029358,\"z\":-0.915405},\
                                                                                                                                {\"x\":0.963867,\"y\":0.028442,\"z\":-0.912354},\
                                                                                                                                {\"x\":0.965088,\"y\":0.027527,\"z\":-0.909302},\
                                                                                                                                {\"x\":0.966187,\"y\":0.026611,\"z\":-0.906372},\
                                                                                                                                {\"x\":0.967407,\"y\":0.025696,\"z\":-0.903320},\
                                                                                                                                {\"x\":0.968506,\"y\":0.024780,\"z\":-0.900269},\
                                                                                                                                {\"x\":0.969727,\"y\":0.023865,\"z\":-0.897217},\
                                                                                                                                {\"x\":0.970825,\"y\":0.022888,\"z\":-0.894165},\
                                                                                                                                {\"x\":0.972046,\"y\":0.021973,\"z\":-0.891235},\
                                                                                                                                {\"x\":0.973145,\"y\":0.021057,\"z\":-0.888184},\
                                                                                                                                {\"x\":0.974365,\"y\":0.020142,\"z\":-0.885132},\
                                                                                                                                {\"x\":0.975464,\"y\":0.019226,\"z\":-0.882080},\
                                                                                                                                {\"x\":0.976685,\"y\":0.018311,\"z\":-0.879150},\
                                                                                                                                {\"x\":0.977905,\"y\":0.017395,\"z\":-0.876099},\
                                                                                                                                {\"x\":0.979004,\"y\":0.016479,\"z\":-0.873047},\
                                                                                                                                {\"x\":0.980225,\"y\":0.015564,\"z\":-0.869995},\
                                                                                                                                {\"x\":0.981323,\"y\":0.014648,\"z\":-0.866943},\
                                                                                                                                {\"x\":0.982544,\"y\":0.013733,\"z\":-0.864014},\
                                                                                                                                {\"x\":0.983643,\"y\":0.012817,\"z\":-0.860962},\
                                                                                                                                {\"x\":0.984863,\"y\":0.011902,\"z\":-0.857910},\
                                                                                                                                {\"x\":0.985962,\"y\":0.010986,\"z\":-0.854858},\
                                                                                                                                {\"x\":0.987183,\"y\":0.010071,\"z\":-0.851929},\
                                                                                                                                {\"x\":0.988281,\"y\":0.009155,\"z\":-0.848877},\
                                                                                                                                {\"x\":0.989502,\"y\":0.008240,\"z\":-0.845825},\
                                                                                                                                {\"x\":0.990601,\"y\":0.007324,\"z\":-0.842773},\
                                                                                                                                {\"x\":0.991821,\"y\":0.006409,\"z\":-0.839722},\
                                                                                                                                {\"x\":0.992920,\"y\":0.005493,\"z\":-0.836792},\
                                                                                                                                {\"x\":0.994141,\"y\":0.004578,\"z\":-0.833740},\
                                                                                                                                {\"x\":0.995239,\"y\":0.003662,\"z\":-0.830688},\
                                                                                                                                {\"x\":0.996460,\"y\":0.002747,\"z\":-0.827637},\
                                                                                                                                {\"x\":0.997559,\"y\":0.001831,\"z\":-0.824707},\
                                                                                                                                {\"x\":0.998779,\"y\":1.178772,\"z\":-0.821655},\
                                                                                                                                {\"x\":1.000000,\"y\":0.000000,\"z\":-0.818604},\
                                                                                                                                {\"x\":0.999634,\"y\":3.214050,\"z\":-0.819336},\
                                                                                                                                {\"x\":0.999390,\"y\":0.001404,\"z\":-0.820068},\
                                                                                                                                {\"x\":0.999146,\"y\":0.002136,\"z\":-0.820679},\
                                                                                                                                {\"x\":0.998901,\"y\":0.002869,\"z\":-0.821411},\
                                                                                                                                {\"x\":0.998657,\"y\":0.003601,\"z\":-0.822144},\
                                                                                                                                {\"x\":0.998413,\"y\":0.004272,\"z\":-0.822876},\
                                                                                                                                {\"x\":0.998169,\"y\":0.005005,\"z\":-0.823486},\
                                                                                                                                {\"x\":0.997925,\"y\":0.005737,\"z\":-0.824219},\
                                                                                                                                {\"x\":0.997681,\"y\":0.006470,\"z\":-0.824951},\
                                                                                                                                {\"x\":0.997437,\"y\":0.007202,\"z\":-0.825562},\
                                                                                                                                {\"x\":0.997192,\"y\":0.007935,\"z\":-0.826294},\
                                                                                                                                {\"x\":0.996948,\"y\":0.008606,\"z\":-0.827026},\
                                                                                                                                {\"x\":0.996704,\"y\":0.009338,\"z\":-0.827759},\
                                                                                                                                {\"x\":0.996460,\"y\":0.010071,\"z\":-0.828369},\
                                                                                                                                {\"x\":0.996216,\"y\":0.010803,\"z\":-0.829102},\
                                                                                                                                {\"x\":0.995972,\"y\":0.011536,\"z\":-0.829834},\
                                                                                                                                {\"x\":0.995728,\"y\":0.012207,\"z\":-0.830566},\
                                                                                                                                {\"x\":0.995361,\"y\":0.012939,\"z\":-0.831177},\
                                                                                                                                {\"x\":0.995117,\"y\":0.013672,\"z\":-0.831909},\
                                                                                                                                {\"x\":0.994873,\"y\":0.014404,\"z\":-0.832642},\
                                                                                                                                {\"x\":0.994629,\"y\":0.015137,\"z\":-0.833374},\
                                                                                                                                {\"x\":0.994385,\"y\":0.015869,\"z\":-0.833984},\
                                                                                                                                {\"x\":0.994141,\"y\":0.016541,\"z\":-0.834717},\
                                                                                                                                {\"x\":0.993896,\"y\":0.017273,\"z\":-0.835449},\
                                                                                                                                {\"x\":0.993652,\"y\":0.018005,\"z\":-0.836182},\
                                                                                                                                {\"x\":0.993408,\"y\":0.018738,\"z\":-0.836792},\
                                                                                                                                {\"x\":0.993164,\"y\":0.019470,\"z\":-0.837524},\
                                                                                                                                {\"x\":0.992920,\"y\":0.020142,\"z\":-0.838257},\
                                                                                                                                {\"x\":0.992676,\"y\":0.020874,\"z\":-0.838867},\
                                                                                                                                {\"x\":0.992432,\"y\":0.021606,\"z\":-0.839600},\
                                                                                                                                {\"x\":0.992188,\"y\":0.022339,\"z\":-0.840332},\
                                                                                                                                {\"x\":0.991943,\"y\":0.023071,\"z\":-0.841064},\
                                                                                                                                {\"x\":0.991699,\"y\":0.023804,\"z\":-0.841675},\
                                                                                                                                {\"x\":0.991455,\"y\":0.024475,\"z\":-0.842407},\
                                                                                                                                {\"x\":0.991089,\"y\":0.025208,\"z\":-0.843140},\
                                                                                                                                {\"x\":0.990845,\"y\":0.025940,\"z\":-0.843872},\
                                                                                                                                {\"x\":0.990601,\"y\":0.026672,\"z\":-0.844482},\
                                                                                                                                {\"x\":0.990356,\"y\":0.027405,\"z\":-0.845215},\
                                                                                                                                {\"x\":0.990112,\"y\":0.028076,\"z\":-0.845947},\
                                                                                                                                {\"x\":0.989868,\"y\":0.028809,\"z\":-0.846680},\
                                                                                                                                {\"x\":0.989624,\"y\":0.029541,\"z\":-0.847290},\
                                                                                                                                {\"x\":0.989380,\"y\":0.030273,\"z\":-0.848022},\
                                                                                                                                {\"x\":0.989380,\"y\":0.030273,\"z\":-0.848022},\
                                                                                                                                {\"x\":0.989380,\"y\":0.030273,\"z\":-0.848022},\
                                                                                                                                {\"x\":0.989380,\"y\":0.030273,\"z\":-0.848022},\
                                                                                                                                {\"x\":0.989380,\"y\":0.030273,\"z\":-0.848022},\
                                                                                                                                {\"x\":0.989380,\"y\":0.030273,\"z\":-0.848022},\
                                                                                                                                {\"x\":0.989380,\"y\":0.030273,\"z\":-0.848022},\
                                                                                                                                {\"x\":0.459106,\"y\":0.061157,\"z\":-0.579590},\
                                                                                                                                {\"x\":0.321533,\"y\":0.063354,\"z\":-0.368042},\
                                                                                                                                {\"x\":0.358398,\"y\":0.059814,\"z\":-0.643433},\
                                                                                                                                {\"x\":0.333130,\"y\":0.140930,\"z\":-0.343994},\
                                                                                                                                {\"x\":0.313477,\"y\":0.110535,\"z\":-0.561890},\
                                                                                                                                {\"x\":0.345947,\"y\":0.234070,\"z\":-0.335938},\
                                                                                                                                {\"x\":0.225586,\"y\":0.246948,\"z\":-0.462646},\
                                                                                                                                {\"x\":0.261475,\"y\":0.311768,\"z\":-0.403809},\
                                                                                                                                {\"x\":0.297363,\"y\":0.376587,\"z\":-0.344971},\
                                                                                                                                {\"x\":0.036377,\"y\":0.448486,\"z\":-0.257202},\
                                                                                                                                {\"x\":0.261841,\"y\":0.478638,\"z\":-0.271973},\
                                                                                                                                {\"x\":0.240601,\"y\":0.513977,\"z\":-0.260864},\
                                                                                                                                {\"x\":0.219360,\"y\":0.549255,\"z\":-0.249756},\
                                                                                                                                {\"x\":0.106812,\"y\":0.612671,\"z\":-0.318848},\
                                                                                                                                {\"x\":0.213989,\"y\":0.666016,\"z\":-0.107300},\
                                                                                                                                {\"x\":0.147461,\"y\":0.717468,\"z\":-0.241455},\
                                                                                                                                {\"x\":0.044678,\"y\":0.701477,\"z\":-0.133667},\
                                                                                                                                {\"x\":0.213989,\"y\":0.635193,\"z\":-0.119141},\
                                                                                                                                {\"x\":0.183472,\"y\":0.709290,\"z\":0.043823},\
                                                                                                                                {\"x\":0.226562,\"y\":0.636047,\"z\":-0.067627},\
                                                                                                                                {\"x\":0.268066,\"y\":0.657898,\"z\":0.114502},\
                                                                                                                                {\"x\":0.307861,\"y\":0.580505,\"z\":-0.487061},\
                                                                                                                                {\"x\":0.346802,\"y\":0.612793,\"z\":0.171631},\
                                                                                                                                {\"x\":0.352295,\"y\":0.523743,\"z\":0.072144},\
                                                                                                                                {\"x\":0.402832,\"y\":0.529846,\"z\":0.239136},\
                                                                                                                                {\"x\":0.361328,\"y\":0.430420,\"z\":0.189453},\
                                                                                                                                {\"x\":0.401611,\"y\":0.408875,\"z\":0.272217},\
                                                                                                                                {\"x\":0.441895,\"y\":0.387329,\"z\":0.354858},\
                                                                                                                                {\"x\":0.476196,\"y\":0.447083,\"z\":0.435913},\
                                                                                                                                {\"x\":0.380859,\"y\":0.387695,\"z\":0.558594},\
                                                                                                                                {\"x\":0.479126,\"y\":0.460876,\"z\":0.547852},\
                                                                                                                                {\"x\":0.335327,\"y\":0.432373,\"z\":0.560913},\
                                                                                                                                {\"x\":0.432617,\"y\":0.493286,\"z\":0.607422},\
                                                                                                                                {\"x\":0.420044,\"y\":0.514587,\"z\":0.575195},\
                                                                                                                                {\"x\":0.407471,\"y\":0.535889,\"z\":0.542969},\
                                                                                                                                {\"x\":0.381226,\"y\":0.567444,\"z\":0.544800},\
                                                                                                                                {\"x\":0.354980,\"y\":0.598999,\"z\":0.546631},\
                                                                                                                                {\"x\":0.360596,\"y\":0.634155,\"z\":0.503906},\
                                                                                                                                {\"x\":0.366211,\"y\":0.669250,\"z\":0.461304},\
                                                                                                                                {\"x\":0.253784,\"y\":0.708374,\"z\":0.298950},\
                                                                                                                                {\"x\":0.317017,\"y\":0.754028,\"z\":0.476929},\
                                                                                                                                {\"x\":0.311523,\"y\":0.793640,\"z\":0.460815},\
                                                                                                                                {\"x\":0.305908,\"y\":0.833313,\"z\":0.444824},\
                                                                                                                                {\"x\":0.219727,\"y\":0.841919,\"z\":0.344238},\
                                                                                                                                {\"x\":0.298706,\"y\":0.884888,\"z\":0.282837},\
                                                                                                                                {\"x\":0.187134,\"y\":0.862732,\"z\":0.190063},\
                                                                                                                                {\"x\":0.259033,\"y\":0.884827,\"z\":0.098389},\
                                                                                                                                {\"x\":0.179932,\"y\":0.845093,\"z\":0.119507},\
                                                                                                                                {\"x\":0.204590,\"y\":0.827881,\"z\":-0.023560},\
                                                                                                                                {\"x\":0.109253,\"y\":0.748962,\"z\":0.034790},\
                                                                                                                                {\"x\":0.109863,\"y\":0.786316,\"z\":-0.084839},\
                                                                                                                                {\"x\":-0.008545,\"y\":0.795471,\"z\":-0.082764},\
                                                                                                                                {\"x\":-0.070801,\"y\":0.706604,\"z\":-0.156494},\
                                                                                                                                {\"x\":-0.113403,\"y\":0.761841,\"z\":-0.020020},\
                                                                                                                                {\"x\":-0.215698,\"y\":0.767212,\"z\":-0.196533},\
                                                                                                                                {\"x\":-0.144897,\"y\":0.826782,\"z\":-0.174072},\
                                                                                                                                {\"x\":-0.436401,\"y\":0.880554,\"z\":0.284058},\
                                                                                                                                {\"x\":-0.209717,\"y\":0.886902,\"z\":-0.151733},\
                                                                                                                                {\"x\":-0.496948,\"y\":0.908997,\"z\":-0.229980},\
                                                                                                                                {\"x\":-0.473999,\"y\":0.939453,\"z\":-0.175049},\
                                                                                                                                {\"x\":-0.451172,\"y\":0.969910,\"z\":-0.119995},\
                                                                                                                                {\"x\":-0.576538,\"y\":1.000000,\"z\":-0.296753},\
                                                                                                                                {\"x\":-0.422119,\"y\":0.967224,\"z\":-0.175781},\
                                                                                                                                {\"x\":-0.597534,\"y\":0.990723,\"z\":-0.170288},\
                                                                                                                                {\"x\":-0.503906,\"y\":0.961548,\"z\":-0.013062},\
                                                                                                                                {\"x\":-0.581177,\"y\":0.935791,\"z\":0.874268},\
                                                                                                                                {\"x\":-0.658447,\"y\":0.910095,\"z\":0.014771},\
                                                                                                                                {\"x\":-0.735840,\"y\":0.884338,\"z\":0.028687},\
                                                                                                                                {\"x\":-0.813110,\"y\":0.858582,\"z\":0.042603},\
                                                                                                                                {\"x\":-0.719116,\"y\":0.860046,\"z\":0.225830},\
                                                                                                                                {\"x\":-0.895630,\"y\":0.851501,\"z\":0.185425},\
                                                                                                                                {\"x\":-0.827515,\"y\":0.830139,\"z\":0.363037},\
                                                                                                                                {\"x\":-0.849854,\"y\":0.798706,\"z\":0.390869},\
                                                                                                                                {\"x\":-0.872192,\"y\":0.767334,\"z\":0.418823},\
                                                                                                                                {\"x\":-0.894531,\"y\":0.735901,\"z\":0.446655},\
                                                                                                                                {\"x\":-0.947266,\"y\":0.685913,\"z\":0.462402},\
                                                                                                                                {\"x\":-1.000000,\"y\":0.635864,\"z\":0.478149},\
                                                                                                                                {\"x\":-0.821289,\"y\":0.629883,\"z\":0.443848},\
                                                                                                                                {\"x\":-0.812622,\"y\":0.568542,\"z\":0.439575},\
                                                                                                                                {\"x\":-0.803833,\"y\":0.507202,\"z\":0.435303},\
                                                                                                                                {\"x\":-0.771240,\"y\":0.489319,\"z\":0.377197},\
                                                                                                                                {\"x\":-0.738770,\"y\":0.471497,\"z\":0.319092},\
                                                                                                                                {\"x\":-0.706177,\"y\":0.453613,\"z\":0.260986},\
                                                                                                                                {\"x\":-0.663330,\"y\":0.452698,\"z\":0.249634},\
                                                                                                                                {\"x\":-0.620483,\"y\":0.451782,\"z\":0.238281},\
                                                                                                                                {\"x\":-0.452026,\"y\":0.462463,\"z\":0.346313},\
                                                                                                                                {\"x\":-0.412964,\"y\":0.513123,\"z\":0.280396},\
                                                                                                                                {\"x\":-0.373901,\"y\":0.563782,\"z\":0.214478},\
                                                                                                                                {\"x\":-0.334717,\"y\":0.614380,\"z\":0.148560},\
                                                                                                                                {\"x\":-0.295166,\"y\":0.641724,\"z\":0.193604},\
                                                                                                                                {\"x\":-0.255615,\"y\":0.669067,\"z\":0.238647},\
                                                                                                                                {\"x\":-0.410522,\"y\":0.669006,\"z\":0.169800},\
                                                                                                                                {\"x\":-0.305420,\"y\":0.711487,\"z\":0.111938},\
                                                                                                                                {\"x\":-0.442871,\"y\":0.681335,\"z\":0.128662},\
                                                                                                                                {\"x\":-0.405396,\"y\":0.739502,\"z\":0.015625},\
                                                                                                                                {\"x\":-0.416504,\"y\":0.734131,\"z\":0.008789},\
                                                                                                                                {\"x\":-0.427490,\"y\":0.728699,\"z\":0.002075},\
                                                                                                                                {\"x\":-0.438599,\"y\":0.723267,\"z\":-0.004639},\
                                                                                                                                {\"x\":-0.303589,\"y\":0.696594,\"z\":-0.119995},\
                                                                                                                                {\"x\":-0.217407,\"y\":0.727234,\"z\":-0.013916},\
                                                                                                                                {\"x\":-0.121704,\"y\":0.698486,\"z\":-0.151733},\
                                                                                                                                {\"x\":-0.119751,\"y\":0.742798,\"z\":0.009399},\
                                                                                                                                {\"x\":-0.099487,\"y\":0.673584,\"z\":0.111816},\
                                                                                                                                {\"x\":-0.095947,\"y\":0.677917,\"z\":0.176758},\
                                                                                                                                {\"x\":-0.092285,\"y\":0.682251,\"z\":0.241699},\
                                                                                                                                {\"x\":0.039429,\"y\":0.700623,\"z\":0.328369},\
                                                                                                                                {\"x\":0.112793,\"y\":0.694824,\"z\":0.302856},\
                                                                                                                                {\"x\":0.186279,\"y\":0.689087,\"z\":0.277344},\
                                                                                                                                {\"x\":0.265015,\"y\":0.679626,\"z\":0.353394},\
                                                                                                                                {\"x\":0.289185,\"y\":0.692078,\"z\":0.488770},\
                                                                                                                                {\"x\":0.227295,\"y\":0.675781,\"z\":0.572998},\
                                                                                                                                {\"x\":0.148560,\"y\":0.667725,\"z\":0.619995},\
                                                                                                                                {\"x\":0.030640,\"y\":0.645447,\"z\":0.621460},\
                                                                                                                                {\"x\":0.081543,\"y\":0.588074,\"z\":0.539429},\
                                                                                                                                {\"x\":0.048096,\"y\":0.569275,\"z\":0.661621},\
                                                                                                                                {\"x\":0.014648,\"y\":0.550476,\"z\":0.783813},\
                                                                                                                                {\"x\":-0.077393,\"y\":0.554077,\"z\":0.850220},\
                                                                                                                                {\"x\":-0.187744,\"y\":0.578735,\"z\":0.798462},\
                                                                                                                                {\"x\":-0.274902,\"y\":0.564331,\"z\":0.808228},\
                                                                                                                                {\"x\":-0.319458,\"y\":0.542969,\"z\":0.933594},\
                                                                                                                                {\"x\":-0.169067,\"y\":0.547485,\"z\":0.902588},\
                                                                                                                                {\"x\":-0.305664,\"y\":0.543091,\"z\":0.965942},\
                                                                                                                                {\"x\":-0.143433,\"y\":0.545593,\"z\":0.976196},\
                                                                                                                                {\"x\":-0.280273,\"y\":0.553894,\"z\":1.000000},\
                                                                                                                                {\"x\":-0.280273,\"y\":0.553894,\"z\":1.000000}],\"code\":1000}";
    #else
    std::vector<std::pair<std::string, std::string>> customHeaders = {};
    int responseCode;
    std::string httpResponse = Curl::HTTPSGet("http://" + ip + (httpPort == 80 ? "" : wxString::Format(":%d", httpPort)) + "/xled/v1/led/layout/full", "", "", HTTP_TIMEOUT, customHeaders, &responseCode);

    if (responseCode != 200) {
        logger_base.error("Twinkly: Error %d : %s", responseCode, (const char*)httpResponse.c_str());
        return false;
    }

    logger_base.debug("%s", (const char*)httpResponse.c_str());
    #endif

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

    is3D = jsonDoc.Get("source", "2d").AsString() == "3d";

    auto coords = jsonDoc.Get("coordinates", "").AsArray();

    for (uint32_t i = 0; i < coords->Count(); i++) {
        auto v = coords->Item(i);
        // we invert Y as that is how it comes from Twinkly
        result.push_back(std::tuple<float, float, float>(v["x"].AsDouble(), 1.0 - v["y"].AsDouble(), v["z"].AsDouble()));
    }

    return true;
}

bool TwinklyOutput::GetLayout(std::vector<std::tuple<float, float, float>>& result, bool& is3D)
{
    return GetLayout(_ip, result, is3D);
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
    wxDatagramSocket* datagram = new wxDatagramSocket(localaddr, wxSOCKET_BLOCK); // dont use NOWAIT as it can result in dropped packets

    if (datagram == nullptr) {
        logger_base.error("Error initialising Twinkly query datagram.");
    } else if (!datagram->IsOk()) {
        logger_base.error("Error initialising Twinkly query datagram ... is network connected? OK : FALSE");
        delete datagram;
        datagram = nullptr;
    } else if (datagram->Error()) {
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
        if (datagram->Error()) {
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
                bool is3D = false;
                if (GetLayout(ip, pixels, is3D)) {
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


#pragma region UI
#ifndef EXCLUDENETWORKUI

#include "OutputModelManager.h"
void TwinklyOutput::UpdateProperties(wxPropertyGrid* propertyGrid, Controller *c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    IPOutput::UpdateProperties(propertyGrid, c, modelManager, expandProperties);
    auto p = propertyGrid->GetProperty("HTTPPort");
    if (p) {
        p->SetValue(GetHttpPort());
    }
    p = propertyGrid->GetProperty("Channels");
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
void TwinklyOutput::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty *before, Controller *c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    IPOutput::AddProperties(propertyGrid, before, c, allSameSize, expandProperties);
    auto p = propertyGrid->Insert(before, new wxUIntProperty("HTTP Port", "HTTPPort", 80));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 65535);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("Twinkly normally listens on port 80 but you may want to change the port if using Artnet To Twinkly.");
    
    p = propertyGrid->Insert(before, new wxUIntProperty("Channels", "Channels", GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", GetMaxChannels());
}
void TwinklyOutput::RemoveProperties(wxPropertyGrid* propertyGrid) {
    IPOutput::RemoveProperties(propertyGrid);
    propertyGrid->DeleteProperty("HTTPPort");
    propertyGrid->DeleteProperty("Channels");
}
bool TwinklyOutput::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller *c) {
    if (IPOutput::HandlePropertyEvent(event, outputModelManager, c)) {
        return true;
    }
    wxString const name = event.GetPropertyName();
    if (name == "HTTPPort") {
        SetHttpPort(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "TwinklyOutput::HandlePropertyEvent::HTTPPort");
        return true;
    } else if (name == "Channels") {
        SetChannels(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "TwinklyOutput::HandlePropertyEvent::Channels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "TwinklyOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "TwinklyOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "TwinklyOutput::HandlePropertyEvent::Channels", nullptr);
        return true;
    }
    return false;
}
#endif
#pragma endregion UI
