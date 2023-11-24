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
#include "../outputs/DDPOutput.h"
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
    wxJSONValue HttpResponse;
    if (GetAdminInformation(HttpResponse)) {
        if (HttpResponse.GetType() == wxJSONTYPE_NULL) {
            return false;
        }
        _UsingHttpConfig = true;
        _model = "ESPixelStick";
        _connected = true;
        _version = HttpResponse["version"].AsString();
        logger_base.debug("Connected to ESPixelStick HTTP - Firmware Version %s", (const char*)_version.c_str());
        return true;
    }

    return false;
}

bool ESPixelStick::GetAdminInformation(wxJSONValue& Response)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("GetAdminInformation: Getting ESPSv4 HTTP admin info");
    return GetHttpConfig("admininfo", "admin", Response);
}

bool ESPixelStick::GetInputConfig(wxJSONValue& Response)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("GetInputConfig: Getting ESPSv4 HTTP input config");
    return (_UsingHttpConfig) ? GetHttpConfig("input_config", "input_config", Response) : GetWsConfig("input", "input_config", Response);
}

bool ESPixelStick::SetInputConfig(wxJSONValue& Data)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SetInputConfig: Setting ESPSv4 HTTP input config");
    return (_UsingHttpConfig) ? SetHttpConfig("input_config", "input_config", Data) : SetWsConfig("input", "input_config", Data);
}

bool ESPixelStick::GetOutputConfig(wxJSONValue& Response)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("GetOutputConfig: Getting ESPSv4 HTTP output config");
    return (_UsingHttpConfig) ? GetHttpConfig("output_config", "output_config", Response) : GetWsConfig("output", "output_config", Response);
}

bool ESPixelStick::SetOutputConfig(wxJSONValue& Data)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SetOutputConfig: Setting ESPSv4 HTTP output config");
    return ((_UsingHttpConfig) ? SetHttpConfig("output_config", "output_config", Data) : SetWsConfig("output", "output_config", Data));
}

bool ESPixelStick::GetHttpConfig(std::string FileName, std::string key, wxJSONValue& Response)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("GetHttpConfig: Getting ESPSv4 HTTP config file");

    std::string url = "/conf/" + FileName + ".json";
    std::string RawData = GetURL(url);
    // logger_base.debug(std::string("GetHttpConfig: RawData: ") + RawData);

    wxJSONReader reader;
    wxJSONValue ParsedData;
    reader.Parse(RawData, &ParsedData);
    Response = ParsedData[key];
    logger_base.debug(std::string("GetHttpConfig: Response: ") + std::to_string(Response.Size()));
    return (0 != Response.Size());
}

bool ESPixelStick::SetHttpConfig(std::string filename, std::string key, wxJSONValue& _Data)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SetHttpConfig: Setting ESPSv4 HTTP config file");

    std::string url = "http://" + _ip + "/conf/" + filename + ".json";
    wxJSONWriter writer;
    wxString Data;
    wxJSONValue newJson;
    newJson[key] = _Data;
    writer.Write(newJson, Data);
    // logger_base.debug(std::string("SetHttpConfig: Data: '") + Data + "'");

    std::string contentType = "application/json";
    int ReturnCode = -1;
    std::vector<unsigned char> value(Data.begin(), Data.end());
    CurlManager::INSTANCE.doPost(url, contentType, value, ReturnCode);

    // logger_base.debug(std::string("SetHttpConfig: ReturnCode: '") + std::to_string(ReturnCode) + "'");
    return (200 == ReturnCode);
}

bool ESPixelStick::GetWsConfig(std::string SectionName, std::string key, wxJSONValue& Response)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("GetWsConfig: Getting ESPSv4 HTTP config file");

    wxJSONWriter writer(wxJSONWRITER_NONE, 0, 3);
    wxString message;
    wxJSONValue newJson;
    newJson["cmd"]["get"] = SectionName;
    writer.Write(newJson, message);
    message.Replace(" : ", ":");
    // logger_base.debug(std::string("GetWsConfig: cmd: ") + message);
    _wsClient.Send(message);

    wxJSONValue ParsedData;
    wxJSONReader reader;
    std::string RawData = GetWSResponse();
    // logger_base.debug(std::string("GetWsConfig: RawData: ") + RawData);
    reader.Parse(RawData, &ParsedData);
    Response = ParsedData["get"][key];
    // logger_base.debug(std::string("GetWsConfig: Response: ") + std::to_string(Response.Size()));
    return (0 != Response.Size());
}

bool ESPixelStick::SetWsConfig(std::string SectionName, std::string key, wxJSONValue& Data)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SetWsConfig: Setting ESPSv4 HTTP config file");

    wxJSONWriter writer(wxJSONWRITER_NONE, 0, 3);
    wxString message;
    wxJSONValue newJson;
    newJson["cmd"]["set"][SectionName] = Data;
    writer.Write(newJson, message);
    message.Replace(" : ", ":");
    // logger_base.debug(std::string("SetWsConfig: cmd: ") + message);
    _wsClient.Send(message);

    wxJSONValue ParsedData;
    wxJSONReader reader;
    std::string RawData = GetWSResponse();
    // logger_base.debug(std::string("SetWsConfig: RawData: ") + RawData);
    reader.Parse(RawData, &ParsedData);
    wxJSONValue Response = ParsedData["cmd"];
    std::string returnValue = Response.AsString();
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
bool ESPixelStick::SetInputUniverses(Controller* controller, wxWindow* parent) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_version.size() > 0 && _version[0] == '4') {
        //only needed on V4.   V3 will upload inputs with outputs

        bool changed = false;
        wxJSONValue inputConfig; // get the input_config element
        
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
        std::string origTypeIdx = std::to_string(inputConfig["channels"]["0"]["type"].AsInt());
        std::string origType = inputConfig["channels"]["0"][origTypeIdx]["type"].AsString();
        if (origType != type) {
            changed = true;
            for (int x = 0; x < 10; x++) {
                std::string idx = std::to_string(x);
                if (!inputConfig["channels"]["0"].HasMember(idx)) {
                    continue;
                }
                if (inputConfig["channels"]["0"][idx]["type"].AsString() == type) {
                    //found the new element, flip over to using that protocol
                    inputConfig["channels"]["0"]["type"] = x;
                    origTypeIdx = idx;
                    break;
                }
            }
        }
        if (type == "E1.31" || type == "Artnet" ) {
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
            if (outputs.front()->GetType() == OUTPUT_DDP)
            {
                DDPOutput* ddp = dynamic_cast<DDPOutput*>(outputs.front());
                if (ddp) {
                    if (ddp->IsKeepChannelNumbers()) {
                        DisplayError("The DDP 'Keep Channel Numbers' option is not support with ESPixelStick, Please Disable");
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

    if (success) {
        bool changed = false;
        wxJSONValue outputConfig;

        GetOutputConfig(outputConfig);

        for (int x = 0; x < cud.GetMaxPixelPort(); x++) {
            UDControllerPort* port = cud.GetControllerPixelPort(x + 1);
            std::string const proto = MapV4Type(port->GetProtocol());
            std::string const pixels = std::to_string(port->Pixels());
            int brightness{ -1 };
            float gamma{ -1.0F };
            std::string colorOrder;
            int groupCount{ -1 };
            int startNulls{ -1 };
            int endNulls{ -1 };
            auto s = port->GetModels().front();
            if (s) {
                brightness = s->GetBrightness(-1);
                colorOrder = MapV4ColorOrder(s->GetColourOrder(""));
                gamma = s->GetGamma(-1.0F);
                int gc = s->GetGroupCount(-1);
                if (gc == 0) {
                    gc = 1;
                }
                groupCount = gc;
                startNulls = s->GetStartNullPixels(-1);
                endNulls = s->GetEndNullPixels(-1);
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
                if (gamma > -1.0F) {
                    if (gamma < 1.0F) {
                        gamma = 1.0F;
                    }
                    if (gamma > 5.0F) {
                        gamma = 5.0F;
                    }
                    std::string const s_gamma = std::to_string(gamma);
                    if (s_gamma != outputConfig["channels"][outidx][curIdx]["gamma"].AsString()) {
                        changed = true;
                        outputConfig["channels"][outidx][curIdx]["gamma"] = s_gamma;
                    }
                }
                if (brightness != -1) {
                    int b = brightness;
                    if (b > 100) {
                        b = 100;
                    }

                    std::string const b2 = std::to_string(b);
                    if (b2 != outputConfig["channels"][outidx][curIdx]["brightness"].AsString()) {
                        changed = true;
                        outputConfig["channels"][outidx][curIdx]["brightness"] = b2;
                    }
                }
                if (!colorOrder.empty() && colorOrder != outputConfig["channels"][outidx][curIdx]["color_order"].AsString()) {
                    changed = true;
                    outputConfig["channels"][outidx][curIdx]["color_order"] = colorOrder;
                }
                if (groupCount != -1) {
                    std::string const s_groupCount = std::to_string(groupCount);
                    if (s_groupCount != outputConfig["channels"][outidx][curIdx]["group_size"].AsString()) {
                        changed = true;
                        outputConfig["channels"][outidx][curIdx]["group_size"] = s_groupCount;
                    }
                }
                if (startNulls != -1) {
                    std::string const s_startNulls = std::to_string(startNulls);
                    if (s_startNulls != outputConfig["channels"][outidx][curIdx]["prependnullcount"].AsString()) {
                        changed = true;
                        outputConfig["channels"][outidx][curIdx]["prependnullcount"] = s_startNulls;
                    }
                }
                if (endNulls != -1) {
                    std::string const s_endNulls = std::to_string(endNulls);
                    if (s_endNulls != outputConfig["channels"][outidx][curIdx]["appendnullcount"].AsString()) {
                        changed = true;
                        outputConfig["channels"][outidx][curIdx]["appendnullcount"] = s_endNulls;
                    }
                }
                if (pixels != outputConfig["channels"][outidx][curIdx]["pixel_count"].AsString()) {
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
                    if (b2 != outputConfig["channels"][outidx][curIdx]["brightness"].AsString()) {
                        changed = true;
                        outputConfig["channels"][outidx][curIdx]["brightness"] = b2;
                    }
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
        newJson["e131"]["channel_start"] = port->GetUniverseStartChannel();
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
                float bval = (float)brightness;
                bval /= 100.0F;
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
