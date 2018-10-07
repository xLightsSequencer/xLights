#include <map>

#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/regex.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/wfstream.h>

#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"

#include "FPP.h"
#include "xLightsXmlFile.h"
#include "models/Model.h"
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "outputs/DDPOutput.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>

FPP::FPP(OutputManager* outputManager, const std::string& ip, const std::string& user, const std::string& password)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _outputManager = outputManager;
 	_user = user;
	_password = password;
	_ip = ip;
    _version = "";

	_connected = _http.Connect(_ip);

    if (_connected) {
        int oldTimeout = _http.GetTimeout();
        _http.SetTimeout(2);
        
        wxJSONValue val;
        if (GetURLAsJSON("/fppjson.php?command=getSysInfo", val)) {
            _version = val["Version"].AsString().ToStdString();
        } else {
            std::string version = GetURL("//");

            if (version == "") {
                logger_base.error("FPP: Unable to retrieve FPP web page. Possible they have a password on the UI.");
            } else {
                //Version: <a href = 'about.php' class = 'nonULLink'>v1.6 - 25 - gd87f066
                static wxRegEx versionregex("(Version: .*?nonULLink..v)([0-9]+\\.[0-9]+)", wxRE_ADVANCED | wxRE_NEWLINE);
                if (versionregex.Matches(wxString(version))) {
                    _version = versionregex.GetMatch(wxString(version), 2).ToStdString();
                }
            }
        }
        _http.SetTimeout(oldTimeout);
        _ftp.Connect(ip, user, password);
    }
}

FPP::FPP(OutputManager* outputManager)
{
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
        logger_base.error("Unable to connect to fpp '%s'.", (const char *)url.c_str());
        wxMessageBox(_T("Unable to connect!"));
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
        if (_version[0] == '2') {
            cancelled = _ftp.UploadFile(file, "/home/fpp/media/config", "ci-universes.json", true, false, parent);
        } else {
            cancelled = _ftp.UploadFile(file, "/home/fpp/media", "universes", true, false, parent);
            // deactive outputs to these inputs
            E131Output(false);
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
        if (_version[0] == '2') {
            cancelled = _ftp.UploadFile(file, "/home/fpp/media/config", "co-universes.json", true, false, parent);
        } else {
            cancelled = _ftp.UploadFile(file, "/home/fpp/media", "universes", true, false, parent);
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
                channelmemorymaps.Write(wxString::Format("%s,%i,%ul,horizontal,TL,%i,%i\n",
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
    if (_version[0] == '2') {
        return SaveFPPUniversesV2(onlyip, selected, onebased, input);
    }
    return SaveFPPUniversesV1(onlyip, selected, onebased);
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

bool FPP::UploadSequence(const std::string& file, const std::string& fseqDir, wxWindow* parent)
{
    bool cancelled = false;
    wxString media = "";

    wxXmlDocument doc(file);
    if (doc.IsOk()) {
        wxXmlNode* root = doc.GetRoot();
        if (root->GetName() == "xsequence") {
            for (auto n = root->GetChildren(); n != nullptr; n = n->GetNext()) {
                if (n->GetName() == "head") {
                    for (auto n1 = n->GetChildren(); n1 != nullptr; n1 = n1->GetNext()) {
                        if (n1->GetName() == "mediaFile") {
                            media = n1->GetNodeContent();
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    wxFileName fn(file);
    wxString fseq = fseqDir + wxFileName::GetPathSeparator() + fn.GetName() + ".fseq";
    if (wxFile::Exists(fseq)) {
        cancelled = _ftp.UploadFile(fseq.ToStdString(), "/home/fpp/media/sequences", fn.GetName().ToStdString() + ".fseq", false, true, parent);
    } else {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.error("Unable to upload fseq file %s as it does not exist.", (const char *)fseq.c_str());
        wxMessageBox("Unable to upload fseq file " + fseq + " as it does not exist.", "Error", 4, parent);
    }

    if (!cancelled && media != "") {
        media = FixFile("", media);
        wxFileName fnmedia(media);

        if (fnmedia.Exists()) {
            cancelled = _ftp.UploadFile(media.ToStdString(), "/home/fpp/media/music", fnmedia.GetName().ToStdString() + "." + fnmedia.GetExt().ToStdString(), false, true, parent);
        } else {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.error("Unable to upload media file %s as it does not exist.", (const char *)(fnmedia.GetName().ToStdString() + "." + fnmedia.GetExt().ToStdString()).c_str());
            wxMessageBox("Unable to upload media file "+ fnmedia.GetName() + "." + fnmedia.GetExt() +" as it does not exist.", "Error", 4, parent);
        }
    }

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
    if (controller == "PiHat") {
        fppFileName = "co-pixelStrings.json";
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

    // Get universes based on IP
    std::list<Output*> outputs = _outputManager->GetAllOutputs(_ip, selected);
    for (auto ito = outputs.begin(); ito != outputs.end(); ++ito) {
        // find all the models in this range
        for (auto it = allmodels->begin(); it != allmodels->end(); ++it)
        {
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
                            logger_base.warn("FPP Outputs Upload: Model %s on controller %s does not have its Controller Connection details completed: '%s'. Model ignored.", (const char *)it->first.c_str(), (const char *)_ip.c_str(), (const char *)it->second->GetControllerConnection().c_str());
                        }
                    } else {
                        // model uses channels in this universe
                        logger_base.debug("FPP Outputs Upload: Uploading Model %s. %s:%d ports %d", (const char *)it->first.c_str(), (const char *)it->second->GetProtocol().c_str(), it->second->GetPort(), it->second->GetNumPhysicalStrings());
                        if (it->second->GetProtocol() == "dmx") {
                            if (it->second->GetFirstChannel() < DMXMin[it->second->GetPort()]) {
                                DMXMin[it->second->GetPort()] = it->second->GetFirstChannel();
                            }
                            if (it->second->GetLastChannel() > DMXMax[it->second->GetPort()]) {
                                DMXMax[it->second->GetPort()] = it->second->GetLastChannel();
                            }
                            maxDMXPort = std::max(maxDMXPort, it->second->GetPort());
                        } else {
                            int st = it->second->GetFirstChannel();
                            while (models[st] != nullptr) {
                                st++;
                            }
                            models[st] = it->second;
                            int mp = it->second->GetPort() + it->second->GetNumPhysicalStrings() - 1;
                            if (mp > maxport) {
                                maxport = mp;
                            }
                        }
                    }
                }
            }
        }
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
            int port = x + model->GetPort() - 1;
            
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
    for (int x = 1; x <= 8; x++) {
        wxJSONValue port;
        port["outputNumber"] = (x - 1);
        port["outputType"] = wxString("DMX");
        if (DMXMin[x] == INT_MAX) {
            port["startChannel"] = 1;
        } else {
            port["startChannel"] = DMXMin[x];
            if ((DMXMin[x] + 513) > maxChan) {
                maxChan = DMXMin[x] + 513;
            }
            maxDmxPort = std::max(maxDmxPort, x);
        }
        port["channelCount"] = 512;
        dmxData["outputs"].Append(port);
    }
    dmxData["channelCount"] = maxChan < 512 ? 512 : maxChan;
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


