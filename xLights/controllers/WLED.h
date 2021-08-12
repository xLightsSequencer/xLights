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
#include "ControllerUploadData.h"

class wxJSONValue;
struct WLEDOutput;

class WLED : public BaseController
{
    #pragma region Member Variables
    std::vector<WLEDOutput*> _pixelOutputs;
    int _vid;

    #pragma endregion

    #pragma region Private Functions
    bool SetupInput(ControllerEthernet* controller, wxJSONValue &jsonVal);
    void UpdatePortData(WLEDOutput* pd, UDControllerPort* stringData, int startNumber) const;

    WLEDOutput* FindPortData(int port);

    bool ParseOutputJSON(wxJSONValue const& jsonVal, int maxPort, ControllerCaps* caps);

    WLEDOutput* ExtractOutputJSON(wxJSONValue const& jsonVal, int port, ControllerCaps* caps);

    int EncodeColorOrder(const std::string& colorOrder) const;
    bool EncodeDirection(const std::string& direction) const;
    int EncodeStringPortProtocol(const std::string& protocol) const;

    void UpdatePixelOutputs(bool& worked, int totalPixelCount, wxJSONValue& jsonVal);

    bool PostJSON(wxJSONValue const& jsonVal);

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
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) override;
#endif
    virtual bool UsesHTTP() const override { return true; }
    #pragma endregion
};
