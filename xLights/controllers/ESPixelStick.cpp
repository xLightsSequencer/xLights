#include "ESPixelStick.h"
#include <wx/msgdlg.h>
#include <wx/regex.h>

#include "models/Model.h"
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "models/ModelManager.h"
#include "UtilFunctions.h"
#include "ControllerRegistry.h"

#include <log4cpp/Category.hh>
#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"


// This is tested with a pixel stick running v3.0 of the firmware

static std::vector<ESPixelStickControllerRules> CONTROLLER_TYPE_MAP = {
    ESPixelStickControllerRules()
};

void ESPixelStick::RegisterControllers() {
    for (auto &a : CONTROLLER_TYPE_MAP) {
        ControllerRegistry::AddController(&a);
    }
}


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
    UDController cud(_ip, _ip, allmodels, outputManager, &selected, check);

    ESPixelStickControllerRules rules;
    bool success = cud.Check(&rules, check);

    cud.Dump();

    logger_base.debug(check);

    if (success && cud.GetMaxPixelPort() > 0) {
        if (check != "") {
            DisplayWarning("Upload warnings:\n" + check);
        }

        UDControllerPort* port = cud.GetControllerPixelPort(1);

        _wsClient.Send("G1");
        wxMilliSleep(500);
        std::string config = _wsClient.Receive();
        config = config.substr(2);

        wxJSONValue origJson;
        wxJSONReader reader;
        reader.Parse(config, &origJson);
        
        wxJSONValue newJson;
        //copy stuff over to act as defaults
        newJson["device"] = origJson["device"];
        newJson["mqtt"] = origJson["mqtt"];
        newJson["e131"] = origJson["e131"];
        newJson["pixel"] = origJson["pixel"];

        newJson["e131"]["universe"] = port->GetUniverse();
        newJson["e131"]["universe_limit"] = cud.GetFirstOutput()->GetChannels();
        newJson["e131"]["channel_start"] = port->GetStartChannel();
        newJson["e131"]["channel_count"] = port->Channels();
        newJson["e131"]["multicast"] = ((cud.GetFirstOutput()->GetIP() == "MULTICAST") ? true : false);

        auto s = port->GetModels().front();
        if (s) {
            int brightness = s->GetBrightness(-9999);
            std::string colourOrder = s->GetColourOrder("unknown");
            float gamma = s->GetGamma(-9999);
            int groupCount = s->GetGroupCount(-9999);

            if (gamma > 0) {
                newJson["pixel"]["gammaVal"] = gamma;
            }
            if (groupCount > 0) {
                newJson["pixel"]["groupSize"] = groupCount;
            }
            if (colourOrder != "unknown") {
                if (colourOrder == "GRB") {
                    newJson["pixel"]["colourOrder"] = 1;
                } else if (colourOrder == "BRG") {
                    newJson["pixel"]["colourOrder"] = 2;
                } else if (colourOrder == "RBG") {
                    newJson["pixel"]["colourOrder"] = 3;
                } else if (colourOrder == "GBR") {
                    newJson["pixel"]["colourOrder"] = 4;
                } else if (colourOrder == "BGR") {
                    newJson["pixel"]["colourOrder"] = 5;
                } else {
                    newJson["pixel"]["colourOrder"] = 0;
                }
            }
            if (brightness > 0) {
                float bval = brightness;
                bval /= 100.0f;
                newJson["pixel"]["briteVal"] = bval;
            }
        }

        wxJSONWriter writer(wxJSONWRITER_NONE, 0, 3);
        wxString message;
        writer.Write(newJson, message);
        message = "S2" + message;

        if (_wsClient.Send(message)) {
            success = true;
            logger_base.debug("ESPixelStick Outputs Upload: Success!!!");
        }
        wxMilliSleep(500);
        _wsClient.Receive();
    } else {
        DisplayError("Not uploaded due to errors.\n" + check);
    }

    return success;
}

std::string ESPixelStick::DecodeStringPortProtocol(std::string protocol)
{
    wxString p(protocol);
    p = p.Lower();
    if (p == "ws2811") return "0";
    if (p == "gece") return "1";
    
    return "null";
}

std::string ESPixelStick::DecodeSerialPortProtocol(std::string protocol)
{
    // This is not right as I dont actually have a board that supports this
    wxString p(protocol);
    p = p.Lower();

    if (p == "dmx") return "null";
    if (p == "renard") return "null";
    
    return "null";
}

std::string ESPixelStick::DecodeSerialSpeed(std::string protocol)
{
    // This is not right as I dont actually have a board that supports this
    wxString p(protocol);
    p = p.Lower();

    if (p == "dmx") return "null";
    if (p == "renard") return "null";
    return "null";
}

