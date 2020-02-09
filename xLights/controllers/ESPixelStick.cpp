
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/msgdlg.h>
#include <wx/regex.h>

#include "ESPixelStick.h"
#include "models/Model.h"
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "models/ModelManager.h"
#include "UtilFunctions.h"
#include "ControllerCaps.h"
#include "../outputs/ControllerEthernet.h"

#include <log4cpp/Category.hh>

#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"

#pragma region Constructors and Destructors
// This is tested with a pixel stick running v3.0 of the firmware
ESPixelStick::ESPixelStick(const std::string& ip, const std::string& proxy) : BaseController(ip, proxy) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_fppProxy.empty()) {
        _wsClient.Connect(_fppProxy, "/proxy/" + _ip + "/ws");
    }
    else {
        _wsClient.Connect(_ip, "/ws");
    }

    if (_wsClient.IsConnected()) {
        _model = "ESPixelStick";
        _connected = true;
        _wsClient.Send("G2");
        wxMilliSleep(500);
        std::string g2 = _wsClient.Receive();
        _version = GetFromJSON("", "version", g2);
        logger_base.debug("Connected to ESPixelStick - Firmware Version %s", (const char *)_version.c_str());
    }
    else {
        _connected = false;
        logger_base.error("Error connecting to ESPixelStick controller on %s.", (const char *)_ip.c_str());
    }
}
#pragma endregion

#pragma region Private Functions
std::string ESPixelStick::DecodeStringPortProtocol(std::string protocol) {

    wxString p(protocol);
    p = p.Lower();
    if (p == "ws2811") return "0";
    if (p == "gece") return "1";
    return "null";
}

std::string ESPixelStick::DecodeSerialPortProtocol(std::string protocol) {

    // This is not right as I dont actually have a board that supports this
    wxString p(protocol);
    p = p.Lower();

    if (p == "dmx") return "null";
    if (p == "renard") return "null";
    return "null";
}

std::string ESPixelStick::DecodeSerialSpeed(std::string protocol) {

    // This is not right as I dont actually have a board that supports this
    wxString p(protocol);
    p = p.Lower();

    if (p == "dmx") return "null";
    if (p == "renard") return "null";
    return "null";
}

std::string ESPixelStick::GetFromJSON(std::string section, std::string key, std::string json) {

    if (section == "") {
        wxString rkey = wxString::Format("(%s\\\":[\\\"]*)([^\\\",\\}]*)(\\\"|,|\\})", key);
        wxRegEx regexKey(rkey);
        if (regexKey.Matches(wxString(json))) {
            return regexKey.GetMatch(wxString(json), 2);
        }
    }
    else {
        wxString rsection = wxString::Format("(%s\\\":\\{)([^\\}]*)\\}", section);
        wxRegEx regexSection(rsection);
        if (regexSection.Matches(wxString(json))) {
            wxString sec = regexSection.GetMatch(wxString(json), 2);

            wxString rkey = wxString::Format("(%s\\\":[\\\"]*)([^\\\",\\}]*)(\\\"|,|\\})", key);
            wxRegEx regexKey(rkey);
            if (regexKey.Matches(wxString(sec))) {
                return regexKey.GetMatch(wxString(sec), 2);
            }
        }
    }
    return "";
}
#pragma endregion

#pragma region Getters and Setters
bool ESPixelStick::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("ESPixelStick Outputs Upload: Uploading to %s", (const char *)_ip.c_str());

    std::string check;
    UDController cud(controller, outputManager, allmodels, check);

    auto rules = ControllerCaps::GetControllerConfig(controller);
    bool success = cud.Check(rules, check);

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
#pragma endregion
