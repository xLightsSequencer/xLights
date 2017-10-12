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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _ip = ip;
    _firmwareVersion = "";
    _modelString = "";
    _version = 0;
    _model = 0;

    _http.SetMethod("GET");
    _connected = _http.Connect(_ip);

    if (_connected)
    {
        std::string versionxml = GetURL("/status.xml");
        if (versionxml == "")
        {
            logger_base.error("Error connecting to falcon controller on %s.", (const char *)_ip.c_str());
            _connected = false;
            return;
        }

        std::string version = GetURL("/index.htm");
        if (versionxml != "")
        {
            static wxRegEx versionregex("(\\<v\\>)([0-9]+\\.[0-9]+)\\<\\/v\\>", wxRE_ADVANCED | wxRE_NEWLINE);
            if (versionregex.Matches(wxString(versionxml)))
            {
                _firmwareVersion = versionregex.GetMatch(wxString(versionxml), 2).ToStdString();
            }
        }

        if (_firmwareVersion == "")
        {
            //<title>F4V2            - v1.10</title>
            static wxRegEx firmwareversionregex("(title.*?v)([0-9]+\\.[0-9]+)\\<\\/title\\>", wxRE_ADVANCED | wxRE_NEWLINE);
            if (firmwareversionregex.Matches(wxString(version)))
            {
                _firmwareVersion = firmwareversionregex.GetMatch(wxString(version), 2).ToStdString();
            }
        }

        static wxRegEx modelstringregex("(SW Version:.*?\\>)(F[0-9]+V[0-9]+)", wxRE_ADVANCED);
        if (modelstringregex.Matches(wxString(version)))
        {
            _modelString = modelstringregex.GetMatch(wxString(version), 2).ToStdString();
        }

        static wxRegEx versionregex("(F[0-9]+V)([0-9]+)", wxRE_ADVANCED);
        if (versionregex.Matches(wxString(version)))
        {
            _version = wxAtoi(versionregex.GetMatch(wxString(version), 2).ToStdString());
        }

        static wxRegEx modelregex("(F)([0-9]+)V", wxRE_ADVANCED);
        if (modelregex.Matches(wxString(version)))
        {
            _model = wxAtoi(modelregex.GetMatch(wxString(version), 2).ToStdString());
        }

        logger_base.debug("Connected to falcon - Model: %s Firmware Version %s. %d:%d", _modelString.c_str(), _firmwareVersion.c_str(), _model, _version);
    }
    else
    {
        logger_base.error("Error connecting to falcon controller on %s.", (const char *)_ip.c_str());
    }

    if (_version == 0 || _model == 0)
    {
        _connected = false;
    }
}

int Falcon::GetMaxStringOutputs() const
{
    if (IsF4())
    {
        return 12;
    }
    else if (IsF16() && IsV2())
    {
        return 32;
    }
    else if (IsF16() && IsV3())
    {
        return 48;
    }
    return 100;
}

int Falcon::GetMaxSerialOutputs() const
{
    if (IsF4())
    {
        return 1;
    }
    else if (IsF16())
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

bool Falcon::SetInputUniverses(OutputManager* outputManager, std::list<int>& selected)
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
    return (response != "");
}

bool compare_startchannel(const Model* first, const Model* second)
{
    int firstmodelstart = first->GetNumberFromChannelString(first->ModelStartChannel);
    int secondmodelstart = second->GetNumberFromChannelString(second->ModelStartChannel);

    return firstmodelstart < secondmodelstart;
}

std::string Falcon::SafeDescription(const std::string description) const
{
    wxString desc(description);
    int replaced = desc.Replace("  ", " ");
    while (replaced != 0)
    {
        replaced = desc.Replace("  ", " ");
    }
    return desc.Left(25).ToStdString();
}

int Falcon::GetVirtualStringPixels(const std::vector<FalconString*> &virtualStringData, int port)
{
    int count = 0;
    for (int i = 0; i < virtualStringData.size(); ++i)
    {
        if (virtualStringData[i]->port == port)
        {
            count += virtualStringData[i]->pixels;
        }
    }
    return count;
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
    logger_base.info("Scanning models.");

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
                            logger_base.debug("Falcon Outputs Upload: Uploading Model %s. %s:%d", (const char *)it->first.c_str(), (const char *)it->second->GetProtocol().c_str(), it->second->GetPort());
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
    logger_base.info("Retrieving string configuration from Falcon.");

    // get the current config before I start
    std::string strings = GetURL("/strings.xml");
    if (strings == "")
    {
        progress.Update(100, "Aborting.");
        logger_base.error("Falcon Outputs Upload: Falcon would not return strings.xml.");
        wxMessageBox("Error occured trying to upload to Falcon.", "Error", wxOK, parent);
        return false;
    }

    std::vector<FalconString*> stringData;
    std::vector<FalconString*> virtualStringData;

    wxStringInputStream strm(wxString(strings.c_str()));
    wxXmlDocument stringsDoc(strm);

    if (!stringsDoc.IsOk())
    {
        progress.Update(100, "Aborting.");
        logger_base.error("Falcon Outputs Upload: Could not parse Falcon strings.xml.");
        wxMessageBox("Error occured trying to upload to Falcon.", "Error", wxOK, parent);
        return false;
    }

    progress.Update(40, "Processing current configuration data.");
    logger_base.info("Processing current configuration data.");

    int currentStrings = CountStrings(stringsDoc);
    int mainPixels = GetMaxPixels();
    int daughter1Pixels = 0;
    int daughter2Pixels = 0;
    if (SupportsVariableExpansions())
    {
        mainPixels = MaxPixels(stringsDoc, 0);
        daughter1Pixels = MaxPixels(stringsDoc, 1);
        daughter2Pixels = MaxPixels(stringsDoc, 2);
    }
    else
    {
        if (currentStrings > GetDaughter1Threshold())
        {
            mainPixels = mainPixels / 2;
            daughter1Pixels = mainPixels;
        }
    }
 
    logger_base.info("Current Falcon configuration split: Main = %d, Expansion1 = %d, Expansion2 = %d, Strings = %d", mainPixels, daughter1Pixels, daughter2Pixels, currentStrings);
    logger_base.info("Maximum string port configured in xLights: %d", maxport);

    int virtualStrings = ReadStringData(stringsDoc, stringData, virtualStringData);

    if (virtualStrings > 0)
    {
        if (wxMessageBox("At least one String Port has virtual strings defined. Proceeding will overwrite the first one only and will need to be manually corrected. Are you sure you want to do this?", "Are you sure?", wxYES_NO, parent) == wxYES)
        {
            // ok let it happen
            logger_base.warn("Falcon Outputs Upload: User chose to upload string port outputs even though it had %d virtual strings defined.", virtualStrings);
        }
        else
        {
            return false;
        }
    }

    int maxPixels = GetMaxPixels();

    if (maxport > GetDaughter2Threshold() && currentStrings < GetMaxStringOutputs())
    {
        logger_base.info("Adjusting string port count to %d.", GetMaxStringOutputs());
        progress.Update(45, "Adjusting string port count.");
        InitialiseStrings(stringData, GetMaxStringOutputs(), virtualStrings);
    }
    else if (maxport > GetDaughter1Threshold() && currentStrings < GetDaughter2Threshold())
    {
        logger_base.info("Adjusting string port count to %d.", GetDaughter2Threshold());
        progress.Update(45, "Adjusting string port count.");
        InitialiseStrings(stringData, GetDaughter2Threshold(), virtualStrings);
    }

    logger_base.info("Falcon pixel split: Main = %d, Expansion1 = %d, Expansion2 = %d", mainPixels, daughter1Pixels, daughter2Pixels);

    progress.Update(50, "Configuring string ports.");
    logger_base.info("Configuring string ports.");

    bool portdone[100];
    memset(&portdone, 0x00, sizeof(portdone)); // all false

    // for each protocol
    for (auto protocol = protocolsused.begin(); protocol != protocolsused.end(); ++protocol)
    {
        logger_base.info("Protocol %s.", (const char *)protocol->c_str());

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
                logger_base.debug("First model on port %d: %s.", i, (const char *)first->GetName().c_str());

                int portstart = first->GetNumberFromChannelString(first->ModelStartChannel);
                int portend = last->GetNumberFromChannelString(last->ModelStartChannel) + last->GetChanCount() - 1;
                int numstrings = first->GetNumPhysicalStrings();
                int channelsperstring = first->NodesPerString() * first->GetChanCountPerNode();

                if (DecodeStringPortProtocol(*protocol) >= 0)
                {
                    if (first == last && numstrings > 1)
                    {
                        logger_base.debug("Model has %d strings.", numstrings);
                        for (int j = 0; j < numstrings; j++)
                        {
                            if (portdone[i + j])
                            {
                                logger_base.warn("Falcon Outputs Upload: Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i + j);
                                wxMessageBox(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i + j));
                                success = false;
                            }
                            else
                            {
                                portdone[i + j] = true;
                                long startChannel;
                                Output* output = outputManager->GetOutput(portstart + j * channelsperstring, startChannel);

                                if (output == nullptr || FindPort(stringData, i + j - 1) == nullptr)

                                {
                                    logger_base.warn("Falcon Outputs Upload: Attempt to find output for channel %ld failed. Do you have enough outputs?", portstart + j * channelsperstring);
                                    wxMessageBox(wxString::Format("Attempt to find output for channel %ld failed. Do you have enough outputs?", portstart + j * channelsperstring));
                                    success = false;
                                }
                                else
                                {
                                    FindPort(stringData, i + j - 1)->protocol = DecodeStringPortProtocol(*protocol);
                                    FindPort(stringData, i + j - 1)->universe = output->GetUniverse();
                                    FindPort(stringData, i + j - 1)->startChannel = startChannel;
                                    FindPort(stringData, i + j - 1)->pixels = channelsperstring / 3;
                                    FindPort(stringData, i + j - 1)->description = SafeDescription(first->GetName());
                                }
                            }
                        }
                    }
                    else
                    {
                        if (portdone[i])
                        {
                            logger_base.warn("Falcon Outputs Upload: Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i);
                            wxMessageBox(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i));
                            success = false;
                        }
                        else
                        {
                            portdone[i] = true;
                            long startChannel;
                            Output* output = outputManager->GetOutput(portstart, startChannel);
                            if (output == nullptr || FindPort(stringData, i - 1) == nullptr)
                            {
                                logger_base.warn("Falcon Outputs Upload: Attempt to find output for channel %ld failed. Do you have enough outputs?", portstart);
                                wxMessageBox(wxString::Format("Attempt to find output for channel %ld failed. Do you have enough outputs?", portstart));
                                success = false;
                            }
                            else
                            {
                                FindPort(stringData, i - 1)->protocol = DecodeStringPortProtocol(*protocol);
                                FindPort(stringData, i - 1)->universe = output->GetUniverse();
                                FindPort(stringData, i - 1)->startChannel = startChannel;
                                FindPort(stringData, i - 1)->pixels = (portend - portstart + 1) / 3;
                                FindPort(stringData, i - 1)->description = SafeDescription(first->GetName());
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
    }

    progress.Update(60, "Uploading string ports.");
    logger_base.info("Working out required pixel splits.");

    int maxMain = 0;
    int maxDaughter1 = 0;
    int maxDaughter2 = 0;

    for (auto i = 0; i < stringData.size(); ++i)
    {
        int pixels = stringData[i]->pixels + GetVirtualStringPixels(virtualStringData, stringData[i]->port);
        if (i < GetDaughter1Threshold())
        {
            if (pixels > maxMain) maxMain = pixels;
        }
        else if (i < GetDaughter2Threshold())
        {
            if (pixels > maxDaughter1) maxDaughter1 = pixels;
        }
        else
        {
            if (pixels > maxDaughter2) maxDaughter2 = pixels;
        }
    }

    if (!SupportsVariableExpansions())
    {
        // minimum of main is 1
        if (maxMain == 0)
        {
            maxMain = 1;
        }

        if (maxDaughter1 > 0)
        {
            if (maxMain > maxPixels / 2 || maxDaughter1 > maxPixels / 2)
            {
                logger_base.warn("Falcon Outputs Upload: %s V2 Controller only supports 340/340 pixel split with expansion board. (%d/%d)",
                    (const char *)_ip.c_str(), maxMain, maxDaughter1);
                wxMessageBox(wxString::Format("Falcon Outputs Upload: %s V2 Controller only supports 340/340 pixel split with expansion board. (%d/%d)",
                    (const char *)_ip.c_str(), maxMain, maxDaughter1));
                success = false;
            }

            maxMain = maxPixels / 2;
            maxDaughter1 = maxPixels / 2;

            if (maxDaughter2 > 0)
            {
                logger_base.warn("Falcon Outputs Upload: %s V2 Controller only supports one expansion board.",
                    (const char *)_ip.c_str());
                wxMessageBox(wxString::Format("Falcon Outputs Upload: %s V2 Controller only supports one expansion board.",
                    (const char *)_ip.c_str()));
                success = false;
                maxDaughter2 = 0;
            }
        }

        logger_base.info("Falcon pixel fixed split: Main = %d, Expansion1 = %d", maxMain, maxDaughter1);
    }
    else
    {
        if (maxMain == 0)
        {
            maxMain = 1;
        }

        if (stringData.size() > GetDaughter2Threshold() && maxDaughter2 == 0)
        {
            maxDaughter2 = 1;
        }

        if (stringData.size() > GetDaughter1Threshold() && maxDaughter1 == 0)
        {
            maxDaughter1 = 1;
        }

        logger_base.info("Falcon pixel required split: Main = %d, Expansion1 = %d, Expansion2 = %d", maxMain, maxDaughter1, maxDaughter2);

        if (maxMain + maxDaughter1 + maxDaughter2 > maxPixels)
        {
            maxMain = maxPixels - maxDaughter1 - maxDaughter2;
            logger_base.warn("     Total was more than %d ... main adjusted to %d.", maxPixels, maxMain);
        }

        if (maxMain + maxDaughter1 + maxDaughter2 < maxPixels)
        {
            if (maxDaughter2 != 0)
            {
                maxDaughter2 = maxPixels - maxMain - maxDaughter1;
            }
            else if (maxDaughter1 != 0)
            {
                maxDaughter1 = maxPixels - maxMain;
            }
            else
            {
                maxMain = maxPixels;
            }
            logger_base.info("Falcon pixel split adjusted to add up to %d: Main = %d, Expansion1 = %d, Expansion2 = %d", maxPixels, maxMain, maxDaughter1, maxDaughter2);
        }
    }

    logger_base.info("Uploading string ports.");
    UploadStringPorts(stringData, maxMain, maxDaughter1, maxDaughter2, virtualStringData);

    // delete all our string data
    while (stringData.size() > 0)
    {
        delete stringData[stringData.size() - 1];
        stringData.pop_back();
    }
    while (virtualStringData.size() > 0)
    {
        delete virtualStringData[virtualStringData.size() - 1];
        virtualStringData.pop_back();
    }

    progress.Update(100, "Done.");
    logger_base.info("Falcon upload done.");

    return success;
}

int Falcon::ReadStringData(const wxXmlDocument& stringsDoc, std::vector<FalconString*>& stringData, std::vector<FalconString*>& virtualStringData)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int virtualStrings = 0;

    if (stringsDoc.GetRoot() == nullptr) return 0;

    int count = wxAtoi(stringsDoc.GetRoot()->GetAttribute("c"));

    logger_base.debug("Strings.xml had %d entries.", count);
    if (count == 0) return 0;

    stringData.resize(count);

    int i = 0;
    int lastString = -1;
    for (auto e = stringsDoc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext())
    {
        int index = wxAtoi(e->GetAttribute("p"));

        if (index == lastString + 1)
        {
            FalconString* string = new FalconString();
            string->startChannel = wxAtoi(e->GetAttribute("us")) + 1;
            if (string->startChannel < 1 || string->startChannel > 512) string->startChannel = 1;
            string->pixels = wxAtoi(e->GetAttribute("c"));
            if (string->pixels < 0 || string->pixels > GetMaxPixels()) string->pixels = 0;
            string->protocol = wxAtoi(e->GetAttribute("t"));
            string->universe = wxAtoi(e->GetAttribute("u"));
            if (string->universe <= 1 || string->universe > 64000) string->universe = 1;
            string->description = e->GetAttribute("y").ToStdString();
            string->port = wxAtoi(e->GetAttribute("p"));
            string->index = i;
            stringData[index] = string;
        }
        else
        {
            FalconString* string = new FalconString();
            string->startChannel = wxAtoi(e->GetAttribute("us")) + 1;
            if (string->startChannel < 1 || string->startChannel > 512) string->startChannel = 1;
            string->pixels = wxAtoi(e->GetAttribute("c"));
            string->protocol = -1;
            string->universe = wxAtoi(e->GetAttribute("u"));
            if (string->universe <= 1 || string->universe > 64000) string->universe = 1;
            string->description = e->GetAttribute("y").ToStdString();
            string->port = wxAtoi(e->GetAttribute("p"));
            string->index = i;
            virtualStringData.push_back(string);

            logger_base.warn("     Virtual string exists on port %d.", index + 1);
            virtualStrings++;
        }
        lastString = index;
        i++;
    }

    if (virtualStrings > 0)
    {
        stringData.resize(count - virtualStrings);
    }

    return virtualStrings;
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

#define MINIMUMPIXELS 50

void Falcon::InitialiseStrings(std::vector<FalconString*>& stringsData, int max, int virtualStrings)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Expanding strings from %d to %d.", stringsData.size(), max);

    int oldsize = stringsData.size();

    stringsData.resize(max);

    for (int i = oldsize; i < max; ++i)
    {
        FalconString* string = new FalconString();
        string->startChannel = 1;
        string->pixels = MINIMUMPIXELS;
        string->protocol = 0;
        string->universe = 1;
        string->description = "";
        string->port = i;
        string->index = i + virtualStrings;
        stringsData[i] = string;
    }
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

FalconString* Falcon::FindPort(const std::vector<FalconString*>& stringData, int port) const 
{
    for (int i = 0; i < stringData.size(); ++i)
    {
        if (stringData[i]->port == port)
        {
            return stringData[i];
        }
    }

    wxASSERT(false);
    return nullptr;
}

void Falcon::UploadStringPorts(const std::vector<FalconString*>& stringData, int maxMain, int maxDaughter1, int maxDaughter2, const std::vector<FalconString*>& virtualStringData)
{
    int S = stringData.size() + virtualStringData.size();
    int m = 0;

    if (stringData.size() > GetDaughter2Threshold())
    {
        m = 2;
    }
    else if (stringData.size() > GetDaughter1Threshold())
    {
        m = 1;
    }

    std::string base = wxString::Format("m=%i&S=%i", m, S).ToStdString();

    if (SupportsVariableExpansions())
    {
        base += wxString::Format("&k0=%i&k1=%i&k2=%i", maxMain, maxDaughter1, maxDaughter2).ToStdString();
    }

    bool hasGreaterThan40 = false;

    std::string message = base + "&q=0";
    for (int i = 0; i < stringData.size(); ++i)
    {
        if (stringData[i]->port < 40)
        {
            message += BuildStringPort(stringData[i]);
        }
        else
        {
            hasGreaterThan40 = true;
        }
    }
 
    for (int i = 0; i < virtualStringData.size(); ++i)
    {
        if (virtualStringData[i]->port < 40)
        {
            message += BuildStringPort(virtualStringData[i]);
        }
    }

    UploadStringPort(message, stringData.size() <= 40);

    if (hasGreaterThan40)
    {
        message = base + "&q=1";
        for (int i = 0; i < stringData.size(); ++i)
        {
            if (stringData[i]->port >= 40)
            {
                message += BuildStringPort(stringData[i]);
            }
        }

        for (int i = 0; i < virtualStringData.size(); ++i)
        {
            if (virtualStringData[i]->port >= 40)
            {
                message += BuildStringPort(virtualStringData[i]);
            }
        }

        UploadStringPort(message, true);
    }
}

int Falcon::MaxPixels(const wxXmlDocument& stringsDoc, int board) const
{
    if (stringsDoc.GetRoot() == nullptr)
    {
        return 0;
    }

    switch(board)
    {
    case 0:
        return wxAtoi(stringsDoc.GetRoot()->GetAttribute("k0"));
    case 1:
        return wxAtoi(stringsDoc.GetRoot()->GetAttribute("k1"));
    case 2:
        return wxAtoi(stringsDoc.GetRoot()->GetAttribute("k2"));
    default:
        return 0;
    }
}

int Falcon::CountStrings(const wxXmlDocument& stringsDoc) const
{
    if (stringsDoc.GetRoot() == nullptr)
    {
        return 0;
    }

    int count = 0;
    int last = -1;
    
    for (auto e = stringsDoc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext())
    {
        int index = wxAtoi(e->GetAttribute("p"));
        if (index == last + 1)
        {
            count++;
        }
        last = index;
    }

    return count;
}

std::string Falcon::BuildStringPort(FalconString* string) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    return wxString::Format("&p%i=%i&t%i=%i&u%i=%i&s%i=%i&c%i=%i&y%i=%s", 
        string->index, string->port, 
        string->index, string->protocol,
        string->index, string->universe,
        string->index, string->startChannel,
        string->index, string->pixels,
        string->index, string->description).ToStdString();
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
    if (output > GetMaxSerialOutputs())
    {
        logger_base.warn("Falcon Outputs Upload: Falcon %s only supports %d serial outputs. Attempt to upload to serail output %d.", (const char *)_modelString.c_str(), GetMaxStringOutputs(), output);
        wxMessageBox("Falcon " + wxString(_modelString.c_str()) + " only supports " + wxString::Format("%d", GetMaxSerialOutputs()) + " outputs. Attempt to upload to output " + wxString::Format("%d", output) + ".", "Invalid Serial Output", wxOK, parent);
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
