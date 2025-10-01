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

#pragma region Global Data
static std::map<wxString, wxString> EspsV4ColorOrders = {
    { "rgb nodes",  "rgb" },
    { "rbg nodes",  "rgb" },
    { "gbr nodes",  "rgb" },
    { "grb nodes",  "rgb" },
    { "brg nodes",  "rgb" },
    { "bgr nodes",  "rgb" },
    { "wrgb nodes", "rgbw" },
    { "wrbg nodes", "rgbw" },
    { "wgbr nodes", "rgbw" },
    { "wgrb nodes", "rgbw" },
    { "wbrg nodes", "rgbw" },
    { "wbgr nodes", "rgbw" },
    { "rgbw nodes", "rgbw" },
    { "rbgw nodes", "rgbw" },
    { "gbrw nodes", "rgbw" },
    { "grbw nodes", "rgbw" },
    { "brgw nodes", "rgbw" },
    { "bgrw nodes", "rgbw" }
};

#pragma endregion

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

bool EspsPort::WriteConfigToJson(nlohmann::json& outputConfig) {
    uint32_t NumItemsChanged = 0;
    nlohmann::json& CurrentPort = outputConfig["channels"][PortId];

    if (std::to_string(CurrentPort["type"].get<int>()) != CurrentProtocolId)
    {
        NumItemsChanged++;
        CurrentPort["type"] = wxAtoi(CurrentProtocolId);
    }
    nlohmann::json& CurrentProtocol = CurrentPort[CurrentProtocolId];

    NumItemsChanged += ProtocolsByName[CurrentProtocolName].WriteConfigToJson(CurrentProtocol);

    return NumItemsChanged != 0;
} // WriteConfigToJson

int EspsV4Protocol::WriteConfigToJson(nlohmann::json& JsonConfig)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    for (auto& [ElementName, ElementValue] : Settings)
    {
        if (ElementName.IsSameAs("type"))
        {
            // never write the type field
            continue;
        }

        if (JsonConfig[ElementName].is_string())
        {
            JsonConfig[ElementName] = ElementValue;
        }
        else if (JsonConfig[ElementName].is_number_float())
        {
            double temp;
            ElementValue.ToDouble(&temp);
            JsonConfig[ElementName] = temp;
            // logger_base.debug("float:ElementName: '" + ElementName + "' value: '" + std::to_string(JsonConfig[ElementName].get<float>()) + "'");
        }
        else if (JsonConfig[ElementName].is_number_integer())
        {
            JsonConfig[ElementName] = wxAtoi(ElementValue);
        }
        else if (JsonConfig[ElementName].is_boolean())
        {
            JsonConfig[ElementName] = ElementValue.IsSameAs("true", false) ? true : false;
        }
        else
        {
            logger_base.error("EspsV4Protocol:WriteConfigToJson: Encountered an unsupported field type in the esps protocol configuration for Element: " + ElementName);
            continue;
        }
    }

    return NumItemsChanged;
} // WriteConfigToJson

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

bool EspsV4Protocol::GetSetting(wxString Name, wxString & value)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // logger_base.debug("EspsV4Protocol:GetSetting: Start");
    bool Response = true;

    if (Settings.contains(Name))
    {
        value = Settings[Name];
    }
    else
    {
        DisplayError(std::string("EspsV4Protocol::PutSetting: Could not get '") + Name + "' No such setting in target data set.");
        Response = false;
    }

    // logger_base.debug("EspsV4Protocol:GetSetting: Done");
    return Response;
} // GetSetting

bool EspsV4Protocol::PutSetting(wxString Name, const wxString & value)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // logger_base.debug("EspsV4Protocol:PutSetting: Start");
    bool Response = true;

    if (Settings.contains(Name))
    {
        if (!value.IsSameAs(Settings[Name]))
        {
            // logger_base.debug("EspsV4Protocol:PutSetting: Name: '" + Name + "' value: '" + value + "' json: '" + Settings[Name] + "'");
            Settings[Name] = value;
            NumItemsChanged++;
        }
    }
    else
    {
        DisplayError(std::string("EspsV4Protocol::PutSetting: Could not update '") + Name + "' No such setting in target data set.");
        Response = false;
    }

    // logger_base.debug("EspsV4Protocol:GetSetting: Done");
    return Response;
} // PutSetting

void EspsV4Protocol::ParseV4Settings(wxString Id, nlohmann::json & JsonConfig)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // logger_base.debug("EspsV4Protocol:ParseV4Config: Start");
    _Id = Id;

    // process the element in the protocol
    for (auto& [ElementId, Elementval] : JsonConfig.items()) {

        wxString FinalValue;

        if (JsonConfig[ElementId].is_string())
        {
            FinalValue = JsonConfig[ElementId].get<std::string>();

        } else if (JsonConfig[ElementId].is_number_float())
        {
            FinalValue = std::to_string(JsonConfig[ElementId].get<float>());
        }
        else if (JsonConfig[ElementId].is_number_integer())
        {
            FinalValue = std::to_string(JsonConfig[ElementId].get<int>());
        }
        else if (JsonConfig[ElementId].is_boolean())
        {
            FinalValue = std::to_string(JsonConfig[ElementId].get<bool>());
        }
        else
        {
            logger_base.error("EspsV4Protocol:ParseV4Config: Encountered an unsupported field type in the esps protocol configuration for Element: " + ElementId);
            continue;
        }

        logger_base.debug("EspsPort:ParseV4Config: Id: " + ElementId + ", value: " + FinalValue);
        Settings[ElementId] = FinalValue;
    }

    _Name = Settings["type"];
    _Name.MakeLower();
    // logger_base.debug("EspsV4Protocol:ParseV4Config: Done");

} // ParseV4Settings

bool EspsPort::ParseV4Settings(nlohmann::json& JsonConfig)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // logger_base.debug("EspsPort:ParseV4Config: Start");

    bool Response = true;

    do // once
    {
        // process the port members
        for (auto& [ElementId, Elementval] : JsonConfig.items()) {
            logger_base.debug("EspsPort:ParseV4Config: ProtocolId: " + ElementId);

            // protocol IDs are a one or two character value
            if (3 > ElementId.length()) {
                // this is a protocol ID
                EspsV4Protocol Protocol;
                Protocol.ParseV4Settings(ElementId, JsonConfig[ElementId]);

                // add protocol to the list of protocols valid for this port
                wxString ProtocolName = Protocol.Name();
                ProtocolName.MakeLower();

                logger_base.debug("Adding protocol '" + ProtocolName + "' to port " + PortId);
                ProtocolsByName[ProtocolName] = Protocol;
                ProtocolIdToProtocolName[Protocol.Id()] = ProtocolName;
            } else if (ElementId == "type") {
                CurrentProtocolId = std::to_string(Elementval.get<int>());
            } else {
                // error getting port information
                logger_base.error("EspsPort:ParseV4Config: Could not parse data for port: " + ElementId);
                Response = false;
                break;
            }
        }

        if (Response)
        {
            // set up quick access to the disabled mode
            DisabledId = ProtocolsByName["disabled"].Id();
            DisabledName = "disabled";

            CurrentProtocolName = ProtocolIdToProtocolName[CurrentProtocolId];
        }
        else
        {
            CurrentProtocolId = DisabledId;
            CurrentProtocolName = DisabledName;
        }

    } while (false);

    // logger_base.debug("EspsPort:ParseV4Config: Done");
    return Response;

} // ParseV4Settings

bool ESPixelStick::ParseV4Config(nlohmann::json& outputConfig) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // logger_base.debug("ParseV4Config: Start");

    bool Response = true;

    EspsConfig.clear();

    do // once
    {
        if (!outputConfig.contains("channels"))
        {
            // config is not valid
            Response = false;
            break;
        }

        // dereference the port configuration
        nlohmann::json& Ports = outputConfig["channels"];

        // process each port
        for (auto & [Portkey, Portval] : Ports.items())
        {
            EspsPort NewEspsPort;
            // logger_base.debug("ParseV4Config: Portkey: " + Portkey);
            NewEspsPort.PortId = Portkey;

            NewEspsPort.ParseV4Settings(Ports[Portkey]);

            // add to the list of ports
            EspsConfig[NewEspsPort.PortId] = NewEspsPort;
        }

    } while (false);

    // logger_base.debug("ParseV4Config: End");
    return Response;

} // ParseV4Config

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
        nlohmann::json outputConfig;

        GetOutputConfig(outputConfig);

        if (!ParseV4Config(outputConfig))
        {
            // no valid config present
            std::string msg = "ESPixelStick Outputs Upload: Could not parse config from ESPixelstick";
            DisplayError(msg);
            return false;
        }

        for (int currentPortId = 0; currentPortId < cud.GetMaxPixelPort(); currentPortId++)
        {
            if (!cud.HasPixelPort(currentPortId + 1))
            {
                // not a valid port
                continue;
            }

            wxString s_currentPortId = wxString::Format(wxT("%d"), currentPortId);
            EspsPort& CurrentEspsPort = EspsConfig[s_currentPortId];
            UDControllerPort* port = cud.GetControllerPixelPort(currentPortId + 1);

            if (!EspsConfig.contains(s_currentPortId))
            {
                // not a valid ESPS port
                CurrentEspsPort.Disable();
                continue;
        	}

            wxString targetProtocolName = port->GetProtocol();
            targetProtocolName = targetProtocolName.Lower();

            if (!CurrentEspsPort.ProtocolsByName.contains(targetProtocolName))
            {
                // not a supported protocol. Use disabled
                CurrentEspsPort.Disable();
                DisplayError("ESPixelStick Outputs Upload: Protocol '" + targetProtocolName + "' not supported by ESPS V4");
                continue;
            }

            EspsV4Protocol & Protocol = CurrentEspsPort.ProtocolsByName[targetProtocolName];
            if (!Protocol.IsPixel())
            {
                // not a pixel protocol
                CurrentEspsPort.Disable();
                continue;
            }

            // bind protocol to the port
            CurrentEspsPort.CurrentProtocolId = Protocol.Id();
            CurrentEspsPort.CurrentProtocolName = Protocol.Name();

            int numberOfChannels = port->Channels();

            auto model = port->GetModels().front();
            if (model)
            {
                int channelsPerPixel = model->GetChannelsPerPixel();
                Protocol.PutSetting("pixel_count", std::to_string(INTROUNDUPDIV(numberOfChannels, channelsPerPixel)));

                wxString colorOrder = model->GetModel()->GetStringType();
                colorOrder.MakeLower();
                if (!EspsV4ColorOrders.contains(colorOrder))
                {
                    // unsupported color order
                    CurrentEspsPort.Disable();
                    std::string msg = "ESPixelStick Outputs Upload: Color Order '" + colorOrder + "' not supported by ESPS V4";
                    DisplayError(msg);
                    return false;
                }
                Protocol.PutSetting("color_order", EspsV4ColorOrders[colorOrder]);

                int brightness = model->GetBrightness(-1);
                if (brightness < 0 && controller->IsFullxLightsControl())
                {
                    brightness = controller->GetDefaultBrightnessUnderFullControl();
                };
                Protocol.PutSetting("brightness", std::to_string(brightness));

                float gamma = model->GetGamma(-1.0F);
                if (gamma < 0 && controller->IsFullxLightsControl())
                {
                    gamma = controller->GetDefaultGammaUnderFullControl();
                };
                Protocol.PutSetting("gamma", std::to_string(gamma));

                Protocol.PutSetting("group_size", std::to_string(model->GetGroupCount(1)));
                Protocol.PutSetting("prependnullcount", std::to_string(model->GetStartNullPixels(0)));
                Protocol.PutSetting("appendnullcount", std::to_string(model->GetEndNullPixels(0)));
                Protocol.PutSetting("zig_size", std::to_string(model->GetZigZag(1)));
            } // end have a model

            changed = CurrentEspsPort.WriteConfigToJson(outputConfig);
        } // end for each xLights pixel port on this controller

        for (int currentPortId = 0; currentPortId < cud.GetMaxSerialPort(); currentPortId++)
        {
            if (!cud.HasSerialPort(currentPortId + 1))
            {
                // not a valid serial port
                continue;
            }

            wxString s_currentPortId = wxString::Format(wxT("%d"), currentPortId);
            EspsPort& CurrentEspsPort = EspsConfig[s_currentPortId];

            if (!EspsConfig.contains(s_currentPortId))
            {
                // not a valid ESPS port
                CurrentEspsPort.Disable();
                continue;
            }

            UDControllerPort* port = cud.GetControllerSerialPort(currentPortId + 1);
            auto model = port->GetModels().front();

            if (!model)
            {
                // no model then ignore the port
                CurrentEspsPort.Disable();
                continue;
            }

            wxString targetProtocolName = port->GetProtocol();
            targetProtocolName = targetProtocolName.Lower();
            if (targetProtocolName == "genericserial")
            {
                targetProtocolName = "serial";
            }

            if (!CurrentEspsPort.ProtocolsByName.contains(targetProtocolName))
            {
                // not a supported protocol. Use disabled
                CurrentEspsPort.Disable();
                DisplayError("ESPixelStick Outputs Upload: Protocol '" + targetProtocolName + "' not supported by ESPS V4");
                continue;
            }

            EspsV4Protocol & Protocol = CurrentEspsPort.ProtocolsByName[targetProtocolName];
            if (Protocol.IsPixel())
            {
                // not a serial protocol
                CurrentEspsPort.Disable();
                continue;
            }

            // bind protocol to the port
            CurrentEspsPort.CurrentProtocolId = Protocol.Id();
            CurrentEspsPort.CurrentProtocolName = Protocol.Name();

            Protocol.PutSetting("num_chan", std::to_string(port->Channels()));
            Protocol.PutSetting("baudrate", std::to_string(model->GetModel()->GetControllerProtocolSpeed()));

            changed = CurrentEspsPort.WriteConfigToJson(outputConfig);

        } // end for each serial port on this controller

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
