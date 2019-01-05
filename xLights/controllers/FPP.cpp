#include <map>
#include <string.h>

#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/regex.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/sckstrm.h>
#include <wx/zstream.h>
#include <wx/mstream.h>

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

#include <log4cpp/Category.hh>

FPP::FPP(OutputManager* outputManager, const std::string& defaultVersion, const std::string& ip, const std::string& user, const std::string& password)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _outputManager = outputManager;
 	_user = user;
	_password = password;
	_ip = ip;
    _version = "";
    _forceFTP = false;
    std::string dv = "2.0";
    if (defaultVersion == "1.x") dv = "1.10";

	_connected = _http.Connect(_ip);

    if (_connected) {
        int oldTimeout = _http.GetTimeout();
        _http.SetTimeout(5);

        wxJSONValue val;
        if (GetURLAsJSON("/fppjson.php?command=getSysInfo", val)) {
            _version = val["Version"].AsString().ToStdString();
        }
        if (_version == "" || _version == "null")
        {
            std::string version = GetURL("//");
            if (version == "") {
                logger_base.error("FPP: Unable to retrieve FPP web page. Possible they have a password on the UI.");
                //we'll assume a version 2.x so most things work, but need to use FTP at this point since
                //the upload page will be password protected.  That said, a bunch of things are not supported with FTP
                //FIXME - add basic auth support
                _version = dv;
                _forceFTP = true;
            } else {
                //Version: <a href = 'about.php' class = 'nonULLink'>v1.6 - 25 - gd87f066
                static wxRegEx versionregex("(Version: .*?nonULLink..v)([0-9]+\\.[0-9x]+)", wxRE_ADVANCED | wxRE_NEWLINE);
                if (versionregex.Matches(wxString(version))) {
                    _version = versionregex.GetMatch(wxString(version), 2).ToStdString();
                }
                if (_version == "") {
                    logger_base.debug("FPP: Unable to determine version from HTML page: %s", version.c_str());

                    //I tested 1.8/1.9/1.10 and the regex above detects the 1.x version fine
                    //if we're getting here, figure out why the regex is not working don't change this default
                    _version = dv;
                    _forceFTP = true;
                }
            }
        }
        logger_base.debug("FPP: using version %s.", _version.c_str());
        if (!(_version[0] >= '2')) {
            //either old version or could not determine version
            DisplayWarning("Unsupported FPP version: FPP 1.x is no longer supported by the FPP developers.  Some things may not work.  We strongly recommend upgrading to FPP 2.x");
        }
        _http.SetTimeout(oldTimeout);
        if (!_ftp.Connect(ip, user, password)) {
            _connected = false;
        }
    }
}

FPP::FPP(OutputManager* outputManager)
{
    _forceFTP = false;
    _outputManager = outputManager;
    _connected = false;
}

FPP::~FPP()
{
    _http.Close();
}

bool FPP::GetURLAsJSON(const std::string& url, wxJSONValue& val) {
    _http.SetMethod("GET");
    wxInputStream *httpStream = _http.GetInputStream(wxString(url));
    int rc = _http.GetResponse();
    if (rc == 200) {
        wxJSONReader reader;
        reader.Parse(*httpStream, &val);
        delete httpStream;
        return true;
    } else {
        httpStream = _http.GetInputStream(wxString(url));
        int rc = _http.GetResponse();
        if (rc == 200) {
            wxJSONReader reader;
            reader.Parse(*httpStream, &val);
            delete httpStream;
            return true;
        }
    }
    return false;
}

std::string FPP::GetURL(const std::string& url, bool logresult)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString res = "";
    _http.SetMethod("GET");
    wxInputStream *httpStream = _http.GetInputStream(wxString(url));
    logger_base.debug("Making request to fpp '%s'.", (const char *)url.c_str());

    if (_http.GetError() == wxPROTO_NOERR) {
        wxStringOutputStream out_stream(&res);
        httpStream->Read(out_stream);

        if (logresult) {
            logger_base.debug("Response from fpp '%s' : %d.", (const char *)res.c_str(), _http.GetError());
        }
    } else {
        DisplayError(wxString::Format("Unable to connect to fpp '%s'.", url).ToStdString());
    }

    wxDELETE(httpStream);
    return res.ToStdString();
}

bool FPP::Exists(const std::string& ip)
{
    if (ip == "") return false;

    wxHTTP http;
    bool connected = http.Connect(ip);

    if (connected) {
        http.SetMethod("GET");
        http.SetTimeout(2);
        wxInputStream *httpStream = http.GetInputStream("//");

        if (http.GetError() == wxPROTO_NOERR) {
            // ok
        } else {
            connected = false;
        }

        if (httpStream != nullptr) {
            delete httpStream;
        }
        http.Close();
    }

    return connected;
}

void FPP::RestartFFPD()
{
    GetURL("//fppxml.php?command=restartFPPD");
}

void FPP::E131Output(bool enable)
{
    if (enable) {
        GetURL("//fppjson.php?command=setSetting&plugin=&key=E131Enabled&value=1");
    } else {
        GetURL("//fppjson.php?command=setSetting&plugin=&key=E131Enabled&value=0");
    }
}

bool FPP::SetInputUniversesBridge(std::list<int>& selected, wxWindow* parent)
{
    if (_ftp.IsConnected()) {
        // now create a universes file
        std::string file = SaveFPPUniverses(_ip, selected, false, true);

        bool cancelled = false;
        if (_version[0] == '1') {
            cancelled = _ftp.UploadFile(file, "/home/fpp/media", "universes", true, false, parent);
            // deactive outputs to these inputs
            E131Output(false);
        } else {
            cancelled = _ftp.UploadFile(file, "/home/fpp/media/config", "ci-universes.json", true, false, parent);
        }

        // restart ffpd
        RestartFFPD();

        ::wxRemoveFile(wxString(file));

        return cancelled;
    }

    return true;
}

bool FPP::SetOutputUniversesPlayer(wxWindow* parent)
{
    if (_ftp.IsConnected()) {
        // now create a universes file
        std::string file = SaveFPPUniverses("", std::list<int>(), false, false);

        bool cancelled = false;
        if (_version[0] == '1') {
            cancelled = _ftp.UploadFile(file, "/home/fpp/media", "universes", true, false, parent);
        } else {
            cancelled = _ftp.UploadFile(file, "/home/fpp/media/config", "co-universes.json", true, false, parent);
        }
        // active outputs
        E131Output(true);

        // restart ffpd
        RestartFFPD();

        ::wxRemoveFile(wxString(file));

        return cancelled;
    }

    return true;
}

bool FPP::SetChannelMemoryMaps(ModelManager* allmodels, wxWindow* parent)
{
    if (_ftp.IsConnected()) {
        // now create a universes file
        std::string file = SaveFPPChannelMemoryMaps(allmodels);

        bool cancelled = _ftp.UploadFile(file, "/home/fpp/media", "channelmemorymaps", true, false, parent);

        // restart ffpd
        RestartFFPD();

        ::wxRemoveFile(wxString(file));

        return cancelled;
    }

    return true;
}

std::string FPP::SaveFPPChannelMemoryMaps(ModelManager* allmodels) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxFileName fn;
    fn.AssignTempFileName("channelmemorymaps");
    std::string file = fn.GetFullPath().ToStdString();

    logger_base.debug("FPP models memory map file written to %s.", (const char *)file.c_str());

    wxFile channelmemorymaps;
    channelmemorymaps.Open(file, wxFile::write);

    if (channelmemorymaps.IsOpened()) {
        for (auto m = allmodels->begin(); m != allmodels->end(); ++m) {
            Model* model = m->second;
            wxString stch = model->GetModelXml()->GetAttribute("StartChannel", wxString::Format("%d?", model->NodeStartChannel(0) + 1)); //NOTE: value coming from model is probably not what is wanted, so show the base ch# instead
            int ch = model->GetNumberFromChannelString(model->ModelStartChannel);
            wxString name(model->name);
            name.Replace(" ", "_");
            if (model->GetNumStrands() > 0) {
                channelmemorymaps.Write(wxString::Format("%s,%i,%lu,horizontal,TL,%i,%i\n",
                    name.c_str(),
                    ch,
                    (unsigned long)model->GetActChanCount(),
                    (int)model->GetNumStrands(),
                    1));
            }
        }
        channelmemorymaps.Close();
    }

    return file;
}
std::string FPP::SaveFPPUniverses(const std::string& onlyip, const std::list<int>& selected, bool onebased, bool input) const
{
    if (_version[0] == '1') {
        return SaveFPPUniversesV1(onlyip, selected, onebased);
    }
    return SaveFPPUniversesV2(onlyip, selected, onebased, input);
}

std::string FPP::SaveFPPUniversesV2(const std::string& onlyip, const std::list<int>& selected, bool onebased, bool input) const
{
    wxJSONValue root;
    root["type"] = wxString("universes");
    root["enabled"] = 1;
    root["startChannel"] = 1;
    root["channelCount"] = -1;

    wxJSONValue universes;

    // Get universes based on IP
    std::list<Output*> outputs = _outputManager->GetAllOutputs(onlyip, selected);
    long onebasedcount = 1;
    for (auto it = outputs.begin(); it != outputs.end(); ++it) {
        int c = (*it)->GetStartChannel();
        if (onebased) {
            c = onebasedcount;
        }

        wxJSONValue universe;
        universe["active"] = (*it)->IsEnabled() ? 1 : 0;
        universe["description"] = wxString((*it)->GetDescription());
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
        onebasedcount += (*it)->GetChannels();
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

    wxFileName fn;
    fn.AssignTempFileName("universes");
    std::string file = fn.GetFullPath().ToStdString();

    wxFileOutputStream ufile(fn.GetFullPath());
    wxJSONWriter writer(wxJSONWRITER_STYLED, 0, 3);
    writer.Write(json, ufile);
    ufile.Close();

    return file;
}
std::string FPP::SaveFPPUniversesV1(const std::string& onlyip, const std::list<int>& selected, bool onebased) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxFileName fn;
    fn.AssignTempFileName("universes");
    std::string file = fn.GetFullPath().ToStdString();

    logger_base.debug("FPP universes file written to %s.", (const char *)file.c_str());

    wxFile universes;
    universes.Open(file, wxFile::write);

    if (universes.IsOpened()) {
        // Get universes based on IP
        std::list<Output*> outputs = _outputManager->GetAllOutputs(onlyip, selected);

        long onebasedcount = 1;

        for (auto it = outputs.begin(); it != outputs.end(); ++it) {
            int c = (*it)->GetStartChannel();
            if (onebased) {
                c = onebasedcount;
            }

            if ((*it)->GetType() == OUTPUT_E131) {
                if ((*it)->GetIP() == "MULTICAST") {
                    universes.Write(((*it)->IsEnabled() ? "1," : "0,") + (*it)->GetUniverseString() + "," + wxString::Format(wxT("%i"), c).ToStdString() + "," + wxString::Format(wxT("%ld"), (*it)->GetChannels()).ToStdString() + ",0,,\r\n");
                } else {
                    universes.Write(((*it)->IsEnabled() ? "1," : "0,") + (*it)->GetUniverseString() + "," + wxString::Format(wxT("%i"), c).ToStdString() + "," + wxString::Format(wxT("%ld"), (*it)->GetChannels()).ToStdString() + ",1," + (*it)->GetIP() + ",\r\n");
                }
                onebasedcount += (*it)->GetChannels();
            } else if ((*it)->GetType() == OUTPUT_ARTNET) {
                universes.Write(((*it)->IsEnabled() ? "1," : "0,") + wxString::Format("%d", (*it)->GetUniverse()) + "," + wxString::Format(wxT("%i"), c).ToStdString() + "," + wxString::Format(wxT("%ld"), (*it)->GetChannels()).ToStdString() + ",3," + (*it)->GetIP() + ",\r\n");
                onebasedcount += (*it)->GetChannels();
            }
        }

        universes.Close();
    }

    return file;
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

static inline bool supportsGZIP(const std::string & version) {
    //printf("%s\n", version.c_str());
    if (version[0] <= '1') return false; //FPP 1.x does not
    if (version[0] > '2') return true;   //assume FPP 3.x will support gzip
    if (version[2] == 'x') return true;  //FPP 2.x-master, it does
    if (std::atoi(&version[2]) <= 3) return false; // FPP 2.0 - 2.3 does not
    return true;
}

static inline void addString(wxMemoryBuffer &buffer, const std::string &str) {
    buffer.AppendData(str.c_str(), str.length());
}

bool FPP::uploadFileViaHTTP(const std::string &filename, const std::string &file, wxWindow* parent, bool compress)  {
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
    std::string hostname = _ip;
    address.Hostname(hostname);
    address.Service(80);

    size_t fileLen = 0;
    wxMemoryOutputStream mout;

    bool cancelled = false;
    wxProgressDialog progress("FPP Upload", filename, 1000, parent, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    logger_base.debug("FPP upload via http of %s.", (const char*)filename.c_str());
    progress.Update(0, wxEmptyString, &cancelled);
    int lastDone = 0;

    std::string ct = "Content-Type: application/octet-stream\r\n\r\n";

    if (compress && supportsGZIP(_version)) {
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
        addString(memBuffHeader, "Host: " + hostname + "\r\n");
        addString(memBuffHeader, "Connection: close\r\n");
        addString(memBuffHeader, "Accept: application/json, text/javascript, */*; q=0.01\r\n");
        addString(memBuffHeader, "Origin: http://" + hostname + "\r\n");
        addString(memBuffHeader, "X-Requested-With: XMLHttpRequest\r\n");
        addString(memBuffHeader, "Content-Type: multipart/form-data; boundary=" + bound + "\r\n");
        addString(memBuffHeader, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36\r\n");
        addString(memBuffHeader, "DNT: 1\r\n");
        addString(memBuffHeader, "Referer: http://" + hostname + "/uploadfile.php\r\n");
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
                cancelled = !progress.Update(donePct, "Transferring " + filename, &cancelled);
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
            wxHTTP http;
            http.Connect(hostname);
            wxInputStream *inp = http.GetInputStream("/fppxml.php?command=moveFile&file=" + URLEncode(filename + ext));
            if (inp) {
                delete inp;
            }
            logger_base.debug("Renaming done.");
        }
        progress.Update(100, wxEmptyString, &cancelled);
        delete [] rbuf;

        socket.Close();
        return cancelled;
    }

    return true;
}

bool FPP::UploadSequence(const std::string& file, const std::string& fseqDir, wxWindow* parent, bool suppressZip, bool uploadMedia)
{
    bool cancelled = false;
    wxString media = "";
    wxString type = "";
    wxXmlDocument doc(file);
    if (doc.IsOk()) {
        wxXmlNode* root = doc.GetRoot();
        if (root->GetName() == "xsequence") {
            for (auto n = root->GetChildren(); n != nullptr; n = n->GetNext()) {
                if (n->GetName() == "head") {
                    for (auto n1 = n->GetChildren(); n1 != nullptr; n1 = n1->GetNext()) {
                        if (n1->GetName() == "mediaFile") {
                            media = n1->GetNodeContent();
                        } else if (n1->GetName() == "sequenceType") {
                            type = n1->GetNodeContent();
                        }
                    }
                    break;
                }
            }
        }
    }
    if (type == "Animation") {
        media = "";
    }
    //issue a ftp command so it doesn't timeout while we are uploading files
    _ftp.Pwd();
    wxFileName fn(file);
    wxString fseq = fseqDir + wxFileName::GetPathSeparator() + fn.GetName() + ".fseq";
    if (wxFile::Exists(fseq)) {
        if (_forceFTP) {
            cancelled = _ftp.UploadFile(fseq.ToStdString(), "/home/fpp/media/sequences", fn.GetName().ToStdString() + ".fseq", false, true, parent);
        } else {
            cancelled = uploadFileViaHTTP(fn.GetName().ToStdString() + ".fseq", fseq.ToStdString(), parent, true && !suppressZip);
        }
        sequences[fn.GetName().ToStdString() + ".fseq"] = "";
    } else {
        DisplayError("Unable to upload fseq file " + fseq + " as it does not exist.", parent);
    }

    if (!cancelled && media != "" && uploadMedia) {
        media = FixFile("", media);
        wxFileName fnmedia(media);

        if (fnmedia.Exists()) {
            if (_forceFTP) {
                cancelled = _ftp.UploadFile(media.ToStdString(), "/home/fpp/media/music", fnmedia.GetName().ToStdString() + "." + fnmedia.GetExt().ToStdString(), false, true, parent);
            } else {
                cancelled = uploadFileViaHTTP(fnmedia.GetName().ToStdString() + "." + fnmedia.GetExt().ToStdString(), media.ToStdString(), parent);
            }
            sequences[fn.GetName().ToStdString() + ".fseq"] = fnmedia.GetName().ToStdString() + "." + fnmedia.GetExt().ToStdString();
        } else {
            DisplayError("Unable to upload media file "+ fnmedia.GetName() + "." + fnmedia.GetExt() +" as it does not exist.", parent);
        }
    }

    return cancelled;
}

bool FPP::SetPlaylist(const std::string &name, wxWindow *parent) {
    wxJSONValue origJson;
    GetURLAsJSON("/fppjson.php?command=getPlayListEntries&pl=" + URLEncode(name) + "&reload=true", origJson);

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
    wxFileName fn;
    fn.AssignTempFileName(name);
    std::string file = fn.GetFullPath().ToStdString();
    wxFileOutputStream ufile(fn.GetFullPath());
    wxJSONWriter writer(wxJSONWRITER_STYLED, 0, 3);
    writer.Write(origJson, ufile);
    ufile.Close();

    bool cancelled = _ftp.UploadFile(file, "/home/fpp/media/playlists", name + ".json", true, false, parent);
    ::wxRemoveFile(wxString(file));
    return cancelled;
}


bool FPP::IsDefaultPassword(const std::string& user, const std::string& password)
{
    if ((user == "pi" && password == "raspberry") ||
        (user == "fpp" && password == "falcon")) {
        return true;
    }

    return false;
}

bool FPP::SetOutputs(const std::string &controller, ModelManager* allmodels,
                     std::list<int>& selected, wxWindow* parent,
                     int maxstring, int maxdmx)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("FPP Outputs Upload: Uploading to %s", (const char *)_ip.c_str());

    std::string fppFileName = "co-bbbStrings.json";
    int minPorts = 1;
    if (controller == "PiHat") {
        fppFileName = "co-pixelStrings.json";
        minPorts = 2;
    }

    wxFileName fnOrig;
    fnOrig.AssignTempFileName("pixelOutputs");
    std::string file = fnOrig.GetFullPath().ToStdString();
    wxJSONValue origJson;
    if (!_ftp.GetFile(file, "/home/fpp/media/config", fppFileName, false, parent)) {
        wxFileInputStream ufile(fnOrig.GetFullPath());
        wxJSONReader reader;
        reader.Parse(ufile, &origJson);
    }
    ::wxRemoveFile(wxString(file));

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


    // build a list of models on this controller
    std::map<int, Model*> models;
    std::list<Model*> warnedmodels;
    int maxport = 0;

    std::vector<int> DMXMin = {INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX};
    std::vector<int> DMXMax = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
    int maxDMXPort = -1;


    wxJSONValue stringData;
    stringData["enabled"] = 1;
    stringData["startChannel"] = 1;
    stringData["channelCount"] = -1;

    wxJSONValue dmxData;
    dmxData["enabled"] = 1;
    dmxData["startChannel"] = 1;
    dmxData["type"] = wxString("BBBSerial");
    dmxData["subType"] = wxString("DMX");
    bool isDMX = true;

    // Get universes based on IP
    std::list<Output*> outputs = _outputManager->GetAllOutputs(_ip, selected);
    for (auto ito = outputs.begin(); ito != outputs.end(); ++ito) {
        // find all the models in this range
        for (auto it = allmodels->begin(); it != allmodels->end(); ++it) {
            if (it->second->GetDisplayAs() != "ModelGroup") {
                int modelstart = it->second->GetNumberFromChannelString(it->second->ModelStartChannel);
                int modelend = modelstart + it->second->GetChanCount() - 1;
                if ((modelstart >= (*ito)->GetStartChannel() && modelstart <= (*ito)->GetEndChannel()) ||
                    (modelend >= (*ito)->GetStartChannel() && modelend <= (*ito)->GetEndChannel()))
                {
                    if (!it->second->IsControllerConnectionValid()) {
                        // only warn if we have not already warned
                        if (std::find(warnedmodels.begin(), warnedmodels.end(), it->second) == warnedmodels.end()) {
                            warnedmodels.push_back(it->second);
                            logger_base.warn("FPP Outputs Upload: Model %s on controller %s does not have its Controller Connection details completed. Model ignored.", (const char *)it->first.c_str(), (const char *)_ip.c_str());
                        }
                    } else {
                        // model uses channels in this universe
                        logger_base.debug("FPP Outputs Upload: Uploading Model %s. %s ports %d", (const char *)it->first.c_str(), (const char *)it->second->GetControllerConnectionString().c_str(), it->second->GetNumPhysicalStrings());
                        if (it->second->GetControllerProtocol() == "dmx" || it->second->GetControllerProtocol() == "DMX"
                            || it->second->GetControllerProtocol() == "pixelnet" || it->second->GetControllerProtocol() == "PIXELNET") {
                            if (it->second->GetControllerProtocol() == "pixelnet" || it->second->GetControllerProtocol() == "PIXELNET") {
                                dmxData["subType"] = wxString("Pixelnet");
                                isDMX = false;
                            }

                            int firstC = it->second->GetFirstChannel() + 1; //DMX channels are 1 based
                            int lastC = it->second->GetLastChannel();
                            wxXmlNode *v = it->second->GetControllerConnection();
                            if (v->HasAttribute("channel")) {
                                //if the DMX device has a channel # assigned, we need to subtract out so the
                                //DMX channel is sent out correctly
                                int i = wxAtoi(v->GetAttribute("channel", "1"));
                                i = i - 1;
                                firstC -= i;
                            }
                            
                            if (firstC < DMXMin[it->second->GetControllerPort()]) {
                                DMXMin[it->second->GetControllerPort()] = firstC;
                            }
                            if (lastC > DMXMax[it->second->GetControllerPort()]) {
                                DMXMax[it->second->GetControllerPort()] = lastC;
                            }
                            maxDMXPort = std::max(maxDMXPort, it->second->GetControllerPort());
                        } else {
                            int st = it->second->GetFirstChannel();
                            while (models[st] != nullptr) {
                                st++;
                            }
                            models[st] = it->second;
                            int mp = it->second->GetControllerPort() + it->second->GetNumPhysicalStrings() - 1;
                            if (mp > maxport) {
                                maxport = mp;
                            }
                        }
                    }
                }
            }
        }
    }
    if (maxport < minPorts) {
        maxport = minPorts;
    }

    if (controller == "PiHat") {
        stringData["type"] = wxString("RPIWS281X");
        stringData["subType"] = wxString("");
    } else {
        stringData["type"] = wxString("BBB48String");
        stringData["subType"] = wxString(controller);
        stringData["pinoutVersion"] = pinout;
    }
    stringData["outputCount"] = maxport;
    dmxData["device"] = wxString(controller);


    for (int x = 0; x < maxport; x++) {
        wxJSONValue port;
        port["portNumber"] = x;

        stringData["outputs"].Append(port);
    }
    for (auto mm = models.begin(); mm != models.end(); ++mm) {
        Model *model = mm->second;
        for (int x = 0; x < model->GetNumPhysicalStrings(); x++) {
            int port = x + model->GetControllerPort() - 1;
            
            wxJSONValue vs;
            if (model->GetNumPhysicalStrings() == 1) {
                vs["description"] = wxString(model->GetName());
            } else {
                vs["description"] = (model->GetName()) + wxString::Format(" String-%d", (x + 1));
            }
            vs["startChannel"] = model->GetStringStartChan(x);
            vs["pixelCount"] = model->NodesPerString();

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
                if (model->GetNodeChannelCount((model->GetStringType())) == 4) {
                    vs["colorOrder"] = wxString("RGBW");
                } else {
                    vs["colorOrder"] = wxString("RGB");
                }
                vs["nullNodes"] = 0;
                vs["zigZag"] = 0; // If we zigzag in xLights, we don't do it in the controller, if we need it in the controller, we don't know about it here
                vs["brightness"] = 100;
                vs["gamma"] = wxString("1.0");
            }
            wxXmlNode *node = model->GetControllerConnection();
            if (node->HasAttribute("reverse")) {
                vs["reverse"] = wxAtoi(node->GetAttribute("reverse"));
            }
            if (node->HasAttribute("gamma")) {
                vs["gamma"] = wxAtof(node->GetAttribute("gamma"));
            }
            if (node->HasAttribute("brightness")) {
                vs["brightness"] = wxAtoi(node->GetAttribute("brightness"));
            }
            if (node->HasAttribute("nullNodes")) {
                vs["nullNodes"] = wxAtoi(node->GetAttribute("nullNodes"));
            }
            if (node->HasAttribute("colorOrder")) {
                vs["colorOrder"] = node->GetAttribute("colorOrder");
            }
            if (node->HasAttribute("groupCount")) {
                vs["groupCount"] = wxAtoi(node->GetAttribute("groupCount"));
            }

            if (vs["groupCount"].AsInt() > 1) {
                //if the group count is >1, we need to adjust the number of pixels
                vs["pixelCount"] = model->NodesPerString() * vs["groupCount"].AsInt();
            }

            stringData["outputs"][port]["virtualStrings"].Append(vs);
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

    int maxChan = 0;
    int maxDmxPort = -1;
    int chanPerOut = isDMX ? 512 : 4096;
    for (int x = 1; x <= 8; x++) {
        wxJSONValue port;
        port["outputNumber"] = (x - 1);
        port["outputType"] = isDMX ? wxString("DMX") : wxString("Pixelnet");
        if (DMXMin[x] == INT_MAX) {
            port["startChannel"] = 0;
        } else {
            port["startChannel"] = DMXMin[x];
            if ((DMXMin[x] + (chanPerOut+1) > maxChan)) {
                maxChan = DMXMin[x] + chanPerOut + 1;
            }
            maxDmxPort = std::max(maxDmxPort, x);
        }
        port["channelCount"] = chanPerOut;
        dmxData["outputs"].Append(port);
    }
    dmxData["channelCount"] = maxChan < chanPerOut ? chanPerOut : maxChan;
    if (maxChan == 0) {
        dmxData["enabled"] = 0;
        dmxData["subType"] = wxString("off");
    }


    wxJSONValue root;
    root["channelOutputs"].Append(stringData);
    if (controller != "PiHat") {
        root["channelOutputs"].Append(dmxData);
    } else {
        wxString dev = controller;
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

    bool cancelled = _ftp.UploadFile(file, "/home/fpp/media/config", fppFileName, true, false, parent);
    ::wxRemoveFile(wxString(file));

    // restart ffpd
    RestartFFPD();

    return !cancelled;
}

#define FPP_CTRL_PORT 32320
void FPP::Discover(std::list<FPPInstance> &instances) {
    wxDatagramSocket *socket;
    wxIPV4address localaddr;
    localaddr.AnyAddress();
    localaddr.Service(FPP_CTRL_PORT);

    socket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST);
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

    wxIPV4address bcAddress;
    bcAddress.BroadcastAddress();
    bcAddress.Service(FPP_CTRL_PORT);
    socket->SendTo(bcAddress, buffer, 207);

    uint64_t time = wxGetLocalTimeMillis().GetValue() + 1000l;
    while (wxGetLocalTimeMillis().GetValue() < time) {
        memset(buffer, 0x00, sizeof(buffer));
        socket->Read(&buffer[0], sizeof(buffer));
        if (socket->GetLastIOReadSize() == 0) {
            socket->WaitForRead(0, 50);
        } else if (buffer[0] == 'F' && buffer[1] == 'P' && buffer[2] == 'P' && buffer[3] == 'D' && buffer[4] == 0x04) {
            char ip[64];
            sprintf(ip, "%d.%d.%d.%d", (int)buffer[15], (int)buffer[16], (int)buffer[17], (int)buffer[18]);
            if (strcmp(ip, "0.0.0.0")) {
                //we found a system!!!
                FPPInstance inst;
                inst.hostName = (char *)&buffer[19];
                inst.model = (char *)&buffer[125];
                inst.ipAddress = ip;
                inst.fullVersion = (char *)&buffer[84];
                inst.minorVersion = buffer[13] + (buffer[12] << 8);
                inst.majorVersion = buffer[11] + (buffer[10] << 8);
                inst.ranges = (char*)&buffer[166];
                instances.push_back(inst);
            }
        }
    }
    socket->Close();
    delete socket;
    //discovered based on broadcast, now lets get the lists they know about... (may span other networks)
    for (auto a : instances) {
        wxHTTP http;
        http.Connect(a.ipAddress);
        http.SetMethod("GET");
        wxInputStream *inp = http.GetInputStream("/fppjson.php?command=getFPPSystems");
        if (inp) {
            wxJSONValue origJson;
            wxJSONReader reader;
            reader.Parse(*inp, &origJson);
            delete inp;

            for (int x = 0; x < origJson.Size(); x++) {
                wxJSONValue system = origJson[x];
                wxString address = system["IP"].AsString();
                FPPInstance *found = nullptr;
                for (auto &b : instances) {
                    if (b.ipAddress == address) {
                        found = &b;
                    }
                }
                FPPInstance inst;
                inst.hostName = system["HostName"].AsString();
                if (!system["Platform"].IsNull()) {
                    inst.platform = system["Platform"].AsString();
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
                if (found) {
                    if (found->majorVersion == 0) {
                        *found = inst;
                    } else if (found->platform == "") {
                        found->platform = inst.platform;
                    }
                } else {
                    instances.push_back(inst);
                }
            }
        }
    }
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


