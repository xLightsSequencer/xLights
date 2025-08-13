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

#include <vector>
#include <string>

#include <nlohmann/json.hpp>

#include "BaseController.h"
#include "ControllerUploadData.h"

struct WLEDOutput;

class WLED : public BaseController
{
    #pragma region Member Variables
    std::vector<WLEDOutput*> _pixelOutputs;
    int _vid{ 0 };

    #pragma endregion

    #pragma region Private Functions
    bool SetupInput(Controller* controller, nlohmann::json& jsonVal, bool rgbw);
    void UpdatePortData(WLEDOutput* pd, UDControllerPort* stringData, int startNumber, bool& rgbw) const;

    WLEDOutput* FindPortData(int port);

    bool ParseOutputJSON(nlohmann::json const& jsonVal, int maxPort, ControllerCaps* caps);

    WLEDOutput* ExtractOutputJSON(nlohmann::json const& jsonVal, int port, ControllerCaps* caps);

    int EncodeColorOrder(const std::string& colorOrder) const;
    bool EncodeDirection(const std::string& direction) const;
    int EncodeStringPortProtocol(const std::string& protocol) const;

    void UpdatePixelOutputs(bool& worked, int totalPixelCount, nlohmann::json& jsonVal);

    bool PostJSON(nlohmann::json const& jsonVal);

    const std::string GetFirmware() { return _version; }

    const uint8_t GetOutputPin( int port, ControllerCaps* caps);
    #pragma endregion

    #pragma region Private Static Functions
    static const std::string GetInfoURL() { return"/json/info"; };
    static const std::string GetCfgURL() { return"/json/cfg"; };
    #pragma endregion

public:
    
    #pragma region Constructors and Destructors
    WLED(const std::string& ip, const std::string &fppProxy);
    virtual ~WLED();
    #pragma endregion
    
    #pragma region Getters and Setters
#ifndef DISCOVERYONLY
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
#endif
    virtual bool UsesHTTP() const override { return true; }
    #pragma endregion
};
