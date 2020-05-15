/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#define NOMINMAX

#include <map>
#include <string.h>
#include <cctype>

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
#include <zstd.h>

#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"

#include "FPP.h"
#include "../xLightsXmlFile.h"
#include "../models/Model.h"
#include "../models/MatrixModel.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../outputs/E131Output.h"
#include "../outputs/DDPOutput.h"
#include "../outputs/ControllerEthernet.h"
#include "../UtilFunctions.h"
#include "../xLightsVersion.h"
#include "../Parallel.h"
#include "ControllerCaps.h"

#include <log4cpp/Category.hh>
#include "ControllerUploadData.h"
#include "../FSEQFile.h"

#include "Falcon.h"
#include "SanDevices.h"
#include "J1Sys.h"

#include "TraceLog.h"
using namespace TraceLog;

class WXDLLIMPEXP_CORE xlPasswordEntryDialog : public wxPasswordEntryDialog
{
public:
    xlPasswordEntryDialog(wxWindow *parent,
                      const wxString& message,
                      const wxString& caption = wxGetPasswordFromUserPromptStr,
                      const wxString& value = wxEmptyString,
                      long style = wxTextEntryDialogStyle,
                      const wxPoint& pos = wxDefaultPosition)
    {
        Create(parent, message, caption, value, style, pos);
    }

    bool Create(wxWindow *parent,
                const wxString& message,
                const wxString& caption = wxGetPasswordFromUserPromptStr,
                const wxString& value = wxEmptyString,
                long style = wxTextEntryDialogStyle,
                const wxPoint& pos = wxDefaultPosition) {
        bool b = wxPasswordEntryDialog::Create(parent, message, caption, value, style, pos);

        savePasswordCheckbox = new wxCheckBox(this, wxID_ANY, _("Save Password"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_PWD"));
        savePasswordCheckbox->SetValue(false);

        wxSizerFlags flagsBorder2;
        flagsBorder2.DoubleBorder();

#if wxUSE_SECRETSTORE
        GetSizer()->Insert(2, savePasswordCheckbox, flagsBorder2);
        GetSizer()->SetSizeHints(this);
        GetSizer()->Fit(this);
#endif
        return b;
    }

    bool shouldSavePassword() const {
        return savePasswordCheckbox && savePasswordCheckbox->IsChecked();
    }

private:
    wxCheckBox *savePasswordCheckbox = nullptr;


public:
#if wxUSE_SECRETSTORE
    static wxSecretStore pwdStore;
    static bool GetStoredPasswordForService(const std::string &service, std::string &user, std::string &pwd) {
        if (pwdStore.IsOk()) {
            wxSecretValue password;
            wxString usr;
            if (pwdStore.Load("xLights/FPP/" + service, usr, password)) {
                user = usr;
                pwd = password.GetAsString();
                return true;
            }
        }
        return false;
    }

    static bool StorePasswordForService(const std::string &service, const std::string &user, const std::string &pwd) {
        if (pwdStore.IsOk()) {
            wxSecretValue password(pwd);
            if (pwdStore.Save("xLights/FPP/" + service, user, password)) {
                return true;
            }
        }
        return false;
    }
#else
    static bool GetStoredPasswordForService(const std::string &service, std::string &user, std::string &pwd) {
        return false;
    }
    static bool StorePasswordForService(const std::string &service, const std::string &user, const std::string &pwd) {
        return false;
    }
#endif
};

#if wxUSE_SECRETSTORE
wxSecretStore xlPasswordEntryDialog::pwdStore = wxSecretStore::GetDefault();
#endif



static const std::string PIHAT("Pi Hat");
static const std::string LEDPANELS("LED Panels");

FPP::FPP(const std::string &ad) : BaseController(ad, ""), majorVersion(0), minorVersion(0), outputFile(nullptr), parent(nullptr), ipAddress(ad), curl(nullptr), isFPP(true) {
    wxIPV4address address;
    if (address.Hostname(ad)) {
        hostName = ad;
        ipAddress = address.IPAddress();
        _ip = ipAddress;

    }
    _connected = true; // well not really but i need to fake it
}


FPP::FPP(const std::string &ip, const std::string &proxy, const std::string &model) : BaseController(ip, proxy), majorVersion(0), minorVersion(0), outputFile(nullptr), parent(nullptr), curl(nullptr), isFPP(true) {
    ipAddress = ip;
    pixelControllerType = model;
    wxIPV4address address;
    if (address.Hostname(ipAddress)) {
        hostName = ipAddress;
        ipAddress = address.IPAddress();
        _ip = ipAddress;
    }
    _connected = true; // well not really but i need to fake it
}

FPP::FPP(const FPP &c)
    : majorVersion(c.majorVersion), minorVersion(c.minorVersion), outputFile(nullptr), parent(nullptr), curl(nullptr),
    hostName(c.hostName), description(c.description), ipAddress(c.ipAddress), fullVersion(c.fullVersion), platform(c.platform),
    model(c.model), ranges(c.ranges), mode(c.mode), pixelControllerType(c.pixelControllerType), username(c.username), password(c.password), isFPP(c.isFPP)
{

}

FPP::~FPP() {
    if (outputFile) {
        delete outputFile;
        outputFile = nullptr;
    }
    if (tempFileName != "") {
        ::wxRemoveFile(tempFileName);
        tempFileName = "";
    }
    if (curl) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

void FPP::setIPAddress(const std::string &ip) {
    ipAddress = ip;
}


class FPPWriteData {
public:
    FPPWriteData() : file(nullptr), progress(nullptr), data(nullptr), dataSize(0), curPos(0),
        postData(nullptr), postDataSize(0), totalWritten(0), cancelled(false), lastDone(0) {}

    uint8_t *data;
    size_t dataSize;
    size_t curPos;

    wxFile *file;

    uint8_t *postData;
    size_t postDataSize;

    wxProgressDialog *progress;
    std::string progressString;
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
            totalWritten += t;

            if (progress) {
                size_t donePct = totalWritten;
                donePct *= 1000;
                donePct /= file->Length();
                if (donePct != lastDone) {
                    lastDone = donePct;
                    cancelled = !progress->Update(donePct, progressString, &cancelled);
                    wxYield();
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

void FPP::setupCurl() {
    if (curl == nullptr) {
        curl = curl_easy_init();
    }
    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, BaseController::writeFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlInputBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 2000);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 30000);
}

bool FPP::GetURLAsString(const std::string& url, std::string& val)  {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    setupCurl();
    curlInputBuffer.clear();
    char error[1024];
    std::string fullUrl = "http://" + ipAddress + url;
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error);

    if (username != "") {
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC | CURLAUTH_DIGEST | CURLAUTH_NEGOTIATE);
    }

    bool retValue = false;
    int i = ::curl_easy_perform(curl);
    long response_code = 0;
    if (i == CURLE_OK) {
        val = curlInputBuffer;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 401) {
            curlInputBuffer.clear();
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
        retValue = (response_code == 200);
    }
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, nullptr);
    logger_base.info("FPPConnect GET %s  - Return: %d - RC: %d  - %s", fullUrl.c_str(), i, response_code, val.c_str());
    return retValue;

}
int FPP::PostToURL(const std::string& url, const wxMemoryBuffer &val, const std::string &contentType) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    setupCurl();
    curlInputBuffer.clear();
    char error[1024];
    std::string fullUrl = "http://" + ipAddress + url;
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error);
    if (username != "") {
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC | CURLAUTH_DIGEST | CURLAUTH_NEGOTIATE);
    }

    struct curl_slist *chunk = nullptr;
    chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
    std::string ct = "Content-Type: " + contentType;
    chunk = curl_slist_append(chunk, ct.c_str());

    FPPWriteData data;
    data.data = (uint8_t*)val.GetData();
    data.dataSize = val.GetDataLen();
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    int i = curl_easy_perform(curl);
    curl_slist_free_all(chunk);
    long response_code = 0;
    if (i == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        return response_code;
    }
    logger_base.info("FPPConnect Post %s  - RC: %d - Return %d", fullUrl.c_str(), response_code, i);
    return 500;
}

bool FPP::GetURLAsJSON(const std::string& url, wxJSONValue& val) {
    std::string sval;
    if (GetURLAsString(url, sval)) {
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
        auto s = caps->GetCustomPropertyByPath(wxString::Format("fpp%d", i), "0,0");
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
    std::string conf;
    if (GetURLAsString("/config.php", conf)) {
        parseConfig(conf);
        wxJSONValue val;
        if (GetURLAsJSON("/fppjson.php?command=getSysInfo&simple", val)) {
            return isFPP && parseSysInfo(val);
        }
    }
    return false;
}
bool FPP::parseSysInfo(wxJSONValue& val) {
    platform = val["Platform"].AsString();
    model = val["Variant"].AsString();
    fullVersion = val["Version"].AsString();
    hostName = val["HostName"].AsString();
    description = val["HostDescription"].AsString();
    mode = val["Mode"].AsString();

    if (fullVersion != "") {
        majorVersion = wxAtoi(fullVersion);
        if (fullVersion[2] == 'x') {
            minorVersion = wxAtoi(fullVersion.substr(4)) + 1000;
        } else {
            minorVersion = wxAtoi(fullVersion.substr(2));
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


void FPP::LoadPlaylists(std::list<std::string> &playlists) {
    if (IsVersionAtLeast(2, 6) && !IsDrive()) {
        //ip address, load playlists
        wxJSONValue val;
        if (GetURLAsJSON("/api/playlists", val)) {
            for (int x = 0; x < val.Size(); x++) {
                if (val[x].IsString()) {
                    playlists.push_back(val[x].AsString());
                }
            }
        }
    }
}
void FPP::probePixelControllerType() {
    std::string file = "co-pixelStrings";
    if (platform.find("Beagle") != std::string::npos) {
        file = "co-bbbStrings";
    }
    wxJSONValue val;
    if (GetURLAsJSON("/fppjson.php?command=getChannelOutputs&file=" + file, val)) {
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
        if (val["channelOutputs"][x]["type"].AsString() == "RPIWS281X") {
            if (val["channelOutputs"][x]["enabled"].AsInt()) {
                pixelControllerType = PIHAT;
            }
        } else if (val["channelOutputs"][x]["type"].AsString() == "BBB48String") {
            if (val["channelOutputs"][x]["enabled"].AsInt()) {
                pixelControllerType = val["channelOutputs"][x]["subType"].AsString();
            }
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
                std::string orientation = val["channelOutputs"][x]["panels"][p]["orientation"].AsString();
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
    while(std::getline(ss, to, '\n')){
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
    if (curMode == "") {
        curMode = settings["fppMode"];
    }
    isFPP = settings["Title"].find("Falcon Player") != std::string::npos;
}


bool FPP::IsDrive() {
    return ipAddress.find("/") != std::string::npos || ipAddress.find("\\") != std::string::npos;
}
bool FPP::IsVersionAtLeast(uint32_t maj, uint32_t min) {
    if (majorVersion < maj) {
        return false;
    }
    if (majorVersion > maj) {
        return true;
    }
    return minorVersion >= min;
}

static wxString URLEncode(const wxString &value)
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
                case wxT(' '):  ret.Append('+'); break;
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
    return ret;
}
static inline void addString(wxMemoryBuffer &buffer, const std::string &str) {
    buffer.AppendData(str.c_str(), str.length());
}
bool FPP::GetPathAsJSON(const std::string &path, wxJSONValue &val) {
    wxFileName fn;
    fn = path;
    if (fn.Exists()) {
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
    fn = path;
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
    wxMemoryBuffer memBuffPost;
    addString(memBuffPost, str);
    return PostToURL(url, memBuffPost, "application/json");
}
int FPP::PostJSONToURLAsFormData(const std::string& url, const std::string &extra, const wxJSONValue& val) {
    wxMemoryBuffer memBuffPost;
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
    logger_base.debug(str);
}

int FPP::PostToURL(const std::string& url, const std::string &val, const std::string &contentType) {
    wxMemoryBuffer memBuffPost;
    addString(memBuffPost, val);
    return PostToURL(url, memBuffPost, contentType);
}



bool FPP::uploadFile(const std::string &filename, const std::string &file)  {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxString fn;
    wxString ext;

    for (int a = 0; a < filename.length(); a++) {
        wxChar ch = filename[a];
        if (ch == '"') {
            fn.Append("\\\"");
        } else {
            fn.Append(ch);
        }
    }

    bool cancelled = false;
    progressDialog->SetTitle("FPP Upload");
    logger_base.debug("FPP upload via http of %s.", (const char*)filename.c_str());
    progressDialog->Update(0, "Transferring " + filename + " to " + ipAddress, &cancelled);
    int lastDone = 0;

    std::string ct = "Content-Type: application/octet-stream";
    bool deleteFile = false;
    std::string fullFileName = file;

    setupCurl();
    //if we cannot upload it in 5 minutes, we have serious issues
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000*5*60);

    curlInputBuffer.clear();
    char error[1024];
    std::string fullUrl = "http://" + ipAddress + "/jqupload.php";
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error);
    if (username != "") {
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC | CURLAUTH_DIGEST | CURLAUTH_NEGOTIATE);
    }
    const std::string bound = "----WebKitFormBoundaryb29a7c2fe47b9481";
    struct curl_slist *chunk = nullptr;
    std::string ctMime = "Content-Type: multipart/form-data; boundary=" + bound;
    chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
    chunk = curl_slist_append(chunk, ctMime.c_str());
    chunk = curl_slist_append(chunk, "X-Requested-With: FPPConnect");
    chunk = curl_slist_append(chunk, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);


    wxMemoryBuffer memBuffPost;
    addString(memBuffPost, "\r\n--");
    addString(memBuffPost, bound);
    addString(memBuffPost,"\r\nContent-Disposition: form-data; name=\"\"\r\n\r\nundefined\r\n--");
    addString(memBuffPost, bound);
    addString(memBuffPost,"\r\nContent-Disposition: form-data; name=\"\"\r\n\r\nundefined\r\n--");
    addString(memBuffPost, bound);
    addString(memBuffPost, "--\r\n");

    std::string cd = "Content-Disposition: form-data; name=\"myfile\"; filename=\"";
    cd += fn.ToStdString();
    cd += "\"\r\n";
    wxMemoryBuffer memBuffPre;
    addString(memBuffPre, "--");
    addString(memBuffPre, bound);
    addString(memBuffPre, "\r\n");
    addString(memBuffPre, cd);
    addString(memBuffPre, ct);
    addString(memBuffPre, "\r\n\r\n");

    FPPWriteData data;
    wxFile fileobj(fullFileName);
    fileobj.Seek(0);
    data.data = (uint8_t*)memBuffPre.GetData();
    data.dataSize = memBuffPre.GetDataLen();
    data.progress = progressDialog;
    data.file = &fileobj;
    data.postData =  (uint8_t*)memBuffPost.GetData();
    data.postDataSize = memBuffPost.GetDataLen();
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &data);

    data.progress = progressDialog;
    data.progressString = "Transferring " + filename + " to " + ipAddress;
    data.lastDone = lastDone;

    int i = curl_easy_perform(curl);
    curl_slist_free_all(chunk);
    if (deleteFile) {
        wxRemoveFile(fullFileName);
    }
    long response_code = 0;
    if (i == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            std::string val;
            if (!GetURLAsString("/fppxml.php?command=moveFile&file=" + URLEncode(filename + ext), val)) {
                logger_base.warn("Error trying to rename file.");
            } else {
                logger_base.debug("Renaming done.");
            }
        } else {
            logger_base.warn("Did not get 200 resonse code:  %d", response_code);
            cancelled = true;
        }
    } else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        logger_base.warn("Curl did not upload file:  %d   %s", response_code, error);
    }
    progressDialog->Update(1000, wxEmptyString, &cancelled);
    logger_base.info("FPPConnect Upload file %s  - Return: %d - RC: %d - File: %s", fullUrl.c_str(), i, response_code, filename.c_str());

    return data.cancelled;
}


bool FPP::copyFile(const std::string &filename,
                           const std::string &file,
                           const std::string &dir) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool cancelled = false;

    progressDialog->SetTitle("FPP Upload");
    logger_base.debug("FPP upload via file copy of %s.", (const char*)filename.c_str());
    progressDialog->Update(0, "Transferring " + filename + " to " + ipAddress, &cancelled);
    progressDialog->Show();
    wxFile in;
    in.Open(file);

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
                progressDialog->Update(prgs, wxEmptyString, &cancelled);
                if (!cancelled) {
                    cancelled = progressDialog->WasCancelled();
                }
            }
            progressDialog->Update(1000, wxEmptyString);
            in.Close();
            out.Close();
        } else {
            progressDialog->Update(1000, wxEmptyString);
            logger_base.warn("   Copy of file %s failed ... target file %s could not be opened.", (const char *)file.c_str(), (const char *)target.c_str());
        }
    } else {
        progressDialog->Update(1000, wxEmptyString);
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
    return uploadFile(filename, file);
}


bool FPP::PrepareUploadSequence(const FSEQFile &file,
                                const std::string &seq, const std::string &media,
                                int type) {
    if (outputFile) {
        delete outputFile;
        outputFile = nullptr;
    }
    if (tempFileName != "") {
        ::wxRemoveFile(tempFileName);
        tempFileName = "";
    }

    wxFileName fn(seq);
    std::string baseName = fn.GetFullName();
    std::string mediaBaseName = "";
    bool cancelled = false;
    if (media != "") {
        wxFileName mfn(media);
        mediaBaseName = mfn.GetFullName();

        cancelled |= uploadOrCopyFile(mediaBaseName, media, "music");
        if (cancelled) {
            return cancelled;
        }
    }
    sequences[baseName] = mediaBaseName;

    std::string fileName;
    if (IsDrive()) {
        fileName = ipAddress + wxFileName::GetPathSeparator() + "sequences" + wxFileName::GetPathSeparator() + baseName;
    } else {
        tempFileName = wxFileName::CreateTempFileName(baseName);
        fileName = tempFileName;
    }
    if ((type == 0 && file.getVersionMajor() == 1)
        || fn.GetExt() == "eseq") {

        //these just get uploaded directly
        return uploadOrCopyFile(baseName, seq, fn.GetExt() == "eseq" ? "effects" : "sequences");
    }

    if (type == 1 && file.getVersionMajor() == 2) {
        // Full v2 file, upload directly
        return uploadOrCopyFile(baseName, seq, fn.GetExt() == "eseq" ? "effects" : "sequences");
    }
    baseSeqName = baseName;
    FSEQFile::CompressionType ctype = ::FSEQFile::CompressionType::zstd;
    if (type == 3) {
        ctype = ::FSEQFile::CompressionType::none;
    }
    int clevel = 2;
    if (model.find(" Zero") != std::string::npos
        || model.find("Pi Model A") != std::string::npos
        || model.find("Pi Model B") != std::string::npos) {
        clevel = 1;
        if (ZSTD_versionNumber() > 10305) {
            clevel = -5;
        }
    }
    outputFile = FSEQFile::createFSEQFile(fileName, type == 0 ? 1 : 2, ctype, clevel);
    outputFile->initializeFromFSEQ(file);
    if (type >= 2 && ranges != "") {
        wxArrayString r1 = wxSplit(wxString(ranges), ',');
        for (const auto& a : r1) {
            wxArrayString r = wxSplit(a, '-');
            int start = wxAtoi(r[0]);
            int len = 4; //at least 4
            if (r.size() == 2) {
                len = wxAtoi(r[1]) - start + 1;
            }
            ((V2FSEQFile*)outputFile)->m_sparseRanges.push_back(std::pair<uint32_t, uint32_t>(start, len));
        }
    }
    outputFile->writeHeader();
    return false;
}
bool FPP::AddFrameToUpload(uint32_t frame, uint8_t *data) {
    if (outputFile) {
        outputFile->addFrame(frame, data);
    }
    return false;
}
bool FPP::FinalizeUploadSequence() {
    bool cancelled = false;
    if (outputFile) {
        outputFile->finalize();

        delete outputFile;
        outputFile = nullptr;
        if (tempFileName != "") {
            cancelled = uploadOrCopyFile(baseSeqName, tempFileName, "sequences");
            ::wxRemoveFile(tempFileName);
            tempFileName = "";
        }
    }
    return cancelled;
}
bool FPP::UploadPlaylist(const std::string &name) {
    wxJSONValue origJson;
    std::string fn;
    if (IsDrive() && IsVersionAtLeast(2, 0)) {
        fn = (ipAddress + wxFileName::GetPathSeparator() + "playlists" + wxFileName::GetPathSeparator() + name + ".json");
        GetPathAsJSON(fn, origJson);
    } else if (IsVersionAtLeast(2, 6)) {
        GetURLAsJSON("/api/playlist/" + URLEncode(name), origJson);
    }

    for (const auto& info : sequences) {
        wxJSONValue entry;
        if (info.second != "") {
            entry["type"] = wxString("both");
            entry["enabled"] = 1;
            entry["playOnce"] = 0;
            entry["sequenceName"] = info.first;
            entry["mediaName"] = info.second;
            entry["videoOut"] = wxString("--Default--");
        } else {
            entry["type"] = wxString("sequence");
            entry["enabled"] = 1;
            entry["playOnce"] = 0;
            entry["sequenceName"] = info.first;
        }
        origJson["mainPlaylist"].Append(entry);
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
        std::string fn = (ipAddress + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + "model-overlays.json");
        WriteJSONToPath(fn, models);
    } else if (IsVersionAtLeast(3, 0)) {
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
    } else if (IsVersionAtLeast(3, 6)) {
        PostToURL("/api/configfile/virtualdisplaymap", displayMap);
    }
    return false;
}
bool FPP::UploadUDPOut(const wxJSONValue &udp) {
    if (IsDrive()) {
        std::string fn = (ipAddress + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + "co-universes.json");
        WriteJSONToPath(fn, udp);
    } else if (IsVersionAtLeast(2, 4)) {
        PostJSONToURLAsFormData("/fppjson.php", "command=setChannelOutputs&file=universeOutputs", udp);
    }
    return false;
}

wxJSONValue FPP::CreateModelMemoryMap(ModelManager* allmodels) {
    wxJSONValue json;
    wxJSONValue models;
    for (const auto& m : *allmodels) {
        Model* model = m.second;
        wxString stch = model->GetModelXml()->GetAttribute("StartChannel", wxString::Format("%d?", model->NodeStartChannel(0) + 1)); //NOTE: value coming from model is probably not what is wanted, so show the base ch# instead
        int ch = model->GetNumberFromChannelString(model->ModelStartChannel);
        wxString name(model->name);
        name.Replace(" ", "_");


        int numStr = model->GetNumStrings();
        if (numStr == 0) {
            numStr = 1;
        }
        int straPerStr =  model->GetNumStrands() / numStr;
        if (straPerStr < 1) straPerStr = 1;

        wxJSONValue jm;
        jm["Name"] = name;
        jm["ChannelCount"] = model->GetActChanCount();
        jm["StartChannel"] = ch;
        jm["StrandsPerString"] = straPerStr;
        jm["StringCount"] = numStr;

        MatrixModel *mm = dynamic_cast<MatrixModel*>(model);
        if (mm) {
            if (mm->isVerticalMatrix()) {
                jm["Orientation"] = wxString("vertical");
            } else {
                jm["Orientation"] = wxString("horizontal");
            }
        } else {
            jm["Orientation"] = wxString("horizontal");
        }
        std::string corner = model->GetIsBtoT() ? "B" : "T";
        corner += model->GetIsLtoR() ? "L" : "R";
        jm["StartCorner"] = corner;

        models.Append(jm);
    }
    json["models"] = models;
    return json;
}

std::string FPP::CreateVirtualDisplayMap(ModelManager* allmodels, bool center0) {
    std::string ret;

    int ch = 0;
    std::vector<wxRealPoint> pts;
    int first = 1;
    std::string stringType;

    int xoffset = 0;
    for (auto m = allmodels->begin(); m != allmodels->end(); ++m)
    {
        Model* model = m->second;

        if (model->GetLayoutGroup() != "Default")
            continue;

        if (model->GetDisplayAs() == "ModelGroup")
            continue;

        if (first)
        {
            first = 0;
            ret += "# Preview Size\n";
            ret += wxString::Format("%d,%d\n", model->GetModelScreenLocation().previewW, model->GetModelScreenLocation().previewH);

            if (center0) {
                xoffset = model->GetModelScreenLocation().previewW / 2;
            }
        }

        stringType = model->GetStringType();

        if (stringType == "RGB Nodes")
            stringType = "RGB";
        else if (stringType == "RBG Nodes")
            stringType = "RBG";
        else if (stringType == "GBR Nodes")
            stringType = "GBR";
        else if (stringType == "BGR Nodes")
            stringType = "BGR";
        else if (stringType == "3 Channel RGB")
            stringType = "RGB";
        else if (stringType == "4 Channel RGBW")
            stringType = "RGBW";
        else if (stringType == "Strobes")
            stringType = "White";
        else if (stringType == "Single Color Red")
            stringType = "Red";
        else if ((stringType == "Single Color Green") || (stringType == "G"))
            stringType = "Green";
        else if ((stringType == "Single Color Blue") || (stringType == "B"))
            stringType = "Blue";
        else if ((stringType == "Single Color White") || (stringType == "W"))
            stringType = "White";
        else if (stringType == "Single Color Custom")
        {
            stringType = "White";
        }
        else if (stringType == "Node Single Color")             {
            stringType = "White";
        }

        ret += wxString::Format("# Model: '%s', %d nodes\n", model->GetName().c_str(), model->GetNodeCount());

        float x, y, z;
        for (int i = 0; i < model->GetNodeCount(); i++)
        {
            pts.clear();
            model->GetNodeScreenCoords(i, pts);
            ch = model->NodeStartChannel(i);

            for (int i = 0; i < pts.size(); i++)
            {
                x = pts[i].x;
                y = pts[i].y;
                z = 0;
                model->GetModelScreenLocation().TranslatePoint(x, y, z);
                x += xoffset;
                ret += wxString::Format("%d,%d,%d,%d,%s\n",
                    (int)x, (int)y, ch,
                    model->GetChanCountPerNode(), stringType.c_str());
            }
        }
    }

    return ret;
}

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


bool FPP::UploadUDPOutputsForProxy(OutputManager* outputManager) {
    std::list<ControllerEthernet*> selected;
    for (const auto& it : outputManager->GetControllers())
    {
        auto c = dynamic_cast<ControllerEthernet*>(it);
        if (c != nullptr)
        {
            if (c->GetFPPProxy() == ipAddress)
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

wxJSONValue FPP::CreateUniverseFile(ControllerEthernet* controller, bool input) {
    std::list<ControllerEthernet*> selected;
    selected.push_back(controller);
    return CreateUniverseFile(selected, false);
}

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

bool FPP::SetInputUniverses(ControllerEthernet* controller, wxWindow* parentWin) {

    wxConfigBase* config = wxConfigBase::Get();
    wxString fip;
    config->Read("xLightsPiIP", &fip, "");
    wxString ausername;
    config->Read("xLightsPiUser", &ausername, "fpp");
    wxString apassword;
    config->Read("xLightsPiPassword", &apassword, "true");
    username = ausername;
    password = apassword;

    auto ips = wxSplit(fip, '|');
    auto users = wxSplit(ausername, '|');
    auto passwords = wxSplit(password, '|');

    // they should all be the same size ... but if not base it off the smallest
    int count = std::min(ips.size(), std::min(users.size(), passwords.size()));

    username = "fpp";
    wxString thePassword = "true";
    for (int i = 0; i < count; i++) {
        if (ips[i] == controller->GetIP()) {
            username = users[i];
            thePassword = passwords[i];
        }
    }

    if (thePassword == "true") {
        if (username == "pi") {
            password = "raspberry";
        }
        else if (username == "fpp") {
            password = "falcon";
        }
        else {
            wxTextEntryDialog ted(parentWin, "Enter password for " + username, "Password", controller->GetIP());
            if (ted.ShowModal() == wxID_OK) {
                password = ted.GetValue();
            }
        }
    }
    else {
        wxTextEntryDialog ted(parentWin, "Enter password for " + username, "Password", controller->GetIP());
        if (ted.ShowModal() == wxID_OK) {
            password = ted.GetValue();
        }
    }

    parentWin = parent;

    return (AuthenticateAndUpdateVersions() && !SetInputUniversesBridge(controller));
}

bool FPP::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent)
{
    parent = parent;
    return AuthenticateAndUpdateVersions() && !UploadPixelOutputs(allmodels, outputManager, controller);
}
bool FPP::UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) {
    parent = parent;
    bool b = AuthenticateAndUpdateVersions();
    if (!b) return b;
    UploadPixelOutputs(allmodels, outputManager, controller);
    SetInputUniversesBridge(controller);
    std::string val;
    if (restartNeeded || curMode != "bridge") {
        Restart("bridge");
    }
    return b;
}

wxJSONValue FPP::CreateUniverseFile(const std::list<ControllerEthernet*>& selected, bool input) {
    wxJSONValue root;
    root["type"] = wxString("universes");
    root["enabled"] = 1;
    root["startChannel"] = 1;
    root["channelCount"] = -1;

    wxJSONValue universes;

    for (const auto& it2 : selected)
    {
        auto controllerEnabled = it2->GetActive();
        bool const allSameSize = it2->AllSameSize();
        // Get universes based on IP
        std::list<Output*> outputs = it2->GetOutputs();
        for (const auto& it : outputs) {
            int c = it->GetStartChannel();

            wxJSONValue universe;
            universe["active"] = controllerEnabled == Controller::ACTIVESTATE::ACTIVE ? 1 : 0;
            universe["description"] = stripInvalidChars(it2->GetName());
            universe["id"] = it->GetUniverse();
            universe["startChannel"] = c;
            universe["channelCount"] = it->GetChannels();
            universe["priority"] = 0;
            universe["address"] = wxString("");

            if (it->GetType() == OUTPUT_E131) {
                universe["type"] = (int)(it->GetIP() != "MULTICAST" ? 1 : 0);
                if (!input && (it->GetIP() != "MULTICAST")) {
                    universe["address"] = wxString(it->GetIP());
                }

                // TODO this needs work to restore the loading of multiple universes as a single line
                if (allSameSize) {
                    universe["universeCount"] = it2->GetOutputCount();
                    universes.Append(universe);
                    break;
                }
                else {
                    universe["universeCount"] = 1;
                }
                universes.Append(universe);
            }
            else if (it->GetType() == OUTPUT_DDP) {
                if (!input) {
                    universe["address"] = wxString(it->GetIP());
                    DDPOutput* ddp = (DDPOutput*)it;
                    universe["type"] = ddp->IsKeepChannelNumbers() ? 4 : 5;
                    universes.Append(universe);
                }
                else {
                    //don't need to do anything to configure DDP input
                }
            }
            else if (it->GetType() == OUTPUT_ARTNET) {
                universe["type"] = (int)((it2->GetIP() != "MULTICAST") + 2);
                if (!input && (it->GetIP() != "MULTICAST")) {
                    universe["address"] = wxString(it->GetIP());
                }
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
        std::string m = majorVersion >= 4 ? "2" : "1";
        return GetURLAsString("/fppjson.php?command=setSetting&key=restartFlag&value=" + m, val);
    }
    return false;
}

bool FPP::Restart(const std::string &mode) {
    std::string val;
    if (mode != "" && mode != curMode) {
        std::string m = "1"; //bridge;
        if (mode == "standalone") {
            m = "2";
        } else if (mode == "master") {
            m = "6";
        } else if (mode == "remote") {
            m = "8";
        }
        GetURLAsString("/fppxml.php?command=setFPPDmode&mode=" + m, val);
        SetRestartFlag();
        curMode = mode;
    }
    GetURLAsString("/fppxml.php?command=restartFPPD&quick=1", val);
    restartNeeded = false;
    return false;
}

void FPP::SetDescription(const std::string &st) {
    if (!IsDrive()) {
        std::string val;
        GetURLAsString("/fppjson.php?command=setSetting&key=HostDescription&value=" + URLEncode(st), val);
    }
}


bool FPP::SetInputUniversesBridge(Controller* controller) {
    auto c = dynamic_cast<ControllerEthernet*>(controller);
    if (c == nullptr) return false;

    wxJSONValue udp = CreateUniverseFile(std::list<ControllerEthernet*>({ c }), true);
    if (IsDrive()) {
        std::string fn = (c->GetResolvedIP() + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + "ci-universes.json");
        WriteJSONToPath(fn, udp);
    } else if (IsVersionAtLeast(2, 4)) {
        PostJSONToURLAsFormData("/fppjson.php", "command=setChannelOutputs&file=universeInputs", udp);
    }
    return false;
}
static bool mergeSerialInto(wxJSONValue &otherDmxData, wxJSONValue &otherOrigRoot) {
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
                    if (otherOrigRoot["channelOutputs"][y]["enabled"].AsInt() != 1) {
                        otherOrigRoot["channelOutputs"][y]["enabled"] = 1;
                        changed = true;
                    }
                    if (otherOrigRoot["channelOutputs"][y]["startChannel"].AsInt() != otherDmxData["channelOutputs"][x]["startChannel"].AsInt()) {
                        otherOrigRoot["channelOutputs"][y]["startChannel"] = otherDmxData["channelOutputs"][x]["startChannel"].AsInt();
                        changed = true;
                    }
                    if (otherOrigRoot["channelOutputs"][y]["channelCount"].AsInt() != otherDmxData["channelOutputs"][x]["channelCount"].AsInt()) {
                        otherOrigRoot["channelOutputs"][y]["channelCount"] = otherDmxData["channelOutputs"][x]["channelCount"].AsInt();
                        changed = true;
                    }
                } else if (otherOrigRoot["channelOutputs"][y]["enabled"].AsInt() != 0) {
                    otherOrigRoot["channelOutputs"][y]["enabled"] = 0;
                    changed = true;
                }
            }
        }
        if (!found) {
            //add some defaults if needed
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
            otherOrigRoot["channelOutputs"].Append(otherDmxData["channelOutputs"][x]);
            changed = true;
        }
    }
    return changed;
}



bool FPP::UploadPixelOutputs(ModelManager* allmodels,
                             OutputManager* outputManager,
                             Controller* controller) {
    int maxString = 1;
    int maxdmx = 0;

    auto rules = ControllerCaps::GetControllerConfig(controller);
    if (rules == nullptr) {
        return false;
    }

    maxdmx = rules->GetMaxSerialPort();
    maxString = rules->GetMaxPixelPort();

    std::map<int, int> rngs;
    FillRanges(rngs);

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("FPP Outputs Upload: Uploading to %s", (const char *)ipAddress.c_str());

    std::string check;
    UDController cud(controller, outputManager, allmodels, check, false);
    if (rules->SupportsLEDPanelMatrix()) {
        //LED panel cape, nothing we can really do except update the start channel
        int startChannel = -1;
        if (cud.GetMaxPixelPort()) {
            //The matrix actually has the controller connection defined, we'll use it
            startChannel = cud.GetControllerPixelPort(1)->GetStartChannel();
            startChannel--;
        } else if (!cud.GetNoConnectionModels().empty()) {
            startChannel = cud.GetNoConnectionModels().front()->GetStringStartChan(0);
        }
        if (startChannel >= 0) {
            startChannel++;  //one based
            wxJSONValue origJson;
            if (IsDrive()) {
                GetPathAsJSON(ipAddress + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + "channeloutputs.json", origJson);
            } else {
                GetURLAsJSON("/fppjson.php?command=getChannelOutputs&file=channelOutputsJSON", origJson);
            }
            bool changed = true;
            for (int x = 0; x < origJson["channelOutputs"].Size(); x++) {
                if (origJson["channelOutputs"][x]["type"].AsString() == "LEDPanelMatrix") {
                    if (origJson["channelOutputs"][x].HasMember("startChannel")
                        && origJson["channelOutputs"][x]["startChannel"].AsInt() == startChannel) {
                        changed = false;
                    } else {
                        origJson["channelOutputs"][x]["startChannel"] = startChannel;
                    }
                    rngs[startChannel - 1] = origJson["channelOutputs"][x]["channelCount"].AsLong();
                }
            }

            if (changed) {
                if (IsDrive()) {
                    WriteJSONToPath(ipAddress + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + "channeloutputs.json", origJson);
                } else {
                    PostJSONToURLAsFormData("/fppjson.php", "command=setChannelOutputs&file=channelOutputsJSON", origJson);
                    SetRestartFlag();
                }
            }
        }
        SetNewRanges(rngs);
        return false;
    }

    std::string fppFileName = "co-bbbStrings";
    int minPorts = 1;
    if (pixelControllerType == PIHAT) {
        fppFileName = "co-pixelStrings";
        minPorts = 2;
    }
    cud.Check(rules, check);
    cud.Dump();

    wxFileName fnOrig;
    fnOrig.AssignTempFileName("pixelOutputs");
    std::string file = fnOrig.GetFullPath().ToStdString();
    wxJSONValue origJson;
    if (IsDrive()) {
        GetPathAsJSON(ipAddress + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + fppFileName +".json", origJson);
    } else {
        GetURLAsJSON("/fppjson.php?command=getChannelOutputs&file=" + fppFileName, origJson);
    }
    logger_base.debug("Original JSON");
    DumpJSON(origJson);

    wxString pinout = "1.x";
    std::map<std::string, wxJSONValue> origStrings;
    if (origJson["channelOutputs"].IsArray()) {
        for (int x = 0; x < origJson["channelOutputs"].Size(); x++) {
            wxJSONValue &f = origJson["channelOutputs"][x];
            if (f["type"].AsString() == "BBB48String") {
                pinout = f["pinoutVersion"].AsString();
                if (pinout == "") {
                    pinout = "1.x";
                }
            }
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
    int maxport = 0;

    wxJSONValue stringData;
    stringData["enabled"] = 1;
    stringData["startChannel"] = 1;
    stringData["channelCount"] = -1;

    maxport = cud.GetMaxPixelPort(); // 1 based
    if (maxport < minPorts) {
        maxport = minPorts;
    }

    if (pixelControllerType == PIHAT) {
        stringData["type"] = wxString("RPIWS281X");
        stringData["subType"] = wxString("");
    } else {
        stringData["type"] = wxString("BBB48String");
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
                    vs["zigZag"] = vo["zigZag"];
                    vs["brightness"] = vo["brightness"];
                    vs["gamma"] = vo["gamma"];
                } else {
                    vs["groupCount"] = 0;
                    vs["reverse"] = 0;
                    if (pvs->_channelsPerPixel == 4) {
                        vs["colorOrder"] = wxString("RGBW");
                    } else {
                        vs["colorOrder"] = wxString("RGB");
                    }
                    vs["nullNodes"] = 0;
                    vs["zigZag"] = 0; // If we zigzag in xLights, we don't do it in the controller, if we need it in the controller, we don't know about it here
                    vs["brightness"] = 100;
                    vs["gamma"] = wxString("1.0");
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
                    vs["brightness"] = pvs->_brightness;
                }
                if (pvs->_nullPixelsSet) {
                    vs["nullNodes"] = pvs->_nullPixels;
                }
                if (pvs->_colourOrderSet) {
                    vs["colorOrder"] = pvs->_colourOrder;
                }
                if (pvs->_groupCountSet) {
                    vs["groupCount"] = pvs->_groupCount;
                }
                if (vs["groupCount"].AsInt() > 1) {
                    //if the group count is >1, we need to adjust the number of pixels
                    vs["pixelCount"] = vs["pixelCount"].AsInt() * vs["groupCount"].AsInt();
                }
                std::string vsname = "virtualStrings";
                if (pvs->_smartRemote == 2) {
                    vsname += "B";
                } else if (pvs->_smartRemote == 3) {
                    vsname += "C";
                }
                if (pvs->_smartRemote >= 1) {
                    stringData["outputs"][port->GetPort() - 1]["differentialType"] = 1;
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
            vs["zigZag"] = 0;
            vs["brightness"] = 100;
            vs["gamma"] = wxString("1.0");
            stringData["outputs"][x]["virtualStrings"].Append(vs);
        }
        if ((x & 0x3) == 0) {
            //need to check the group of 4 to see if we need a smartRemote or not
            int remoteType = 0;
            for (int z = 0; z < 4; z++) {
                if ((x + z) < maxport) {
                    if (stringData["outputs"][x+z].HasMember("virtualStringsC")) {
                        remoteType = std::max(remoteType, 3);
                    } else if (stringData["outputs"][x+z].HasMember("virtualStringsB")) {
                        remoteType = std::max(remoteType, 2);
                    } else if (stringData["outputs"][x+z].HasMember("differentialType")) {
                        remoteType = std::max(remoteType, 1);
                    }
                }
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
                    if (stringData["outputs"][x+p].HasMember("differentialType") && stringData["outputs"][x+p]["differentialType"].AsInt()) {
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
            isDMX &= ((port->GetProtocol() == "DMX") || (port->GetProtocol() == "dmx"));

            int dmxOffset = 1;
            UDControllerPortModel* m = port->GetFirstModel();
            if (m != nullptr) {
                dmxOffset = m->GetDMXChannelOffset();
                if (dmxOffset < 1) dmxOffset = 1; // a value less than 1 makes no sense
            }
            int sc = port->GetStartChannel() - dmxOffset + 1;
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
            int dmxOffset = 1;
            UDControllerPortModel* m = vport->GetFirstModel();
            if (m != nullptr) {
                dmxOffset = m->GetDMXChannelOffset();
                if (dmxOffset < 1) dmxOffset = 1; // a value less than 1 makes no sense
            }
            int sc = vport->GetStartChannel() - dmxOffset + 1;
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
                port["channelCount"] = mx;
                port["device"] = portType;
                port["enabled"] = 1;
                port["type"] = vport->GetProtocol();
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
        bbbDmxData["subType"] = wxString("DMX");
        bbbDmxData["device"] = wxString(pixelControllerType);
        root["channelOutputs"].Append(bbbDmxData);
    } else {
        wxJSONValue otherOrigRoot;

        if (IsDrive()) {
            GetPathAsJSON(ipAddress + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + "co-other.json", otherOrigRoot);
        } else {
            GetURLAsJSON("/api/configfile/co-other.json", otherOrigRoot);
        }
        bool changed = mergeSerialInto(otherDmxData, otherOrigRoot);
        if (changed) {
            if (IsDrive()) {
                WriteJSONToPath(ipAddress + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + "co-other.json", otherOrigRoot);
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
            WriteJSONToPath(ipAddress + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + fppFileName +".json", root);
        } else {
            PostJSONToURLAsFormData("/fppjson.php", "command=setChannelOutputs&file=" + fppFileName, root);
            SetRestartFlag();
        }
    }
    else
    {
        logger_base.debug("Skipping JSON upload as it has not changed.");
    }
    SetNewRanges(rngs);
    return false;
}

class CurlData {
public:
    CurlData(const std::string &a) : url(a), type(0), fpp(nullptr), authenticated(0) {
        errorBuffer[0] = 0;
        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, BaseController::writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 2000);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 30000);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &errorBuffer);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
    }
    ~CurlData() {
        curl_easy_cleanup(curl);
    }

    void setFPP(FPP *f) {
        fpp = f;
        if (f->username != "" || f->password != "")  {
            curl_easy_setopt(curl, CURLOPT_USERNAME, f->username.c_str());
            curl_easy_setopt(curl, CURLOPT_PASSWORD, f->password.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC | CURLAUTH_DIGEST | CURLAUTH_NEGOTIATE);
            authenticated = 1;
        }
    }
    void setUsernamePassword(const std::string &username, const std::string &password) {
        this->username = username;
        this->password = password;
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC | CURLAUTH_DIGEST | CURLAUTH_NEGOTIATE);
        authenticated = 1;
    }
    std::string url;
    CURL *curl;
    std::string buffer;
    char errorBuffer[CURL_ERROR_SIZE];
    int type;
    int authenticated;

    //either fpp or the ipAddress/username/password
    FPP *fpp;

    std::string ipAddress;
    std::string username;
    std::string password;

};

#define FPP_CTRL_PORT 32320

static std::string toIp(uint8_t *buffer) {
    std::string address = std::to_string(buffer[0]) + "." + std::to_string(buffer[1]) + "." + std::to_string(buffer[2]) + "." + std::to_string(buffer[3]);
    return address;
}

void FPP::Discover(const std::list<std::string> &addresses, std::list<FPP*> &instances, bool doBroadcast, bool allPlatforms) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    AddTraceMessage("Running FPP Discovery");
    std::vector<CurlData*> curls;
    CURLM * curlMulti = curl_multi_init();
    for (const auto &a : addresses) {
        std::string fullAddress = "http://" + a + "/fppjson.php?command=getFPPSystems";
        CurlData *data = new CurlData(fullAddress);
        data->ipAddress = a;
        curls.push_back(data);
        curl_multi_add_handle(curlMulti, data->curl);
    }
    std::vector<wxDatagramSocket *> sockets;


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

    // scan all ip networks
    wxIPV4address localaddr;
    localaddr.AnyAddress();
    localaddr.Service(FPP_CTRL_PORT);
    wxDatagramSocket *mainSocket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST | wxSOCKET_NOWAIT);
    mainSocket->SetTimeout(1);
    mainSocket->Notify(false);
    if (mainSocket->IsOk()) {
        sockets.push_back(mainSocket);
    }
    auto localIPs = GetLocalIPs();
    for (const auto& ip : localIPs) {
        if (ip == "127.0.0.1") {
            continue;
        }
        wxIPV4address localaddr;
        localaddr.Hostname(ip);

        wxDatagramSocket *socket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST | wxSOCKET_NOWAIT);
        socket->SetTimeout(1);
        socket->Notify(false);
        if (socket->IsOk()) {
            sockets.push_back(socket);
        } else {
            delete socket;
        }
    }
    wxIPV4address bcAddress;
    bcAddress.BroadcastAddress();
    bcAddress.Service(FPP_CTRL_PORT);
    for (const auto& socket : sockets) {
        socket->SendTo(bcAddress, buffer, 207);
    }
    if (mainSocket->IsOk()) {
        wxIPV4address remoteaddr;
        uint8_t target1[] = { 0xE0, 0x00, 0x1E, 0x05 };
        uint8_t target2[] = { 0xE0, 0x46, 0x50, 0x50 };
        remoteaddr.Hostname(toIp(target1));
        remoteaddr.Service(0x7535);
        uint8_t buf[] = { 0x5a, 0x43, 0x50, 0x50, 0, 0, 0, 0 };
        mainSocket->SendTo(remoteaddr, buf, sizeof(buf));

        remoteaddr.Hostname(toIp(target2));
        remoteaddr.Service(0x7E40);
        mainSocket->SendTo(remoteaddr, buffer, 207);
    }


    uint64_t endBroadcastTime = wxGetLocalTimeMillis().GetValue() + 1200l;
    int running = curls.size();
    while (running || (wxGetLocalTimeMillis().GetValue() < endBroadcastTime)) {
        memset(buffer, 0x00, sizeof(buffer));
        int readSize = 0;
        for (const auto& socket : sockets) {
            if (socket->IsOk()) {
                socket->Read(&buffer[0], sizeof(buffer));
                readSize = socket->GetLastIOReadSize();
                if (readSize != 0) {
                    break;
                }
            }
        }
        if (readSize != 0) {
            if (buffer[0] == 'F' && buffer[1] == 'P' && buffer[2] == 'P' && buffer[3] == 'D' && buffer[4] == 0x04) {
                char ip[64];
                sprintf(ip, "%d.%d.%d.%d", (int)buffer[15], (int)buffer[16], (int)buffer[17], (int)buffer[18]);
                if (strcmp(ip, "0.0.0.0")) {
                    AddTraceMessage("Received UDP result " + std::string(ip));

                    //we found a system!!!
                    std::string hostname = (char *)&buffer[19];
                    std::string ipStr = ip;
                    FPP *found = nullptr;
                    for (const auto& a : instances) {
                        if (a->hostName == hostname || a->ipAddress == hostname || a->ipAddress == ipStr) {
                            found = a;
                        }
                    }
                    int platform = buffer[9];
                    //printf("%d: %s  %s     %d\n", found ? 1 : 0, hostname.c_str(), ipStr.c_str(), platform);
                    if (!found && (allPlatforms || (platform > 0 && platform < 0x80))) {
                        //platform > 0x80 is Falcon controllers or xLights
                        FPP *inst = new FPP();
                        inst->hostName = (char *)&buffer[19];
                        inst->model = (char *)&buffer[125];
                        inst->setIPAddress(ip);
                        inst->fullVersion = (char *)&buffer[84];
                        inst->minorVersion = buffer[13] + (buffer[12] << 8);
                        inst->majorVersion = buffer[11] + (buffer[10] << 8);
                        inst->ranges = (char*)&buffer[166];
                        instances.push_back(inst);

                        std::string fullAddress = "http://" + inst->ipAddress + "/fppjson.php?command=getFPPSystems";
                        CurlData *data = new CurlData(fullAddress);
                        data->type = 0;
                        data->setFPP(inst);
                        curls.push_back(data);
                        curl_multi_add_handle(curlMulti, data->curl);
                        running++;

                        fullAddress = "http://" + inst->ipAddress + "/fppjson.php?command=getSysInfo&simple";
                        data = new CurlData(fullAddress);
                        data->type = 1;
                        data->setFPP(inst);
                        curls.push_back(data);
                        curl_multi_add_handle(curlMulti, data->curl);
                        running++;

                        fullAddress = "http://" + inst->ipAddress + "/api/proxies";
                        data = new CurlData(fullAddress);
                        data->type = 4;
                        data->setFPP(inst);
                        curls.push_back(data);
                        curl_multi_add_handle(curlMulti, data->curl);
                        running++;
                    }
                }
            } else {
                std::string address = toIp(&buffer[32]);
                bool found = false;
                for (const auto& f : instances) {
                    if (f->ipAddress == address) {
                        found = true;
                    }
                }
                if (found) {
                    continue;
                }

                FPP *fpp = new FPP();
                fpp->setIPAddress(address);
                fpp->hostName = address;
                instances.push_back(fpp);


                std::string fullAddress = "http://" + address + "/";
                CurlData *data = new CurlData(fullAddress);
                data->type = 5;
                data->setFPP(fpp);
                curls.push_back(data);
                curl_multi_add_handle(curlMulti, data->curl);
                running++;
            }
        } else {
            int start = running;
            curl_multi_perform(curlMulti, &running);
            if (start != running) {
                struct CURLMsg *m;
                int msgq = 0;
                while ((m = curl_multi_info_read(curlMulti, &msgq))) {
                    if (m->msg == CURLMSG_DONE) {
                        CURL *e = m->easy_handle;
                        long response_code = 0;
                        curl_easy_getinfo(e, CURLINFO_HTTP_CODE, &response_code);
                        for (int x = 0; x < curls.size(); x++) {
                            if (curls[x] && curls[x]->curl == e) {
                                logger_base.info("FPP Discovery %s - RC: %d", curls[x]->url.c_str(), response_code);
                                /*
                                if (curls[x]->errorBuffer[0]) {
                                    printf("error:  %s    %d\n      %s\n", curls[x]->url.c_str(), curls[x]->type, curls[x]->errorBuffer);
                                } else {
                                    printf("OK:  %s     %d\n      %s\n", curls[x]->url.c_str(), curls[x]->type, curls[x]->buffer.c_str());
                                }
                                */
                                wxJSONValue origJson;
                                wxJSONReader reader;
                                bool parsed = true;
                                AddTraceMessage("Received curl response: " + std::to_string(curls[x]->type) + " - " + std::to_string(response_code));
                                AddTraceMessage("    url: " + curls[x]->url);

                                if (response_code == 401 && curls[x]->authenticated != 1) {
                                    bool handled = false;
                                    std::string ip = curls[x]->ipAddress;
                                    if (ip == "") {
                                        ip = curls[x]->url;
                                    }
                                    if (curls[x]->fpp && curls[x]->authenticated != 2) {
                                        ip = curls[x]->fpp->ipAddress;
                                        if (xlPasswordEntryDialog::GetStoredPasswordForService(curls[x]->fpp->ipAddress, curls[x]->fpp->username, curls[x]->fpp->password)) {
                                            if (curls[x]->fpp->password != "") {
                                                CurlData *data = new CurlData(curls[x]->url);
                                                data->type = curls[x]->type;
                                                data->setFPP(curls[x]->fpp);
                                                data->authenticated = 2;
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                curls[x]->authenticated = 2;
                                                running++;
                                                handled = true;
                                            }
                                        }
                                    } else if (curls[x]->authenticated != 2) {
                                        if (xlPasswordEntryDialog::GetStoredPasswordForService(curls[x]->ipAddress, curls[x]->username, curls[x]->password)) {
                                            if (curls[x]->password != "") {
                                                CurlData *data = new CurlData(curls[x]->url);
                                                data->type = curls[x]->type;
                                                data->ipAddress = curls[x]->ipAddress;
                                                data->setUsernamePassword(curls[x]->username, curls[x]->password);
                                                data->authenticated = 2;
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                curls[x]->authenticated = 2;
                                                running++;
                                                handled = true;
                                            }
                                        }
                                    }
                                    if (!handled) {
                                        if (curls[x]->fpp) {
                                            ip = curls[x]->fpp->ipAddress;
                                        }
                                        xlPasswordEntryDialog dlg(nullptr, "Password needed to connect to " + ip, "Password Required");
                                        int rc = dlg.ShowModal();
                                        if (rc != wxID_CANCEL) {
                                            std::string username = "admin";
                                            std::string password = dlg.GetValue().ToStdString();
                                            if (dlg.shouldSavePassword()) {
                                                xlPasswordEntryDialog::StorePasswordForService(ip, username, password);
                                            }
                                            if (curls[x]->fpp) {
                                                curls[x]->fpp->username = username;
                                                curls[x]->fpp->password = password;
                                                CurlData *data = new CurlData(curls[x]->url);
                                                data->type = curls[x]->type;
                                                data->setFPP(curls[x]->fpp);
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                running++;
                                            } else {
                                                CurlData *data = new CurlData(curls[x]->url);
                                                data->type = curls[x]->type;
                                                data->setUsernamePassword(username, password);
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                running++;
                                            }
                                        }
                                    }
                                } else if (response_code > 202) {
                                    logger_base.warn("Received response code %d from %s - %s", response_code, curls[x]->url.c_str(), curls[x]->errorBuffer);
                                }

                                switch (curls[x]->type) {
                                    case 1: {
                                        parsed = reader.Parse(curls[x]->buffer, &origJson) == 0;
                                        if (parsed) {
                                            curls[x]->fpp->parseSysInfo(origJson);
                                            if (!origJson["channelRanges"].IsNull()) {
                                                std::string r = origJson["channelRanges"].AsString().ToStdString();
                                                if (r.size() > curls[x]->fpp->ranges.size()) {
                                                    curls[x]->fpp->ranges = r;
                                                }
                                            }
                                            if (!origJson["Mode"].IsNull()) {
                                                std::string mode = origJson["Mode"].AsString();
                                                if (mode != "") {
                                                    curls[x]->fpp->curMode = mode;
                                                }
                                            }

                                            std::string file = "co-pixelStrings";
                                            if (curls[x]->fpp->platform.find("Beagle") != std::string::npos) {
                                                file = "co-bbbStrings";
                                            }
                                            std::string baseUrl = "http://" + curls[x]->fpp->ipAddress;
                                            if (curls[x]->fpp->proxy != "") {
                                                baseUrl = "http://" + curls[x]->fpp->proxy + "/proxy/" + curls[x]->fpp->ipAddress;
                                            }
                                            std::string fullAddress = baseUrl + "/fppjson.php?command=getChannelOutputs&file=" + file;
                                            CurlData *data = new CurlData(fullAddress);
                                            data->type = 2;
                                            data->setFPP(curls[x]->fpp);
                                            curls.push_back(data);
                                            curl_multi_add_handle(curlMulti, data->curl);
                                            running++;

                                            fullAddress = baseUrl + "/fppjson.php?command=getChannelOutputs&file=channelOutputsJSON";
                                            data = new CurlData(fullAddress);
                                            data->type = 2;
                                            data->setFPP(curls[x]->fpp);
                                            curls.push_back(data);
                                            curl_multi_add_handle(curlMulti, data->curl);
                                            running++;

                                            fullAddress = baseUrl + "/config.php";
                                            data = new CurlData(fullAddress);
                                            data->type = 3;
                                            data->setFPP(curls[x]->fpp);
                                            curls.push_back(data);
                                            curl_multi_add_handle(curlMulti, data->curl);
                                            running++;

                                            if (curls[x]->fpp->proxy == "") {
                                                fullAddress = baseUrl + "/api/proxies";
                                                data = new CurlData(fullAddress);
                                                data->type = 4;
                                                data->setFPP(curls[x]->fpp);
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                running++;
                                            }
                                        }
                                        break;
                                    }
                                    case 2:
                                        parsed = reader.Parse(curls[x]->buffer, &origJson) == 0;
                                        if (parsed) curls[x]->fpp->parseControllerType(origJson);
                                        break;
                                    case 3:
                                        curls[x]->fpp->parseConfig(curls[x]->buffer);
                                        break;
                                    case 4:
                                        parsed = reader.Parse(curls[x]->buffer, &origJson) == 0;
                                        if (parsed) curls[x]->fpp->parseProxies(origJson);
                                        for (const auto& address : curls[x]->fpp->proxies) {
                                            bool found = false;
                                            for (const auto& f : instances) {
                                                if (f->ipAddress == address
                                                    && f->proxy == curls[x]->fpp->ipAddress) {
                                                    found = true;
                                                }
                                            }
                                            if (found) {
                                                continue;
                                            }

                                            FPP *fpp = new FPP();
                                            fpp->setIPAddress(address);
                                            fpp->hostName = address;
                                            fpp->proxy = curls[x]->fpp->ipAddress;
                                            fpp->username = curls[x]->fpp->username;
                                            fpp->password = curls[x]->fpp->password;
                                            curls[x]->fpp->isFPP = false;
                                            instances.push_back(fpp);


                                            std::string fullAddress = "http://" + curls[x]->fpp->ipAddress + "/proxy/" + address + "/";
                                            CurlData *data = new CurlData(fullAddress);
                                            data->type = 5;
                                            data->setFPP(fpp);
                                            curls.push_back(data);
                                            curl_multi_add_handle(curlMulti, data->curl);
                                            running++;
                                        }
                                        break;
                                    case 5: //  just the root HTTP page, likely hard to figure out?
                                        if (response_code == 200) {
                                            if (curls[x]->buffer.find("SanDevices SACN") != std::string::npos && curls[x]->buffer.find("Pixel Controller") != std::string::npos) {
                                                //likely SanDevices
                                                SanDevices sand(curls[x]->fpp->ipAddress, curls[x]->fpp->proxy);
                                                if (sand.IsConnected()) {
                                                    curls[x]->fpp->pixelControllerType = sand.GetFullName();
                                                    curls[x]->fpp->model = sand.GetModel();
                                                    curls[x]->fpp->fullVersion = sand.GetVersion();
                                                    curls[x]->fpp->platform = "SanDevices";
                                                    curls[x]->fpp->mode = "bridge";
                                                    curls[x]->fpp->isFPP = false;
                                                }
                                            } else if (curls[x]->buffer.find("Falcon Player - FPP") != std::string::npos) {
                                                curls[x]->fpp->isFPP = true;
                                                std::string fullAddress = "http://" + curls[x]->fpp->proxy + "/proxy/" + curls[x]->fpp->ipAddress + "/fppjson.php?command=getSysInfo&simple";
                                                CurlData *data = new CurlData(fullAddress);
                                                data->type = 1;
                                                data->setFPP(curls[x]->fpp);
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                running++;
                                            } else if (curls[x]->buffer.find("pixelcontroller.com") != std::string::npos && curls[x]->buffer.find("Controller Information") != std::string::npos) {
                                                Falcon falc(curls[x]->fpp->ipAddress, curls[x]->fpp->proxy);
                                                if (falc.IsConnected()) {
                                                    if (falc.GetName() != "") {
                                                        curls[x]->fpp->hostName = falc.GetName();
                                                        curls[x]->fpp->description = falc.GetName();
                                                    }
                                                    curls[x]->fpp->pixelControllerType = falc.GetModel();
                                                    curls[x]->fpp->model = falc.GetModel();
                                                    curls[x]->fpp->fullVersion = falc.GetFullName();
                                                    curls[x]->fpp->platform = "Falcon";

                                                    curls[x]->fpp->mode = "bridge";
                                                    int idx = curls[x]->buffer.find("name=\"m\" ");
                                                    if (idx > 0) {
                                                        idx = curls[x]->buffer.find("value=\"", idx);
                                                        if (idx > 0) {
                                                            idx += 7;
                                                            if (curls[x]->buffer[idx] != '0') {
                                                                curls[x]->fpp->mode = "unknown";
                                                            }
                                                        }
                                                    }

                                                    curls[x]->fpp->isFPP = false;
                                                }
                                            } else {
                                                //there is a web server on port 80 running, lets see if we can determine what this is
                                                FPP *fpp = curls[x]->fpp;
                                                std::string fullAddress = "http://" + curls[x]->fpp->proxy + "/proxy/" + curls[x]->fpp->ipAddress + "/status.xml";
                                                CurlData *data = new CurlData(fullAddress);
                                                data->type = 6;
                                                data->setFPP(fpp);
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                running++;

                                                fullAddress = "http://" + curls[x]->fpp->proxy + "/proxy/" + curls[x]->fpp->ipAddress + "/H?";
                                                data = new CurlData(fullAddress);
                                                data->type = 7;
                                                data->setFPP(fpp);
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                running++;

                                                fullAddress = "http://" + curls[x]->fpp->proxy + "/proxy/" + curls[x]->fpp->ipAddress + "/sysinfo.htm";
                                                data = new CurlData(fullAddress);
                                                data->type = 8;
                                                data->setFPP(fpp);
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                running++;
                                            }
                                        } else {
                                            curls[x]->fpp->isFPP = false;
                                        }
                                        break;
                                    case 6: // /status.xml found, likely Falcon
                                        if (response_code == 200) {
                                            Falcon falc(curls[x]->fpp->ipAddress, curls[x]->fpp->proxy);
                                            if (falc.IsConnected()) {
                                                if (falc.GetName() != "") {
                                                    curls[x]->fpp->hostName = falc.GetName();
                                                    curls[x]->fpp->description = falc.GetName();
                                                }
                                                curls[x]->fpp->pixelControllerType = falc.GetModel();
                                                curls[x]->fpp->model = falc.GetModel();
                                                curls[x]->fpp->fullVersion = falc.GetFullName();
                                                curls[x]->fpp->platform = "Falcon";
                                                curls[x]->fpp->mode = "bridge";
                                                curls[x]->fpp->isFPP = false;
                                            }
                                        }
                                        break;
                                    case 7: // /H? - likely v5 sandevice
                                        if (response_code == 200) {
                                            SanDevices sand(curls[x]->fpp->ipAddress, curls[x]->fpp->proxy);
                                            if (sand.IsConnected()) {
                                                curls[x]->fpp->pixelControllerType = sand.GetFullName();
                                                curls[x]->fpp->model = sand.GetModel();
                                                curls[x]->fpp->fullVersion = sand.GetVersion();
                                                curls[x]->fpp->platform = "SanDevices";
                                                curls[x]->fpp->mode = "bridge";
                                                curls[x]->fpp->isFPP = false;
                                            }
                                        }
                                        break;
                                    case 8: // /sysinfo.htm - for j1sys
                                        if (response_code == 200) {
                                            J1Sys j1sys(curls[x]->fpp->ipAddress, curls[x]->fpp->proxy);
                                            if (j1sys.IsConnected()) {
                                                curls[x]->fpp->pixelControllerType = j1sys.GetFullName();
                                                curls[x]->fpp->model = j1sys.GetModel();
                                                curls[x]->fpp->fullVersion = j1sys.GetVersion();
                                                curls[x]->fpp->platform = "J1Sys";
                                                curls[x]->fpp->mode = "bridge";
                                                curls[x]->fpp->isFPP = false;
                                            }
                                        }
                                        break;
                                    default:
                                        if (response_code == 200) {
                                            parsed = reader.Parse(curls[x]->buffer, &origJson) == 0;
                                            for (int x = 0; x < origJson.Size(); x++) {
                                                wxJSONValue system = origJson[x];
                                                wxString address = system["IP"].AsString();
                                                wxString hostName = system["HostName"].IsNull() ? "" : system["HostName"].AsString();
                                                if (address == "null" || hostName == "null") {
                                                    continue;
                                                }
                                                if (address.length() > 16) {
                                                    //ignore for some reason, FPP is occassionally returning an IPV6 address
                                                    continue;
                                                }
                                                FPP *found = nullptr;
                                                for (const auto &b : instances) {
                                                    if (b->ipAddress == address) {
                                                        found = b;
                                                    } else if (b->ipAddress == hostName || (b->hostName == hostName && b->ipAddress == "")) {
                                                        found = b;
                                                    }
                                                }
                                                FPP inst;
                                                inst.hostName = hostName;
                                                if (!system["Platform"].IsNull()) {
                                                    inst.platform = system["Platform"].AsString();
                                                }
                                                if (inst.platform.find("xLights") != std::string::npos) {
                                                    continue;
                                                }
                                                if (inst.platform.find("Falcon ") != std::string::npos) {
                                                    continue;
                                                }
                                                if (!allPlatforms) {
                                                    if (inst.platform.find("Unknown") != std::string::npos) {
                                                        continue;
                                                    }
                                                    if (inst.platform.find("unknown") != std::string::npos) {
                                                        continue;
                                                    }
                                                }
                                                if (!system["model"].IsNull()) {
                                                    inst.model = system["model"].AsString();
                                                }
                                                inst.setIPAddress(address);
                                                if (!system["version"].IsNull()) {
                                                    inst.fullVersion = system["version"].AsString();
                                                }
                                                if (system["minorVersion"].IsInt()) {
                                                    inst.minorVersion = system["minorVersion"].AsInt();
                                                }
                                                if (system["majorVersion"].IsInt()) {
                                                    inst.majorVersion = system["majorVersion"].AsInt();
                                                }
                                                if (!system["channelRanges"].IsNull()) {
                                                    inst.ranges = system["channelRanges"].AsString();
                                                }
                                                if (!system["HostDescription"].IsNull()) {
                                                    inst.description = system["HostDescription"].AsString();
                                                }
                                                if (!system["fppMode"].IsNull()) {
                                                    inst.mode = system["fppMode"].AsString();
                                                }
                                                if (found) {
                                                    if (found->majorVersion == 0) {
                                                        *found = inst;
                                                    } else {
                                                        if (found->platform == "") {
                                                            found->platform = inst.platform;
                                                        }
                                                        if (found->mode == "") {
                                                            found->mode = inst.mode;
                                                        }
                                                        if (found->model == "") {
                                                            found->model = inst.model;
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
                                                    FPP *fpp = new FPP(inst);
                                                    instances.push_back(fpp);

                                                    std::string fullAddress = "http://" + fpp->ipAddress + "/fppjson.php?command=getFPPSystems";
                                                    if (fpp->ipAddress == "") {
                                                        fullAddress = "http://" + fpp->hostName + "/fppjson.php?command=getFPPSystems";
                                                    }
                                                    CurlData *data = new CurlData(fullAddress);
                                                    data->type = 0;
                                                    data->setFPP(fpp);
                                                    curls.push_back(data);
                                                    curl_multi_add_handle(curlMulti, data->curl);
                                                    running++;

                                                    fullAddress = "http://" + fpp->ipAddress + "/fppjson.php?command=getSysInfo&simple";
                                                    if (fpp->ipAddress == "") {
                                                        fullAddress = "http://" + fpp->hostName + "/fppjson.php?command=getSysInfo&simple";
                                                    }
                                                    data = new CurlData(fullAddress);
                                                    data->type = 1;
                                                    data->setFPP(fpp);
                                                    curls.push_back(data);
                                                    curl_multi_add_handle(curlMulti, data->curl);
                                                    running++;

                                                }
                                            }
                                        }
                                }
                                curl_multi_remove_handle(curlMulti, e);
                                e = nullptr;
                                delete curls[x];
                                curls[x] = nullptr;
                            }
                        }
                        if (e) {
                            curl_multi_remove_handle(curlMulti, e);
                        }
                    }
                }
            }
        }
    }
    for (const auto& data : curls) {
        if (data) {
            AddTraceMessage("Still had a curl outstanding");
            curl_multi_remove_handle(curlMulti, data->curl);
            delete data;
        }
    }
    AddTraceMessage("Closing network resources");
    curl_multi_cleanup(curlMulti);

    for (const auto& socket : sockets) {
        socket->Close();
        delete socket;
    }
    AddTraceMessage("Checking instances and applying fixups");

    std::list<FPP*> toRemove;
    for (const auto& a : instances) {
        //apply some fixups
        if (a->platform == "") {
            a->platform = a->model;
        }
        if (a->platform == "ESPixelStick") {
            a->pixelControllerType = "ESPixelStick";
        }
        if (!a->isFPP) {
            if (!allPlatforms || a->pixelControllerType == "") {
                FPP *f = a;
                toRemove.push_back(f);
            }
        }
    }
    for (const auto& a : toRemove) {
        instances.remove(a);
        delete a;
    }
    AddTraceMessage("Done discovery");

    /*
    for (const auto& a : instances) {
        printf("%s/%s:\n", a.hostName.c_str(), a.ipAddress.c_str());
        printf("    version: %s    %d.%d\n", a.fullVersion.c_str(), a.majorVersion, a.minorVersion);
        printf("    platform: %s\n", a.platform.c_str());
        printf("    model: %s\n", a.model.c_str());
        printf("    ranges: %s\n", a.ranges.c_str());
    }
    */
}

void FPP::Probe(const std::list<std::string> &addresses, std::list<FPP*> &instances) {
    Discover(addresses, instances, false);
}
