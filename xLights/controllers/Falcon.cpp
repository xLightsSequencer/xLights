#include "Falcon.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/regex.h>
#include <wx/xml/xml.h>
#include "models/Model.h"
#include <log4cpp/Category.hh>
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "models/ModelManager.h"
#include <wx/progdlg.h>

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
    if (_model == "F4V2" || _model == "F4V3")
    {
        return 12;
    }
    else if (_model == "F16V2")
    {
        return 32;
    }
    else if (_model == "F16V3")
    {
        return 48;
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

bool Falcon::SetInputUniverses(OutputManager* outputManager, std::list<int>& selected)
{
    wxString request;
    int output = 0;
    bool success = true;

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
    return (response != "");
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

bool Falcon::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent)
{
    //ResetStringOutputs(); // this shouldnt be used normally

    wxProgressDialog progress("Uploading ...", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    progress.Show();

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Falcon Outputs Upload: Uploading to %s", (const char *)_ip.c_str());
    // build a list of models on this controller
    std::list<Model*> models;
    std::list<std::string> protocolsused;
    std::list<Model*> warnedmodels;
    int maxport = 0;
    bool success = true;

    // Get universes based on IP
    std::list<Output*> outputs = outputManager->GetAllOutputs(_ip, selected);

    progress.Update(0, "Scanning models");

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

    if (warnedmodels.size() > 0)
    {
        std::string m = "";

        for (auto it = warnedmodels.begin(); it != warnedmodels.end(); ++it)
        {
            m = m + (*it)->GetName() + "\n";
        }

        wxMessageBox("Models\n\n" + m + "\non controller " + _ip + " do not have their Contoller Connection details completed. Models ignored.", "Models Ignored");
    }

    // sort the models by start channel
    models.sort(compare_startchannel);

    progress.Update(10, "Retrieving string configuration from Falcon.");

    // get the current config before I start
    std::string strings = GetURL("/strings.xml");
    if (strings == "")
    {
        progress.Update(100, "Aborting.");
        logger_base.error("Falcon Outputs Upload: Falcon would not return strings.xml.");
        wxMessageBox("Error occured trying to upload to Falcon.", "Error", wxOK, parent);
        return false;
    }

    int currentStrings = CountStrings(strings);
    logger_base.info("Falcon has %d strings. Max port we are trying to use is %d.", currentStrings, maxport);

    int mainPixels = MaxPixels(strings, 0);
    int daughter1Pixels = MaxPixels(strings, 1);
    int daughter2Pixels = MaxPixels(strings, 2);

    bool changed = false;
    if (_model == "F16V2" || _model == "F16V3")
    {
        if (maxport > 32 && currentStrings < 48)
        {
            logger_base.info("Adjusting string port count to 48.");
            progress.Update(25, "Adjusting string port count to 48.");
            InitialiseStrings("m=2&S=48", currentStrings + 1, 48, mainPixels, daughter1Pixels, daughter2Pixels);
            changed = true;
        }
        else if (maxport > 16 && currentStrings < 32)
        {
            logger_base.info("Adjusting string port count to 32.");
            progress.Update(25, "Adjusting string port count to 32.");
            InitialiseStrings("m=1&S=32", currentStrings + 1, 32, mainPixels, daughter1Pixels, daughter2Pixels);
            changed = true;
        }
    }
    else
    {
        if (maxport > 4 && currentStrings != 12)
        {
            logger_base.info("Adjusting string port count to 12.");
            progress.Update(25, "Adjusting string port count to 12.");
            InitialiseStrings("m=1&S=12", currentStrings + 1, 12, mainPixels, daughter1Pixels, daughter2Pixels);
            changed = true;
        }
    }

    if (changed)
    {
        progress.Update(35, "Retrieving string configuration from Falcon ... again.");
        // reget the strings
        strings = GetURL("/strings.xml");
        if (strings == "")
        {
            progress.Update(100, "Aborting.");
            logger_base.error("Falcon Outputs Upload: Falcon would not return strings.xml.");
            wxMessageBox("Error occured trying to upload to Falcon.", "Error", wxOK, parent);
            return false;
        }
    }

    mainPixels = MaxPixels(strings, 0);
    daughter1Pixels = MaxPixels(strings, 1);
    daughter2Pixels = MaxPixels(strings, 2);

    logger_base.info("Falcon pixel split: Main = %d, Expansion1 = %d, Expansion2 = %d", mainPixels, daughter1Pixels, daughter2Pixels);

    progress.Update(50, "Configuring string ports.");

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
                                logger_base.warn("Falcon Outputs Upload: Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i +j);
                                wxMessageBox(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i + j));
                                success = false;
                            }
                            else
                            {
                                portdone[i + j] = true;
                                count++;
                                if (sendmessage != "") sendmessage = sendmessage + "&";
                                long startChannel;
                                Output* output = outputManager->GetOutput(portstart + j * channelsperstring, startChannel);
                                std::string portmessage = BuildStringPort(strings, i + j, DecodeStringPortProtocol(*protocol), startChannel, output->GetUniverse(), channelsperstring / 3, first->GetName(), parent, mainPixels, daughter1Pixels, daughter2Pixels);

                                if (portmessage == "ABORT")
                                {
                                    return false;
                                }

                                sendmessage = sendmessage + portmessage;

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
                            success = false;
                        }
                        else
                        {
                            portdone[i] = true;
                            count++;
                            if (sendmessage != "") sendmessage = sendmessage + "&";
                            long startChannel;
                            Output* output = outputManager->GetOutput(portstart, startChannel);
                            std::string portmessage = BuildStringPort(strings, i, DecodeStringPortProtocol(*protocol), startChannel, output->GetUniverse(), (portend - portstart + 1) / 3, first->GetName(), parent, mainPixels, daughter1Pixels, daughter2Pixels);
                            sendmessage = sendmessage + portmessage;

                            if (portmessage == "ABORT")
                            {
                                return false;
                            }

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
                    UploadSerialOutput(i, outputManager, DecodeSerialOutputProtocol(*protocol), portstart, parent);
                }
                else
                {
                    logger_base.warn("Falcon Outputs Upload: Controller %s protocol %s not supported by this controller.",
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
        UploadStringPort(sendmessage, true);
    }

    progress.Update(100, "Done.");

    return success;
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

void Falcon::InitialiseStrings(const std::string& prefix, int start, int end, int mainPixels, int daughter1Pixels, int daughter2Pixels)
{
    std::string request = prefix;

    if (end == 32)
    {
        if (daughter1Pixels == 0)
        {
            if (request != "")
            {
                request += "&";
            }
            request += wxString::Format("k0=%d&k1=%d", mainPixels - 1, 1);
        }
    }
    else if (end == 48)
    {
        if (daughter1Pixels == 0)
        {
            daughter1Pixels = 1;
            mainPixels -= 1;
        }
        if (daughter2Pixels == 0)
        {
            daughter2Pixels = 1;
            mainPixels -= 1;
        }

        if (request != "")
        {
            request += "&";
        }
        request += wxString::Format("k0=%d&k1=%d&k2=%d", mainPixels, daughter1Pixels, daughter2Pixels);
    }
    else if (end == 12)
    {
        if (daughter1Pixels == 0)
        {
            if (request != "")
            {
                request += "&";
            }
            request += wxString::Format("k0=%d&k1=%d", mainPixels - 1, 1);
        }
    }

    for (int i = start-1; i < end; ++i)
    {
        if (request != "")
        {
            request += "&";
        }
        request += wxString::Format("p%d=%d&u%d=1&s%d=1&c%d=50", i, i, i, i, i);
    }

    UploadStringPort(request, true);
}

void Falcon::UploadStringPort(const std::string& request, bool final)
{
    std::string r = request;
    if (final)
    {
        r = "r=1&" + r;
    }
    else
    {
        r = "r=0&" + r;
    }

    PutURL("/StringPorts.htm", r);
}

int Falcon::MaxPixels(const std::string& strings, int board)
{
    wxStringInputStream strm(wxString(strings.c_str()));
    wxXmlDocument stringsdoc(strm);

    if (stringsdoc.GetRoot() == nullptr)
    {
        return 0;
    }

    switch(board)
    {
    case 0:
        return wxAtoi(stringsdoc.GetRoot()->GetAttribute("k0"));
    case 1:
        return wxAtoi(stringsdoc.GetRoot()->GetAttribute("k1"));
    case 2:
        return wxAtoi(stringsdoc.GetRoot()->GetAttribute("k2"));
    }

    return 0;
}

int Falcon::CountStrings(const std::string& strings)
{
    wxStringInputStream strm(wxString(strings.c_str()));
    wxXmlDocument stringsdoc(strm);

    if (stringsdoc.GetRoot() == nullptr)
    {
        return 0;
    }

    int count = wxAtoi(stringsdoc.GetRoot()->GetAttribute("c"));

    if (count == 0)
    {
        for (auto e = stringsdoc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext())
        {
            count = std::max(count, wxAtoi(e->GetAttribute("p")) + 1);
        }
    }

    return count;
}

std::string Falcon::BuildStringPort(const std::string& strings, int output, int protocol, int portstartchannel, int universe, int pixels, const std::string& description, wxWindow* parent, int mainPixels, int daughter1Pixels, int daughter2Pixels)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int row = -1;

    if (output <= 16)
    {
        if (pixels > mainPixels)
        {
            wxString msg = wxString::Format("Output %d on main board has %d pixels but falcon is configured for a maximum of %d pixels. Aborting upload.", output, pixels, mainPixels);
            logger_base.error(msg.c_str());
            wxMessageBox(msg, "Error", wxOK, parent);
            return "ABORT";
        }
    }
    else if (output <= 32)
    {
        if (pixels > daughter1Pixels)
        {
            wxString msg = wxString::Format("Output %d on expansion board 1 has %d pixels but falcon is configured for a maximum of %d pixels. Aborting upload.", output, pixels, daughter1Pixels);
            logger_base.error(msg.c_str());
            wxMessageBox(msg, "Error", wxOK, parent);
            return "ABORT";
        }
    }
    else
    {
        if (pixels > daughter2Pixels)
        {
            wxString msg = wxString::Format("Output %d on expansion board 2 has %d pixels but falcon is configured for a maximum of %d pixels. Aborting upload.", output, pixels, daughter2Pixels);
            logger_base.error(msg.c_str());
            wxMessageBox(msg, "Error", wxOK, parent);
            return "ABORT";
        }
    }

    wxString desc(description);
    int replaced = desc.Replace("  ", " ");
    while (replaced != 0)
    {
        replaced = desc.Replace("  ", " ");
    }

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

    wxString request = wxString::Format("p%d=%d&t%d=%d&u%d=%d&s%d=%d&c%d=%d&y%d=%s", 
                                        row, output - 1,
                                        row, protocol,
                                        row, universe,
                                        row, portstartchannel,
                                         row, pixels, 
                                         row, desc);
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

void Falcon::UploadSerialOutput(int output, OutputManager* outputManager, int protocol, int portstart, wxWindow* parent)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (output >= GetMaxSerialOutputs())
    {
        logger_base.warn("Falcon Outputs Upload: Falcon %s only supports %d serial outputs. Attempt to upload to serail output %d.", (const char *)_model.c_str(), GetMaxStringOutputs(), output);
        wxMessageBox("Falcon " + wxString(_model.c_str()) + " only supports " + wxString::Format("%d", GetMaxSerialOutputs()) + " outputs. Attempt to upload to output " + wxString::Format("%d", output) + ".", "Invalid Serial Output", wxOK, parent);
        return;
    }

    long sc;
    auto o = outputManager->GetOutput(portstart, sc);

    if (o != nullptr)
    {
        wxString request = wxString::Format("btnSave=Save&t%d=%d&u%d=%d&s%d=%d", 
            output-1, protocol, 
            output-1, o->GetUniverse(), 
            output-1, sc);
        PutURL("/SerialOutputs.htm", request.ToStdString());
    }
    else
    {
        logger_base.warn("Error uploading serial output to falcon. %d does not map to a universe.", portstart);
        wxMessageBox("Error uploading serial output to falcon. "+ wxString::Format("%i", portstart) +" does not map to a universe.");
    }
}
