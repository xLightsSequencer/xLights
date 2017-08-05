#include "Pixlite16.h"
#include <wx/msgdlg.h>
#include <wx/regex.h>
#include "models/Model.h"
#include <log4cpp/Category.hh>
#include "outputs/OutputManager.h"
#include "outputs/Output.h"

Pixlite16::Pixlite16(const std::string& ip)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    
    _ip = ip;
    _connected = false;

	// broadcast packet to find all of them
    wxIPV4address localAddr;
    localAddr.AnyAddress();
    localAddr.Service(49150);

    auto discovery = new wxDatagramSocket(localAddr, wxSOCKET_NOWAIT | wxSOCKET_BROADCAST);

    if (discovery == nullptr)
    {
        logger_base.error("Error initialising Pixlite discovery datagram.");
        return;
    }

    if (!discovery->IsOk())
    {
        logger_base.error("Error initialising Pixlite discovery datagram ... is network connected.");
        return;
    }

    if (discovery->Error())
    {
        logger_base.error("Error creating socket to broadcast from => %d.", discovery->LastError());
        return;
    }

    wxString broadcast = "255.255.255.255";
    logger_base.debug("Pixlite broadcasting to %s.", (const char *)broadcast.c_str());
    wxIPV4address broadcastAddr;
    broadcastAddr.Hostname(broadcast);
    broadcastAddr.Service(49150);

    wxByte discoveryData[12];
    discoveryData[0] = 0x41;
    discoveryData[1] = 0x64;
    discoveryData[2] = 0x76;
    discoveryData[3] = 0x61;
    discoveryData[4] = 0x74;
    discoveryData[5] = 0x65;
    discoveryData[6] = 0x63;
    discoveryData[7] = 0x68;
    discoveryData[8] = 0x00;
    discoveryData[9] = 0x00;
    discoveryData[10] = 0x01;
    discoveryData[11] = 0x05;
    discovery->SendTo(broadcastAddr, discoveryData, sizeof(discoveryData));

    if (discovery->Error())
    {
        logger_base.debug("Pixlite error broadcasting to %s => %d.", (const char *)broadcast.c_str(), discovery->LastError());
        return;
    }

    wxMilliSleep(500);

    // look through responses for one that matches my ip

    wxByte data[384];
    memset(data, 0x00, sizeof(data));

    wxIPV4address pixliteAddr;
    while (discovery->IsData())
    {
        discovery->RecvFrom(pixliteAddr, data, sizeof(data));

        if (!discovery->Error() && data[10] == 0x02 && data[11] == 0x04)
        {
            wxString rcvIP = wxString::Format("%i.%i.%i.%i", data[141], data[142], data[143], data[144]);

            if (_ip == rcvIP)
            {
                memcpy(_pixliteData, data, 10);
                _pixliteData[10] = 0x05;
                _pixliteData[11] = 0x02;
                memcpy(&_pixliteData[12], &data[12], 10);
                memset(&_pixliteData[22], 0x00, 3);
                memcpy(&_pixliteData[25], &data[25], 7);
                memcpy(&_pixliteData[32], &data[37], 41);
                memcpy(&_pixliteData[73], &data[106], 54);
                memcpy(&_pixliteData[127], &data[164], 142);
                memcpy(&_pixliteData[269], &data[307], 67);
                _pixliteData[336] = data[378]; // normal/compressed
                _pixliteData[337] = 0x00;
 
                _connected = true;
                logger_base.error("Success connecting to Pixlite controller on %s.", (const char *)_ip.c_str());
                break;
            }
        }
        else if (discovery->Error())
        {
            logger_base.error("Error reading broadcast response => %d.", discovery->LastError());
        }
    }

    if (!_connected)
    {
        logger_base.error("Error connecting to Pixlite controller on %s.", (const char *)_ip.c_str());
    }

    discovery->Close();
}

int Pixlite16::GetMaxStringOutputs() const
{
    // of course it could be 4 if it was a pixlite 4
    return 16;
}

int Pixlite16::GetMaxSerialOutputs() const
{
    // and 1 if this was a pixlite 4
    return 4;
}

Pixlite16::~Pixlite16()
{
}

bool Pixlite16::SendConfig(bool logresult)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // broadcast packet to find all of them
    wxIPV4address localAddr;
    localAddr.AnyAddress();
    localAddr.Service(49150);

    auto config = new wxDatagramSocket(localAddr, wxSOCKET_NOWAIT);

    if (config == nullptr)
    {
        logger_base.error("Error initialising Pixlite config datagram.");
        return false;
    }

    if (!config->IsOk())
    {
        logger_base.error("Error initialising Pixlite config datagram ... is network connected.");
        return false;
    }

    logger_base.debug("Pixlite sending config to %s.", (const char *)_ip.c_str());
    wxIPV4address toAddr;
    toAddr.Hostname(_ip);
    toAddr.Service(49150);

    config->SendTo(toAddr, _pixliteData, sizeof(_pixliteData));

    config->Close();

    return true;
}

bool pixlite_compare_startchannel(const Model* first, const Model* second)
{
    int firstmodelstart = first->GetNumberFromChannelString(first->ModelStartChannel);
    int secondmodelstart = second->GetNumberFromChannelString(second->ModelStartChannel);

    return firstmodelstart < secondmodelstart;
}

void Pixlite16::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent)
{
    //ResetStringOutputs(); // this shouldnt be used normally

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Pixlite Outputs Upload: Uploading to %s", (const char *)_ip.c_str());
    
    // build a list of models on this controller
    std::list<Model*> models;
    std::string protocol;
    std::list<Model*> warnedmodels;
    std::list<int> dmxOutputs;
    std::list<Model*> dmxModels;
    int maxport = 0;

    // Get universes based on IP
    std::list<Output*> outputs = outputManager->GetAllOutputs(_ip, selected);

    bool isArtNET = false;

    if (outputs.size() > 0)
    {
        isArtNET = outputs.front()->GetType() == "ArtNET";

        if (isArtNET)
        {
            _pixliteData[105] = 0x01;
        }
        else
        {
            _pixliteData[105] = 0x00;
        }
    }

    for (auto ito = outputs.begin(); ito != outputs.end(); ++ito)
    {
        // this universe is sent to the Pixlite
        if (((*ito)->GetType() == "ArtNET") != isArtNET)
        {
            logger_base.warn("Pixlite controllers require all output to be e1.31 or ArtNET ... you cant mix them.");
        }

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
                            logger_base.warn("Pixlite Outputs Upload: Model %s on controller %s does not have its Controller Connection details completed: '%s'. Model ignored.", (const char *)it->first.c_str(), (const char *)_ip.c_str(), (const char *)it->second->GetControllerConnection().c_str());
                            wxMessageBox("Model " + it->first + " on controller " + _ip + " does not have its Contoller Connection details completed: '" + it->second->GetControllerConnection() + "'. Model ignored.", "Model Ignored");
                        }
                    }
                    else
                    {
                        // model uses channels in this universe
                        std::string mp = wxString(it->second->GetProtocol()).Lower().ToStdString();

                        if (mp == "dmx")
                        {
                            // check we dont already have this model in our list
                            if (std::find(dmxModels.begin(), dmxModels.end(), it->second) == dmxModels.end())
                            {
                                logger_base.debug("Pixlite DMX Outputs Upload: Uploading Model %s.", (const char *)it->first.c_str());
                                dmxModels.push_back(it->second);

                                if (std::find(dmxOutputs.begin(), dmxOutputs.end(), it->second->GetPort()) == dmxOutputs.end())
                                {
                                    dmxOutputs.push_back(it->second->GetPort());
                                }
                            }
                        }
                        else
                        {
                            // check we dont already have this model in our list
                            if (std::find(models.begin(), models.end(), it->second) == models.end())
                            {
                                logger_base.debug("Pixlite Outputs Upload: Uploading Model %s.", (const char *)it->first.c_str());
                                models.push_back(it->second);

                                if (protocol == "")
                                {
                                    protocol = mp;

                                    _pixliteData[122] = DecodeStringPortProtocol(protocol);
                                }
                                else
                                {
                                    if (protocol != it->second->GetProtocol())
                                    {
                                        logger_base.warn("Pixlite only supports one bulb protocol.");
                                    }
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
    }

    if (maxport > 16) maxport = 16;

    // sort the models by start channel
    models.sort(pixlite_compare_startchannel);

    bool portdone[16];
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
            if ((*model)->GetPort() == i)
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

            // upload it
            if (DecodeStringPortProtocol(protocol) >= 0)
            {
                // turn on advanced mode
                _pixliteData[118] = 0x00;
                _pixliteData[119] = 0x00;
                _pixliteData[120] = 0x00;
                _pixliteData[121] = 0x00;

                if (first == last && numstrings > 1)
                {
                    for (int j = 0; j < numstrings; j++)
                    {
                        if (portdone[i+j])
                        {
                            logger_base.warn("Pixlite Outputs Upload: Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i +j);
                            wxMessageBox(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i + j));
                        }
                        else
                        {
                            long sc = 1;
                            Output* o = outputManager->GetOutput(portstart + j * (portend - portstart + 1) / numstrings, sc);

                            portdone[i + j] = true;

                            // universe
                            int universe = o->GetUniverse();
                            _pixliteData[124 + (i + j) * 2] = (universe & 0xFF00) >> 8;
                            _pixliteData[124 + (i + j) * 2 + 1] = (universe & 0x00FF);

                            // start channel
                            _pixliteData[156 + (i + j) * 2] = ((sc) & 0xFF00) >> 8;
                            _pixliteData[156 + (i + j) * 2 + 1] = ((sc) & 0x00FF);

                            // bulbs
                            int bulbs = (portend - portstart + 1) / numstrings / 3;
                            _pixliteData[71 + (i + j) * 2] = (bulbs & 0xFF00) >> 8;
                            _pixliteData[71 + (i + j) * 2 + 1] = (bulbs & 0x00FF);

                            logger_base.debug("Uploading to Pixlite %s output %d, universe %d, start channel %d, bulbs %d.", (const char *)_ip.c_str(), i + j, universe, sc, bulbs);
                        }
                    }
                }
                else
                {
                    if (portdone[i])
                    {
                        logger_base.warn("Pixlite Outputs Upload: Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str() , i);
                        wxMessageBox(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i));
                    }
                    else
                    {
                        portdone[i] = true;

                        long sc = 1;
                        Output* o = outputManager->GetOutput(portstart, sc);

                        // universe
                        int universe = o->GetUniverse();
                        _pixliteData[124 + i * 2] = (universe & 0xFF00) >> 8;
                        _pixliteData[124 + i * 2 + 1] = (universe & 0x00FF);

                        // start channel
                        _pixliteData[156 + i * 2] = ((sc) & 0xFF00) >> 8;
                        _pixliteData[156 + i * 2 + 1] = ((sc) & 0x00FF);

                        // bulbs
                        int bulbs = (portend - portstart + 1) / 3;
                        _pixliteData[71 + i * 2] = (bulbs & 0xFF00) >> 8;
                        _pixliteData[71 + i * 2 + 1] = (bulbs & 0x00FF);

                        logger_base.debug("Uploading to Pixlite %s output %d, universe %d, start channel %d, bulbs %d.", (const char *)_ip.c_str(), i, universe, sc, bulbs);
                    }
                }                    
            }
            else
            {
                logger_base.warn("Pixlite Outputs Upload: Controller %s protocol %s not supported by this controller.",
                    (const char *)_ip.c_str(), (const char *)protocol.c_str());
                wxMessageBox("Controller " + _ip + " protocol " + (protocol) + " not supported by this controller.", "Protocol Ignored");
            }
        }
        else
        {
            // nothing on this port ... ignore it
        }
    }

    for (int i = 0; i < GetMaxSerialOutputs(); ++i)
    {
        _pixliteData[256 + i] = 0x00; // turn it off
    }

    for (auto it = dmxOutputs.begin(); it != dmxOutputs.end(); ++it)
    {
        if (*it > 0 && *it <= GetMaxSerialOutputs())
        {
            // find a model using this port
            Model* first = nullptr;
            long start = 0;

            for (auto model = dmxModels.begin(); model != dmxModels.end(); ++model)
            {
                if ((*model)->GetPort() == *it)
                {
                    first = *model;
                    start = (*model)->GetNumberFromChannelString((*model)->ModelStartChannel);
                    break;
                }
            }

            if (first != nullptr)
            {
                long sc = 1;
                Output* o = outputManager->GetOutput(start, sc);
                int universe = o->GetUniverse();
                _pixliteData[259 + *it * 2] = (universe & 0xFF00) >> 8;
                _pixliteData[259 + *it * 2 + 1] = (universe & 0x00FF);
                _pixliteData[256 + *it] = 0x01; // turn it on
                logger_base.debug("Uploading to Pixlite %s dmx output %d, universe %d.", (const char *)_ip.c_str(), *it, universe);
            }
        }
    }

    SendConfig(false);
}

int Pixlite16::DecodeStringPortProtocol(std::string protocol)
{
    if (protocol == "tls3001") return 0;
    if (protocol == "sm16716") return 1;
    if (protocol == "ws2801") return 2;
    if (protocol == "lpd6803") return 3;
    if (protocol == "ws2811") return 4;
    if (protocol == "mb16020") return 5;
    if (protocol == "tm1803") return 6;
    if (protocol == "tm1804") return 7;
    if (protocol == "tm1809") return 8;
    if (protocol == "my9231") return 9;
    if (protocol == "apa102") return 10;
    if (protocol == "my9221") return 11;
    if (protocol == "sk6812") return 12;
    if (protocol == "ucs1903") return 13;

    return -1;
}

int Pixlite16::DecodeSerialOutputProtocol(std::string protocol)
{
    if (protocol == "dmx") return 0;
    return -1;
}

