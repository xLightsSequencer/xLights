#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <list>
#include <string>

#include "BaseController.h"
#include "WebSocketClient.h"
#include "ControllerUploadData.h"

class ESPixelStick : public BaseController
{
    #pragma region Member Variables
    WebSocketClient _wsClient;
    #pragma endregion

    #pragma region Private Functions
    std::string DecodeStringPortProtocol(std::string const& protocol) const;
    std::string DecodeSerialPortProtocol(std::string const& protocol) const;
    std::string DecodeSerialSpeed(std::string const& protocol) const;

    std::string GetFromJSON(std::string const& section, std::string const& key, std::string const& json) const;
    #pragma endregion

    bool SetOutputsV3(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent);
    bool SetOutputsV4(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent);


    std::string GetWSResponse();
public:
    #pragma region Constructors and Destructors
    ESPixelStick(const std::string& ip);
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
