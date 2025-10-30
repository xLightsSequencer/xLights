#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <list>
#include <string>
#include <map>

#include <nlohmann/json.hpp>

#include "BaseController.h"
#include "WebSocketClient.h"
#include "ControllerUploadData.h"
#include "../utils/CurlManager.h"

class EspsV4Protocol
{
public:
    void ParseV4Settings(std::string const& Id, const nlohmann::json& JsonConfig);
    bool GetSetting(std::string const& Name, std::string& value);
    bool PutSetting(std::string const& Name, std::string value, std::string const& DefaultValue);
    bool PutSetting(std::string const& Name, int value, int DefaultValue);
    bool PutSetting(std::string const& Name, float value, float DefaultValue);
    int WriteConfigToJson(nlohmann::json& JsonConfig);
    inline bool IsPixel() { return Settings.contains("color_order"); }
    inline std::string Name() { return _Name; }
    inline std::string Id() { return _Id; }
    void SetIsFullxLightsControl(bool value) {IsFullxLightsControl = value;}

private:
    std::string _Id;
    std::string _Name;
    std::map<std::string, std::string> Settings;
    uint32_t NumItemsChanged = 0;
    bool IsFullxLightsControl = false;
};

class EspsPort
{
public:
    std::string PortId;
    std::string CurrentProtocolId = "0";
    std::string CurrentProtocolName = "disabled";
    std::string DisabledId = "0";
    std::string DisabledName = "disabled";
    std::map<std::string, EspsV4Protocol> ProtocolsByName;
    std::map<std::string, std::string> ProtocolIdToProtocolName;

    bool ParseV4Settings(const nlohmann::json& JsonConfig);
    bool WriteConfigToJson(nlohmann::json& JsonConfig);
    void Disable() {
        CurrentProtocolId = DisabledId;
        CurrentProtocolName = DisabledName;
    }

private:

};

class ESPixelStick : public BaseController
{
#pragma region Member Variables
private:

    WebSocketClient _wsClient;
    bool _UsingHttpConfig = false;

    std::map<std::string, EspsPort> EspsConfig;

    #pragma endregion

    #pragma region Private Functions

    std::string GetFromJSON(std::string const& section, std::string const& key, std::string const& json) const;
    bool CheckWsConnection();
    bool CheckHTTPconnection();

    bool GetAdminInformation(nlohmann::json& Result);
    bool GetInputConfig(nlohmann::json& Result);
    bool GetOutputConfig(nlohmann::json& Result);
    bool GetHttpConfig(std::string const& FileName, std::string const& key, nlohmann::json& Result);
    bool GetWsConfig(std::string const& FileName, std::string const& key, nlohmann::json& Result);

    bool SetInputConfig(nlohmann::json& Data);
    bool SetOutputConfig(nlohmann::json& Data);
    bool SetHttpConfig(std::string const& FileName, std::string const& key, nlohmann::json const& Data);
    bool SetWsConfig(std::string const& FileName, std::string const& key, nlohmann::json const& Data);

    bool SetOutputsV3(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent);
    bool SetOutputsV4(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent);

    bool ParseV4Config(nlohmann::json& outputConfig);
    std::string GetWSResponse();

#pragma endregion

public:
    #pragma region Constructors and Destructors
    ESPixelStick(const std::string& ip, const std::string &proxy);
    virtual ~ESPixelStick() {}
    #pragma endregion

    #pragma region Getters and Setters
#ifndef DISCOVERYONLY
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
    virtual bool SetInputUniverses(Controller* controller, wxWindow* parent) override;
    virtual bool UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
#endif

    virtual bool UsesHTTP() const override { return false; }
    #pragma endregion
};
