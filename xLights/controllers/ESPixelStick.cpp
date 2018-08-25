#include "ESPixelStick.h"
#include <wx/msgdlg.h>
#include <wx/regex.h>
#include "models/Model.h"
#include <log4cpp/Category.hh>
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "models/ModelManager.h"
#include "ControllerUploadData.h"

// This is tested with a pixel stick running v3.0 of the firmware

class ESPixelStickControllerRules : public ControllerRules
{
public:
    ESPixelStickControllerRules() : ControllerRules() {}
    virtual ~ESPixelStickControllerRules() {}
    virtual int GetMaxPixelPortChannels() const override { return 1360 * 3; }
    virtual int GetMaxPixelPort() const override { return 1; }
    virtual int GetMaxSerialPortChannels() const override { return 0; } // not implemented yet
    virtual int GetMaxSerialPort() const override { return 0; } // not implemented yet
    virtual bool IsValidPixelProtocol(const std::string protocol) const override
    {
        return (protocol == "ws2811" || protocol == "gece");
    }
    virtual bool IsValidSerialProtocol(const std::string protocol) const override
    {
        return (protocol == "renard" || protocol == "dmx");
    }
    virtual bool SupportsMultipleProtocols() const override { return false; }
    virtual bool AllUniversesSameSize() const override { return true; }
    virtual std::list<std::string> GetSupportedInputProtocols() const { 
        std::list<std::string> res;
        res.push_back("E131");
        return res;
    };
    virtual bool UniversesMustBeSequential() const { return true; }
};

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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("ESPixelStick Outputs Upload: Uploading to %s", (const char *)_ip.c_str());

    std::string check;
    UDController cud(_ip, allmodels, outputManager, &selected, check);

    ESPixelStickControllerRules rules;
    bool success = cud.Check(&rules, check);

    cud.Dump();

    logger_base.debug(check);

    if (success && cud.GetMaxPixelPort() > 0)
    {
        if (check != "")
        {
            wxMessageBox("Upload warnings:\n" + check);
        }

        UDControllerPort* port = cud.GetControllerPixelPort(1);

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
            "\"e131\":{\"universe\":" + wxString::Format("%d", port->GetUniverse()) +
            ",\"universe_limit\":" + wxString::Format("%ld", cud.GetFirstOutput()->GetChannels()) +
            ",\"channel_start\":" + wxString::Format("%ld", port->GetStartChannel()) +
            ",\"channel_count\":" + wxString::Format("%ld", port->Channels()) +
            ",\"multicast\":" + ((cud.GetFirstOutput()->GetIP() == "MULTICAST") ? "true" : "false") + "}," +
            "\"pixel\":{\"type\":" + DecodeStringPortProtocol(port->GetProtocol()) +
            ",\"color\":" + color +
            ",\"gamma\":" + gamma + "}," +
            "\"serial\":{\"type\":" + DecodeSerialPortProtocol(port->GetProtocol()) +
            ",\"baudrate\":" + DecodeSerialSpeed(port->GetProtocol()) + "}}";

        if (_wsClient.Send(message))
        {
            success = true;
            logger_base.debug("ESPixelStick Outputs Upload: Success!!!");
        }
        wxMilliSleep(500);
        _wsClient.Receive();
    }
    else
    {
        wxMessageBox("Not uploaded due to errors.\n" + check);
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

