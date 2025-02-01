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
#include "../UtilClasses.h"

class wxJSONValue;

class Experience : public BaseController
{

#pragma region Member Variables
    std::string _controllerType;
    std::string _controllerModel;
    int _firmwareMajor{ 0 };
    int _firmwareMinor{ 0 };
    int _firmwarePatch{ 0 };
    int _firmwareBuild{ 0 };
    int _modelYear{ 0 };
    int _numberOfPixelOutputs{ 0 };
    int _numberOfRemoteOutputs{ 0 };
    int _numberOfSerialOutputs{ 0 };
    bool _has_efuses{ false };
    std::string _opMode = "ddp";

#pragma endregion

#pragma region Private Functions
    [[nodiscard]] bool GetJSONData(std::string const& url, wxJSONValue& val) const;
    std::string PostJSONToURL(std::string const& url, wxJSONValue const& val) const;
    bool DecodeFirmwareInformation(wxString const& firmware);
    bool DecodeModelInformation(std::string const& model);
    [[nodiscard]] bool IsVersionAtLeast(int maj, int min, int patch) const;

    [[nodiscard]] std::string GetVersionStr() const {
        return std::to_string(_firmwareMajor) + "." +
               std::to_string(_firmwareMinor) + "." +
               std::to_string(_firmwarePatch) + "." + std::to_string(_firmwareBuild);
    }

    static std::string GetStateURL() { return "/api/state"; };
    static std::string GetConfigURL() { return "/api/config"; };
    [[nodiscard]] int GetNumberOfPixelOutputs() const { return _numberOfPixelOutputs; }
    [[nodiscard]] int GetNumberOfRemoteOutputs() const { return _numberOfRemoteOutputs; }
    [[nodiscard]] int GetNumberOfSerial() const { return _numberOfSerialOutputs; }

#pragma endregion

#pragma region Encode and Decode
    [[nodiscard]] int EncodeBrightness(int brightness) const;
    [[nodiscard]] int EncodeGamma(double gamma) const;
    [[nodiscard]] wxString EncodeColorOrder(std::string const& colorOrder) const;
#pragma endregion

public:
#pragma region Constructors and Destructors
    Experience(std::string const& ip, std::string const& fppProxy);
    virtual ~Experience() override {};
#pragma endregion

    [[nodiscard]] bool UploadSequence(std::string const& seq, std::string const& file, std::function<bool(int, std::string)> progress);

#pragma region Getters and Setters
#ifndef DISCOVERYONLY
    [[nodiscard]] int32_t SetInputUniverses(wxJSONValue& data, Controller* controller);
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
#endif
    [[nodiscard]] bool UsesHTTP() const override { return true; }

    [[nodiscard]] std::string GetFullName() const override {
        return _model + " v" + GetVersionStr();
    }
#pragma endregion
};
