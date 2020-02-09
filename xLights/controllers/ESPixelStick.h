#pragma once 

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
    std::string DecodeStringPortProtocol(std::string protocol);
    std::string DecodeSerialPortProtocol(std::string protocol);
    std::string DecodeSerialSpeed(std::string protocol);

    std::string GetFromJSON(std::string section, std::string key, std::string json);
    #pragma endregion

public:
    #pragma region Constructors and Destructors
    ESPixelStick(const std::string& ip);
    virtual ~ESPixelStick() {}
    #pragma endregion

    #pragma region Getters and Setters
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) override;
    virtual bool UsesHTTP() const override { return false; }
    #pragma endregion
};
