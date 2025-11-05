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

#include <nlohmann/json.hpp>

#include "BaseController.h"
#include "WebSocketClient.h"
#include "ControllerUploadData.h"
#include "../utils/CurlManager.h"


class ESPixelStick : public BaseController
{
#pragma region Member Variables
private:

    WebSocketClient _wsClient;
    bool _UsingHttpConfig = false;

    #pragma endregion

    #pragma region Private Functions

    std::string DecodeStringPortProtocol(std::string const& protocol) const;
    std::string DecodeSerialPortProtocol(std::string const& protocol) const;
    std::string DecodeSerialSpeed(std::string const& protocol) const;

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
