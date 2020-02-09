#pragma once

#include <list>
#include "models/ModelManager.h"

#include "BaseController.h"

class Pixlite16 : public BaseController
{
public:
    struct Config
    {
        uint8_t _mac[6] = {0,0,0,0,0,0};
        int _modelNameLen = 0;
        std::string _modelName = "";
        uint8_t _hwRevision = 0;
        uint8_t _minAssistantVer[3] = {0,0,0};
        int _firmwareVersionLen = 0;
        std::string _firmwareVersion = "";
        uint8_t _brand = 0;
        uint8_t _currentIP[4] = {0,0,0,0};
        uint8_t _currentSubnetMask[4] = {255,255,255,0};
        uint8_t _dhcp = 0;
        uint8_t _staticIP[4] = { 0,0,0,0 };
        uint8_t _staticSubnetMask[4] = { 255,255,255,0 };
        uint8_t _protocol = 0;
        uint8_t _holdLastFrame = 0;
        uint8_t _simpleConfig = 0;
        uint16_t _maxPixelsPerOutput = 0;
        uint8_t _numOutputs = 0;
        uint8_t _realOutputs = 0;
        std::vector<uint16_t> _outputPixels;
        std::vector<uint16_t> _outputUniverse;
        std::vector<uint16_t> _outputStartChannel;
        std::vector<uint8_t> _outputNullPixels;
        std::vector<uint16_t> _outputZigZag;
        std::vector<uint8_t> _outputReverse;
        std::vector<uint8_t> _outputColourOrder;
        std::vector<uint16_t> _outputGrouping;
        std::vector<uint8_t> _outputBrightness;
        uint8_t _numDMX = 0;
        uint8_t _realDMX = 0;
        std::vector<uint8_t> _dmxOn;
        std::vector<uint16_t> _dmxUniverse;
        uint8_t _numDrivers = 0;
        std::vector<uint8_t> _driverType;
        std::vector<uint8_t> _driverSpeed;
        std::vector<uint8_t> _driverExpandable;
        int _driverNameLen = 0;
        std::vector<std::string> _driverName;
        uint8_t _currentDriver = 0;
        uint8_t _currentDriverType = 0;
        uint8_t _currentDriverSpeed = 0;
        uint8_t _currentDriverExpanded = 0;
        std::vector<uint8_t> _gamma = {0,0,0};
        int _nicknameLen = 0;
        std::string _nickname = "";
        uint16_t _temperature = 0;
        uint8_t _maxTargetTemp = 0;
        uint8_t _numBanks = 0;
        std::vector<uint8_t> _bankVoltage;
        uint8_t _testMode = 0;
        std::vector<uint8_t> _testParameters;
    };

protected:

    #pragma region Member Variables
    Config _config;
    int _protocolVersion = 0;
    #pragma endregion

    #pragma region Encode and Decode
    static int DecodeStringPortProtocol(std::string protocol);
    static int DecodeSerialOutputProtocol(std::string protocol);
    #pragma endregion

    #pragma region Private Functions
    static uint16_t Read16(uint8_t* data, int& pos);
    static void Write16(uint8_t* data, int& pos, int value);
    static void WriteString(uint8_t* data, int& pos, int len, const std::string& value);
    
    bool ParseV4Config(uint8_t* data);
    bool ParseV5Config(uint8_t* data);
    bool ParseV6Config(uint8_t* data);
    int PrepareV4Config(uint8_t* data) const;
    int PrepareV5Config(uint8_t* data) const;
    int PrepareV6Config(uint8_t* data) const;

    bool SendConfig(bool logresult = false) const;

    void DumpConfiguration() const;
    #pragma endregion

public:

    #pragma region Constructors and Destructors
    Pixlite16(const std::string& ip);
    ~Pixlite16() {}
    #pragma endregion

    #pragma region Getters and Setters
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent);
    virtual bool UsesHTTP() const override { return false; }
    #pragma endregion
};

