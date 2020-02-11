
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/msgdlg.h>
#include <wx/regex.h>

#include "Pixlite16.h"
#include "models/Model.h"
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "UtilFunctions.h"
#include "ControllerUploadData.h"
#include "../outputs/ControllerEthernet.h"
#include "ControllerCaps.h"

#include <log4cpp/Category.hh>

#pragma region Encode and Decode
int Pixlite16::DecodeStringPortProtocol(std::string protocol) {
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

int Pixlite16::DecodeSerialOutputProtocol(std::string protocol) {

    wxString p(protocol);
    p = p.Lower();
    if (p == "dmx") return 0;
    return -1;
}
#pragma endregion

#pragma region Private Functions
uint16_t Pixlite16::Read16(uint8_t* data, int& pos) {

    uint16_t res = (static_cast<uint16_t>(data[pos]) << 8) + data[pos + 1];
    pos += 2;
    return res;
}

void Pixlite16::Write16(uint8_t* data, int& pos, int value) {

    data[pos++] = (value & 0xFF00) >> 8;
    data[pos++] = value & 0xFF;
}

void Pixlite16::WriteString(uint8_t* data, int& pos, int len, const std::string& value) {

    memset(&data[pos], 0x00, len);
    strncpy(reinterpret_cast<char*>(&data[pos]), static_cast<const char*>(value.c_str()), std::min(len, static_cast<int>(value.length())));
    pos += len;
}

bool Pixlite16::ParseV4Config(uint8_t* data, Pixlite16::Config& config) {

    int pos = 12;
    char buffer[256];

    config._maxPixelsPerOutput = 340;
    config._modelNameLen = 20;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._modelNameLen);
    config._modelName = buffer;
    pos += config._modelNameLen;

    memcpy(config._currentIP, &data[pos], sizeof(config._currentIP));
    pos += sizeof(config._currentIP);
    config._dhcp = data[pos++]; // I cant find this

    pos++; // unused

    config._nicknameLen = 40;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._nicknameLen);
    pos += config._nicknameLen;
    config._nickname = buffer;

    config._firmwareVersionLen = 20;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._firmwareVersionLen);
    pos += config._firmwareVersionLen;
    config._firmwareVersion = buffer;

    memcpy(config._mac, &data[pos], sizeof(config._mac));
    pos += sizeof(config._mac);

    config._temperature = Read16(data, pos);

    config._numOutputs = 16;
    if (config._modelName.find("16") != std::string::npos) {
        config._realOutputs = 16;
    }
    else {
        config._realOutputs = 4;
    }
    config._outputPixels.resize(config._numOutputs);
    for (int i = 0; i < config._numOutputs; i++) { config._outputPixels[i] = Read16(data, pos); }

    config._protocol = data[pos++];

    pos++; // unused
    pos++; // unused

    memcpy(config._staticIP, &data[pos], sizeof(config._staticIP));
    pos += sizeof(config._staticIP);

    memcpy(config._staticSubnetMask, &data[pos], sizeof(config._staticSubnetMask));
    pos += sizeof(config._staticSubnetMask);

    pos++; // unused
    pos++; // unused

    pos++; // 0x00 static universe - must be 0
    pos++; // 0x00
    pos++; // 0x00 static start channel - must be 0
    pos++; // 0x00

    config._currentDriver = data[pos++];

    config._gamma.resize(3);
    for (int i = 0; i < config._gamma.size(); i++) { config._gamma[i] = data[pos++]; }

    config._numBanks = 2;
    config._bankVoltage.resize(config._numBanks);
    for (int i = 0; i < config._numBanks; i++) { config._bankVoltage[i] = Read16(data, pos); }

    config._outputUniverse.resize(config._numOutputs);
    for (int i = 0; i < config._numOutputs; i++) { config._outputUniverse[i] = Read16(data, pos); }

    config._outputStartChannel.resize(config._numOutputs);
    for (int i = 0; i < config._numOutputs; i++) { config._outputStartChannel[i] = Read16(data, pos); }

    config._outputNullPixels.resize(config._numOutputs);
    for (int i = 0; i < config._numOutputs; i++) { config._outputNullPixels[i] = data[pos++]; }

    config._outputZigZag.resize(config._numOutputs);
    for (int i = 0; i < config._numOutputs; i++) { config._outputZigZag[i] = Read16(data, pos); }

    config._outputReverse.resize(config._numOutputs);
    for (int i = 0; i < config._numOutputs; i++) { config._outputReverse[i] = data[pos++]; }

    uint8_t rgb = data[pos++]; // 0xff turn on advanced RGB orders - if not FF then make it FF can copy the value to advanced
    pos++; // unused
    pos++; // unused

    config._numDMX = 4;
    if (config._modelName.find("16") != std::string::npos) {
        config._realDMX = 4;
    }
    else {
        config._realDMX = 1;
    }
    config._dmxOn.resize(config._numDMX);
    config._dmxUniverse.resize(config._numDMX);
    for (int i = 0; i < config._numDMX; i++) { config._dmxOn[i] = data[pos++]; }
    for (int i = 0; i < config._numDMX; i++) { config._dmxUniverse[i] = Read16(data, pos); }

    config._hwRevision = data[pos++];

    config._currentDriverSpeed = data[pos++];

    config._outputColourOrder.resize(config._numOutputs);
    if (rgb != 0xff) {
        for (int i = 0; i < config._numOutputs; i++) { config._outputColourOrder[i] = rgb; }
        pos += config._numOutputs;
    }
    else {
        for (int i = 0; i < config._numOutputs; i++) { config._outputColourOrder[i] = data[pos++]; }
    }

    config._outputGrouping.resize(config._numOutputs);
    for (int i = 0; i < config._numOutputs; i++) { config._outputGrouping[i] = Read16(data, pos); }

    config._outputBrightness.resize(config._numOutputs);
    for (int i = 0; i < config._numOutputs; i++) { config._outputBrightness[i] = data[pos++]; }

    config._holdLastFrame = data[pos++];
    config._maxTargetTemp = data[pos++];

    memcpy(config._currentSubnetMask, &data[pos], sizeof(config._currentSubnetMask));
    pos += sizeof(config._currentSubnetMask);

    config._currentDriverExpanded = data[pos++];

    memcpy(config._minAssistantVer, &data[pos], sizeof(config._minAssistantVer));
    pos += sizeof(config._minAssistantVer);

    config._testMode = data[pos++];

    config._currentDriverType = data[pos++];

    pos++; // 0xff
    pos++; // 0xff
    pos++; // 0xff
    pos++; // 0xff

    return true;
}


bool Pixlite16::ParseV5Config(uint8_t* data, Pixlite16::Config& config) {

    int pos = 12;
    char buffer[256];
    memcpy(config._mac, &data[pos], sizeof(config._mac));
    pos += 6;

    config._modelNameLen = data[pos++];
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._modelNameLen);
    config._modelName = buffer;
    pos += config._modelNameLen;

    config._hwRevision = data[pos++];
    memcpy(config._minAssistantVer, &data[pos], sizeof(config._minAssistantVer));
    pos += sizeof(config._minAssistantVer);

    config._firmwareVersionLen = 20;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._firmwareVersionLen);
    pos += config._firmwareVersionLen;
    config._firmwareVersion = buffer;

    config._brand = data[pos++];

    memcpy(config._currentIP, &data[pos], sizeof(config._currentIP));
    pos += sizeof(config._currentIP);

    memcpy(config._currentSubnetMask, &data[pos], sizeof(config._currentSubnetMask));
    pos += sizeof(config._currentSubnetMask);

    config._dhcp = data[pos++];

    memcpy(config._staticIP, &data[pos], sizeof(config._staticIP));
    pos += sizeof(config._staticIP);

    memcpy(config._staticSubnetMask, &data[pos], sizeof(config._staticSubnetMask));
    pos += sizeof(config._staticSubnetMask);

    config._protocol = data[pos++];
    config._holdLastFrame = data[pos++];
    config._simpleConfig = data[pos++];
    config._maxPixelsPerOutput = Read16(data, pos);
    config._numOutputs = data[pos++];
    config._realOutputs = config._numOutputs;
    config._outputPixels.resize(config._numOutputs);
    config._outputUniverse.resize(config._numOutputs);
    config._outputStartChannel.resize(config._numOutputs);
    config._outputNullPixels.resize(config._numOutputs);
    config._outputZigZag.resize(config._numOutputs);
    config._outputReverse.resize(config._numOutputs);
    config._outputColourOrder.resize(config._numOutputs);
    config._outputGrouping.resize(config._numOutputs);
    config._outputBrightness.resize(config._numOutputs);
    for (int i = 0; i < config._numOutputs; i++) { config._outputPixels[i] = Read16(data, pos); }
    for (int i = 0; i < config._numOutputs; i++) { config._outputUniverse[i] = Read16(data, pos); }
    for (int i = 0; i < config._numOutputs; i++) { config._outputStartChannel[i] = Read16(data, pos); }
    for (int i = 0; i < config._numOutputs; i++) { config._outputNullPixels[i] = data[pos++]; }
    for (int i = 0; i < config._numOutputs; i++) { config._outputZigZag[i] = Read16(data, pos); }
    for (int i = 0; i < config._numOutputs; i++) { config._outputReverse[i] = data[pos++]; }
    for (int i = 0; i < config._numOutputs; i++) { config._outputColourOrder[i] = data[pos++]; }
    for (int i = 0; i < config._numOutputs; i++) { config._outputGrouping[i] = Read16(data, pos); }
    for (int i = 0; i < config._numOutputs; i++) { config._outputBrightness[i] = data[pos++]; }

    config._numDMX = data[pos++];
    config._realDMX = config._numDMX;
    config._dmxOn.resize(config._numDMX);
    config._dmxUniverse.resize(config._numDMX);
    for (int i = 0; i < config._numDMX; i++) { config._dmxOn[i] = data[pos++]; }
    for (int i = 0; i < config._numDMX; i++) { config._dmxUniverse[i] = Read16(data, pos); }

    config._numDrivers = data[pos++];
    config._driverNameLen = data[pos++];
    config._driverType.resize(config._numDrivers);
    config._driverSpeed.resize(config._numDrivers);
    config._driverExpandable.resize(config._numDrivers);
    config._driverName.resize(config._numDrivers);
    for (int i = 0; i < config._numDrivers; i++) { config._driverType[i] = data[pos++]; }
    for (int i = 0; i < config._numDrivers; i++) { config._driverSpeed[i] = data[pos++]; }
    for (int i = 0; i < config._numDrivers; i++) { config._driverExpandable[i] = data[pos++]; }
    for (int i = 0; i < config._numDrivers; i++) {
        memset(buffer, 0x00, sizeof(buffer));
        memcpy(buffer, &data[pos], config._driverNameLen);
        pos += config._driverNameLen;
        config._driverName[i] = buffer;
    }

    config._currentDriver = data[pos++];
    config._currentDriverType = data[pos++];
    config._currentDriverSpeed = data[pos++];
    config._currentDriverExpanded = data[pos++];
    config._gamma.resize(3);
    for (int i = 0; i < config._gamma.size(); i++) { config._gamma[i] = data[pos++]; }

    config._nicknameLen = 40;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._nicknameLen);
    pos += config._nicknameLen;
    config._nickname = buffer;

    config._temperature = Read16(data, pos);
    config._maxTargetTemp = data[pos++];

    config._numBanks = data[pos++];
    config._bankVoltage.resize(config._numBanks);
    for (int i = 0; i < config._numBanks; i++) { config._bankVoltage[i] = Read16(data, pos); }

    config._testMode = data[pos++];

    //_testParameters not in v5

    return true;
}

bool Pixlite16::ParseV6Config(uint8_t* data, Pixlite16::Config& config) {

    int pos = 13;
    char buffer[256];
    memcpy(config._mac, &data[pos], sizeof(config._mac));
    pos += 6;

    config._modelNameLen = data[pos++];
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._modelNameLen);
    config._modelName = buffer;
    pos += config._modelNameLen;

    config._hwRevision = data[pos++];
    memcpy(config._minAssistantVer, &data[pos], sizeof(config._minAssistantVer));
    pos += sizeof(config._minAssistantVer);

    config._firmwareVersionLen = data[pos++];
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos + 1], config._firmwareVersionLen);
    config._firmwareVersion = buffer;
    pos += config._firmwareVersionLen;

    config._brand = data[pos++];

    memcpy(config._currentIP, &data[pos], sizeof(config._currentIP));
    pos += sizeof(config._currentIP);

    memcpy(config._currentSubnetMask, &data[pos], sizeof(config._currentSubnetMask));
    pos += sizeof(config._currentSubnetMask);

    config._dhcp = data[pos++];

    memcpy(config._staticIP, &data[pos], sizeof(config._staticIP));
    pos += sizeof(config._staticIP);

    memcpy(config._staticSubnetMask, &data[pos], sizeof(config._staticSubnetMask));
    pos += sizeof(config._staticSubnetMask);

    config._protocol = data[pos++];
    config._holdLastFrame = data[pos++];
    config._simpleConfig = data[pos++];
    config._maxPixelsPerOutput = Read16(data, pos);
    config._numOutputs = data[pos++];
    config._realOutputs = config._numOutputs;
    config._outputPixels.resize(config._numOutputs);
    config._outputUniverse.resize(config._numOutputs);
    config._outputStartChannel.resize(config._numOutputs);
    config._outputNullPixels.resize(config._numOutputs);
    config._outputZigZag.resize(config._numOutputs);
    config._outputReverse.resize(config._numOutputs);
    config._outputColourOrder.resize(config._numOutputs);
    config._outputGrouping.resize(config._numOutputs);
    config._outputBrightness.resize(config._numOutputs);
    for (int i = 0; i < config._numOutputs; i++) { config._outputPixels[i] = Read16(data, pos); }
    for (int i = 0; i < config._numOutputs; i++) { config._outputUniverse[i] = Read16(data, pos); }
    for (int i = 0; i < config._numOutputs; i++) { config._outputStartChannel[i] = Read16(data, pos); }
    for (int i = 0; i < config._numOutputs; i++) { config._outputNullPixels[i] = data[pos++]; }
    for (int i = 0; i < config._numOutputs; i++) { config._outputZigZag[i] = Read16(data, pos); }
    for (int i = 0; i < config._numOutputs; i++) { config._outputReverse[i] = data[pos++]; }
    for (int i = 0; i < config._numOutputs; i++) { config._outputColourOrder[i] = data[pos++]; }
    for (int i = 0; i < config._numOutputs; i++) { config._outputGrouping[i] = Read16(data, pos); }
    for (int i = 0; i < config._numOutputs; i++) { config._outputBrightness[i] = data[pos++]; }

    config._numDMX = data[pos++];
    config._realDMX = config._numDMX;
    config._dmxOn.resize(config._numDMX);
    config._dmxUniverse.resize(config._numDMX);
    for (int i = 0; i < config._numDMX; i++) { config._dmxOn[i] = data[pos++]; }
    for (int i = 0; i < config._numDMX; i++) { config._dmxUniverse[i] = Read16(data, pos); }

    config._numDrivers = data[pos++];
    config._driverNameLen = data[pos++];
    config._driverType.resize(config._numDrivers);
    config._driverSpeed.resize(config._numDrivers);
    config._driverExpandable.resize(config._numDrivers);
    config._driverName.resize(config._numDrivers);
    for (int i = 0; i < config._numDrivers; i++) { config._driverType[i] = data[pos++]; }
    for (int i = 0; i < config._numDrivers; i++) { config._driverSpeed[i] = data[pos++]; }
    for (int i = 0; i < config._numDrivers; i++) { config._driverExpandable[i] = data[pos++]; }
    for (int i = 0; i < config._numDrivers; i++) {
        memset(buffer, 0x00, sizeof(buffer));
        memcpy(buffer, &data[pos], config._driverNameLen);
        pos += config._driverNameLen;
        config._driverName[i] = buffer;
    }

    config._currentDriver = data[pos++];
    config._currentDriverType = data[pos++];
    config._currentDriverSpeed = data[pos++];
    config._currentDriverExpanded = data[pos++];
    config._gamma.resize(4);
    for (int i = 0; i < config._gamma.size(); i++) { config._gamma[i] = data[pos++]; }

    config._nicknameLen = 40;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._nicknameLen);
    pos += config._nicknameLen;
    config._nickname = buffer;

    config._temperature = Read16(data, pos);
    config._maxTargetTemp = data[pos++];

    config._numBanks = data[pos++];
    config._bankVoltage.resize(config._numBanks);
    for (int i = 0; i < config._numBanks; i++) { config._bankVoltage[i] = Read16(data, pos); }

    config._testMode = data[pos++];

    config._testParameters.resize(4);
    for (int i = 0; i < config._testParameters.size(); i++) { config._testParameters[i] = data[pos++]; }

    return true;
}

int Pixlite16::PrepareV4Config(uint8_t* data) const {

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
    if (EndsWith(_config._modelName, " LR")) {
        mn = _config._modelName.substr(0, _config._modelName.size() - 3);
    }
    else if (StartsWith(_config._modelName, "PixCon")) {
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

int Pixlite16::PrepareV5Config(uint8_t* data) const {

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

int Pixlite16::PrepareV6Config(uint8_t* data) const {

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

std::list<Pixlite16::Config> Pixlite16::DoDiscover()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::list<Pixlite16::Config> res;

    // broadcast packet to find all of them
    wxIPV4address localAddr;
    localAddr.AnyAddress();
    localAddr.Service(49150);

    auto discovery = new wxDatagramSocket(localAddr, wxSOCKET_NOWAIT | wxSOCKET_BROADCAST);

    if (discovery == nullptr) {
        logger_base.error("Error initialising PixLite/PixCon discovery datagram.");
        return res;
    }
    else if (!discovery->IsOk()) {
        logger_base.error("Error initialising PixLite/PixCon discovery datagram ... is network connected? OK : FALSE");
        delete discovery;
        return res;
    }
    else if (discovery->Error()) {
        logger_base.error("Error creating socket to broadcast from => %d : %s.", discovery->LastError(), (const char*)DecodeIPError(discovery->LastError()).c_str());
        delete discovery;
        return res;
    }

    wxString broadcast = "255.255.255.255";
    logger_base.debug("PixLite/PixCon broadcasting to %s.", (const char*)broadcast.c_str());
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

    if (discovery->Error()) {
        logger_base.error("PixLite/PixCon error broadcasting to %s => %d : %s.", (const char*)broadcast.c_str(), discovery->LastError(), (const char*)DecodeIPError(discovery->LastError()).c_str());
        return res ;
    }

    wxMilliSleep(500);

    // look through responses for one that matches my ip

    while (discovery->IsData()) {
        uint8_t data[1500];
        memset(data, 0x00, sizeof(data));
        wxIPV4address pixliteAddr;
        discovery->RecvFrom(pixliteAddr, data, sizeof(data));

        if (!discovery->Error() && data[10] == 0x02) {
            Pixlite16::Config config;
            memset(&config, 0x00, sizeof(config));
            bool connected = false;
            config._protocolVersion = data[11];
            switch (config._protocolVersion) {
            case 4:
                connected = ParseV4Config(data, config);
                break;
            case 5:
                connected = ParseV5Config(data, config);
                break;
            case 6:
                connected = ParseV6Config(data, config);
                break;
            default:
                logger_base.error("Unsupported protocol : %d.", config._protocolVersion);
                wxASSERT(false);
                break;
            }

            if (connected) {
                wxString rcvIP = wxString::Format("%i.%i.%i.%i", config._currentIP[0], config._currentIP[1], config._currentIP[2], config._currentIP[3]);

                logger_base.debug("Found PixLite/PixCon controller on %s.", (const char*)rcvIP.c_str());
                logger_base.debug("    Model %s %.1f.", (const char*)config._modelName.c_str(), (float)config._hwRevision / 10.0);
                logger_base.debug("    Firmware %s.", (const char*)config._firmwareVersion.c_str());
                logger_base.debug("    Nickname %s.", (const char*)config._nickname.c_str());
                logger_base.debug("    Brand %d.", config._brand);

                res.push_back(config);
            }
        }
        else if (discovery->Error()) {
            logger_base.error("Error reading broadcast response => %d : %s.", discovery->LastError(), (const char*)DecodeIPError(discovery->LastError()).c_str());
        }
    }

    discovery->Close();
    delete discovery;

    return res;
}

bool Pixlite16::SendConfig(bool logresult) const {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // broadcast packet to find all of them
    wxIPV4address localAddr;
    localAddr.AnyAddress();
    localAddr.Service(49150);

    auto config = new wxDatagramSocket(localAddr, wxSOCKET_NOWAIT);

    if (config == nullptr) {
        logger_base.error("Error initialising PixLite/PixCon config datagram.");
        return false;
    }
    else if (!config->IsOk()) {
        logger_base.error("Error initialising PixLite/PixCon config datagram ... is network connected? OK : FALSE");
        delete config;
        return false;
    }
    else if (config->Error()) {
        logger_base.error("Error creating PixLite/PixCon config datagram => %d : %s.", config->LastError(), (const char*)DecodeIPError(config->LastError()).c_str());
        delete config;
        return false;
    }

    logger_base.debug("PixLite/PixCon sending config to %s.", (const char*)_ip.c_str());
    wxIPV4address toAddr;
    toAddr.Hostname(_ip);
    toAddr.Service(49150);

    uint8_t data[1500];
    memset(data, 0x00, sizeof(data));
    int size = 0;

    switch (_protocolVersion) {
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

    if (size > 0) {
        config->SendTo(toAddr, data, size);
    }

    config->Close();
    delete config;

    return true;
}

void Pixlite16::DumpConfiguration(Pixlite16::Config& config) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Dumping PixLite/PixCon configuration: Packet Version: %d.", config._protocolVersion);
    logger_base.debug("    MAC %02x:%02x:%02x:%02x:%02x:%02x:", config._mac[0], config._mac[1], config._mac[2], config._mac[3], config._mac[4], config._mac[5]);
    logger_base.debug("    Nickname %d : %s", config._nicknameLen, (const char*)config._nickname.c_str());
    logger_base.debug("    Model Name %d : %s", config._modelNameLen, (const char*)config._modelName.c_str());
    logger_base.debug("    Firmware Version %d : %s", config._firmwareVersionLen, (const char*)config._firmwareVersion.c_str());
    logger_base.debug("    Brand : %d", config._brand);
    logger_base.debug("    Hardware Revision : %d", config._hwRevision);
    logger_base.debug("    Minimum Assistant Version : %d.%d.%d", config._minAssistantVer[0], config._minAssistantVer[1], config._minAssistantVer[2]);
    logger_base.debug("    Current IP : %d.%d.%d.%d", config._currentIP[0], config._currentIP[1], config._currentIP[2], config._currentIP[3]);
    logger_base.debug("    Subnet Mask : %d.%d.%d.%d", config._currentSubnetMask[0], config._currentSubnetMask[1], config._currentSubnetMask[2], config._currentSubnetMask[3]);
    logger_base.debug("    DHCP : %d", config._dhcp);
    logger_base.debug("    Static IP : %d.%d.%d.%d", config._staticIP[0], config._staticIP[1], config._staticIP[2], config._staticIP[3]);
    logger_base.debug("    Static Subnet Mask : %d.%d.%d.%d", config._staticSubnetMask[0], config._staticSubnetMask[1], config._staticSubnetMask[2], config._staticSubnetMask[3]);
    logger_base.debug("    Network Protocol : %d", config._protocol);
    logger_base.debug("    Hold Last Frame : %d", config._holdLastFrame);
    logger_base.debug("    Simple Config : %d", config._simpleConfig);
    logger_base.debug("    Max Pixels Per Output : %d", config._maxPixelsPerOutput);
    logger_base.debug("    Num Pixel Outputs : %d but really %d", config._numOutputs, config._realOutputs);
    logger_base.debug("    Pixel Outputs :");
    for (int i = 0; i < config._numOutputs; i++) {
        logger_base.debug("        Pixel Output %d", i + 1);
        logger_base.debug("            Pixels %d", config._outputPixels[i]);
        logger_base.debug("            Universe/StartChannel %d/%d", config._outputUniverse[i], config._outputStartChannel[i]);
        logger_base.debug("            Null Pixels %d", config._outputNullPixels[i]);
        logger_base.debug("            Zig Zag %d", config._outputZigZag[i]);
        logger_base.debug("            Brightness %d", config._outputBrightness[i]);
        logger_base.debug("            Colour Order %d", config._outputColourOrder[i]);
        logger_base.debug("            Reverse %d", config._outputReverse[i]);
        logger_base.debug("            Grouping %d", config._outputGrouping[i]);
    }
    logger_base.debug("    Num DMX Outputs : %d but really %d", config._numDMX, config._realDMX);
    for (int i = 0; i < config._numDMX; i++) {
        logger_base.debug("        DMX Output %d", i + 1);
        logger_base.debug("            On %d", config._dmxOn[i]);
        logger_base.debug("            Universe %d", config._dmxUniverse[i]);
    }
    logger_base.debug("    Current Driver : %d ", config._currentDriver);
    logger_base.debug("    Current Driver Type : %d ", config._currentDriverType);
    logger_base.debug("    Current Driver Speed : %d ", config._currentDriverSpeed);
    logger_base.debug("    Current Driver Expanded : %d ", config._currentDriverExpanded);
    logger_base.debug("    Gamma : %.1f/%.1f/%.1f ", (float)config._gamma[0] / 10.0, (float)config._gamma[1] / 10.0, (float)config._gamma[2] / 10.0);
    logger_base.debug("    Temperature : %.1f/%d ", (float)config._temperature / 10.0, config._maxTargetTemp);
    logger_base.debug("    Voltage Banks : %d ", config._numBanks);
    for (int i = 0; i < config._numBanks; i++) {
        logger_base.debug("        Voltage Bank %d : %.1f ", i + 1, (float)config._bankVoltage[i] / 10.0);
    }
}
#pragma endregion

#pragma region Constructors and Destructors
Pixlite16::Pixlite16(const std::string& ip) : BaseController(ip, "") {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    auto configs = DoDiscover();

    for (const auto& it : configs)
    {
        wxString rcvIP = wxString::Format("%i.%i.%i.%i", it._currentIP[0], it._currentIP[1], it._currentIP[2], it._currentIP[3]);

        if (_ip == rcvIP) {
            logger_base.debug("*** Success connecting to PixLite/PixCon controller on %s.", (const char *)_ip.c_str());
            _config = it;
            _protocolVersion = _config._protocolVersion;
            _model = _config._modelName;
            _version = _config._firmwareVersion;
            break;
        }
        else {
            _connected = false;
        }
    }

    if (!_connected) {
        logger_base.error("Error connecting to PixLite/PixCon controller on %s.", (const char *)_ip.c_str());
    }

    if (_connected) {
        DumpConfiguration(_config);
    }
}
#pragma endregion

#pragma region Getters and Setters
bool Pixlite16::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) {
    //ResetStringOutputs(); // this shouldnt be used normally

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("PixLite/PixCon Outputs Upload: Uploading to %s", (const char *)_ip.c_str());
    
    std::string check;
    UDController cud(controller, outputManager, allmodels, check);

    auto rules = ControllerCaps::GetControllerConfig(controller);
    bool success = cud.Check(rules, check);

    cud.Dump();

    logger_base.debug(check);

    std::list<Output*> outputs = controller->GetOutputs();
    if (success && cud.GetMaxPixelPort() > 0) {
        for (int pp = 1; pp <= rules->GetMaxPixelPort(); pp++) {
            if (cud.HasPixelPort(pp)) {
                UDControllerPort* port = cud.GetControllerPixelPort(pp);

                // update the data
                _config._currentDriver = DecodeStringPortProtocol(port->GetProtocol());

                _config._outputUniverse[pp-1] = port->GetUniverse();
                _config._outputStartChannel[pp-1] = port->GetUniverseStartChannel();
                _config._outputPixels[pp-1] = port->Pixels();
                _config._outputNullPixels[pp-1] = port->GetFirstModel()->GetNullPixels(0);
                _config._outputGrouping[pp-1] = port->GetFirstModel()->GetGroupCount(1);
                _config._outputBrightness[pp-1] = port->GetFirstModel()->GetBrightness(100);
                if (port->GetFirstModel()->GetDirection("Forward") == "Reverse") {
                    _config._outputReverse[pp-1] =  1;
                }
                else {
                    _config._outputReverse[pp-1] = 0;
                }

                port->CreateVirtualStrings(true);
                if (port->GetVirtualStringCount() > 1) {
                    check += wxString::Format("WARN: String port %d has model settings that can't be uploaded.\n", pp);
                }
            }
        }
    }

    if (success) {
        if (cud.GetMaxSerialPort() > 0) {
            for (int sp = 1; sp <= rules->GetMaxSerialPort(); sp++) {
                if (cud.HasSerialPort(sp)) {
                    UDControllerPort* port = cud.GetControllerSerialPort(sp);

                    _config._dmxUniverse[sp-1] = port->GetUniverse();
                    _config._dmxOn[sp-1] = 0x01; // turn it on

                    port->CreateVirtualStrings(true);
                    if (port->GetVirtualStringCount() > 1) {
                        check += wxString::Format("WARN: Serial port %d has model settings that can't be uploaded.\n", sp);
                    }
                }
            }
        }
    }

    if (success) {
        if (check != "") {
            DisplayWarning("Upload warnings:\n" + check);
        }

        DumpConfiguration(_config);

        return SendConfig(false);
    }

    DisplayError("Not uploaded due to errors.\n" + check);

    return false;
}

std::list<std::pair<std::string, std::string>> Pixlite16::Discover(wxWindow* parent)
{
    std::list<std::pair<std::string, std::string>> res;

    auto configs = DoDiscover();

    for (const auto& it : configs)
    {
        res.push_back({ wxString::Format("%i.%i.%i.%i", it._currentIP[0], it._currentIP[1], it._currentIP[2], it._currentIP[3]).ToStdString(), it._modelName});
    }

    return res;
}

#pragma endregion

