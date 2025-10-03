/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/msgdlg.h>
#include <wx/regex.h>

#include "ESPixelStick.h"
#include "../models/Model.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../outputs/DDPOutput.h"
#include "../models/ModelManager.h"
#include "../UtilFunctions.h"
#include "ControllerCaps.h"
#include "../outputs/ControllerEthernet.h"

#include <log4cpp/Category.hh>

#pragma region Constructors and Destructors
// This is tested with a pixel stick running v3.0 of the firmware
ESPixelStick::ESPixelStick(const std::string& ip, const std::string &proxy) : BaseController(ip, proxy) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("connecting to ESPixelStick controller on %s.", (const char*)_ip.c_str());

    if (!CheckHTTPconnection()) 
    {
        if (!CheckWsConnection()) {
            _connected = false;
            logger_base.error("Error connecting to ESPixelStick controller on %s.", (const char*)_ip.c_str());
        }
    }
}
#pragma endregion

#pragma region Private Functions

bool ESPixelStick::CheckWsConnection()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _wsClient.Connect(_ip, "/ws");

    if (_wsClient.IsConnected()) {
        _model = "ESPixelStick";
        _connected = true;
        _wsClient.Send("G2");
        _version = GetFromJSON("", "version", GetWSResponse());
        logger_base.debug("Connected to ESPixelStick WebSocket - Firmware Version %s", (const char*)_version.c_str());
        return true;
    }

    return false;
}

bool ESPixelStick::CheckHTTPconnection()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("CheckHTTPconnection ", (const char*)_ip.c_str());

    // open HTTP connection to get admininfo
    nlohmann::json HttpResponse;
    if (GetAdminInformation(HttpResponse)) {
        if (HttpResponse.is_null()) {
            return false;
        }
        _UsingHttpConfig = true;
        _model = "ESPixelStick";
        _connected = true;
        _version = HttpResponse["version"].get<std::string>();
        logger_base.debug("Connected to ESPixelStick HTTP - Firmware Version %s", (const char*)_version.c_str());
        return true;
    }

    return false;
}

bool ESPixelStick::GetAdminInformation(nlohmann::json& Response)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("GetAdminInformation: Getting ESPSv4 HTTP admin info");
    return GetHttpConfig("admininfo", "admin", Response);
}

bool ESPixelStick::GetInputConfig(nlohmann::json& Response) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("GetInputConfig: Getting ESPSv4 HTTP input config");
    return (_UsingHttpConfig) ? GetHttpConfig("input_config", "input_config", Response) : GetWsConfig("input", "input_config", Response);
}

bool ESPixelStick::SetInputConfig(nlohmann::json& Data) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SetInputConfig: Setting ESPSv4 HTTP input config");
    return (_UsingHttpConfig) ? SetHttpConfig("input_config", "input_config", Data) : SetWsConfig("input", "input_config", Data);
}

bool ESPixelStick::GetOutputConfig(nlohmann::json& Response) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("GetOutputConfig: Getting ESPSv4 HTTP output config");
    return (_UsingHttpConfig) ? GetHttpConfig("output_config", "output_config", Response) : GetWsConfig("output", "output_config", Response);
}

bool ESPixelStick::SetOutputConfig(nlohmann::json& Data) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SetOutputConfig: Setting ESPSv4 HTTP output config");
    return ((_UsingHttpConfig) ? SetHttpConfig("output_config", "output_config", Data) : SetWsConfig("output", "output_config", Data));
}

bool ESPixelStick::GetHttpConfig(std::string FileName, std::string key, nlohmann::json& Response) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("GetHttpConfig: Getting ESPSv4 HTTP config file");

    std::string const url = "/conf/" + FileName + ".json";
    std::string RawData = GetURL(url);
    // logger_base.debug(std::string("GetHttpConfig: RawData: ") + RawData);

    nlohmann::json ParsedData = nlohmann::json::parse(RawData);
    Response = ParsedData[key];
    logger_base.debug(std::string("GetHttpConfig: Response: ") + std::to_string(Response.size()));
    return (0 != Response.size());
}

bool ESPixelStick::SetHttpConfig(std::string filename, std::string key, nlohmann::json& _Data) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SetHttpConfig: Setting ESPSv4 HTTP config file");

    std::string const url = "http://" + _ip + "/conf/" + filename + ".json";

    nlohmann::json newJson;
    newJson[key] = _Data;
    auto Data = newJson.dump();
    // logger_base.debug(std::string("SetHttpConfig: Data: '") + Data + "'");

    std::string contentType = "application/json";
    int ReturnCode = -1;
    std::vector<unsigned char> value(Data.begin(), Data.end());
    CurlManager::INSTANCE.doPost(url, contentType, value, ReturnCode);

    // logger_base.debug(std::string("SetHttpConfig: ReturnCode: '") + std::to_string(ReturnCode) + "'");
    return (200 == ReturnCode);
}

bool ESPixelStick::GetWsConfig(std::string SectionName, std::string key, nlohmann::json& Response) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("GetWsConfig: Getting ESPSv4 HTTP config file");

    nlohmann::json newJson;
    newJson["cmd"]["get"] = SectionName;
    // logger_base.debug(std::string("GetWsConfig: cmd: ") + newJson.dump());
    _wsClient.Send(newJson.dump());
    std::string RawData = GetWSResponse();
    nlohmann::json ParsedData = nlohmann::json::parse(RawData);
    Response = ParsedData["get"][key];
    // logger_base.debug(std::string("GetWsConfig: Response: ") + std::to_string(Response.Size()));
    return (0 != Response.size());
}

bool ESPixelStick::SetWsConfig(std::string SectionName, std::string key, nlohmann::json& Data) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SetWsConfig: Setting ESPSv4 HTTP config file");

    nlohmann::json newJson;
    newJson["cmd"]["set"][SectionName] = Data;
    // logger_base.debug(std::string("SetWsConfig: cmd: ") + newJson.dump());
    _wsClient.Send(newJson.dump());

    std::string RawData = GetWSResponse();
    // logger_base.debug(std::string("SetWsConfig: RawData: ") + RawData);
    nlohmann::json ParsedData = nlohmann::json::parse(RawData);
    nlohmann::json Response = ParsedData["cmd"];
    std::string returnValue = Response.get<std::string>();
    // logger_base.debug(std::string("SetWsConfig: returnValue: '") + returnValue + "'");
    bool result = returnValue == "OK";
    // logger_base.debug(std::string("SetWsConfig: result: ") + std::to_string(result));
    return result;
}

std::string ESPixelStick::GetWSResponse()
{
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

std::string ESPixelStick::DecodeStringPortProtocol(std::string const& protocol) const {
    wxString p(protocol);
    p = p.Lower();
    if (p == "ws2811") {
        return "0";
    }
    if (p == "gece"){
        return "1";
    }
    return "null";
}

std::string ESPixelStick::DecodeSerialPortProtocol(std::string const& protocol) const {
    // This is not right as I dont actually have a board that supports this
    wxString p(protocol);
    p = p.Lower();

    if (p == "dmx") {
        return "null";
    }
    if (p == "renard") {
        return "null";
    }
    return "null";
}

std::string ESPixelStick::DecodeSerialSpeed(std::string const& protocol) const {
    // This is not right as I dont actually have a board that supports this
    wxString p(protocol);
    p = p.Lower();

    if (p == "dmx") {
        return "null";
    }
    if (p == "renard") {
        return "null";
    }
    return "null";
}

std::string ESPixelStick::GetFromJSON(std::string const& section, std::string const& key, std::string const& json) const {
    //skip over the "G2" header or whatever
    for (int x = 0; x < json.size(); x++) {
        if (json[x] == '{' || json[x] == '[') {
            auto config = json.substr(x);
            nlohmann::json origJson = nlohmann::json::parse(config);
            nlohmann::json val = origJson;
            if (section != "") {
                val = origJson[section];
            }
            if (!val.contains(key))
            { 
                return "";
            }

            val = val.at(key);
            if (val.is_string()) {
                return val.get<std::string>();
            }
            if (val.is_number_integer()) {
                return std::to_string(val.get<int>());
            }
            if (val.is_number_float()) {
                return std::to_string(val.get<float>());
            }
        }
    }
    return "";
}
#pragma endregion

#pragma region Getters and Setters
bool ESPixelStick::SetInputUniverses(Controller* controller, wxWindow* parent) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_version.size() > 0 && _version[0] == '4') {
        //only needed on V4.   V3 will upload inputs with outputs

        bool changed = false;
        nlohmann::json inputConfig; // get the input_config element
        
        GetInputConfig(inputConfig);

        std::list<Output*> outputs = controller->GetOutputs();

        std::string type = "DDP";
        int startUniverse = 0;
        int chanPerUniverse = 512;
        if (outputs.front()->GetType() == OUTPUT_E131) {
            type = "E1.31";
            startUniverse = outputs.front()->GetUniverse();
            chanPerUniverse = outputs.front()->GetChannels();
        } else if (outputs.front()->GetType() == OUTPUT_ARTNET) {
            type = "Artnet";
            startUniverse = outputs.front()->GetUniverse();
            chanPerUniverse = outputs.front()->GetChannels();
        }
        std::string s_origTypeIdx = std::to_string(inputConfig["channels"]["0"]["type"].get<int>());
        std::string const s_origType = inputConfig["channels"]["0"][s_origTypeIdx]["type"].get<std::string>();
        if (s_origType != type) {
            changed = true;
            for (int x = 0; x < 10; x++) {
                std::string idx = std::to_string(x);
                if (!inputConfig["channels"]["0"].contains(idx)) {
                    continue;
                }
                if (inputConfig["channels"]["0"][idx]["type"].get<std::string>() == type) {
                    //found the new element, flip over to using that protocol
                    inputConfig["channels"]["0"]["type"] = x;
                    s_origTypeIdx = idx;
                    break;
                }
            }
        }
        if (type == "E1.31" || type == "Artnet" ) {
            std::string const univString = std::to_string(startUniverse);
            std::string const sizeString = std::to_string(chanPerUniverse);

            //{"type":"E1.31","universe":1,"universe_limit":512,"channel_start":1}
            if (inputConfig["channels"]["0"][s_origTypeIdx]["universe"].get<std::string>() != univString) {
                inputConfig["channels"]["0"][s_origTypeIdx]["universe"] = startUniverse;
                changed = true;
            }
            if (inputConfig["channels"]["0"][s_origTypeIdx]["universe_limit"].get<std::string>() != sizeString) {
                inputConfig["channels"]["0"][s_origTypeIdx]["universe_limit"] = chanPerUniverse;
                changed = true;
            }
            //inputConfig["channels"]["0"][s_origTypeIdx]["channel_start"] = channel_start;
        } else if (type == "DDP") {
            //nothing to do for DDP
            if (outputs.front()->GetType() == OUTPUT_DDP)
            {
                DDPOutput* ddp = dynamic_cast<DDPOutput*>(outputs.front());
                if (ddp) {
                    if (ddp->IsKeepChannelNumbers()) {
                        DisplayError("The DDP 'Keep Channel Numbers' option is not supported with ESPixelStick. Please disable.");
                        return false;
                    }
                }
            }
        }

        if (changed) {
            if (SetInputConfig(inputConfig)) {
                logger_base.debug("ESPixelStick Inputs Upload: Success!!!");
            }
        }
    }
    return true;
}

bool ESPixelStick::UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) {
    SetInputUniverses(controller, parent);
    return SetOutputs(allmodels, outputManager, controller, parent);
}

bool ESPixelStick::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) {
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

bool ESPixelStick::SetOutputsV4(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("ESPixelStick Outputs Upload: Uploading to %s", (const char *)_ip.c_str());

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);
    auto rules = ControllerCaps::GetControllerConfig(controller);
    bool success = cud.Check(rules, check);
    cud.Dump();
    logger_base.debug(check);

    auto getJSONAsNumStr = [](nlohmann::json& json, std::string const& parm) {
        if (!json.contains(parm)){
            return std::string();
        }
        if (json[parm].is_string()) {
            return json[parm].get<std::string>();
        }
        if (json[parm].is_number_integer()) {
            return std::to_string(json[parm].get<int>());
        }
        if (json[parm].is_number_float()) {
            return std::to_string(json[parm].get<float>());
        }
        return std::string();
    };

    if (success) {
        bool changed = false;
        nlohmann::json outputConfig;

        GetOutputConfig(outputConfig);

        if (controller->IsFullxLightsControl()) {
            if (outputConfig.contains("channels")) {
                nlohmann::json& channels = outputConfig["channels"];
                for (const auto& [key, _] : channels.items()) {
                    nlohmann::json& channel = channels[key];
                    if (channel.contains("type"))
                        channel["type"] = 0; // disable all outputs, since used ones will be re-enabled next
                }
                changed = true;
            }
        }

        for (int x = 0; x < cud.GetMaxPixelPort(); x++) {
            if (cud.HasPixelPort(x + 1)) {
                UDControllerPort* port = cud.GetControllerPixelPort(x + 1);
                std::string const proto = MapV4Type(port->GetProtocol());
                int const pixels = port->Pixels();
                int brightness{ -1 };
                float gamma{ -1.0F };
                std::string colorOrder;
                int groupCount{ -1 };
                int startNulls{ -1 };
                int endNulls{ -1 };
                int zigzag{ -1 };
                auto s = port->GetModels().front();
                if (s) {
                    brightness = s->GetBrightness(-1);
                    if (brightness < 0 && controller->IsFullxLightsControl()) {
                        brightness = controller->GetDefaultBrightnessUnderFullControl();
                    };
                    gamma = s->GetGamma(-1.0F);
                    if (gamma < 0 && controller->IsFullxLightsControl()) {
  						gamma = controller->GetDefaultGammaUnderFullControl();
					};
                    colorOrder = MapV4ColorOrder(s->GetColourOrder(""));
                    int gc = s->GetGroupCount(-1);
                    if (gc == 0) {
                        gc = 1;
                    }
                    groupCount = gc;
                    startNulls = s->GetStartNullPixels(-1);
                    endNulls = s->GetEndNullPixels(-1);
                    zigzag = s->GetZigZag(1);
                }

                std::string const outidx = std::to_string(x);
                std::string curIdx = std::to_string(outputConfig["channels"][outidx]["type"].get<int>());
                if (outputConfig["channels"][outidx][curIdx]["type"].get<std::string>() != proto) {
                    changed = true;
                    for (int i = 0; i < 20; i++) {
                        std::string idx = std::to_string(i);
                        if (!outputConfig["channels"][outidx].contains(idx)) {
                            return false;
                        }
                        if (outputConfig["channels"][outidx][idx]["type"].get<std::string>() == proto) {
                            // found the new element, flip over to using that protocol
                            outputConfig["channels"][outidx]["type"] = i;
                            curIdx = idx;
                            break;
                        }
                    }
                }
                if (proto == "WS2811" || proto == "TM1814") {
                    if (gamma > -1.0F) {
                        if (gamma < 1.0F) {
                            gamma = 1.0F;
                        }
                        if (gamma > 5.0F) {
                            gamma = 5.0F;
                        }
                        std::string const s_gamma = std::to_string(gamma);
                        if (s_gamma != getJSONAsNumStr(outputConfig["channels"][outidx][curIdx], "gamma")) {
                            changed = true;
                            outputConfig["channels"][outidx][curIdx]["gamma"] = gamma;
                        }
                    }
                    if (brightness != -1) {
                        int b = brightness;
                        if (b > 100) {
                            b = 100;
                        }

                        std::string const b2 = std::to_string(b);
                        if (b2 != getJSONAsNumStr(outputConfig["channels"][outidx][curIdx],"brightness")) {
                            changed = true;
                            outputConfig["channels"][outidx][curIdx]["brightness"] = b;
                        }
                    }
                    if (!colorOrder.empty() && colorOrder != outputConfig["channels"][outidx][curIdx]["color_order"].get<std::string>()) {
                        changed = true;
                        outputConfig["channels"][outidx][curIdx]["color_order"] = colorOrder;
                    }
                    if (groupCount != -1) {
                        std::string const s_groupCount = std::to_string(groupCount);
                        if (s_groupCount != getJSONAsNumStr(outputConfig["channels"][outidx][curIdx],"group_size")) {
                            changed = true;
                            outputConfig["channels"][outidx][curIdx]["group_size"] = groupCount;
                        }
                    }
                    if (zigzag != -1) {
						std::string const s_zigzag = std::to_string(zigzag);
                        if (s_zigzag != getJSONAsNumStr(outputConfig["channels"][outidx][curIdx],"zig_size")) {
							changed = true;
							outputConfig["channels"][outidx][curIdx]["zig_size"] = zigzag;
						}
					}
                    if (startNulls != -1) {
                        std::string const s_startNulls = std::to_string(startNulls);
                        if (s_startNulls != getJSONAsNumStr(outputConfig["channels"][outidx][curIdx],"prependnullcount")) {
                            changed = true;
                            outputConfig["channels"][outidx][curIdx]["prependnullcount"] = startNulls;
                        }
                    }
                    if (endNulls != -1) {
                        std::string const s_endNulls = std::to_string(endNulls);
                        if (s_endNulls != getJSONAsNumStr(outputConfig["channels"][outidx][curIdx],"appendnullcount")) {
                            changed = true;
                            outputConfig["channels"][outidx][curIdx]["appendnullcount"] = endNulls;
                        }
                    }
                    std::string const s_pixels = std::to_string(pixels);
                    if (s_pixels != getJSONAsNumStr(outputConfig["channels"][outidx][curIdx],"pixel_count")) {
                        changed = true;
                        outputConfig["channels"][outidx][curIdx]["pixel_count"] = pixels;
                    }
                } else if (proto == "GECE") {
                    if (brightness != -1) {
                        int b = brightness;
                        if (b > 100) {
                            b = 100;
                        }
                        std::string const b2 = std::to_string(b);
                        if (b2 != getJSONAsNumStr(outputConfig["channels"][outidx][curIdx],"brightness")) {
                            changed = true;
                            outputConfig["channels"][outidx][curIdx]["brightness"] = b;
                        }
                    }
                    std::string const s_pixels = std::to_string(pixels);
                    if (s_pixels != getJSONAsNumStr(outputConfig["channels"][outidx][curIdx],"pixel_count")) {
                        changed = true;
                        outputConfig["channels"][outidx][curIdx]["pixel_count"] = pixels;
                    }
                }
            }
        }
        for (int x = 0; x < cud.GetMaxSerialPort(); x++) {
            if (cud.HasSerialPort(x + 1)) {
                UDControllerPort* port = cud.GetControllerSerialPort(x + 1);
                std::string const proto = MapV4Type(port->GetProtocol());
                std::string const channels = std::to_string(port->Channels());

                std::string const outidx = std::to_string(x);
                std::string curIdx = std::to_string(outputConfig["channels"][outidx]["type"].get<int>());
                if (outputConfig["channels"][outidx][curIdx]["type"].get<std::string>() != proto) {
                    changed = true;
                    for (int i = 0; i < 20; i++) {
                        std::string const idx = std::to_string(i);
                        if (!outputConfig["channels"][outidx].contains(idx)) {
                            return false;
                        }
                        if (outputConfig["channels"][outidx][idx]["type"].get<std::string>() == proto) {
                            // found the new element, flip over to using that protocol
                            outputConfig["channels"][outidx]["type"] = i;
                            curIdx = idx;
                            break;
                        }
                    }
                }
                if (channels != outputConfig["channels"][outidx][curIdx]["num_chan"].get<std::string>()) {
                    changed = true;
                    outputConfig["channels"][outidx][curIdx]["num_chan"] = channels;
                }
            }
        }
        if (changed)
        {
            if (SetOutputConfig(outputConfig)) {
                success = true;
                logger_base.debug("ESPixelStick Outputs Upload: Success!!!");
            } else {
                success = false;
                logger_base.error("ESPixelStick Outputs Upload: Failure!!!");
            }
        } else {
            logger_base.debug("ESPixelStick Outputs Upload: No Changes to upload");
        }
    }
    return success;
}
bool ESPixelStick::SetOutputsV3(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("ESPixelStick Outputs Upload: Uploading to %s", (const char *)_ip.c_str());

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);

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
        if (config.empty()) {
            DisplayError("Failed to get Data from ESPixelStick");
            return false;
        }
        config = config.substr(2);

        nlohmann::json origJson = nlohmann::json::parse(config);

        nlohmann::json newJson;
        //copy stuff over to act as defaults
        newJson["device"] = origJson["device"];
        newJson["mqtt"] = origJson["mqtt"];
        newJson["e131"] = origJson["e131"];
        newJson["pixel"] = origJson["pixel"];

        newJson["e131"]["universe"] = port->GetUniverse();
        newJson["e131"]["universe_limit"] = cud.GetFirstOutput()->GetChannels();
        newJson["e131"]["channel_start"] = port->GetUniverseStartChannel();
        newJson["e131"]["channel_count"] = port->Channels();
        newJson["e131"]["multicast"] = ((cud.GetFirstOutput()->GetIP() == "MULTICAST") ? true : false);

        auto s = port->GetModels().front();
        if (s) {
            int const brightness = s->GetBrightness(-9999);
            std::string const colourOrder = s->GetColourOrder("unknown");
            float const gamma = s->GetGamma(-9999);
            int const groupCount = s->GetGroupCount(-9999);

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
                float bval = (float)brightness;
                bval /= 100.0F;
                newJson["pixel"]["briteVal"] = bval;
            }
        }

        std::string message = newJson.dump(3);
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
