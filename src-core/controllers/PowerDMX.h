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

#include <array>
#include <list>
#include <memory>
#include <string>

#include "BaseController.h"
#include "ControllerUploadData.h"

#include <nlohmann/json.hpp>

class PowerDMX : public BaseController {
#pragma region Member Variables
    int _numberOfPixelOutputs{ 0 };

#pragma endregion

#pragma region Private Functions
    [[nodiscard]] bool GetJSONData(std::string const& url, nlohmann::json& val) const;
    std::string PostJSONToURL(std::string const& url, nlohmann::json const& val) const;

    static std::string GetStatusURL() {
        return "/api/powerdmx/status";
    };

    static std::string GetConfigURL() {
        return "/api/powerdmx/config";
    };

    static std::string GetUploadURL() {
        return "/api/powerdmx/upload";
    };

    [[nodiscard]] int GetNumberOfPixelOutputs() const {
        return _numberOfPixelOutputs;
    }


#pragma endregion

#pragma region Encode and Decode
    [[nodiscard]] int EncodeBrightness(int brightness) const;
    [[nodiscard]] int EncodeGamma(double gamma) const;
    [[nodiscard]] std::string EncodeColorOrder(std::string const& colorOrder) const;
#pragma endregion

public:
#pragma region Constructors and Destructors
    PowerDMX(std::string const& ip, std::string const& fppProxy);
    virtual ~PowerDMX() override {};
#pragma endregion

    [[nodiscard]] bool UploadSequence(std::string const& seq, std::string const& file, std::function<bool(int, std::string)> progress);

#pragma region Getters and Setters
#ifndef DISCOVERYONLY
    [[nodiscard]] int32_t SetInputUniverses(nlohmann::json& data, Controller* controller);
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, UICallbacks* ui) override;
#endif
    [[nodiscard]] bool UsesHTTP() const override {
        return true;
    }

    [[nodiscard]] std::string GetFullName() const override {
        return _model + " v" + _version;
    }
#pragma endregion
};
