/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
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

#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"

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


FPP::FPP(const std::string& ad) :
    BaseController(ad, ""), majorVersion(0), minorVersion(0), patchVersion(0), outputFile(nullptr), parent(nullptr), ipAddress(ad), fppType(FPP_TYPE::FPP) {
    wxIPV4address address;
    if (address.Hostname(ad)) {
        hostName = ad;
        ipAddress = ToUTF8(address.IPAddress());
        _ip = ipAddress;

    }
    _connected = true; // well not really but i need to fake it
}


FPP::FPP(const std::string& ip_, const std::string& proxy_, const std::string& model_) :
    BaseController(ip_, proxy_), majorVersion(0), minorVersion(0), patchVersion(0), outputFile(nullptr), parent(nullptr),
    fppType(FPP_TYPE::FPP), proxy(proxy_), pixelControllerType(model_)
{
    ipAddress = ip_;
    wxIPV4address address;
    if (address.Hostname(ipAddress)) {
        hostName = ipAddress;
        ipAddress = ToStdString(address.IPAddress());
        _ip = ipAddress;
    }
    _connected = true; // well not really but i need to fake it
}

FPP::FPP(const FPP &c)
    : majorVersion(c.majorVersion), minorVersion(c.minorVersion), patchVersion(c.patchVersion), outputFile(nullptr), parent(nullptr), hostName(c.hostName), description(c.description), ipAddress(c.ipAddress), fullVersion(c.fullVersion), platform(c.platform),
    model(c.model), ranges(c.ranges), mode(c.mode), pixelControllerType(c.pixelControllerType), username(c.username), password(c.password), 
    fppType(c.fppType), proxy(c.proxy), capeInfo(c.capeInfo) {

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

CURL *FPP::setupCurl(const std::string &url, bool isGet, int timeout) {
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, BaseController::writeFunction);
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

    std::string fullUrl = ipAddress + url;
    std::string ipAddForGet = ipAddress;
    if (fppType == FPP_TYPE::ESPIXELSTICK) {
        fullUrl = ipAddress + "/fpp?path=" +  url;
    }
    if (!_fppProxy.empty()) {
        fullUrl = "http://" + _fppProxy + "/proxy/" + fullUrl;
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
        if (password == "" && xlPasswordEntryDialog::GetStoredPasswordForService(ipAddress, username, password)) {
            if (password != "") {
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
int FPP::PostToURL(const std::string& url, const std::vector<uint8_t> &val, const std::string &contentType) {
    return TransferToURL(url, val, contentType, true);
}
int FPP::PutToURL(const std::string& url, const std::vector<uint8_t> &val, const std::string &contentType) {
    return TransferToURL(url, val, contentType, false);
}
int FPP::TransferToURL(const std::string& url, const std::vector<uint8_t> &val, const std::string &contentType, bool isPost) {

    std::string fullUrl = ipAddress + url;
    std::string ipAddForGet = ipAddress;
    if (fppType == FPP_TYPE::ESPIXELSTICK) {
        fullUrl = ipAddress + "/fpp?path=" +  url;
    }
    if (!_fppProxy.empty()) {
        fullUrl = "http://" + _fppProxy + "/proxy/" + fullUrl;
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

bool FPP::GetURLAsJSON(const std::string& url, wxJSONValue& val, bool recordError) {
    std::string sval;
    if (GetURLAsString(url, sval, recordError)) {
        wxJSONReader reader;
        reader.Parse(sval, &val);
        return true;
    }
    return false;
}

std::map<int, int> FPP::GetExpansionPorts(ControllerCaps* caps) const
{
    std::map<int, int> res;

    int ports = wxAtoi(caps->GetCustomPropertyByPath("fpp", "0"));

    for (int i = 1; i <= ports; i++)
    {
        auto s = caps->GetCustomPropertyByPath(ToUTF8(wxString::Format("fpp%d", i)), "0,0");
        if (s != "0,0")
        {
            auto ss = wxSplit(s, ',');
            if (ss.size() == 2)
            {
                res[wxAtoi(ss[0])] = wxAtoi(ss[1]);
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
            wxJSONValue val;
            if (GetURLAsJSON("/api/system/info", val)) {
                sysInfoLoaded = true;
                return fppType == FPP_TYPE::FPP && parseSysInfo(val);
            } else if (GetURLAsJSON("/fppjson.php?command=getSysInfo&simple", val)) {
                sysInfoLoaded = true;
                return fppType == FPP_TYPE::FPP && parseSysInfo(val);
            }
        }
        return false;
    }
    return fppType == FPP_TYPE::FPP;
}
bool FPP::parseSysInfo(wxJSONValue& val) {
    platform = ToStdString(val["Platform"].AsString());
    model = ToStdString(val["Variant"].AsString());
    fullVersion = ToStdString(val["Version"].AsString());
    hostName = ToStdString(val["HostName"].AsString());
    description = ToStdString(val["HostDescription"].AsString());
    mode = ToStdString(val["Mode"].AsString());
    if (mode == "player" && val.HasMember("multisync") && val["multisync"].AsBool()) {
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
    if (val.HasMember("channelRanges")) {
        std::string r = val["channelRanges"].AsString().ToStdString();
        if (r.size() > ranges.size()) {
            ranges = r;
        }
    }
    if (val.HasMember("minorVersion")) {
        minorVersion = val["minorVersion"].AsInt();
    }
    if (val.HasMember("majorVersion")) {
        majorVersion = val["majorVersion"].AsInt();
    }
    return true;
}

bool FPP::IsDDPInputEnabled() {
    if (!IsDrive()) {
        wxJSONValue origRoot;
        if (GetURLAsJSON("/api/configfile/ci-universes.json", origRoot, false)) {
            if (origRoot.HasMember("channelInputs") && origRoot.ItemAt("channelInputs").HasMember(0)
                && origRoot.ItemAt("channelInputs").ItemAt(0).HasMember("enabled") &&
                origRoot.ItemAt("channelInputs").ItemAt(0).ItemAt("enabled").AsInt() == 1) {
                return true;
            }
        }
    }
    return false;
}

void FPP::probePixelControllerType() {
    std::string file = "co-pixelStrings";
    if (platform.find("Beagle") != std::string::npos) {
        file = "co-bbbStrings";
    }
    wxJSONValue val;
    if (GetURLAsJSON("/api/channel/output/" + file, val)) {
        parseControllerType(val);
    }
}
void FPP::parseProxies(wxJSONValue& val) {
    for (int x = 0; x < val.Size(); x++) {
        proxies.emplace(val[x].AsString());
    }
}
void FPP::parseControllerType(wxJSONValue& val) {
    for (int x = 0; x < val["channelOutputs"].Size(); x++) {
        if (val["channelOutputs"][x]["enabled"].AsInt()) {
            if (val["channelOutputs"][x]["type"].AsString() == "RPIWS281X"||
                val["channelOutputs"][x]["type"].AsString() == "BBB48String" ||
                val["channelOutputs"][x]["type"].AsString() == "BBShiftString" ||
                val["channelOutputs"][x]["type"].AsString() == "DPIPixels") {
                pixelControllerType = ToUTF8(val["channelOutputs"][x]["subType"].AsString());
            } else if (val["channelOutputs"][x]["type"].AsString() == "LEDPanelMatrix") {
                pixelControllerType = LEDPANELS;
                int pw = val["channelOutputs"][x]["panelWidth"].AsInt();
                int ph = val["channelOutputs"][x]["panelHeight"].AsInt();
                int nw = 0; int nh = 0;
                bool tall = false;
                for (int p = 0; p < val["channelOutputs"][x]["panels"].Size(); ++p) {
                    int r = val["channelOutputs"][x]["panels"][p]["row"].AsInt();
                    int c = val["channelOutputs"][x]["panels"][p]["col"].AsInt();
                    nw = std::max(c, nw);
                    nh = std::max(r, nh);
                    wxString orientation = val["channelOutputs"][x]["panels"][p]["orientation"].AsString();
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
            } else if (val["channelOutputs"][x]["type"].AsString() == "VirtualMatrix") {
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


bool FPP::IsDrive() {
    return ipAddress.find("/") != std::string::npos || ipAddress.find("\\") != std::string::npos;
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
static inline void addString(std::vector<uint8_t> &buffer, const wxString &str) {
    addString(buffer, ToUTF8(str));
}
static inline void addString(wxMemoryBuffer &buffer, const char *str) {
    buffer.AppendData(str, strlen(str));
}
static inline void addString(wxMemoryBuffer &buffer, const std::string &str) {
    buffer.AppendData(str.c_str(), str.length());
}

bool FPP::GetPathAsJSON(const std::string &path, wxJSONValue &val) {
    wxFileName fn;
    fn = ToWXString(path);
    if (FileExists(fn)) {
        wxJSONReader reader;
        wxFile tf(fn.GetFullPath());
        wxString json;
        tf.ReadAll(&json);
        reader.Parse(json, &val);
        tf.Close();
        return true;
    }
    return false;
}
bool FPP::WriteJSONToPath(const std::string& path, const wxJSONValue& val) {
    wxFileName fn;
    fn = ToWXString(path);
    wxFileOutputStream ufile(fn.GetFullPath());
    wxJSONWriter writer(wxJSONWRITER_STYLED, 0, 3);
    writer.Write(val, ufile);
    ufile.Close();
    return true;
}

int FPP::PostJSONToURL(const std::string& url, const wxJSONValue& val) {
    wxString str;
    wxJSONWriter writer(wxJSONWRITER_STYLED, 0, 3);
    writer.Write(val, str);
    std::vector<uint8_t> memBuffPost;
    addString(memBuffPost, str);
    return PostToURL(url, memBuffPost, "application/json");
}
int FPP::PostJSONToURLAsFormData(const std::string& url, const std::string &extra, const wxJSONValue& val) {
    std::vector<uint8_t> memBuffPost;
    addString(memBuffPost, extra);
    addString(memBuffPost, "&data={");
    wxString str;
    wxJSONWriter writer(wxJSONWRITER_STYLED, 0, 3);
    writer.Write(val, str);
    addString(memBuffPost, str);
    addString(memBuffPost, "}");
    return PostToURL(url, memBuffPost, "application/x-www-form-urlencoded; charset=UTF-8");
}

void FPP::DumpJSON(const wxJSONValue& json)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString str;
    wxJSONWriter writer(wxJSONWRITER_STYLED, 0, 3);
    writer.Write(json, str);
    logger_base.debug(ToUTF8(str));
}

int FPP::PostToURL(const std::string& url, const std::string &val, const std::string &contentType) {
    std::vector<uint8_t> memBuffPost;
    addString(memBuffPost, val);
    return PostToURL(url, memBuffPost, contentType);
}
int FPP::PutToURL(const std::string& url, const std::string &val, const std::string &contentType) {
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


    std::string fullUrl = ipAddress + "/jqupload.php";
    bool usingJqUpload = true;
    bool usingMove = true;
    if (fppType == FPP_TYPE::ESPIXELSTICK) {
        fullUrl = ipAddress + "/fpp?path=uploadFile&filename=" + URLEncode(filename);
        usingJqUpload = false;
        usingMove = false;
    } else if (IsVersionAtLeast(6, 3, 3)) {
        fullUrl = ipAddress + "/api/file/uploads/" + URLEncode(filename);
        usingJqUpload = false;
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
    const std::string bound = "----WebKitFormBoundaryb29a7c2fe47b9481";
    struct curl_slist *chunk = nullptr;
    std::string ctMime = "Content-Type: multipart/form-data; boundary=" + bound;
    if (!usingJqUpload) {
        ctMime = "Content-Type: application/octet-stream";
    } else {
        chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
    }
    chunk = curl_slist_append(chunk, ctMime.c_str());
    chunk = curl_slist_append(chunk, "X-Requested-With: FPPConnect");
    chunk = curl_slist_append(chunk, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    FPPWriteData *data = new FPPWriteData();
    if (usingJqUpload) {
        addString(data->memBuffPost, "\r\n--");
        addString(data->memBuffPost, bound);
        addString(data->memBuffPost,"\r\nContent-Disposition: form-data; name=\"\"\r\n\r\nundefined\r\n--");
        addString(data->memBuffPost, bound);
        addString(data->memBuffPost,"\r\nContent-Disposition: form-data; name=\"\"\r\n\r\nundefined\r\n--");
        addString(data->memBuffPost, bound);
        addString(data->memBuffPost, "--\r\n");

        std::string cd = "Content-Disposition: form-data; name=\"myfile\"; filename=\"";
        cd += fn.ToStdString();
        cd += "\"\r\n";
        addString(data->memBuffPre, "--");
        addString(data->memBuffPre, bound);
        addString(data->memBuffPre, "\r\n");
        addString(data->memBuffPre, cd);
        addString(data->memBuffPre, ct);
        addString(data->memBuffPre, "\r\n\r\n");
    }

    data->realFile.Open(fullFileName);
    if (!usingJqUpload) {
        std::string cl = "Content-Length: " + std::to_string(data->realFile.Length());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data->realFile.Length());
        chunk = curl_slist_append(chunk, cl.c_str());
    }
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
        } else {
            messages.push_back("ERROR Uploading file: " + utfFilename + "     Response Code: " + std::to_string(response_code));
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
    if (IsVersionAtLeast(6, 3, 2)) {
        return GetURLAsString("/api/file/move/" + URLEncode(filename), val);
    }
    //api/file/move is broken on older versions of FPP (doesn't decode filename properly), use fppxml
    return GetURLAsString("/fppxml.php?command=moveFile&file=" + URLEncode(filename), val);
}

struct V7ProgressStruct {
    wxFile in;
    FPP *instance;
    size_t length;

    size_t offset = 0;
    int lastPct = 0;
    
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
    
    constexpr uint64_t BLOCK_SIZE = 64*1024*1024;
    CurlManager::CurlPrivateData *cpd = nullptr;
    CURL *curl = CurlManager::INSTANCE.createCurl(ps->fullUrl, &cpd, true);

    //if we cannot upload it in 5 minutes, we have serious issues
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000*5*60);

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    headers = curl_slist_append(headers, "X-Requested-With: FPPConnect");

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
        ps->instance->messages.push_back("ERROR Uploading file: " + ps->filename + "     Could not read source file.");
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
        logger_curl.info("    FPPConnect CURL Callbak - URL: %s    Response: %d", ps->fullUrl.c_str(), response_code);
        ps->offset += remaining;
        uint64_t pct = (ps->offset * 1000) / ps->length;
        bool cancelled = ps->instance->updateProgress(pct, false);
        if (cancelled || ps->offset >= ps->length) {
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

bool FPP::copyFile(const std::string &filename,
                   const std::string &file,
                   const std::string &dir) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool cancelled = false;
    cancelled |= updateProgress(0, true);

    wxFile in;
    in.Open(ToWXString(file));

    if (in.IsOpened()) {
        wxFile out;
        wxString target = ipAddress +  wxFileName::GetPathSeparator() + dir +  wxFileName::GetPathSeparator() + filename;
        out.Open(target, wxFile::write);

        if (out.IsOpened()) {
            wxFileOffset length = in.Length();
            wxFileOffset done = 0;

            uint8_t buffer[8192]; // 8KB at a time
            while (!in.Eof() && !cancelled) {
                size_t read = in.Read(&buffer[0], sizeof(buffer));
                size_t written = out.Write(&buffer[0], read);
                while (written < read) {
                    written += out.Write(&buffer[written], read - written);
                }
                done += read;

                int prgs = done * 1000 / length;
                cancelled |= updateProgress(prgs, true);
            }
            cancelled |= updateProgress(1000, true);
            in.Close();
            out.Close();
        } else {
            cancelled |= updateProgress(1000, true);
            logger_base.warn("   Copy of file %s failed ... target file %s could not be opened.", (const char *)file.c_str(), (const char *)target.c_str());
        }
    } else {
        cancelled |= updateProgress(1000, true);
        logger_base.warn("   Copy of file %s failed ... file could not be opened.", (const char *)file.c_str());
    }
    return cancelled;
}

bool FPP::uploadOrCopyFile(const std::string &filename,
                           const std::string &file,
                           const std::string &dir) {
    if (IsDrive()) {
        return copyFile(filename, file, dir);
    }
    if (IsVersionAtLeast(6, 3, 2)) {
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
        wxFileName mfn(FromUTF8(media));
        std::string mediaFile = media;
        mediaBaseName = ToUTF8(mfn.GetFullName());

        if (majorVersion >= 6) {
            FindHostSpecificMedia(hostName, mediaBaseName, mediaFile, mfn);
        }

        bool doMediaUpload = true;
        wxJSONValue currentMeta;
        if (GetURLAsJSON("/api/media/" + URLEncode(mediaBaseName) + "/meta", currentMeta, false)) {
            if (currentMeta.HasMember("format") && currentMeta["format"].HasMember("size") &&
                (mfn.GetSize() == std::atoi(currentMeta["format"]["size"].AsString().c_str()))) {
                doMediaUpload = false;
            }
        }
        if (doMediaUpload) {
            std::string dir = "music";
            for (auto &a : FPP_VIDEO_EXT) {
                if (mfn.GetExt() == a) {
                    dir = "videos";
                }
            }
            cancelled |= uploadOrCopyFile(mediaBaseName, mediaFile, dir);
        }
        if (cancelled) {
            return cancelled;
        }
    }
    sequences[baseName].sequence = baseName;
    sequences[baseName].media = mediaBaseName;
    sequences[baseName].duration = ((float)(file->getStepTime() * file->getNumFrames())) / 1000.0f;

    std::string fileName;
    if (IsDrive()) {
        fileName = ipAddress + GetPathSeparator() + "sequences" + GetPathSeparator() + baseName;
    } else {
        tempFileName = ToStdString(wxFileName::CreateTempFileName(ToWXString(baseName)));
        fileName = tempFileName;
    }

    FSEQFile::CompressionType ctype = ::FSEQFile::CompressionType::zstd;
    if (type == 3 || type == 4) {
        ctype = ::FSEQFile::CompressionType::none;
    } else if (type == 5 || type == 6) {
        ctype = ::FSEQFile::CompressionType::zlib;
    }

    bool doSeqUpload = true;
    int currentMaxChannel = 0;
    int currentChannelCount = 0;
    std::vector<std::pair<uint32_t, uint32_t>> currentRanges;
    std::vector<std::pair<uint32_t, uint32_t>> newRanges;
    if (!IsDrive() && fppType == FPP_TYPE::FPP) {
        wxJSONValue currentMeta;
        if (GetURLAsJSON("/api/sequence/" + URLEncode(baseName) + "/meta", currentMeta, false)) {
            doSeqUpload = false;
            char buf[24];
            sprintf(buf, "%" PRIu64, file->getUniqueId());
            wxString version = currentMeta["Version"].AsString();
            if (type == 0 && version[0] != '1') doSeqUpload = true;
            if (type != 0 && version[0] == '1') doSeqUpload = true;
            int currentCompression = 1;
            if (version[0] == '1') {
                currentCompression = 0;
            }
            if (currentMeta.HasMember("CompressionType")) {
                currentCompression = currentMeta["CompressionType"].AsLong();
            }
            if ((type == 2 || type == 1) && currentCompression != 1) {
                doSeqUpload = true;
            }
            if ((type == 0 || type == 3) && currentCompression != 0) {
                doSeqUpload = true;
            }
            if (currentMeta["ID"].AsString() != buf) doSeqUpload = true;
            if (currentMeta["NumFrames"].AsLong() != file->getNumFrames()) doSeqUpload = true;
            if (currentMeta["StepTime"].AsLong() != file->getStepTime()) doSeqUpload = true;

            currentMaxChannel = currentMeta["MaxChannel"].AsLong();
            currentChannelCount = currentMeta["ChannelCount"].AsLong();
            if (currentMeta.HasMember("Ranges")) {
                for (int x = 0; x < currentMeta["Ranges"].Size(); x++) {
                    int s = currentMeta["Ranges"][x]["Start"].AsLong();
                    int l = currentMeta["Ranges"][x]["Length"].AsLong();
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

static bool PlaylistContainsEntry(wxJSONValue &pl, const std::string &media, const std::string &seq) {
    for (int x = 0; x < pl.Size(); x++) {
        wxJSONValue entry = pl[x];
        if (seq == ToStdString(entry["sequenceName"].AsString())) {
            if (media == "") {
                if (entry["type"].AsString() == wxString("sequence")) {
                    return true;
                }
            } else if (entry["type"].AsString() == "both") {
                if (media == ToStdString(entry["mediaName"].AsString())) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool FPP::UploadPlaylist(const std::string &name) {
    wxJSONValue origJson;
    std::string fn;
    if (IsDrive()) {
        fn = (ipAddress + GetPathSeparator() + "playlists" +GetPathSeparator() + name + ".json");
        GetPathAsJSON(fn, origJson);
    } else {
        GetURLAsJSON("/api/playlist/" + URLEncode(name), origJson, false);
    }

    for (const auto& info : sequences) {
        if (!PlaylistContainsEntry(origJson["mainPlaylist"], info.second.media, info.first)) {
            wxJSONValue entry;
            if (info.second.media != "") {
                entry["type"] = wxString("both");
                entry["enabled"] = 1;
                entry["playOnce"] = 0;
                entry["sequenceName"] = info.first;
                entry["mediaName"] = info.second.media;
                entry["videoOut"] = wxString("--Default--");
                entry["duration"] = info.second.duration;
            } else {
                entry["type"] = wxString("sequence");
                entry["enabled"] = 1;
                entry["playOnce"] = 0;
                entry["sequenceName"] = info.first;
                entry["duration"] = info.second.duration;
            }
            origJson["mainPlaylist"].Append(entry);
        }
    }
    origJson.Remove(wxString("playlistInfo"));
    origJson["name"] = name;

    if (IsDrive()) {
        WriteJSONToPath(fn, origJson);
    } else {
        PostJSONToURL("/api/playlist/" + URLEncode(name), origJson);
    }
    return false;
}

bool FPP::UploadModels(const wxJSONValue &models) {
    if (IsDrive()) {
        std::string fn = (ipAddress +GetPathSeparator() + "config" +GetPathSeparator() + "model-overlays.json");
        WriteJSONToPath(fn, models);
    } else {
        PostJSONToURL("/api/models", models);
    }
    return false;
}

bool FPP::UploadDisplayMap(const std::string &displayMap) {
    if (IsDrive()) {
        wxFileName fn = (ipAddress + wxFileName::GetPathSeparator() + "config/virtualdisplaymap");
        wxFile tf(fn.GetFullPath());
        tf.Write(displayMap);
        tf.Close();
    } else {
        PostToURL("/api/configfile/virtualdisplaymap", displayMap);
    }
    return false;
}

bool FPP::UploadUDPOut(const wxJSONValue &udp) {
    if (IsDrive()) {
        std::string fn = ipAddress +GetPathSeparator() + "config" +GetPathSeparator() + "co-universes.json";
        WriteJSONToPath(fn, udp);
    } else {
        wxJSONValue orig;
        wxJSONValue newudp = udp;
        
        if (GetURLAsJSON("/api/channel/output/universeOutputs", orig)) {
            if (orig.HasMember("channelOutputs")) {
                for (int x = 0; x < orig["channelOutputs"].Size(); x++) {
                    if (orig["channelOutputs"][x]["type"].AsString() == "universes" && orig["channelOutputs"][x].HasMember("interface")) {
                        newudp["channelOutputs"][0]["interface"] = orig["channelOutputs"][x]["interface"].AsString();
                    }
                }
            }
        }
        PostJSONToURL("/api/channel/output/universeOutputs", newudp);
    }
    return false;
}

wxJSONValue FPP::CreateModelMemoryMap(ModelManager* allmodels, int32_t startChan, int32_t endChannel) {
    wxJSONValue json;
    wxJSONValue models;
    std::vector<wxString> names;
    for (const auto& m : *allmodels) {
        Model* model = m.second;

        if (model->GetDisplayAs() == "ModelGroup") {
            continue;
        }

        wxString stch = model->GetModelXml()->GetAttribute("StartChannel", wxString::Format("%d?", model->NodeStartChannel(0) + 1)); //NOTE: value coming from model is probably not what is wanted, so show the base ch# instead
        int ch = model->GetNumberFromChannelString(model->ModelStartChannel);
        if (ch < startChan || ch > endChannel) {
            continue;
        }

        wxString name(model->name);
        name.Replace(" ", "_");


        int numStr = model->GetNumStrings();
        if (numStr == 0) {
            numStr = 1;
        }
        int straPerStr =  model->GetNumStrands() / numStr;
        if (straPerStr < 1) straPerStr = 1;

        if (model->GetDisplayAs() == "Custom") {
            straPerStr = 1;
        }

        wxJSONValue jm;
        jm["Name"] = name;
        jm["ChannelCount"] = model->GetActChanCount();
        jm["StartChannel"] = ch;
        jm["ChannelCountPerNode"] = model->GetChanCountPerNode();

        MatrixModel *mm = dynamic_cast<MatrixModel*>(model);
        if (mm) {
            if (mm->isVerticalMatrix()) {
                jm["Orientation"] = wxString("vertical");
            } else {
                jm["Orientation"] = wxString("horizontal");
            }
        } else if (model->GetDisplayAs() == "Custom") {
            straPerStr = 1;
            numStr = 1;
            jm["Orientation"] = wxString("custom");
            CustomModel *cm = dynamic_cast<CustomModel*>(model);
            jm["data"] = cm->GetCustomData();
        } else {
            jm["Orientation"] = wxString("horizontal");
        }
        jm["StringCount"] = numStr;
        jm["StrandsPerString"] = straPerStr;
        std::string corner = model->GetIsBtoT() ? "B" : "T";
        corner += model->GetIsLtoR() ? "L" : "R";
        jm["StartCorner"] = corner;

        if (IsVersionAtLeast(6, 0)) {
            jm["Type"] = wxString("Channel");
        }
        names.emplace_back(name);
        models.Append(jm);
    }

    wxJSONValue ogModelJSON;
    if (GetURLAsJSON("/api/models", ogModelJSON)) {
        auto ogModels = ogModelJSON.AsArray();
        for (size_t i = 0; i < ogModels->Count(); i++) {
            if (!ogModels->Item(i).HasMember("Name") ) {
                continue;
            }
            if (!ogModels->Item(i)["Name"].IsString()) {
                continue;
            }
            auto ogName = ogModels->Item(i)["Name"].AsString();

            if (ogModels->Item(i).HasMember("autoCreated") && ogModels->Item(i)["autoCreated"].IsBool()) {
                auto wasAutoCreated = ogModels->Item(i)["autoCreated"].AsBool();
                if (wasAutoCreated) {
                    continue;
                }
            }

            if (ogModels->Item(i).HasMember("StartChannel") && ogModels->Item(i)["StartChannel"].IsInt32()) {
                auto ogStartChan = ogModels->Item(i)["StartChannel"].AsInt32();
                if (ogStartChan < startChan || ogStartChan > endChannel ) {
                    continue;
                }
            }
            
            if (std::find(names.cbegin(), names.cend(), ogName) != names.end()) { // only add if name doesn't exist
                continue;
            }
            models.Append(ogModels->Item(i));
        }
    }

    json["models"] = models;
    return json;
}

static bool Compare3dPointTuple(const std::tuple<float, float, float, int> &l,
                                const std::tuple<float, float, float, int> &r) {
    return std::get<2>(l) < std::get<2>(r);
}

std::string FPP::CreateVirtualDisplayMap(ModelManager* allmodels) {
    std::string ret;

    constexpr float PADDING{ 10.0F };
    bool first { true };
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

        if (first) {
            first = false;
            maxY = model->GetModelScreenLocation().previewH;
            maxX = model->GetModelScreenLocation().previewW;
        }

        minY = std::min(model->GetModelScreenLocation().GetBottom() - PADDING, minY);
        maxY = std::max(model->GetModelScreenLocation().GetTop() + PADDING, maxY);
        minX = std::min(model->GetModelScreenLocation().GetLeft() - PADDING, minX);
        maxX = std::max(model->GetModelScreenLocation().GetRight() + PADDING, maxX);
    }

    ret += "# Preview Size\n";
    ret += ToUTF8(wxString::Format("%d,%d\n", int(maxX - minX), int(maxY - minY)));

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
        for (auto [x,y,z, ch] : modelPts) {
            ret += ToUTF8(wxString::Format("%d,%d,%d,%d,%d,%s\n",
                (int)std::round(x), (int)std::round(y), (int)std::round(z), ch,
                model->GetChanCountPerNode(), stringType.c_str()));
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
        wxArrayString r1 = wxSplit(wxString(ranges), ',');
        for (const auto& a : r1) {
            wxArrayString r = wxSplit(a, '-');
            int start = wxAtoi(r[0]);
            int len = 4; //at least 4
            if (r.size() == 2) {
                len = wxAtoi(r[1]) - start + 1;
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

    wxJSONValue f = CreateUniverseFile(selected, false);

    std::map<int, int> rng;
    FillRanges(rng);
    for (int x = 0; x < f["channelOutputs"][0]["universes"].Size(); x++) {
        wxJSONValue u = f["channelOutputs"][0]["universes"][x];
        int start = u["startChannel"].AsLong() - 1;
        int len = u["channelCount"].AsLong();
        rng[start] = len;
    }
    SetNewRanges(rng);

    return UploadUDPOut(f);
}

wxJSONValue FPP::CreateUniverseFile(Controller* controller, bool input) {
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
    SetInputUniversesBridge(controller);
    
    if (restartNeeded) {
        Restart();
    }
    return b;
}

wxJSONValue FPP::CreateUniverseFile(const std::list<Controller*>& selected, bool input, std::map<int, int> *rngs) {
    wxJSONValue root;
    root["type"] = wxString("universes");
    root["enabled"] = 1;
    root["timeout"] = 1000;
    root["startChannel"] = 1;
    root["channelCount"] = -1;

    wxJSONValue universes;

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

            wxJSONValue universe;
            if (!input) {
                universe["active"] = controllerEnabled == Controller::ACTIVESTATE::ACTIVE ? 1 : 0;
            } else {
                universe["active"] = 1;
            }
            universe["description"] = stripInvalidChars(it2->GetName());
            universe["id"] = it->GetUniverse();
            universe["startChannel"] = c;
            universe["channelCount"] = it->GetChannels();
            universe["address"] = wxString("");
            universe["priority"] = 0;
            universe["deDuplicate"] = eth->IsSuppressDuplicateFrames() ? 1 : 0;
            universe["monitor"] = eth->IsMonitoring() ? 1 : 0;

            if (rngs && it->GetChannels() > 0 && controllerEnabled == Controller::ACTIVESTATE::ACTIVE) {
                (*rngs)[c] = c + it->GetChannels() - 1;
            }

            if (it->GetType() == OUTPUT_E131) {
                universe["type"] = (int)(it->GetIP() != "MULTICAST" ? 1 : 0);
                if (!input && (it->GetIP() != "MULTICAST")) {
                    universe["address"] = wxString(it->GetIP());
                }
                if (it->GetIP() == "MULTICAST") {
                    universe["monitor"] = 0;
                }

                E131Output* e131 = dynamic_cast<E131Output*>(it);
                universe["priority"] = e131->GetPriority();

                // TODO this needs work to restore the loading of multiple universes as a single line
                if (allSameSize) {
                    universe["universeCount"] = it2->GetOutputCount();
                    universes.Append(universe);
                    break;
                }
                universe["universeCount"] = 1;

                universes.Append(universe);
            } else if (it->GetType() == OUTPUT_DDP || it->GetType() == OUTPUT_ZCPP) {
                if (!input) {
                    universe["address"] = wxString(it->GetIP());
                    DDPOutput* ddp = dynamic_cast<DDPOutput*>(it);
                    if (ddp) {
                        universe["type"] = ddp->IsKeepChannelNumbers() ? 4 : 5;
                    } else {
                        universe["type"] = 5;
                    }
                    universes.Append(universe);
                } else {
                    //create empty array DDP input
                    universes = wxJSONValue(wxJSONTYPE_ARRAY);
                }
            } else if (it->GetType() == OUTPUT_ARTNET) {
                universe["type"] = (int)((eth->GetIP() != "MULTICAST") + 2);
                if (!input && (it->GetIP() != "MULTICAST")) {
                    universe["address"] = wxString(it->GetIP());
                }
                if (it->GetIP() == "MULTICAST") {
                    universe["monitor"] = 0;
                }
                if (allSameSize) {
                    universe["universeCount"] = it2->GetOutputCount();
                    universes.Append(universe);
                    break;
                }
                //ArtNetOutput* ano = dynamic_cast<ArtNetOutput*>(it);
                universe["universeCount"] = 1;
                universes.Append(universe);
            } else if (it->GetType() == OUTPUT_KINET) {
                KinetOutput* kiNet = dynamic_cast<KinetOutput*>(it);
                universe["address"] = wxString(kiNet->GetIP());
                universe["type"] = kiNet->GetVersion() + 5;
                universes.Append(universe);
            } else if (it->GetType() == OUTPUT_TWINKLY) {
                universe["address"] = wxString(it->GetIP());
                universe["type"] = 8;
                universes.Append(universe);
            }
        }
    }

    root["universes"] = universes;

    wxJSONValue json;
    wxJSONValue chan;
    chan.Append(root);
    if (input) {
        json["channelInputs"] = chan;
    } else {
        json["channelOutputs"] = chan;
    }
    return json;
}

bool FPP::SetRestartFlag() {
    std::string val;
    if (!IsDrive()) {
        restartNeeded = true;
        return PutToURL("/api/settings/restartFlag", "2", "text/plain");
    }
    return false;
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
    if (fppType != FPP_TYPE::FPP || IsDrive()) {
        return;
    }
    wxJSONValue jval;
    int count = 0;
    while (count < 20) {
        if (GetURLAsJSON("/api/system/info", jval, false)) {
            if (jval.HasMember("channelRanges")) {
                std::string r = jval["channelRanges"].AsString().ToStdString();
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
            }
            else {
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
    if (!IsDrive()) {
        PutToURL("/api/settings/HostDescription", st, "text/plain");
    }
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

    wxJSONValue udp = CreateUniverseFile(std::list<Controller*>({ controller }), true);
    if (udp["channelInputs"][0]["universes"].Size() != 0 || forceUpload) {
        if (IsDrive()) {
            std::string fn = (c->GetResolvedIP() +GetPathSeparator() + "config" +GetPathSeparator() + "ci-universes.json");
            WriteJSONToPath(fn, udp);
        } else {
            PostJSONToURL("/api/channel/output/universeInputs", udp);
        }
    }

    return false;
}


static bool UpdateJSONValue(wxJSONValue &v, const std::string &key, int newValue) {
    if (!v.HasMember(key)) {
        v[key] = newValue;
        return true;
    }
    int origValue = v[key].AsLong();
    if (origValue != newValue) {
        v[key] = newValue;
        return true;
    }
    return false;
}
static bool UpdateJSONValue(wxJSONValue &v, const std::string &key, const wxString &newValue) {
    if (!v.HasMember(key)) {
        v[key] = newValue;
        return true;
    }
    wxString origValue = v[key].AsString();
    if (origValue != newValue) {
        v[key] = newValue;
        return true;
    }
    return false;
}

static bool mergeSerialInto(wxJSONValue &otherDmxData, wxJSONValue &otherOrigRoot, bool addDefaults) {
    bool changed = false;
    for (int x = 0; x < otherDmxData["channelOutputs"].Size(); x++) {
        wxString device = otherDmxData["channelOutputs"][x]["device"].AsString();
        wxString type = otherDmxData["channelOutputs"][x]["type"].AsString();
        bool found = false;
        for (int y = 0; y < otherOrigRoot["channelOutputs"].Size(); y++) {
            if (otherOrigRoot["channelOutputs"][y]["device"].AsString() == device) {
                //same device, see if type matches and update or disable
                if (type == otherOrigRoot["channelOutputs"][y]["type"].AsString()) {
                    //device and type the same, update values
                    found = true;
                    changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "description", otherDmxData["channelOutputs"][x]["description"].AsString());
                    changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "enabled", 1);
                    changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "startChannel", otherDmxData["channelOutputs"][x]["startChannel"].AsLong());
                    changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "channelCount", otherDmxData["channelOutputs"][x]["channelCount"].AsLong());

                    if (!addDefaults) {
                        if (type == "Renard") {
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "renardspeed", otherDmxData["channelOutputs"][x]["renardspeed"].AsLong());
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "renardparm", otherDmxData["channelOutputs"][x]["renardparm"].AsString());
                        } else if (type == "LOR") {
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "firstControllerId", otherDmxData["channelOutputs"][x]["firstControllerId"].AsLong());
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "speed", otherDmxData["channelOutputs"][x]["speed"].AsLong());
                        } else if (type == "GenricSerial") {
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "speed", otherDmxData["channelOutputs"][x]["speed"].AsLong());
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "header", otherDmxData["channelOutputs"][x]["header"].AsString());
                            changed |= UpdateJSONValue(otherOrigRoot["channelOutputs"][y], "footer", otherDmxData["channelOutputs"][x]["footer"].AsString());
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
            otherOrigRoot["channelOutputs"].Append(otherDmxData["channelOutputs"][x]);
            changed = true;
        }
    }
    return changed;
}
#endif

bool FPP::IsCompatible(const ControllerCaps *rules,
                       std::string &origVend, std::string &origMod, std::string origVar, const std::string &origId,
                       std::string &driver) {
    if (origMod == "") {
        Controller::ConvertOldTypeToVendorModel(origId, origVend, origMod, origVar);
    }
    if (IsVersionAtLeast(7, 0)) {
        // we can verify that the ID actually can load a pinout
        bool found = false;
        wxJSONValue val;
        wxString id = rules->GetID();
        if (GetURLAsJSON("/api/cape/strings", val)) {
            for (int x = 0; x < val.Size(); x++) {
                if (val[x].AsString() == id) {
                    found = true;
                }
            }
            //certain older capes may have versioned pin config files,
            //we'll need to check them
            if (!found) {
                id = rules->GetID() + "_v2";
                for (int x = 0; x < val.Size(); x++) {
                    if (val[x].AsString() == id) {
                        found = true;
                    }
                }
            }
            if (!found) {
                id = rules->GetID() + "_v3";
                for (int x = 0; x < val.Size(); x++) {
                    if (val[x].AsString() == id) {
                        found = true;
                    }
                }
            }
            if (!found) {
                id = rules->GetID() + "-v2";
                for (int x = 0; x < val.Size(); x++) {
                    if (val[x].AsString() == id) {
                        found = true;
                    }
                }
            }
            if (!found) {
                id = rules->GetID() + "-v3";
                for (int x = 0; x < val.Size(); x++) {
                    if (val[x].AsString() == id) {
                        found = true;
                    }
                }
            }
        }
        if (found) {
            wxJSONValue val;
            if (GetURLAsJSON("/api/cape/strings/" + id, val)) {
                if (val.HasMember("driver")) {
                    driver = val["driver"].AsString();
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

    wxJSONValue origJson;
    bool changed = false;
    int startChannel = -1;

    if (rules->SupportsLEDPanelMatrix() && cud.GetMaxLEDPanelMatrixPort()) {
        startChannel = cud.GetControllerLEDPanelMatrixPort(1)->GetStartChannel();
        startChannel--;
    }
    if (startChannel >= 0 || fullcontrol) {
        if (IsDrive()) {
            GetPathAsJSON(ipAddress +GetPathSeparator() + "config" +GetPathSeparator() + "channeloutputs.json", origJson);
        } else {
            GetURLAsJSON("/api/channel/output/channelOutputsJSON", origJson, false);
        }
    }
    if (startChannel >= 0) {
        std::map<int, int> rngs;
        FillRanges(rngs);
        //LED panel cape, nothing we can really do except update the start channel, and enable
        startChannel++;  //one based
        for (int x = 0; x < origJson["channelOutputs"].Size(); x++) {
            if (origJson["channelOutputs"][x]["type"].AsString() == "LEDPanelMatrix") {
                if (UpdateJSONValue(origJson["channelOutputs"][x], "startChannel", startChannel)) {
                    changed = true;
                    rngs[startChannel - 1] = origJson["channelOutputs"][x]["channelCount"].AsLong();
                }
                changed |= UpdateJSONValue(origJson["channelOutputs"][x], "enabled", 1);
            }
        }
        SetNewRanges(rngs);
    } else if (fullcontrol) {
        //disable
        for (int x = 0; x < origJson["channelOutputs"].Size(); x++) {
            if (origJson["channelOutputs"][x]["type"].AsString() == "LEDPanelMatrix") {
                changed |= UpdateJSONValue(origJson["channelOutputs"][x], "enabled", 0);
            }
        }
    }
    if (changed) {
        if (IsDrive()) {
            WriteJSONToPath(ipAddress +GetPathSeparator() + "config" +GetPathSeparator() + "channeloutputs.json", origJson);
        } else {
            PostJSONToURL("/api/channel/output/channelOutputsJSON", origJson);
            SetRestartFlag();
        }
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
    wxJSONValue origJson;
    if (fullcontrol || (rules->SupportsVirtualMatrix() && cud.GetMaxVirtualMatrixPort())) {
        if (IsDrive()) {
            GetPathAsJSON(ipAddress +GetPathSeparator()  + "config" +GetPathSeparator()  + "co-other.json", origJson);
        } else {
            GetURLAsJSON("/api/channel/output/co-other", origJson, false);
        }
        if (fullcontrol) {
            for (int x = 0; x < origJson["channelOutputs"].Size(); x++) {
                if (origJson["channelOutputs"][x]["type"].AsString() == "VirtualMatrix") {
                    origJson["channelOutputs"].Remove(x);
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
                wxString layout = "";
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
                        layout = wxString::Format("%dx%d", w, h);
                    }
                }

                models[port].insert(name);
                bool found = false;
                for (int x = 0; x < origJson["channelOutputs"].Size(); x++) {
                    if (origJson["channelOutputs"][x]["type"].AsString() == "VirtualMatrix"
                        && origJson["channelOutputs"][x]["description"].AsString() == name) {
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
                    wxJSONValue v;
                    v["enabled"] = 1;
                    v["type"] = wxString("VirtualMatrix");
                    v["startChannel"] = startChannel;
                    v["channelCount"] = m->GetEndChannel() - m->GetStartChannel() + 1;
                    v["width"] = w > 0 ? w : 64;
                    v["height"] = h > 0 ? h : 32;
                    v["layout"] = layout;
                    v["colorOrder"] = wxString("RGB");
                    v["invert"] = 0;
                    v["device"] = wxString::Format("fb%d", port);
                    v["xoff"] = 0;
                    v["description"] = name;
                    v["yoff"] = curOffset;
                    curOffset += h > 0 ? h : 0;
                    if (countModels > 1) {
                        v["scaling"] = wxString("None");
                    } else {
                        v["scaling"] = wxString("Hardware");
                    }
                    origJson["channelOutputs"].Append(v);
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
        for (int x = 0; x < origJson["channelOutputs"].Size(); x++) {
            if (origJson["channelOutputs"][x]["type"].AsString() == "VirtualMatrix") {
                wxString dev = origJson["channelOutputs"][x]["device"].AsString();
                int port = (char)dev[2] - '0';
                if (models[port].find(ToUTF8(origJson["channelOutputs"][x]["description"].AsString())) == models[port].end()) {
                    UpdateJSONValue(origJson["channelOutputs"][x], "enabled", 0);
                }
            }
        }
    }
    if (changed) {
        if (IsDrive()) {
            WriteJSONToPath(ipAddress +GetPathSeparator() + "config" +GetPathSeparator() + "co-other.json", origJson);
        } else {
            PostJSONToURL("/api/channel/output/co-other", origJson);
            SetRestartFlag();
        }
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

    wxJSONValue otherData;
    for (int sp = 1; sp <= rules->GetMaxSerialPort(); sp++) {
        wxJSONValue port;
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
                port["type"] = wxString("DMX-Pro");
                if (mx < 16) {
                    //several controllers have issues if the DMX data stream has less than 16 channels
                    mx = 16;
                }
            } else if (tp == OUTPUT_OPENDMX) {
                port["type"] = wxString("DMX-Open");
                if (mx < 16) {
                    //several controllers have issues if the DMX data stream has less than 16 channels
                    mx = 16;
                }
            } else if (tp == OUTPUT_PIXELNET) {
                port["type"] = wxString("Pixelnet-Lynx");
                mx = 4096;
            } else if (tp == OUTPUT_OPENPIXELNET) {
                port["type"] = wxString("Pixelnet-Open");
                mx = 4096;
            } else if (tp == OUTPUT_LOR) {
                port["type"] = wxString("LOR");
                port["speed"] = controller->GetSpeed();
                port["firstControllerId"] = 1;
            } else if (tp == OUTPUT_RENARD) {
                port["type"] = wxString("Renard");
                port["speed"] = controller->GetSpeed();
                port["renardparm"] = "8N1";
            } else if (tp == OUTPUT_GENERICSERIAL) {
                port["type"] = wxString("GenericSerial");
                port["speed"] = controller->GetSpeed();
                port["header"] = controller->GetSaveablePreFix();
                port["footer"] = controller->GetSaveablePostFix();
            }
            std::string description = controller->GetDescription();
            if (description == "") {
                description = controller->GetName();
            }
            port["description"] = description;
            port["channelCount"] = mx;
            otherData["channelOutputs"].Append(port);

            rngs[sc - 1] = mx;

            wxJSONValue otherOrigRoot = otherData;
            bool changed = true;
            if (IsDrive()) {
                GetPathAsJSON(ipAddress +GetPathSeparator() + "config" +GetPathSeparator() + "co-other.json", otherOrigRoot);
                changed = mergeSerialInto(otherData, otherOrigRoot, false);
            } else {
                if (GetURLAsJSON("/api/configfile/co-other.json", otherOrigRoot, false)) {
                    changed = mergeSerialInto(otherData, otherOrigRoot, false);
                }
            }
            if (changed) {
                if (IsDrive()) {
                    WriteJSONToPath(ipAddress +GetPathSeparator() + "config" +GetPathSeparator() + "co-other.json", otherOrigRoot);
                } else {
                    PostJSONToURL("/api/configfile/co-other.json", otherOrigRoot);
                    SetRestartFlag();
                }
                SetNewRanges(rngs);
            }
        }
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
    if (fppFileName == "") {
        fppFileName = "co-bbbStrings";
    }
    std::string check;
    cud.Check(rules, check);
    cud.Dump();

    wxFileName fnOrig;
    fnOrig.AssignTempFileName("pixelOutputs");
    std::string file = fnOrig.GetFullPath().ToStdString();
    wxJSONValue origJson;
    if (IsDrive()) {
        GetPathAsJSON(ipAddress +GetPathSeparator() + "config" +GetPathSeparator() + fppFileName +".json", origJson);
    } else {
        GetURLAsJSON("/api/channel/output/" + fppFileName, origJson, false);
    }
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

    wxString pinout = "1.x";
    std::map<wxString, wxJSONValue> origStrings;
    std::string origSubType = "";
    if (origJson["channelOutputs"].IsArray()) {
        for (int x = 0; x < origJson["channelOutputs"].Size(); x++) {
            wxJSONValue &f = origJson["channelOutputs"][x];
            if (f.HasMember("pinoutVersion")) {
                pinout = f["pinoutVersion"].AsString();
            }
            if (pinout == "") {
                pinout = "1.x";
            }
            if (f.HasMember("subType")) {
                origSubType = ToUTF8(f["subType"].AsString());
            }
            if (!fullcontrol) {
                for (int o = 0; o < f["outputs"].Size(); o++) {
                    if (f["outputs"][o].HasMember("virtualStrings")) {
                        for (int vs = 0; vs < f["outputs"][o]["virtualStrings"].Size(); vs++) {
                            wxJSONValue val = f["outputs"][o]["virtualStrings"][vs];
                            if (val["description"].AsString() != "") {
                                origStrings[val["description"].AsString()] = val;
                            }
                        }
                    }
                }
            }
        }
    }
    int maxport = 0;

    wxJSONValue stringData;
    stringData["enabled"] = 1;
    stringData["startChannel"] = 1;
    stringData["channelCount"] = -1;

    maxport = cud.GetMaxPixelPort(); // 1 based

    std::string fppDriver = rules->GetCustomPropertyByPath("fppStringDriverType");
    if (fppFileName == "co-bbbStrings") {
        if (!IsCompatible(rules, controllerVendor, controllerModel, controllerVariant, origSubType, fppDriver)) {
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
        wxJSONValue port;
        port["portNumber"] = x;

        stringData["outputs"].Append(port);
    }

    for (int pp = 1; pp <= rules->GetMaxPixelPort(); pp++) {
        if (cud.HasPixelPort(pp)) {
            UDControllerPort* port = cud.GetControllerPixelPort(pp);
            port->CreateVirtualStrings(false);
            for (const auto& pvs : port->GetVirtualStrings()) {
                wxJSONValue vs;
                vs["description"] = pvs->_description;
                vs["startChannel"] = pvs->_startChannel - 1; // we need 0 based
                vs["pixelCount"] = pvs->Channels() / pvs->_channelsPerPixel;

                rngs[pvs->_startChannel - 1] = pvs->Channels();

                if (origStrings.find(vs["description"].AsString()) != origStrings.end()) {
                    wxJSONValue &vo = origStrings[vs["description"].AsString()];
                    vs["groupCount"] = vo["groupCount"];
                    vs["reverse"] = vo["reverse"];
                    vs["colorOrder"] = vo["colorOrder"];
                    vs["nullNodes"] = vo["nullNodes"];
                    vs["endNulls"] = vo.HasMember("endNulls") ? vo["endNulls"] : 0;
                    vs["zigZag"] = vo["zigZag"];
                    vs["brightness"] = vo["brightness"];
                    vs["gamma"] = vo["gamma"];
                } else {
                    vs["groupCount"] = 0;
                    vs["reverse"] = 0;
                    if (pvs->_channelsPerPixel == 4) {
                        vs["colorOrder"] = wxString("RGBW");
                    } else if (pvs->_channelsPerPixel == 1) {
                        vs["colorOrder"] = wxString("W");
                    } else {
                        vs["colorOrder"] = wxString("RGB");
                    }
                    vs["nullNodes"] = 0;
                    vs["endNulls"] = 0;
                    vs["zigZag"] = 0; // If we zigzag in xLights, we don't do it in the controller, if we need it in the controller, we don't know about it here
                    vs["brightness"] = defaultBrightness;
                    vs["gamma"] = wxString::Format("%.1f", defaultGamma);
                }
                if (pvs->_reverseSet) {
                    vs["reverse"] = pvs->_reverse == "Reverse" ? 1 : 0;
                }
                if (pvs->_gammaSet) {
                    char buf[16];
                    sprintf(buf, "%g", pvs->_gamma);
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
                    vs["colorOrder"] = wxString("W");
                }
                if (pvs->_groupCountSet) {
                    vs["groupCount"] = pvs->_groupCount;
                }
                if (vs["groupCount"].AsLong() > 1) {
                    //if the group count is >1, we need to adjust the number of pixels
                    vs["pixelCount"] = vs["pixelCount"].AsLong() * vs["groupCount"].AsLong();
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
                    if (pvs->_smartRemoteType.find("v2") != std::string::npos) {
                        stringData["outputs"][port->GetPort() - 1]["differentialType"] = 4;
                    } else {
                        stringData["outputs"][port->GetPort() - 1]["differentialType"] = 1;
                    }
                }

                stringData["outputs"][port->GetPort() - 1][vsname].Append(vs);
            }
        }
    }

    for (int x = 0; x < rules->GetMaxPixelPort(); x++) {
        if (!stringData["outputs"][x].HasMember("virtualStrings")
            || stringData["outputs"][x]["virtualStrings"].IsNull()
            || stringData["outputs"][x]["virtualStrings"].Size() == 0) {
            wxJSONValue vs;
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
            vs["gamma"] = wxString::Format("%.1f", defaultGamma);
            stringData["outputs"][x]["virtualStrings"].Append(vs);
        }
        if ((x & 0x3) == 0) {
            //need to check the group of 4 to see if we need a smartRemote or not
            int remoteType = 0;
            bool remoteTypeV2 = false;
            for (int z = 0; z < 4; z++) {
                if ((x + z) < maxport) {
                    if (stringData["outputs"][x + z].HasMember("virtualStringsF")) {
                        remoteType = std::max(remoteType, 6);
                    } else if (stringData["outputs"][x + z].HasMember("virtualStringsE")) {
                        remoteType = std::max(remoteType, 5);
                    } else if (stringData["outputs"][x + z].HasMember("virtualStringsD")) {
                        remoteType = std::max(remoteType, 4);
                    } else if (stringData["outputs"][x + z].HasMember("virtualStringsC")) {
                        remoteType = std::max(remoteType, 3);
                    } else if (stringData["outputs"][x+z].HasMember("virtualStringsB")) {
                        remoteType = std::max(remoteType, 2);
                    } else if (stringData["outputs"][x+z].HasMember("differentialType")) {
                        remoteType = std::max(remoteType, 1);
                    }
                    if (stringData["outputs"][x + z].HasMember("differentialType") &&
                        stringData["outputs"][x + z]["differentialType"].AsLong() > 3) {
                        remoteTypeV2 = true;
                    }
                }
            }
            if (remoteTypeV2) {
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
                wxJSONValue port;
                port["portNumber"] = x;
                port["expansionType"] = expansionPorts.find(x+1)->second;
                stringData["outputs"].Append(port);
            } else {
                int count = expansionPorts.find(x+1)->second;
                int expansionType = 0;
                for (int p = 0; p < count; p++) {
                    if (stringData["outputs"][x+p].HasMember("differentialType") && stringData["outputs"][x+p]["differentialType"].AsLong()) {
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

    wxJSONValue bbbDmxData;
    bool hasBBBDmx = false;
    wxJSONValue otherDmxData;
    for (int sp = 1; sp <= rules->GetMaxSerialPort(); sp++) {
        wxJSONValue port;
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
                port["outputType"] = isDMX ? wxString("DMX") : wxString("Pixelnet");
                bbbDmxData["outputs"].Append(port);
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
                port["description"] = wxString("");
                otherDmxData["channelOutputs"].Append(port);
            }

        } else if (portType == "BBBSerial") {
            hasBBBDmx = true;
            port["startChannel"] = 0;
            port["channelCount"] = 0;
            port["outputNumber"] = (sp - 1);
            port["outputType"] = isDMX ? wxString("DMX") : wxString("Pixelnet");
            bbbDmxData["outputs"].Append(port);
        }
    }
    if (hasBBBDmx) {
        bbbDmxData["channelCount"] = isDMX ? (maxChan < 16 ? 16 : maxChan) : 4096;
        if (maxChan == 0) {
            bbbDmxData["enabled"] = 0;
            bbbDmxData["subType"] = wxString("off");
        }
    }
    // let the string handling know if it's safe to use the other PRU
    // or if the serial out will need it
    stringData["serialInUse"] = hasSerial;

    wxJSONValue root;
    root["channelOutputs"].Append(stringData);
    if (hasBBBDmx) {
        bbbDmxData["enabled"] = 1;
        bbbDmxData["startChannel"] = 1;
        bbbDmxData["type"] = wxString("BBBSerial");
        bbbDmxData["subType"] = isDMX ? wxString("DMX") : wxString("PixelNet") ;
        bbbDmxData["device"] = wxString(rules->GetID());
        bbbDmxData["pinoutVersion"] = pinout;
        root["channelOutputs"].Append(bbbDmxData);
    } else {
        wxJSONValue otherOrigRoot = otherDmxData;
        bool changed = true;
        if (IsDrive()) {
            GetPathAsJSON(ipAddress +GetPathSeparator()  + "config" +GetPathSeparator()  + "co-other.json", otherOrigRoot);
            changed = mergeSerialInto(otherDmxData, otherOrigRoot, true);
        } else {
            if (GetURLAsJSON("/api/configfile/co-other.json", otherOrigRoot, false)) {
                changed = mergeSerialInto(otherDmxData, otherOrigRoot, true);
            }
        }
        if (changed) {
            if (IsDrive()) {
                WriteJSONToPath(ipAddress +GetPathSeparator()  + "config" +GetPathSeparator()  + "co-other.json", otherOrigRoot);
            } else {
                PostJSONToURL("/api/configfile/co-other.json", otherOrigRoot);
                SetRestartFlag();
            }
        }
    }

    logger_base.debug("New JSON");
    DumpJSON(root);

    if (!origJson.IsSameAs(root)) {
        logger_base.debug("Uploading New JSON");
        wxFileName fn;
        fn.AssignTempFileName("pixelOutputs");
        file = fn.GetFullPath().ToStdString();
        wxFileOutputStream ufile(fn.GetFullPath());
        wxJSONWriter writer(wxJSONWRITER_STYLED, 0, 3);
        writer.Write(root, ufile);
        ufile.Close();

        if (IsDrive()) {
            WriteJSONToPath(ipAddress +GetPathSeparator()  + "config" +GetPathSeparator()  + fppFileName +".json", root);
        } else {
            PostJSONToURL("/api/channel/output/" + fppFileName, root);
            SetRestartFlag();
        }
    } else {
        logger_base.debug("Skipping JSON upload as it has not changed.");
    }
    SetNewRanges(rngs);
    return false;
}
#endif

#define FPP_CTRL_PORT 32320
static void ProcessFPPSysinfo(Discovery &discovery, const std::string &ip, const std::string &proxyIp, const std::string &sysInfo);

static bool resolvableHostname(const std::string &hn, const std::string &ip) {
    wxIPV4address address;
    // hostname resolves to correct IP, DNS works, we can use it
    return address.Hostname(hn) && address.IPAddress() == ip;
}
static void setRangesToChannelCount(DiscoveredData *inst) {
    int min = 9999999; int max = 0;
    if (inst->ranges != "") {
        wxArrayString r1 = wxSplit(wxString(inst->ranges), ',');
        for (auto a : r1) {
            wxArrayString r = wxSplit(a, '-');
            int start = wxAtoi(r[0]);
            int len = 4; //at least 4
            if (r.size() == 2) {
                len = wxAtoi(r[1]) - start + 1;
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
    } else if (inst->typeId == 0xC2 || inst->typeId == 0xC3) {
        if (inst->controller->GetProtocol() != OUTPUT_DDP) {
            inst->controller->SetProtocol(OUTPUT_DDP);
            dynamic_cast<DDPOutput*>(inst->controller->GetOutputs().front())->SetKeepChannelNumber(false);
        }
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
            inst->controller->SetProtocol(OUTPUT_E131);
        }
    }
    setRangesToChannelCount(inst);
}

static void ProcessFPPSystems(Discovery &discovery, const std::string &systemsString) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxJSONValue origJson;
    wxJSONReader reader;
    bool parsed = reader.Parse(systemsString, &origJson) == 0;
    if (!parsed) {
        return;
    }

    wxString IPKey = "IP";
    wxString PlatformKey = "Platform";
    wxString HostNameKey = "HostName";
    wxString ModeStringKey = "fppMode";
    if (origJson.HasMember("systems")) {
        IPKey = "address";
        PlatformKey = "type";
        HostNameKey = "hostname";
        ModeStringKey = "fppModeString";
    }
    wxJSONValue systems = origJson.HasMember("systems") ? origJson["systems"] : origJson;

    for (int x = 0; x < systems.Size(); x++) {
        wxJSONValue &system = systems[x];
        std::string address = ToUTF8(system[IPKey].AsString());
        std::string hostName = system[HostNameKey].IsNull() ? "" : ToUTF8(system[HostNameKey].AsString());
        std::string uuid = system.HasMember("uuid") ? ToUTF8(system["uuid"].AsString()) : (system.HasMember("UUID") ? ToUTF8(system["UUID"].AsString()) : "");
        
        logger_base.info("Processing ip: %s   host: %s    uuid: %s", address.c_str(), hostName.c_str(), uuid.c_str());
        
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
        if (!system[PlatformKey].IsNull()) {
            inst.platform = ToUTF8(system[PlatformKey].AsString());
        }

        if (!system["model"].IsNull()) {
            inst.platformModel = ToUTF8(system["model"].AsString());
        }
        inst.ip = address;
        if (!system["version"].IsNull()) {
            inst.version = ToUTF8(system["version"].AsString());
            if (inst.version.size() > 3 && (inst.version[3] == '-' || inst.version[3] == '.')) {
                inst.patchVersion = wxAtoi(inst.version.substr(4));
            }
        }
        if (system["minorVersion"].IsInt()) {
            inst.minorVersion = system["minorVersion"].AsInt();
        }
        if (system["majorVersion"].IsInt()) {
            inst.majorVersion = system["majorVersion"].AsInt();
        }
        if (system["typeId"].IsInt()) {
            inst.typeId = system["typeId"].AsInt();
        }
        if (!system["channelRanges"].IsNull()) {
            inst.ranges = ToUTF8(system["channelRanges"].AsString());
        }
        if (!system["HostDescription"].IsNull()) {
            inst.description = ToUTF8(system["HostDescription"].AsString());
        }
        if (!system[ModeStringKey].IsNull()) {
            inst.mode = ToUTF8(system[ModeStringKey].AsString());
            if (inst.mode == "player" && system.HasMember("multisync") && system["multisync"].AsBool()) {
                inst.mode += " w/multisync";
            }
        }
        if (inst.typeId == 0xC2 || inst.typeId == 0xC3) {
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
                if (found->platform == "") {
                    found->platform = inst.platform;
                }
                if (found->mode == "") {
                    found->mode = inst.mode;
                }
                if (found->platformModel == "") {
                    found->platformModel = inst.platformModel;
                }
                if (found->typeId == 0) {
                    found->typeId = inst.typeId;
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
                    } else {
                        discovery.AddCurl(ipAddr, "/fppjson.php?command=getSysInfo&simple", [&discovery, ipAddr, found] (int rc, const std::string &buffer, const std::string &err) {
                            if (rc == 200) {
                                found->extraData["httpConnected"] = true;
                                ProcessFPPSysinfo(discovery, ipAddr, "", buffer);
                            }
                            return true;
                        });
                    }
                    return true;
                });
            } else if (found->typeId >= 0xD0) {
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
    wxJSONValue origJson;
    wxJSONReader reader;
    bool parsed = reader.Parse(proxies, &origJson) == 0;
    if (!parsed) {
        return;
    }
    DiscoveredData *ipinst = discovery.FindByIp(ip, "", true);
    ipinst->extraData["httpConnected"] = true;
    for (int x = 0; x < origJson.Size(); x++) {
        std::string proxy = ToUTF8(origJson[x].AsString());
        DiscoveredData *inst = discovery.FindByIp(proxy, "", true);
        if (!inst->extraData.HasMember("httpConnected")) {
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

    wxJSONValue val;
    wxJSONReader reader;
    bool parsed = reader.Parse(outputs, &val) == 0;
    if (!parsed) {
        return;
    }
    DiscoveredData *inst = discovery.FindByIp(ip, "", true);
    inst->extraData["httpConnected"] = true;
    for (int x = 0; x < val["channelOutputs"].Size(); x++) {
        if (val["channelOutputs"][x]["enabled"].AsInt()) {
            if (val["channelOutputs"][x]["type"].AsString() == "RPIWS281X"||
                val["channelOutputs"][x]["type"].AsString() == "BBB48String" ||
                val["channelOutputs"][x]["type"].AsString() == "BBShiftString" ||
                val["channelOutputs"][x]["type"].AsString() == "DPIPixels") {
                inst->pixelControllerType = ToUTF8(val["channelOutputs"][x]["subType"].AsString());
            } else if (val["channelOutputs"][x]["type"].AsString() == "LEDPanelMatrix") {
                if (inst->pixelControllerType.empty()) {
                    inst->pixelControllerType = LEDPANELS;
                }
                int pw = val["channelOutputs"][x]["panelWidth"].AsInt();
                int ph = val["channelOutputs"][x]["panelHeight"].AsInt();
                int nw = 0; int nh = 0;
                bool tall = false;
                for (int p = 0; p < val["channelOutputs"][x]["panels"].Size(); ++p) {
                    int r = val["channelOutputs"][x]["panels"][p]["row"].AsInt();
                    int c = val["channelOutputs"][x]["panels"][p]["col"].AsInt();
                    nw = std::max(c, nw);
                    nh = std::max(r, nh);
                    wxString orientation = val["channelOutputs"][x]["panels"][p]["orientation"].AsString();
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
            } else if (val["channelOutputs"][x]["type"].AsString() == "VirtualMatrix") {
                if (inst->pixelControllerType.empty()) {
                    inst->pixelControllerType = "Virtual Matrix";
                }
            }
        }
    }
    SetControllerType(inst);
}
static void ProcessFPPSysinfo(Discovery &discovery, const std::string &ip, const std::string &proxy, const std::string &sysInfo) {
    wxJSONValue val;
    wxJSONReader reader;
    bool parsed = reader.Parse(sysInfo, &val) == 0;
    if (!parsed) {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Could not parse sysinfo for %s(%s)", ip.c_str(), proxy.c_str());
        DiscoveredData *inst = discovery.FindByIp(ip, "", true);
        inst->extraData["httpConnected"] = false;
        if (proxy != "") {
            inst->SetProxy(proxy);
        }
    }
    if (parsed) {
        std::string uuid = ToUTF8(val.HasMember("uuid") ? val["uuid"].AsString() : (val.HasMember("UUID") ? val["UUID"].AsString() : ""));

        DiscoveredData *inst = discovery.FindByUUID(uuid, ip);
        if (inst == nullptr) {
            inst = discovery.FindByIp(ip, "", true);
        }
        inst->extraData["httpConnected"] = true;
        if (proxy != "") {
            inst->SetProxy(proxy);
        }
        
        inst->platform = ToUTF8(val["Platform"].AsString());
        inst->platformModel = ToUTF8(val["Variant"].AsString());
        inst->version = ToUTF8(val["Version"].AsString());
        inst->hostname =ToUTF8( val["HostName"].AsString());
        inst->description = ToUTF8(val["HostDescription"].AsString());
        inst->mode = ToUTF8(val["Mode"].AsString());
        if (inst->mode == "player" && val.HasMember("multisync") && val["multisync"].AsBool()) {
            inst->mode += " w/multisync";
        }
        inst->uuid = uuid;
        if (inst->typeId == 0 && val["typeId"].IsInt()) {
            inst->typeId = val["typeId"].AsInt();
        }

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
        if (val.HasMember("channelRanges")) {
            std::string r = val["channelRanges"].AsString().ToStdString();
            if (r.size() > inst->ranges.size()) {
                inst->ranges = r;
            }
        }
        if (val.HasMember("minorVersion")) {
            inst->minorVersion = val["minorVersion"].AsInt();
        }
        if (val.HasMember("majorVersion")) {
            inst->majorVersion = val["majorVersion"].AsInt();
        }

        std::string file = "co-pixelStrings";
        if (inst->platform.find("Beagle") != std::string::npos) {
            file = "co-bbbStrings";
        }
        std::string baseUrl = "";
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
                wxJSONValue val;
                wxJSONReader reader;
                if (reader.Parse(buffer, &val) == 0) {
                    inst->extraData["playlists"] = val;
                }
            }
            return true;
        });
        discovery.AddCurl(baseIp, baseUrl + "/api/cape",
                          [&discovery, host, inst, baseUrl, baseIp] (int rc, const std::string &buffer, const std::string &err) {
            if (rc == 200) {
                wxJSONValue val;
                wxJSONReader reader;
                if (reader.Parse(buffer, &val) == 0) {
                    inst->extraData["cape"] = val;
                }
            }
            return true;
        });
        if (inst->proxy == "") {
            discovery.AddCurl(baseIp, "/api/proxies",
                              [&discovery, host] (int rc, const std::string &buffer, const std::string &err) {
                if (rc == 200) {
                    ProcessFPPProxies(discovery, host, buffer);
                }
                return true;
            });
        }
    }
}


static void ProcessFPPPingPacket(Discovery &discovery, uint8_t *buffer,int len) {
    if (buffer[0] == 'F' && buffer[1] == 'P' && buffer[2] == 'P' && buffer[3] == 'D' && buffer[4] == 0x04) {
        char ip[64];
        sprintf(ip, "%d.%d.%d.%d", (int)buffer[15], (int)buffer[16], (int)buffer[17], (int)buffer[18]);
        //printf("Ping %s\n", ip);
        if (strcmp(ip, "0.0.0.0")) {
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.info("FPP Discovery - Received Ping response from %s", ip);
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
                        } else {
                            discovery.AddCurl(ipAddr, "/fppjson.php?command=getSysInfo&simple", [&discovery, ipAddr] (int rc, const std::string &buffer, const std::string &err) {
                                if (rc == 200) {
                                    ProcessFPPSysinfo(discovery, ipAddr, "", buffer);
                                }
                                return true;
                            });
                        }
                        return true;
                    });
                }
            }
            if (inst->typeId == 0) {
                inst->typeId = buffer[9];
            }
            if (inst->hostname == "") {
                inst->hostname = (char *)&buffer[19];
            }
            if (inst->platformModel == "") {
                inst->platformModel = (char *)&buffer[125];
            }
            if (inst->platform == "") {
                inst->platform = (char *)&buffer[125];
            }
            if (inst->ip == "") {
                inst->ip = ip;
            }
            if (inst->version == "") {
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
        discovery.AddCurl(a, "/api/fppd/multiSyncSystems", [&discovery] (int rc, const std::string &buffer, const std::string &err) {
            if (rc == 200) {
                ProcessFPPSystems(discovery, buffer);
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
    if (this->IsVersionAtLeast(6, 0)) {
        return true;
    }
    if (this->IsVersionAtLeast(5, 3)) {
        if (capeInfo.HasMember("verifiedKeyId")) {
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
        if (res->extraData.HasMember("httpConnected") && res->extraData["httpConnected"].AsBool() == true) {
            // genuine FPP instance and able to connect via http
            return true;
        } else {
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.info("FPP Discovery - Skipping %s no http connection", (const char *)res->ip.c_str());
            return false;
        }
    }

    if ((res->typeId >= 0xC2) && (res->typeId <= 0xC3)) {
        if (res->ranges == "") {
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

    if (res->typeId == 0x88 || res->typeId == 0x89) {
        // F16V4 / F48V4
        return res->mode != "bridge";
    }

    return false;
}

inline void setIfEmpty(std::string &val, const std::string &nv) {
    if (val == "") {
        val = nv;
    }
}
inline void setIfEmpty(uint32_t &val, uint32_t nv) {
    if (val == 0) {
        val = nv;
    }
}

void FPP::MapToFPPInstances(Discovery &discovery, std::list<FPP*> &instances, OutputManager* outputManager) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    for (auto res : discovery.GetResults()) {
        if (::supportedForFPPConnect(res, outputManager)) {
            logger_base.info("FPP Discovery - Found Supported FPP Instance: %s (h: %s)(p: %s)(r: %s)", res->ip.c_str(), res->hostname.c_str(), res->proxy.c_str(), res->ranges.c_str());
            FPP *fpp = nullptr;

            for (auto f : instances) {
                if (f->ipAddress == res->ip) {
                    fpp = f;
                }
            }
            if (fpp == nullptr) {
                FPP *fpp = new FPP(res->ip, res->proxy, res->pixelControllerType);
                fpp->ipAddress = res->ip;//not needed, in constructor
                fpp->hostName = res->hostname;
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
                if (res->extraData.HasMember("playlists")) {
                    for (int x = 0; x < res->extraData["playlists"].Size(); x++) {
                        fpp->playlists.push_back(res->extraData["playlists"][x].AsString());
                    }
                }
                if (res->extraData.HasMember("cape")) {
                    fpp->capeInfo = res->extraData["cape"];
                }
                instances.push_back(fpp);
            } else {
                setIfEmpty(fpp->proxy, res->proxy);
                setIfEmpty(fpp->hostName, res->hostname);
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
                if (fpp->playlists.empty() && res->extraData.HasMember("playlists")) {
                    for (int x = 0; x < res->extraData["playlists"].Size(); x++) {
                        fpp->playlists.push_back(res->extraData["playlists"][x].AsString());
                    }
                }
                if (res->extraData.HasMember("cape")) {
                    fpp->capeInfo = res->extraData["cape"];
                }
            }
        } else {
            logger_base.info("FPP Discovery - %s is not a supported FPP Instance", res->ip.c_str());
        }
    }
}

void FPP::TypeIDtoControllerType(int typeId, FPP* inst) {
    if (typeId < 0x80) {
        inst->fppType = FPP_TYPE::FPP;
    } else if (typeId == 0x88 || typeId == 0x89) {
        inst->fppType = FPP_TYPE::FALCONV4;
    } else if (typeId == 0xC2 || typeId == 0xC3) {
        inst->fppType = FPP_TYPE::ESPIXELSTICK;
    }
}

std::vector<std::string> FPP::GetProxies()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::vector<std::string> res;

    if (IsConnected()) {
        wxJSONValue val;
        if (GetURLAsJSON("/api/proxies", val)) {
            for (int x = 0; x < val.Size(); x++) {
                if (val[x].IsString()) {
                    logger_base.debug("FPP %s proxies %s.", (const char*)ipAddress.c_str(), (const char*)val[x].AsString().c_str());
                    res.push_back(ToUTF8(val[x].AsString()));
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
        for (const auto& it : fpp.GetProxies()) {
            if (to == it) return true;
        }
    }
    return false;
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
        if (eth != nullptr && eth->GetIP() != "" && eth->GetIP() != "MULTICAST") {
            if (eth->GetResolvedIP() == "") {
                startAddresses.push_back(::Lower(eth->GetIP()));
            } else {
                startAddresses.push_back(::Lower(eth->GetResolvedIP()));
            }
            if (eth->GetFPPProxy() != "") {
                startAddresses.push_back(::Lower(ip_utils::ResolveIP(eth->GetFPPProxy())));
            }
        }
    }

    startAddresses.sort();
    startAddresses.unique();

    Discovery discovery(frame, outputManager);
    FPP::PrepareDiscovery(discovery, startAddresses);
    discovery.Discover();
    FPP::MapToFPPInstances(discovery, instances, outputManager);
    instances.sort(sortByIP);

    wxString newForce = "";
    for (const auto& a : startAddressesForced) {
        for (const auto& fpp : instances) {
            if (case_insensitive_match(a, fpp->hostName) || case_insensitive_match(a, fpp->ipAddress)) {
                if (newForce != "") {
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

    return instances;
}
