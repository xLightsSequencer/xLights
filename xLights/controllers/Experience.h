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
    int _numberOfPixelOutputs{ 0 };
    int _numberOfRemoteOutputs{ 0 };
    int _numberOfSerialOutputs{ 0 };

#pragma endregion

#pragma region Private Functions
    bool GetJSONData(const std::string& url, wxJSONValue& val) const;
    std::string PostJSONToURL(const std::string& url, const wxJSONValue& val) const;

    static const std::string GetStateURL() { return "/api/state"; };
    static const std::string GetConfigURL() { return "/api/config"; };
    const int GetNumberOfPixelOutputs() { return _numberOfPixelOutputs; }
    const int GetNumberOfRemoteOutputs() { return _numberOfRemoteOutputs; }
    const int GetNumberOfSerial() { return _numberOfSerialOutputs; }

#pragma endregion

#pragma region Encode and Decode
    int EncodeBrightness(int brightness) const;
    int EncodeGamma(double gamma) const;
    wxString EncodeColorOrder(std::string const& colorOrder) const;
#pragma endregion

public:
#pragma region Constructors and Destructors
    Experience(const std::string& ip, const std::string& fppProxy);
    virtual ~Experience(){};
#pragma endregion

    bool UploadSequence(const std::string& seq, const std::string& file, std::function<bool(int, std::string)> progress);

#pragma region Getters and Setters
#ifndef DISCOVERYONLY
    int32_t SetInputUniverses(wxJSONValue& data, Controller* controller);
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
#endif
    virtual bool UsesHTTP() const override { return true; }
#pragma endregion
};
