#include "Pixlite16.h"
#include <wx/msgdlg.h>
#include <wx/regex.h>
#include "models/Model.h"
#include <log4cpp/Category.hh>
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "UtilFunctions.h"
#include "ControllerUploadData.h"

class PixLite16ControllerRules : public ControllerRules
{
public:
    PixLite16ControllerRules() : ControllerRules() {}
    virtual ~PixLite16ControllerRules() {}
    virtual int GetMaxPixelPortChannels() const override
    {
        return 340 * 3;
    }
    virtual int GetMaxPixelPort() const override { return 16; }
    virtual int GetMaxSerialPortChannels() const override { return 512; }
    virtual int GetMaxSerialPort() const override { return 4; }
    virtual bool IsValidPixelProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        return (p == "ws2811" || 
            p == "tm1803" ||
            p == "tm1804" ||
            p == "tm1809" ||
            p == "tls3001" || 
            p == "lpd6803" ||
            p == "sm16716" ||
            p == "ws2801" ||
            p == "mb16020" ||
            p == "my9231" ||
            p == "apa102" ||
            p == "my9221" ||
            p == "sk6812" ||
            p == "ucs1903");
    }
    virtual bool IsValidSerialProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        return (p == "dmx");
    }
    virtual bool SupportsMultipleProtocols() const override { return false; }
    virtual bool SupportsMultipleInputProtocols() const override { return false; }
    virtual bool AllUniversesSameSize() const override { return false; }
    virtual std::list<std::string> GetSupportedInputProtocols() const override {
        std::list<std::string> res;
        res.push_back("E131");
        res.push_back("ARTNET");
        return res;
    };
    virtual bool UniversesMustBeSequential() const override { return true; }
};

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
    } else if (!discovery->IsOk())
    {
        logger_base.error("Error initialising Pixlite discovery datagram ... is network connected? OK : FALSE");
        delete discovery;
        return;
    } 
    else if (discovery->Error())
    {
        logger_base.error("Error creating socket to broadcast from => %d : %s.", discovery->LastError(), (const char *)DecodeIPError(discovery->LastError()).c_str());
        delete discovery;
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
        logger_base.debug("Pixlite error broadcasting to %s => %d : %s.", (const char *)broadcast.c_str(), discovery->LastError(), (const char *)DecodeIPError(discovery->LastError()).c_str());
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
            logger_base.error("Error reading broadcast response => %d : %s.", discovery->LastError(), (const char *)DecodeIPError(discovery->LastError()).c_str());
        }
    }

    if (!_connected)
    {
        logger_base.error("Error connecting to Pixlite controller on %s.", (const char *)_ip.c_str());
    }

    discovery->Close();
    delete discovery;
}

Pixlite16::~Pixlite16()
{
}

bool Pixlite16::SendConfig(bool logresult) const
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
    } else if (!config->IsOk())
    {
        logger_base.error("Error initialising Pixlite config datagram ... is network connected? OK : FALSE");
        delete config;
        return false;
    }
    else if (config->Error())
    {
        logger_base.error("Error creating Pixlite config datagram => %d : %s.", config->LastError(), (const char *)DecodeIPError(config->LastError()).c_str());
        delete config;
        return false;
    }

    logger_base.debug("Pixlite sending config to %s.", (const char *)_ip.c_str());
    wxIPV4address toAddr;
    toAddr.Hostname(_ip);
    toAddr.Service(49150);

    config->SendTo(toAddr, _pixliteData, sizeof(_pixliteData));

    config->Close();
    delete config;

    return true;
}

bool pixlite_compare_startchannel(const Model* first, const Model* second)
{
    int firstmodelstart = first->GetNumberFromChannelString(first->ModelStartChannel);
    int secondmodelstart = second->GetNumberFromChannelString(second->ModelStartChannel);

    return firstmodelstart < secondmodelstart;
}

bool Pixlite16::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent)
{
    //ResetStringOutputs(); // this shouldnt be used normally

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Pixlite Outputs Upload: Uploading to %s", (const char *)_ip.c_str());
    
    std::string check;
    UDController cud(_ip, allmodels, outputManager, &selected, check);

    PixLite16ControllerRules rules;
    bool success = cud.Check(&rules, check);

    cud.Dump();

    logger_base.debug(check);

    // Get universes based on IP
    std::list<Output*> outputs = outputManager->GetAllOutputs(_ip, selected);

    if (outputs.size() > 0)
    {
        if (outputs.front()->GetType() == "ArtNET")
        {
            _pixliteData[105] = 0x01;
        }
        else
        {
            _pixliteData[105] = 0x00;
        }
    }

    if (success && cud.GetMaxPixelPort() > 0)
    {
        // turn on advanced mode
        _pixliteData[118] = 0x00;
        _pixliteData[119] = 0x00;
        _pixliteData[120] = 0x00;
        _pixliteData[121] = 0x00;

        for (int pp = 1; pp <= rules.GetMaxPixelPort(); pp++)
        {
            if (cud.HasPixelPort(pp))
            {
                UDControllerPort* port = cud.GetControllerPixelPort(pp);

                // update the data
                _pixliteData[122] = DecodeStringPortProtocol(port->GetProtocol());

                // universe
                int universe = port->GetUniverse();
                _pixliteData[0xa6 - 0x2a + pp * 2] = (universe & 0xFF00) >> 8;
                _pixliteData[0xa6 - 0x2a + pp * 2 + 1] = (universe & 0x00FF);

                // start channel
                int sc = port->GetUniverseStartChannel();
                _pixliteData[0xc6 - 0x2a + pp * 2] = ((sc) & 0xFF00) >> 8;
                _pixliteData[0xc6 - 0x2a + pp * 2 + 1] = ((sc) & 0x00FF);

                // bulbs
                int bulbs = port->Pixels();
                _pixliteData[0x71 - 0x2a + pp * 2] = (bulbs & 0xFF00) >> 8;
                _pixliteData[0x71 - 0x2a + pp * 2 + 1] = (bulbs & 0x00FF);

                // null pixels - e8
                int nullPixels = port->GetFirstModel()->GetNullPixels(0);
                _pixliteData[0xe7 - 0x2a + pp] = nullPixels;

                // Group
                int group = port->GetFirstModel()->GetGroupCount(1);
                _pixliteData[0x146 - 0x2a + pp * 2] = (group & 0xFF00) >> 8;
                _pixliteData[0x146 - 0x2a + pp * 2 + 1] = (group & 0xFF);

                // Brightness
                int brightness = port->GetFirstModel()->GetBrightness(100);
                _pixliteData[0x167 - 0x2a + pp] = brightness;

                // Reversed
                if (port->GetFirstModel()->GetDirection("Forward") == "Reverse")
                {
                    _pixliteData[0x117 - 0x2a + pp] = 1;
                }
                else
                {
                    _pixliteData[0x117 - 0x2a + pp] = 0;
                }

                port->CreateVirtualStrings();
                if (port->GetVirtualStringCount() > 1)
                {
                    check += wxString::Format("WARN: String port %d has model settings that can't be uploaded.\n", pp);
                }
            }
        }
    }

    if (success)
    {
        if (cud.GetMaxSerialPort() > 0)
        {
            for (int sp = 1; sp <= rules.GetMaxSerialPort(); sp++)
            {
                if (cud.HasSerialPort(sp))
                {
                    UDControllerPort* port = cud.GetControllerSerialPort(sp);

                    int universe = port->GetUniverse();
                    _pixliteData[259 + sp * 2] = (universe & 0xFF00) >> 8;
                    _pixliteData[259 + sp * 2 + 1] = (universe & 0x00FF);
                    _pixliteData[256 + sp] = 0x01; // turn it on

                    port->CreateVirtualStrings();
                    if (port->GetVirtualStringCount() > 1)
                    {
                        check += wxString::Format("WARN: Serial port %d has model settings that can't be uploaded.\n", sp);
                    }
                }
            }
        }
    }

    if (success)
    {
        if (check != "")
        {
            DisplayWarning("Upload warnings:\n" + check);
        }

        return SendConfig(false);
    }

    DisplayError("Not uploaded due to errors.\n" + check);

    return false;
}

int Pixlite16::DecodeStringPortProtocol(std::string protocol)
{
    wxString p(protocol);
    p = p.Lower();

    if (p == "tls3001") return 0;
    if (p == "sm16716") return 1;
    if (p == "ws2801") return 2;
    if (p == "lpd6803") return 3;
    if (p == "ws2811") return 4;
    if (p == "mb16020") return 5;
    if (p == "tm1803") return 6;
    if (p == "tm1804") return 7;
    if (p == "tm1809") return 8;
    if (p == "my9231") return 9;
    if (p == "apa102") return 10;
    if (p == "my9221") return 11;
    if (p == "sk6812") return 12;
    if (p == "ucs1903") return 13;

    return -1;
}

int Pixlite16::DecodeSerialOutputProtocol(std::string protocol)
{
    wxString p(protocol);
    p = p.Lower();
    if (p == "dmx") return 0;
    return -1;
}