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
#include "../models/Model.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../models/ModelManager.h"
#include "../UtilFunctions.h"
#include "ControllerCaps.h"
#include "../outputs/ControllerEthernet.h"

#include <log4cpp/Category.hh>

#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"

#pragma region Constructors and Destructors
// This is tested with a pixel stick running v3.0 of the firmware
ESPixelStick::ESPixelStick(const std::string& ip) : BaseController(ip, "") {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _wsClient.Connect(_ip, "/ws");

    if (_wsClient.IsConnected()) {
        _model = "ESPixelStick";
        _connected = true;
        _wsClient.Send("G2");
        _version = GetFromJSON("", "version", GetWSResponse());
        logger_base.debug("Connected to ESPixelStick - Firmware Version %s", (const char *)_version.c_str());
    }
    else {
        _connected = false;
        logger_base.error("Error connecting to ESPixelStick controller on %s.", (const char *)_ip.c_str());
    }
}
#pragma endregion

#pragma region Private Functions
std::string ESPixelStick::GetWSResponse() {
    wxLongLong start = wxGetLocalTimeMillis();
    wxLongLong diff = 0;
    std::string resp = "";
    while (diff < 500 && resp == "") {
        wxMilliSleep(5);
        resp = _wsClient.Receive();
        diff = wxGetLocalTimeMillis() - start;
    }
    return resp;
}


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
    //skip over the "G2" header or whatever
    for (int x = 0; x < json.size(); x++) {
        if (json[x] == '{' || json[x] == '[') {
            wxJSONValue origJson;
            wxJSONReader reader;
            wxString config = json.substr(x);
            reader.Parse(config, &origJson);
            wxJSONValue val = origJson;
            if (section != "") {
                val = origJson[section];
            }
            if(!val.HasMember(key))
            { 
                return "";
            }

            val = val.ItemAt(key);
            if (val.IsString()) {
                return val.AsString().ToStdString();
            }
            if (val.IsInt()) {
                return std::to_string(val.AsInt());
            }
            if (val.IsDouble()) {
                return std::to_string(val.AsDouble());
            }
        }
    }
    return "";
}
#pragma endregion

#pragma region Getters and Setters
bool ESPixelStick::SetInputUniverses(ControllerEthernet* controller, wxWindow* parent) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_version.size() > 0 && _version[0] == '4') {
        //only needed on V4.   V3 will upload inputs with outputs
        
        _wsClient.Send("{\"cmd\":{\"get\":\"input\"}}");
        std::string config = GetWSResponse();
        wxJSONValue origJson;
        wxJSONReader reader;
        reader.Parse(config, &origJson);
        bool changed = false;
        wxJSONValue inputConfig = origJson["get"]["input_config"]; //get the input_config element
        
        std::list<Output*> outputs = controller->GetOutputs();
        if (outputs.size() > 6) {
            DisplayError(wxString::Format("Attempt to upload %d universes to ESPixelStick controller but only 6 are supported.", outputs.size()).ToStdString());
            return false;
        }

        std::string type = "DDP";
        int startUniverse = 0;
        int chanPerUniverse = 512;
        if (outputs.front()->GetType() == OUTPUT_E131) {
            type = "E1.31";
            startUniverse = outputs.front()->GetUniverse();
            chanPerUniverse = outputs.front()->GetChannels();
        }
        std::string origTypeIdx = std::to_string(inputConfig["channels"]["0"]["type"].AsInt());
        std::string origType = inputConfig["channels"]["0"][origTypeIdx]["type"].AsString();
        if (origType != type) {
            changed = true;
            for (int x = 0; x < 20; x++) {
                std::string idx = std::to_string(x);
                if (!inputConfig["channels"]["0"].HasMember(idx)) {
                    return false;
                }
                if (inputConfig["channels"]["0"][idx]["type"].AsString() == type) {
                    //found the new element, flip over to using that protocol
                    inputConfig["channels"]["0"]["type"] = x;
                    origTypeIdx = idx;
                    break;
                }
            }
        }
        if (type == "E1.31") {
            std::string univString = std::to_string(startUniverse);
            std::string sizeString = std::to_string(chanPerUniverse);

            //{"type":"E1.31","universe":1,"universe_limit":512,"channel_start":1}
            if (inputConfig["channels"]["0"][origTypeIdx]["universe"].AsString() != univString) {
                inputConfig["channels"]["0"][origTypeIdx]["universe"] = univString;
                changed = true;
            }
            if (inputConfig["channels"]["0"][origTypeIdx]["universe_limit"].AsString() != sizeString) {
                inputConfig["channels"]["0"][origTypeIdx]["universe_limit"] = sizeString;
                changed = true;
            }
            //inputConfig["channels"]["0"][origTypeIdx]["channel_start"] = channel_start;
        } else if (type == "DDP") {
            //nothing to do for DDP
        }
        
        if (changed) {
            wxJSONWriter writer(wxJSONWRITER_NONE, 0, 3);
            wxString message;
            
            wxJSONValue newJson;
            newJson["cmd"]["set"]["input"]["input_config"] = inputConfig;
            
            writer.Write(newJson, message);
            message.Replace(" : ", ":");
            if (_wsClient.Send(message)) {
                logger_base.debug("ESPixelStick Inputs Upload: Success!!!");
            }
            GetWSResponse();
        }
    }
    return true;
}
bool ESPixelStick::UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) {
    SetInputUniverses(controller, parent);
    return SetOutputs(allmodels, outputManager, controller, parent);
}

bool ESPixelStick::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) {
    if (_version.size() > 0 && _version[0] == '4') {
        return SetOutputsV4(allmodels, outputManager, controller, parent);
    }
    return SetOutputsV3(allmodels, outputManager, controller, parent);
}

static std::string MapV4Type(const std::string &p) {
    if (p == "ws2811" || p == "WS2811") {
        return "WS2811";
    }
    if (p == "gece" || p == "GECE") {
        return "GECE";
    }
    if (p == "dmx" || p == "DMX") {
        return "DMX";
    }
    if (p == "renard" || p == "Renard") {
        return "Renard";
    }
    if (p == "tm1814" || p == "TM1814") {
        return "TM1814";
    }

    return "Disabled";
}
static std::string MapV4ColorOrder(const std::string &p) {
    if (p == "RGB") {
        return "rgb";
    }
    if (p == "RBG") {
        return "rbg";
    }
    if (p == "BGR") {
        return "bgr";
    }
    if (p == "BRG") {
        return "brg";
    }
    if (p == "GRB") {
        return "grb";
    }
    if (p == "GBR") {
        return "gbr";
    }
    if (p == "RGBW") {
        return "rgbw";
    }
    if (p == "RBGW") {
        return "rbgw";
    }
    if (p == "BGRW") {
        return "bgrw";
    }
    if (p == "BRGW") {
        return "brgw";
    }
    if (p == "GRBW") {
        return "grbw";
    }
    if (p == "GBRW") {
        return "gbrw";
    }
    return p;
}
bool ESPixelStick::SetOutputsV4(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("ESPixelStick Outputs Upload: Uploading to %s", (const char *)_ip.c_str());

    std::string check;
    UDController cud(controller, outputManager, allmodels, check, false);
    auto rules = ControllerCaps::GetControllerConfig(controller);
    bool success = cud.Check(rules, check);
    cud.Dump();
    logger_base.debug(check);

    if (success) {
        _wsClient.Send("{\"cmd\":{\"get\":\"output\"}}");
        std::string config = GetWSResponse();
        wxJSONValue origJson;
        wxJSONReader reader;
        reader.Parse(config, &origJson);
        bool changed = false;
        wxJSONValue outputConfig = origJson["get"]["output_config"]; //get the output_config element
        
        for (int x = 0; x < cud.GetMaxPixelPort(); x++) {
            UDControllerPort* port = cud.GetControllerPixelPort(x + 1);
            std::string proto = MapV4Type(port->GetProtocol());
            std::string pixels = std::to_string(port->Pixels());
            int brightness = 100;
            std::string gamma = "1";
            std::string colorOrder = "rgb";
            std::string groupCount = "1";
            std::string startNulls = "0";
            std::string endNulls = "0";
            auto s = port->GetModels().front();
            if (s) {
                brightness = s->GetBrightness(100);
                colorOrder = MapV4ColorOrder(s->GetColourOrder("rgb"));
                gamma = std::to_string(s->GetGamma(1.0));
                int gc = s->GetGroupCount(1);
                if (gc == 0) {
                    gc = 1;
                }
                groupCount = std::to_string(gc);
                startNulls = std::to_string(s->GetStartNullPixels(0));
                endNulls = std::to_string(s->GetEndNullPixels(0));
            }
            
            std::string outidx = std::to_string(x);
            std::string curIdx = std::to_string(outputConfig["channels"][outidx]["type"].AsInt());
            if (outputConfig["channels"][outidx][curIdx]["type"].AsString() != proto) {
                changed = true;
                for (int i = 0; i < 20; i++) {
                    std::string idx = std::to_string(i);
                    if (!outputConfig["channels"][outidx].HasMember(idx)) {
                        return false;
                    }
                    if (outputConfig["channels"][outidx][idx]["type"].AsString() == proto) {
                        //found the new element, flip over to using that protocol
                        outputConfig["channels"][outidx]["type"] = i;
                        curIdx = idx;
                        break;
                    }
                }
            }
            if (proto == "WS2811" || proto == "TM1814") {
                if (gamma != outputConfig["channels"][outidx][curIdx]["gamma"].AsString()) {
                    changed = true;
                    outputConfig["channels"][outidx][curIdx]["gamma"] = gamma;
                }
                int b = brightness;
                if (b > 100) {
                    b = 100;
                }

                std::string b2 = std::to_string(b);
                if (b2 != outputConfig["channels"][outidx][curIdx]["brightness"].AsString()) {
                    changed = true;
                    outputConfig["channels"][outidx][curIdx]["brightness"] = b2;
                }
                if (colorOrder != outputConfig["channels"][outidx][curIdx]["color_order"].AsString()) {
                    changed = true;
                    outputConfig["channels"][outidx][curIdx]["color_order"] = colorOrder;
                }
                if (groupCount != outputConfig["channels"][outidx][curIdx]["group_size"].AsString()) {
                    changed = true;
                    outputConfig["channels"][outidx][curIdx]["group_size"] = groupCount;
                }
                if (startNulls != outputConfig["channels"][outidx][curIdx]["prependnullcount"].AsString()) {
                    changed = true;
                    outputConfig["channels"][outidx][curIdx]["prependnullcount"] = startNulls;
                }
                if (endNulls != outputConfig["channels"][outidx][curIdx]["appendnullcount"].AsString()) {
                    changed = true;
                    outputConfig["channels"][outidx][curIdx]["appendnullcount"] = endNulls;
                }
                if (pixels != outputConfig["channels"][outidx][curIdx]["pixel_count"].AsString()) {
                    changed = true;
                    outputConfig["channels"][outidx][curIdx]["pixel_count"] = pixels;
                }
            } else if (proto == "GECE") {
                int b = brightness;
                if (b > 100) {
                    b = 100;
                }
                std::string b2 = std::to_string(b);
                if (b2 != outputConfig["channels"][outidx][curIdx]["brightness"].AsString()) {
                    changed = true;
                    outputConfig["channels"][outidx][curIdx]["brightness"] = b2;
                }

                if (pixels != outputConfig["channels"][outidx][curIdx]["pixel_count"].AsString()) {
                    changed = true;
                    outputConfig["channels"][outidx][curIdx]["pixel_count"] = pixels;
                }
            }
        }
        for (int x = 0; x < cud.GetMaxSerialPort(); x++) {
            UDControllerPort* port = cud.GetControllerSerialPort(x + 1);
            std::string proto = MapV4Type(port->GetProtocol());
            std::string channels = std::to_string(port->Channels());
            
            std::string outidx = std::to_string(x + rules->GetMaxPixelPort());
            std::string curIdx = std::to_string(outputConfig["channels"][outidx]["type"].AsInt());
            if (outputConfig["channels"][outidx][curIdx]["type"].AsString() != proto) {
                changed = true;
                for (int i = 0; i < 20; i++) {
                    std::string idx = std::to_string(i);
                    if (!outputConfig["channels"][outidx].HasMember(idx)) {
                        return false;
                    }
                    if (outputConfig["channels"][outidx][idx]["type"].AsString() == proto) {
                        //found the new element, flip over to using that protocol
                        outputConfig["channels"][outidx]["type"] = i;
                        curIdx = idx;
                        break;
                    }
                }
            }
            if (channels != outputConfig["channels"][outidx][curIdx]["num_chan"].AsString()) {
                changed = true;
                outputConfig["channels"][outidx][curIdx]["num_chan"] = channels;
            }
        }
        if (changed) {
            wxJSONWriter writer(wxJSONWRITER_NONE, 0, 3);
            wxString message;
            
            wxJSONValue newJson;
            newJson["cmd"]["set"]["output"]["output_config"] = outputConfig;
            
            writer.Write(newJson, message);
            message.Replace(" : ", ":");

            if (_wsClient.Send(message)) {
                success = true;
                logger_base.debug("ESPixelStick Outputs Upload: Success!!!");
            }
            GetWSResponse();
        }
    }
    return true;
}
bool ESPixelStick::SetOutputsV3(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("ESPixelStick Outputs Upload: Uploading to %s", (const char *)_ip.c_str());

    std::string check;
    UDController cud(controller, outputManager, allmodels, check, false);

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
        std::string config = GetWSResponse();
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
        GetWSResponse();
    } else {
        DisplayError("Not uploaded due to errors.\n" + check);
    }

    return success;
}
#pragma endregion
