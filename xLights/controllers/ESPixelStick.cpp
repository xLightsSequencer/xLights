#include "ESPixelStick.h"
#include <wx/msgdlg.h>
#include <wx/regex.h>
#include "models/Model.h"
#include <log4cpp/Category.hh>
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "models/ModelManager.h"

// This is tested with a pixel stick running v3.0 of the firmware

ESPixelStick::ESPixelStick(const std::string& ip)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _ip = ip;
    _connected = false;

    _wsClient.Connect(_ip, "/ws");

    if (_wsClient.IsConnected())
    {
        _connected = true;
        _wsClient.Send("G2");
        wxMilliSleep(500);
        std::string g2 = _wsClient.Receive();
        _version = GetFromJSON("", "version", g2);
        logger_base.debug("Connected to ESPixelStick - Firmware Version %s", (const char *)_version.c_str());
    }
    else
    {
        _connected = false;
        logger_base.error("Error connecting to ESPixelStick controller on %s.", (const char *)_ip.c_str());
    }
}

int ESPixelStick::GetMaxStringOutputs() const
{
    return 1;
}

int ESPixelStick::GetMaxSerialOutputs() const
{
    return 0;
}

std::string ESPixelStick::GetFromJSON(std::string section, std::string key, std::string json)
{
    if (section == "")
    {
        wxString rkey = wxString::Format("(%s\\\":[\\\"]*)([^\\\",\\}]*)(\\\"|,|\\})", key);
        wxRegEx regexKey(rkey);
        if (regexKey.Matches(wxString(json)))
        {
            return regexKey.GetMatch(wxString(json), 2);
        }
    }
    else
    {
        wxString rsection = wxString::Format("(%s\\\":\\{)([^\\}]*)\\}", section);
        wxRegEx regexSection(rsection);
        if (regexSection.Matches(wxString(json)))
        {
            wxString sec = regexSection.GetMatch(wxString(json), 2);

            wxString rkey = wxString::Format("(%s\\\":[\\\"]*)([^\\\",\\}]*)(\\\"|,|\\})", key);
            wxRegEx regexKey(rkey);
            if (regexKey.Matches(wxString(sec)))
            {
                return regexKey.GetMatch(wxString(sec), 2);
            }
        }
    }
    return "";
}

ESPixelStick::~ESPixelStick()
{
}

bool ESPixelStickcompare_startchannel(const Model* first, const Model* second)
{
    int firstmodelstart = first->GetNumberFromChannelString(first->ModelStartChannel);
    int secondmodelstart = second->GetNumberFromChannelString(second->ModelStartChannel);

    return firstmodelstart < secondmodelstart;
}

bool ESPixelStick::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent)
{
    bool success = true;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("ESPixelStick Outputs Upload: Uploading to %s", (const char *)_ip.c_str());
    // build a list of models on this controller
    std::list<Model*> models;
    std::list<std::string> protocolsused;
    std::list<Model*> warnedmodels;
    int maxport = 0;

    // Get universes based on IP
    std::list<Output*> outputs = outputManager->GetAllOutputs(_ip, selected);

    for (auto ito = outputs.begin(); ito != outputs.end(); ++ito)
    {
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
                            logger_base.warn("ESPixelStick Outputs Upload: Model %s on controller %s does not have its Controller Connection details completed: '%s'. Model ignored.", (const char *)it->first.c_str(), (const char *)_ip.c_str(), (const char *)it->second->GetControllerConnection().c_str());
                            wxMessageBox("Model " + it->first + " on controller " + _ip + " does not have its Contoller Connection details completed: '" + it->second->GetControllerConnection() + "'. Model ignored.", "Model Ignored");
                        }
                    }
                    else
                    {
                        // model uses channels in this universe

                        // check we dont already have this model in our list
                        if (std::find(models.begin(), models.end(), it->second) == models.end())
                        {
                            logger_base.debug("ESPixelStick Outputs Upload: Uploading Model %s.", (const char *)it->first.c_str());
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

    if (protocolsused.size() != 1)
    {
        logger_base.warn("ESPixelStick Outputs Upload: Controller %s only one protocol supported %d.",
            (const char *)_ip.c_str(), (int)protocolsused.size());
        wxMessageBox("Controller " + _ip + " supports only one protocol. "+wxString::Format("%d", (int)protocolsused.size()) +" found.", "Error");
        success = false;
    }

    // sort the models by start channel
    models.sort(ESPixelStickcompare_startchannel);

    // for each protocol
    for (auto protocol = protocolsused.begin(); protocol != protocolsused.end(); ++protocol)
    {
        bool portdone = false;

        // for each port ... this is the max of any port type but it should be ok
        for (int i = 1; i <= maxport; i++)
        {
            if (i > 1)
            {
                logger_base.warn("ESPixelStick Outputs Upload: Controller %s only supports one port %d.",
                    (const char *)_ip.c_str(), i);
            }
            else
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
                    int numstrings = first->GetNumPhysicalStrings();

                    if (numstrings > 1)
                    {
                        logger_base.warn("ESPixelStick Outputs Upload: Controller %s only supports one string per model %s %d.",
                            (const char *)_ip.c_str(), (const char *)first->GetName().c_str(), numstrings);
                        success = false;
                    }

                    int channelsperstring = first->NodesPerString() * first->GetChanCountPerNode();
    
                    // upload it
                    if (DecodeStringPortProtocol(*protocol) != "null")
                    {
                        if (portdone)
                        {
                            logger_base.warn("ESPixelStick Outputs Upload: Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i);
                            wxMessageBox(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i));
                            success = false;
                        }
                        else
                        {
                            portdone = true;
                            long sc = 1;
                            Output* o = outputManager->GetOutput(first->GetFirstChannel() + 1, sc);

                            int channels = last->GetLastChannel() - first->GetFirstChannel() + 1;

                            _wsClient.Send("G1");
                            wxMilliSleep(500);
                            std::string config = _wsClient.Receive();

                            std::string id = GetFromJSON("device", "id", config);
                            std::string mqttenabled = GetFromJSON("mqtt", "enabled", config);
                            std::string mqttip = GetFromJSON("mqtt", "ip", config);
                            std::string mqttport = GetFromJSON("mqtt", "port", config);
                            std::string mqttuser = GetFromJSON("mqtt", "user", config);
                            std::string mqttpassword = GetFromJSON("mqtt", "password", config);
                            std::string mqtttopic = GetFromJSON("mqtt", "topic", config);
                            std::string color = GetFromJSON("pixel", "color", config);
                            std::string gamma = GetFromJSON("pixel", "gamma", config);

                            wxString message = "S2{\"device\":{\"id\":\"" + id + "\"}," +
                                "\"mqtt\":{\"enabled\":" + mqttenabled +
                                ",\"ip\":\"" + mqttip + "\"" +
                                ",\"port\":\"" + mqttport + "\"" +
                                ",\"user\":\"" + mqttuser + "\"" +
                                ",\"password\":\"" + mqttpassword + "\"" +
                                ",\"topic\":\"" + mqtttopic + "\"}," +
                                "\"e131\":{\"universe\":" + wxString::Format("%d", o->GetUniverse()) +
                                ",\"universe_limit\":" + wxString::Format("%ld", o->GetChannels()) +
                                ",\"channel_start\":" + wxString::Format("%ld", sc) +
                                ",\"channel_count\":" + wxString::Format("%ld", channels) +
                                ",\"multicast\":" + ((o->GetIP() == "MULTICAST") ? "true" : "false") + "}," +
                                "\"pixel\":{\"type\":" + DecodeStringPortProtocol(*protocol) +
                                ",\"color\":" + color +
                                ",\"gamma\":" + gamma + "}," +
                                "\"serial\":{\"type\":" + DecodeSerialPortProtocol(*protocol) +
                                ",\"baudrate\":" + DecodeSerialSpeed(*protocol) + "}}";

                            if (_wsClient.Send(message))
                            {
                                success = true;
                                logger_base.warn("ESPixelStick Outputs Upload: Success!!!");
                            }
                            wxMilliSleep(500);
                            _wsClient.Receive();
                        }
                    }
                }
                else
                {
                    logger_base.warn("ESPixelStick Outputs Upload: Controller %s protocol %s not supported by this controller.",
                        (const char *)_ip.c_str(), (const char *)protocol->c_str());
                    wxMessageBox("Controller " + _ip + " protocol " + (*protocol) + " not supported by this controller.", "Protocol Ignored");
                    success = false;
                }
            }
        }
    }

    return success;
}

std::string ESPixelStick::DecodeStringPortProtocol(std::string protocol)
{
    if (protocol == "ws2811") return "0";
    if (protocol == "gece") return "1";

    return "null";
}

std::string ESPixelStick::DecodeSerialPortProtocol(std::string protocol)
{
    // This is not right as I dont actually have a board that supports this

    if (protocol == "dmx") return "null";
    if (protocol == "renard") return "null";

    return "null";
}

std::string ESPixelStick::DecodeSerialSpeed(std::string protocol)
{
    // This is not right as I dont actually have a board that supports this

    if (protocol == "dmx") return "null";
    if (protocol == "renard") return "null";
    return "null";
}

