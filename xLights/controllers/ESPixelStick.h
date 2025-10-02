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
    void ParseV4Settings(wxString Id, nlohmann::json & JsonConfig);
    bool GetSetting(wxString Name, wxString & value);
    bool PutSetting(wxString Name, const wxString & value);
    int WriteConfigToJson(nlohmann::json& JsonConfig);
    inline bool IsPixel() { return Settings.contains("color_order"); }
    inline wxString Name() { return _Name; }
    inline wxString Id() { return _Id; }

private:
    wxString _Id;
    wxString _Name;
    std::map<wxString, wxString> Settings;
    uint32_t NumItemsChanged = 0;
};

class EspsPort
{
public:
    wxString PortId;
    wxString CurrentProtocolId = "0";
    wxString CurrentProtocolName = "disabled";
    wxString DisabledId = "0";
    wxString DisabledName = "disabled";
    std::map<wxString, EspsV4Protocol> ProtocolsByName;
    std::map<wxString, wxString> ProtocolIdToProtocolName;

    bool ParseV4Settings(nlohmann::json& JsonConfig);
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
    bool GetHttpConfig(std::string FileName, std::string key, nlohmann::json& Result);
    bool GetWsConfig(std::string FileName, std::string key, nlohmann::json& Result);

    bool SetInputConfig(nlohmann::json& Data);
    bool SetOutputConfig(nlohmann::json& Data);
    bool SetHttpConfig(std::string FileName, std::string key, nlohmann::json& Data);
    bool SetWsConfig(std::string FileName, std::string key, nlohmann::json& Data);

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
