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
#include "xLightsXmlFile.h"
#include "models/Model.h"
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "outputs/DDPOutput.h"
#include "UtilFunctions.h"
#include "xLightsVersion.h"
#include "../Parallel.h"

#include <log4cpp/Category.hh>
#include "ControllerUploadData.h"
#include "../FSEQFile.h"



static std::map<std::string, PixelCapeInfo> CONTROLLER_TYPE_MAP = {
    {"PiHat", PixelCapeInfo("PiHat", 2, 0)},
    {"F8-B", PixelCapeInfo("F8-B (8 serial)", 12, 8)},
    {"F8-B-16", PixelCapeInfo("F8-B (4 serial)", 16, 4)},
    {"F8-B-20", PixelCapeInfo("F8-B (No serial)", 20, 0)},
    {"F8-B-EXP", PixelCapeInfo("F8-B w/ Expansion (8 serial)", 28, 8)},
    {"F8-B-EXP-32", PixelCapeInfo("F8-B w/ Expansion (4 serial)", 32, 4)},
    {"F8-B-EXP-36", PixelCapeInfo("F8-B w/ Expansion (No serial)", 36, 0)},
    {"F16-B", PixelCapeInfo("F16-B", 16, 8)},
    {"F16-B-32", PixelCapeInfo("F16-B w/ 32 outputs", 32, 8)},
    {"F16-B-48", PixelCapeInfo("F16-B w/ 48 outputs (No Serial)", 48, 0)},
    {"F4-B", PixelCapeInfo("F4-B", 4, 1)},
    {"F32-B", PixelCapeInfo("F32-B (8 Serial)", 40, 8)},
    {"F32-B-44", PixelCapeInfo("F32-B (4 Serial)", 44, 4)},
    {"F32-B-48", PixelCapeInfo("F32-B (No Serial)", 48, 0)},
    {"F40D-PB", PixelCapeInfo("F40D-PB (8 Serial)", 32, 8)},
    {"F40D-PB-36", PixelCapeInfo("F40D-PB (4 Serial)", 36, 4)},
    {"F40D-PB-40", PixelCapeInfo("F40D-PB (No Serial)", 40, 0)},
    {"RGBCape24", PixelCapeInfo("RGBCape24", 48, 0)},
    {"RGBCape48C", PixelCapeInfo("RGBCape48C", 48, 0)},
    {"RGBCape48F", PixelCapeInfo("RGBCape48F", 48, 0)},
    {"LED Panels", PixelCapeInfo("LED Panels", 0, 0)}
};

const std::string &FPP::PixelContollerDescription() const {
    if (CONTROLLER_TYPE_MAP.find(pixelControllerType) == CONTROLLER_TYPE_MAP.end()) {
        return pixelControllerType;
    }
    return CONTROLLER_TYPE_MAP[pixelControllerType].description;
}

PixelCapeInfo& FPP::GetCapeRules(const std::string& type)
{
    if (type.find("LED Panels", 0) == 0) {
        return CONTROLLER_TYPE_MAP["LED Panels"];
    }
    return CONTROLLER_TYPE_MAP.find(type) != CONTROLLER_TYPE_MAP.end() ? CONTROLLER_TYPE_MAP[type] : CONTROLLER_TYPE_MAP["PiHat"];
}

FPP::FPP(const std::string &ad) : majorVersion(0), minorVersion(0), outputFile(nullptr), parent(nullptr), ipAddress(ad), curl(nullptr) {
    wxIPV4address address;
    if (address.Hostname(ad)) {
        hostName = ad;
        ipAddress = address.IPAddress();
    }
}


FPP::FPP(const FPP &c)
    : majorVersion(c.majorVersion), minorVersion(c.minorVersion), outputFile(nullptr), parent(nullptr), curl(nullptr),
    hostName(c.hostName), description(c.description), ipAddress(c.ipAddress), fullVersion(c.fullVersion), platform(c.platform),
    model(c.model), ranges(c.ranges), mode(c.mode), pixelControllerType(c.pixelControllerType), username(c.username), password(c.password)
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

void FPP::setupCurl() {
    if (curl == nullptr) {
        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, buffer_writer);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlInputBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 100);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000);
    }
}
bool FPP::GetURLAsString(const std::string& url, std::string& val)  {
    setupCurl();
    curlInputBuffer.clear();
    char error[1024];
    std::string fullUrl = "http://" + ipAddress + url;
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error);
    bool retValue = false;
    int i = curl_easy_perform(curl);
    if (i == CURLE_OK) {
        val = curlInputBuffer;
        retValue = true;
    }
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, nullptr);
    return retValue;

}

bool FPP::GetURLAsJSON(const std::string& url, wxJSONValue& val) {
    setupCurl();
    curlInputBuffer.clear();
    char error[1024];
    std::string fullUrl = "http://" + ipAddress + url;
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error);
    bool retValue = false;
    int i = curl_easy_perform(curl);
    if (i == CURLE_OK) {
        wxJSONReader reader;
        reader.Parse(curlInputBuffer, &val);
        retValue = true;
    }
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, nullptr);
    return retValue;
}


bool FPP::AuthenticateAndUpdateVersions() {
    wxJSONValue val;
    if (GetURLAsJSON("/fppjson.php?command=getSysInfo&simple", val)) {
        return parseSysInfo(val);
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
            pixelControllerType = "LED Panels - ";
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
            pixelControllerType.append(std::to_string(pw * nw));
            pixelControllerType.append("x");
            pixelControllerType.append(std::to_string(ph * nh));

        }
    }
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
int FPP::PostToURL(const std::string& url, const wxMemoryBuffer &val, const std::string &contentType) {
    wxHTTP http;
    http.Connect(ipAddress);
    http.SetMethod("POST");
    http.SetPostBuffer(contentType, val);
    std::unique_ptr<wxInputStream> httpStream(http.GetInputStream(wxString(url)));
    return http.GetResponse();
}


bool FPP::uploadFile(const std::string &filename, const std::string &file, bool compress)  {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static int bufLen = 1024*1024*4; //4MB buffer
    
    //we cannot use wxHTTP for a few reasons:
    //1) It doesn't support transfers larger than 2GB, including all headers and boundaries and such
    //2) It has no way to monitor the bytes transferred
    //3) Cannot use wxSOCKET_NOWAIT_WRITE (which is much faster) as it doesn't actually check
    //   if all the data was sent
    //4) Nothing is virtual so cannot even subclass to fix issues
    
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
    
    wxIPV4address address;
    address.Hostname(ipAddress);
    address.Service(80);
    
    size_t fileLen = 0;
    wxMemoryOutputStream mout;
    
    bool cancelled = false;
    wxProgressDialog progress("FPP Upload", "Transferring " + filename + " to " + ipAddress, 1000, parent, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    logger_base.debug("FPP upload via http of %s.", (const char*)filename.c_str());
    progress.Update(0, "Transferring " + filename + " to " + ipAddress, &cancelled);
    int lastDone = 0;
    
    std::string ct = "Content-Type: application/octet-stream\r\n\r\n";
    
    if (compress) {
        logger_base.debug("Uploading it compressed.");
        //determine size of gzipped data
        progress.Show();
        unsigned char *rbuf = new unsigned char[bufLen];
        wxFile f_in(file);
        wxZlibOutputStream zlib(mout, wxZ_DEFAULT_COMPRESSION, wxZLIB_GZIP);
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
        fileLen = mout.GetOutputStreamBuffer()->GetIntPosition();
        //printf("compressed: %zu to %zu    %zu\n", total, fileLen, fileLen*100/total);
        mout.GetOutputStreamBuffer()->SetIntPosition(0);
        fn += ".gz";
        ext = ".gz";
        ct = "Content-Type: application/z-gzip\r\n\r\n";
        
        delete [] rbuf;
    } else {
        logger_base.debug("Uploading it uncompressed.");
        compress = false;
        wxFile f_in(file);
        fileLen = f_in.Length();
    }
    
    wxSocketClient socket(wxSOCKET_NOWAIT_WRITE);
    if (socket.Connect(address)) {
        progress.Show();
        
        const std::string bound = "----WebKitFormBoundaryb29a7c2fe47b9481";
        
        wxMemoryBuffer memBuffPost;
        addString(memBuffPost, "\r\n--");
        addString(memBuffPost, bound);
        addString(memBuffPost,"\r\nContent-Disposition: form-data; name=\"\"\r\n\r\nundefined\r\n--");
        addString(memBuffPost, bound);
        addString(memBuffPost,"\r\nContent-Disposition: form-data; name=\"\"\r\n\r\nundefined\r\n--");
        addString(memBuffPost, bound);
        addString(memBuffPost, "--\r\n");
        
        wxMemoryBuffer memBuffHeader;
        addString(memBuffHeader, "POST /jqupload.php HTTP/1.1\r\n");
        addString(memBuffHeader, "Host: " + ipAddress + "\r\n");
        addString(memBuffHeader, "Connection: close\r\n");
        addString(memBuffHeader, "Accept: application/json, text/javascript, */*; q=0.01\r\n");
        addString(memBuffHeader, "Origin: http://" + ipAddress + "\r\n");
        addString(memBuffHeader, "X-Requested-With: XMLHttpRequest\r\n");
        addString(memBuffHeader, "Content-Type: multipart/form-data; boundary=" + bound + "\r\n");
        addString(memBuffHeader, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36\r\n");
        addString(memBuffHeader, "DNT: 1\r\n");
        addString(memBuffHeader, "Referer: http://" + ipAddress + "/uploadfile.php\r\n");
        addString(memBuffHeader, "Accept-Language: en-US,en;q=0.9\r\n");
        
        
        unsigned char *rbuf = new unsigned char[bufLen];
        
        
        std::string cd = "Content-Disposition: form-data; name=\"myfile\"; filename=\"";
        cd += fn.ToStdString();
        cd += "\"\r\n";
        wxMemoryBuffer memBuffPre;
        addString(memBuffPre, "--");
        addString(memBuffPre, bound);
        addString(memBuffPre, "\r\n");
        addString(memBuffPre, cd);
        addString(memBuffPre, ct);
        
        size_t totalLen = fileLen + memBuffPre.GetDataLen() + memBuffPost.GetDataLen();
        size_t totalWritten = 0;
        addString(memBuffHeader, wxString::Format("Content-Length: %zu\r\n\r\n", totalLen));
        
        size_t len = memBuffHeader.GetDataLen();
        const char *data = (const char *) memBuffHeader.GetData();
        socket.Write(data, len);
        size_t written = socket.LastWriteCount();
        while (written < len) {
            socket.Write(&data[written], len - written);
            written += socket.LastWriteCount();
        }
        
        len = memBuffPre.GetDataLen();
        data = (const char *) memBuffPre.GetData();
        socket.Write(data, len);
        written = socket.LastWriteCount();
        while (written < len) {
            socket.Write(&data[written], len - written);
            written += socket.LastWriteCount();
        }
        totalWritten += written;
        
        int totalReadFromFile = 0;
        wxFile f_in(file);
        while (totalReadFromFile < fileLen) {
            size_t iRead = 0;
            if (compress) {
                iRead = mout.GetOutputStreamBuffer()->Read(rbuf, bufLen);
            } else {
                iRead = f_in.Read(rbuf, bufLen);
            }
            totalReadFromFile += iRead;
            if (iRead) {
                socket.Write(rbuf, iRead);
                written = socket.LastWriteCount();
                while (written < iRead) {
                    socket.Write(&rbuf[written], iRead - written);
                    written += socket.LastWriteCount();
                }
            }
            
            totalWritten += written;
            
            size_t donePct = totalWritten;
            donePct *= compress ? 600 : 1000;
            donePct /= totalLen;
            if (compress) {
                donePct += 333;
            }
            //printf("%ld / %ld    (%ld)\n", totalWritten, totalLen, donePct);
            if (donePct != lastDone) {
                lastDone = donePct;
                cancelled = !progress.Update(donePct, "Transferring " + filename + " to " + ipAddress, &cancelled);
                wxYield();
            }
            if (cancelled) {
                delete [] rbuf;
                f_in.Close();
                socket.Close();
                return cancelled;
            }
        }
        f_in.Close();
        
        len = memBuffPost.GetDataLen();
        data = (const char *) memBuffPost.GetData();
        socket.Write(data, len);
        written = socket.LastWriteCount();
        while (written < len) {
            socket.Write(&data[written], len - written);
            written += socket.LastWriteCount();
        }
        logger_base.debug("Uploaded.");
        if (compress) {
            progress.Update(999, "Decompressing " + filename, &cancelled);
            logger_base.debug("Decompressing.");
        }
        socket.Read(rbuf, bufLen-1);
        int i = socket.LastReadCount();
        rbuf[i] = 0;
        
        if (strstr((char *)rbuf, fn.c_str()) != nullptr) {
            //upload OK, now rename
            std::string val;
            GetURLAsString("/fppxml.php?command=moveFile&file=" + URLEncode(filename + ext), val);
            logger_base.debug("Renaming done.");
        }
        progress.Update(1000, wxEmptyString, &cancelled);
        delete [] rbuf;
        
        socket.Close();
        return cancelled;
    }
    
    return true;
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
    std::list<Output*> outputs = outputManager->GetAllOutputs(onlyip, selected);
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
    
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("FPP Outputs Upload: Uploading to %s", (const char *)ipAddress.c_str());

    std::string check;
    UDController cud(ipAddress, hostName, allmodels, outputManager, &selected, check);
    if (maxString == 0 && maxdmx == 0) {
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
                }
            }
            
            if (IsDrive()) {
                WriteJSONToPath(ipAddress + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + "channeloutputs.json", origJson);
            } else {
                PostJSONToURLAsFormData("/fppjson.php", "command=setChannelOutputs&file=channelOutputsJSON", origJson);
            }
        }
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
    if (IsDrive()) {
        GetPathAsJSON(ipAddress + wxFileName::GetPathSeparator() + "config" + wxFileName::GetPathSeparator() + fppFileName +".json", origJson);
    } else {
        GetURLAsJSON("/fppjson.php?command=getChannelOutputs&file=" + fppFileName, origJson);
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
            port->CreateVirtualStrings(false);
            for (auto pvs : port->GetVirtualStrings()) {
                wxJSONValue vs;
                vs["description"] = pvs->_description;
                vs["startChannel"] = pvs->_startChannel - 1; // we need 0 based
                vs["pixelCount"] = pvs->Channels() / pvs->_channelsPerPixel;
                
                if (origStrings.find(vs["description"].AsString()) != origStrings.end()) {
                    wxJSONValue &vo = origStrings[vs["description"].AsString()];
                    vs["groupCount"] = vo["groupCount"];
                    vs["reverse"] = vo["reverse"];
                    vs["colorOrder"] = vo["colorOrder"];
                    vs["nullNodes"] = vo["nullNodes"];
                    vs["zigZag"] = vo["zigZag"];
                    vs["brightness"] = vo["brightness"];
                    vs["gamma"] =vo["gamma"];
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
    }

    bool isDMX = true;
    int maxChan = 0;
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
        } else {
            port["startChannel"] = 0;
            port["channelCount"] = 0;
        }
        dmxData["outputs"].Append(port);
    }
    
    dmxData["channelCount"] = isDMX ? maxChan : 4096;
    if (maxChan == 0) {
        dmxData["enabled"] = 0;
        dmxData["subType"] = wxString("off");
    }


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
void FPP::Discover(const std::list<std::string> &addresses, std::list<FPP*> &instances, bool doBroadcast) {
    std::vector<CurlData*> curls;
    CURLM * curlMulti = curl_multi_init();
    for (auto &a : addresses) {
        std::string fullAddress = "http://" + a + "/fppjson.php?command=getFPPSystems";
        CurlData *data = new CurlData(fullAddress);
        curls.push_back(data);
        curl_multi_add_handle(curlMulti, data->curl);
    }
    wxDatagramSocket *socket;
    wxIPV4address localaddr;
    localaddr.AnyAddress();
    localaddr.Service(FPP_CTRL_PORT);

    socket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST | wxSOCKET_NOWAIT);
    socket->SetTimeout(1);
    socket->Notify(false);

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

    if (socket->IsOk()) {
        wxIPV4address bcAddress;
        bcAddress.BroadcastAddress();
        bcAddress.Service(FPP_CTRL_PORT);
        socket->SendTo(bcAddress, buffer, 207);
    }
    uint64_t endBroadcastTime = wxGetLocalTimeMillis().GetValue() + 1200l;
    int running = curls.size();
    while (running || (wxGetLocalTimeMillis().GetValue() < endBroadcastTime)) {
        memset(buffer, 0x00, sizeof(buffer));
        int readSize = 0;
        if (socket->IsOk()) {
            socket->Read(&buffer[0], sizeof(buffer));
            readSize = socket->GetLastIOReadSize();
        }
        if (readSize != 0
            && buffer[0] == 'F' && buffer[1] == 'P' && buffer[2] == 'P' && buffer[3] == 'D' && buffer[4] == 0x04) {
            char ip[64];
            sprintf(ip, "%d.%d.%d.%d", (int)buffer[15], (int)buffer[16], (int)buffer[17], (int)buffer[18]);
            if (strcmp(ip, "0.0.0.0")) {
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
                if (!found && (platform > 0 && platform < 0x80)) {
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
                        curl_multi_remove_handle(curlMulti, e);
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
                                reader.Parse(curls[x]->buffer, &origJson);
                                switch (curls[x]->type) {
                                    case 1: {
                                        curls[x]->fpp->parseSysInfo(origJson);
                                        std::string file = "co-pixelStrings";
                                        if (curls[x]->fpp->platform.find("Beagle") != std::string::npos) {
                                            file = "co-bbbStrings";
                                        }
                                        std::string fullAddress = "http://" + curls[x]->fpp->ipAddress + "/fppjson.php?command=getChannelOutputs&file=" + file;
                                        CurlData *data = new CurlData(fullAddress);
                                        data->type = 2;
                                        data->fpp = curls[x]->fpp;
                                        curls.push_back(data);
                                        curl_multi_add_handle(curlMulti, data->curl);
                                        running++;

                                        fullAddress = "http://" + curls[x]->fpp->ipAddress + "/fppjson.php?command=getChannelOutputs&file=channelOutputsJSON";
                                        data = new CurlData(fullAddress);
                                        data->type = 2;
                                        data->fpp = curls[x]->fpp;
                                        curls.push_back(data);
                                        curl_multi_add_handle(curlMulti, data->curl);
                                        running++;
                                        break;
                                    }
                                    case 2:
                                        curls[x]->fpp->parseControllerType(origJson);
                                        break;
                                    default:
                                        for (int x = 0; x < origJson.Size(); x++) {
                                            wxJSONValue system = origJson[x];
                                            wxString address = system["IP"].AsString();
                                            wxString hostName = system["HostName"].AsString();
                                            if (address.length() > 16) {
                                                //ignore for some reason, FPP is occassionally returning an IPV6 address
                                                continue;
                                            }
                                            FPP *found = nullptr;
                                            for (auto &b : instances) {
                                                if (b->ipAddress == address || b->ipAddress == hostName || b->hostName == hostName) {
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
                                            if (inst.platform.find("Unknown") != std::string::npos) {
                                                continue;
                                            }
                                            if (inst.platform.find("unknown") != std::string::npos) {
                                                continue;
                                            }
                                            if (inst.platform.find("Falcon ") != std::string::npos) {
                                                continue;
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
                                                std::string fullAddress = "http://" + fpp->ipAddress + "/fppjson.php?command=getSysInfo&simple";
                                                if (fpp->ipAddress == "") {
                                                    fullAddress = "http://" + fpp->hostName + "/fppjson.php?command=getSysInfo&simple";
                                                }
                                                CurlData *data = new CurlData(fullAddress);
                                                data->type = 1;
                                                data->fpp = fpp;
                                                curls.push_back(data);
                                                curl_multi_add_handle(curlMulti, data->curl);
                                                
                                                instances.push_back(fpp);
                                            }
                                        }
                                }
                                delete curls[x];
                                curls[x] = nullptr;
                            }
                        }
                    }
                }
            }
        }
    }
    for (auto data : curls) {
        if (data) {
            curl_multi_remove_handle(curlMulti, data->curl);
            delete data;
        }
    }
    curl_multi_cleanup(curlMulti);
    socket->Close();
    delete socket;
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
