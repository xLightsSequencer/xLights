#include "Pixlite16.h"
#include <wx/msgdlg.h>
#include <wx/regex.h>
#include "models/Model.h"
#include <log4cpp/Category.hh>
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "UtilFunctions.h"
#include "ControllerUploadData.h"
#include "ControllerRegistry.h"

class PixLite16ControllerRules : public ControllerRules
{
    int _maxChannelsPerOutput = 0;
    int _pixelPorts = 0;
    int _dmxPorts = 0;

public:
    PixLite16ControllerRules(Pixlite16::Config& config) : ControllerRules()
    {
        _maxChannelsPerOutput = config._maxPixelsPerOutput * 3;
        _pixelPorts = config._realOutputs;
        _dmxPorts = config._realDMX;
    }

    PixLite16ControllerRules(int pixelsPerOutput, int pixelPorts, int dmxPorts) : ControllerRules()
    {
        _maxChannelsPerOutput = pixelsPerOutput * 3;
        _pixelPorts = pixelPorts;
        _dmxPorts = dmxPorts;
    }
    virtual ~PixLite16ControllerRules() {}
    virtual const std::string GetControllerId() const override {
        return std::to_string(_pixelPorts) + std::string((_maxChannelsPerOutput > 2040) ? " MkII" : "");
    }
    virtual const std::string GetControllerManufacturer() const override {
        return "PixLite";
    }

    virtual bool SupportsLEDPanelMatrix() const override {
        return false;
    }
    virtual int GetMaxPixelPortChannels() const override
    {
        return _maxChannelsPerOutput;
    }
    virtual int GetMaxPixelPort() const override { return _pixelPorts; }
    virtual int GetMaxSerialPortChannels() const override { return 512; }
    virtual int GetMaxSerialPort() const override { return _dmxPorts; }
    virtual bool IsValidPixelProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        return (p == "ws2811" || 
            p == "tm1803" ||
            p == "tm1804" ||
            p == "tm1809" ||
            p == "tls3001" || 
            p == "lpd6803" ||
            p == "sm16716" ||
            p == "ws2801" ||
            p == "mb16020" ||
            p == "my9231" ||
            p == "apa102" ||
            p == "my9221" ||
            p == "sk6812" ||
            p == "ucs1903");
    }
    virtual bool IsValidSerialProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        return (p == "dmx");
    }
    virtual bool SupportsMultipleProtocols() const override { return false; }
    virtual bool SupportsSmartRemotes() const override { return false; }
    virtual bool SupportsMultipleInputProtocols() const override { return false; }
    virtual bool AllUniversesSameSize() const override { return false; }
    virtual std::set<std::string> GetSupportedInputProtocols() const override {
        std::set<std::string> res = {"E131", "ARTNET"};
        return res;
    };
    virtual bool UniversesMustBeSequential() const override { return true; }

    virtual bool SingleUpload() const override { return true; }
};

static std::vector<PixLite16ControllerRules> CONTROLLER_TYPE_MAP = {
    PixLite16ControllerRules(680, 4, 1),
    PixLite16ControllerRules(340, 16, 4),
    PixLite16ControllerRules(1020, 4, 1),
    PixLite16ControllerRules(1020, 16, 4)
};

bool Pixlite16::ParseV4Config(uint8_t* data)
{
    int pos = 12;
    char buffer[256];

    _config._maxPixelsPerOutput = 340;
    _config._modelNameLen = 20;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], _config._modelNameLen);
    _config._modelName = buffer;
    pos += _config._modelNameLen;

    memcpy(_config._currentIP, &data[pos], sizeof(_config._currentIP));
    pos += sizeof(_config._currentIP);
    _config._dhcp = data[pos++]; // I cant find this

    pos++; // unused

    _config._nicknameLen = 40;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], _config._nicknameLen);
    pos += _config._nicknameLen;
    _config._nickname = buffer;

    _config._firmwareVersionLen = 20;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], _config._firmwareVersionLen);
    pos += _config._firmwareVersionLen;
    _config._firmwareVersion = buffer;

    memcpy(_config._mac, &data[pos], sizeof(_config._mac));
    pos += sizeof(_config._mac);

    _config._temperature = Read16(data, pos);

    _config._numOutputs = 16;
    if (_config._modelName.find("16") != std::string::npos)
    {
        _config._realOutputs = 16;
    }
    else
    {
        _config._realOutputs = 4;
    }
    _config._outputPixels.resize(_config._numOutputs);
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputPixels[i] = Read16(data, pos); }

    _config._protocol = data[pos++];

    pos++; // unused
    pos++; // unused

    memcpy(_config._staticIP, &data[pos], sizeof(_config._staticIP));
    pos += sizeof(_config._staticIP);

    memcpy(_config._staticSubnetMask, &data[pos], sizeof(_config._staticSubnetMask));
    pos += sizeof(_config._staticSubnetMask);

    pos++; // unused
    pos++; // unused

    pos++; // 0x00 static universe - must be 0
    pos++; // 0x00
    pos++; // 0x00 static start channel - must be 0
    pos++; // 0x00

    _config._currentDriver = data[pos++];

    _config._gamma.resize(3);
    for (int i = 0; i < _config._gamma.size(); i++) { _config._gamma[i] = data[pos++]; }

    _config._numBanks = 2;
    _config._bankVoltage.resize(_config._numBanks);
    for (int i = 0; i < _config._numBanks; i++) { _config._bankVoltage[i] = Read16(data, pos); }
    
    _config._outputUniverse.resize(_config._numOutputs);
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputUniverse[i] = Read16(data, pos); }

    _config._outputStartChannel.resize(_config._numOutputs);
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputStartChannel[i] = Read16(data, pos); }

    _config._outputNullPixels.resize(_config._numOutputs);
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputNullPixels[i] = data[pos++]; }

    _config._outputZigZag.resize(_config._numOutputs);
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputZigZag[i] = Read16(data, pos); }

    _config._outputReverse.resize(_config._numOutputs);
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputReverse[i] = data[pos++]; }

    uint8_t rgb = data[pos++]; // 0xff turn on advanced RGB orders - if not FF then make it FF can copy the value to advanced
    pos++; // unused
    pos++; // unused

    _config._numDMX = 4;
    if (_config._modelName.find("16") != std::string::npos)
    {
        _config._realDMX = 4;
    }
    else
    {
        _config._realDMX = 1;
    }
    _config._dmxOn.resize(_config._numDMX);
    _config._dmxUniverse.resize(_config._numDMX);
    for (int i = 0; i < _config._numDMX; i++) { _config._dmxOn[i] = data[pos++]; }
    for (int i = 0; i < _config._numDMX; i++) { _config._dmxUniverse[i] = Read16(data, pos); }

    _config._hwRevision = data[pos++]; 

    _config._currentDriverSpeed = data[pos++];

    _config._outputColourOrder.resize(_config._numOutputs);
    if (rgb != 0xff)
    {
        for (int i = 0; i < _config._numOutputs; i++) { _config._outputColourOrder[i] = rgb; }
        pos += _config._numOutputs;
    }
    else
    {
        for (int i = 0; i < _config._numOutputs; i++) { _config._outputColourOrder[i] = data[pos++]; }
    }

    _config._outputGrouping.resize(_config._numOutputs);
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputGrouping[i] = Read16(data, pos); }

    _config._outputBrightness.resize(_config._numOutputs);
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputBrightness[i] = data[pos++]; }

    _config._holdLastFrame = data[pos++];
    _config._maxTargetTemp = data[pos++];

    memcpy(_config._currentSubnetMask, &data[pos], sizeof(_config._currentSubnetMask));
    pos += sizeof(_config._currentSubnetMask);

    _config._currentDriverExpanded = data[pos++];
    
    memcpy(_config._minAssistantVer, &data[pos], sizeof(_config._minAssistantVer));
    pos += sizeof(_config._minAssistantVer);

    _config._testMode = data[pos++];

    _config._currentDriverType = data[pos++];

    pos++; // 0xff
    pos++; // 0xff
    pos++; // 0xff
    pos++; // 0xff

    return true;
}

uint16_t Pixlite16::Read16(uint8_t* data, int& pos)
{
    uint16_t res = (static_cast<uint16_t>(data[pos]) << 8) + data[pos + 1];
    pos += 2;
    return res;
}

void Pixlite16::Write16(uint8_t* data, int& pos, int value)
{
    data[pos++] = (value & 0xFF00) >> 8;
    data[pos++] = value & 0xFF;
}

void Pixlite16::WriteString(uint8_t* data, int& pos, int len, const std::string& value)
{
    memset(&data[pos], 0x00, len);
    strncpy(reinterpret_cast<char*>(&data[pos]), static_cast<const char *>(value.c_str()), std::min(len, static_cast<int>(value.length())));
    pos += len;
}

bool Pixlite16::ParseV5Config(uint8_t* data)
{
    int pos = 12;
    char buffer[256];
    memcpy(_config._mac, &data[pos], sizeof(_config._mac));
    pos += 6;

    _config._modelNameLen = data[pos++];
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], _config._modelNameLen);
    _config._modelName = buffer;
    pos += _config._modelNameLen;

    _config._hwRevision = data[pos++];
    memcpy(_config._minAssistantVer, &data[pos], sizeof(_config._minAssistantVer));
    pos += sizeof(_config._minAssistantVer);

    _config._firmwareVersionLen = 20;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], _config._firmwareVersionLen);
    pos += _config._firmwareVersionLen;
    _config._firmwareVersion = buffer;

    _config._brand = data[pos++];

    memcpy(_config._currentIP, &data[pos], sizeof(_config._currentIP));
    pos += sizeof(_config._currentIP);

    memcpy(_config._currentSubnetMask, &data[pos], sizeof(_config._currentSubnetMask));
    pos += sizeof(_config._currentSubnetMask);

    _config._dhcp = data[pos++];

    memcpy(_config._staticIP, &data[pos], sizeof(_config._staticIP));
    pos += sizeof(_config._staticIP);

    memcpy(_config._staticSubnetMask, &data[pos], sizeof(_config._staticSubnetMask));
    pos += sizeof(_config._staticSubnetMask);

    _config._protocol = data[pos++];
    _config._holdLastFrame = data[pos++];
    _config._simpleConfig = data[pos++];
    _config._maxPixelsPerOutput = Read16(data, pos);
    _config._numOutputs = data[pos++];
    _config._realOutputs = _config._numOutputs;
    _config._outputPixels.resize(_config._numOutputs);
    _config._outputUniverse.resize(_config._numOutputs);
    _config._outputStartChannel.resize(_config._numOutputs);
    _config._outputNullPixels.resize(_config._numOutputs);
    _config._outputZigZag.resize(_config._numOutputs);
    _config._outputReverse.resize(_config._numOutputs);
    _config._outputColourOrder.resize(_config._numOutputs);
    _config._outputGrouping.resize(_config._numOutputs);
    _config._outputBrightness.resize(_config._numOutputs);
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputPixels[i] = Read16(data, pos); }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputUniverse[i] = Read16(data, pos); }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputStartChannel[i] = Read16(data, pos); }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputNullPixels[i] = data[pos++]; }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputZigZag[i] = Read16(data, pos); }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputReverse[i] = data[pos++]; }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputColourOrder[i] = data[pos++]; }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputGrouping[i] = Read16(data, pos); }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputBrightness[i] = data[pos++]; }

    _config._numDMX = data[pos++];
    _config._realDMX = _config._numDMX;
    _config._dmxOn.resize(_config._numDMX);
    _config._dmxUniverse.resize(_config._numDMX);
    for (int i = 0; i < _config._numDMX; i++) { _config._dmxOn[i] = data[pos++]; }
    for (int i = 0; i < _config._numDMX; i++) { _config._dmxUniverse[i] = Read16(data ,pos); }

    _config._numDrivers = data[pos++];
    _config._driverNameLen = data[pos++];
    _config._driverType.resize(_config._numDrivers);
    _config._driverSpeed.resize(_config._numDrivers);
    _config._driverExpandable.resize(_config._numDrivers);
    _config._driverName.resize(_config._numDrivers);
    for (int i = 0; i < _config._numDrivers; i++) { _config._driverType[i] = data[pos++]; }
    for (int i = 0; i < _config._numDrivers; i++) { _config._driverSpeed[i] = data[pos++]; }
    for (int i = 0; i < _config._numDrivers; i++) { _config._driverExpandable[i] = data[pos++]; }
    for (int i = 0; i < _config._numDrivers; i++) {
        memset(buffer, 0x00, sizeof(buffer));
        memcpy(buffer, &data[pos], _config._driverNameLen);
        pos += _config._driverNameLen;
        _config._driverName[i] = buffer;
    }

    _config._currentDriver = data[pos++];
    _config._currentDriverType = data[pos++];
    _config._currentDriverSpeed = data[pos++];
    _config._currentDriverExpanded = data[pos++];
    _config._gamma.resize(3);
    for (int i = 0; i < _config._gamma.size(); i++) { _config._gamma[i] = data[pos++]; }

    _config._nicknameLen = 40;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], _config._nicknameLen);
    pos += _config._nicknameLen;
    _config._nickname = buffer;

    _config._temperature = Read16(data, pos);
    _config._maxTargetTemp = data[pos++];

    _config._numBanks = data[pos++];
    _config._bankVoltage.resize(_config._numBanks);
    for (int i = 0; i < _config._numBanks; i++) { _config._bankVoltage[i] = Read16(data, pos); }

    _config._testMode = data[pos++];

    //_testParameters not in v5

    return true;
}

bool Pixlite16::ParseV6Config(uint8_t* data)
{
    int pos = 13;
    char buffer[256];
    memcpy(_config._mac, &data[pos], sizeof(_config._mac));
    pos += 6;

    _config._modelNameLen = data[pos++];
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], _config._modelNameLen);
    _config._modelName = buffer;
    pos += _config._modelNameLen;

    _config._hwRevision = data[pos++];
    memcpy(_config._minAssistantVer, &data[pos], sizeof(_config._minAssistantVer));
    pos += sizeof(_config._minAssistantVer);

    _config._firmwareVersionLen = data[pos++];
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos+1], _config._firmwareVersionLen);
    _config._firmwareVersion = buffer;
    pos += _config._firmwareVersionLen;

    _config._brand = data[pos++];

    memcpy(_config._currentIP, &data[pos], sizeof(_config._currentIP));
    pos += sizeof(_config._currentIP);

    memcpy(_config._currentSubnetMask, &data[pos], sizeof(_config._currentSubnetMask));
    pos += sizeof(_config._currentSubnetMask);

    _config._dhcp = data[pos++];

    memcpy(_config._staticIP, &data[pos], sizeof(_config._staticIP));
    pos += sizeof(_config._staticIP);

    memcpy(_config._staticSubnetMask, &data[pos], sizeof(_config._staticSubnetMask));
    pos += sizeof(_config._staticSubnetMask);

    _config._protocol = data[pos++];
    _config._holdLastFrame = data[pos++];
    _config._simpleConfig = data[pos++];
    _config._maxPixelsPerOutput = Read16(data, pos);
    _config._numOutputs = data[pos++];
    _config._realOutputs = _config._numOutputs;
    _config._outputPixels.resize(_config._numOutputs);
    _config._outputUniverse.resize(_config._numOutputs);
    _config._outputStartChannel.resize(_config._numOutputs);
    _config._outputNullPixels.resize(_config._numOutputs);
    _config._outputZigZag.resize(_config._numOutputs);
    _config._outputReverse.resize(_config._numOutputs);
    _config._outputColourOrder.resize(_config._numOutputs);
    _config._outputGrouping.resize(_config._numOutputs);
    _config._outputBrightness.resize(_config._numOutputs);
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputPixels[i] = Read16(data, pos); }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputUniverse[i] = Read16(data, pos); }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputStartChannel[i] = Read16(data, pos); }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputNullPixels[i] = data[pos++]; }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputZigZag[i] = Read16(data, pos); }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputReverse[i] = data[pos++]; }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputColourOrder[i] = data[pos++]; }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputGrouping[i] = Read16(data, pos); }
    for (int i = 0; i < _config._numOutputs; i++) { _config._outputBrightness[i] = data[pos++]; }

    _config._numDMX = data[pos++];
    _config._realDMX = _config._numDMX;
    _config._dmxOn.resize(_config._numDMX);
    _config._dmxUniverse.resize(_config._numDMX);
    for (int i = 0; i < _config._numDMX; i++) { _config._dmxOn[i] = data[pos++]; }
    for (int i = 0; i < _config._numDMX; i++) { _config._dmxUniverse[i] = Read16(data, pos); }

    _config._numDrivers = data[pos++];
    _config._driverNameLen = data[pos++];
    _config._driverType.resize(_config._numDrivers);
    _config._driverSpeed.resize(_config._numDrivers);
    _config._driverExpandable.resize(_config._numDrivers);
    _config._driverName.resize(_config._numDrivers);
    for (int i = 0; i < _config._numDrivers; i++) { _config._driverType[i] = data[pos++]; }
    for (int i = 0; i < _config._numDrivers; i++) { _config._driverSpeed[i] = data[pos++]; }
    for (int i = 0; i < _config._numDrivers; i++) { _config._driverExpandable[i] = data[pos++]; }
    for (int i = 0; i < _config._numDrivers; i++) {
        memset(buffer, 0x00, sizeof(buffer));
        memcpy(buffer, &data[pos], _config._driverNameLen);
        pos += _config._driverNameLen;
        _config._driverName[i] = buffer;
    }

    _config._currentDriver = data[pos++];
    _config._currentDriverType = data[pos++];
    _config._currentDriverSpeed = data[pos++];
    _config._currentDriverExpanded = data[pos++];
    _config._gamma.resize(4);
    for (int i = 0; i < _config._gamma.size(); i++) { _config._gamma[i] = data[pos++]; }

    _config._nicknameLen = 40;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], _config._nicknameLen);
    pos += _config._nicknameLen;
    _config._nickname = buffer;

    _config._temperature = Read16(data, pos);
    _config._maxTargetTemp = data[pos++];

    _config._numBanks = data[pos++];
    _config._bankVoltage.resize(_config._numBanks);
    for (int i = 0; i < _config._numBanks; i++) { _config._bankVoltage[i] = Read16(data, pos); }

    _config._testMode = data[pos++];

    _config._testParameters.resize(4);
    for (int i = 0; i < _config._testParameters.size(); i++) { _config._testParameters[i] = data[pos++]; }

    return true;
}

int Pixlite16::PrepareV4Config(uint8_t* data) const
{
    int pos = 0;

    data[pos++] = 'A';
    data[pos++] = 'd';
    data[pos++] = 'v';
    data[pos++] = 'a';
    data[pos++] = 't';
    data[pos++] = 'e';
    data[pos++] = 'c';
    data[pos++] = 'h';
    data[pos++] = 0x00;
    Write16(data, pos, 5);
    data[pos++] = 0x02; 

    // Not sure why it insists on dropping the LR but if you dont it wont upload
    std::string mn = _config._modelName;
    if (EndsWith(_config._modelName, " LR"))
    {
        mn = _config._modelName.substr(0, _config._modelName.size() - 3);
    }
    else if (StartsWith(_config._modelName, "PixCon"))
    {
        mn = "PixLite" + _config._modelName.substr(6);
    }

    WriteString(data, pos, _config._modelNameLen, mn);
    data[pos++] = _config._dhcp;
    WriteString(data, pos, _config._nicknameLen, _config._nickname);
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputPixels[i]); }
    data[pos++] = _config._protocol;
    pos++; // unused
    pos++; // unused
    memcpy(&data[pos], _config._staticIP, sizeof(_config._staticIP));
    pos += sizeof(_config._staticIP);
    memcpy(&data[pos], _config._staticSubnetMask, sizeof(_config._staticSubnetMask));
    pos += sizeof(_config._staticSubnetMask);
    pos++; // unused
    pos++; // unused
    pos++; // no static universe
    pos++; // 
    pos++; // no static start channel
    pos++; // 
    data[pos++] = _config._currentDriver;

    for (auto g : _config._gamma) { data[pos++] = g; }
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputUniverse[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputStartChannel[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { data[pos++] = _config._outputNullPixels[i]; }
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputZigZag[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { data[pos++] = _config._outputReverse[i]; }
    data[pos++] = 0xFF;
    pos++; // unused
    pos++; // unused
    for (int i = 0; i < _config._numDMX; i++) { data[pos++] = _config._dmxOn[i]; }
    for (int i = 0; i < _config._numDMX; i++) { Write16(data, pos, _config._dmxUniverse[i]); }
    data[pos++] = _config._currentDriverSpeed;
    for (int i = 0; i < _config._numOutputs; i++) { data[pos++] = _config._outputColourOrder[i]; }
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputGrouping[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { data[pos++] = _config._outputBrightness[i]; }
    data[pos++] = _config._holdLastFrame;
    data[pos++] = _config._maxTargetTemp;
    data[pos++] = _config._currentDriverExpanded;
    data[pos++] = _config._currentDriverType;

    return pos;
}

int Pixlite16::PrepareV5Config(uint8_t* data) const
{
    int pos = 0;

    data[pos++] = 'A';
    data[pos++] = 'd';
    data[pos++] = 'v';
    data[pos++] = 'a';
    data[pos++] = 't';
    data[pos++] = 'e';
    data[pos++] = 'c';
    data[pos++] = 'h';
    data[pos++] = 0x00;
    Write16(data, pos, 5);
    data[pos++] = 0x05;

    memcpy(&data[pos], _config._mac, sizeof(_config._mac));
    pos += sizeof(_config._mac);
    data[pos++] = _config._dhcp;
    memcpy(&data[pos], _config._staticIP, sizeof(_config._staticIP));
    pos += sizeof(_config._staticIP);
    memcpy(&data[pos], _config._staticSubnetMask, sizeof(_config._staticSubnetMask));
    pos += sizeof(_config._staticSubnetMask);
    data[pos++] = _config._protocol;
    data[pos++] = _config._holdLastFrame;
    data[pos++] = _config._simpleConfig;
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputPixels[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputUniverse[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputStartChannel[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { data[pos++] = _config._outputNullPixels[i]; }
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputZigZag[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { data[pos++] = _config._outputReverse[i]; }
    for (int i = 0; i < _config._numOutputs; i++) { data[pos++] = _config._outputColourOrder[i]; }
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputGrouping[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { data[pos++] = _config._outputBrightness[i]; }
    for (int i = 0; i < _config._numDMX; i++) { data[pos++] = _config._dmxOn[i]; }
    for (int i = 0; i < _config._numDMX; i++) { Write16(data, pos, _config._dmxUniverse[i]); }
    data[pos++] = _config._currentDriver;
    data[pos++] = _config._currentDriverType;
    data[pos++] = _config._currentDriverSpeed;
    data[pos++] = _config._currentDriverExpanded;
    for (auto g : _config._gamma) { data[pos++] = g; }
    WriteString(data, pos, _config._nicknameLen, _config._nickname);
    data[pos++] = _config._maxTargetTemp;

    return pos;
}

int Pixlite16::PrepareV6Config(uint8_t* data) const
{
    int pos = 0;

    data[pos++] = 'A';
    data[pos++] = 'd';
    data[pos++] = 'v';
    data[pos++] = 'a';
    data[pos++] = 't';
    data[pos++] = 'e';
    data[pos++] = 'c';
    data[pos++] = 'h';
    data[pos++] = 0x00;
    Write16(data, pos, 5);
    data[pos++] = 0x06;

    memcpy(&data[pos], _config._mac, sizeof(_config._mac));
    pos += sizeof(_config._mac);
    data[pos++] = _config._dhcp;
    memcpy(&data[pos], _config._staticIP, sizeof(_config._staticIP));
    pos += sizeof(_config._staticIP);
    memcpy(&data[pos], _config._staticSubnetMask, sizeof(_config._staticSubnetMask));
    pos += sizeof(_config._staticSubnetMask);
    data[pos++] = _config._protocol;
    data[pos++] = _config._holdLastFrame;
    data[pos++] = _config._simpleConfig;
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputPixels[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputUniverse[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputStartChannel[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { data[pos++] = _config._outputNullPixels[i]; }
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputZigZag[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { data[pos++] = _config._outputReverse[i]; }
    for (int i = 0; i < _config._numOutputs; i++) { data[pos++] = _config._outputColourOrder[i]; }
    for (int i = 0; i < _config._numOutputs; i++) { Write16(data, pos, _config._outputGrouping[i]); }
    for (int i = 0; i < _config._numOutputs; i++) { data[pos++] = _config._outputBrightness[i]; }
    for (int i = 0; i < _config._numDMX; i++) { data[pos++] = _config._dmxOn[i]; }
    for (int i = 0; i < _config._numDMX; i++) { Write16(data, pos, _config._dmxUniverse[i]); }
    data[pos++] = _config._currentDriver;
    data[pos++] = _config._currentDriverType;
    data[pos++] = _config._currentDriverSpeed;
    data[pos++] = _config._currentDriverExpanded;
    for (auto g : _config._gamma) { data[pos++] = g; }
    WriteString(data, pos, _config._nicknameLen, _config._nickname);
    data[pos++] = _config._maxTargetTemp;

    return pos;
}

Pixlite16::Pixlite16(const std::string& ip)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    
    _ip = ip;
    _connected = false;

	// broadcast packet to find all of them
    wxIPV4address localAddr;
    localAddr.AnyAddress();
    localAddr.Service(49150);

    auto discovery = new wxDatagramSocket(localAddr, wxSOCKET_NOWAIT | wxSOCKET_BROADCAST);

    if (discovery == nullptr)
    {
        logger_base.error("Error initialising PixLite/PixCon discovery datagram.");
        return;
    } else if (!discovery->IsOk())
    {
        logger_base.error("Error initialising PixLite/PixCon discovery datagram ... is network connected? OK : FALSE");
        delete discovery;
        return;
    } 
    else if (discovery->Error())
    {
        logger_base.error("Error creating socket to broadcast from => %d : %s.", discovery->LastError(), (const char *)DecodeIPError(discovery->LastError()).c_str());
        delete discovery;
        return;
    }

    wxString broadcast = "255.255.255.255";
    logger_base.debug("PixLite/PixCon broadcasting to %s.", (const char *)broadcast.c_str());
    wxIPV4address broadcastAddr;
    broadcastAddr.Hostname(broadcast);
    broadcastAddr.Service(49150);

    wxByte discoveryData[12];
    discoveryData[0] = 'A';
    discoveryData[1] = 'd';
    discoveryData[2] = 'v';
    discoveryData[3] = 'a';
    discoveryData[4] = 't';
    discoveryData[5] = 'e';
    discoveryData[6] = 'c';
    discoveryData[7] = 'h';
    discoveryData[8] = 0x00;
    discoveryData[9] = 0x00;
    discoveryData[10] = 0x01;
    discoveryData[11] = 0x06;
    discovery->SendTo(broadcastAddr, discoveryData, sizeof(discoveryData));

    if (discovery->Error())
    {
        logger_base.error("PixLite/PixCon error broadcasting to %s => %d : %s.", (const char *)broadcast.c_str(), discovery->LastError(), (const char *)DecodeIPError(discovery->LastError()).c_str());
        return;
    }

    wxMilliSleep(500);

    // look through responses for one that matches my ip

    while (discovery->IsData())
    {
        uint8_t data[1500];
        memset(data, 0x00, sizeof(data));
        wxIPV4address pixliteAddr;
        discovery->RecvFrom(pixliteAddr, data, sizeof(data));

        if (!discovery->Error() && data[10] == 0x02)
        {
            _protocolVersion = data[11];
            switch (_protocolVersion)
            {
            case 4:
                _connected = ParseV4Config(data);
                break;
            case 5:
                _connected = ParseV5Config(data);
                break;
            case 6:
                _connected = ParseV6Config(data);
                break;
            default:
                logger_base.error("Unsupported protocol : %d.", _protocolVersion);
                wxASSERT(false);
                break;
            }

            if (_connected)
            {
                wxString rcvIP = wxString::Format("%i.%i.%i.%i", _config._currentIP[0], _config._currentIP[1], _config._currentIP[2], _config._currentIP[3]);

                logger_base.debug("Found PixLite/PixCon controller on %s.", (const char *)rcvIP.c_str());
                logger_base.debug("    Model %s %.1f.", (const char *)_config._modelName.c_str(), (float)_config._hwRevision / 10.0);
                logger_base.debug("    Firmware %s.", (const char *)_config._firmwareVersion.c_str());
                logger_base.debug("    Nickname %s.", (const char *)_config._nickname.c_str());
                logger_base.debug("    Brand %d.", _config._brand);

                if (_ip == rcvIP)
                {
                    logger_base.debug("*** Success connecting to PixLite/PixCon controller on %s.", (const char *)_ip.c_str());
                    break;
                }
                else
                {
                    _connected = false;
                }
            }
        }
        else if (discovery->Error())
        {
            logger_base.error("Error reading broadcast response => %d : %s.", discovery->LastError(), (const char *)DecodeIPError(discovery->LastError()).c_str());
        }
    }

    if (!_connected)
    {
        logger_base.error("Error connecting to PixLite/PixCon controller on %s.", (const char *)_ip.c_str());
    }

    discovery->Close();
    delete discovery;

    if (_connected)
    {
        DumpConfiguration();
    }
}

Pixlite16::~Pixlite16()
{
}

bool Pixlite16::SendConfig(bool logresult) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // broadcast packet to find all of them
    wxIPV4address localAddr;
    localAddr.AnyAddress();
    localAddr.Service(49150);

    auto config = new wxDatagramSocket(localAddr, wxSOCKET_NOWAIT);

    if (config == nullptr)
    {
        logger_base.error("Error initialising PixLite/PixCon config datagram.");
        return false;
    } else if (!config->IsOk())
    {
        logger_base.error("Error initialising PixLite/PixCon config datagram ... is network connected? OK : FALSE");
        delete config;
        return false;
    }
    else if (config->Error())
    {
        logger_base.error("Error creating PixLite/PixCon config datagram => %d : %s.", config->LastError(), (const char *)DecodeIPError(config->LastError()).c_str());
        delete config;
        return false;
    }

    logger_base.debug("PixLite/PixCon sending config to %s.", (const char *)_ip.c_str());
    wxIPV4address toAddr;
    toAddr.Hostname(_ip);
    toAddr.Service(49150);

    uint8_t data[1500];
    memset(data, 0x00, sizeof(data));
    int size = 0;

    switch (_protocolVersion)
    {
    case 4:
        size = PrepareV4Config(data);
        break;
    case 5:
        size = PrepareV5Config(data);
        break;
    case 6:
        size = PrepareV6Config(data);
        break;
    default:
        logger_base.error("Unsupported protocol : %d.", _protocolVersion);
        wxASSERT(false);
        break;
    }

    if (size > 0)
    {
        config->SendTo(toAddr, data, size);
    }

    config->Close();
    delete config;

    return true;
}

bool Pixlite16::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent)
{
    //ResetStringOutputs(); // this shouldnt be used normally

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("PixLite/PixCon Outputs Upload: Uploading to %s", (const char *)_ip.c_str());
    
    std::string check;
    UDController cud(_ip, _ip, allmodels, outputManager, &selected, check);

    PixLite16ControllerRules rules(_config);
    bool success = cud.Check(&rules, check);

    cud.Dump();

    logger_base.debug(check);

    // Get universes based on IP
    std::list<Output*> outputs = outputManager->GetAllOutputs(_ip, selected);

    if (success && cud.GetMaxPixelPort() > 0)
    {
        for (int pp = 1; pp <= rules.GetMaxPixelPort(); pp++)
        {
            if (cud.HasPixelPort(pp))
            {
                UDControllerPort* port = cud.GetControllerPixelPort(pp);

                // update the data
                _config._currentDriver = DecodeStringPortProtocol(port->GetProtocol());

                _config._outputUniverse[pp-1] = port->GetUniverse();
                _config._outputStartChannel[pp-1] = port->GetUniverseStartChannel();
                _config._outputPixels[pp-1] = port->Pixels();
                _config._outputNullPixels[pp-1] = port->GetFirstModel()->GetNullPixels(0);
                _config._outputGrouping[pp-1] = port->GetFirstModel()->GetGroupCount(1);
                _config._outputBrightness[pp-1] = port->GetFirstModel()->GetBrightness(100);
                if (port->GetFirstModel()->GetDirection("Forward") == "Reverse")
                {
                    _config._outputReverse[pp-1] =  1;
                }
                else
                {
                    _config._outputReverse[pp-1] = 0;
                }

                port->CreateVirtualStrings(true);
                if (port->GetVirtualStringCount() > 1)
                {
                    check += wxString::Format("WARN: String port %d has model settings that can't be uploaded.\n", pp);
                }
            }
        }
    }

    if (success)
    {
        if (cud.GetMaxSerialPort() > 0)
        {
            for (int sp = 1; sp <= rules.GetMaxSerialPort(); sp++)
            {
                if (cud.HasSerialPort(sp))
                {
                    UDControllerPort* port = cud.GetControllerSerialPort(sp);

                    _config._dmxUniverse[sp-1] = port->GetUniverse();
                    _config._dmxOn[sp-1] = 0x01; // turn it on

                    port->CreateVirtualStrings(true);
                    if (port->GetVirtualStringCount() > 1)
                    {
                        check += wxString::Format("WARN: Serial port %d has model settings that can't be uploaded.\n", sp);
                    }
                }
            }
        }
    }

    if (success)
    {
        if (check != "")
        {
            DisplayWarning("Upload warnings:\n" + check);
        }

        DumpConfiguration();

        return SendConfig(false);
    }

    DisplayError("Not uploaded due to errors.\n" + check);

    return false;
}

int Pixlite16::DecodeStringPortProtocol(std::string protocol)
{
    wxString p(protocol);
    p = p.Lower();

    if (p == "tls3001") return 0;
    if (p == "sm16716") return 1;
    if (p == "ws2801") return 2;
    if (p == "lpd6803") return 3;
    if (p == "ws2811") return 4;
    if (p == "mb16020") return 5;
    if (p == "tm1803") return 6;
    if (p == "tm1804") return 7;
    if (p == "tm1809") return 8;
    if (p == "my9231") return 9;
    if (p == "apa102") return 10;
    if (p == "my9221") return 11;
    if (p == "sk6812") return 12;
    if (p == "ucs1903") return 13;
    if (p == "p9813") return 14;

    return -1;
}

int Pixlite16::DecodeSerialOutputProtocol(std::string protocol)
{
    wxString p(protocol);
    p = p.Lower();
    if (p == "dmx") return 0;
    return -1;
}

void Pixlite16::DumpConfiguration() const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Dumping PixLite/PixCon configuration: Packet Version: %d.", _protocolVersion);
    logger_base.debug("    MAC %02x:%02x:%02x:%02x:%02x:%02x:", _config._mac[0], _config._mac[1], _config._mac[2], _config._mac[3], _config._mac[4], _config._mac[5]);
    logger_base.debug("    Nickname %d : %s", _config._nicknameLen, (const char *)_config._nickname.c_str());
    logger_base.debug("    Model Name %d : %s", _config._modelNameLen, (const char *)_config._modelName.c_str());
    logger_base.debug("    Firmware Version %d : %s", _config._firmwareVersionLen, (const char *)_config._firmwareVersion.c_str());
    logger_base.debug("    Brand : %d", _config._brand);
    logger_base.debug("    Hardware Revision : %d", _config._hwRevision);
    logger_base.debug("    Minimum Assistant Version : %d.%d.%d", _config._minAssistantVer[0], _config._minAssistantVer[1], _config._minAssistantVer[2]);
    logger_base.debug("    Current IP : %d.%d.%d.%d", _config._currentIP[0], _config._currentIP[1], _config._currentIP[2], _config._currentIP[3]);
    logger_base.debug("    Subnet Mask : %d.%d.%d.%d", _config._currentSubnetMask[0], _config._currentSubnetMask[1], _config._currentSubnetMask[2], _config._currentSubnetMask[3]);
    logger_base.debug("    DHCP : %d", _config._dhcp);
    logger_base.debug("    Static IP : %d.%d.%d.%d", _config._staticIP[0], _config._staticIP[1], _config._staticIP[2], _config._staticIP[3]);
    logger_base.debug("    Static Subnet Mask : %d.%d.%d.%d", _config._staticSubnetMask[0], _config._staticSubnetMask[1], _config._staticSubnetMask[2], _config._staticSubnetMask[3]);
    logger_base.debug("    Network Protocol : %d", _config._protocol);
    logger_base.debug("    Hold Last Frame : %d", _config._holdLastFrame);
    logger_base.debug("    Simple Config : %d", _config._simpleConfig);
    logger_base.debug("    Max Pixels Per Output : %d", _config._maxPixelsPerOutput);
    logger_base.debug("    Num Pixel Outputs : %d but really %d", _config._numOutputs, _config._realOutputs);
    logger_base.debug("    Pixel Outputs :");
    for (int i = 0; i < _config._numOutputs; i++)
    {
        logger_base.debug("        Pixel Output %d", i + 1);
        logger_base.debug("            Pixels %d", _config._outputPixels[i]);
        logger_base.debug("            Universe/StartChannel %d/%d", _config._outputUniverse[i], _config._outputStartChannel[i]);
        logger_base.debug("            Null Pixels %d", _config._outputNullPixels[i]);
        logger_base.debug("            Zig Zag %d", _config._outputZigZag[i]);
        logger_base.debug("            Brightness %d", _config._outputBrightness[i]);
        logger_base.debug("            Colour Order %d", _config._outputColourOrder[i]);
        logger_base.debug("            Reverse %d", _config._outputReverse[i]);
        logger_base.debug("            Grouping %d", _config._outputGrouping[i]);
    }
    logger_base.debug("    Num DMX Outputs : %d but really %d", _config._numDMX, _config._realDMX);
    for (int i = 0; i < _config._numDMX; i++)
    {
        logger_base.debug("        DMX Output %d", i + 1);
        logger_base.debug("            On %d", _config._dmxOn[i]);
        logger_base.debug("            Universe %d", _config._dmxUniverse[i]);
    }
    logger_base.debug("    Current Driver : %d ", _config._currentDriver);
    logger_base.debug("    Current Driver Type : %d ", _config._currentDriverType);
    logger_base.debug("    Current Driver Speed : %d ", _config._currentDriverSpeed);
    logger_base.debug("    Current Driver Expanded : %d ", _config._currentDriverExpanded);
    logger_base.debug("    Gamma : %.1f/%.1f/%.1f ", (float)_config._gamma[0] / 10.0, (float)_config._gamma[1] / 10.0, (float)_config._gamma[2] / 10.0);
    logger_base.debug("    Temperature : %.1f/%d ", (float)_config._temperature / 10.0, _config._maxTargetTemp);
    logger_base.debug("    Voltage Banks : %d ", _config._numBanks);
    for (int i = 0; i < _config._numBanks; i++)
    {
        logger_base.debug("        Voltage Bank %d : %.1f ", i + 1, (float)_config._bankVoltage[i] / 10.0);
    }
}

void Pixlite16::RegisterControllers() {
    for (auto &a : CONTROLLER_TYPE_MAP) {
        ControllerRegistry::AddController(&a);
    }
}
