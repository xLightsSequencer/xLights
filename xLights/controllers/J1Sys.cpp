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

J1Sys::J1Sys(const std::string& ip)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _ip = ip;

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
                logger_base.error("Connected to J1Sys controller version %s.", (const char *)_version.c_str());
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
    return 2;
}

int J1Sys::GetMaxSerialOutputs() const
{
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
            logger_base.debug("Response from J1Sys '%s'.", (const char *)res.c_str());
        }
    }
    else
    {
        logger_base.error("Unable to connect to J1Sys '%s'.", (const char *)url.c_str());
        wxMessageBox(_T("Unable to connect!"));
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
        logger_base.error("Unable to connect to J1Sys '%s' => %d.", (const char *)url.c_str(), httpres);
        wxMessageBox(_T("Unable to connect!"));
    }
    _http.SetPostText("", "");

    wxDELETE(httpStream);
    return res.ToStdString();
}

bool J1Sys::SetInputUniverses(OutputManager* outputManager, std::list<int>& selected)
{
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

    std::string request = wxString::Format("an=0&e1en=%d&anen=%d", (e131) ? 1 : 0, (artnet) ? 1 : 0).ToStdString();
    std::string res = PutURL("/protect/ipConfig.htm", request);
    if (res != "")
    {
        Reboot();
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
    bool success = true;
    SetInputUniverses(outputManager, selected);

    ResetStringOutputs(); // this shouldnt be used normally

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("J1Sys Outputs Upload: Uploading to %s", (const char *)_ip.c_str());
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
                            logger_base.warn("J1Sys Outputs Upload: Model %s on controller %s does not have its Controller Connection details completed: '%s'. Model ignored.", (const char *)it->first.c_str(), (const char *)_ip.c_str(), (const char *)it->second->GetControllerConnection().c_str());
                            wxMessageBox("Model " + it->first + " on controller " + _ip + " does not have its Contoller Connection details completed: '" + it->second->GetControllerConnection() + "'. Model ignored.", "Model Ignored");
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

    std::string request;

    // for each protocol
    for (auto protocol = protocolsused.begin(); protocol != protocolsused.end(); ++protocol)
    {
        std::string sendmessage;

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
                int portend = last->GetNumberFromChannelString(last->ModelStartChannel) + last->GetChanCount() - 1;
                int numstrings = first->GetNumPhysicalStrings();
                int channelsperstring = first->NodesPerString() * first->GetChanCountPerNode();
                // upload it
                if (DecodeStringPortProtocol(*protocol) >= 0)
                {
                    if (first == last && numstrings > 1)
                    {
                        for (int j = 0; j < numstrings; j++)
                        {
                            if (portdone[i+j])
                            {
                                logger_base.warn("J1Sys Outputs Upload: Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i +j);
                                wxMessageBox(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i + j));
                                success = false;
                            }
                            else
                            {
                                portdone[i + j] = true;
                                int string = (i + j - 1) * 4;
                                int s = portstart + (i + j - 1) * channelsperstring;
                                int e = s + channelsperstring - 1;
                                for (auto it = usedoutputs.begin(); it != usedoutputs.end(); ++it)
                                {
                                    if ((*it)->GetStartChannel() <= e && (*it)->GetEndChannel() >= s)
                                    {
                                        if (request != "")
                                            request += "&";
                                        request += BuildStringPort(string++, DecodeStringPortProtocol(*protocol), DecodeProtocolSpeed(*protocol), 1, (*it)->GetUniverse(), (*it)->GetChannels() / 3, parent);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (portdone[i])
                        {
                            logger_base.warn("J1Sys Outputs Upload: Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str() , i);
                            wxMessageBox(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i));
                            success = false;
                        }
                        else
                        {
                            portdone[i] = true;
                            int string = (i-1) * 4;
                            for (auto it = usedoutputs.begin(); it != usedoutputs.end(); ++it)
                            {
                                if (request != "")
                                    request += "&";
                                request += BuildStringPort(string++, DecodeStringPortProtocol(*protocol), DecodeProtocolSpeed(*protocol), 1, (*it)->GetUniverse(), (*it)->GetChannels() / 3, parent);
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

    if (request != "")
    {
        std::string res = PutURL("/protect/stringConfig.htm", request);
        if (res != "")
        {
            Reboot();
            return success;
        }
    }

    return false;
}

char J1Sys::DecodeStringPortProtocol(std::string protocol)
{
    if (protocol == "ws2811") return '4';
    if (protocol == "tm18xx") return '2';
    if (protocol == "ws2801") return '1';
    if (protocol == "tls3001") return '3';
    if (protocol == "lpd6803") return '0';

    return -1;
}

int J1Sys::DecodeProtocolSpeed(std::string protocol)
{
    if (protocol == "ws2811") return 3600;
    if (protocol == "ws2801") return 750;

    return 0;
}

std::string J1Sys::BuildStringPort(int string, char protocol, int speed, int startChannel, int universe, int pixels, wxWindow* parent)
{
    int out = 65 + string;
    return wxString::Format("sA%c=1&sT%c=%c&sB%c=%d&sU%c=%d&sS%c=%d&sC%c=%d",
                            out,
                            out, protocol,
                            out, speed,
                            out, universe,
                            out, startChannel,
                            out, pixels).ToStdString();
}

void J1Sys::ResetStringOutputs()
{
    PutURL("/protect/stringConfig.htm","");
}

void J1Sys::Reboot()
{
    // TODO need to write this
}
