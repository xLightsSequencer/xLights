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

class WLEDOutput;

enum class WLEDType
{
    ESP8266,
    ESP32,
    QuinLEDDigQuadESP8266,
    QuinLEDDigQuadESP32
};

class WLED : public BaseController
{
    #pragma region Member Variables
    std::vector<WLEDOutput*> _pixelOutputs;
    WLEDType _controllerType{ WLEDType::ESP8266 };
    #pragma endregion

    #pragma region Private Functions
    void UpdatePortData(WLEDOutput* pd, UDControllerPort* stringData, int startNumber) const;

    WLEDOutput* FindPortData(int port);

    bool ParseOutputWebpage(const std::string& page);

    WLEDOutput* ExtractOutputData(const std::string& page, int port);

    int ExtractIntFromPage(const std::string& page, const std::string& parameter, int defaultValue = 0);
    bool ExtractBoolFromPage(const std::string& page, const std::string& parameter, bool defaultValue = false);
    std::string ExtractStringFromPage(const std::string& page, const std::string& parameter, std::string defaultValue = "");

    int EncodeColorOrder(const std::string& colorOrder) const;
    bool EncodeDirection(const std::string& direction) const;
    int EncodeStringPortProtocol(const std::string& protocol) const;

    void UploadPixelOutputs(bool& worked, int totalPixelCount);

    const std::string GetFirmware() { return _version; }
    const int GetNumberOfOutputs();
    const int GetNumberOfSerial() { return 0; }

    const uint8_t GetOutputPin(int port);
    #pragma endregion

    #pragma region Private Static Functions
    static const std::string GetInfoURL() { return"/json/info"; };
    static const std::string GetSyncPageURL() { return"/settings/sync"; };
    static const std::string GetLEDPageURL() { return"/settings/leds"; };
    #pragma endregion

public:
    
    #pragma region Constructors and Destructors
    WLED(const std::string& ip, const std::string &fppProxy);
    virtual ~WLED();
    #pragma endregion
    
    #pragma region Getters and Setters
#ifndef DISCOVERYONLY
    virtual bool SetInputUniverses(ControllerEthernet* controller, wxWindow* parent) override;
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) override;
#endif
    virtual bool UsesHTTP() const override { return true; }
    #pragma endregion
};
