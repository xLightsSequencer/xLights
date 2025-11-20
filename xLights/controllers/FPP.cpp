/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <map>
#include <string.h>
#include <cctype>
#include <thread>
#include <cinttypes>

#include <curl/curl.h>

#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/regex.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/sckstrm.h>
#include <wx/zstream.h>
#include <wx/mstream.h>
#include <wx/protocol/http.h>
#include <wx/config.h>
#include <wx/secretstore.h>
#include <wx/progdlg.h>
#include <wx/gauge.h>
#include <zstd.h>
#include <wx/debugrpt.h>

#include "FPP.h"
#include "../models/CustomModel.h"
#include "../models/Model.h"
#include "../models/MatrixModel.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../outputs/E131Output.h"
#include "../outputs/DDPOutput.h"
#include "../outputs/ArtNetOutput.h"
#include "../outputs/KinetOutput.h"
#include "../outputs/TwinklyOutput.h"
#include "../outputs/ControllerEthernet.h"
#include "../outputs/ControllerSerial.h"
#include "../UtilFunctions.h"
#include "../xLightsVersion.h"
#include "../Parallel.h"
#include "ControllerCaps.h"
#include "../ExternalHooks.h"
#include "../TempFileManager.h"

#include <log4cpp/Category.hh>
#include "ControllerUploadData.h"
#include "FPPUploadProgressDialog.h"
#include "../FSEQFile.h"
#include "../Discovery.h"
#include "../utils/CurlManager.h"
#include "../utils/ip_utils.h"

#include "Falcon.h"
#include "Minleon.h"
#include "SanDevices.h"
#include "J1Sys.h"

#include "../TraceLog.h"
using namespace TraceLog;

static const std::string LEDPANELS("LED Panels");


static std::set<std::string> FPP_MEDIA_EXT = {
    "mp3", "ogg", "m4a", "m4p", "wav", "au", "wma", "flac", "aac",
    "MP3", "OGG", "M4A", "M4P", "WAV", "AU", "WMA", "FLAC", "AAC",
    "mp4", "MP4", "avi", "AVI", "mov", "MOV", "mkv", "MKV",
    "mpg", "MPG", "mpeg", "MPEG"
};
static std::set<std::string> FPP_VIDEO_EXT = {
    "mp4", "MP4", "avi", "AVI", "mov", "MOV", "mkv", "MKV",
    "mpg", "MPG", "mpeg", "MPEG"
};

struct FPPDInfo {
    std::string hostname;
    std::string ip;
    std::string uuid;

    bool operator<(const FPPDInfo& other) const {
        // Custom comparison operator for uniqueness and sorting
        return std::tie(hostname, ip, uuid) <
               std::tie(other.hostname, other.ip, other.uuid);
    }
};
std::set<FPPDInfo> fppDiscInfo;

FPP::FPP(const std::string& ad) : BaseController(ad, ""), majorVersion(0), minorVersion(0), patchVersion(0), outputFile(nullptr), parent(nullptr), ipAddress(ad), fppType(FPP_TYPE::FPP) {
        
    if (ip_utils::IsValidHostname(ipAddress)) {
        hostName = ipAddress;
        _ip = ip_utils::ResolveIP(ipAddress);
    }
    _connected = true; // well not really but i need to fake it
}


FPP::FPP(const std::string& ip_, const std::string& proxy_, const std::string& model_) :
    BaseController(ip_, proxy_), majorVersion(0), minorVersion(0), patchVersion(0), outputFile(nullptr), parent(nullptr),
    fppType(FPP_TYPE::FPP), pixelControllerType(model_)
{
    ipAddress = ip_;
    if (ip_utils::IsValidHostname(ipAddress)) {
        hostName = ipAddress;
        _ip = ip_utils::ResolveIP(ipAddress);
    }
    _connected = true; // well not really but i need to fake it
}

FPP::FPP(const FPP &c)
    : majorVersion(c.majorVersion), minorVersion(c.minorVersion), patchVersion(c.patchVersion), outputFile(nullptr), parent(nullptr), hostName(c.hostName), description(c.description), ipAddress(c.ipAddress), fullVersion(c.fullVersion), platform(c.platform),
    model(c.model), ranges(c.ranges), mode(c.mode), pixelControllerType(c.pixelControllerType), username(c.username), password(c.password), 
    fppType(c.fppType), capeInfo(c.capeInfo) {

}

FPP::~FPP() {
    if (outputFile && !outputFileIsOriginal) {
        delete outputFile;
        outputFile = nullptr;
    }
    if (tempFileName != "") {
        ::wxRemoveFile(tempFileName);
        tempFileName = "";
    }
}

struct FPPWriteData {
    FPPWriteData() : file(nullptr), instance(nullptr), data(nullptr), dataSize(0), curPos(0),
        postData(nullptr), postDataSize(0), totalWritten(0), cancelled(false), lastDone(0) {}

    wxFile realFile;
    wxMemoryBuffer memBuffPost;
    wxMemoryBuffer memBuffPre;

    uint8_t *data;
    size_t dataSize;
    size_t curPos;

    wxFile *file;

    uint8_t *postData;
    size_t postDataSize;

    FPP *instance;
    size_t totalWritten;
    size_t lastDone;
    bool cancelled;

    size_t readData(void *ptr, size_t buffer_size) {
        if (data != nullptr) {
            size_t remaining = dataSize - curPos;
            if (remaining) {
                size_t copy_this_much = remaining;
                if (copy_this_much > buffer_size) {
                    copy_this_much = buffer_size;
                }
                if (copy_this_much > 8*1024*1024) {
                    copy_this_much = 8*1024*1024;
                }
                memcpy(ptr, &data[curPos], copy_this_much);
                curPos += copy_this_much;
                return copy_this_much; /* we copied this many bytes */
            } else {
                //done reading from the memory data
                curPos = 0;
                if (file == nullptr) {
                    data = postData;
                    dataSize = postDataSize;
                } else {
                    data = nullptr;
                    dataSize = 0;
                }
            }
        }
        if (file != nullptr) {
            size_t t = file->Read(ptr, buffer_size);
            if (t == wxInvalidOffset) {
                return 0;
            }
            totalWritten += t;

            if (instance) {
                size_t donePct = totalWritten;
                donePct *= 1000;
                donePct /= file->Length();
                if (donePct != lastDone) {
                    lastDone = donePct;
                    cancelled = instance->updateProgress(donePct, false);
                }
            }
            if (file->Eof()) {
                curPos = 0;
                data = postData;
                dataSize = postDataSize;
                file = nullptr;
            }
            if (cancelled) {
                return CURL_READFUNC_ABORT;
            }
            return t;
        }
        return 0;
    }
};


static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp) {
    size_t buffer_size = size*nmemb;
    struct FPPWriteData *dt = (struct FPPWriteData*)userp;
    return dt->readData(ptr, buffer_size);
}

static size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {

    if (data == nullptr) return 0;
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}
CURL *FPP::setupCurl(const std::string &url, bool isGet, int timeout) {
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlInputBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, defaultConnectTimeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);
    curl_easy_setopt(curl, CURLOPT_TCP_FASTOPEN, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

    // seems to be a bug in SOME versions of libcurl where GET requests over
    // HTTP2 are not handling the spaces (%20) correctly. Most escaped character
    // are OK, but not all.   Only seems to effect GET, POST/PATCH are fine.
    // We'll drop to HTTP1 for GET's with URL's that have % in them
    if (!isGet || url.find("%") == std::string::npos) {
        //printf("HTTP2: %s\n", url.c_str());
        //curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
        //curl_easy_setopt(curl, CURLOPT_PIPEWAIT, 1);
    } else {
        //printf("HTTP1: %s\n", url.c_str());
        //curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    }
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    return curl;
}

bool FPP::GetURLAsString(const std::string& url, std::string& val, bool recordError) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));

    std::string fullUrl = (ip_utils::IsIPv6(ipAddress) ? "[" + ipAddress + "]" : ipAddress) + url;
    std::string ipAddForGet = ipAddress;
    if (fppType == FPP_TYPE::ESPIXELSTICK) {
        fullUrl = ipAddress + "/fpp?path=" + url;
    }
    if (!_fppProxy.empty()) {
        fullUrl = "http://" +  (ip_utils::IsIPv6(_fppProxy) ? "[" + _fppProxy + "]" : _fppProxy) + "/proxy/" + fullUrl;
        ipAddForGet = _fppProxy;
    } else {
        fullUrl = "http://" + fullUrl;
    }
    
    if (username != "") {
        CurlManager::INSTANCE.setHostUsernamePassword(ipAddForGet, username, password);
    }
    int response_code = 0;
    val = CurlManager::INSTANCE.doGet(fullUrl, response_code);

    logger_curl.debug("RESPONSE START --------- RC: %d ----", response_code);
    logger_curl.debug(val.c_str());
    logger_curl.debug("RESPONSE END ---------");
    if (response_code == 401) {
        if (password.empty() && xlPasswordEntryDialog::GetStoredPasswordForService(ipAddress, username, password)) {
            if (!password.empty()) {
                return GetURLAsString(url, val);
            }
        }
        xlPasswordEntryDialog dlg(nullptr, "Password needed to connect to " + ipAddress, "Password Required");
        int rc = dlg.ShowModal();
        if (rc == wxID_CANCEL) {
            return false;
        }
        username = "admin";
        password = dlg.GetValue().ToStdString();
        if (dlg.shouldSavePassword()) {
            xlPasswordEntryDialog::StorePasswordForService(ipAddress, username, password);
        }
        return GetURLAsString(url, val);
    }
    if (response_code != 200) {
        if (recordError) {
            messages.push_back("ERROR - Error on GET \"" + fullUrl + "\"    Response Code: " + std::to_string(response_code));
        }
        logger_base.info("FPPConnect GET %s  - Return: RC: %d  - %s", fullUrl.c_str(), response_code, val.c_str());
    } else {
        logger_base.info("FPPConnect GET %s  - Return: RC: %d", fullUrl.c_str(), response_code);
    }
    return response_code == 200;
}
int FPP::PostToURL(const std::string& url, const std::vector<uint8_t>& val, const std::string& contentType) const {
    return TransferToURL(url, val, contentType, true);
}
int FPP::PutToURL(const std::string& url, const std::vector<uint8_t>& val, const std::string& contentType) const {
    return TransferToURL(url, val, contentType, false);
}
int FPP::TransferToURL(const std::string& url, const std::vector<uint8_t>& val, const std::string& contentType, bool isPost) const {

    std::string fullUrl = (ip_utils::IsIPv6(ipAddress) ? "[" + ipAddress + "]" : ipAddress) + url;
    std::string ipAddForGet = ipAddress;
    if (fppType == FPP_TYPE::ESPIXELSTICK) {
        fullUrl = ipAddress + "/fpp?path=" +  url;
    }
    if (!_fppProxy.empty()) {
        fullUrl = "http://" +  (ip_utils::IsIPv6(_fppProxy) ? "[" + _fppProxy + "]" : _fppProxy) + "/proxy/" + fullUrl;
        ipAddForGet = _fppProxy;
    } else {
        fullUrl = "http://" + fullUrl;
    }
    
    if (username != "") {
        CurlManager::INSTANCE.setHostUsernamePassword(ipAddForGet, username, password);
    }
    int response_code = 0;
    if (isPost) {
        CurlManager::INSTANCE.doPost(fullUrl, contentType, val, response_code);
    } else {
        CurlManager::INSTANCE.doPut(fullUrl, contentType, val, response_code);
    }
    return response_code;
}

bool FPP::GetURLAsJSON(const std::string& url, nlohmann::json& val, bool recordError) {
    std::string sval;
    if (GetURLAsString(url, sval, recordError)) {
        try {
            val = nlohmann::json::parse(sval, nullptr, false);
            if (!val.is_discarded()) {
                return true;
            }
        } catch (...) {
        }
    }
    return false;
}

std::map<int, int> FPP::GetExpansionPorts(ControllerCaps* caps) const
{
    std::map<int, int> res;

    int const ports = wxAtoi(caps->GetCustomPropertyByPath("fpp", "0"));

    for (int i = 1; i <= ports; i++)
    {
        auto s = caps->GetCustomPropertyByPath(ToUTF8(wxString::Format("fpp%d", i)), "0,0");
        if (s != "0,0")
        {
            auto ss = Split(s, ',');
            if (ss.size() == 2)
            {
                res[std::stoi(ss[0])] = std::stoi(ss[1]);
            }
        }
    }

    return res;
}

bool FPP::AuthenticateAndUpdateVersions() {
    if (!sysInfoLoaded) {
        std::string conf;
        if (GetURLAsString("/config.php", conf)) {
            parseConfig(conf);
            nlohmann::json val;
            if (GetURLAsJSON("/api/system/info", val)) {
                sysInfoLoaded = true;
                return fppType == FPP_TYPE::FPP && parseSysInfo(val);
            }
        }
        return false;
    }
    return fppType == FPP_TYPE::FPP;
}

static std::string GetJSONStringValue(const nlohmann::json& val, const std::string &key, const std::string &def = "") {
    if (val.contains(key) && val[key].is_string()) {
        return val[key].get<std::string>();
    }
    return def;
}
static int GetJSONIntValue(const nlohmann::json& val, const std::string &key, int def = 0) {
    if (val.contains(key) && val[key].is_number_integer()) {
        return val[key].get<int>();
    }
    return def;
}
static uint32_t GetJSONUInt32Value(const nlohmann::json& val, const std::string &key, uint32_t def = 0) {
    if (val.contains(key) && val[key].is_number_integer()) {
        return val[key].get<uint32_t>();
    }
    return def;
}
static uint64_t GetJSONUInt64Value(const nlohmann::json& val, const std::string &key, uint64_t def = 0) {
    if (val.contains(key) && val[key].is_number_integer()) {
        return val[key].get<uint64_t>();
    }
    return def;
}
static bool GetJSONBoolValue(const nlohmann::json& val, const std::string &key, bool def = false) {
    if (val.contains(key) && val[key].is_boolean()) {
        return val[key].get<bool>();
    }
    return def;
}
static bool GetJSONDoubleValue(const nlohmann::json& val, const std::string &key, double def = 0.0) {
    if (val.contains(key) && val[key].is_number()) {
        return val[key].get<double>();
    }
    return def;
}
bool FPP::parseSysInfo(nlohmann::json& val) {
    platform = GetJSONStringValue(val, "Platform");
    model = GetJSONStringValue(val, "Variant");
    fullVersion = GetJSONStringValue(val, "Version");
    hostName = GetJSONStringValue(val, "HostName");
    description = GetJSONStringValue(val, "HostDescription");
    mode = GetJSONStringValue(val, "Mode");
    if (mode == "player" && GetJSONBoolValue(val, "multisync")) {
        mode += " w/multisync";
    }

    if (fullVersion != "") {
        majorVersion = wxAtoi(fullVersion);
        if (fullVersion[2] == 'x') {
            minorVersion = wxAtoi(fullVersion.substr(4)) + 1000;
        } else {
            minorVersion = wxAtoi(fullVersion.substr(2));
        }
        if (fullVersion.size() > 3 && (fullVersion[3] == '-' || fullVersion[3] == '.')) {
            patchVersion = wxAtoi(fullVersion.substr(4));
        }
    }
    if (val.contains("channelRanges")) {
        std::string r = GetJSONStringValue(val, "channelRanges");
        if (r.size() > ranges.size()) {
            ranges = r;
        }
    }
    if (val.contains("minorVersion")) {
        minorVersion = GetJSONIntValue(val, "minorVersion");
    }
    if (val.contains("majorVersion")) {
        majorVersion = GetJSONIntValue(val, "majorVersion");
    }
    return true;
}

bool FPP::IsDDPInputEnabled() {
    nlohmann::json origRoot;
    if (GetURLAsJSON("/api/configfile/ci-universes.json", origRoot, false)) {
        if (origRoot.contains("channelInputs") && origRoot.at("channelInputs").size() > 0
            && GetJSONIntValue(origRoot.at("channelInputs").at(0), "enabled", 0) == 1) {
            return true;
        }
    }
    return false;
}

void FPP::probePixelControllerType() {
    std::string file = "co-pixelStrings";
    if (platform.find("Beagle") != std::string::npos) {
        file = "co-bbbStrings";
    }
    nlohmann::json val;
    if (GetURLAsJSON("/api/channel/output/" + file, val)) {
        parseControllerType(val);
    }
}
void FPP::parseProxies(nlohmann::json& val) {
    for (int x = 0; x < val.size(); x++) {
        proxies.emplace(val[x].get<std::string>());
    }
}
void FPP::parseControllerType(nlohmann::json& val) {
    for (int x = 0; x < val["channelOutputs"].size(); x++) {
        if (GetJSONIntValue(val["channelOutputs"][x], "enabled")) {
            std::string outputType = GetJSONStringValue(val["channelOutputs"][x], "type");
            if (outputType == "RPIWS281X"||
                outputType == "BBB48String" ||
                outputType == "BBShiftString" ||
                outputType == "DPIPixels") {
                pixelControllerType = GetJSONStringValue(val["channelOutputs"][x], "subType");
            } else if (outputType == "LEDPanelMatrix") {
                pixelControllerType = LEDPANELS;
                int pw = GetJSONIntValue(val["channelOutputs"][x], "panelWidth");
                int ph = GetJSONIntValue(val["channelOutputs"][x], "panelHeight");
                int nw = 0; int nh = 0;
                bool tall = false;
                for (int p = 0; p < val["channelOutputs"][x]["panels"].size(); ++p) {
                    int r = GetJSONIntValue(val["channelOutputs"][x]["panels"][p], "row");
                    int c = GetJSONIntValue(val["channelOutputs"][x]["panels"][p], "col");
                    nw = std::max(c, nw);
                    nh = std::max(r, nh);
                    std::string orientation = GetJSONStringValue(val["channelOutputs"][x]["panels"][p], "orientation");
                    if (orientation == "E" || orientation == "W") {
                        tall = true;
                    }
                }
                nw++; nh++;
                if (tall) {
                    std::swap(pw, ph);
                }
                panelSize = std::to_string(pw * nw);
                panelSize.append("x");
                panelSize.append(std::to_string(ph * nh));
            } else if (outputType == "VirtualMatrix") {
                pixelControllerType = "Virtual Matrix";
            }
        }
    }
}

static std::string trimfront(const std::string &s) {
    int x = 0;
    while (x < s.length() && std::isspace(s[x])) {
        x++;
    }
    return s.substr(x);
}

void FPP::parseConfig(const std::string& v) {
    std::stringstream ss(v);
    std::string to;

    std::map<std::string, std::string> settings;
    while(std::getline(ss, to, '\n')) {
        to = trimfront(to);
        if (to.substr(0, 8) == "settings") {
            to = to.substr(10);
            int i = to.find("'");
            std::string key = to.substr(0, i);
            to = to.substr(to.find("\"") + 1);
            to = to.substr(0, to.find(";") - 1);
            settings[key] = to;
        }
    }

    if (settings["Title"].find("Falcon Player") != std::string::npos) {
        fppType = FPP_TYPE::FPP;
    }
}

bool FPP::IsVersionAtLeast(uint32_t maj, uint32_t min, uint32_t patch) const{
    if (majorVersion < maj) {
        return false;
    }
    if (majorVersion > maj) {
        return true;
    }
    if (minorVersion < min) {
        return false;
    }
    if (minorVersion > min) {
        return true;
    }
    return patchVersion >= patch;
}

static std::string URLEncode(const wxString &value)
{
    wxString ret = wxT("");
    unsigned int nPos = 0;

    while (value.length() > nPos) {
        wxChar cChar = value.GetChar(nPos);

        if( ( isalpha( cChar )) || ( isdigit( cChar )) || (cChar == wxT('-')) || (cChar == wxT('@'))
           || (cChar == wxT('*')) || (cChar == wxT('_')) ) {
            ret.Append( cChar );
        } else {
            switch( cChar ) {
                case wxT(' '):  ret.Append("%20"); break;
                case wxT('\n'): ret.Append("%0D%0A"); break;
                case wxT('.'):  ret.Append('.'); break;
                case wxT('\"'):  ret.Append("%22"); break;
                default: {
                    ret.Append("%");
                    if (cChar < 16) {
                        ret.Append(wxString::Format("0%x", cChar));
                    } else {
                        ret.Append(wxString::Format("%x", cChar));
                    }
                }
            }
        }
        nPos++;
    }
    return ToUTF8(ret);
}
static std::string URLEncode(const std::string &value) {
    return ToStdString(URLEncode(ToWXString(value)));
}

static inline void addString(std::vector<uint8_t> &buffer, const char *str) {
    size_t sz = strlen(str);
    size_t pos = buffer.size();
    buffer.resize(pos + sz);
    memcpy(&buffer[pos], str, sz);
}
static inline void addString(std::vector<uint8_t> &buffer, const std::string &str) {
    size_t sz = str.length();
    size_t pos = buffer.size();
    buffer.resize(pos + sz);
    memcpy(&buffer[pos], str.c_str(), sz);
}

int FPP::PostJSONToURL(const std::string& url, const nlohmann::json& val) {
    std::string const str = val.dump(3);
    std::vector<uint8_t> memBuffPost;
    addString(memBuffPost, str);
    return PostToURL(url, memBuffPost, "application/json");
}
int FPP::PostJSONToURLAsFormData(const std::string& url, const std::string& extra, const nlohmann::json& val) {
    std::vector<uint8_t> memBuffPost;
    addString(memBuffPost, extra);
    addString(memBuffPost, "&data={");
    std::string const str = val.dump(3);
    addString(memBuffPost, str);
    addString(memBuffPost, "}");
    return PostToURL(url, memBuffPost, "application/x-www-form-urlencoded; charset=UTF-8");
}

void FPP::DumpJSON(const nlohmann::json& json) const {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::string const str = json.dump(3);
    logger_base.debug(str);
}

int FPP::PostToURL(const std::string& url, const std::string& val, const std::string& contentType) const {
    std::vector<uint8_t> memBuffPost;
    addString(memBuffPost, val);
    return PostToURL(url, memBuffPost, contentType);
}
int FPP::PutToURL(const std::string& url, const std::string& val, const std::string& contentType) const {
    std::vector<uint8_t> memBuffPost;
    addString(memBuffPost, val);
    return PutToURL(url, memBuffPost, contentType);
}

bool FPP::updateProgress(int val, bool yield) {
    if (progress != nullptr) {
        progress->SetValue(val);
        if (yield) {
            wxYield();
        }
    }
    if (progressDialog) {
        return progressDialog->isCancelled();
    }
    return false;
}


bool FPP::uploadFile(const std::string &utfFilename, const std::string &file) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxString filename = ToWXString(utfFilename);
    wxString fn;
    wxString ext;

    for (int a = 0; a < utfFilename.length(); a++) {
        wxChar ch = utfFilename[a];
        if (ch == '"') {
            fn.Append("\\\"");
        } else {
            fn.Append(ch);
        }
    }

    updateProgress(0, true);
    int lastDone = 0;

    std::string ct = "Content-Type: application/octet-stream";
    bool deleteFile = false;
    std::string fullFileName = file;

    curlInputBuffer.clear();
    char error[1024];


    std::string fullUrl;
    bool usingMove = true;
    if (fppType == FPP_TYPE::ESPIXELSTICK) {
        if (this->canZipUpload) {
            auto from = fullFileName;
            wxDebugReportCompress report;
            report.AddFile(fullFileName, wxFileName(from).GetFullName());
            report.Process();
            from = report.GetCompressedFileName();
            wxRenameFile(from, fullFileName);
            wxFileName to = filename;
            to.SetExt("xlz");
            fullUrl = ipAddress + "/fpp?path=uploadFile&filename=" + URLEncode(to.GetFullPath());
        }
        else {
			fullUrl = ipAddress + "/fpp?path=uploadFile&filename=" + URLEncode(filename);
        }
        usingMove = false;
    } else {
        fullUrl = ipAddress + "/api/file/uploads/" + URLEncode(filename);
    }
    if (!_fppProxy.empty()) {
        fullUrl = "http://" + _fppProxy + "/proxy/" + fullUrl;
    } else {
        fullUrl = "http://" + fullUrl;
    }
    //if we cannot upload it in 5 minutes, we have serious issues
    CURL *curl = setupCurl(fullUrl, false, 5*60*1000);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error);
    if (username != "") {
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC | CURLAUTH_DIGEST | CURLAUTH_NEGOTIATE);
    }
    struct curl_slist *chunk = nullptr;
    std::string ctMime = "Content-Type: application/octet-stream";
    chunk = curl_slist_append(chunk, ctMime.c_str());
    chunk = curl_slist_append(chunk, "X-Requested-With: FPPConnect");
    chunk = curl_slist_append(chunk, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    FPPWriteData *data = new FPPWriteData();
    data->realFile.Open(fullFileName);
    std::string cl = "Content-Length: " + std::to_string(data->realFile.Length());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data->realFile.Length());
    chunk = curl_slist_append(chunk, cl.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    data->realFile.Seek(0);
    data->data = (uint8_t*)data->memBuffPre.GetData();
    data->dataSize = data->memBuffPre.GetDataLen();
    data->instance = this;
    data->file = &data->realFile;
    data->postData =  (uint8_t*)data->memBuffPost.GetData();
    data->postDataSize = data->memBuffPost.GetDataLen();
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, data);

    data->lastDone = lastDone;

    
    CurlManager::INSTANCE.addCURL(fullUrl, curl, [this, chunk, deleteFile, fullFileName, usingMove, filename, ext, utfFilename, data] (CURL *curl) {
        long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        if (response_code == 200) {
            if (usingMove) {
                if (!callMoveFile(ToUTF8(filename + ext))) {
                    logger_base.warn("Error trying to rename file.");
                } else {
                    logger_base.debug("Renaming done.");
                }
            }
            logger_base.debug(utfFilename + " upload complete to " + this->hostName + " (" + this->ipAddress + "). Bytes sent:" + std::to_string(data->totalWritten) + ".");
        } else {
            messages.push_back("ERROR Uploading file: " + utfFilename + "     Response Code: " + std::to_string(response_code));
            faileduploads.push_back(filename);
            logger_base.warn("Did not get 200 response code:  %d", response_code);
        }
        
        delete data;
        curl_slist_free_all(chunk);
        if (deleteFile) {
            wxRemoveFile(fullFileName);
        }
        updateProgress(1000, false);
    }, true);

    return false;
}

bool FPP::callMoveFile(const std::string &filename) {
    std::string val;
    return GetURLAsString("/api/file/move/" + URLEncode(filename), val);
}

class V7ProgressStruct {
public:
    wxFile in;
    FPP *instance;
    size_t length;

    size_t offset = 0;
    int lastPct = 0;
    int errorCount = 0;
    
    std::string fullUrl;
    std::string fileSizeHeader;
    std::string fileNameHeader;
    std::string filename;
};
int progress_callback(void *clientp,
                      curl_off_t dltotal,
                      curl_off_t dlnow,
                      curl_off_t ultotal,
                      curl_off_t ulnow) {
    V7ProgressStruct *p = (V7ProgressStruct*)clientp;
    if (p->instance) {
        size_t start = p->offset;
        start += ulnow;
        start *= 1000;
        start /= p->length;
        if (p->lastPct != start) {
            p->instance->updateProgress(p->lastPct, false);
            p->lastPct = start;
        }
    }
    return 0;
}


void prepareCurlForMulti(V7ProgressStruct *ps) {
    static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    constexpr uint64_t BLOCK_SIZE = 16*1024*1024;
    CurlManager::CurlPrivateData *cpd = nullptr;
    CURL *curl = CurlManager::INSTANCE.createCurl(ps->fullUrl, &cpd, true);

    //if we cannot upload a single chunk in 3 minutes, we have serious issues
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000*3*60);

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/offset+octet-stream");
    headers = curl_slist_append(headers, "X-Requested-With: FPPConnect");
    headers = curl_slist_append(headers, "Expect:");
    headers = curl_slist_append(headers, "Connection: keep-alive");
    
    std::string offsetHeader = "Upload-Offset: " + std::to_string(ps->offset);
    headers = curl_slist_append(headers, offsetHeader.c_str());
    headers = curl_slist_append(headers, ps->fileSizeHeader.c_str());
    headers = curl_slist_append(headers, ps->fileNameHeader.c_str());
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36");

    uint64_t remaining = ps->length - ps->offset;
    if (remaining > BLOCK_SIZE) {
        remaining = BLOCK_SIZE;
    }
    cpd->req->resize(remaining);
    uint64_t read = ps->in.Read(cpd->req->data(), remaining);
    if (read != remaining) {
        logger_curl.info("ERROR Uploading file: " + ps->filename + "     Could not read source file.");
        ps->instance->messages.push_back("ERROR Uploading file: " + ps->filename + "     Could not read source file.");
        ps->instance->faileduploads.push_back(ps->filename);
    }
    std::string contentSizeHeader = "Content-Length: " + std::to_string(remaining);
    headers = curl_slist_append(headers, contentSizeHeader.c_str());
    
    curl_easy_setopt(curl, CURLOPT_UPLOAD, (long)1);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, ps);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, (long)0);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)remaining);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cpd->req->data());
    
    logger_curl.info("FPPConnect Adding CURL - URL: %s    Method: PATCH    Start: %zd   Length: %zd   Total: %zd", ps->fullUrl.c_str(), ps->offset, remaining, ps->length);
    
    CurlManager::INSTANCE.addCURL(ps->fullUrl, curl, [headers, remaining, ps] (CURL *c) {

        curl_slist_free_all(headers);
        long response_code = 0;
        curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &response_code);
        logger_curl.info("    FPPConnect CURL Callback - URL: %s    Response: %d", ps->fullUrl.c_str(), response_code);
        bool cancelled = false;
        if (response_code != 200 && ps->errorCount < 3) {
            // strange error on upload, let's restart and try again (up to three attempts)
            ps->offset = 0;
            ps->in.Seek(0);
            ++ps->errorCount;
        } else if (response_code != 200) {
            ps->instance->messages.push_back("ERROR Uploading file: " + ps->filename + ". Response code: " + std::to_string(response_code));
            ps->instance->faileduploads.push_back(ps->filename);
            cancelled = true;
        } else {
            ps->offset += remaining;
        }
        uint64_t pct = (ps->offset * 1000) / ps->length;
        cancelled |= ps->instance->updateProgress(pct, false);
        if (cancelled || ps->offset >= ps->length) {
            logger_base.debug(ps->filename + " upload complete to " + ps->instance->hostName + " (" + ps->instance->ipAddress + "). Bytes sent:" + std::to_string(ps->length) + ".");
            delete ps;
        } else {
            prepareCurlForMulti(ps);
        }
    });
}


bool FPP::uploadFileV7(const std::string &filename,
                       const std::string &file,
                       const std::string &dir) {
    bool cancelled = false;

    V7ProgressStruct *ps = new V7ProgressStruct();
    ps->in.Open(ToWXString(file));
    if (ps->in.IsOpened()) {
        ps->filename = filename;
        
        ps->length = ps->in.Length();
        ps->offset = 0;
        ps->fullUrl = ipAddress + "/api/file/" + dir;
        if (!_fppProxy.empty()) {
            ps->fullUrl = "http://" + _fppProxy + "/proxy/" + ps->fullUrl;
        } else {
            ps->fullUrl = "http://" + ps->fullUrl;
        }
        ps->fileSizeHeader = "Upload-Length: " + std::to_string(ps->length);
        ps->fileNameHeader = "Upload-Name: " + filename;
        ps->instance = this;
        if (progressDialog != nullptr) {
            cancelled |= updateProgress(0, true);
        }
        prepareCurlForMulti(ps);
    } else {
        delete ps;
    }
    return cancelled;
}

bool FPP::uploadOrCopyFile(const std::string &filename,
                           const std::string &file,
                           const std::string &dir) {
    if (fppType == FPP_TYPE::FPP) {
        return uploadFileV7(filename, file, dir);
    }
    return uploadFile(filename, file);
}

#ifndef DISCOVERYONLY
// types
// 0 - V1
// 1 - V2 zstd
// 2 - V2 sparse zstd
// 3 - V2 sparse uncompressed
// 4 - V2 uncompressed
// 5 - V2 zlib
// 6 - V2 sparse zlib


static void FindHostSpecificMedia(const std::string &hostName, std::string &mediaBaseName, std::string &mediaFile, wxFileName &mfn) {
    wxFileName mfn2(FromUTF8(mediaFile));
    mfn2.SetName(mfn2.GetName() + "-" + FromUTF8(hostName));
    //first, check filename-hostname with same extension
    if (mfn2.Exists()) {
        mediaFile = ToUTF8(mfn2.GetFullPath());
        mediaBaseName =  ToUTF8(mfn2.GetFullName());
        mfn = mfn2;
        return;
    }
    //next, check "filename-hostname" with all the extensions
    for (auto &a : FPP_MEDIA_EXT) {
        mfn2.SetExt(a);
        if (mfn2.Exists()) {
            mediaFile = ToUTF8(mfn2.GetFullPath());
            mediaBaseName =  ToUTF8(mfn2.GetFullName());
            mfn = mfn2;
            return;
        }
    }
    //did not find, check for a directory with the hostname
    wxFileName mfn3(FromUTF8(mediaFile));
    mfn3.AppendDir(FromUTF8(hostName));
    mfn2 = mfn3;
    mfn2.SetName(mfn2.GetName() + "-" + FromUTF8(hostName));
    if (wxFileName::DirExists(mfn3.GetPath())) {
        //file of same name, but in new directory
        if (mfn3.Exists()) {
            mediaFile = ToUTF8(mfn3.GetFullPath());
            mediaBaseName =  ToUTF8(mfn3.GetFullName());
            mfn = mfn3;
            return;
        }
        if (mfn2.Exists()) {
            mediaFile = ToUTF8(mfn2.GetFullPath());
            mediaBaseName =  ToUTF8(mfn2.GetFullName());
            mfn = mfn2;
            return;
        }
        for (auto &a : FPP_MEDIA_EXT) {
            mfn2.SetExt(a);
            if (mfn2.Exists()) {
                mediaFile = ToUTF8(mfn2.GetFullPath());
                mediaBaseName =  ToUTF8(mfn2.GetFullName());
                mfn = mfn2;
                return;
            }
            mfn3.SetExt(a);
            if (mfn3.Exists()) {
                mediaFile = ToUTF8(mfn3.GetFullPath());
                mediaBaseName =  ToUTF8(mfn3.GetFullName());
                mfn = mfn3;
                return;
            }
        }
    }
}
bool FPP::CheckUploadMedia(const std::string &media, std::string &mediaBaseName) {
    bool cancelled = false;
    wxFileName mfn(FromUTF8(media));
    std::string mediaFile = media;
    mediaBaseName = ToUTF8(mfn.GetFullName());

    if (majorVersion >= 6) {
        FindHostSpecificMedia(hostName, mediaBaseName, mediaFile, mfn);
    }
    
    std::string url = "/api/media/" + URLEncode(mediaBaseName) + "/meta";
    std::string fullUrl = (ip_utils::IsIPv6(ipAddress) ? "[" + ipAddress + "]" : ipAddress) + url;
    std::string ipAddForGet = ipAddress;
    if (!_fppProxy.empty()) {
        fullUrl = "http://" + _fppProxy + "/proxy/" + fullUrl;
        ipAddForGet = _fppProxy;
    } else {
        fullUrl = "http://" + fullUrl;
    }
    if (username != "") {
        CurlManager::INSTANCE.setHostUsernamePassword(ipAddForGet, username, password);
    }
    CurlManager::INSTANCE.addGet(fullUrl, [this, mfn, mediaBaseName, mediaFile](int rc, const std::string &resp) {
        bool doMediaUpload = true;
        if (rc == 200) {
            try {
                nlohmann::json currentMeta = nlohmann::json::parse(resp, nullptr, false);
                if (currentMeta.contains("format") && currentMeta["format"].contains("size") &&
                    (mfn.GetSize() == GetJSONIntValue(currentMeta["format"], "size"))) {
                    doMediaUpload = false;
                }
            } catch (...) {
            }
        }
        if (doMediaUpload) {
            std::string dir = "music";
            for (auto &a : FPP_VIDEO_EXT) {
                if (mfn.GetExt() == a) {
                    dir = "videos";
                }
            }
            uploadOrCopyFile(mediaBaseName, mediaFile, dir);
        }
    });
        
    return cancelled;
}

bool FPP::PrepareUploadSequence(FSEQFile *file,
                                const std::string &seq,
                                const std::string &media,
                                int type) {
    if (outputFile && !outputFileIsOriginal) {
        delete outputFile;
    }
    outputFile = nullptr;
    if (tempFileName != "") {
        ::wxRemoveFile(tempFileName);
        tempFileName = "";
    }

    updateProgress(0, true);
    wxFileName fn(FromUTF8(seq));
    std::string baseName = ToUTF8(fn.GetFullName());
    std::string mediaBaseName = "";
    bool cancelled = false;
    if (media != "" && fppType == FPP_TYPE::FPP) {
        cancelled = CheckUploadMedia(media, mediaBaseName);
        if (cancelled) {
            return true;
        }
    }
    sequences[baseName].sequence = baseName;
    sequences[baseName].media = mediaBaseName;
    sequences[baseName].duration = ((float)(file->getStepTime() * file->getNumFrames())) / 1000.0f;

    tempFileName = ToStdString(wxFileName::CreateTempFileName(ToWXString(baseName)));
    TempFileManager::GetTempFileManager().AddTempFile(tempFileName);
    std::string fileName = tempFileName;

    FSEQFile::CompressionType ctype = ::FSEQFile::CompressionType::zstd;
    if (type == 3 || type == 4) {
        ctype = ::FSEQFile::CompressionType::none;
    } else if (type == 5 || type == 6) {
        ctype = ::FSEQFile::CompressionType::zlib;
    }

    bool doSeqUpload = true;
    uint32_t currentMaxChannel = 0;
    uint32_t currentChannelCount = 0;
    std::vector<std::pair<uint32_t, uint32_t>> currentRanges;
    std::vector<std::pair<uint32_t, uint32_t>> newRanges;
    if (fppType == FPP_TYPE::FPP) {
        nlohmann::json currentMeta;
        if (GetURLAsJSON("/api/sequence/" + URLEncode(baseName) + "/meta", currentMeta, false)) {
            doSeqUpload = false;
            char buf[24];
            snprintf(buf, sizeof(buf), "%" PRIu64, file->getUniqueId());
            std::string version = GetJSONStringValue(currentMeta, "Version");
            if (type == 0 && version[0] != '1') doSeqUpload = true;
            if (type != 0 && version[0] == '1') doSeqUpload = true;
            int currentCompression = 1;
            if (version[0] == '1') {
                currentCompression = 0;
            }
            if (currentMeta.contains("CompressionType")) {
                currentCompression = GetJSONIntValue(currentMeta, "CompressionType");
            }
            if ((type == 2 || type == 1) && currentCompression != 1) {
                doSeqUpload = true;
            }
            if ((type == 0 || type == 3) && currentCompression != 0) {
                doSeqUpload = true;
            }
            if (GetJSONStringValue(currentMeta, "ID") != buf) {
                doSeqUpload = true;
            }
            if (GetJSONUInt64Value(currentMeta, "NumFrames") != file->getNumFrames()) {
                doSeqUpload = true;
            }
            if (GetJSONIntValue(currentMeta, "StepTime") != file->getStepTime()) {
                doSeqUpload = true;
            }
            currentMaxChannel = GetJSONIntValue(currentMeta, "MaxChannel", currentMaxChannel);
            currentChannelCount = GetJSONIntValue(currentMeta, "ChannelCount", currentChannelCount);
            if (currentMeta.contains("Ranges")) {
                for (int x = 0; x < currentMeta["Ranges"].size(); x++) {
                    uint32_t s = GetJSONUInt32Value(currentMeta["Ranges"][x], "Start");
                    uint32_t l = GetJSONUInt32Value(currentMeta["Ranges"][x], "Length");
                    currentRanges.push_back(std::pair<uint32_t, uint32_t>(s, l));
                }
            }
        }
    }

    int channelCount = 0;
    if (type <= 1 || type == 4 || type == 5) {
        //full file, non sparse
        if (currentMaxChannel != file->getMaxChannel()) doSeqUpload = true;
        if (currentChannelCount != file->getChannelCount()) doSeqUpload = true;
        if (!currentRanges.empty()) {
            V2FSEQFile *v2File = dynamic_cast<V2FSEQFile*>(file);
            if (v2File == nullptr) {
                doSeqUpload = true;
            } else if (v2File->m_sparseRanges != currentRanges) {
                doSeqUpload = true;
            }
        }
        channelCount = file->getMaxChannel();
        // at this point, if we are uploading a full file, we know if something has changed or not
        // and can bail quickly if not
    } else if (ranges != "") {
        if (ranges != "") {
            wxArrayString r1 = wxSplit(wxString(ranges), ',');
            for (const auto& a : r1) {
                wxArrayString r = wxSplit(a, '-');
                int start = wxAtoi(r[0]);
                int len = 4; //at least 4
                if (r.size() == 2) {
                    len = wxAtoi(r[1]) - start + 1;
                }
                newRanges.push_back(std::pair<uint32_t, uint32_t>(start, len));
                channelCount += len;
            }
            if (newRanges != currentRanges) doSeqUpload = true;
        }
    } else if (!currentRanges.empty()) {
        doSeqUpload = true;
    }
    if (!doSeqUpload) {
        //nothing will change... we can bail
        return false;
    }

    baseSeqName = baseName;
    if (fppType == FPP_TYPE::FPP) {
        if ((type == 0 && file->getVersionMajor() == 1) || fn.GetExt() == "eseq") {
            //these just get uploaded directly
            outputFile = file;
            outputFileIsOriginal = true;
            tempFileName = file->getFilename();
            return false;
        }
        if (type == 1 && file->getVersionMajor() == 2) {
            // Full v2 file, upload directly
            outputFile = file;
            outputFileIsOriginal = true;
            tempFileName = file->getFilename();
            return false;
        }
    }

    int clevel = 2;
    int fastLevel = ZSTD_versionNumber() > 10305 ? -5 : 1;

    if (ctype == ::FSEQFile::CompressionType::zlib) {
        clevel = 1; // 9;
    } else {
        if (model.find(" Zero") != std::string::npos
            || model.find("Pi Model A") != std::string::npos
            || model.find("Pi Model B") != std::string::npos) {
            clevel = fastLevel;
        } else if (model.find("Beagle") != std::string::npos) {
            // lots of channels actually needed.  Possibly a P# panel or similar
            // where we'll need CPU to actually process the channels so
            // drop to lower compression, faster decommpression
            if (channelCount > 50000) {
                clevel = fastLevel;
            } else if (channelCount > 20000) {
                if (fastLevel < 0) {
                    clevel = -1;
                } else {
                    clevel = 1;
                }
            } else {
                clevel = 2;
            }
        }
    }
    outputFile = FSEQFile::createFSEQFile(fileName, type == 0 ? 1 : 2, ctype, clevel);
    outputFileIsOriginal = false;
    outputFile->initializeFromFSEQ(*file);
    if (fppType == FPP_TYPE::FPP && IsVersionAtLeast(7, 0)) {
        outputFile->enableMinorVersionFeatures(2);
    }
    if (type >= 2 && !newRanges.empty()) {
        for (auto &a : newRanges) {
            ((V2FSEQFile*)outputFile)->m_sparseRanges.push_back(a);
        }
    }
    if (fppType != FPP_TYPE::FPP || type < 2 || !IsVersionAtLeast(9, 3)) {
        // need to remove some variable headers that could trigger extra memory usage
        outputFile->removeVariableHeader('X', 'S');
        outputFile->removeVariableHeader('X', 'N');
        outputFile->removeVariableHeader('X', 'R');
    }
    outputFile->writeHeader();
    return false;
}

bool FPP::WillUploadSequence() const {
    return outputFile != nullptr;
}
bool FPP::NeedCustomSequence() const {
    return outputFile != nullptr && !outputFileIsOriginal;
}
bool FPP::AddFrameToUpload(uint32_t frame, uint8_t *data) {
    if (outputFile && !outputFileIsOriginal) {
        outputFile->addFrame(frame, data);
    }
    return false;
}

bool FPP::FinalizeUploadSequence() {
    bool cancelled = false;
    if (outputFile) {
        if (!outputFileIsOriginal) {
            outputFile->finalize();
            delete outputFile;
        }
        outputFile = nullptr;
        if (tempFileName != "" && (fppType == FPP_TYPE::FPP || fppType == FPP_TYPE::ESPIXELSTICK)) {
            std::string directory = "sequences";
            if (EndsWith(baseSeqName, ".eseq")) {
                directory = "effects";
            }
            cancelled = uploadOrCopyFile(baseSeqName, tempFileName, directory);
            if (!outputFileIsOriginal) {
                ::wxRemoveFile(tempFileName);
            }
            tempFileName = "";
            outputFileIsOriginal = false;
        }
    } else {
        updateProgress(1000, false);
    }
    return cancelled;
}

static bool PlaylistContainsEntry(nlohmann::json &pl, const std::string &media, const std::string &seq) {
    for (int x = 0; x < pl.size(); x++) {
        nlohmann::json entry = pl[x];
        if (seq == GetJSONStringValue(entry, "sequenceName")) {
            if (media.empty()) {
                if (GetJSONStringValue(entry, "type") == "sequence") {
                    return true;
                }
            } else if (GetJSONStringValue(entry, "type") == "both") {
                if (media == GetJSONStringValue(entry, "mediaName")) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool FPP::UploadPlaylist(const std::string &name) {
    nlohmann::json origJson;
    GetURLAsJSON("/api/playlist/" + URLEncode(name), origJson, false);

    if (!origJson.is_object()) {
        origJson = nlohmann::json::object();
    }

    for (const auto& info : sequences) {
        if (!PlaylistContainsEntry(origJson["mainPlaylist"], info.second.media, info.first)) {
            nlohmann::json entry;
            if (!info.second.media.empty()) {
                entry["type"] = std::string("both");
                entry["enabled"] = 1;
                entry["playOnce"] = 0;
                entry["sequenceName"] = info.first;
                entry["mediaName"] = info.second.media;
                entry["videoOut"] = std::string("--Default--");
                entry["duration"] = info.second.duration;
            } else {
                entry["type"] = std::string("sequence");
                entry["enabled"] = 1;
                entry["playOnce"] = 0;
                entry["sequenceName"] = info.first;
                entry["duration"] = info.second.duration;
            }
            origJson["mainPlaylist"].push_back(entry);
        }
    }
    origJson.erase(std::string("playlistInfo"));
    origJson["name"] = name;
    if (!origJson.contains("random")) {
        origJson["random"] = 0;
    }

    nlohmann::json playlistInfo;
    playlistInfo["total_items"] = origJson["mainPlaylist"].size();
    double total_duration = 0.0;
    for (const auto& entry : origJson["mainPlaylist"]) {
        if (entry.contains("duration")) {
            total_duration += GetJSONDoubleValue(entry, "duration");
        }
    }
    playlistInfo["total_duration"] = total_duration;
    origJson["playlistInfo"] = playlistInfo;

    PostJSONToURL("/api/playlist/" + URLEncode(name), origJson);
    return false;
}

bool FPP::UploadModels(const nlohmann::json &models) {
    PostJSONToURL("/api/models", models);
    return false;
}

bool FPP::UploadDisplayMap(const std::string &displayMap) {
    PostToURL("/api/configfile/virtualdisplaymap", displayMap);
    return false;
}

bool FPP::UploadUDPOut(const nlohmann::json &udp) {
    nlohmann::json orig;
    nlohmann::json newudp = udp;

    if (GetURLAsJSON("/api/channel/output/universeOutputs", orig)) {
        if (orig.contains("channelOutputs")) {
            for (int x = 0; x < orig["channelOutputs"].size(); x++) {
                if (GetJSONStringValue(orig["channelOutputs"][x], "type") == "universes" && orig["channelOutputs"][x].contains("interface")) {
                    newudp["channelOutputs"][0]["interface"] = GetJSONStringValue(orig["channelOutputs"][x], "interface");
                }
            }
        }
    }
    PostJSONToURL("/api/channel/output/universeOutputs", newudp);
    return false;
}

nlohmann::json FPP::CreateModelMemoryMap(ModelManager* allmodels, int32_t startChan, int32_t endChannel) {
    nlohmann::json json;
    nlohmann::json models;
    std::vector<std::string> names;
    for (const auto& m : *allmodels) {
        Model* model = m.second;

        if (model->GetDisplayAs() == "ModelGroup") {
            continue;
        }
        if (!model->IsActive()) {
            continue;
        }

        int ch = model->GetNumberFromChannelString(model->ModelStartChannel);
        if (ch < startChan || ch > endChannel) {
            continue;
        }

        std::string name(model->name);
        Replace(name, " ", "_");

        int numStr = model->GetNumStrings();
        if (numStr == 0) {
            numStr = 1;
        }
        int straPerStr =  model->GetNumStrands() / numStr;
        if (straPerStr < 1) straPerStr = 1;

        nlohmann::json jm;
        jm["Name"] = name;
        jm["ChannelCount"] = model->GetActChanCount();
        jm["StartChannel"] = ch;
        jm["ChannelCountPerNode"] = model->GetChanCountPerNode();
        jm["xLights"] = true;

        MatrixModel *mm = dynamic_cast<MatrixModel*>(model);
        if (mm) {
            if (mm->isVerticalMatrix()) {
                jm["Orientation"] = std::string("vertical");
            } else {
                jm["Orientation"] = std::string("horizontal");
            }
        } else if (model->GetDisplayAs() == "Custom") {
            CustomModel *cm = dynamic_cast<CustomModel *>(model);
            straPerStr = 1;
            numStr = 1;
            if ((cm->GetCustomWidth() * cm->GetCustomHeight() * cm->GetCustomDepth()) > (512 * 512)) {
                jm["Orientation"] = std::string("horizontal");
            } else {
                jm["Orientation"] = std::string("custom");
                jm["data"] = cm->GetCustomData();
            }
        } else {
            jm["Orientation"] = std::string("horizontal");
        }
        jm["StringCount"] = numStr;
        jm["StrandsPerString"] = straPerStr;
        std::string corner = model->GetIsBtoT() ? "B" : "T";
        corner += model->GetIsLtoR() ? "L" : "R";
        jm["StartCorner"] = corner;
        jm["Type"] = std::string("Channel");
        names.emplace_back(name);
        models.push_back(jm);
    }

    nlohmann::json ogModelJSON;
    if (GetURLAsJSON("/api/models", ogModelJSON)) {
        for (auto const& ogmodel : ogModelJSON) {
            if (!ogmodel.contains("Name")) {
                continue;
            }
            if (!ogmodel["Name"].is_string()) {
                continue;
            }
            auto ogName = GetJSONStringValue(ogmodel, "Name");
            if (GetJSONBoolValue(ogmodel, "autoCreated")) {
                continue;
            }

            if (!IsVersionAtLeast(8, 0)) {
                //I don't think this works
                if (ogmodel.contains("StartChannel") && ogmodel["StartChannel"].is_number_integer()) {
                    auto ogStartChan = ogmodel["StartChannel"].get<int32_t>();
                    if (ogStartChan < startChan || ogStartChan > endChannel ) {
                        continue;
                    }
                }
            }

            if (ogmodel.contains("xLights") && ogmodel["xLights"].is_boolean()) {
                auto isfromXlights = ogmodel["xLights"].get<bool>();
                if (isfromXlights) {
                    continue;
                }
            }
            
            if (std::find(names.cbegin(), names.cend(), ogName) != names.end()) { // only add if name doesn't exist
                continue;
            }
            models.push_back(ogmodel);
        }
    }

    json["models"] = models;
    return json;
}

static bool Compare3dPointTuple(const std::tuple<float, float, float, int> &l,
                                const std::tuple<float, float, float, int> &r) {
    return std::get<2>(l) < std::get<2>(r);
}

std::string FPP::CreateVirtualDisplayMap(ModelManager* allmodels, int previewWi, int previewHi) {
    std::string ret;

    constexpr float PADDING{ 10.0F };
    float minX{ 0.0F };
    float maxX{ 0.0F };
    float minY{ 0.0F };
    float maxY{ 0.0F };

    if (allmodels->size() == 0) {
        return ret;
    }

    for (auto m = allmodels->begin(); m != allmodels->end(); ++m) {
        Model* model = m->second;

        if (model->GetLayoutGroup() != "Default") {
            continue;
        }

        if (model->GetDisplayAs() == "ModelGroup") {
            continue;
        }
        
        minY = std::min(model->GetModelScreenLocation().GetBottom() - PADDING, minY);
        maxY = std::max(model->GetModelScreenLocation().GetTop() + PADDING, maxY);
        minX = std::min(model->GetModelScreenLocation().GetLeft() - PADDING, minX);
        maxX = std::max(model->GetModelScreenLocation().GetRight() + PADDING, maxX);
    }

    int totW = std::max(previewWi, int(maxX - minX));
    int totH = std::max(previewHi, int(maxY - minY));

    ret += "# Preview Size\n";
    ret += ToUTF8(wxString::Format("%d,%d\n", totW, totH));

    for (auto m = allmodels->begin(); m != allmodels->end(); ++m) {
        Model* model = m->second;

        if (model->GetLayoutGroup() != "Default") {
            continue;
        }

        if (model->GetDisplayAs() == "ModelGroup") {
            continue;
        }

        std::string stringType = model->GetStringType();

        if (Contains(stringType, "Nodes")) {
            stringType = BeforeFirst(stringType, ' ');
        } else if (stringType == "3 Channel RGB") {
            stringType = "RGB";
        } else if (stringType == "4 Channel RGBW") {
            stringType = "RGBW";
        } else if (stringType == "Strobes") {
            stringType = "White";
        } else if (stringType == "Single Color Red") {
            stringType = "Red";
        } else if ((stringType == "Single Color Green") || (stringType == "G")) {
            stringType = "Green";
        } else if ((stringType == "Single Color Blue") || (stringType == "B")) {
            stringType = "Blue";
        } else if ((stringType == "Single Color White") || (stringType == "W")) {
            stringType = "White";
        } else if (stringType == "Single Color Custom") {
            stringType = "White";
        } else if (stringType == "Node Single Color") {
            stringType = "White";
        }

        ret += ToUTF8(wxString::Format("# Model: '%s', %d nodes\n", model->GetName().c_str(), model->GetNodeCount()));

        std::multiset<std::tuple<float, float, float, int>,
                bool (*)(const std::tuple<float, float, float, int>& l,
                        const std::tuple<float, float, float, int>& r)>
                                modelPts(Compare3dPointTuple);

        for (size_t i = 0; i < model->GetNodeCount(); i++) {
            std::vector<std::tuple<float, float, float>> pts;
            model->GetNode3DScreenCoords(i, pts);
            int ch = model->NodeStartChannel(i);

            for (auto [x,y,z] : pts) {
                model->GetModelScreenLocation().TranslatePoint(x, y, z);
                x -= minX;
                y -= minY;
                modelPts.insert(std::make_tuple(x, y, z, ch));
            }
        }
        for (auto const&[x,y,z, ch] : modelPts) {
            ret += ToUTF8(wxString::Format("%d,%d,%d,%d,%d,%s,%d\n",
                (int)std::round(x), (int)std::round(y), (int)std::round(z), ch,
                model->GetChanCountPerNode(), stringType.c_str(), model->GetPixelSize()));
        }

    }
    return ret;
}
#endif

inline wxString stripInvalidChars(const std::string &str) {
    wxString s = str;
    s.Replace("&", "_");
    s.Replace("<", "_");
    s.Replace(">", "_");
	s.Replace("\"", "\\\"");
    return s;
}

void FPP::FillRanges(std::map<int, int> &rngs) {
    if (ranges != "") {
        auto const r1 = Split(ranges, ',');
        for (const auto& a : r1) {
            auto const r = Split(a, '-');
            int const start = std::stoi(r[0]);
            int len = 4; //at least 4
            if (r.size() == 2) {
                len = std::stoi(r[1]) - start + 1;
            }
            rngs[start] = len;
        }
    }
}
void FPP::SetNewRanges(const std::map<int, int> &rngs) {
    if (rngs.empty()) {
        ranges = "";
        return;
    }
    std::string rngList;
    int curFirst = -1;
    int curLast = -1;
    for (const auto &a : rngs) {
        int s = a.first;
        int l = a.second;
        if (curFirst == -1) {
            curFirst = s;
            curLast = s + l - 1;
        } else if (s == (curLast + 1)) {
            curLast += l;
        } else if (s < (curFirst + curLast -1)) {
            //start is within the previous's range
            int nl = s + l - 1;
            curLast = std::max(curLast, nl);
        } else {
            if (rngList != "") {
                rngList += ",";
            }
            rngList += std::to_string(curFirst) + "-" + std::to_string(curLast);
            curFirst = s;
            curLast = a.first + l - 1;
        }
    }
    if (curFirst != -1) {
        if (rngList != "") {
            rngList += ",";
        }
        rngList += std::to_string(curFirst) + "-" + std::to_string(curLast);
    }
    ranges = rngList;
}

#ifndef DISCOVERYONLY
bool FPP::UploadUDPOutputsForProxy(OutputManager* outputManager) {
    std::list<Controller*> selected;
    for (const auto& it : outputManager->GetControllers()) {
        auto c = dynamic_cast<ControllerEthernet*>(it);
        if (c != nullptr) {
            std::string proxy_ip = ip_utils::ResolveIP(c->GetFPPProxy());
            std::string ipAddress_ip = ip_utils::ResolveIP(ipAddress);
            if (
                    (::Lower(c->GetFPPProxy()) == ::Lower(ipAddress)) 
                 || (::Lower(proxy_ip) == ::Lower(ipAddress)) 
                 || (::Lower(c->GetFPPProxy()) == ::Lower(ipAddress_ip))
                 || (::Lower(proxy_ip) == ::Lower(ipAddress_ip))
                ) 
            {
                selected.push_back(c);
            }
        }
    }

    nlohmann::json f = CreateUniverseFile(selected, false);

    std::map<int, int> rng;
    FillRanges(rng);
    for (int x = 0; x < f["channelOutputs"][0]["universes"].size(); x++) {
        nlohmann::json u = f["channelOutputs"][0]["universes"][x];
        int const start = u["startChannel"].get<int>() - 1;
        int const len = u["channelCount"].get<int>();
        rng[start] = len;
    }
    SetNewRanges(rng);

    return UploadUDPOut(f);
}

nlohmann::json FPP::CreateUniverseFile(Controller* controller, bool input) {
    std::list<Controller*> selected;
    selected.push_back(controller);
    return CreateUniverseFile(selected, false);
}
#endif

std::string FPP::GetVendor(const std::string& type)
{
    std::string v, m, var;

    Controller::ConvertOldTypeToVendorModel(type, v, m, var);
    return v;
}

std::string FPP::GetModel(const std::string& type)
{
    std::string v, m, var;

    Controller::ConvertOldTypeToVendorModel(type, v, m, var);
    return m;
}

#ifndef DISCOVERYONLY
bool FPP::SetInputUniverses(Controller* controller, wxWindow* parentWin) {

    wxConfigBase* config = wxConfigBase::Get();
    wxString fip;
    config->Read("xLightsPiIP", &fip, "");
    wxString ausername;
    config->Read("xLightsPiUser", &ausername, "fpp");
    wxString apassword;
    config->Read("xLightsPiPassword", &apassword, "true");
    username = ToUTF8(ausername);
    password = ToUTF8(apassword);

    auto ips = wxSplit(fip, '|');
    auto users = wxSplit(ausername, '|');
    auto passwords = wxSplit(password, '|');

    // they should all be the same size ... but if not base it off the smallest
    int count = std::min(ips.size(), std::min(users.size(), passwords.size()));

    username = "fpp";
    wxString thePassword = "true";
    for (int i = 0; i < count; i++) {
        if (ips[i] == controller->GetIP()) {
            username = ToUTF8(users[i]);
            thePassword = ToUTF8(passwords[i]);
        }
    }

    if (thePassword == "true") {
        if (username == "pi") {
            password = "raspberry";
        } else if (username == "fpp") {
            password = "falcon";
        } else {
            wxTextEntryDialog ted(parentWin, "Enter password for " + username, "Password", controller->GetIP());
            if (ted.ShowModal() == wxID_OK) {
                password = ToUTF8(ted.GetValue());
            }
        }
    } else {
        wxTextEntryDialog ted(parentWin, "Enter password for " + username, "Password", controller->GetIP());
        if (ted.ShowModal() == wxID_OK) {
            password = ToUTF8(ted.GetValue());
        }
    }

    parent = parentWin;

    return (AuthenticateAndUpdateVersions() && !SetInputUniversesBridge(controller));
}

bool FPP::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent)
{
    parent = parent;
    return AuthenticateAndUpdateVersions()
        && !UploadPanelOutputs(allmodels, outputManager, controller)
        && !UploadVirtualMatrixOutputs(allmodels, outputManager, controller)
        && !UploadPixelOutputs(allmodels, outputManager, controller)
        && !UploadSerialOutputs(allmodels, outputManager, controller)
        && !UploadPWMOutputs(allmodels, outputManager, controller)
        && !Restart("");
}

bool FPP::UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) {
    parent = parent;
    bool b = AuthenticateAndUpdateVersions();
    if (!b) return b;
    UploadPanelOutputs(allmodels, outputManager, controller);
    UploadVirtualMatrixOutputs(allmodels, outputManager, controller);
    UploadPixelOutputs(allmodels, outputManager, controller);
    UploadSerialOutputs(allmodels, outputManager, controller);
    UploadPWMOutputs(allmodels, outputManager, controller);
    SetInputUniversesBridge(controller);
    
    if (restartNeeded) {
        Restart();
    }
    return b;
}

nlohmann::json FPP::CreateUniverseFile(const std::list<Controller*>& selected, bool input, std::map<int, int> *rngs) {
    nlohmann::json root;
    root["type"] = std::string("universes");
    root["enabled"] = 1;
    root["timeout"] = 1000;
    root["startChannel"] = 1;
    root["channelCount"] = -1;

    nlohmann::json universes;

    for (const auto& it2 : selected) {
        auto eth = dynamic_cast<ControllerEthernet*>(it2);
        if (eth == nullptr) {
            continue;
        }
        auto controllerEnabled = eth->GetActive();
        bool const allSameSize = eth->AllSameSize();
        // Get universes based on IP
        std::list<Output*> outputs = eth->GetOutputs();
        for (const auto& it : outputs) {
            int c = it->GetStartChannel();

            nlohmann::json universe;
            if (!input) {
                universe["active"] = controllerEnabled == Controller::ACTIVESTATE::ACTIVE ? 1 : 0;
            } else {
                universe["active"] = 1;
            }
            universe["description"] = stripInvalidChars(it2->GetName()).ToStdString();
            universe["id"] = it->GetUniverse();
            universe["startChannel"] = c;
            universe["channelCount"] = it->GetChannels();
            universe["address"] = std::string("");
            universe["priority"] = 0;
            universe["deDuplicate"] = eth->IsSuppressDuplicateFrames() ? 1 : 0;
            universe["monitor"] = eth->IsMonitoring() ? 1 : 0;

            if (rngs && it->GetChannels() > 0 && controllerEnabled == Controller::ACTIVESTATE::ACTIVE) {
                (*rngs)[c] = c + it->GetChannels() - 1;
            }

            if (it->GetType() == OUTPUT_E131) {
                universe["type"] = (int)(it->GetIP() != "MULTICAST" ? 1 : 0);
                if (!input && (it->GetIP() != "MULTICAST")) {
                    universe["address"] = it->GetIP();
                }
                if (it->GetIP() == "MULTICAST") {
                    universe["monitor"] = 0;
                }

                E131Output* e131 = dynamic_cast<E131Output*>(it);
                universe["priority"] = e131->GetPriority();

                // TODO this needs work to restore the loading of multiple universes as a single line
                if (allSameSize) {
                    universe["universeCount"] = it2->GetOutputCount();
                    universes.push_back(universe);
                    break;
                }
                universe["universeCount"] = 1;

                universes.push_back(universe);
            } else if (it->GetType() == OUTPUT_DDP || it->GetType() == OUTPUT_ZCPP) {
                if (!input) {
                    universe["address"] = it->GetIP();
                    DDPOutput* ddp = dynamic_cast<DDPOutput*>(it);
                    if (ddp) {
                        universe["type"] = ddp->IsKeepChannelNumbers() ? 4 : 5;
                    } else {
                        universe["type"] = 5;
                    }
                    universes.push_back(universe);
                } else {
                    //create empty array DDP input
                    universes = nlohmann::json::array();
                }
            } else if (it->GetType() == OUTPUT_ARTNET) {
                universe["type"] = (int)((eth->GetIP() != "MULTICAST") + 2);
                if (!input && (it->GetIP() != "MULTICAST")) {
                    universe["address"] = it->GetIP();
                }
                if (it->GetIP() == "MULTICAST") {
                    universe["monitor"] = 0;
                }
                if (allSameSize) {
                    universe["universeCount"] = it2->GetOutputCount();
                    universes.push_back(universe);
                    break;
                }
                //ArtNetOutput* ano = dynamic_cast<ArtNetOutput*>(it);
                universe["universeCount"] = 1;
                universes.push_back(universe);
            } else if (it->GetType() == OUTPUT_KINET) {
                KinetOutput* kiNet = dynamic_cast<KinetOutput*>(it);
                universe["address"] = kiNet->GetIP();
                universe["type"] = kiNet->GetVersion() + 5;
                universes.push_back(universe);
            } else if (it->GetType() == OUTPUT_TWINKLY) {
                universe["address"] = it->GetIP();
                universe["type"] = 8;
                universes.push_back(universe);
            }
        }
    }

    root["universes"] = universes;

    nlohmann::json json;
    nlohmann::json chan;
    chan.push_back(root);
    if (input) {
        json["channelInputs"] = chan;
    } else {
        json["channelOutputs"] = chan;
    }
    return json;
}

bool FPP::SetRestartFlag(bool forceOn9) {
    if (forceOn9 || !IsVersionAtLeast(9, 0)) {
        restartNeeded = true;
        return PutToURL("/api/settings/restartFlag", "2", "text/plain");
    }
    return true;
}

bool FPP::Restart( bool ifNeeded) {
    std::string val;
    if (ifNeeded && !restartNeeded) {
        return false;
    }
    GetURLAsString("/api/system/fppd/restart?quick=1", val);
    PutToURL("/api/settings/restartFlag", "0", "text/plain");
    restartNeeded = false;
    return false;
}

void FPP::UpdateChannelRanges()
{
    // This probably should handle drives correctly but as is it doesnt bail for now until we add drive support
    if (fppType != FPP_TYPE::FPP) {
        return;
    }
    nlohmann::json jval;
    int count = 0;
    while (count < 20) {
        if (GetURLAsJSON("/api/system/info", jval, false)) {
            if (jval.contains("channelRanges")) {
                std::string r = GetJSONStringValue(jval, "channelRanges");
                if (r.size() > 0) {
                    //append the  new ranges,  then parse/reset which will do a merge/cleanup
                    if (ranges.size() > 0) {
                        ranges += ",";
                    }
                    ranges += r;
                    std::map<int, int> rngs;
                    FillRanges(rngs);
                    SetNewRanges(rngs);
                    return;
                }
            } else {
                //fppd hasn't restarted yet, wait a tiny bit and try again
                ++count;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        else {
            // get call failed
            ++count;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void FPP::SetDescription(const std::string &st) {
    PutToURL("/api/settings/HostDescription", st, "text/plain");
}

bool FPP::SetInputUniversesBridge(Controller* controller) {

    bool forceUpload = false;
    if (!IsDDPInputEnabled()){
        forceUpload = restartNeeded = true;
    }

    auto c = dynamic_cast<ControllerEthernet*>(controller);
    if (c == nullptr && !forceUpload) {
        //DDP is already enabled and this isn't an ethernet controller so no inputs need to be added
        return false;
    }

    nlohmann::json udp = CreateUniverseFile(std::list<Controller*>({ controller }), true);
    if (udp["channelInputs"][0]["universes"].size() != 0 || forceUpload) {
        PostJSONToURL("/api/channel/output/universeInputs", udp);
    }

    return false;
}


static bool UpdateJSONValue(nlohmann::json &v, const std::string &key, int newValue) {
    if (!v.contains(key)) {
        v[key] = newValue;
        return true;
    }
    int origValue = GetJSONIntValue(v, key);
    if (origValue != newValue) {
        v[key] = newValue;
        return true;
    }
    return false;
}
static bool UpdateJSONFloatValue(nlohmann::json &v, const std::string &key, double newValue) {
    if (!v.contains(key)) {
        v[key] = newValue;
        return true;
    }
    float origValue = GetJSONDoubleValue(v, key);
    if (origValue != newValue) {
        v[key] = newValue;
        return true;
    }
    return false;
}
static bool UpdateJSONValue(nlohmann::json& v, const std::string& key, const std::string& newValue) {
    if (!v.contains(key)) {
        v[key] = newValue;
        return true;
    }
    std::string origValue = GetJSONStringValue(v, key);
    if (origValue != newValue) {
        v[key] = newValue;
        return true;
    }
    return false;
}

static bool mergeSerialInto(nlohmann::json &otherDmxData, nlohmann::json &otherOrigRoot, bool addDefaults) {
    bool changed = false;
    for (int x = 0; x < otherDmxData["channelOutputs"].size(); x++) {
        std::string device = GetJSONStringValue(otherDmxData["channelOutputs"][x], "device");
        std::string type = GetJSONStringValue(otherDmxData["channelOutputs"][x], "type");
        bool found = false;
        for (int y = 0; y < otherOrigRoot["channelOutputs"].size(); y++) {
            std::string origDevice = GetJSONStringValue(otherOrigRoot["channelOutputs"][y], "device");
            if (!device.empty() && !origDevice.empty() && origDevice == device) {
                //same device, see if type matches and update or disable
                std::string origType = GetJSONStringValue(otherOrigRoot["channelOutputs"][y], "type");
                if (!type.empty() && !origType.empty() && origType == type) {
                    //device and type the same, update values
                    found = true;
                    changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "description", GetJSONStringValue(otherDmxData["channelOutputs"][x], "description"));
                    changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "enabled", 1);
                    changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "startChannel", GetJSONIntValue(otherDmxData["channelOutputs"][x], "startChannel"));
                    changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "channelCount", GetJSONIntValue(otherDmxData["channelOutputs"][x], "channelCount"));

                    if (!addDefaults) {
                        if (type == "Renard") {
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "renardspeed", GetJSONIntValue(otherDmxData["channelOutputs"][x], "renardspeed"));
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "renardparm", GetJSONStringValue(otherDmxData["channelOutputs"][x], "renardparm"));
                        } else if (type == "LOR") {
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "firstControllerId", GetJSONIntValue(otherDmxData["channelOutputs"][x], "firstControllerId"));
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "speed", GetJSONIntValue(otherDmxData["channelOutputs"][x], "speed"));
                        } else if (type == "GenricSerial") {
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "speed", GetJSONIntValue(otherDmxData["channelOutputs"][x], "speed"));
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "header", GetJSONStringValue(otherDmxData["channelOutputs"][x], "header"));
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "footer", GetJSONStringValue(otherDmxData["channelOutputs"][x], "footer"));
                        }
                    }
                } else {
                    changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "enabled", 0);
                }
            }
        }
        if (!found) {
            //add some defaults if needed
            if (addDefaults) {
                if (type == "Renard") {
                    otherDmxData["channelOutputs"][x]["renardparm"] = "8N1";
                    otherDmxData["channelOutputs"][x]["renardspeed"] = 57600;
                } else if (type == "LOR") {
                    otherDmxData["channelOutputs"][x]["firstControllerId"] = 1;
                    otherDmxData["channelOutputs"][x]["speed"] = 19200;
                } else if (type == "GenericSerial") {
                    otherDmxData["channelOutputs"][x]["header"] = "";
                    otherDmxData["channelOutputs"][x]["footer"] = "";
                    otherDmxData["channelOutputs"][x]["speed"] = 9600;
                }
            }
            otherOrigRoot["channelOutputs"].push_back(otherDmxData["channelOutputs"][x]);
            changed = true;
        }
    }
    return changed;
}
#endif

bool FPP::IsCompatible(const ControllerCaps *rules,
                       std::string &origVend, std::string &origMod, std::string origVar, const std::string &origId,
                       std::string& driver, bool& supportsV5Receivers) {
    if (origMod.empty()) {
        Controller::ConvertOldTypeToVendorModel(origId, origVend, origMod, origVar);
    }
    if (IsVersionAtLeast(7, 0)) {
        // we can verify that the ID actually can load a pinout
        bool found = false;
        nlohmann::json val;
        std::string id = rules->GetID();
        if (GetURLAsJSON("/api/cape/strings", val)) {
            for (int x = 0; x < val.size(); x++) {
                if (val[x].get<std::string>() == id) {
                    found = true;
                }
            }
            //certain older capes may have versioned pin config files,
            //we'll need to check them
            if (!found) {
                id = rules->GetID() + "_v2";
                for (int x = 0; x < val.size(); x++) {
                    if (val[x].get<std::string>() == id) {
                        found = true;
                    }
                }
            }
            if (!found) {
                id = rules->GetID() + "_v3";
                for (int x = 0; x < val.size(); x++) {
                    if (val[x].get<std::string>() == id) {
                        found = true;
                    }
                }
            }
            if (!found) {
                id = rules->GetID() + "-v2";
                for (int x = 0; x < val.size(); x++) {
                    if (val[x].get<std::string>() == id) {
                        found = true;
                    }
                }
            }
            if (!found) {
                id = rules->GetID() + "-v3";
                for (int x = 0; x < val.size(); x++) {
                    if (val[x].get<std::string>() == id) {
                        found = true;
                    }
                }
            }
        }
        if (found) {
            nlohmann::json val;
            if (GetURLAsJSON("/api/cape/strings/" + id, val)) {
                if (val.contains("driver")) {
                    driver = val["driver"].get<std::string>();
                }
                if (val.contains("falconV5ListenerConfig")) {
                    supportsV5Receivers = true;;
                }
            } else {
                found = false;
            }
        }
        if (!found) {
            wxString msg = "Could not detect a pinout for " + rules->GetID() + " for controller type " + rules->GetModel() + ".  Configuration will not work.  Verify controller type/model/variant.  Continue?";
            if (wxMessageBox(msg, "Confirm", wxYES_NO, parent) != wxYES) {
                return false;
            }
        }
    }
    if (origMod != "" && rules->GetModel() != origMod) {
        wxString msg = "Configured controller type " + rules->GetModel() + " for " + ipAddress + " is not compatible with type already configured: "
            + origMod + ".   Continue?";
        if (wxMessageBox(msg, "Confirm", wxYES_NO, parent) != wxYES) {
            return false;
        }
    }
    return true;
}

#ifndef DISCOVERYONLY
bool FPP::UploadPanelOutputs(ModelManager* allmodels,
                             OutputManager* outputManager,
                             Controller* controller) {
    auto rules = ControllerCaps::GetControllerConfig(controller);
    if (rules == nullptr) {
        return false;
    }
    std::string check;
    UDController cud(controller, outputManager, allmodels, false);
    bool fullcontrol = rules->SupportsFullxLightsControl() && controller->IsFullxLightsControl();

    nlohmann::json origJson;
    bool changed = false;
    bool hasPanel = false;
    
    if (rules->SupportsLEDPanelMatrix()) {
        for (int x = 0; x < cud.GetMaxLEDPanelMatrixPort(); ++x) {
            if (cud.GetControllerLEDPanelMatrixPort(1 + x)->GetStartChannel() > 0) {
                hasPanel = true;
            }
        }
    }

    if (hasPanel || fullcontrol) {
        GetURLAsJSON("/api/channel/output/channelOutputsJSON", origJson, false);
    }
    if (hasPanel) {
        std::map<int, int> rngs;
        FillRanges(rngs);
        for (int panel = 0; panel < cud.GetMaxLEDPanelMatrixPort(); ++panel) {
            if (panel < origJson["channelOutputs"].size()) {
                int startChannel = cud.GetControllerLEDPanelMatrixPort(1 + panel)->GetStartChannel();
                if (startChannel > 0) {
                    if (UpdateJSONValue(origJson["channelOutputs"][panel], "startChannel", startChannel)) {
                        changed = true;
                        rngs[startChannel - 1] = origJson["channelOutputs"][panel]["channelCount"].get<int>();
                    }
                    changed |= UpdateJSONValue(origJson["channelOutputs"][panel], "enabled", 1);
                } else {
                    // need to disable the panel
                    changed |= UpdateJSONValue(origJson["channelOutputs"][panel], "enabled", 0);
                }
            }
        }
        SetNewRanges(rngs);
    } else if (fullcontrol) {
        //disable
        for (int x = 0; x < origJson["channelOutputs"].size(); x++) {
            if (origJson["channelOutputs"][x]["type"].get<std::string>() == "LEDPanelMatrix") {
                changed |= UpdateJSONValue(origJson["channelOutputs"][x], "enabled", 0);
            }
        }
    }
    if (changed) {
        PostJSONToURL("/api/channel/output/channelOutputsJSON", origJson);
        SetRestartFlag();
    }
    return false;
}


bool FPP::UploadVirtualMatrixOutputs(ModelManager* allmodels,
                                     OutputManager* outputManager,
                                     Controller* controller) {
    auto rules = ControllerCaps::GetControllerConfig(controller);
    if (rules == nullptr) {
        return false;
    }
    std::string check;
    UDController cud(controller, outputManager, allmodels, false);
    bool fullcontrol = rules->SupportsFullxLightsControl() && controller->IsFullxLightsControl();
    bool changed = false;
    nlohmann::json origJson;
    if (fullcontrol || (rules->SupportsVirtualMatrix() && cud.GetMaxVirtualMatrixPort())) {
        GetURLAsJSON("/api/channel/output/co-other", origJson, false);
        if (fullcontrol) {
            for (int x = 0; x < origJson["channelOutputs"].size(); x++) {
                if (origJson["channelOutputs"][x]["type"].get<std::string>() == "VirtualMatrix") {
                    origJson["channelOutputs"].erase(x);
                    x--;
                    changed = true;
                }
            }
        }
    }
    std::map<int, std::set<std::string>> models;
    if (rules->SupportsVirtualMatrix() && cud.GetMaxVirtualMatrixPort()) {
        std::map<int, int> rngs;
        FillRanges(rngs);
        for (int port = 0; port < cud.GetMaxVirtualMatrixPort(); port++) {
            int curOffset = 0;
            int countModels = cud.GetControllerVirtualMatrixPort(port+1)->GetModels().size();
            for (auto m : cud.GetControllerVirtualMatrixPort(port+1)->GetModels()) {
                int startChannel = m->GetStartChannel();
                std::string name = m->GetName();
                MatrixModel *mm = dynamic_cast<MatrixModel*>(m->GetModel());
                std::string layout;
                int w = -1;
                int h = -1;
                if (mm != nullptr) {
                    if (mm->isVerticalMatrix()) {
                        w = mm->GetNumStrings();
                        h = mm->NodesPerString();
                    } else {
                        w = mm->NodesPerString();
                        h = mm->GetNumStrings();
                    }
                    if (w != -1 && h != -1) {
                        layout = std::to_string(w) + "x" + std::to_string(h);
                    }
                }

                models[port].insert(name);
                bool found = false;
                for (int x = 0; x < origJson["channelOutputs"].size(); x++) {
                    if (origJson["channelOutputs"][x]["type"].get<std::string>() == "VirtualMatrix"
                        && origJson["channelOutputs"][x]["description"].get<std::string>() == name) {
                        found = true;
                        changed |= UpdateJSONValue(origJson["channelOutputs"][x], "enabled", 1);
                        changed |= UpdateJSONValue(origJson["channelOutputs"][x], "startChannel", startChannel);
                        changed |= UpdateJSONValue(origJson["channelOutputs"][x], "channelCount", m->GetEndChannel() - startChannel + 1);
                        changed |= UpdateJSONValue(origJson["channelOutputs"][x], "width", w > 0 ? w : 64);
                        changed |= UpdateJSONValue(origJson["channelOutputs"][x], "height", h > 0 ? h : 32);
                        changed |= UpdateJSONValue(origJson["channelOutputs"][x], "layout", layout);
                        changed |= UpdateJSONValue(origJson["channelOutputs"][x], "yoff", curOffset);
                        curOffset += h > 0 ? h : 0;
                        if (countModels > 1) {
                            changed |= UpdateJSONValue(origJson["channelOutputs"][x], "scaling", "None");
                        } else {
                            changed |= UpdateJSONValue(origJson["channelOutputs"][x], "scaling", "Hardware");
                        }
                        rngs[m->GetStartChannel()] = m->GetEndChannel() - m->GetStartChannel() + 1;
                    }
                }
                if (!found) {
                    nlohmann::json v;
                    v["enabled"] = 1;
                    v["type"] = std::string("VirtualMatrix");
                    v["startChannel"] = startChannel;
                    v["channelCount"] = m->GetEndChannel() - m->GetStartChannel() + 1;
                    v["width"] = w > 0 ? w : 64;
                    v["height"] = h > 0 ? h : 32;
                    v["layout"] = layout;
                    v["colorOrder"] = std::string("RGB");
                    v["invert"] = 0;
                    if (IsVersionAtLeast(8, 0)) {
                        v["device"] = wxString::Format("HDMI-A-%d", port + 1).ToStdString(); //hdmi ports are 1 based, not 0 like fb
                    } else {
                        v["device"] = wxString::Format("fb%d", port).ToStdString();
                    }
                    v["xoff"] = 0;
                    v["description"] = name;
                    v["yoff"] = curOffset;
                    curOffset += h > 0 ? h : 0;
                    if (countModels > 1) {
                        v["scaling"] = std::string("None");
                    } else {
                        v["scaling"] = std::string("Hardware");
                    }
                    origJson["channelOutputs"].push_back(v);
                    rngs[m->GetStartChannel()] = m->GetEndChannel() - m->GetStartChannel() + 1;
                    changed = true;
                }
                if (changed) {
                    SetNewRanges(rngs);
                }
            }
        }
    }
    if (!fullcontrol && changed) {
        //we need to disable the virtual matrices that are on the ports of the
        //models we uploaded or they will conflict and produce errors
        for (int x = 0; x < origJson["channelOutputs"].size(); x++) {
            if (origJson["channelOutputs"][x]["type"].get<std::string>() == "VirtualMatrix") {
                wxString dev = origJson["channelOutputs"][x]["device"].get<std::string>();
                int port = (char)dev[2] - '0';
                if (models[port].find(ToUTF8(origJson["channelOutputs"][x]["description"].get<std::string>())) == models[port].end()) {
                    UpdateJSONValue(origJson["channelOutputs"][x], "enabled", 0);
                }
            }
        }
    }
    if (changed) {
        PostJSONToURL("/api/channel/output/co-other", origJson);
        SetRestartFlag();
    }

    return false;
}

bool FPP::UploadSerialOutputs(ModelManager* allmodels,
                              OutputManager* outputManager,
                              Controller* c) {

    ControllerSerial *controller = dynamic_cast<ControllerSerial*>(c);
    if (controller == nullptr) {
        //non SerialControllers are handled in the UploadPixelOutputs method at this point
        return false;
    }
    auto rules = ControllerCaps::GetControllerConfig(controller);
    if (rules == nullptr) {
        return false;
    }

    int maxSerial = rules->GetMaxSerialPort();
    if (maxSerial == 0) {
        return false;
    }

    std::map<int, int> rngs;
    FillRanges(rngs);

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("FPP Serial Outputs Upload: Uploading to %s", (const char *)ipAddress.c_str());

    UDController cud(controller, outputManager, allmodels, false);
    if (cud.GetMaxSerialPort() == 0) {
        return false;
    }
    std::string check;
    cud.Check(rules, check);
    cud.Dump();

    nlohmann::json otherData;
    for (int sp = 1; sp <= rules->GetMaxSerialPort(); sp++) {
        nlohmann::json port;
        if (cud.HasSerialPort(sp)) {
            UDControllerPort* vport = cud.GetControllerSerialPort(sp);
            int sc = vport->GetStartChannel();
            port["startChannel"] = sc;
            int mx = vport->GetEndChannel() - sc + 1;
            std::string dev = controller->GetPort().substr(controller->GetPort().find(":") + 1);
            port["device"] = dev;
            port["enabled"] = 1;
            std::string tp = controller->GetProtocol();
            if (tp == "DMX" || tp == "dmx" || tp == OUTPUT_DMX) {
                port["type"] = std::string("DMX-Pro");
                if (mx < 16) {
                    //several controllers have issues if the DMX data stream has less than 16 channels
                    mx = 16;
                }
            } else if (tp == OUTPUT_OPENDMX) {
                port["type"] = std::string("DMX-Open");
                if (mx < 16) {
                    //several controllers have issues if the DMX data stream has less than 16 channels
                    mx = 16;
                }
            } else if (tp == OUTPUT_PIXELNET) {
                port["type"] = std::string("Pixelnet-Lynx");
                mx = 4096;
            } else if (tp == OUTPUT_OPENPIXELNET) {
                port["type"] = std::string("Pixelnet-Open");
                mx = 4096;
            } else if (tp == OUTPUT_LOR) {
                port["type"] = std::string("LOR");
                port["speed"] = controller->GetSpeed();
                port["firstControllerId"] = 1;
            } else if (tp == OUTPUT_RENARD) {
                port["type"] = std::string("Renard");
                port["speed"] = controller->GetSpeed();
                port["renardparm"] = "8N1";
            } else if (tp == OUTPUT_GENERICSERIAL) {
                port["type"] = std::string("GenericSerial");
                port["speed"] = controller->GetSpeed();
                port["header"] = controller->GetSaveablePreFix();
                port["footer"] = controller->GetSaveablePostFix();
            }
            std::string description = controller->GetDescription();
            if (description.empty()) {
                description = controller->GetName();
            }
            port["description"] = description;
            port["channelCount"] = mx;
            otherData["channelOutputs"].push_back(port);

            rngs[sc - 1] = mx;

            nlohmann::json otherOrigRoot = otherData;
            bool changed = true;
            if (GetURLAsJSON("/api/configfile/co-other.json", otherOrigRoot, false)) {
                changed = mergeSerialInto(otherData, otherOrigRoot, false);
            }
            if (changed) {
                PostJSONToURL("/api/configfile/co-other.json", otherOrigRoot);
                SetRestartFlag();
                SetNewRanges(rngs);
            }
        }
    }

    return false;
}

bool FPP::UploadPWMOutputs(ModelManager* allmodels,
                           OutputManager* outputManager,
                           Controller* controller) {
    auto rules = ControllerCaps::GetControllerConfig(controller);
    if (rules == nullptr) {
        return false;
    }
    int maxPort = rules->GetMaxPWMPort();
    if (maxPort <= 0) {
        return false;
    }
    if (!IsVersionAtLeast(8, 0)) {
        //PWM output requires FPP 8.0 or later
        return true;
    }
    bool hasPWM = false;
    if (!capeInfo.contains("id")) {
        GetURLAsJSON("/api/cape", capeInfo);
    }
    for (int x = 0; x < capeInfo["provides"].size(); x++) {
        if (capeInfo["provides"][x].get<std::string>() == "pwm") {
            hasPWM = true;
        }
    }
    if (!hasPWM) {
        return true;
    }
    
    UDController cud(controller, outputManager, allmodels, false);
    if (cud.GetMaxPWMPort() == 0) {
        return false;
    }
    bool const fullcontrol = rules->SupportsFullxLightsControl() && controller->IsFullxLightsControl();

    std::map<int, int> rngs;
    FillRanges(rngs);
    bool changed = false;
    
    nlohmann::json root;
    int pca9685Index = -1;
    if (!fullcontrol && GetURLAsJSON("/api/configfile/co-pwm.json", root, false)) {
        if (root.contains("channelOutputs")) {
            for (int x = 0; x < root["channelOutputs"].size(); x++) {
                if (root["channelOutputs"][x]["type"].get<std::string>() == "PCA9685") {
                    pca9685Index = x;
                    break;
                }
            }
        }
    }
    if (pca9685Index == -1) {
        changed = true;
        pca9685Index = 0;
        root["channelOutputs"] = nlohmann::json::array();
        root["channelOutputs"][pca9685Index]["type"] = std::string("PCA9685");
        root["channelOutputs"][pca9685Index]["subType"] = rules->GetID();
        root["channelOutputs"][pca9685Index]["enabled"] = 1;
        root["channelOutputs"][pca9685Index]["frequency"] = controller->GetExtraProperty("PWMFrequency", "50hz");
        root["channelOutputs"][pca9685Index]["startChannel"] = 0;
        root["channelOutputs"][pca9685Index]["channelCount"] = -1;
        root["channelOutputs"][pca9685Index]["outputs"] = nlohmann::json::array();
    }
    // make sure we have enough ports....
    while (maxPort > root["channelOutputs"][pca9685Index]["outputs"].size()) {
        changed = true;
        nlohmann::json v;
        v["description"] = "";
        v["startChannel"] = 0;
        v["is16bit"] = 1;
        v["type"] = std::string("Servo");
        v["min"] = 1000;
        v["max"] = 2000;
        v["reverse"] = 0;
        v["zero"] = std::string("Hold");
        v["dataType"] = std::string("Scaled");
        root["channelOutputs"][pca9685Index]["outputs"].push_back(v);
    }
    for (int x = 0; x < maxPort; x++) {
        if (x < cud.GetMaxPWMPort()) {
            auto *p = cud.GetControllerPWMPort(x + 1);
            auto *m = p->GetFirstModel();
            auto &jv = root["channelOutputs"][pca9685Index]["outputs"][x];
            if (m) {
                const auto &props = m->GetPWMProperties();
                changed |= UpdateJSONValue(jv, "startChannel", m->GetStartChannel());
                std::string mname = m->GetName();
                changed |= UpdateJSONValue(jv, "description", mname + " - " + props.label);
                changed |= UpdateJSONValue(jv, "is16bit", m->GetStartChannel() != m->GetEndChannel() ? 1 : 0);
                if (props.type == 0) {
                    //LED
                    changed |= UpdateJSONValue(jv, "type", "LED");
                    changed |= UpdateJSONValue(jv, "brightness", props.brightness);
                    changed |= UpdateJSONFloatValue(jv, "gamma", props.gamma);
                } else {
                    //SERVO
                    changed |= UpdateJSONValue(jv, "type", "Servo");
                    changed |= UpdateJSONValue(jv, "min", props.minValue);
                    changed |= UpdateJSONValue(jv, "max", props.maxValue);
                    changed |= UpdateJSONValue(jv, "reverse", props.reverse ? 1 : 0);
                    changed |= UpdateJSONValue(jv, "zero", props.zeroBehavior);
                    changed |= UpdateJSONValue(jv, "dataType", props.dateType);
                }
            }
        }
    }
    
    if (changed) {
        PostJSONToURL("/api/configfile/co-pwm.json", root);
        SetRestartFlag();
        SetNewRanges(rngs);
    }
    return false;
}


bool FPP::UploadPixelOutputs(ModelManager* allmodels,
                             OutputManager* outputManager,
                             Controller* controller) {
    int maxString = 1;
    auto rules = ControllerCaps::GetControllerConfig(controller);
    if (rules == nullptr) {
        return false;
    }

    maxString = rules->GetMaxPixelPort();
    if (maxString == 0) {
        return false;
    }

    std::map<int, int> rngs;
    FillRanges(rngs);

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("FPP Pixel Outputs Upload: Uploading to %s", (const char *)ipAddress.c_str());

    UDController cud(controller, outputManager, allmodels, false);

    if (cud.GetMaxPixelPort() == 0 && cud.GetMaxSerialPort() == 0) {
        return false;
    }
    std::string fppFileName = rules->GetCustomPropertyByPath("fppStringFileName");
    if (fppFileName.empty()) {
        fppFileName = "co-bbbStrings";
    }
    std::string check;
    cud.Check(rules, check);
    cud.Dump();

    nlohmann::json origJson;
    GetURLAsJSON("/api/channel/output/" + fppFileName, origJson, false);
    logger_base.debug("Original JSON");
    DumpJSON(origJson);

    bool fullcontrol = rules->SupportsFullxLightsControl() && controller->IsFullxLightsControl();
    int defaultBrightness = controller->GetDefaultBrightnessUnderFullControl() + 2;
    // round to nearest 5
    defaultBrightness -= defaultBrightness % 5;
    if (defaultBrightness == 0) {
        defaultBrightness = 100;
    }

    float defaultGamma = controller->GetDefaultGammaUnderFullControl();

    std::string pinout = "1.x";
    std::map<std::string, nlohmann::json> origStrings;
    std::string origSubType;
    if (origJson["channelOutputs"].is_array()) {
        for (int x = 0; x < origJson["channelOutputs"].size(); x++) {
            nlohmann::json &f = origJson["channelOutputs"][x];
            if (f.contains("pinoutVersion")) {
                pinout = f["pinoutVersion"].get<std::string>();
            }
            if (pinout.empty()) {
                pinout = "1.x";
            }
            if (f.contains("subType")) {
                origSubType = (f["subType"].get<std::string>());
            }
            if (!fullcontrol) {
                for (int o = 0; o < f["outputs"].size(); o++) {
                    if (f["outputs"][o].contains("virtualStrings")) {
                        for (int vs = 0; vs < f["outputs"][o]["virtualStrings"].size(); vs++) {
                            nlohmann::json val = f["outputs"][o]["virtualStrings"][vs];
                            if (val["description"].get<std::string>() != "") {
                                origStrings[val["description"].get<std::string>()] = val;
                            }
                        }
                    }
                }
            }
        }
    }
    int maxport = 0;

    nlohmann::json stringData;
    stringData["enabled"] = 1;
    stringData["startChannel"] = 1;
    stringData["channelCount"] = -1;

    maxport = cud.GetMaxPixelPort(); // 1 based

    std::string fppDriver = rules->GetCustomPropertyByPath("fppStringDriverType");
    bool supportsV5Receivers{false};
    if (fppFileName == "co-bbbStrings") {
        if (!IsCompatible(rules, controllerVendor, controllerModel, controllerVariant, origSubType, fppDriver, supportsV5Receivers)) {
            return true;
        }
        if (fppDriver.empty()) {
            fppDriver = "BBB48String";
        }
        stringData["type"] = fppDriver;
        stringData["subType"] = rules->GetID();
        stringData["pinoutVersion"] = pinout;
    } else {
        if (fppDriver.empty()) {
            fppDriver = "RPIWS281X";
        }
        stringData["type"] = fppDriver;
        stringData["subType"] = rules->GetID();
        stringData["pinoutVersion"] = pinout;
    }

    if (maxport > rules->GetMaxPixelPort()) {
        maxport = rules->GetMaxPixelPort();
    }
    stringData["outputCount"] = maxport;

    for (int x = 0; x < rules->GetMaxPixelPort(); x++) {
        nlohmann::json port;
        port["portNumber"] = x;

        stringData["outputs"].push_back(port);
    }

    for (int pp = 1; pp <= rules->GetMaxPixelPort(); pp++) {
        if (cud.HasPixelPort(pp)) {
            UDControllerPort* port = cud.GetControllerPixelPort(pp);
            port->CreateVirtualStrings(false, false);
            for (const auto& pvs : port->GetVirtualStrings()) {
                nlohmann::json vs;
                if (pvs->_isDummy) {
                    vs["description"] = std::string("");
                    vs["startChannel"] = 0;
                    vs["pixelCount"] = 0;
                } else {
                    vs["description"] = pvs->_description;
                    vs["startChannel"] = pvs->_startChannel - 1; // we need 0 based
                    vs["pixelCount"] = pvs->Channels() / pvs->_channelsPerPixel;

                    rngs[pvs->_startChannel - 1] = pvs->Channels();
                }

                if (!pvs->_isDummy && (origStrings.find(vs["description"].get<std::string>()) != origStrings.end())) {
                    nlohmann::json &vo = origStrings[vs["description"].get<std::string>()];
                    vs["groupCount"] = vo["groupCount"];
                    vs["reverse"] = vo["reverse"];
                    vs["colorOrder"] = vo["colorOrder"];
                    vs["nullNodes"] = vo["nullNodes"];
                    vs["endNulls"] = vo.contains("endNulls") ? vo["endNulls"].get<int>() : 0;
                    vs["zigZag"] = vo["zigZag"];
                    vs["brightness"] = vo["brightness"];
                    vs["gamma"] = vo["gamma"];
                } else {
                    vs["groupCount"] = 0;
                    vs["reverse"] = 0;
                    if (pvs->_channelsPerPixel == 4) {
                        vs["colorOrder"] = std::string("RGBW");
                    } else if (pvs->_channelsPerPixel == 1) {
                        vs["colorOrder"] = std::string("W");
                    } else {
                        vs["colorOrder"] = std::string("RGB");
                    }
                    vs["nullNodes"] = 0;
                    vs["endNulls"] = 0;
                    vs["zigZag"] = 0; // If we zigzag in xLights, we don't do it in the controller, if we need it in the controller, we don't know about it here
                    vs["brightness"] = defaultBrightness;
                    vs["gamma"] = wxString::Format("%.1f", defaultGamma).ToStdString();
                }
                if (pvs->_reverseSet) {
                    vs["reverse"] = pvs->_reverse == "Reverse" ? 1 : 0;
                }
                if (pvs->_gammaSet) {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%g", pvs->_gamma);
                    std::string gam = buf;
                    vs["gamma"] = gam;
                }
                if (pvs->_brightnessSet) {
                    // round to nearest 5
                    int i = pvs->_brightness + 2;
                    i -= i % 5;
                    vs["brightness"] = i;
                }
                if (pvs->_startNullPixelsSet) {
                    vs["nullNodes"] = pvs->_startNullPixels;
                }
                if (pvs->_endNullPixelsSet) {
                    vs["endNulls"] = pvs->_endNullPixels;
                }
                if (pvs->_colourOrderSet) {
                    vs["colorOrder"] = pvs->_colourOrder;
                }
                if (pvs->_channelsPerPixel == 1) {
                    vs["colorOrder"] = std::string("W");
                }
                if (pvs->_groupCountSet) {
                    vs["groupCount"] = pvs->_groupCount;
                }
                if (vs["groupCount"].get<int>() > 1) {
                    //if the group count is >1, we need to adjust the number of pixels
                    vs["pixelCount"] = vs["pixelCount"].get<int>() * vs["groupCount"].get<int>();
                }
                if (pvs->_zigZagSet) {
                    vs["zigZag"] = pvs->_zigZag;
                }
                std::string vsname = "virtualStrings";
                if (pvs->_smartRemote == 2) {
                    vsname += "B";
                } else if (pvs->_smartRemote == 3) {
                    vsname += "C";
                } else if (pvs->_smartRemote == 4) {
                    vsname += "D";
                } else if (pvs->_smartRemote == 5) {
                    vsname += "E";
                } else if (pvs->_smartRemote == 6) {
                    vsname += "F";
                }
                if (pvs->_smartRemote >= 1) {
                    auto const diff_type = DecodeReceiverType(pvs->_smartRemoteType, supportsV5Receivers);
                    if (diff_type == ReceiverType::FalconV5) {
                        stringData["outputs"][port->GetPort() - 1]["differentialType"] = 10;
                    } else if(diff_type == ReceiverType::v2) {
                        stringData["outputs"][port->GetPort() - 1]["differentialType"] = 4;
                    } else {
                        stringData["outputs"][port->GetPort() - 1]["differentialType"] = 1;
                    }
                }

                stringData["outputs"][port->GetPort() - 1][vsname].push_back(vs);
            }
        }
    }

    for (int x = 0; x < rules->GetMaxPixelPort(); x++) {
        if (!stringData["outputs"][x].contains("virtualStrings")
            || stringData["outputs"][x]["virtualStrings"].is_null()
            || stringData["outputs"][x]["virtualStrings"].size() == 0) {
            nlohmann::json vs;
            vs["description"] = wxString("");
            vs["startChannel"] = 0;
            vs["pixelCount"] = 0;
            vs["groupCount"] = 0;
            vs["reverse"] = 0;
            vs["colorOrder"] = wxString("RGB");
            vs["nullNodes"] = 0;
            vs["endNulls"] = 0;
            vs["zigZag"] = 0;
            vs["brightness"] = defaultBrightness;
            vs["gamma"] = wxString::Format("%.1f", defaultGamma).ToStdString();
            stringData["outputs"][x]["virtualStrings"].push_back(vs);
        }
        if ((x & 0x3) == 0) {
            //need to check the group of 4 to see if we need a smartRemote or not
            int remoteType = 0;
            ReceiverType receiverType{ ReceiverType::Standard };
            for (int z = 0; z < 4; z++) {
                if ((x + z) < maxport) {
                    if (stringData["outputs"][x + z].contains("virtualStringsF")) {
                        remoteType = std::max(remoteType, 6);
                    } else if (stringData["outputs"][x + z].contains("virtualStringsE")) {
                        remoteType = std::max(remoteType, 5);
                    } else if (stringData["outputs"][x + z].contains("virtualStringsD")) {
                        remoteType = std::max(remoteType, 4);
                    } else if (stringData["outputs"][x + z].contains("virtualStringsC")) {
                        remoteType = std::max(remoteType, 3);
                    } else if (stringData["outputs"][x + z].contains("virtualStringsB")) {
                        remoteType = std::max(remoteType, 2);
                    } else if (stringData["outputs"][x + z].contains("differentialType")) {
                        remoteType = std::max(remoteType, 1);
                    }
                    if (stringData["outputs"][x + z].contains("differentialType")) {
                        receiverType = DecodeReceiverType(stringData["outputs"][x + z]["differentialType"].get<int>(), supportsV5Receivers);
                    }
                }
            }
            if (ReceiverType::FalconV5 == receiverType) {
                remoteType += 9;
            } else if (ReceiverType::v2 == receiverType) {
                remoteType += 3;
            }
            if (remoteType) {
                for (int z = 0; z < 4; z++) {
                    if ((x + z) < maxport) {
                        stringData["outputs"][x+z]["differentialType"] = remoteType;
                    }
                }
            }
        }
    }
    auto expansionPorts = GetExpansionPorts(rules);
    for (int x = 0; x <= rules->GetMaxPixelPort(); x++) {
        if (expansionPorts.find(x+1) != expansionPorts.end()) {
            if (x == rules->GetMaxPixelPort()) {
                nlohmann::json port;
                port["portNumber"] = x;
                port["expansionType"] = expansionPorts.find(x+1)->second;
                stringData["outputs"].push_back(port);
            } else {
                int count = expansionPorts.find(x+1)->second;
                int expansionType = 0;
                for (int p = 0; p < count; p++) {
                    if (stringData["outputs"][x + p].contains("differentialType") && stringData["outputs"][x + p]["differentialType"].get<int>()) {
                        expansionType = 1;
                    }
                }
                stringData["outputs"][x]["expansionType"] = expansionType;
            }
        }
    }

    bool isDMX = true;
    int maxChan = 0;
    bool hasSerial = false;
    for (int sp = 1; sp <= cud.GetMaxSerialPort(); sp++) {
        if (cud.HasSerialPort(sp)) {
            UDControllerPort* port = cud.GetControllerSerialPort(sp);
            isDMX &= ((port->GetProtocol() == "DMX") || (port->GetProtocol() == "dmx") || (port->GetProtocol() == "DMX-Open"));

            //int dmxOffset = 1;
            //UDControllerPortModel* m = port->GetFirstModel();
            //if (m != nullptr) {
            //    dmxOffset = m->GetDMXChannelOffset();
            //    if (dmxOffset < 1) dmxOffset = 1; // a value less than 1 makes no sense
            //}
            int sc = port->GetStartChannel();
            int mx = port->GetEndChannel() - sc + 1;
            maxChan = std::max(mx, maxChan);
        }
    }

    nlohmann::json bbbDmxData;
    bool hasBBBDmx = false;
    nlohmann::json otherDmxData;
    for (int sp = 1; sp <= rules->GetMaxSerialPort(); sp++) {
        nlohmann::json port;
        std::string portType = rules->GetCustomPropertyByPath("fppSerialPort" + std::to_string(sp), "BBBSerial");
        if (cud.HasSerialPort(sp)) {
            UDControllerPort* vport = cud.GetControllerSerialPort(sp);
            //int dmxOffset = 1;
            //UDControllerPortModel* m = vport->GetFirstModel();
            //if (m != nullptr) {
            //    dmxOffset = m->GetDMXChannelOffset();
            //    if (dmxOffset < 1) dmxOffset = 1; // a value less than 1 makes no sense
            //}
            int sc = vport->GetStartChannel();
            port["startChannel"] = sc;
            if (portType == "BBBSerial") {
                port["channelCount"] = isDMX ? (maxChan < 16 ? 16 : maxChan) : 4096;
                port["outputNumber"] = (sp - 1);
                port["outputType"] = isDMX ? ("DMX") : ("Pixelnet");
                bbbDmxData["outputs"].push_back(port);
                rngs[sc] = isDMX ? (maxChan < 16 ? 16 : maxChan) : 4096;
                hasBBBDmx = true;
                hasSerial = true;
            } else {
                int mx = vport->GetEndChannel() - sc + 1;
                port["device"] = portType;
                port["enabled"] = 1;
                std::string tp = vport->GetProtocol();
                if (tp == "DMX" || tp == "dmx") {
                    tp = "DMX-Open";
                } else if (tp == "PIXELNET" || tp == "pixelnet") {
                    tp = "Pixelnet-Open";
                }
                if (mx < 16) {
                    //several controllers have issues if the DMX data stream has less than 16 channels
                    mx = 16;
                }
                port["channelCount"] = mx;
                port["type"] = tp;
                port["description"] = ("");
                otherDmxData["channelOutputs"].push_back(port);
            }

        } else if (portType == "BBBSerial") {
            hasBBBDmx = true;
            port["startChannel"] = 0;
            port["channelCount"] = 0;
            port["outputNumber"] = (sp - 1);
            port["outputType"] = isDMX ? ("DMX") : ("Pixelnet");
            bbbDmxData["outputs"].push_back(port);
        }
    }
    if (hasBBBDmx) {
        bbbDmxData["channelCount"] = isDMX ? (maxChan < 16 ? 16 : maxChan) : 4096;
        if (maxChan == 0) {
            bbbDmxData["enabled"] = 0;
            bbbDmxData["subType"] = ("off");
        }
    }
    // let the string handling know if it's safe to use the other PRU
    // or if the serial out will need it
    stringData["serialInUse"] = hasSerial;

    nlohmann::json root;
    root["channelOutputs"].push_back(stringData);
    if (hasBBBDmx) {
        bbbDmxData["enabled"] = 1;
        bbbDmxData["startChannel"] = 1;
        bbbDmxData["type"] = ("BBBSerial");
        bbbDmxData["subType"] = isDMX ? ("DMX") : ("PixelNet") ;
        bbbDmxData["device"] = (rules->GetID());
        bbbDmxData["pinoutVersion"] = pinout;
        root["channelOutputs"].push_back(bbbDmxData);
    } else {
        nlohmann::json otherOrigRoot = otherDmxData;
        bool changed = true;
        if (GetURLAsJSON("/api/configfile/co-other.json", otherOrigRoot, false)) {
            changed = mergeSerialInto(otherDmxData, otherOrigRoot, true);
        }
        if (changed) {
            PostJSONToURL("/api/configfile/co-other.json", otherOrigRoot);
            SetRestartFlag();
        }
    }

    logger_base.debug("New JSON");
    DumpJSON(root);

    if (origJson != (root)) {
        logger_base.debug("Uploading New JSON");
        PostJSONToURL("/api/channel/output/" + fppFileName, root);
        SetRestartFlag();
    } else {
        logger_base.debug("Skipping JSON upload as it has not changed.");
    }
    SetNewRanges(rngs);
    return false;
}

bool FPP::UploadControllerProxies(OutputManager* outputManager)
{
    if(IsVersionAtLeast(8, 0)) {
        auto currentProxies = GetProxies();
        for (const auto& it : outputManager->GetControllers()) {
            auto c = dynamic_cast<ControllerEthernet*>(it);
            if (c != nullptr) {
                std::string proxy_ip = ip_utils::ResolveIP(c->GetFPPProxy());
                if (ipAddress.compare(proxy_ip) == 0) {
                    auto const& controller_ip = c->GetIP();
                    auto const& controller_name = c->GetName();
                    if (std::find_if(currentProxies.begin(), currentProxies.end(),
                        [controller_ip](auto const& pro) { return get<0>(pro) == controller_ip; }) == currentProxies.end()) {
                        currentProxies.emplace_back(controller_ip, controller_name);
                    }
                }
            }
        }

        nlohmann::json proxies;
        for (const auto& [ip, description] : currentProxies) {
            nlohmann::json proxy;
            proxy["host"] = ip;
            proxy["description"] = description;
            proxies.push_back(proxy);
        }

        PostJSONToURL("/api/proxies", proxies);
    } else {
        auto currentProxies = GetProxyList();
        std::vector<std::string> newProxies;

        for (const auto& it : outputManager->GetControllers()) {
            auto c = dynamic_cast<ControllerEthernet*>(it);
            if (c != nullptr) {
                std::string proxy_ip = ip_utils::ResolveIP(c->GetFPPProxy());
                if (ipAddress.compare(proxy_ip) == 0) {
                    auto controllerip = c->GetIP();
                    if (std::find(currentProxies.begin(), currentProxies.end(), controllerip) == currentProxies.end()) {
                        newProxies.push_back(controllerip);
                        currentProxies.push_back(controllerip);
                    }
                }
            }
        }

        for (const auto& nprox : newProxies) {
            PostToURL("/api/proxies/" + nprox, "", "text/plain");
        }
    }
    return false;
}
#endif

#define FPP_CTRL_PORT 32320
static void ProcessFPPSysinfo(Discovery &discovery, const std::string &ip, const std::string &proxyIp, const std::string &sysInfo);

static bool resolvableHostname(const std::string &hn, const std::string &ip) {
    if (ip_utils::IsValidHostname(hn)) {
        return ip == ip_utils::ResolveIP(hn);
    }
    return false;
}
static void setRangesToChannelCount(DiscoveredData *inst) {
    int min = 9999999; int max = 0;
    if (inst->ranges != "") {
        auto const r1 = Split(inst->ranges, ',');
        for (auto const& a : r1) {
            auto const r = Split(a, '-');
            int start = std::stoi(r[0]);
            int len = 4; //at least 4
            if (r.size() == 2) {
                len = std::stoi(r[1]) - start + 1;
            }
            min = std::min(min, start);
            max = std::max(max, start + len - 1);
        }
    }
    int count = max - min + 1;
    if (count < 512) {
        count = 512;
    }

    // This will create universes if E131 or just set DDP
    std::list<Model*> models;
    inst->controller->SetChannelSize(count, models, 512);
}

static void SetControllerType(DiscoveredData *inst) {
    if (inst->pixelControllerType != "") {
        std::string v, m, var;
        Controller::ConvertOldTypeToVendorModel(inst->pixelControllerType, v, m, var);
        if (v != "") {
            inst->SetVendor(v);
        }
        if (m != "") {
            inst->SetModel(m);
        }
        if (var != "") {
            inst->SetVariant(var);
        }
        ControllerCaps *caps = inst->controller->GetControllerCaps();
        if (caps != nullptr && caps->SupportsAutoLayout()) {
            inst->controller->SetAutoLayout(true);
            inst->controller->SetAutoSize(true, nullptr);
        }
        if (caps != nullptr && caps->SupportsAutoUpload()) {
            inst->controller->SetAutoUpload(true);
        }
    }
}
static void CreateController(Discovery &discovery, DiscoveredData *inst) {
    bool created = false;
    if (inst->controller == nullptr) {
        inst->controller = new ControllerEthernet(discovery.GetOutputManager(), false);
        if (resolvableHostname(inst->ip, inst->hostname)) {
            inst->controller->SetIP(inst->hostname);
        } else {
            inst->controller->SetIP(inst->ip);
        }
        if (inst->hostname != "") {
            inst->controller->SetName(inst->hostname);
        }
        created = true;
    }
    if (inst->typeId < 0x80) {
        if (inst->controller->GetProtocol() != OUTPUT_DDP) {
            inst->controller->SetProtocol(OUTPUT_DDP);
        }
        SetControllerType(inst);
    } else if (inst->typeId == 0xC2 || inst->typeId == 0xC3) {
        if (inst->controller->GetProtocol() != OUTPUT_DDP) {
            inst->controller->SetProtocol(OUTPUT_DDP);
        }
        dynamic_cast<DDPOutput*>(inst->controller->GetOutputs().front())->SetKeepChannelNumber(false);
        if (inst->majorVersion <= 3) {
            inst->pixelControllerType = inst->platformModel;
        } else if (inst->typeId == 0xC2) {
            inst->pixelControllerType = "ESP8266";
        } else if (inst->typeId == 0xC3) {
            inst->pixelControllerType = "ESP32";
        }
        SetControllerType(inst);
    } else if (inst->typeId >= 0xA0 && inst->typeId <= 0xAF) {
        //Experience Lights
        if (inst->controller->GetProtocol() != OUTPUT_DDP) {
            inst->controller->SetProtocol(OUTPUT_DDP);
        }
        inst->pixelControllerType = inst->platformModel;
        SetControllerType(inst);
    } else if (inst->typeId >= 0x80 && inst->typeId <= 0x8F) {
        //falcon range
        if (created) {
            if (inst->mode == "bridge") {
                inst->controller->SetProtocol(OUTPUT_E131);
            } else {
                inst->controller->SetProtocol(OUTPUT_DDP);
            }
            inst->SetVendor("Falcon");
            inst->SetModel(AfterLast(inst->platformModel, ' '));
            inst->controller->SetAutoLayout(true);
            inst->controller->SetAutoSize(true, nullptr);
        }
    }
    setRangesToChannelCount(inst);
}

static void ProcessFPPSystems(Discovery &discovery, const std::string &systemsString) {
    nlohmann::json origJson;

    try {
        origJson = nlohmann::json::parse(systemsString, nullptr, false);
    } catch (...) {
        origJson = nlohmann::json::value_t::discarded;
    }
    if (origJson.is_discarded()) {
        return;
    }
    
    std::string IPKey = "IP";
    std::string PlatformKey = "Platform";
    std::string HostNameKey = "HostName";
    std::string ModeStringKey = "fppMode";
    if (origJson.contains("systems")) {
        IPKey = "address";
        PlatformKey = "type";
        HostNameKey = "hostname";
        ModeStringKey = "fppModeString";
    }
    nlohmann::json systems = origJson.contains("systems") ? origJson["systems"] : origJson;

    for (int x = 0; x < systems.size(); x++) {
        nlohmann::json &system = systems[x];
        std::string address = GetJSONStringValue(system, IPKey);
        std::string hostName = system[HostNameKey].is_null() ? "" : GetJSONStringValue(system, HostNameKey);
        std::string uuid = system.contains("uuid") ? GetJSONStringValue(system, "uuid") : GetJSONStringValue(system, "UUID");
        
        //logger_base.info("Processing ip: %s   host: %s    uuid: %s", address.c_str(), hostName.c_str(), uuid.c_str());
        if (!uuid.empty()) {
            fppDiscInfo.insert({ hostName, address, uuid });
        }
        if (address == "null" || hostName == "null") {
            continue;
        }
        if (address.length() > 16) {
            //ignore for some reason, FPP is occasionally returning an IPV6 address
            continue;
        }
        DiscoveredData *found = discovery.FindByUUID(uuid, address);
        if (found == nullptr) {
            found = discovery.FindByIp(address, hostName);
        }
        DiscoveredData inst;
        inst.hostname = hostName;
        inst.uuid = uuid;
        if (!system[PlatformKey].is_null()) {
            inst.platform = GetJSONStringValue(system, PlatformKey);
        }

        if (!system["model"].is_null()) {
            inst.platformModel = GetJSONStringValue(system, "model");
        }
        inst.ip = address;
        if (!system["version"].is_null()) {
            inst.version = GetJSONStringValue(system, "version");
            if (inst.version.size() > 3 && (inst.version[3] == '-' || inst.version[3] == '.')) {
                inst.patchVersion = wxAtoi(inst.version.substr(4));
            }
        }
        inst.minorVersion = GetJSONIntValue(system, "minorVersion", inst.minorVersion);
        inst.majorVersion = GetJSONIntValue(system, "majorVersion", inst.majorVersion);
        inst.typeId = GetJSONIntValue(system, "typeId", inst.typeId);
        if (!system["channelRanges"].is_null()) {
            inst.ranges = GetJSONStringValue(system, "channelRanges");
        }
        if (!system["HostDescription"].is_null()) {
            inst.description = GetJSONStringValue(system, "HostDescription");
        }
        if (!system[ModeStringKey].is_null()) {
            inst.mode = GetJSONStringValue(system, ModeStringKey);
            if (inst.mode == "player" && GetJSONBoolValue(system, "multisync")) {
                inst.mode += " w/multisync";
            }
        }
        if (inst.typeId > 0x80) {
            inst.pixelControllerType = inst.platformModel;
        }

        if (found) {
            if (found->majorVersion == 0) {
                found->hostname = inst.hostname;
                found->platform = inst.platform;
                found->platformModel = inst.platformModel;
                found->version = inst.version;
                found->majorVersion = inst.majorVersion;
                found->minorVersion = inst.minorVersion;
                found->patchVersion = inst.patchVersion;
                found->description = inst.description;
                found->ranges = inst.ranges;
                found->mode = inst.mode;
                found->typeId = inst.typeId;
                found->uuid = inst.uuid;
            } else {
                if (found->platform.empty()) {
                    found->platform = inst.platform;
                }
                if (found->mode.empty()) {
                    found->mode = inst.mode;
                }
                if (found->platformModel.empty()) {
                    found->platformModel = inst.platformModel;
                }
                if (found->typeId == 0) {
                    found->typeId = inst.typeId;
                }
                if (found->uuid.empty()) {
                    found->uuid = inst.uuid;
                }
                if (inst.ranges.size() > found->ranges.size()) {
                    //if the json has the ranges, use it as the json can have a more exact set of ranges
                    //the Ping packet is limited to either 40 (v2) or 120 (v3) characters so
                    //the range may be squashed a bit.   The json has no limit so can have the
                    //full set of range definitions
                    found->ranges = inst.ranges;
                }
            }
        } else {
            found = discovery.FindByIp(address, hostName, true);
            found->extraData["httpConnected"] = false;
            found->hostname = inst.hostname;
            found->platform = inst.platform;
            found->platformModel = inst.platformModel;
            found->version = inst.version;
            found->majorVersion = inst.majorVersion;
            found->minorVersion = inst.minorVersion;
            found->patchVersion = inst.patchVersion;
            found->description = inst.description;
            found->ranges = inst.ranges;
            found->mode = inst.mode;
            found->typeId = inst.typeId;
            found->uuid = inst.uuid;

            std::string ipAddr = inst.ip;
            CreateController(discovery, found);
            if (found->typeId > 0 && found->typeId < 0x80) {
                discovery.AddCurl(ipAddr, "/api/fppd/multiSyncSystems", [&discovery, found, ipAddr] (int rc, const std::string &buffer, const std::string &err) {
                    if (rc == 200) {
                        found->extraData["httpConnected"] = true;
                        ProcessFPPSystems(discovery, buffer);
                    }
                    return true;
                });
                discovery.AddCurl(ipAddr, "/api/system/info", [&discovery, ipAddr, found] (int rc, const std::string &buffer, const std::string &err) {
                    if (rc == 200) {
                        found->extraData["httpConnected"] = true;
                        ProcessFPPSysinfo(discovery, ipAddr, "", buffer);
                    }
                    return true;
                });
            } else if (found->typeId >= 0x80) {
                discovery.AddCurl(ipAddr, "/", [&discovery, ipAddr, found](int rc, const std::string &buffer, const std::string &err) {
                    if (rc == 200 && buffer != "") {
                        found->extraData["httpConnected"] = true;
                        discovery.DetectControllerType(ipAddr, "", buffer);
                    }
                    return true;
                });
            }
       }
   }
}
static void ProcessFPPProxies(Discovery &discovery, const std::string &ip, const std::string &proxies) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    nlohmann::json origJson;
    try {
        origJson = nlohmann::json::parse(proxies, nullptr, false);
    } catch (...) {
        origJson = nlohmann::json::value_t::discarded;
    }
    if (origJson.is_discarded()) {
        return;
    }
    DiscoveredData *ipinst = discovery.FindByIp(ip, "", true);
    ipinst->extraData["httpConnected"] = true;
    for (int x = 0; x < origJson.size(); x++) {
        std::string proxy;
        if (origJson[x].is_string()) {
            proxy = (origJson[x].get<std::string>());
        } else if (origJson[x].is_object()) { // FPP 8 change
            proxy = GetJSONStringValue(origJson[x], "host");
        }
        DiscoveredData *inst = discovery.FindByIp(proxy, "", true);
        if (!inst->extraData.contains("httpConnected")) {
            inst->extraData["httpConnected"] = false;
        }
        inst->SetProxy(ip);
        inst->hostname = "";
        inst->username = ipinst->username;
        inst->password = ipinst->password;
        discovery.AddCurl(ip, "/proxy/" + proxy + "/", [&discovery, proxy, ip, inst](int rc, const std::string &buffer, const std::string &err) {
            if (rc == 200 && buffer.find("Falcon Player - FPP") != std::string::npos) {
                std::string p = proxy;
                std::string i = ip;
                inst->extraData["httpConnected"] = true;

                logger_base.info("Found proxied instance ip: %s     proxyip: %s", proxy.c_str(), ip.c_str());
                discovery.AddCurl(ip, "/proxy/" + proxy + "/api/system/info", [&discovery, p, i](int rc, const std::string &buffer, const std::string &err) {
                    if (rc == 200) {
                        ProcessFPPSysinfo(discovery, p, i, buffer);
                    }
                    return true;
                });
            } else {
                discovery.DetectControllerType(proxy, ip, buffer);
            }
            return true;
        });
    }
}

static void ProcessFPPChannelOutput(Discovery &discovery, const std::string &ip, const std::string &outputs) {

    nlohmann::json val;
    try {
        val = nlohmann::json::parse(outputs, nullptr, false);
    } catch (...) {
        val = nlohmann::json::value_t::discarded;
    }
    if (val.is_discarded()) {
        return;
    }
    DiscoveredData *inst = discovery.FindByIp(ip, "", true);
    inst->extraData["httpConnected"] = true;
    for (int x = 0; x < val["channelOutputs"].size(); x++) {
        if (val["channelOutputs"][x]["enabled"].get<int>()) {
            std::string outputType = GetJSONStringValue(val["channelOutputs"][x], "type");
            if (outputType == "RPIWS281X"|| outputType == "BBB48String" ||
                outputType == "BBShiftString" || outputType == "DPIPixels") {
                inst->pixelControllerType = GetJSONStringValue(val["channelOutputs"][x], "subType");
            } else if (outputType == "LEDPanelMatrix") {
                if (inst->pixelControllerType.empty()) {
                    inst->pixelControllerType = LEDPANELS;
                }
                int pw = GetJSONIntValue(val["channelOutputs"][x], "panelWidth");
                int ph = GetJSONIntValue(val["channelOutputs"][x], "panelHeight");
                int nw = 0; int nh = 0;
                bool tall = false;
                for (int p = 0; p < val["channelOutputs"][x]["panels"].size(); ++p) {
                    int r = GetJSONIntValue(val["channelOutputs"][x]["panels"][p], "row");
                    int c = GetJSONIntValue(val["channelOutputs"][x]["panels"][p], "col");
                    nw = std::max(c, nw);
                    nh = std::max(r, nh);
                    std::string orientation = GetJSONStringValue(val["channelOutputs"][x]["panels"][p], "orientation");
                    if (orientation == "E" || orientation == "W") {
                        tall = true;
                    }
                }
                nw++; nh++;
                if (tall) {
                    std::swap(pw, ph);
                }
                inst->panelSize = std::to_string(pw * nw);
                inst->panelSize.append("x");
                inst->panelSize.append(std::to_string(ph * nh));
            } else if (outputType == "VirtualMatrix") {
                if (inst->pixelControllerType.empty()) {
                    inst->pixelControllerType = "Virtual Matrix";
                }
            }
        }
    }
    SetControllerType(inst);
}
static void ProcessFPPSysinfo(Discovery &discovery, const std::string &ip, const std::string &proxy, const std::string &sysInfo) {
    nlohmann::json val;
    try {
        val = nlohmann::json::parse(sysInfo, nullptr, false);
    } catch (...) {
        val = nlohmann::json::value_t::discarded;
    }
    if (val.is_discarded()) {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Could not parse sysinfo for %s(%s)", ip.c_str(), proxy.c_str());
        DiscoveredData* inst = discovery.FindByIp(ip, "", true);
        inst->extraData["httpConnected"] = false;
        if (proxy != "") {
            inst->SetProxy(proxy);
        }
        return;
    }
    
    std::string uuid = GetJSONStringValue(val, "uuid");
    if (uuid.empty()) {
        uuid = GetJSONStringValue(val, "UUID");
    }
    if (uuid.empty()) {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Could not process sysinfo for %s(%s). No UUID found.", ip.c_str(), proxy.c_str());
        DiscoveredData* inst = discovery.FindByIp(ip, "", true);
        inst->extraData["httpConnected"] = false;
        if (proxy != "") {
            inst->SetProxy(proxy);
        }
        return;
    }
    

    DiscoveredData *inst = discovery.FindByUUID(uuid, ip);
    if (inst == nullptr) {
        inst = discovery.FindByIp(ip, "", true);
    }
    inst->extraData["httpConnected"] = true;
    if (proxy != "") {
        inst->SetProxy(proxy);
    }
        
    inst->platform = GetJSONStringValue(val, "Platform");
    inst->platformModel = GetJSONStringValue(val, "Variant");
    inst->version = GetJSONStringValue(val, "Version");
    inst->hostname = GetJSONStringValue(val, "HostName");
    inst->description = GetJSONStringValue(val, "HostDescription");
    inst->mode = GetJSONStringValue(val, "Mode");
    if (inst->mode == "player" && GetJSONBoolValue(val, "multisync")) {
        inst->mode += " w/multisync";
    }
    inst->uuid = uuid;
    if (inst->typeId == 0 && val.contains("typId") && val["typeId"].is_number_integer()) {
        inst->typeId = val["typeId"].get<int>();
    }
    inst->canZipUpload = val.contains("zip");
    if (inst->version != "") {
        inst->majorVersion = wxAtoi(inst->version);
        if (inst->version[2] == 'x') {
            inst->minorVersion = wxAtoi(inst->version.substr(4)) + 1000;
        } else {
            inst->minorVersion = wxAtoi(inst->version.substr(2));
        }
        if (inst->version.size() > 3 && (inst->version[3] == '-' || inst->version[3] == '.')) {
            inst->patchVersion = wxAtoi(inst->version.substr(4));
        }
    }
    std::string r = GetJSONStringValue(val, "channelRanges");
    if (r.size() > inst->ranges.size()) {
        inst->ranges = r;
    }
    inst->minorVersion =  GetJSONIntValue(val, "minorVersion", inst->minorVersion);
    inst->majorVersion =  GetJSONIntValue(val, "majorVersion", inst->majorVersion);
    if (val.contains("capeInfo")) {
        inst->pixelControllerType = GetJSONStringValue(val["capeInfo"], "id");
    }

    std::string file = "co-pixelStrings";
    if (inst->platform.find("Beagle") != std::string::npos) {
        file = "co-bbbStrings";
    }
    std::string baseUrl;
    std::string host = inst->ip;
    std::string baseIp = inst->ip;
    if (inst->proxy != "") {
        baseIp = inst->proxy;
        baseUrl = "/proxy/" + inst->ip;
    }
    discovery.AddCurl(baseIp, baseUrl + "/api/channel/output/" + file,
                        [&discovery, host] (int rc, const std::string &buffer, const std::string &err) {
        if (rc == 200) {
            ProcessFPPChannelOutput(discovery, host, buffer);
        }
        return true;
    });
    discovery.AddCurl(baseIp, baseUrl + "/api/channel/output/channelOutputsJSON",
                        [&discovery, host] (int rc, const std::string &buffer, const std::string &err) {
        if (rc == 200) {
            ProcessFPPChannelOutput(discovery, host, buffer);
        }
        return true;
    });
    discovery.AddCurl(baseIp, baseUrl + "/api/channel/output/co-other",
                        [&discovery, host] (int rc, const std::string &buffer, const std::string &err) {
        if (rc == 200) {
            ProcessFPPChannelOutput(discovery, host, buffer);
        }
        return true;
    });
    discovery.AddCurl(baseIp, baseUrl + "/api/playlists",
                        [&discovery, host, inst] (int rc, const std::string &buffer, const std::string &err) {
        if (rc == 200) {
            try {
                nlohmann::json val = nlohmann::json::parse(buffer, nullptr, false);
                if (!val.is_discarded()) {
                    inst->extraData["playlists"] = val;
                }
            } catch (...) {
            }
        }
        return true;
    });
    discovery.AddCurl(baseIp, baseUrl + "/api/cape",
                        [&discovery, host, inst, baseUrl, baseIp] (int rc, const std::string &buffer, const std::string &err) {
        if (rc == 200) {
            try {
                nlohmann::json val = nlohmann::json::parse(buffer, nullptr, false);
                if (!val.is_discarded()) {
                    inst->extraData["cape"] = val;
                }
            } catch (...) {
            }
        }
        return true;
    });
    if (inst->proxy.empty()) {
        discovery.AddCurl(baseIp, "/api/proxies",
                            [&discovery, host] (int rc, const std::string &buffer, const std::string &err) {
            if (rc == 200) {
                ProcessFPPProxies(discovery, host, buffer);
            }
            return true;
        });
    }
}


static void ProcessFPPPingPacket(Discovery &discovery, uint8_t *buffer,int len) {
    if (buffer[0] == 'F' && buffer[1] == 'P' && buffer[2] == 'P' && buffer[3] == 'D' && buffer[4] == 0x04) {
        char ip[64];
        snprintf(ip, sizeof(ip), "%d.%d.%d.%d", (int)buffer[15], (int)buffer[16], (int)buffer[17], (int)buffer[18]);
        //printf("Ping %s\n", ip);
        if (strcmp(ip, "0.0.0.0")) {
            //static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            //logger_base.info("FPP Discovery - Received Ping response from %s", ip);
            AddTraceMessage("Received UDP result " + std::string(ip));

            //we found a system!!!
            std::string hostname = (char *)&buffer[19];
            std::string ipStr = ip;
            DiscoveredData *inst = discovery.FindByIp(ip, hostname, false);

            //int platform = buffer[9];
            //printf("%d: %s  %s     %d\n", found ? 1 : 0, hostname.c_str(), ipStr.c_str(), platform);
            if (!inst) {
                inst = discovery.FindByIp(ip, hostname, true);

                if (buffer[9] < 0x80) {
                    std::string ipAddr = ip;
                    discovery.AddCurl(ipAddr, "/api/fppd/multiSyncSystems", [&discovery] (int rc, const std::string &buffer, const std::string &err) {
                        if (rc == 200) {
                            ProcessFPPSystems(discovery, buffer);
                        }
                        return true;
                    });
                    discovery.AddCurl(ipAddr, "/api/system/info", [&discovery, ipAddr] (int rc, const std::string &buffer, const std::string &err) {
                        if (rc == 200) {
                            ProcessFPPSysinfo(discovery, ipAddr, "", buffer);
                        }
                        return true;
                    });
                }
            }
            if (inst->typeId == 0) {
                inst->typeId = buffer[9];
            }
            if (inst->hostname.empty()) {
                inst->hostname = (char *)&buffer[19];
            }
            if (inst->platformModel.empty()) {
                inst->platformModel = (char *)&buffer[125];
            }
            if (inst->platform.empty()) {
                inst->platform = (char *)&buffer[125];
            }
            if (inst->ip.empty()) {
                inst->ip = ip;
            }
            if (inst->version.empty()) {
                inst->version = (char *)&buffer[84];
            }
            if (inst->minorVersion == 0) {
                inst->minorVersion = buffer[13] + (buffer[12] << 8);
            }
            if (inst->majorVersion == 0) {
                inst->majorVersion = buffer[11] + (buffer[10] << 8);
            }
            switch (buffer[14]) {
                case 1:
                    inst->mode = "bridge";
                    break;
                case 2:
                    inst->mode = "player";
                    break;
                case 4:
                case 6:
                    inst->mode = "master";
                    break;
                case 8:
                    inst->mode = "remote";
                    break;
            }
            std::string rgn = (char*)&buffer[166];;
            if (rgn != "0-0" && inst->ranges.size() < rgn.size()) {
                inst->ranges = rgn;
            }
            CreateController(discovery, inst);
        }
    }
}
void FPP::PrepareDiscovery(Discovery &discovery) {
    std::list<std::string> startAddresses;
    wxConfigBase* config = wxConfigBase::Get();
    wxString force;
    if (config->Read("FPPConnectForcedIPs", &force)) {
        wxArrayString ips = wxSplit(force, '|');
        for (auto& a : ips) {
            if (a != "") {
                startAddresses.push_back(ToUTF8(a));
            }
        }
    }
    for (auto &it : discovery.GetOutputManager()->GetControllers()) {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr) {
            if (eth->GetIP() != "") {
                startAddresses.push_back(eth->GetIP());
            }
            if (eth->GetFPPProxy() != "") {
                startAddresses.push_back(eth->GetFPPProxy());
            }
        }
    }
    PrepareDiscovery(discovery, startAddresses);
}

void FPP::PrepareDiscovery(Discovery &discovery, const std::list<std::string> &addresses, bool broadcastPing) {
    uint8_t buffer[512] = { 'F', 'P', 'P', 'D', 0x04};
    buffer[5] = 207-7;
    buffer[7] = 2; //v2 ping
    buffer[8] = 1; //discovery
    buffer[9] = 0xC0;

    wxString ver = xlights_version_string;
    auto parts = wxSplit(ver, '.');
    int maj = wxAtoi(parts[0]);
    int min = wxAtoi(parts[1]);

    buffer[10] = (maj >> 8) & 0xFF;
    buffer[11] = maj & 0xFF;
    buffer[12] = 0;
    buffer[13] = min;

    buffer[14] = 0; // MODE?!?!?

    //Technically, the IP address but since we aren't actually an FPP instance,
    //we don't want anyone trying to contact us, so we'll set to 0
    buffer[15] = buffer[16] = buffer[17] = buffer[18] = 0;
    strcpy((char *)&buffer[84], ver.c_str());

    for (const auto &a : addresses) {
        discovery.AddCurl(a, "/api/fppd/multiSyncSystems", [a, &discovery] (int rc, const std::string &buffer, const std::string &err) {
            if (rc == 200) {
                ProcessFPPSystems(discovery, buffer);
            } else if (rc == 404) {
                discovery.AddCurl(a, "/", [a, &discovery] (int rc, const std::string &buffer, const std::string &errorBuffer) {
                    if (rc == 200) {
                        discovery.DetectControllerType(a, "", buffer);
                    }
                    return true;
                });
            }
            return true;
        });
        discovery.AddCurl(a, "/api/system/info", [&discovery, a](int rc, const std::string &buffer, const std::string &err) {
            if (rc == 200) {
                ProcessFPPSysinfo(discovery, a, "", buffer);
            }
            return true;
        });
    }
    discovery.AddCurl("localhost", "/api/system/info", [&discovery](int rc, const std::string &buffer, const std::string &err) {
        if (rc == 200) {
            ProcessFPPSysinfo(discovery, "localhost", "", buffer);
        }
        return true;
    });
    discovery.AddCurl("localhost", "/api/fppd/multiSyncSystems", [&discovery] (int rc, const std::string &buffer, const std::string &err) {
        if (rc == 200) {
            ProcessFPPSystems(discovery, buffer);
        }
        return true;
    });

    discovery.AddMulticast("239.70.80.80", FPP_CTRL_PORT, [&discovery](wxDatagramSocket* socket, uint8_t *buffer, int len) {
        ProcessFPPPingPacket(discovery, buffer, len);
    });
    if (broadcastPing) {
        discovery.SendBroadcastData(FPP_CTRL_PORT, buffer, 207);
        discovery.SendData(FPP_CTRL_PORT, "239.70.80.80", buffer, 207);
    }
    for (auto & a : addresses) {
        // go ahead and send a unicast ping as well
        discovery.SendData(FPP_CTRL_PORT, a, buffer, 207);
    }
    discovery.AddBonjour("_fppd._udp", [&](const std::string &ip) {
        discovery.AddCurl(ip, "/api/fppd/multiSyncSystems", [&discovery] (int rc, const std::string &buffer, const std::string &err) {
            if (rc == 200) {
                ProcessFPPSystems(discovery, buffer);
            }
            return true;
        });
        discovery.AddCurl(ip, "/api/system/info", [&discovery, ip](int rc, const std::string &buffer, const std::string &err) {
            if (rc == 200) {
                ProcessFPPSysinfo(discovery, ip, "", buffer);
            }
            return true;
        });
    });
}
bool FPP::supportedForFPPConnect() const {
    if (this->IsVersionAtLeast(7, 1)) {
        return true;
    }
    if (this->IsVersionAtLeast(6, 3, 3)) {
        if (capeInfo.contains("verifiedKeyId")) {
            return true;
        }
    }
    return false;
}

static bool supportedForFPPConnect(DiscoveredData* res, OutputManager* outputManager) {
    if (res->typeId == 0) {
        return false;
    }
    if (res->typeId < 0x80) {
        if (res->extraData.contains("httpConnected") && res->extraData["httpConnected"].get<bool>() == true) {
            // genuine FPP instance and able to connect via http
            return true;
        } else {
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.info("FPP Discovery - Skipping %s no http connection", (const char *)res->ip.c_str());
            return false;
        }
    }

    if ((res->typeId >= 0xC2) && (res->typeId <= 0xC3)) {
        if (res->ranges.empty()) {
            auto c = outputManager->GetControllers(res->ip);
            if (c.size() == 1) {
                ControllerEthernet *controller = dynamic_cast<ControllerEthernet*>(c.front());
                if (controller) {
                    uint32_t sc = controller->GetStartChannel() - 1;
                    res->ranges = std::to_string(sc) + "-" + std::to_string(sc + controller->GetChannels()-1);
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
        return res->majorVersion >= 4 && res->mode == "remote";
    }

    if (res->typeId >= 0x88 && res->typeId <= 0x9F) {
        // F16V4 / F48V4 / F16V5 / F32V5 /  F48V5
        return res->mode != "bridge";
    }

    if (res->typeId >= 0xA0 && res->typeId <= 0xAF) {
        // Genius
        return res->mode != "bridge";
    }

    return false;
}

inline void setIfEmpty(std::string &val, const std::string &nv) {
    if (val.empty()) {
        val = nv;
    }
}
inline void setIfEmpty(uint32_t &val, uint32_t nv) {
    if (val == 0) {
        val = nv;
    }
}

void FPP::MapToFPPInstances(Discovery& discovery, std::list<FPP*>& instances, OutputManager* outputManager) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool foundActiveController = false;
    uint16_t activePlayerCount = 0;
    std::unordered_set<std::string> allProxyList;
    std::map<std::string, std::string> configuredIPs;
    if (discovery.GetOutputManager()->GetGlobalFPPProxy() != "") {
        auto ip = ip_utils::ResolveIP(discovery.GetOutputManager()->GetGlobalFPPProxy());
        allProxyList.insert(ip);
    };
    for (auto& it : discovery.GetOutputManager()->GetControllers()) {
        auto c = dynamic_cast<ControllerEthernet*>(it);
        if (c != nullptr) {
            configuredIPs[c->GetResolvedIP()] = c->GetIP();
            if (!c->GetFPPProxy().empty()) {
                auto ip = ip_utils::ResolveIP(c->GetFPPProxy());
                allProxyList.insert(ip);
            }
            if (Controller::DecodeActiveState(c->GetActive()) == "Active") {
                foundActiveController = true;
            }
        }
    }
    logger_base.info("----------- FPP Discovery Results ------------");
    for (auto res : discovery.GetResults()) {
        bool http = (res->extraData.contains("httpConnected") && res->extraData["httpConnected"].get<bool>() == true);
        logger_base.info("   Instance: %s (uuid: %s)(hn: %s)(proxy: %s)(ver: %s)(http: %s)(t: %X)", res->ip.c_str(), res->uuid.c_str(), res->hostname.c_str(), res->proxy.c_str(), res->version.c_str(), http ? "true" : "false", res->typeId);
    }
    logger_base.info("----------------------------------------------");
    for (auto res : discovery.GetResults()) {
        if (::supportedForFPPConnect(res, outputManager)) {
            logger_base.info("FPP Discovery - Found Supported FPP Instance: %s (u: %s)(h: %s)(p: %s)(r: %s)", res->ip.c_str(), res->uuid.c_str(), res->hostname.c_str(), res->proxy.c_str(), res->ranges.c_str());
            FPP *fpp = nullptr;
            bool skipit = false;

            for (auto f : instances) {
                if (f->ipAddress == res->ip) {
                    fpp = f;
                }
                if (!res->uuid.empty() && f->uuid == res->uuid) {
                    if (configuredIPs.count(res->ip) > 0) {
                        logger_base.info("FPP Discovery - Found Configured IP - %s for the same UUID - %s. Going to use this instead.", res->ip.c_str(), res->uuid.c_str());
                        fpp = f;
                    } else {
                        skipit = true;
                    }
                }
            }

            if (!skipit && fpp == nullptr) {
                FPP *fpp = new FPP(res->ip, res->proxy, res->pixelControllerType);
                fpp->ipAddress = res->ip;//not needed, in constructor
                fpp->hostName = res->hostname;
                fpp->uuid = res->uuid;
                fpp->description = res->description;
                fpp->platform = res->platform;
                fpp->model = res->platformModel;
                fpp->majorVersion = res->majorVersion;
                fpp->minorVersion = res->minorVersion;
                fpp->patchVersion = res->patchVersion;
                fpp->fullVersion = res->version;
                fpp->ranges = res->ranges;
                fpp->mode = res->mode;
                fpp->pixelControllerType = res->pixelControllerType;//not needed, in constructor
                fpp->panelSize = res->panelSize;
                fpp->username = res->username;
                fpp->password = res->password;
                fpp->controllerVendor = res->vendor;
                fpp->controllerModel = res->model;
                fpp->controllerVariant = res->variant;
                TypeIDtoControllerType(res->typeId, fpp);
                if (res->extraData.contains("playlists")) {
                    for (int x = 0; x < res->extraData["playlists"].size(); x++) {
                        fpp->playlists.push_back(res->extraData["playlists"][x].get<std::string>());
                    }
                }
                if (res->extraData.contains("cape")) {
                    fpp->capeInfo = res->extraData["cape"];
                }
                auto it = configuredIPs.find(res->ip);
                if (it != configuredIPs.end()) {
                    if (allProxyList.count(it->first) > 0 || allProxyList.count(it->second) > 0) {
                        fpp->isaProxy = true;
                    }
                }
                if (StartsWith(res->mode, "player")) {
                    activePlayerCount++;
                };
                fpp->canZipUpload = res->canZipUpload;
                instances.push_back(fpp);
            } else if (!skipit) {
                fpp->ipAddress = res->ip;
                if (fpp->proxy().empty()) {
                    fpp->proxy() = res->proxy;
                }
                setIfEmpty(fpp->hostName, res->hostname);
                setIfEmpty(fpp->uuid, res->uuid);
                setIfEmpty(fpp->description, res->description);
                setIfEmpty(fpp->platform, res->platform);
                setIfEmpty(fpp->model, res->platformModel);
                setIfEmpty(fpp->fullVersion, res->version);
                setIfEmpty(fpp->mode, res->mode);
                setIfEmpty(fpp->pixelControllerType, res->pixelControllerType);
                setIfEmpty(fpp->ranges, res->ranges);
                setIfEmpty(fpp->panelSize, res->panelSize);
                setIfEmpty(fpp->username, res->username);
                setIfEmpty(fpp->password, res->password);
                setIfEmpty(fpp->controllerVendor, res->vendor);
                setIfEmpty(fpp->controllerModel, res->model);
                setIfEmpty(fpp->controllerVariant, res->variant);
                setIfEmpty(fpp->minorVersion, res->minorVersion);
                setIfEmpty(fpp->patchVersion, res->patchVersion);
                setIfEmpty(fpp->majorVersion, res->majorVersion);
                TypeIDtoControllerType(res->typeId, fpp);
                if (fpp->playlists.empty() && res->extraData.contains("playlists")) {
                    for (int x = 0; x < res->extraData["playlists"].size(); x++) {
                        fpp->playlists.push_back(res->extraData["playlists"][x].get<std::string>());
                    }
                }
                if (res->extraData.contains("cape")) {
                    fpp->capeInfo = res->extraData["cape"];
                }
                fpp->canZipUpload = res->canZipUpload;
            }
        } else {
            logger_base.info("FPP Discovery - %s is not a supported FPP Instance", res->ip.c_str());
        }
    }
    for (auto f : instances) {
        if (activePlayerCount == 1 && StartsWith(f->mode, "player")) {
            f->solePlayer = true;
        }
    }
}

void FPP::TypeIDtoControllerType(int typeId, FPP* inst) {
    if (typeId < 0x80) {
        inst->fppType = FPP_TYPE::FPP;
    } else if (typeId >= 0x88 && typeId <= 0x9F) {
        inst->fppType = FPP_TYPE::FALCONV4V5;
    } else if (typeId == 0xC2 || typeId == 0xC3) {
        inst->fppType = FPP_TYPE::ESPIXELSTICK;
    } else if (typeId >= 0xA0 && typeId <= 0xAF) {
        inst->fppType = FPP_TYPE::GENIUS;
    }
}

std::vector<std::string> FPP::GetProxyList() {
    auto proxies = GetProxies();
    std::vector<std::string> keys;
    std::transform(proxies.begin(), proxies.end(), std::back_inserter(keys),
                   [](auto const& host) { return std::get<0>(host); });
    return keys;
}

std::vector<std::tuple<std::string, std::string>> FPP::GetProxies() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::vector<std::tuple<std::string, std::string>> res;

    if (IsConnected()) {
        nlohmann::json val;
        if (GetURLAsJSON("/api/proxies", val)) {
            for (int x = 0; x < val.size(); x++) {
                if (val[x].is_string()) {
                    logger_base.debug("FPP %s proxies %s.", (const char*)ipAddress.c_str(), (const char*)val[x].get<std::string>().c_str());
                    res.push_back({ (val[x].get<std::string>()), std::string() });
                } else if (val[x].is_object()) { // FPP 8 change
                    logger_base.debug("FPP %s proxies %s.", (const char*)ipAddress.c_str(), (const char*)val[x]["host"].get<std::string>().c_str());
                    res.push_back({ (val[x]["host"].get<std::string>()), (val[x]["description"].get<std::string>()) });
                }
            }
        }
    }

    return res;
}

// returns true if proxy FPP is available and the to address is in its proxy table
bool FPP::ValidateProxy(const std::string& to, const std::string& via)
{
    FPP fpp(via);
    if (fpp.IsConnected()) {
        for (const auto& it : fpp.GetProxyList()) {
            if (to == it) return true;
        }
    }
    return false;
}

class xlDiscoveryWaitTimer : public wxTimer {
public:
    std::list<Discovery *> discs;
    
    virtual void Notify() override {
        if (!CurlManager::INSTANCE.processCurls()) {
            for (auto d : discs) {
                delete d;
            }
            discs.clear();
            Stop();
        }
    }
};
static void waitForCurlsComplete(Discovery *discovery) {
    // close the resources that we can, but don't block
    // on it as we can delay till later
    discovery->Close(false);
    
    // if there are still http requests outstanding, we'll
    // need to delay the delete
    if (CurlManager::INSTANCE.processCurls()) {
        static xlDiscoveryWaitTimer discTimer;
        discTimer.discs.push_back(discovery);
        discTimer.Start(500);
    } else {
        delete discovery;
    }
}

std::list<FPP*> FPP::GetInstances(wxWindow* frame, OutputManager* outputManager)
{
    std::list<FPP*> instances;

    std::list<std::string> startAddresses;
    std::list<std::string> startAddressesForced;

    wxConfigBase* config = wxConfigBase::Get();
    wxString force;
    if (config->Read("FPPConnectForcedIPs", &force)) {
        wxArrayString ips = wxSplit(force, '|');
        for (const auto& a : ips) {
            if (!a.empty()) {
                startAddresses.push_back(ToUTF8(a));
                startAddressesForced.push_back(ToUTF8(a));
            }
        }
    }
    // add existing controller IP's to the discovery, helps speed up
    // discovery as well as makes it more reliable to discover those,
    // particularly if on a different subnet.   This also helps
    // make sure actually configured controllers are found
    // so the FPP Connect dialog is more likely to
    // have the entire list allowing the uploads to then entire
    // show network to be easier to do
    for (auto& it : outputManager->GetControllers()) {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr && !eth->GetIP().empty() && eth->GetIP() != "MULTICAST") {
            std::string resolvedIP = eth->GetResolvedIP(true);
            if (resolvedIP.empty()) {
                startAddresses.push_back(::Lower(eth->GetIP()));
            } else {
                // only add the instances where we were actually able to resolve an IP address
                if (eth->IsActive() && (ip_utils::IsIPValid(resolvedIP) || resolvedIP != eth->GetIP())) {
                    startAddresses.push_back(::Lower(resolvedIP));
                }
            }
            if (!eth->GetFPPProxy().empty()) {
                startAddresses.push_back(::Lower(ip_utils::ResolveIP(eth->GetFPPProxy())));
            }
        }
    }

    startAddresses.sort();
    startAddresses.unique();

    Discovery *discovery = new Discovery(frame, outputManager);
    FPP::PrepareDiscovery(*discovery, startAddresses);
    discovery->Discover();
    FPP::MapToFPPInstances(*discovery, instances, outputManager);
    instances.sort(sortByIP);

    wxString newForce = "";
    for (const auto& a : startAddressesForced) {
        for (const auto& fpp : instances) {
            if (case_insensitive_match(a, fpp->hostName) || case_insensitive_match(a, fpp->ipAddress)) {
                if (!newForce.empty()) {
                    newForce.append(",");
                }
                newForce.append(a);
            }
        }
    }
    if (newForce != force) {
        config->Write("FPPConnectForcedIPs", newForce);
        config->Flush();
    }
    waitForCurlsComplete(discovery);
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("FPP Discovery Complete.  Found " + std::to_string(instances.size()) + " instances.");
    return instances;
}


ReceiverType FPP::DecodeReceiverType(const std::string& type, bool supportsV5) {
    if (type.find("v1") != std::string::npos) {
        return ReceiverType::v1;
    }
    if (type.find("v2") != std::string::npos) {
        return ReceiverType::v2;
    }
    if (type.find("v5") != std::string::npos) {
        if (supportsV5) {
            return ReceiverType::FalconV5;
        } else {
            return ReceiverType::v2;
        }
    }
    return ReceiverType::Standard;
}

ReceiverType FPP::DecodeReceiverType(int type, bool supportsV5) {
    if (9 < type && supportsV5) {
        return ReceiverType::FalconV5;
    }
    if (3 < type) {
        return ReceiverType::v2;
    }
    if (0 < type) {
        return ReceiverType::v1;
    }
    return ReceiverType::Standard;
}
