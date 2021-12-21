#include "TwinklyOutput.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"
#include "../xSchedule/wxJSON/jsonreader.h"
#include <curl/curl.h>
#include <log4cpp/Category.hh>
#include <wx/base64.h>
#include <wx/protocol/http.h>
#include <wx/sstream.h>

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
#pragma endregion

#pragma region Start and Stop
bool TwinklyOutput::Open()
{
    if (!IPOutput::Open()) {
        return false;
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

    // turn off
    wxJSONValue result;
    MakeCall("POST", "/xled/v1/led/mode", result, "{\"mode\": \"off\"}");

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
    if (_datagram == nullptr && OutputManager::IsRetryOpen()) {
        OpenDatagram();
        if (_ok) {
            logger_base.debug("E131Output: Open retry successful");
        }
    }

    _timer_msec = msec;
}

void TwinklyOutput::EndFrame(int suppressFrames)
{
    if (!_enabled || _suspend || _tempDisable || _datagram == nullptr) {
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
        std::memcpy(&packet[i], m_decodedToken.data(), TOKEN_SIZE);
        i += TOKEN_SIZE;

        // 2 zeros
        packet[i++] = 0;
        packet[i++] = 0;

        // 1 byte fragment number
        packet[i++] = fragmentNumber++;

        // send 900 channels
        int payloadSize = std::min(900, (int)_channels);
        std::memcpy(&packet[i], &m_channelData[offset], payloadSize);
        offset += payloadSize;
        i += payloadSize;

        wxASSERT(i <= PACKET_SIZE);
        _datagram->SendTo(remoteAddr, packet, PACKET_SIZE);
    }

    FrameOutput();
}

void TwinklyOutput::ResetFrame()
{
}
#pragma endregion

#pragma region Frame Handling
void TwinklyOutput::SetOneChannel(int32_t channel, unsigned char data)
{
    if (_channels > m_channelData.size()) {
        m_channelData.resize(_channels);
    }
    m_channelData[channel] = data;
}
void TwinklyOutput::SetManyChannels(int32_t channel, unsigned char* data, size_t size)
{
    if (_channels > m_channelData.size()) {
        m_channelData.resize(_channels);
    }
    std::copy(data, data + size, m_channelData.data());
}
void TwinklyOutput::AllOff()
{
    std::memset(m_channelData.data(), 0, m_channelData.size());
}
#pragma endregion

bool TwinklyOutput::MakeCall(const std::string& method, const std::string& path, wxJSONValue& result, const char* body)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Twinkly: Invoke " + method + " http://" + _ip + path);

    // todo: use curl
    wxHTTP http;
    http.SetTimeout(HTTP_TIMEOUT);
    if (!http.Connect(_ip, 80)) {
        logger_base.error("Twinkly: Connection to " + _ip + " failed");
        return false;
    }
    http.SetMethod(method);
    if (body != nullptr) {
        wxString str = body;
        //http.SetHeader(wxT("Content-Length"), wxString::Format(wxT("%d"), str.Length()));
        http.SetPostBuffer(str);
    }
    if (!m_token.empty()) {
        // assign authentication token if present
        http.SetHeader("X-Auth-Token", m_token);
    }

    wxInputStream* httpStream = http.GetInputStream(path);
    logger_base.error("Twinkly: Http response: " + std::to_string(http.GetResponse()));
    http.Close();

    if (http.GetError() != wxPROTO_NOERR) {
        wxDELETE(httpStream);
        return false;
    }

    wxString res;
    wxStringOutputStream out_stream(&res);
    httpStream->Read(out_stream);
    wxDELETE(httpStream);

    wxJSONReader reader;
    wxString str(res);
    if (reader.Parse(str, &result)) {
        wxString result;
        auto errors = reader.GetErrors();
        for (int i = 0; i < errors.GetCount(); i++) {
            result.Append(errors.Item(i)).Append(", ");
        }
        logger_base.error("Twinkly: Returned json is not valid: " + result);
        return false;
    }

    int32_t code;
    if (!result.Get("code", "").AsInt32(code) || code != 1000) {
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
    if (IPOutput::__localIP == "") {
        localaddr.AnyAddress();
    } else {
        localaddr.Hostname(IPOutput::__localIP);
    }

    _datagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);
    if (_datagram == nullptr) {
        logger_base.error("Twinkly: %s Error opening datagram.", (const char*)localaddr.IPAddress().c_str());
    } else if (!_datagram->IsOk()) {
        logger_base.error("Twinkly: %s Error opening datagram. Network may not be connected? OK : FALSE", (const char*)localaddr.IPAddress().c_str());
        delete _datagram;
        _datagram = nullptr;
    } else if (_datagram->Error() != wxSOCKET_NOERROR) {
        logger_base.error("Twinkly: %s Error creating E131 datagram => %d : %s.", (const char*)localaddr.IPAddress().c_str(), _datagram->LastError(), (const char*)DecodeIPError(_datagram->LastError()).c_str());
        delete _datagram;
        _datagram = nullptr;
    }
}
#pragma endregion

size_t TwinklyOutput::CurlWriteFunction(void* ptr, size_t size, size_t nmemb, std::string* data)
{
    if (data == nullptr)
        return 0;
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

bool TwinklyOutput::GetLayout(wxJSONValue& result, bool& reportError)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    reportError = true;

    // use curl as chunked text may break wxHTTP
    auto curl = curl_easy_init();
    wxASSERT(curl);

    auto url = std::string("http://") + _ip + "/xled/v1/led/layout/full";
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, (long)2);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)2);

    std::string buffer = "";
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteFunction);

    CURLcode ret = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (ret == CURLE_OPERATION_TIMEDOUT) {
        wxMessageBox(wxString::Format("Twinkly device not found at %s", _ip), "Error!", wxOK);
        reportError = false;
        return false;
    }
    if (ret != CURLE_OK) {
        return false;
    }

    wxJSONReader reader;
    wxString str(buffer);
    if (reader.Parse(str, &result)) {
        wxString result;
        auto errors = reader.GetErrors();
        for (int i = 0; i < errors.GetCount(); i++) {
            result.Append(errors.Item(i)).Append(", ");
        }
        logger_base.error("Twinkly: Returned json is not valid: " + result);
        return false;
    }

    int32_t code;
    if (!result.Get("code", "").AsInt32(code) || code != 1000) {
        logger_base.error("Twinkly: Server returned: " + std::to_string(code));
        return false;
    }

    return true;
}
