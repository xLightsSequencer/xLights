#define NOMINMAX

#include <map>
#include <string.h>

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

#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"

#include "FPP.h"
#include "../xLightsXmlFile.h"
#include "../models/Model.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../outputs/E131Output.h"
#include "../outputs/DDPOutput.h"
#include "../UtilFunctions.h"
#include "../xLightsVersion.h"
#include "../Parallel.h"

#include <log4cpp/Category.hh>
#include "ControllerRegistry.h"
#include "ControllerUploadData.h"
#include "../FSEQFile.h"


#include "Falcon.h"
#include "SanDevices.h"
#include "J1Sys.h"



static std::map<std::string, PixelCapeInfo> CONTROLLER_TYPE_MAP = {
    {"PiHat", PixelCapeInfo("PiHat", 2, 0)},
    {"F8-B", PixelCapeInfo("F8-B", "F8-B (8 serial)", 12, 8)},
    {"F8-B-16", PixelCapeInfo("F8-B-16", "F8-B (4 serial)", 16, 4)},
    {"F8-B-20", PixelCapeInfo("F8-B-20", "F8-B (No serial)", 20, 0)},
    {"F8-B-EXP", PixelCapeInfo("F8-B-EXP", "F8-B w/ Expansion (8 serial)", 28, 8, {{13, 16}})},
    {"F8-B-EXP-32", PixelCapeInfo("F8-B-EXP-32", "F8-B w/ Expansion (4 serial)", 32, 4, {{17, 16}})},
    {"F8-B-EXP-36", PixelCapeInfo("F8-B-EXP-36", "F8-B w/ Expansion (No serial)", 36, 0, {{21, 16}})},
    {"F16-B", PixelCapeInfo("F16-B", "F16-B", 16, 8)},
    {"F16-B-32", PixelCapeInfo("F16-B-32", "F16-B w/ 32 outputs", 32, 8, {{17, 16}})},
    {"F16-B-48", PixelCapeInfo("F16-B-48", "F16-B w/ 48 outputs (No Serial)", 48, 0, {{17, 16}, {33, 16}})},
    {"F4-B", PixelCapeInfo("F4-B", "F4-B", 4, 1)},
    {"F32-B", PixelCapeInfo("F32-B", "F32-B (8 Serial)", 40, 8)},
    {"F32-B-44", PixelCapeInfo("F32-B-44", "F32-B (4 Serial)", 44, 4)},
    {"F32-B-48", PixelCapeInfo("F32-B-48", "F32-B (No Serial)", 48, 0)},
    {"F40D-PB", PixelCapeInfo("F40D-PB", "F40D-PB (8 Serial)", 32, 8)},
    {"F40D-PB-36", PixelCapeInfo("F40D-PB-36", "F40D-PB (4 Serial)", 36, 4)},
    {"F40D-PB-40", PixelCapeInfo("F40D-PB-40", "F40D-PB (No Serial)", 40, 0)},
    {"PB16", PixelCapeInfo("PB16", "PB16 (2 serial)", 16, 2)},
    {"PB16-EXP", PixelCapeInfo("PB16-EXP", "PB16 w/ Expansion (2 serial)", 32, 2, {{17, 16}})},
    {"RGBCape24", PixelCapeInfo("RGBCape24", "RGBCape24", 48, 0, {{1, 8}, {9, 8}, {17, 8}})},
    {"RGBCape48C", PixelCapeInfo("RGBCape48C", "RGBCape48C", 48, 0, {{1, 8}, {9, 8}, {17, 8}, {25, 8}, {33, 8}, {41, 8}})},
    {"RGBCape48F", PixelCapeInfo("RGBCape48F", "RGBCape48F", 48, 0, {{1, 8}, {9, 8}, {17, 8}, {25, 8}, {33, 8}, {41, 8}})},
    {"LED Panels", PixelCapeInfo("LED Panels", 0, 0)}
};

const std::string &FPP::PixelContollerDescription() const {
    if (CONTROLLER_TYPE_MAP.find(pixelControllerType) == CONTROLLER_TYPE_MAP.end()) {
        return pixelControllerType;
    }
    return CONTROLLER_TYPE_MAP[pixelControllerType].description;
}
void FPP::RegisterCapes() {
    for (auto &a : CONTROLLER_TYPE_MAP) {
        ControllerRegistry::AddController(&a.second);
    }
}


PixelCapeInfo& FPP::GetCapeRules(const std::string& type)
{
    return CONTROLLER_TYPE_MAP.find(type) != CONTROLLER_TYPE_MAP.end() ? CONTROLLER_TYPE_MAP[type] : CONTROLLER_TYPE_MAP["PiHat"];
}

FPP::FPP(const std::string &ad) : majorVersion(0), minorVersion(0), outputFile(nullptr), parent(nullptr), ipAddress(ad), curl(nullptr), isFPP(true) {
    wxIPV4address address;
    if (address.Hostname(ad)) {
        hostName = ad;
        ipAddress = address.IPAddress();
    }
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

static int buffer_writer(char *data, size_t size, size_t nmemb,
                         std::string *writerData) {
    if (writerData == NULL) {
        return 0;
    }
    writerData->append(data, size * nmemb);
    return size * nmemb;
}
class FPPWriteData {
public:
    FPPWriteData() : file(nullptr), progress(nullptr), data(nullptr), dataSize(0), curPos(0),
        postData(nullptr), postDataSize(0), totalWritten(0), cancelled(false), compressed(false), lastDone(0) {}
    
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
    bool compressed;
    
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
                donePct *= compressed ? 600 : 1000;
                donePct /= file->Length();
                if (compressed) {
                    donePct += 333;
                }
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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, buffer_writer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlInputBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 100);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000);
}
bool FPP::GetURLAsString(const std::string& url, std::string& val)  {
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
    int i = curl_easy_perform(curl);
    if (i == CURLE_OK) {
        val = curlInputBuffer;
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 401) {
            curlInputBuffer.clear();
            wxPasswordEntryDialog dlg(nullptr, "Password needed to connect to " + ipAddress, "Password Required");
            int rc = dlg.ShowModal();
            if (rc == wxID_CANCEL) {
                return false;
            }
            username = "admin";
            password = dlg.GetValue().ToStdString();
            return GetURLAsString(url, val);
        }
        retValue = (response_code == 200);
    }
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, nullptr);
    return retValue;

}
int FPP::PostToURL(const std::string& url, const wxMemoryBuffer &val, const std::string &contentType) {
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
    if (i == CURLE_OK) {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        return response_code;
    }
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
                pixelControllerType = "PiHat";
            }
        } else if (val["channelOutputs"][x]["type"].AsString() == "BBB48String") {
            if (val["channelOutputs"][x]["enabled"].AsInt()) {
                pixelControllerType = val["channelOutputs"][x]["subType"].AsString();
            }
        } else if (val["channelOutputs"][x]["type"].AsString() == "LEDPanelMatrix") {
            pixelControllerType = "LED Panels";
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

int FPP::PostToURL(const std::string& url, const std::string &val, const std::string &contentType) {
    wxMemoryBuffer memBuffPost;
    addString(memBuffPost, val);
    return PostToURL(url, memBuffPost, contentType);
}



bool FPP::uploadFile(const std::string &filename, const std::string &file, bool compress)  {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static int bufLen = 1024*1024*4; //4MB buffer

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
    wxProgressDialog progress("FPP Upload", "Transferring " + filename + " to " + ipAddress, 1000, parent, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    logger_base.debug("FPP upload via http of %s.", (const char*)filename.c_str());
    progress.Update(0, "Transferring " + filename + " to " + ipAddress, &cancelled);
    int lastDone = 0;

    std::string ct = "Content-Type: application/octet-stream";
    bool deleteFile = false;
    std::string fullFileName = file;
    if (compress) {
        logger_base.debug("Uploading it compressed.");
        //determine size of gzipped data
        progress.Show();
        unsigned char *rbuf = new unsigned char[bufLen];
        wxFile f_in(file);
        
        fullFileName = file + ".gz";
        deleteFile = true;
        
        wxFileOutputStream f_out(fullFileName);
        wxZlibOutputStream zlib(f_out, wxZ_DEFAULT_COMPRESSION, wxZLIB_GZIP);
        size_t total = f_in.Length();
        size_t read = 0;
        while (!f_in.Eof()) {
            size_t t = f_in.Read(rbuf, bufLen);
            zlib.WriteAll(rbuf, t);
            read += t;

            size_t f = read;
            f *= 1000;
            f /= total;
            f /= 3;
            if (f != lastDone) {
                lastDone = f;
                cancelled = !progress.Update(f, "Compressing " + filename, &cancelled);
                wxYield();
                if (cancelled) {
                    delete [] rbuf;
                    f_in.Close();
                    return cancelled;
                }
            }
        }
        zlib.Close();
        //printf("compressed: %zu to %zu    %zu\n", total, fileLen, fileLen*100/total);
        fn += ".gz";
        ext = ".gz";
        ct = "Content-Type: application/z-gzip";
        delete [] rbuf;
    } else {
        logger_base.debug("Uploading it uncompressed.");
        compress = false;
    }
    
    
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
    data.progress = &progress;
    data.file = &fileobj;
    data.postData =  (uint8_t*)memBuffPost.GetData();
    data.postDataSize = memBuffPost.GetDataLen();
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &data);
    
    data.progress = &progress;
    data.progressString = "Transferring " + filename + " to " + ipAddress;
    data.lastDone = lastDone;
    data.compressed = compress;

    int i = curl_easy_perform(curl);
    curl_slist_free_all(chunk);
    if (deleteFile) {
        wxRemoveFile(fullFileName);
    }
    if (i == CURLE_OK) {
        long response_code;
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
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        logger_base.warn("Curl did not upload file:  %d   %s", response_code, error);
    }
    progress.Update(1000, wxEmptyString, &cancelled);
    return data.cancelled;
}


bool FPP::copyFile(const std::string &filename,
                           const std::string &file,
                           const std::string &dir) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool cancelled = false;

    wxProgressDialog progress("FPP Upload", "Transferring " + filename + " to " + ipAddress, 100, parent, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    logger_base.debug("FPP upload via file copy of %s.", (const char*)filename.c_str());
    progress.Update(0, "Transferring " + filename + " to " + ipAddress, &cancelled);
    progress.Show();
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

                int prgs = done * 100 / length;
                progress.Update(prgs, wxEmptyString, &cancelled);
                if (!cancelled) {
                    cancelled = progress.WasCancelled();
                }
            }
            progress.Update(100, wxEmptyString);
            in.Close();
            out.Close();
        } else {
            progress.Update(100, wxEmptyString);
            logger_base.warn("   Copy of file %s failed ... target file %s could not be opened.", (const char *)file.c_str(), (const char *)target.c_str());
        }
    } else {
        progress.Update(100, wxEmptyString);
        logger_base.warn("   Copy of file %s failed ... file could not be opened.", (const char *)file.c_str());
    }
    return cancelled;
}
bool FPP::uploadOrCopyFile(const std::string &filename,
                                   const std::string &file,
                                   bool compress,
                                   const std::string &dir) {
    if (IsDrive()) {
        return copyFile(filename, file, dir);
    }
    return uploadFile(filename, file, compress);
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

        cancelled |= uploadOrCopyFile(mediaBaseName, media, false, "music");
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
    uploadCompressed = type == 0 && IsVersionAtLeast(2, 5);  // no sense compressing V2 as they are already compressed, just wastes time
    if ((type == 0 && file.getVersionMajor() == 1)
        || fn.GetExt() == "eseq") {

        //these just get uploaded directly
        return uploadOrCopyFile(baseName, seq, uploadCompressed, fn.GetExt() == "eseq" ? "effects" : "sequences");
    }

    if (type == 1 && file.getVersionMajor() == 2) {
        // Full v2 file, upload directly
        return uploadOrCopyFile(baseName, seq, false, fn.GetExt() == "eseq" ? "effects" : "sequences");
    }
    baseSeqName = baseName;
    outputFile = FSEQFile::createFSEQFile(fileName, type == 0 ? 1 : 2);
    outputFile->initializeFromFSEQ(file);
    if (type == 2 && ranges != "") {
        wxArrayString r1 = wxSplit(wxString(ranges), ',');
        for (auto a : r1) {
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
            cancelled = uploadOrCopyFile(baseSeqName, tempFileName, uploadCompressed, "sequences");
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

    for (auto info : sequences) {
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


    if (IsDrive()) {
        WriteJSONToPath(fn, origJson);
    } else {
        PostJSONToURL("/api/playlist/" + URLEncode(name), origJson);
    }
    return false;
}

bool FPP::UploadModels(const std::string &models) {
    if (IsDrive()) {
        wxFileName fn = (ipAddress + wxFileName::GetPathSeparator() + "channelmemorymaps");
        wxFile tf(fn.GetFullPath());
        tf.Write(models);
        tf.Close();
    } else if (IsVersionAtLeast(2, 6)) {
        PostToURL("/api/models/raw", models);
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

std::string FPP::CreateModelMemoryMap(ModelManager* allmodels) {
    std::string ret;
    for (auto m = allmodels->begin(); m != allmodels->end(); ++m) {
        Model* model = m->second;
        wxString stch = model->GetModelXml()->GetAttribute("StartChannel", wxString::Format("%d?", model->NodeStartChannel(0) + 1)); //NOTE: value coming from model is probably not what is wanted, so show the base ch# instead
        int ch = model->GetNumberFromChannelString(model->ModelStartChannel);
        wxString name(model->name);
        name.Replace(" ", "_");
        if (model->GetNumStrands() > 0) {
            ret += wxString::Format("%s,%i,%lu,horizontal,TL,%i,%i\n",
                                                     name.c_str(),
                                                     ch,
                                                     (unsigned long)model->GetActChanCount(),
                                                     (int)model->GetNumStrands(),
                                                     1).ToStdString();
        }
    }
    return ret;
}

inline wxString stripInvalidChars(const std::string &str) {
    wxString s = str;
    s.Replace("&", "_");
    s.Replace("<", "_");
    s.Replace(">", "_");
    return s;
}

void FPP::FillRanges(std::map<int, int> &rngs) {
    if (ranges != "") {
        wxArrayString r1 = wxSplit(wxString(ranges), ',');
        for (auto a : r1) {
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
    for (auto &a : rngs) {
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
    std::list<int> selected;
    for (int x = 0; x < outputManager->GetOutputCount(); x++) {
        std::string px = outputManager->GetOutput(x)->GetFPPProxyIP();
        if (px == hostName || px == ipAddress) {
            selected.push_back(x);
        }
    }
    wxJSONValue f = CreateUniverseFile(outputManager, "-selected-", selected, false);
    
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

wxJSONValue FPP::CreateOutputUniverseFile(OutputManager* outputManager) {
    std::list<int> selected;
    return CreateUniverseFile(outputManager, "", selected, false);
}
wxJSONValue FPP::CreateUniverseFile(OutputManager* outputManager, const std::string &onlyip, const std::list<int>& selected, bool input) {
    wxJSONValue root;
    root["type"] = wxString("universes");
    root["enabled"] = 1;
    root["startChannel"] = 1;
    root["channelCount"] = -1;

    wxJSONValue universes;
    // Get universes based on IP
    std::list<Output*> outputs = outputManager->GetAllOutputs(onlyip, "", selected, false);
    for (auto it = outputs.begin(); it != outputs.end(); ++it) {
        int c = (*it)->GetStartChannel();

        wxJSONValue universe;
        universe["active"] = (*it)->IsEnabled() ? 1 : 0;
        universe["description"] = stripInvalidChars((*it)->GetDescription());
        universe["id"] = (*it)->GetUniverse();
        universe["startChannel"] = c;
        universe["channelCount"] = (*it)->GetChannels();
        universe["priority"] = 0;
        universe["address"] = wxString("");

        if ((*it)->GetType() == OUTPUT_E131) {
            universe["type"] = (int)((*it)->GetIP() != "MULTICAST" ? 1 : 0);
            if (!input && ((*it)->GetIP() != "MULTICAST")) {
                universe["address"] = wxString((*it)->GetIP());
            }
            if ((*it)->IsOutputCollection()) {
                universe["universeCount"] = ((E131Output*)(*it))->GetUniverses();
            } else {
                universe["universeCount"] = 1;
            }
            universes.Append(universe);
        } else if ((*it)->GetType() == OUTPUT_DDP) {
            if (!input) {
                universe["address"] = wxString((*it)->GetIP());
                DDPOutput *ddp = (DDPOutput*)(*it);
                universe["type"] = ddp->IsKeepChannelNumbers() ? 4 : 5;
                universes.Append(universe);
            } else {
                //don't need to do anything to configure DDP input
            }
        } else if ((*it)->GetType() == OUTPUT_ARTNET) {
            universe["type"] = (int)(((*it)->GetIP() != "MULTICAST") + 2);
            if (!input && ((*it)->GetIP() != "MULTICAST")) {
                universe["address"] = wxString((*it)->GetIP());
            }
            universes.Append(universe);
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
    return GetURLAsString("/fppjson.php?command=setSetting&key=restartFlag&value=1", val);
}

void FPP::SetDescription(const std::string &st) {
    std::string val;
    GetURLAsString("/fppjson.php?command=setSetting&key=HostDescription&value=" + URLEncode(st), val);
}


bool FPP::SetInputUniversesBridge(std::list<int>& selected, OutputManager* outputManager) {
    wxJSONValue udp = CreateUniverseFile(outputManager, ipAddress, selected, true);
    if (IsDrive()) {
        std::string fn = (ipAddress + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + "ci-universes.json");
        WriteJSONToPath(fn, udp);
    } else if (IsVersionAtLeast(2, 4)) {
        PostJSONToURLAsFormData("/fppjson.php", "command=setChannelOutputs&file=universeInputs", udp);
    }
    return false;
}

bool FPP::UploadPixelOutputs(ModelManager* allmodels,
                             OutputManager* outputManager,
                             const std::list<int>& selected) {

    int maxString = 1;
    int maxdmx = 0;
    PixelCapeInfo &rules = GetCapeRules(pixelControllerType);
    maxdmx = rules.maxDMX;
    maxString = rules.maxStrings;
    
    std::map<int, int> rngs;
    FillRanges(rngs);

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("FPP Outputs Upload: Uploading to %s", (const char *)ipAddress.c_str());

    std::string check;
    UDController cud(ipAddress, hostName, allmodels, outputManager, &selected, check);
    if (rules.SupportsLEDPanelMatrix()) {
        //LED panel cape, nothing we can really do except update the start channel
        int startChannel = -1;
        if (cud.GetMaxPixelPort()) {
            //The matrix actually has the controller connection defined, we'll use it
            startChannel = cud.GetControllerPixelPort(0)->GetStartChannel();
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
            for (int x = 0; x < origJson["channelOutputs"].Size(); x++) {
                if (origJson["channelOutputs"][x]["type"].AsString() == "LEDPanelMatrix") {
                    origJson["channelOutputs"][x]["startChannel"] = startChannel;
                    rngs[startChannel - 1] = origJson["channelOutputs"][x]["channelCount"].AsLong();
                }
            }

            if (IsDrive()) {
                WriteJSONToPath(ipAddress + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + "channeloutputs.json", origJson);
            } else {
                PostJSONToURLAsFormData("/fppjson.php", "command=setChannelOutputs&file=channelOutputsJSON", origJson);
            }
        }
        SetNewRanges(rngs);
        return false;
    }

    std::string fppFileName = "co-bbbStrings";
    int minPorts = 1;
    if (pixelControllerType == "PiHat") {
        fppFileName = "co-pixelStrings";
        minPorts = 2;
    }
    cud.Check(&rules, check);
    cud.Dump();

    wxFileName fnOrig;
    fnOrig.AssignTempFileName("pixelOutputs");
    std::string file = fnOrig.GetFullPath().ToStdString();
    wxJSONValue origJson;
    bool doVirtualString = true;
    if (IsDrive()) {
        GetPathAsJSON(ipAddress + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + fppFileName +".json", origJson);
    } else {
        GetURLAsJSON("/fppjson.php?command=getChannelOutputs&file=" + fppFileName, origJson);
        if (IsVersionAtLeast(2, 7)) {
            wxJSONValue capeInfo;
            if (GetURLAsJSON("/api/cape", capeInfo)) {
                doVirtualString = capeInfo["id"].AsString() != "Unsupported";
            }
        }
    }
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
                for (int vs = 0; vs < f["outputs"][o]["virtualStrings"].Size(); vs++) {
                    wxJSONValue val = f["outputs"][o]["virtualStrings"][vs];
                    if (val["description"].AsString() != "") {
                        origStrings[val["description"].AsString()] = val;
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

    wxJSONValue dmxData;
    dmxData["enabled"] = 1;
    dmxData["startChannel"] = 1;
    dmxData["type"] = wxString("BBBSerial");
    dmxData["subType"] = wxString("DMX");

    maxport = cud.GetMaxPixelPort();
    if (maxport < minPorts) {
        maxport = minPorts;
    }

    if (pixelControllerType == "PiHat") {
        stringData["type"] = wxString("RPIWS281X");
        stringData["subType"] = wxString("");
    } else {
        stringData["type"] = wxString("BBB48String");
        stringData["subType"] = wxString(pixelControllerType);
        stringData["pinoutVersion"] = pinout;
    }
    stringData["outputCount"] = maxport;
    dmxData["device"] = wxString(pixelControllerType);

    for (int x = 0; x < maxport; x++) {
        wxJSONValue port;
        port["portNumber"] = x;

        stringData["outputs"].Append(port);
    }

    for (int pp = 1; pp <= rules.GetMaxPixelPort(); pp++) {
        if (cud.HasPixelPort(pp)) {
            UDControllerPort* port = cud.GetControllerPixelPort(pp);
            if (doVirtualString) {
                port->CreateVirtualStrings(false);
                for (auto pvs : port->GetVirtualStrings()) {
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
                        vs["reverse"] = pvs->_reverse;
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
            } else {
                wxJSONValue vs;
                vs["startChannel"] = port->GetStartChannel() - 1; // we need 0 based
                vs["pixelCount"] = port->Pixels();
                
                rngs[port->GetStartChannel() - 1] = port->Pixels() * 3;

                auto s = port->GetModels().front();
                std::string description;
                if (s) {
                    description = s->GetName();
                    vs["description"] = description;
                }
                if (description != "" && origStrings.find(description) != origStrings.end()) {
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
                    vs["colorOrder"] = wxString("RGB");
                    vs["nullNodes"] = 0;
                    vs["zigZag"] = 0;
                    vs["brightness"] = 100;
                    vs["gamma"] = wxString("1.0");
                }
                if (s) {
                    std::string colourOrder = s->GetColourOrder("unknown");
                    float gamma = s->GetGamma(-9999);
                    if (gamma > 0) {
                        char buf[16];
                        sprintf(buf, "%g", gamma);
                        std::string gam = buf;
                        vs["gamma"] = gam;
                    }
                    if (s->GetGroupCount(-9999) > 0) {
                        vs["groupCount"] = s->GetGroupCount(-9999);
                    }
                    if (colourOrder != "unknown") {
                        vs["colorOrder"] = colourOrder;
                    }
                    if (s->GetBrightness(-9999) > 0) {
                        vs["brightness"] = s->GetBrightness(-9999);
                    }
                    if (s->GetNullPixels(-9999) > 0) {
                        vs["nullNodes"] = s->GetNullPixels(-9999);
                    }
                    if (s->GetDirection("unknown") != "unknown") {
                        vs["reverse"] = s->GetDirection("unknown") == "Reverse" ? 1 : 0;
                    }
                }
                stringData["outputs"][port->GetPort() - 1]["virtualStrings"].Append(vs);
            }
        }
    }

    for (int x = 0; x < maxport; x++) {
        if (stringData["outputs"][x]["virtualStrings"].IsNull() || stringData["outputs"][x]["virtualStrings"].Size() == 0) {
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
    for (int x = 0; x < maxport; x++) {
        if (rules.expansionPorts.find(x+1) != rules.expansionPorts.end()) {
            int count = rules.expansionPorts.find(x+1)->second;
            int expansionType = 0;
            for (int p = 0; p < count; p++) {
                if (stringData["outputs"][x+p].HasMember("differentialType") && stringData["outputs"][x+p]["differentialType"].AsInt()) {
                    expansionType = 1;
                }
            }
            stringData["outputs"][x]["expansionType"] = 1;
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
    for (int sp = 1; sp <= rules.GetMaxSerialPort(); sp++) {
        wxJSONValue port;
        port["outputNumber"] = (sp - 1);
        port["outputType"] = isDMX ? wxString("DMX") : wxString("Pixelnet");
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
            port["channelCount"] = isDMX ? maxChan : 4096;
            hasSerial = true;
            
            rngs[sc] = isDMX ? maxChan : 4096;
        } else {
            port["startChannel"] = 0;
            port["channelCount"] = 0;
        }
        dmxData["outputs"].Append(port);
    }
    if (!doVirtualString) {
        maxChan = 0;
        isDMX = true;
        hasSerial = false;
    }
    dmxData["channelCount"] = isDMX ? maxChan : 4096;
    if (maxChan == 0) {
        dmxData["enabled"] = 0;
        dmxData["subType"] = wxString("off");
    }
    // let the string handling know if it's safe to use the other PRU
    // or if the serial out will need it
    stringData["serialInUse"] = hasSerial;

    wxJSONValue root;
    root["channelOutputs"].Append(stringData);
    if (pixelControllerType != "PiHat") {
        root["channelOutputs"].Append(dmxData);
    } else {
        wxString dev = pixelControllerType;
        dmxData["device"] = dev;
        stringData["subType"] = dev;
    }

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
    }
    SetNewRanges(rngs);

    return false;
}



class CurlData {
public:
    CurlData(const std::string &a) : url(a), type(0), fpp(nullptr) {
        errorBuffer[0] = 0;
        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, buffer_writer);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 500);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 8000);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &errorBuffer);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
    }
    ~CurlData() {
        curl_easy_cleanup(curl);
    }
    std::string url;
    CURL *curl;
    std::string buffer;
    char errorBuffer[CURL_ERROR_SIZE];
    int type;
    FPP *fpp;
};

#define FPP_CTRL_PORT 32320

//defined in xLightsMain
void AddTraceMessage(const std::string &msg);

void FPP::Discover(const std::list<std::string> &addresses, std::list<FPP*> &instances, bool doBroadcast, bool allPlatforms) {

    AddTraceMessage("Running FPP Discovery");
    std::vector<CurlData*> curls;
    CURLM * curlMulti = curl_multi_init();
    for (auto &a : addresses) {
        std::string fullAddress = "http://" + a + "/fppjson.php?command=getFPPSystems";
        CurlData *data = new CurlData(fullAddress);
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
    wxDatagramSocket *socket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST | wxSOCKET_NOWAIT);
    socket->SetTimeout(1);
    socket->Notify(false);
    if (socket->IsOk()) {
        sockets.push_back(socket);
    }
    
    auto localIPs = GetLocalIPs();
    for (auto ip : localIPs) {
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
    for (auto socket : sockets) {
        socket->SendTo(bcAddress, buffer, 207);
    }
    
    uint64_t endBroadcastTime = wxGetLocalTimeMillis().GetValue() + 1200l;
    int running = curls.size();
    while (running || (wxGetLocalTimeMillis().GetValue() < endBroadcastTime)) {
        memset(buffer, 0x00, sizeof(buffer));
        int readSize = 0;
        for (auto socket : sockets) {
            if (socket->IsOk()) {
                socket->Read(&buffer[0], sizeof(buffer));
                readSize = socket->GetLastIOReadSize();
                if (readSize != 0) {
                    break;
                }
            }
        }
        if (readSize != 0
            && buffer[0] == 'F' && buffer[1] == 'P' && buffer[2] == 'P' && buffer[3] == 'D' && buffer[4] == 0x04) {
            char ip[64];
            sprintf(ip, "%d.%d.%d.%d", (int)buffer[15], (int)buffer[16], (int)buffer[17], (int)buffer[18]);
            if (strcmp(ip, "0.0.0.0")) {
                AddTraceMessage("Received UDP result " + std::string(ip));

                //we found a system!!!
                std::string hostname = (char *)&buffer[19];
                std::string ipStr = ip;
                FPP *found = nullptr;
                for (auto a : instances) {
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
                    inst->ipAddress = ip;
                    inst->fullVersion = (char *)&buffer[84];
                    inst->minorVersion = buffer[13] + (buffer[12] << 8);
                    inst->majorVersion = buffer[11] + (buffer[10] << 8);
                    inst->ranges = (char*)&buffer[166];
                    instances.push_back(inst);

                    std::string fullAddress = "http://" + inst->ipAddress + "/fppjson.php?command=getFPPSystems";
                    CurlData *data = new CurlData(fullAddress);
                    data->type = 0;
                    data->fpp = inst;
                    curls.push_back(data);
                    curl_multi_add_handle(curlMulti, data->curl);
                    running++;

                    fullAddress = "http://" + inst->ipAddress + "/fppjson.php?command=getSysInfo&simple";
                    data = new CurlData(fullAddress);
                    data->type = 1;
                    data->fpp = inst;
                    curls.push_back(data);
                    curl_multi_add_handle(curlMulti, data->curl);
                    running++;
                    
                    fullAddress = "http://" + inst->ipAddress + "/api/proxies";
                    data = new CurlData(fullAddress);
                    data->type = 4;
                    data->fpp = inst;
                    curls.push_back(data);
                    curl_multi_add_handle(curlMulti, data->curl);
                    running++;
                }
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

                                switch (curls[x]->type) {
                                    case 1: {
                                        parsed = reader.Parse(curls[x]->buffer, &origJson) == 0;
                                        if (parsed) {
                                            curls[x]->fpp->parseSysInfo(origJson);
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
                                            data->fpp = curls[x]->fpp;
                                            curls.push_back(data);
                                            curl_multi_add_handle(curlMulti, data->curl);
                                            running++;

                                            fullAddress = baseUrl + "/fppjson.php?command=getChannelOutputs&file=channelOutputsJSON";
                                            data = new CurlData(fullAddress);
                                            data->type = 2;
                                            data->fpp = curls[x]->fpp;
                                            curls.push_back(data);
                                            curl_multi_add_handle(curlMulti, data->curl);
                                            running++;

                                            fullAddress = baseUrl + "/config.php";
                                            data = new CurlData(fullAddress);
                                            data->type = 3;
                                            data->fpp = curls[x]->fpp;
                                            curls.push_back(data);
                                            curl_multi_add_handle(curlMulti, data->curl);
                                            running++;
                                            
                                            if (curls[x]->fpp->proxy == "") {
                                                fullAddress = baseUrl + "/api/proxies";
                                                data = new CurlData(fullAddress);
                                                data->type = 4;
                                                data->fpp = curls[x]->fpp;
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
                                        for (auto address : curls[x]->fpp->proxies) {
                                            bool found = false;
                                            for (auto f : instances) {
                                                if (f->ipAddress == address
                                                    && f->proxy == curls[x]->fpp->ipAddress) {
                                                    found = true;
                                                }
                                            }
                                            if (found) {
                                                continue;
                                            }
                                            
                                            FPP *fpp = new FPP();
                                            fpp->ipAddress = address;
                                            fpp->hostName = address;
                                            fpp->proxy = curls[x]->fpp->ipAddress;
                                            curls[x]->fpp->isFPP = false;
                                            instances.push_back(fpp);

                                            
                                            std::string fullAddress = "http://" + curls[x]->fpp->ipAddress + "/proxy/" + address + "/";
                                            CurlData *data = new CurlData(fullAddress);
                                            data->type = 5;
                                            data->fpp = fpp;
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
                                                    curls[x]->fpp->pixelControllerType = sand.GetPixelControllerTypeString();
                                                    curls[x]->fpp->model = sand.GetModelName();
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
                                                data->fpp = curls[x]->fpp;
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                running++;
                                            } else {
                                                //there is a web server on port 80 running, lets see if we can determine what this is
                                                FPP *fpp = curls[x]->fpp;
                                                std::string fullAddress = "http://" + curls[x]->fpp->proxy + "/proxy/" + curls[x]->fpp->ipAddress + "/status.xml";
                                                CurlData *data = new CurlData(fullAddress);
                                                data->type = 6;
                                                data->fpp = fpp;
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                running++;
                                                
                                                fullAddress = "http://" + curls[x]->fpp->proxy + "/proxy/" + curls[x]->fpp->ipAddress + "/H?";
                                                data = new CurlData(fullAddress);
                                                data->type = 7;
                                                data->fpp = fpp;
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                running++;
                                                
                                                fullAddress = "http://" + curls[x]->fpp->proxy + "/proxy/" + curls[x]->fpp->ipAddress + "/sysinfo.htm";
                                                data = new CurlData(fullAddress);
                                                data->type = 8;
                                                data->fpp = fpp;
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
                                                curls[x]->fpp->fullVersion = falc.GetFirmwareVersion();
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
                                                curls[x]->fpp->pixelControllerType = sand.GetPixelControllerTypeString();
                                                curls[x]->fpp->model = sand.GetModelName();
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
                                                curls[x]->fpp->pixelControllerType = j1sys.GetPixelControllerTypeString();
                                                curls[x]->fpp->model = j1sys.GetModel();
                                                curls[x]->fpp->fullVersion = j1sys.GetVersion();
                                                curls[x]->fpp->platform = "J1Sys";
                                                curls[x]->fpp->mode = "bridge";
                                                curls[x]->fpp->isFPP = false;
                                            }
                                        }
                                        break;
                                    default:
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
                                            for (auto &b : instances) {
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
                                            inst.ipAddress = address;
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
                                                    if (found->ranges == "") {
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
                                                data->fpp = fpp;
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                running++;

                                                fullAddress = "http://" + fpp->ipAddress + "/fppjson.php?command=getSysInfo&simple";
                                                if (fpp->ipAddress == "") {
                                                    fullAddress = "http://" + fpp->hostName + "/fppjson.php?command=getSysInfo&simple";
                                                }
                                                data = new CurlData(fullAddress);
                                                data->type = 1;
                                                data->fpp = fpp;
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                running++;

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
    for (auto data : curls) {
        if (data) {
            AddTraceMessage("Still had a curl outstanding");
            curl_multi_remove_handle(curlMulti, data->curl);
            delete data;
        }
    }
    AddTraceMessage("Closing network resources");
    curl_multi_cleanup(curlMulti);
    
    for (auto socket : sockets) {
        socket->Close();
        delete socket;
    }
    AddTraceMessage("Checking instances and applying fixups");

    std::list<FPP*> toRemove;
    for (auto a : instances) {
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
    for (auto a : toRemove) {
        instances.remove(a);
        delete a;
    }
    AddTraceMessage("Done discovery");

    /*
    for (auto a : instances) {
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
