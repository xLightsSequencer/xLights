#include "J1Sys.h"
#include <wx/msgdlg.h>
#include <wx/regex.h>
#include <wx/xml/xml.h>
#include "models/Model.h"
#include <log4cpp/Category.hh>
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "models/ModelManager.h"
#include <wx/sstream.h>
#include "UtilFunctions.h"
#include "ControllerUploadData.h"

// This code has been tested with
// ECG-P12S App Version 3.3
// ECG-P2 App Version 2.9b

// P12R - 1 universe per output
//      - 0 serial ports
//      - can receive 12 universes  
// P12S/P12D - 1 universe per output on 3.3
//           - 2 universes per output on 3.4
//           - 2 serial ports
//           - can receive 26 universes

class J1SysControllerRules : public ControllerRules
{
    int _outputs = 0;
public:
    J1SysControllerRules(int outputs) : ControllerRules(), _outputs(outputs) {}
    virtual ~J1SysControllerRules() {}
    virtual int GetMaxPixelPortChannels() const override 
    { 
        if (_outputs == 2) return 9999999; // we will detect this in a different way 
        if (_outputs == 12) return 170 * 3;
        return 0;
    }
    virtual int GetMaxPixelPort() const override { return _outputs; }
    virtual int GetMaxSerialPortChannels() const override { if (_outputs == 12) return 512; return 0; }
    virtual int GetMaxSerialPort() const override { if (_outputs == 12) return 2; return 0; }
    virtual bool IsValidPixelProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        return (p == "ws2811" || p == "tm180x" || p == "tm18xx" || p == "ws2801" || p == "tls3001" || p == "lpd6803");
    }
    virtual bool IsValidSerialProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        return (p == "renard" || p == "dmx");
    }
    virtual bool SupportsMultipleProtocols() const override { return true; }
    virtual bool AllUniversesSameSize() const override { return false; }
    virtual std::list<std::string> GetSupportedInputProtocols() const override {
        std::list<std::string> res;
        res.push_back("E131");
        res.push_back("ARTNET");
        return res;
    };
    virtual bool UniversesMustBeSequential() const override { return false; }
};

J1Sys::J1Sys(const std::string& ip)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _ip = ip;
    _outputs = 0;

    logger_base.debug("J1Sys upload tested to work with:");
    logger_base.debug("    ECG-P2 App Version 2.9b");
    logger_base.debug("    ECG-P12S App Version 3.3");

    _http.SetMethod("GET");
    _connected = _http.Connect(_ip);

    if (_connected)
    {
        std::string page = GetURL("/sysinfo.htm");
        if (page != "")
        {
            static wxRegEx versionregex("(App Version:\\<\\/b\\>\\<\\/td\\>\\<td\\>.nbsp;\\<\\/td\\>\\<td\\>)([^\\<]*)\\<", wxRE_ADVANCED | wxRE_NEWLINE);
            if (versionregex.Matches(wxString(page)))
            {
                _version = versionregex.GetMatch(wxString(page), 2).ToStdString();
                logger_base.debug("Connected to J1Sys controller version %s.", (const char *)_version.c_str());
            }
            static wxRegEx modelregex("(document\\.getElementById\\(.titleRight.\\)\\.innerHTML = .)([^\"]*)\"", wxRE_ADVANCED | wxRE_NEWLINE);
            if (modelregex.Matches(wxString(page)))
            {
                _model = modelregex.GetMatch(wxString(page), 2).ToStdString();
                logger_base.debug("     model %s.", (const char *)_model.c_str());
                static wxRegEx outputsregex("([0-9]+)", wxRE_ADVANCED);
                if (outputsregex.Matches(wxString(_model)))
                {
                    _outputs = wxAtoi(outputsregex.GetMatch(wxString(_model), 1));
                    logger_base.debug("     outputs %d.", _outputs);
                }
            }
        }
        else
        {
            _http.Close();
            _connected = false;
            logger_base.error("Error connecting to J1Sys controller on %s.", (const char *)_ip.c_str());
        }
    }
    else
    {
        logger_base.error("Error connecting to J1Sys controller on %s.", (const char *)_ip.c_str());
    }
}

int J1Sys::GetMaxStringOutputs() const
{
    return _outputs;
}

int J1Sys::GetMaxSerialOutputs() const
{
    if (_outputs == 12)
    {
        return 2;
    }

    return 0;
}

J1Sys::~J1Sys()
{
    _http.Close();
}

std::string J1Sys::GetURL(const std::string& url, bool logresult)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString res;

    if (!_http.IsConnected())
    {
        _connected = _http.Connect(_ip);
    }

    _http.SetMethod("GET");
    wxInputStream *httpStream = _http.GetInputStream(wxString(url));
    logger_base.debug("Making request to J1Sys '%s'.", (const char *)url.c_str());

    if (_http.GetError() == wxPROTO_NOERR)
    {
        wxStringOutputStream out_stream(&res);
        httpStream->Read(out_stream);

        if (logresult)
        {
            logger_base.debug("Response from J1Sys '%s' : %d.", (const char *)res.c_str(), _http.GetError());
        }
    }
    else
    {
        DisplayError(wxString::Format("Unable to connect to J1Sys '%s'.", url).ToStdString());
        res = "";
    }

    wxDELETE(httpStream);
    return res.ToStdString();
}

std::string J1Sys::PutURL(const std::string& url, const std::string& request, bool logresult)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString res;

    if (!_http.IsConnected())
    {
        _connected = _http.Connect(_ip);
    }

    _http.SetMethod("POST");
    _http.SetUser("admin");
    _http.SetPostText("application/x-www-form-urlencoded", request);
    wxInputStream *httpStream = _http.GetInputStream(wxString(url));
    logger_base.debug("Making request to J1Sys '%s'.", (const char *)url.c_str());
    logger_base.debug("    With data '%s'.", (const char *)request.c_str());

    int httpres = _http.GetError();

    if (httpres == wxPROTO_NOERR)
    {
        wxStringOutputStream out_stream(&res);
        httpStream->Read(out_stream);

        if (logresult)
        {
            logger_base.debug("Response from J1Sys '%s'.", (const char *)res.c_str());
        }
    }
    else
    {
        DisplayError(wxString::Format("Unable to connect to J1Sys '%s' => %d.", url, httpres).ToStdString());
    }
    _http.SetPostText("", "");

    wxDELETE(httpStream);
    return res.ToStdString();
}

bool J1Sys::SetInputUniverses(OutputManager* outputManager, std::list<int>& selected)
{
    wxASSERT(_outputs != 0);

    bool e131 = false;
    bool artnet = false;

    // Get universes based on IP
    std::list<Output*> outputs = outputManager->GetAllOutputs(_ip, selected);

    for (auto it = outputs.begin(); it != outputs.end(); ++it)
    {
        if ((*it)->GetType() == "E131")
        {
            e131 = true;
        }
        else if ((*it)->GetType() == "ArtNet")
        {
            artnet = true;
        }
    }

    if (_outputs == 2)
    {
        if (outputs.size() > 8)
        {
            DisplayError(wxString::Format("Attempt to upload %d universes to j1Sys P2 controller but only 8 are supported.", (int)outputs.size()).ToStdString());
            return false;
        }
    }
    else if (_outputs == 12)
    {
        if (outputs.size() > 12)
        {
            DisplayError(wxString::Format("Attempt to upload %d universes to j1Sys P12 controller but only 12 are supported.", (int)outputs.size()).ToStdString());
            return false;
        }
    }

    for (auto o:  outputs)
    {
        if (o->GetChannels() > 510)
        {
            DisplayError(wxString::Format("Attempt to upload universe %d to j1Sys controller of size %ld but maximum is 510.", o->GetUniverse(), o->GetChannels()).ToStdString());
            return false;
        }
    }

    std::string request = wxString::Format("an=0&e1en=%d&anen=%d", (e131) ? 1 : 0, (artnet) ? 1 : 0).ToStdString();
    std::string res = PutURL("/protect/ipConfig.htm", request);
    if (res != "")
    {
        return true;
    }

    return false;
}

bool j1syscompare_startchannel(const Model* first, const Model* second)
{
    int firstmodelstart = first->GetNumberFromChannelString(first->ModelStartChannel);
    int secondmodelstart = second->GetNumberFromChannelString(second->ModelStartChannel);

    return firstmodelstart < secondmodelstart;
}

bool J1Sys::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent)
{
    if (!SetInputUniverses(outputManager, selected))
    {
        return false;
    }

    //ResetStringOutputs(); // this shouldnt be used normally
    //ResetSerialOutputs(); // this shouldnt be used normally

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("J1Sys Outputs Upload: Uploading to %s", (const char *)_ip.c_str());

    std::string check;
    UDController cud(_ip, allmodels, outputManager, &selected, check);

    J1SysControllerRules rules(_outputs);
    bool success = cud.Check(&rules, check);

    cud.Dump();

    logger_base.debug(check);

    if (_outputs == 2)
    {
        if (success && cud.GetMaxPixelPort() > 0)
        {
            // one per config row
            std::vector<J1SysOutput> j1SysOutputs(_outputs*GetBankSize());

            ReadCurrentConfig(j1SysOutputs);
            logger_base.debug("Existing config:");
            DumpConfig(j1SysOutputs);

            for (int pp = 1; pp <= _outputs; pp++)
            {
                if (cud.HasPixelPort(pp))
                {
                    if (_outputs == 2)
                    {
                        //P2
                        UDControllerPort* port = cud.GetControllerPixelPort(pp);
                        long lastEnd = -1;
                        int output = (pp - 1) * GetBankSize();
                        int bankStart = (pp -1) * GetBankSize();

                        for (auto m : port->GetModels())
                        {
                            if (lastEnd != -1 && m->GetStartChannel() != lastEnd + 1)
                            {
                                output++;
                                lastEnd = -1;
                            }

                            j1SysOutputs[output].protocol = DecodeStringPortProtocol(m->GetProtocol());
                            j1SysOutputs[output].speed = DecodeProtocolSpeed(m->GetProtocol());

                            if (lastEnd == -1)
                            {
                                int channels = m->Channels();
                                while (channels > 0)
                                {
                                    if (output >= bankStart + GetBankSize())
                                    {
                                        DisplayError("Controller " + _ip + " too many outputs required for port " +wxString::Format("%d",pp) + ".");
                                        logger_base.debug("Erroneous config:");
                                        DumpConfig(j1SysOutputs);
                                        return false;
                                    }

                                    j1SysOutputs[output].active = true;
                                    for (int i = output % GetBankSize() + 1; i <GetBankSize(); i++)
                                    {
                                        j1SysOutputs[bankStart + i].active = false;
                                        j1SysOutputs[bankStart + i].universe = 0;
                                        j1SysOutputs[bankStart + i].startChannel = 0;
                                        j1SysOutputs[bankStart + i].pixels = 0;
                                    }
                                    long sc;
                                    auto o = outputManager->GetOutput(m->GetStartChannel() + m->Channels() - channels, sc);
                                    j1SysOutputs[output].universe = o->GetUniverse();
                                    j1SysOutputs[output].startChannel = sc;
                                    if (channels < o->GetChannels() - sc + 1)
                                    {
                                        j1SysOutputs[output].pixels = channels / 3;
                                        channels = 0;
                                    }
                                    else
                                    {
                                        j1SysOutputs[output].pixels = (o->GetChannels() - sc + 1) / 3;
                                        channels -= o->GetChannels() - sc + 1;
                                        output++;
                                    }
                                }
                                lastEnd = m->GetEndChannel();
                            }
                            else
                            {
                                int channels = m->Channels();
                                while (channels > 0)
                                {
                                    if (output >= bankStart + GetBankSize())
                                    {
                                        DisplayError("Controller " + _ip + " too many outputs required for port " + wxString::Format("%d", pp) + ".");
                                        logger_base.debug("Erroneous config:");
                                        DumpConfig(j1SysOutputs);
                                        return false;
                                    }

                                    long sc;
                                    auto o = outputManager->GetOutput(m->GetStartChannel() + m->Channels() - channels, sc);
                                    if (j1SysOutputs[output].universe == 0)
                                    {
                                        j1SysOutputs[output].universe = o->GetUniverse();
                                        j1SysOutputs[output].startChannel = sc;
                                        j1SysOutputs[output].active = true;
                                        for (int i = output % GetBankSize() + 1; i <GetBankSize(); i++)
                                        {
                                            j1SysOutputs[bankStart + i].active = false;
                                            j1SysOutputs[bankStart + i].universe = 0;
                                            j1SysOutputs[bankStart + i].startChannel = 0;
                                            j1SysOutputs[bankStart + i].pixels = 0;
                                        }
                                        j1SysOutputs[output].protocol = j1SysOutputs[(pp - 1) * GetBankSize()].protocol;
                                        j1SysOutputs[output].speed = j1SysOutputs[(pp - 1) * GetBankSize()].speed;
                                    }
                                    if (channels < o->GetChannels() - j1SysOutputs[output].startChannel + 1 - j1SysOutputs[output].pixels * 3)
                                    {
                                        j1SysOutputs[output].pixels += channels / 3;
                                        channels = 0;
                                    }
                                    else
                                    {
                                        j1SysOutputs[output].pixels = (o->GetChannels() - j1SysOutputs[output].startChannel + 1) / 3;
                                        channels -= o->GetChannels() - j1SysOutputs[output].startChannel + 1 - j1SysOutputs[output].pixels * 3;
                                        output++;
                                    }
                                }
                                lastEnd = m->GetEndChannel();
                            }
                        }

                        // make sure every row has a copy of the bank protocol and speed
                        for (auto i = 1; i < GetBankSize(); i++)
                        {
                            j1SysOutputs[bankStart + i].protocol = j1SysOutputs[bankStart].protocol;
                            j1SysOutputs[bankStart + i].speed = j1SysOutputs[bankStart].speed;
                        }
                    }
                    else
                    {
                        //P12
                    }
                }
                else
                {
                    // unused port
                }
            }

            int port = 0;
            for (auto& j : j1SysOutputs)
            {
                j.port = port;
                port++;
            }

            logger_base.debug("Uploading config:");
            DumpConfig(j1SysOutputs);

            logger_base.debug("Building upload:");
            std::string requestString;
            for (auto& j : j1SysOutputs)
            {
                if (requestString != "")
                    requestString += "&";
                requestString += BuildStringPort(j.active, j.port, j.protocol, j.speed, j.startChannel, j.universe, j.pixels, parent);
            }

            if (requestString != "")
            {
                std::string res = PutURL("/protect/stringConfig.htm", requestString);
                if (res == "")
                {
                    success = false;
                }
            }
        }
    }
    else
    {
        // old code for P12

        // build a list of models on this controller
        std::list<Model*> models;
        std::list<std::string> protocolsused;
        std::list<Model*> warnedmodels;
        int maxport = 0;

        // Get universes based on IP
        std::list<Output*> outputs = outputManager->GetAllOutputs(_ip, selected);

        for (auto ito = outputs.begin(); ito != outputs.end(); ++ito)
        {
            // this universe is sent to the SanDevices

            // find all the models in this range
            for (auto it = allmodels->begin(); it != allmodels->end(); ++it)
            {
                if (it->second->GetDisplayAs() != "ModelGroup")
                {
                    int modelstart = it->second->GetNumberFromChannelString(it->second->ModelStartChannel);
                    int modelend = modelstart + it->second->GetChanCount() - 1;
                    if ((modelstart >= (*ito)->GetStartChannel() && modelstart <= (*ito)->GetEndChannel()) ||
                        (modelend >= (*ito)->GetStartChannel() && modelend <= (*ito)->GetEndChannel()))
                    {
                        //logger_base.debug("Model %s start %d end %d found on controller %s output %d start %d end %d.",
                        //    (const char *)it->first.c_str(), modelstart, modelend,
                        //    (const char *)_ip.c_str(), node, currentcontrollerstartchannel, currentcontrollerendchannel);
                        if (!it->second->IsControllerConnectionValid())
                        {
                            // only warn if we have not already warned
                            if (std::find(warnedmodels.begin(), warnedmodels.end(), it->second) == warnedmodels.end())
                            {
                                warnedmodels.push_back(it->second);
                                DisplayWarning("Model " + it->first + " on controller " + _ip + " does not have its Contoller Connection details completed. Model ignored.");
                            }
                        }
                        else
                        {
                            // model uses channels in this universe

                            // check we dont already have this model in our list
                            if (std::find(models.begin(), models.end(), it->second) == models.end())
                            {
                                logger_base.debug("J1Sys Outputs Upload: Uploading Model %s.", (const char *)it->first.c_str());
                                models.push_back(it->second);
                                if (std::find(protocolsused.begin(), protocolsused.end(), it->second->GetProtocol()) == protocolsused.end())
                                {
                                    protocolsused.push_back(it->second->GetProtocol());
                                }
                                if (it->second->GetPort() > maxport)
                                {
                                    maxport = it->second->GetPort();
                                }
                            }
                        }
                    }
                }
            }
        }


        // sort the models by start channel
        models.sort(j1syscompare_startchannel);

        std::string requestString;
        std::string requestSerial;

        // for each protocol
        for (auto protocol = protocolsused.begin(); protocol != protocolsused.end(); ++protocol)
        {
            if (DecodeSerialPortProtocol(*protocol) >= 0 && GetMaxSerialOutputs() == 0)
            {
                DisplayError("Controller " + _ip + " protocol " + (*protocol) + " not supported by this controller.");
                success = false;

                continue;
            }

            bool portdone[100];
            memset(&portdone, 0x00, sizeof(portdone)); // all false

            // for each port ... this is the max of any port type but it should be ok
            for (int i = 1; i <= maxport; i++)
            {
                // find the first and last
                std::list<Output*> usedoutputs;
                Model* first = nullptr;
                Model* last = nullptr;
                int highestend = 0;
                long loweststart = 999999999;

                for (auto model = models.begin(); model != models.end(); ++model)
                {
                    if ((*model)->GetProtocol() == *protocol && (*model)->GetPort() == i)
                    {
                        int modelstart = (*model)->GetNumberFromChannelString((*model)->ModelStartChannel);
                        int modelend = modelstart + (*model)->GetChanCount() - 1;

                        for (int j = modelstart; j <= modelend; ++j)
                        {
                            long sc;
                            Output* o = outputManager->GetOutput(j, sc);
                            if (o != nullptr && std::find(usedoutputs.begin(), usedoutputs.end(), o) == usedoutputs.end())
                            {
                                usedoutputs.push_back(o);
                                j = o->GetStartChannel() + o->GetChannels() - 1;
                            }
                        }

                        if (modelstart < loweststart)
                        {
                            loweststart = modelstart;
                            first = *model;
                        }
                        if (modelend > highestend)
                        {
                            highestend = modelend;
                            last = *model;
                        }
                    }
                }

                if (first != nullptr)
                {
                    int portstart = first->GetNumberFromChannelString(first->ModelStartChannel);
                    int numstrings = first->GetNumPhysicalStrings();
                    int channelsperstring = first->NodesPerString() * first->GetChanCountPerNode();
                    // upload it
                    if (DecodeStringPortProtocol(*protocol) >= 0 || DecodeSerialPortProtocol(*protocol) >= 0)
                    {
                        if (first == last && numstrings > 1)
                        {
                            for (int j = 0; j < numstrings; j++)
                            {
                                if (portdone[i + j])
                                {
                                    DisplayError(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", first->GetName(), i + j).ToStdString());
                                    success = false;
                                }
                                else
                                {
                                    portdone[i + j] = true;
                                    int string = first->GetPort() - 1;
                                    if (_outputs == 2) string *= GetBankSize();
                                    int s = portstart + (i + j - 1) * channelsperstring;
                                    int e = s + channelsperstring - 1;
                                    long sc = 1;
                                    outputManager->GetOutput(first->GetFirstChannel() + 1, sc);

                                    if (_outputs == 12 && usedoutputs.size() > 1)
                                    {
                                        // P12 only supports 1 universe per output
                                        DisplayError(wxString::Format("J1Sys Outputs Upload: Controller %s protocol %s port %d does not support %d universes ... only 1.",
                                            _ip, *protocol, i, (int)usedoutputs.size()).ToStdString());
                                        success = false;
                                    }
                                    else if (_outputs == 2 && usedoutputs.size() > GetBankSize())
                                    {
                                        // P2 only supports 4 universes per output
                                        DisplayError(wxString::Format("J1Sys Outputs Upload: Controller %s protocol %s port %d does not support %d universes ... only 4.",
                                            _ip, *protocol, i, (int)usedoutputs.size()).ToStdString());
                                        success = false;
                                    }

                                    for (auto it = usedoutputs.begin(); it != usedoutputs.end(); ++it)
                                    {
                                        if ((*it)->GetStartChannel() <= e && (*it)->GetEndChannel() >= s)
                                        {
                                            int pixels = ((*it)->GetChannels() - sc + 1) / 3;
                                            if (_outputs == 12)
                                            {
                                                pixels = (last->GetLastChannel() - first->GetFirstChannel() + 1) / 3;
                                            }
                                            if (DecodeStringPortProtocol(*protocol) >= 0)
                                            {
                                                if (requestString != "")
                                                    requestString += "&";
                                                requestString += BuildStringPort(true, string++, DecodeStringPortProtocol(*protocol), DecodeProtocolSpeed(*protocol), sc, (*it)->GetUniverse(), pixels, parent);
                                            }
                                            else if (DecodeSerialPortProtocol(*protocol) >= 0)
                                            {
                                                if (requestSerial != "")
                                                    requestSerial += "&";
                                                requestSerial += BuildSerialPort(string++, DecodeSerialPortProtocol(*protocol), DecodeProtocolSpeed(*protocol), (*it)->GetUniverse(), parent);
                                            }
                                            sc = 1;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (portdone[i])
                            {
                                DisplayError(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", first->GetName(), i).ToStdString());
                                success = false;
                            }
                            else
                            {
                                portdone[i] = true;
                                int string = first->GetPort() - 1;
                                if (_outputs == 2) string *= GetBankSize();
                                long sc = 1;
                                outputManager->GetOutput(first->GetFirstChannel() + 1, sc);

                                if (_outputs == 12 && usedoutputs.size() > 1)
                                {
                                    // P12 only supports 1 universe per output
                                    logger_base.warn("J1Sys Outputs Upload: Controller %s protocol %s port %d does not support %d universes ... only 1.",
                                        (const char *)_ip.c_str(), (const char *)protocol->c_str(), i, (int)usedoutputs.size());
                                    wxMessageBox(wxString::Format("Attempt to upload port %d more than 1 universe %d.", i, (int)usedoutputs.size()));
                                    success = false;
                                }
                                else if (_outputs == 2 && usedoutputs.size() > GetBankSize())
                                {
                                    // P2 only supports 4 universes per output
                                    logger_base.warn("J1Sys Outputs Upload: Controller %s protocol %s port %d does not support %d universes ... only 4.",
                                        (const char *)_ip.c_str(), (const char *)protocol->c_str(), i, (int)usedoutputs.size());
                                    wxMessageBox(wxString::Format("Attempt to upload port %d more than 4 universes %d.", i, (int)usedoutputs.size()));
                                    success = false;
                                }

                                for (auto it = usedoutputs.begin(); it != usedoutputs.end(); ++it)
                                {
                                    int pixels = ((*it)->GetChannels() - sc + 1) / 3;
                                    if (_outputs == 12)
                                    {
                                        pixels = (last->GetLastChannel() - first->GetFirstChannel() + 1) / 3;
                                    }
                                    if (DecodeStringPortProtocol(*protocol) >= 0)
                                    {
                                        if (requestString != "")
                                            requestString += "&";
                                        requestString += BuildStringPort(true, string++, DecodeStringPortProtocol(*protocol), DecodeProtocolSpeed(*protocol), sc, (*it)->GetUniverse(), pixels, parent);
                                    }
                                    else if (DecodeSerialPortProtocol(*protocol) >= 0)
                                    {
                                        if (requestSerial != "")
                                            requestSerial += "&";
                                        requestSerial += BuildSerialPort(string++, DecodeSerialPortProtocol(*protocol), DecodeProtocolSpeed(*protocol), (*it)->GetUniverse(), parent);
                                    }
                                    sc = 1;
                                }
                            }
                        }
                    }
                    else
                    {
                        logger_base.warn("J1Sys Outputs Upload: Controller %s protocol %s not supported by this controller.",
                            (const char *)_ip.c_str(), (const char *)protocol->c_str());
                        wxMessageBox("Controller " + _ip + " protocol " + (*protocol) + " not supported by this controller.", "Protocol Ignored");
                        success = false;
                    }
                }
                else
                {
                    // nothing on this port ... ignore it
                }
            }
        }

        if (requestString != "")
        {
            std::string res = PutURL("/protect/stringConfig.htm", requestString);
            if (res == "")
            {
                success = false;
            }
        }

        if (requestSerial != "")
        {
            std::string res = PutURL("/protect/portConfig.htm", requestSerial);
            if (res == "")
            {
                success = false;
            }
        }

    }

    if (success) Reboot();

    return success;
}

void J1Sys::DumpConfig(const std::vector<J1SysOutput>& outputs) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    for (auto j : outputs)
    {
        logger_base.debug("   Port %c, Active: %d, Protocol: %c, Speed %d, Universe %d, StartChannel %d, Pixels %d", j.port + 65, j.active, j.protocol, j.speed, j.universe, j.startChannel, j.pixels);
    }
}

void J1Sys::ReadCurrentConfig(std::vector<J1SysOutput>& j)
{
    std::string config = GetURL("/protect/stringConfig.htm");

    if (!config.empty())
    {
        for (auto i = 0; i < j.size(); i++)
        {
            j[i].port = i;
            wxString activeRegex = wxString::Format("sA%c[^>]*checked", i + 65);
            wxRegEx ar(activeRegex);
            j[i].active = ar.Matches(wxString(config));

            if (i % GetBankSize() == 0)
            {
                wxString protocolRegex = wxString::Format("sT%c>[^#]*selected>([^<]*)", i + 65);
                wxRegEx pr(protocolRegex);
                if (pr.Matches(wxString(config)))
                {
                    j[i].protocol = DecodeStringPortProtocol(pr.GetMatch(wxString(config), 1));
                }
                wxString speedRegex = wxString::Format("sB%c[^>]*value=\"([^\"]*)\"", i + 65);
                wxRegEx sr(speedRegex);
                if (sr.Matches(wxString(config)))
                {
                    j[i].speed = wxAtoi(sr.GetMatch(wxString(config), 1));
                }
            }
            else
            {
                j[i].protocol = j[i / GetBankSize() * GetBankSize()].protocol;
                j[i].speed = j[i / GetBankSize() * GetBankSize()].speed;
            }

            wxString universeRegex = wxString::Format("sU%c[^>]*value=\"([0-9]*)", i + 65);
            wxRegEx ur(universeRegex);
            if (ur.Matches(wxString(config)))
            {
                j[i].universe = wxAtoi(ur.GetMatch(wxString(config), 1));
            }
            wxString startChannelRegex = wxString::Format("sS%c[^>]*value=\"([0-9]*)", i + 65);
            wxRegEx scr(startChannelRegex);
            if (scr.Matches(wxString(config)))
            {
                j[i].startChannel = wxAtoi(scr.GetMatch(wxString(config), 1));
            }
            wxString pixelsRegex = wxString::Format("sC%c[^>]*value=\"([0-9]*)", i + 65);
            wxRegEx pxr(pixelsRegex);
            if (pxr.Matches(wxString(config)))
            {
                j[i].pixels = wxAtoi(pxr.GetMatch(wxString(config), 1));
            }
        }
    }
}

int J1Sys::GetBankSize() const
{
    if (_outputs == 2) return 4;
    return 1;
}

char J1Sys::DecodeStringPortProtocol(std::string protocol) const
{
    wxString p(protocol);
    p = p.Lower();

    if (p == "ws2811") return '4';
    if (p == "tm180x") return '2';
    if (p == "tm18xx") return '2';
    if (p == "ws2801") return '1';
    if (p == "tls3001") return '3';
    if (p == "lpd6803") return '0';

    return -1;
}

char J1Sys::DecodeSerialPortProtocol(std::string protocol) const
{
    wxString p(protocol);
    p = p.Lower();

    if (p == "dmx") return 'D';
    if (p == "renard") return 'R';

    return -1;
}

int J1Sys::DecodeProtocolSpeed(std::string protocol) const
{
    wxString p(protocol);
    p = p.Lower();

    if (p == "ws2811") return 3600;
    if (p == "ws2801") return 750;
    if (p == "dmx") return 5;
    if (p == "renard") return 3;
    return 0;
}

std::string J1Sys::BuildStringPort(bool active, int string, char protocol, int speed, int startChannel, int universe, int pixels, wxWindow* parent) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int out = 65 + string;

    logger_base.debug("     Output String %d, Protocol %c Universe %d StartChannel %d Pixels %d",
        string, protocol, universe, startChannel, pixels);

    return wxString::Format("sA%c=%d&sT%c=%c&sB%c=%d&sU%c=%d&sS%c=%d&sC%c=%d",
        out, active ? 1 : 0,
        out, protocol,
        out, speed,
        out, universe,
        out, startChannel,
        out, pixels).ToStdString();
}

std::string J1Sys::BuildSerialPort(int string, char protocol, int speed, int universe, wxWindow* parent) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    string++;

    logger_base.debug("     Output Serial %d, Protocol %c Universe %d",
        string, protocol, universe);

    return wxString::Format("pA%d=1&pP%d=%c&pB%d=%d&pU%d=%d",
        string,
        string, protocol,
        string, speed,
        string, universe).ToStdString();
}

void J1Sys::ResetStringOutputs()
{
    PutURL("/protect/stringConfig.htm","");
}

void J1Sys::ResetSerialOutputs()
{
    if (_outputs == 12)
    {
        PutURL("/protect/portConfig.htm", "");
    }
}

void J1Sys::Reboot()
{
    GetURL("/protect/reboot.htm?");
}
