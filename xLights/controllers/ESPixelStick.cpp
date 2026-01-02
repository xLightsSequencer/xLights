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

#include "spdlog/spdlog.h"

#pragma region Global Data
static std::map<std::string, std::string> EspsV4ColorOrders = {
    { "rgb",        "rgb"  },
    { "rbg",        "rbg"  },
    { "gbr",        "gbr"  },
    { "grb",        "grb"  },
    { "brg",        "brg"  },
    { "bgr",        "bgr"  },
    { "wrgb",       "wrgb" },
    { "wrbg",       "wrbg" },
    { "wgbr",       "wgbr" },
    { "wgrb",       "wgrb" },
    { "wbrg",       "wbrg" },
    { "wbgr",       "wbgr" },
    { "rgbw",       "rgbw" },
    { "rbgw",       "rbgw" },
    { "gbrw",       "gbrw" },
    { "grbw",       "grbw" },
    { "brgw",       "brgw" },
    { "bgrw",       "bgrw" },
    { "undefined",  "rgb"  },
    { "undefinedw", "rgbw" }
};

#pragma endregion

#pragma region Constructors and Destructors
// This is tested with a pixel stick running v3.0 of the firmware
ESPixelStick::ESPixelStick(const std::string& ip, const std::string &proxy) : BaseController(ip, proxy) {

    spdlog::debug("connecting to ESPixelStick controller on {}.", _ip);

    if (!CheckHTTPconnection()) 
    {
        if (!CheckWsConnection()) {
            _connected = false;
            spdlog::error("Error connecting to ESPixelStick controller on {}.", _ip);
        }
    }
}
#pragma endregion

#pragma region Private Functions

bool ESPixelStick::CheckWsConnection()
{
    _wsClient.Connect(_ip, "/ws");

    if (_wsClient.IsConnected()) {
        _model = "ESPixelStick";
        _connected = true;
        _wsClient.Send("G2");
        _version = GetFromJSON("", "version", GetWSResponse());
        spdlog::debug("Connected to ESPixelStick WebSocket - Firmware Version {}", _version);
        return true;
    }

    return false;
}

bool ESPixelStick::CheckHTTPconnection()
{
    spdlog::debug("CheckHTTPconnection {}", _ip);

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
        spdlog::debug("Connected to ESPixelStick WebSocket - Firmware Version {}", _version);
        return true;
    }

    return false;
}

bool ESPixelStick::GetAdminInformation(nlohmann::json& Response)
{
    spdlog::debug("GetAdminInformation: Getting ESPSv4 HTTP admin info");
    return GetHttpConfig("admininfo", "admin", Response);
}

bool ESPixelStick::GetInputConfig(nlohmann::json& Response) {
    spdlog::debug("GetInputConfig: Getting ESPSv4 HTTP input config");
    return (_UsingHttpConfig) ? GetHttpConfig("input_config", "input_config", Response) : GetWsConfig("input", "input_config", Response);
}

bool ESPixelStick::SetInputConfig(nlohmann::json& Data) {
    spdlog::debug("SetInputConfig: Setting ESPSv4 HTTP input config");
    return (_UsingHttpConfig) ? SetHttpConfig("input_config", "input_config", Data) : SetWsConfig("input", "input_config", Data);
}

bool ESPixelStick::GetOutputConfig(nlohmann::json& Response) {
    spdlog::debug("GetOutputConfig: Getting ESPSv4 HTTP output config");
    return (_UsingHttpConfig) ? GetHttpConfig("output_config", "output_config", Response) : GetWsConfig("output", "output_config", Response);
}

bool ESPixelStick::SetOutputConfig(nlohmann::json& Data) {
    spdlog::debug("SetOutputConfig: Setting ESPSv4 HTTP output config");
    return ((_UsingHttpConfig) ? SetHttpConfig("output_config", "output_config", Data) : SetWsConfig("output", "output_config", Data));
}

bool ESPixelStick::GetHttpConfig(std::string const& FileName, std::string const& key, nlohmann::json& Response) {
    spdlog::debug("GetHttpConfig: Getting ESPSv4 HTTP config file");

    std::string const url = "/conf/" + FileName + ".json";
    std::string RawData = GetURL(url);
    // LOG_DEBUG(std::string("GetHttpConfig: RawData: ") + RawData);
    try
    {
        nlohmann::json ParsedData = nlohmann::json::parse(RawData);
        Response = ParsedData[key];
    }
    catch (nlohmann::json::exception ex)
    {
        Response.clear();
        spdlog::debug(std::string("GetHttpConfig: ERROR: Could not parse json: ") + RawData);
        spdlog::error(std::string("GetHttpConfig: ERROR: Could not parse json: ") + ex.what());
    }

    spdlog::debug(std::string("GetHttpConfig: Response: ") + std::to_string(Response.size()));
    return (0 != Response.size());
}

bool ESPixelStick::SetHttpConfig(std::string const& filename, std::string const& key, nlohmann::json const& _Data) {
    spdlog::debug("SetHttpConfig: Setting ESPSv4 HTTP config file");

    std::string const url = "http://" + _ip + "/conf/" + filename + ".json";

    nlohmann::json newJson;
    newJson[key] = _Data;
    std::string Data = newJson.dump();
    // LOG_DEBUG(std::string("SetHttpConfig: Data: '") + Data + "'");

    std::string contentType = "application/json";
    int ReturnCode = -1;
    std::vector<unsigned char> value(Data.begin(), Data.end());
    CurlManager::INSTANCE.doPost(url, contentType, value, ReturnCode);

    // LOG_DEBUG(std::string("SetHttpConfig: ReturnCode: '") + std::to_string(ReturnCode) + "'");
    return (200 == ReturnCode);
}

bool ESPixelStick::GetWsConfig(std::string const& SectionName, std::string const& key, nlohmann::json& Response) {
    spdlog::debug("GetWsConfig: Getting ESPSv4 HTTP config file");

    nlohmann::json newJson;
    newJson["cmd"]["get"] = SectionName;
    // LOG_DEBUG(std::string("GetWsConfig: cmd: ") + newJson.dump());
    _wsClient.Send(newJson.dump());
    std::string RawData = GetWSResponse();
    try {
        nlohmann::json ParsedData = nlohmann::json::parse(RawData);
        Response = ParsedData["get"][key];
    } catch (nlohmann::json::exception ex) {
        Response.clear();
        spdlog::debug(std::string("GetWsConfig: ERROR: Could not parse json: ") + RawData);
        spdlog::error(std::string("GetWsConfig: ERROR: Could not parse json: ") + ex.what());
    }

    // LOG_DEBUG(std::string("GetWsConfig: Response: ") + std::to_string(Response.Size()));
    return (0 != Response.size());
}

bool ESPixelStick::SetWsConfig(std::string const& SectionName, std::string const& key, nlohmann::json const & Data) {
    spdlog::debug("SetWsConfig: Setting ESPSv4 HTTP config file");

    nlohmann::json newJson;
    newJson["cmd"]["set"][SectionName] = Data;
    // LOG_DEBUG(std::string("SetWsConfig: cmd: ") + newJson.dump());
    _wsClient.Send(newJson.dump());

    std::string const RawData = GetWSResponse();
    // LOG_DEBUG(std::string("SetWsConfig: RawData: ") + RawData);
    nlohmann::json Response;
    try {
        nlohmann::json ParsedData = nlohmann::json::parse(RawData);
        nlohmann::json Response = ParsedData["cmd"];
    } catch (nlohmann::json::exception ex) {
        spdlog::debug(std::string("SetWsConfig: ERROR: Could not parse json: ") + RawData);
        spdlog::error(std::string("SetWsConfig: ERROR: Could not parse json: ") + ex.what());
    }

    std::string const returnValue = Response.get<std::string>();
    // LOG_DEBUG(std::string("SetWsConfig: returnValue: '") + returnValue + "'");
    bool result = returnValue == "OK";
    // LOG_DEBUG(std::string("SetWsConfig: result: ") + std::to_string(result));
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

std::string ESPixelStick::GetFromJSON(std::string const& section, std::string const& key, std::string const & json) const {
    // skip over the "G2" header or whatever

    try {
        for (int x = 0; x < json.size(); x++) {
            if (json[x] == '{' || json[x] == '[') {
                std::string config = json.substr(x);
                nlohmann::json origJson = nlohmann::json::parse(config);
                nlohmann::json val = origJson;
                if (section != "") {
                    val = origJson[section];
                }
                if (!val.contains(key)) {
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
    } catch (nlohmann::json::exception ex) {
        spdlog::error(std::string("GetFromJSON: ERROR: Could not parse json: ") + ex.what());
    }

    return "";
}

bool EspsPort::WriteConfigToJson(nlohmann::json& JsonConfig) {
    uint32_t NumItemsChanged = 0;
    nlohmann::json& CurrentPort = JsonConfig["channels"][PortId];

    if (std::to_string(CurrentPort["type"].get<int>()) != CurrentProtocolId)
    {
        NumItemsChanged++;
        CurrentPort["type"] = std::stoi(CurrentProtocolId);
    }
    nlohmann::json& CurrentProtocol = CurrentPort[CurrentProtocolId];

    NumItemsChanged += ProtocolsByName[CurrentProtocolName].WriteConfigToJson(CurrentProtocol);

    return NumItemsChanged != 0;
} // WriteConfigToJson

int EspsV4Protocol::WriteConfigToJson(nlohmann::json& JsonConfig)
{
    for (auto& [ElementName, ElementValue] : Settings)
    {
        if (ElementName == "type")
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
            try {
                temp = std::stod(ElementValue);
            } catch (const std::exception&) {
                temp = 0.0;
            }
            JsonConfig[ElementName] = temp;
            // LOG_DEBUG("float:ElementName: '" + ElementName + "' value: '" + std::to_string(JsonConfig[ElementName].get<float>()) + "'");
        }
        else if (JsonConfig[ElementName].is_number_integer())
        {
            JsonConfig[ElementName] = std::stoi(ElementValue);
        }
        else if (JsonConfig[ElementName].is_boolean())
        {
            JsonConfig[ElementName] = (ElementValue == "true" || ElementValue == "True");
        }
        else
        {
            spdlog::error("EspsV4Protocol:WriteConfigToJson: Encountered an unsupported field type in the esps protocol configuration for Element: " + ElementName);
            continue;
        }
    }

    return NumItemsChanged;
} // WriteConfigToJson

#pragma endregion

#pragma region Getters and Setters
bool ESPixelStick::SetInputUniverses(Controller* controller, wxWindow* parent) {
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
                spdlog::debug("ESPixelStick Inputs Upload: Success!!!");
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

bool EspsV4Protocol::GetSetting(std::string const& Name, std::string& value) {
    // 
    // LOG_DEBUG("EspsV4Protocol:GetSetting: Start");
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

    // LOG_DEBUG("EspsV4Protocol:GetSetting: Done");
    return Response;
} // GetSetting

bool EspsV4Protocol::PutSetting(std::string const& Name, std::string value, std::string const& DefaultValue) {
    // 
    // LOG_DEBUG("EspsV4Protocol:PutSetting: Start");
    bool Response = false;

    do // once
    {
        if (!Settings.contains(Name))
        {
            DisplayError(std::string("EspsV4Protocol::PutSetting: Could not update '") + Name + "' No such setting in target data set.");
            break;
        }

        if (value.empty())
        {
            // no value to set
            if (!IsFullxLightsControl)
            {
                // we do not have full control. Leave the existing value in place
                break;
            }

            // use the default value when we are in full control mode
            value = DefaultValue;
        }

        // value is accepted
        Response = true;

        if (value == Settings[Name])
        {
            // no need to resave an existing value
            break;
        }

        // LOG_DEBUG("EspsV4Protocol:PutSetting: Name: '" + Name + "' value: '" + value + "' json: '" + Settings[Name] + "'");
        Settings[Name] = value;
        NumItemsChanged++;

    } while (false);

    // LOG_DEBUG("EspsV4Protocol:GetSetting: Done");
    return Response;
} // PutSetting

bool EspsV4Protocol::PutSetting(std::string const& Name, int value, int DefaultValue) {
    // 
    // LOG_DEBUG("EspsV4Protocol:PutSetting: Start");
    bool Response = false;

    do // once
    {
        if (!Settings.contains(Name))
        {
            DisplayError(std::string("EspsV4Protocol::PutSetting: Could not update '") + Name + "' No such setting in target data set.");
            break;
        }

        if (-1 == value)
        {
            // no value to set
            if (!IsFullxLightsControl)
            {
                // we do not have full control. Leave the existing value in place
                break;
            }

            // use the default value when we are in full control mode
            value = DefaultValue;
        }

        // value is accepted
        Response = true;

        auto const sValue = std::to_string(value);
        if (sValue == Settings[Name])
        {
            // no need to resave an existing value
            break;
        }

        // LOG_DEBUG("EspsV4Protocol:PutSetting: Name: '" + Name + "' value: '" + sValue + "' json: '" + Settings[Name] + "'");
        Settings[Name] = sValue;
        NumItemsChanged++;

    } while (false);

    // LOG_DEBUG("EspsV4Protocol:GetSetting: Done");
    return Response;
} // PutSetting

bool EspsV4Protocol::PutSetting(std::string const& Name, float value, float DefaultValue) {
    // 
    // LOG_DEBUG("EspsV4Protocol:PutSetting: Start");
    bool Response = false;

    do // once
    {
        if (!Settings.contains(Name))
        {
            DisplayError(std::string("EspsV4Protocol::PutSetting: Could not update '") + Name + "' No such setting in target data set.");
            break;
        }

        if (-1.0f == value)
        {
            // no value to set
            if (!IsFullxLightsControl)
            {
                // we do not have full control. Leave the existing value in place
                break;
            }

            // use the default value when we are in full control mode
            value = DefaultValue;
        }

        // value is accepted
        Response = true;

        auto const sValue = std::to_string(value);
        if (sValue == Settings[Name])
        {
            // no need to resave an existing value
            break;
        }

        // LOG_DEBUG("EspsV4Protocol:PutSetting: Name: '" + Name + "' value: '" + sValue + "' json: '" + Settings[Name] + "'");
        Settings[Name] = sValue;
        NumItemsChanged++;

    } while (false);

    // LOG_DEBUG("EspsV4Protocol:GetSetting: Done");
    return Response;
} // PutSetting

void EspsV4Protocol::ParseV4Settings(std::string const& Id, const nlohmann::json& JsonConfig) {
    // LOG_DEBUG("EspsV4Protocol:ParseV4Config: Start");
    _Id = Id;

    // process the element in the protocol
    for (auto& [ElementId, Elementval] : JsonConfig.items()) {

        std::string FinalValue;

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
            spdlog::error("EspsV4Protocol:ParseV4Config: Encountered an unsupported field type in the esps protocol configuration for Element: " + ElementId);
            continue;
        }

        spdlog::debug("EspsPort:ParseV4Config: Id: " + ElementId + ", value: " + FinalValue);
        Settings[ElementId] = FinalValue;
    }

    _Name = Settings["type"];
    _Name = Lower(_Name);
    // LOG_DEBUG("EspsV4Protocol:ParseV4Config: Done");

} // ParseV4Settings

bool EspsPort::ParseV4Settings(const nlohmann::json& JsonConfig)
{
    // LOG_DEBUG("EspsPort:ParseV4Config: Start");

    bool Response = true;

    do // once
    {
        // process the port members
        for (auto& [ElementId, Elementval] : JsonConfig.items()) {
            spdlog::debug("EspsPort:ParseV4Config: ProtocolId: " + ElementId);

            // protocol IDs are a one or two character value
            if (3 > ElementId.length()) {
                // this is a protocol ID
                EspsV4Protocol Protocol;
                Protocol.ParseV4Settings(ElementId, JsonConfig[ElementId]);

                // add protocol to the list of protocols valid for this port
                auto ProtocolName = Lower(Protocol.Name());

                spdlog::debug("Adding protocol '" + ProtocolName + "' to port " + PortId);
                ProtocolsByName[ProtocolName] = Protocol;
                ProtocolIdToProtocolName[Protocol.Id()] = ProtocolName;
            } else if (ElementId == "type") {
                CurrentProtocolId = std::to_string(Elementval.get<int>());
            } else {
                // error getting port information
                spdlog::error("EspsPort:ParseV4Config: Could not parse data for port: " + ElementId);
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

    // LOG_DEBUG("EspsPort:ParseV4Config: Done");
    return Response;

} // ParseV4Settings

bool ESPixelStick::ParseV4Config(nlohmann::json& outputConfig) {
    // 
    // LOG_DEBUG("ParseV4Config: Start");

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
            // LOG_DEBUG("ParseV4Config: Portkey: " + Portkey);
            NewEspsPort.PortId = Portkey;

            NewEspsPort.ParseV4Settings(Ports[Portkey]);

            // add to the list of ports
            EspsConfig[NewEspsPort.PortId] = NewEspsPort;
        }

    } while (false);

    // LOG_DEBUG("ParseV4Config: End");
    return Response;

} // ParseV4Config

bool ESPixelStick::SetOutputsV4(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent)
{
    spdlog::debug("ESPixelStick Outputs Upload: Uploading to {}", _ip);

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);
    ControllerCaps * rules = ControllerCaps::GetControllerConfig(controller);
    bool success = cud.Check(rules, check);
    cud.Dump();
    spdlog::debug(check);

    do // once
    {
        if (!success)
        {
            // could not set up the data
            break;
        }

        bool changed = false;
        nlohmann::json outputConfig;

        GetOutputConfig(outputConfig);

        if (!ParseV4Config(outputConfig))
        {
            // no valid config present
            std::string msg = "ESPixelStick Outputs Upload: Could not parse config from ESPixelstick";
            DisplayError(msg);
            success = false;
            break;
        }

        for (int currentPortId = 0; currentPortId < cud.GetMaxPixelPort(); currentPortId++)
        {
            if (!cud.HasPixelPort(currentPortId + 1))
            {
                // not a valid port
                continue;
            }

            auto const s_currentPortId = std::to_string(currentPortId);
            EspsPort& CurrentEspsPort = EspsConfig[s_currentPortId];
            UDControllerPort* port = cud.GetControllerPixelPort(currentPortId + 1);

            if (!EspsConfig.contains(s_currentPortId))
            {
                // not a valid ESPS port
                CurrentEspsPort.Disable();
                continue;
            }

            auto const targetProtocolName = Lower(port->GetProtocol());
            auto const type = port->GetType();

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
            Protocol.SetIsFullxLightsControl(controller->IsFullxLightsControl());

            // bind protocol to the port
            CurrentEspsPort.CurrentProtocolId = Protocol.Id();
            CurrentEspsPort.CurrentProtocolName = Protocol.Name();

            int const numberOfChannels = port->Channels();

            UDControllerPortModel* model = port->GetModels().front();
            if (model)
            {
                int const channelsPerPixel = model->GetChannelsPerPixel();
                Protocol.PutSetting("pixel_count", INTROUNDUPDIV(numberOfChannels, channelsPerPixel), 0);

                auto colorOrder = Lower(model->GetColourOrder(channelsPerPixel > 3 ? "undefinedw" : "undefined"));
                if (!EspsV4ColorOrders.contains(colorOrder))
                {
                    // unsupported color order
                    CurrentEspsPort.Disable();
                    std::string msg = "ESPixelStick Outputs Upload: Color Order '" + colorOrder + "' not supported by ESPS V4";
                    DisplayError(msg);
                    return false;
                }

                Protocol.PutSetting("color_order", EspsV4ColorOrders[colorOrder], "rgb");
                Protocol.PutSetting("brightness", model->GetBrightness(-1), controller->GetDefaultBrightnessUnderFullControl());
                Protocol.PutSetting("gamma", model->GetGamma(-1.0F), controller->GetDefaultGammaUnderFullControl());
                Protocol.PutSetting("group_size", model->GetGroupCount(-1), 1);
                Protocol.PutSetting("prependnullcount", model->GetStartNullPixels(-1), 0);
                Protocol.PutSetting("appendnullcount", model->GetEndNullPixels(-1), 0);
                Protocol.PutSetting("zig_size", model->GetZigZag(-1), 1);

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

            auto const s_currentPortId = std::to_string( currentPortId);
            EspsPort& CurrentEspsPort = EspsConfig[s_currentPortId];

            if (!EspsConfig.contains(s_currentPortId))
            {
                // not a valid ESPS port
                CurrentEspsPort.Disable();
                continue;
            }

            UDControllerPort* port = cud.GetControllerSerialPort(currentPortId + 1);
            UDControllerPortModel* model = port->GetModels().front();

            if (!model) {
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

            Protocol.PutSetting("num_chan", port->Channels(), 0);
            Protocol.PutSetting("baudrate", model->GetModel()->GetControllerProtocolSpeed(), 57600);

            changed = CurrentEspsPort.WriteConfigToJson(outputConfig);

        } // end for each serial port on this controller

        if (changed)
        {
            if (SetOutputConfig(outputConfig))
            {
                success = true;
                spdlog::debug("ESPixelStick Outputs Upload: Success!!!");
            }
            else
            {
                success = false;
                spdlog::debug("ESPixelStick Outputs Upload: Failure!!!");
            }
        }
        else
        {
            spdlog::debug("ESPixelStick Outputs Upload: No Changes to upload");
            success = true;
        }
    } while (false);

    return success;
}

bool ESPixelStick::SetOutputsV3(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) {
    spdlog::debug("ESPixelStick Outputs Upload: Uploading to {}",_ip);

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);

    ControllerCaps * rules = ControllerCaps::GetControllerConfig(controller);
    bool success = cud.Check(rules, check);

    cud.Dump();
    spdlog::debug(check);

    try {
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
            // copy stuff over to act as defaults
            newJson["device"] = origJson["device"];
            newJson["mqtt"] = origJson["mqtt"];
            newJson["e131"] = origJson["e131"];
            newJson["pixel"] = origJson["pixel"];

            newJson["e131"]["universe"] = port->GetUniverse();
            newJson["e131"]["universe_limit"] = cud.GetFirstOutput()->GetChannels();
            newJson["e131"]["channel_start"] = port->GetUniverseStartChannel();
            newJson["e131"]["channel_count"] = port->Channels();
            newJson["e131"]["multicast"] = ((cud.GetFirstOutput()->GetIP() == "MULTICAST") ? true : false);

            UDControllerPortModel* s = port->GetModels().front();
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

            std::string message = newJson.dump(3, ' ', false, nlohmann::json::error_handler_t::replace);
            message = "S2" + message;

            if (_wsClient.Send(message)) {
                success = true;
                spdlog::debug("ESPixelStick Outputs Upload: Success!!!");
            }
            GetWSResponse();
        } else {
            DisplayError("Not uploaded due to errors.\n" + check);
        }
    } catch (nlohmann::json::exception ex) {
        spdlog::error(std::string("SetOutputsV3: ERROR: Could not parse json: ") + ex.what());
    }

    return success;
}
#pragma endregion
