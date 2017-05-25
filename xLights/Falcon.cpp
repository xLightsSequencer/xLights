#include "Falcon.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/regex.h>
#include <wx/xml/xml.h>
#include "models/Model.h"
#include <log4cpp/Category.hh>
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "Models/ModelManager.h"

Falcon::Falcon(const std::string& ip)
{
	_ip = ip;

    _http.SetMethod("GET");
	_connected = _http.Connect(_ip);

    if (_connected)
    {
        std::string versionxml = GetURL("/status.xml");
        std::string version = GetURL("/index.htm");
        if (versionxml != "")
        {
            static wxRegEx versionregex("(\\<v\\>)([0-9]+\\.[0-9]+)\\<\\/v\\>", wxRE_ADVANCED | wxRE_NEWLINE);
            if (versionregex.Matches(wxString(versionxml)))
            {
                _version = versionregex.GetMatch(wxString(versionxml), 2).ToStdString();
            }
        }
        else
        {
            //<title>F4V2            - v1.10</title>
            static wxRegEx versionregex("(title.*?v)([0-9]+\\.[0-9]+)\\<\\/title\\>", wxRE_ADVANCED | wxRE_NEWLINE);
            if (versionregex.Matches(wxString(version)))
            {
                _version = versionregex.GetMatch(wxString(version), 2).ToStdString();
            }
        }
        static wxRegEx modelregex("(SW Version:.*?\\>)(F[0-9]+V[0-9]+)", wxRE_ADVANCED);
        if (modelregex.Matches(wxString(version)))
        {
            _model = modelregex.GetMatch(wxString(version), 2).ToStdString();
        }
    }
    else
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.error("Error connecting to falcon controller on %s.", (const char *)_ip.c_str());
    }
}

int Falcon::GetMaxStringOutputs() const
{
    if (_model == "F4V2")
    {
        return 8;
    }
    else if (_model == "F16V2")
    {
        return 32;
    }
    return 100;
}

int Falcon::GetMaxSerialOutputs() const
{
    if (_model == "F4V2")
    {
        return 1;
    }
    else if (_model == "F16V2")
    {
        return 4;
    }
    return 100;
}

Falcon::~Falcon()
{
    _http.Close();
}

std::string Falcon::GetURL(const std::string& url, bool logresult)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString res;
    _http.SetMethod("GET");
    wxInputStream *httpStream = _http.GetInputStream(wxString(url));
    logger_base.debug("Making request to falcon '%s'.", (const char *)url.c_str());

    if (_http.GetError() == wxPROTO_NOERR)
    {
        wxStringOutputStream out_stream(&res);
        httpStream->Read(out_stream);

        if (logresult)
        {
            logger_base.debug("Response from falcon '%s'.", (const char *)res.c_str());
        }
    }
    else
    {
        logger_base.error("Unable to connect to falcon '%s'.", (const char *)url.c_str());
        wxMessageBox(_T("Unable to connect!"));
        res = "";
    }

    wxDELETE(httpStream);
    return res.ToStdString();
}

std::string Falcon::PutURL(const std::string& url, const std::string& request, bool logresult)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString res;
    _http.SetMethod("POST");
    _http.SetPostText("application/x-www-form-urlencoded", request);
    wxInputStream *httpStream = _http.GetInputStream(wxString(url));
    logger_base.debug("Making request to falcon '%s'.", (const char *)url.c_str());
    logger_base.debug("    With data '%s'.", (const char *)request.c_str());

    if (_http.GetError() == wxPROTO_NOERR)
    {
        wxStringOutputStream out_stream(&res);
        httpStream->Read(out_stream);

        if (logresult)
        {
            logger_base.debug("Response from falcon '%s'.", (const char *)res.c_str());
        }
    }
    else
    {
        logger_base.error("Unable to connect to falcon '%s'.", (const char *)url.c_str());
        wxMessageBox(_T("Unable to connect!"));
    }
    _http.SetPostText("", "");

    wxDELETE(httpStream);
    return res.ToStdString();
}

//void Falcon::SetInputUniverses(OutputManager* outputManager)
//{
//    wxString request;
//    int output = 0;
//
//    auto outputs = outputManager->GetAllOutputs(_ip);
//
//    for (auto it = outputs.begin(); it != outputs.end(); ++it)
//    {
//        int t = -1;
//        if ((*it)->GetType() == "E131")
//        {
//            t = 0;
//        }
//        else if ((*it)->GetType() == "ArtNet")
//        {
//            t = 1;
//        }
//        request += wxString::Format("&u%d=%d&s%d=%d&c%d=%d&t%d=%d",
//            output, (*it)->GetUniverse(),
//            output, (*it)->GetChannels(),
//            output, (*it)->GetStartChannel(),
//            output, t);
//        output++;
//    }
//
//    request = wxString::Format("z=%d", output) + request;
//    std::string response = PutURL("/E131.htm", request.ToStdString());
//}

void Falcon::SetInputUniverses(OutputManager* outputManager, std::list<int>& selected)
{
    wxString request;
    int output = 0;

    // Get universes based on IP
    std::list<Output*> outputs = outputManager->GetAllOutputs(_ip, selected);

    for (auto it = outputs.begin(); it != outputs.end(); ++it)
    {
        int t = -1;
        if ((*it)->GetType() == "E131")
        {
            t = 0;
        }
        else if ((*it)->GetType() == "ArtNet")
        {
            t = 1;
        }
        request += wxString::Format("&u%d=%d&s%d=%d&c%d=%d&t%d=%d",
            output, (*it)->GetUniverse(),
            output, (*it)->GetChannels(),
            output, (*it)->GetStartChannel(),
            output, t);
        output++;
    }

    request = wxString::Format("z=%d&a=1", output) + request;
    std::string response = PutURL("/E131.htm", request.ToStdString());
}

//void Falcon::SetInputUniverses(const std::list<Output*>& inputs)
//{
//    wxString request;
//
//    request = wxString::Format("z=%d", inputs.size());
//
//    int output = 0;
//
//    for (auto it = inputs.begin(); it != inputs.end(); ++it)
//    {
//        int t = -1;
//
//        if ((*it)->GetType() == "E131")
//        {
//            t = 0;
//        }
//        else if ((*it)->GetType() == "ArtNet")
//        {
//            t = 1;
//        }
//
//        request += wxString::Format("&u%d=%d&s%d=%d&c%d=%d&t%d=%d",
//            output, (*it)->GetUniverse(),
//            output, (*it)->GetChannels(),
//            output, (*it)->GetStartChannel(),
//            output, t);
//        output++;
//    }
//
//    std::string response = PutURL("/E131.htm", request.ToStdString());
//}

bool compare_startchannel(const Model* first, const Model* second)
{
    int firstmodelstart = first->GetNumberFromChannelString(first->ModelStartChannel);
    int secondmodelstart = second->GetNumberFromChannelString(second->ModelStartChannel);

    return firstmodelstart < secondmodelstart;
}

void Falcon::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent)
{
    //ResetStringOutputs(); // this shouldnt be used normally

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Falcon Outputs Upload: Uploading to %s", (const char *)_ip.c_str());
    // build a list of models on this controller
    std::list<Model*> models;
    std::list<std::string> protocolsused;
    std::list<Model*> warnedmodels;
    int maxport = 0;

    // Get universes based on IP
    std::list<Output*> outputs = outputManager->GetAllOutputs(_ip, selected);

    for (auto ito = outputs.begin(); ito != outputs.end(); ++ito)
    {
        // this universe is sent to the falcon

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
                            logger_base.warn("Falcon Outputs Upload: Model %s on controller %s does not have its Controller Connection details completed: '%s'. Model ignored.", (const char *)it->first.c_str(), (const char *)_ip.c_str(), (const char *)it->second->GetControllerConnection().c_str());
                            wxMessageBox("Model " + it->first + " on controller " + _ip + " does not have its Contoller Connection details completed: '" + it->second->GetControllerConnection() + "'. Model ignored.", "Model Ignored");
                        }
                    }
                    else
                    {
                        // model uses channels in this universe

                        // check we dont already have this model in our list
                        if (std::find(models.begin(), models.end(), it->second) == models.end())
                        {
                            logger_base.debug("Falcon Outputs Upload: Uploading Model %s.", (const char *)it->first.c_str());
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
    models.sort(compare_startchannel);

    // get the current config before I start
    std::string strings = GetURL("/strings.xml");
    if (strings == "")
    {
        logger_base.error("Falcon Outputs Upload: Falcon would not return strings.xml.");
        wxMessageBox("Error occured trying to upload to Falcon.", "Error", wxOK, parent);
        return;
    }

    // for each protocol
    for (auto protocol = protocolsused.begin(); protocol != protocolsused.end(); ++protocol)
    {
        std::string sendmessage;
        int count = 0;

        bool portdone[100];
        memset(&portdone, 0x00, sizeof(portdone)); // all false

        // for each port ... this is the max of any port type but it should be ok
        for (int i = 1; i <= maxport; i++)
        {
            // find the first and last
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
                int numstrings = first->GetNumStrings();
                bool multistringelement = (first->GetDisplayAs() == "Matrix" || 
                    first->GetDisplayAs() == "Tree" ||
                    first->GetDisplayAs() == "Circle" ||
                    first->GetDisplayAs() == "Star" ||
                    first->GetDisplayAs() == "Wreath" ||
                    first->GetDisplayAs() == "Icicles");
                int channelsperstring = first->NodesPerString() * first->GetChanCountPerNode();
                // upload it
                if (DecodeStringPortProtocol(*protocol) >= 0)
                {
                    if (first == last && numstrings > 1 && multistringelement)
                    {
                        for (int j = 0; j < numstrings; j++)
                        {
                            if (portdone[i+j])
                            {
                                logger_base.warn("Falcon Outputs Upload: Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i +j);
                                wxMessageBox(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i + j));
                            }
                            else
                            {
                                portdone[i + j] = true;
                                count++;
                                if (sendmessage != "") sendmessage = sendmessage + "&";
                                long startChannel;
                                Output* output = outputManager->GetOutput(portstart + j * channelsperstring, startChannel);
                                sendmessage = sendmessage + BuildStringPort(strings, i + j, DecodeStringPortProtocol(*protocol), startChannel, output->GetUniverse(), channelsperstring / 3, first->GetName(), parent);
                                if (count == 40)
                                {
                                    UploadStringPort(sendmessage, false);
                                    sendmessage = "";
                                    count = 0;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (portdone[i])
                        {
                            logger_base.warn("Falcon Outputs Upload: Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str() , i);
                            wxMessageBox(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i));
                        }
                        else
                        {
                            portdone[i] = true;
                            count++;
                            if (sendmessage != "") sendmessage = sendmessage + "&";
                            long startChannel;
                            Output* output = outputManager->GetOutput(portstart, startChannel);
                            sendmessage = sendmessage + BuildStringPort(strings, i, DecodeStringPortProtocol(*protocol), startChannel, output->GetUniverse(), (portend - portstart + 1) / 3, first->GetName(), parent);
                            if (count == 40)
                            {
                                UploadStringPort(sendmessage, false);
                                sendmessage = "";
                                count = 0;
                            }
                        }
                    }                    
                }
                else if (DecodeSerialOutputProtocol(*protocol) >= 0)
                {
                    UploadSerialOutput(i, DecodeSerialOutputProtocol(*protocol), portstart, parent);
                }
                else
                {
                    logger_base.warn("Falcon Outputs Upload: Controller %s protocol %s not supported by this controller.",
                        (const char *)_ip.c_str(), (const char *)protocol->c_str());
                    wxMessageBox("Controller " + _ip + " protocol " + (*protocol) + " not supported by this controller.", "Protocol Ignored");
                }
            }
            else
            {
                // nothing on this port ... ignore it
            }
        }
        UploadStringPort(sendmessage, true);
    }
}

int Falcon::DecodeStringPortProtocol(std::string protocol)
{
    if (protocol == "ws2811") return 0;
    if (protocol == "tm18xx") return 1;
    if (protocol == "lx1203") return 2;
    if (protocol == "ws2801") return 3;
    if (protocol == "tls3001") return 4;
    if (protocol == "lpd6803") return 5;
    if (protocol == "gece") return 6;

    return -1;
}

void Falcon::UploadStringPort(const std::string& request, bool final)
{
    std::string r = request;
    if (final)
    {
        r = r + "&r=1";
    }
    else
    {
        r = r + "&r=0";
    }
    PutURL("/StringPorts.htm", r);
}

std::string Falcon::BuildStringPort(const std::string& strings, int output, int protocol, int portstartchannel, int universe, int pixels, const std::string& description, wxWindow* parent)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int row = -1;

    // first I need to check if they have virtual strings ... as my code cant handle that as we dont have all the information we need
    wxStringInputStream strm(wxString(strings.c_str()));
    wxXmlDocument stringsdoc(strm);
    int vscount = 0;
    int rowcount = 0;
    
    if (stringsdoc.GetRoot() == nullptr)
    {
        logger_base.error("Falcon Outputs Upload: Falcon would not return strings.xml.");
        wxMessageBox("Error occured trying to upload to Falcon.", "Error", wxOK, parent);
        return "";
    }

    for (auto e = stringsdoc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext())
    {
        if (wxAtoi(e->GetAttribute("p")) == output - 1)
        {
            if (row < 0) row = rowcount;
            vscount++;
        }
        rowcount++;
    }

    if (row < 0) row = rowcount++;

    if (vscount > 1)
    {
        if (wxMessageBox("String Port "+wxString::Format("%d",output)+" has virtual strings defined. Proceeding will overwrite the first one only and will need to be manually corrected. Are you sure you want to do this?", "Are you sure?", wxYES_NO, parent) == wxYES)
        {
            // ok let it happen
            logger_base.warn("Falcon Outputs Upload: User chose to upload string port output %d even though it had %d virtual strings defined.", output, vscount - 1);
        }
        else
        {
            return "";
        }
    }

    if (row >= GetMaxStringOutputs())
    {
        logger_base.warn("Falcon Outputs Upload: Falcon %s only supports %d outputs. Attempt to upload to output %d.", (const char *)_model.c_str(), GetMaxStringOutputs(), output);
        wxMessageBox("Falcon " + wxString(_model.c_str()) + " only supports " + wxString::Format("%d", GetMaxStringOutputs()) + " outputs. Attempt to upload to output " + wxString::Format("%d", output) + ".", "Invalid String Output", wxOK, parent);
        return "";
    }

    wxString request = wxString::Format("r=0&p%d=%d&t%d=%d&u%d=%d&s%d=%d&c%d=%d&y%d=%s", 
                                        row, output - 1,
                                        row, protocol,
                                        row, universe,
                                        row, portstartchannel,
                                         row, pixels, 
                                         row, wxString(description.c_str()));
    return request.ToStdString();
}

void Falcon::ResetStringOutputs()
{
    PutURL("/StringPorts.htm", "S=4&p0=0&p1=1&p2=2&p3=3");
}

int Falcon::DecodeSerialOutputProtocol(std::string protocol)
{
    if (protocol == "dmx") return 0;
    if (protocol == "pixelnet") return 1;
    if (protocol == "renard") return 2;
    return -1;
}
void Falcon::UploadSerialOutput(int output, int protocol, int portstart, wxWindow* parent)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (output >= GetMaxSerialOutputs())
    {
        logger_base.warn("Falcon Outputs Upload: Falcon %s only supports %d serial outputs. Attempt to upload to serail output %d.", (const char *)_model.c_str(), GetMaxStringOutputs(), output);
        wxMessageBox("Falcon " + wxString(_model.c_str()) + " only supports " + wxString::Format("%d", GetMaxSerialOutputs()) + " outputs. Attempt to upload to output " + wxString::Format("%d", output) + ".", "Invalid Serial Output", wxOK, parent);
        return;
    }

    wxString request = wxString::Format("t%d=%d&s%d=%d", output, protocol, output, portstart);
    PutURL("/SerialOutputs.htm", request.ToStdString());
}
