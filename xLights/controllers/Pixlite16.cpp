
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
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../UtilFunctions.h"
#include "ControllerUploadData.h"
#include "../outputs/ControllerEthernet.h"
#include "ControllerCaps.h"
#include "../Discovery.h"
#include "../xSchedule/wxJSON/jsonreader.h"
#include "../utils/Curl.h"

#include <log4cpp/Category.hh>

#define PIXLITE_PORT 49150

#pragma region Encode and Decode
int Pixlite16::DecodeStringPortProtocol(const std::string& protocol)
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
    if (p == "ucs2903") return 13;
    if (p == "p9813") return 14;
    if (p == "ucs1903") return 15;
    if (p == "apa109") return 31;

    return -1;
}

int Pixlite16::Mk3FrequencyForProtocol(const std::string& p)
{
    if (p == "apa102" || p == "lpd6803" || p == "mbi6020" || p == "my9221" || p == "my9231" || p == "my9291" || p == "p9813" || p == "sk9822" || p == "sm16716" || p == "ws2801")
        return 1600;
    if (p == "dmx512")
        return 750;
    if (p == "sk6822" || p == "tm1814")
        return 900;
    if (p == "tm1803")
        return 980;
    if (p == "tm1804")
        return 666;
    if (p == "tm1809")
        return 1110;
    if (p == "tm1914" || p == "ucs1903" || p == "ws2811")
        return 800;
    if (p == "tls3001")
        return 1000;
    return 0;
}

int Pixlite16::DecodeSerialOutputProtocol(const std::string& protocol)
{
    wxString p(protocol);
    p = p.Lower();
    if (p == "dmx") return 0;
    return -1;
}

int Pixlite16::EncodeColourOrder(const std::string& colourOrder)
{
    if (colourOrder == "RGB")
        return 0;
    if (colourOrder == "RBG")
        return 1;
    if (colourOrder == "GRB")
        return 2;
    if (colourOrder == "GBR")
        return 3;
    if (colourOrder == "BRG")
        return 4;
    if (colourOrder == "BGR")
        return 5;
    return 0;
}

std::string Pixlite16::DecodeColourOrder(const int colourOrder)
{
    switch (colourOrder) {
    case 0:
        return "RGB";
    case 1:
        return "RBG";
    case 2:
        return "GRB";
    case 3:
        return "GBR";
    case 4:
        return "BRG";
    case 5:
        return "BGR";
    }
    return "RGB";
}

#pragma endregion

#pragma region Private Functions
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
    strncpy(reinterpret_cast<char*>(&data[pos]), static_cast<const char*>(value.c_str()), std::min(len, static_cast<int>(value.length())));
    pos += len;
}

bool Pixlite16::ParseV4Config(uint8_t* data, Pixlite16::Config& config)
{
    int pos = 12;
    char buffer[256];

    config._maxPixelsPerOutput = 340;
    config._modelNameLen = 20;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._modelNameLen);
    config._modelName = std::string(buffer);
    pos += config._modelNameLen;

    memcpy(config._currentIP, &data[pos], sizeof(config._currentIP));
    pos += sizeof(config._currentIP);
    config._dhcp = data[pos++]; // I cant find this

    pos++; // unused

    config._nicknameLen = 40;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._nicknameLen);
    pos += config._nicknameLen;
    config._nickname = std::string(buffer);

    config._firmwareVersionLen = 20;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._firmwareVersionLen);
    pos += config._firmwareVersionLen;
    config._firmwareVersion = std::string(buffer);

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

bool Pixlite16::ParseV5Config(uint8_t* data, Pixlite16::Config& config)
{
    int pos = 12;
    char buffer[256];
    memcpy(config._mac, &data[pos], sizeof(config._mac));
    pos += 6;

    config._modelNameLen = data[pos++];
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._modelNameLen);
    config._modelName = std::string(buffer);
    pos += config._modelNameLen;

    config._hwRevision = data[pos++];
    memcpy(config._minAssistantVer, &data[pos], sizeof(config._minAssistantVer));
    pos += sizeof(config._minAssistantVer);

    config._firmwareVersionLen = 20;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._firmwareVersionLen);
    pos += config._firmwareVersionLen;
    config._firmwareVersion = std::string(buffer);

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
        config._driverName[i] = std::string(buffer);
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
    config._nickname = std::string(buffer);

    config._temperature = Read16(data, pos);
    config._maxTargetTemp = data[pos++];

    config._numBanks = data[pos++];
    config._bankVoltage.resize(config._numBanks);
    for (int i = 0; i < config._numBanks; i++) { config._bankVoltage[i] = Read16(data, pos); }

    config._testMode = data[pos++];

    //_testParameters not in v5

    return true;
}

bool Pixlite16::ParseV6Config(uint8_t* data, Pixlite16::Config& config)
{
    int pos = 13;
    char buffer[256];
    memcpy(config._mac, &data[pos], sizeof(config._mac));
    pos += 6;

    config._modelNameLen = data[pos++];
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._modelNameLen);
    config._modelName = std::string(buffer);
    pos += config._modelNameLen;

    config._hwRevision = data[pos++];
    memcpy(config._minAssistantVer, &data[pos], sizeof(config._minAssistantVer));
    pos += sizeof(config._minAssistantVer);

    config._firmwareVersionLen = data[pos++];
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos + 1], config._firmwareVersionLen);
    config._firmwareVersion = std::string(buffer);
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
        config._driverName[i] = std::string(buffer);
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
    config._nickname = std::string(buffer);

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

bool Pixlite16::ParseV8Config(uint8_t* data, Pixlite16::Config& config)
{
    int pos = 13;
    char buffer[256];
    memcpy(config._mac, &data[pos], sizeof(config._mac));
    pos += 6;

    config._modelNameLen = data[pos++];
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._modelNameLen);
    config._modelName = std::string(buffer);
    pos += config._modelNameLen;

    config._hwRevision = data[pos++];
    memcpy(config._minAssistantVer, &data[pos], sizeof(config._minAssistantVer));
    pos += sizeof(config._minAssistantVer);

    config._firmwareVersionLen = data[pos++];
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos + 1], config._firmwareVersionLen);
    config._firmwareVersion = std::string(buffer);
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
    for (int i = 0; i < config._numOutputs; i++) {
        config._outputPixels[i] = Read16(data, pos);
    }
    for (int i = 0; i < config._numOutputs; i++) {
        config._outputUniverse[i] = Read16(data, pos);
    }
    for (int i = 0; i < config._numOutputs; i++) {
        config._outputStartChannel[i] = Read16(data, pos);
    }
    for (int i = 0; i < config._numOutputs; i++) {
        config._outputNullPixels[i] = data[pos++];
    }
    for (int i = 0; i < config._numOutputs; i++) {
        config._outputZigZag[i] = Read16(data, pos);
    }
    for (int i = 0; i < config._numOutputs; i++) {
        config._outputReverse[i] = data[pos++];
    }
    for (int i = 0; i < config._numOutputs; i++) {
        config._outputColourOrder[i] = data[pos++];
    }
    for (int i = 0; i < config._numOutputs; i++) {
        config._outputGrouping[i] = Read16(data, pos);
    }
    for (int i = 0; i < config._numOutputs; i++) {
        config._outputBrightness[i] = data[pos++];
    }

    config._numDMX = data[pos++];
    config._protocolsOnDMX = data[pos++];
    config._realDMX = config._numDMX;
    config._dmxOn.resize(config._numDMX);
    config._dmxUniverse.resize(config._numDMX);
    for (int i = 0; i < config._numDMX; i++) {
        config._dmxOn[i] = data[pos++];
    }
    for (int i = 0; i < config._numDMX; i++) {
        config._dmxUniverse[i] = Read16(data, pos);
    }

    config._numDrivers = data[pos++];
    config._driverNameLen = data[pos++];
    config._driverType.resize(config._numDrivers);
    config._driverSpeed.resize(config._numDrivers);
    config._driverExpandable.resize(config._numDrivers);
    config._driverName.resize(config._numDrivers);
    for (int i = 0; i < config._numDrivers; i++) {
        config._driverType[i] = data[pos++];
    }
    for (int i = 0; i < config._numDrivers; i++) {
        config._driverSpeed[i] = data[pos++];
    }
    for (int i = 0; i < config._numDrivers; i++) {
        config._driverExpandable[i] = data[pos++];
    }
    for (int i = 0; i < config._numDrivers; i++) {
        memset(buffer, 0x00, sizeof(buffer));
        memcpy(buffer, &data[pos], config._driverNameLen);
        pos += config._driverNameLen;
        config._driverName[i] = std::string(buffer);
    }

    config._currentDriver = data[pos++];
    config._currentDriverType = data[pos++];
    config._currentDriverSpeed = data[pos++];
    config._currentDriverExpanded = data[pos++];
    config._gamma.resize(4);
    for (int i = 0; i < config._gamma.size(); i++) {
        config._gamma[i] = data[pos++];
    }

    config._nicknameLen = 40;
    memset(buffer, 0x00, sizeof(buffer));
    memcpy(buffer, &data[pos], config._nicknameLen);
    pos += config._nicknameLen;
    config._nickname = std::string(buffer);

    config._temperature = Read16(data, pos);
    config._maxTargetTemp = data[pos++];

    config._numBanks = data[pos++];
    config._bankVoltage.resize(config._numBanks);
    for (int i = 0; i < config._numBanks; i++) {
        config._bankVoltage[i] = Read16(data, pos);
    }

    config._testMode = data[pos++];

    config._testParameters.resize(4);
    for (int i = 0; i < config._testParameters.size(); i++) {
        config._testParameters[i] = data[pos++];
    }
    config._testOutputNum = data[pos++];
    config._testPixelNum = Read16(data, pos);

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
    if (EndsWith(_config._modelName, " LR")) {
        mn = _config._modelName.substr(0, _config._modelName.size() - 3);
    }
    else if (StartsWith(_config._modelName, "PixCon")) {
        mn = "PixLite" + _config._modelName.substr(6);
    }

    WriteString(data, pos, 20, mn);
    data[pos++] = _config._dhcp;
    WriteString(data, pos, 40, _config._nickname);

    bool expanded = _config._forceExpanded;
    for (int i = 0; i < _config._numOutputs; i++) {
        Write16(data, pos, _config._outputPixels[i]);
        if (i >= _config._numOutputs / 2 && _config._outputPixels[i] > 0) {
            expanded = true;
        }
    }
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
    data[pos++] = expanded ? 1 : 0;
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
    data[pos++] = 0; // We always do complex config : _config._simpleConfig;

    bool expanded = _config._forceExpanded;
    for (int i = 0; i < _config._numOutputs; i++) {
        Write16(data, pos, _config._outputPixels[i]);
        if (i >= _config._numOutputs / 2 && _config._outputPixels[i] > 0) {
            expanded = true;
        }
    }
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
    data[pos++] = expanded ? 1 : 0;
    for (auto g : _config._gamma) { data[pos++] = g; }
    WriteString(data, pos, 40, _config._nickname);
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
    data[pos++] = 0; // documentation is incorrect - 0 is advanced : _config._simpleConfig;
    bool expanded = _config._forceExpanded;
    for (int i = 0; i < _config._numOutputs; i++) {
        Write16(data, pos, _config._outputPixels[i]);
        if (i >= _config._numOutputs / 2 && _config._outputPixels[i] > 0) {
            expanded = true;
        }
    }
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
    data[pos++] = expanded ? 1 : 0;
    for (auto g : _config._gamma) { data[pos++] = g; }
    WriteString(data, pos, 40, _config._nickname);
    data[pos++] = _config._maxTargetTemp;

    return pos;
}

int Pixlite16::PrepareV8Config(uint8_t* data) const
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
    data[pos++] = 0x08;

    memcpy(&data[pos], _config._mac, sizeof(_config._mac));
    pos += sizeof(_config._mac);
    data[pos++] = _config._dhcp;
    memcpy(&data[pos], _config._staticIP, sizeof(_config._staticIP));
    pos += sizeof(_config._staticIP);
    memcpy(&data[pos], _config._staticSubnetMask, sizeof(_config._staticSubnetMask));
    pos += sizeof(_config._staticSubnetMask);
    data[pos++] = _config._protocol;
    data[pos++] = _config._holdLastFrame;
    data[pos++] = 0; // documentation is incorrect - 0 is advanced : _config._simpleConfig;
    bool expanded = _config._forceExpanded;
    for (size_t i = 0; i < _config._numOutputs; ++i) {
        Write16(data, pos, _config._outputPixels[i]);
        if (i >= _config._numOutputs / 2 && _config._outputPixels[i] > 0) {
            expanded = true;
        }
    }
    for (size_t i = 0; i < _config._numOutputs; ++i) {
        Write16(data, pos, _config._outputUniverse[i]);
    }
    for (size_t i = 0; i < _config._numOutputs; ++i) {
        Write16(data, pos, _config._outputStartChannel[i]);
    }
    for (size_t i = 0; i < _config._numOutputs; ++i) {
        data[pos++] = _config._outputNullPixels[i];
    }
    for (size_t i = 0; i < _config._numOutputs; ++i) {
        Write16(data, pos, _config._outputZigZag[i]);
    }
    for (size_t i = 0; i < _config._numOutputs; ++i) {
        data[pos++] = _config._outputReverse[i];
    }
    for (size_t i = 0; i < _config._numOutputs; ++i) {
        data[pos++] = _config._outputColourOrder[i];
    }
    for (size_t i = 0; i < _config._numOutputs; ++i) {
        Write16(data, pos, _config._outputGrouping[i]);
    }
    for (size_t i = 0; i < _config._numOutputs; ++i) {
        data[pos++] = _config._outputBrightness[i];
    }
    for (size_t i = 0; i < _config._numDMX; ++i) {
        data[pos++] = _config._dmxOn[i];
    }
    for (size_t i = 0; i < _config._numDMX; ++i) {
        Write16(data, pos, _config._dmxUniverse[i]);
    }
    data[pos++] = _config._currentDriver;
    data[pos++] = _config._currentDriverType;
    data[pos++] = _config._currentDriverSpeed;
    data[pos++] = expanded ? 1 : 0;
    for (auto g : _config._gamma) {
        data[pos++] = g;
    }
    WriteString(data, pos, 40, _config._nickname);
    data[pos++] = _config._maxTargetTemp;

    return pos;
}

void Pixlite16::CreateDiscovery(uint8_t* buffer)
{
    buffer[0] = 'A';
    buffer[1] = 'd';
    buffer[2] = 'v';
    buffer[3] = 'a';
    buffer[4] = 't';
    buffer[5] = 'e';
    buffer[6] = 'c';
    buffer[7] = 'h';
    buffer[8] = 0x00;
    buffer[9] = 0x00;
    buffer[10] = 0x01;
    buffer[11] = 0x06;
}

bool Pixlite16::GetConfig()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool res = false;

    memset(&_config, 0x00, sizeof(_config));

    // broadcast packet to find all of them
    wxIPV4address localAddr;
    localAddr.AnyAddress();
    localAddr.Service(PIXLITE_PORT);

    auto discovery = new wxDatagramSocket(localAddr, wxSOCKET_BROADCAST | wxSOCKET_NOWAIT);

    if (discovery == nullptr) {
        logger_base.error("Error initialising PixLite/PixCon datagram.");
    }
    else if (!discovery->IsOk()) {
        logger_base.error("Error initialising PixLite/PixCon datagram ... is network connected? OK : FALSE");
        delete discovery;
    }
    else if (discovery->Error()) {
        logger_base.error("Error creating PixLite/PixCon socket => %d : %s.", discovery->LastError(), (const char*)DecodeIPError(discovery->LastError()).c_str());
        delete discovery;
    }
    else {
        discovery->SetTimeout(1);
        discovery->Notify(false);

        wxIPV4address remoteAddr;
        remoteAddr.Hostname(_ip);
        remoteAddr.Service(PIXLITE_PORT);

        uint8_t discoveryData[12];
        Pixlite16::CreateDiscovery(discoveryData);
        logger_base.debug("Sending discovery to pixlite: %s:%d.", (const char*)_ip.c_str(), PIXLITE_PORT);
        discovery->SendTo(remoteAddr, discoveryData, sizeof(discoveryData));

        if (discovery->Error()) {
            logger_base.error("PixLite/PixCon error sending to %s => %d : %s.", (const char*)_ip.c_str(), discovery->LastError(), (const char*)DecodeIPError(discovery->LastError()).c_str());
        }
        else {
            uint32_t count = 0;
            #define SLP_TIME 100
            while (count < 5000 && !discovery->IsData()) {
                wxMilliSleep(SLP_TIME);
                count += SLP_TIME;
            }
            
            if (!discovery->IsData()) {
                logger_base.warn("No discovery responses.");
            }

            // look through responses for one that matches my ip
            while (discovery->IsData()) {
                uint8_t data[1500];
                memset(data, 0x00, sizeof(data));
                wxIPV4address pixliteAddr;
                discovery->RecvFrom(pixliteAddr, data, sizeof(data));

                if (!discovery->Error() && data[10] == 0x02) {
                    logger_base.debug("   Discover response from %s.", (const char *)pixliteAddr.IPAddress().c_str());
                    bool connected = false;
                    _config._protocolVersion = data[11];
                    logger_base.debug("   Protocol version %d.", _config._protocolVersion);
                    switch (_config._protocolVersion) {
                    case 4:
                        connected = ParseV4Config(data, _config);
                        if (!connected) {
                            logger_base.error("   Failed to parse v4 config packet.");
                        }
                        break;
                    case 5:
                        connected = ParseV5Config(data, _config);
                        if (!connected) {
                            logger_base.error("   Failed to parse v5 config packet.");
                        }
                        break;
                    case 6:
                        connected = ParseV6Config(data, _config);
                        if (!connected) {
                            logger_base.error("   Failed to parse v6 config packet.");
                        }
                        break;
                    case 8:
                        connected = ParseV8Config(data, _config);
                        if (!connected) {
                            logger_base.error("   Failed to parse v8 config packet.");
                        }
                        break;
                    default:
                        logger_base.error("Unsupported Pixlite protocol version: %d.", _config._protocolVersion);
                        wxASSERT(false);
                        break;
                    }

                    if (connected) {
                        wxString rcvIP = wxString::Format("%i.%i.%i.%i", _config._currentIP[0], _config._currentIP[1], _config._currentIP[2], _config._currentIP[3]);

                        logger_base.debug("Found PixLite/PixCon controller on %s.", (const char*)rcvIP.c_str());
                        logger_base.debug("    Model %s %.1f.", (const char*)_config._modelName.c_str(), (float)_config._hwRevision / 10.0);
                        logger_base.debug("    Firmware %s.", (const char*)_config._firmwareVersion.c_str());
                        logger_base.debug("    Nickname %s.", (const char*)_config._nickname.c_str());
                        logger_base.debug("    Brand %d.", _config._brand);
                        res = true;
                        break;
                    }
                    else {
                        logger_base.error("Unable to download PixLite/PixCon controller configuration from %s.", (const char*)_ip.c_str());
                    }
                }
                else if (discovery->Error()) {
                    logger_base.error("Error reading PixLite/PixCon response => %d : %s.", discovery->LastError(), (const char*)DecodeIPError(discovery->LastError()).c_str());
                }
            }
        }
        discovery->Close();
        delete discovery;
    }

    return res;
}

// we populate what we can in the Config structure so we can be consistent with earlier versions
bool Pixlite16::GetMK3Config()
{
    if (_mk3APIVersion == "") {
        _mk3Ver = Curl::HTTPSGet("http://" + _ip + "/ver", "", "");

        wxJSONValue jsonVal;
        wxJSONReader reader;
        reader.Parse(_mk3Ver, &jsonVal);

        if (jsonVal.IsValid() && !jsonVal.IsNull()) {
            _mk3APIVersion = jsonVal["result"]["apiVer"][0]["maj"].AsString() + "." + wxString::Format("%d", jsonVal["result"]["apiVer"][0]["min"][1].AsInt());
            _config._modelName = jsonVal["result"]["prodName"].AsString();
            _config._firmwareVersion = jsonVal["result"]["fwVer"].AsString();
            _config._nickname = jsonVal["result"]["nickname"].AsString();
            _config._brand = jsonVal["result"]["oem"].AsInt();
        }
    }

    if (_mk3APIVersion != "") {
        std::string request = "{\"req\":\"configRead\",\"id\":1,\"params\":{\"path\":[\"\"]}}";
        _mk3Config = Curl::HTTPSPost("http://" + _ip + "/" + _mk3APIVersion, request);

        wxJSONValue jsonVal;
        wxJSONReader reader;
        reader.Parse(_mk3Config, &jsonVal);

        if (jsonVal.IsValid() && !jsonVal.IsNull()) {
            _config._protocol = jsonVal["result"]["config"]["pix"]["dataSrc"].AsString() == "Art-Net" ? 1 : 0;
            _config._holdLastFrame = jsonVal["result"]["config"]["pix"]["holdLastFrm"].AsBool();
            _config._numOutputs = jsonVal["result"]["config"]["pixPort"]["pixCount"].AsArray()->Count();
            _config._currentDriverExpanded = jsonVal["result"]["config"]["pix"]["expand"].AsBool();
            _config._realOutputs = _config._numOutputs;
            if (!_config._currentDriverExpanded) {
                _config._realOutputs /= 2;
            }
            _config._outputPixels.resize(_config._numOutputs);
            _config._outputUniverse.resize(_config._numOutputs);
            _config._outputStartChannel.resize(_config._numOutputs);
            _config._outputNullPixels.resize(_config._numOutputs);
            _config._outputZigZag.resize(_config._numOutputs);
            _config._outputReverse.resize(_config._numOutputs);
            _config._outputColourOrder.resize(_config._numOutputs);
            _config._outputGrouping.resize(_config._numOutputs);
            _config._outputBrightness.resize(_config._numOutputs);

            for (uint32_t i = 0; i < _config._numOutputs; ++i) {
                _config._outputPixels[i] = jsonVal["result"]["config"]["pixPort"]["pixCount"][i].AsInt();
                _config._outputUniverse[i] = jsonVal["result"]["config"]["pixPort"]["startUni"][i].AsInt();
                _config._outputStartChannel[i] = jsonVal["result"]["config"]["pixPort"]["startCh"][i].AsInt();
                _config._outputNullPixels[i] = jsonVal["result"]["config"]["pixPort"]["startCh"][i].AsInt();
                _config._outputZigZag[i] = jsonVal["result"]["config"]["pixPort"]["zigZag"][i].AsInt();
                _config._outputReverse[i] = jsonVal["result"]["config"]["pixPort"]["reverse"][i].AsBool();
                _config._outputColourOrder[i] = EncodeColourOrder(jsonVal["result"]["config"]["pixPort"]["startCh"][i].AsString());
                _config._outputGrouping[i] = jsonVal["result"]["config"]["pixPort"]["group"][i].AsInt();
                _config._outputBrightness[i] = jsonVal["result"]["config"]["pixPort"]["intensity"][i].AsInt();
            }
            _config._numDMX = jsonVal["result"]["config"]["auxPort"]["uni"].AsArray()->Count();
            _config._dmxUniverse.resize(_config._numDMX);
            _config._dmxOn.resize(_config._numDMX);
            _config._realDMX = 0;
            for (uint32_t i = 0; i < _config._numDMX; ++i) {
                _config._dmxUniverse[i] = jsonVal["result"]["config"]["auxPort"]["uni"][i].AsInt();
                _config._dmxOn[i] = jsonVal["result"]["config"]["auxPort"]["mode"][i].AsString() == "DMX512Out" ? 1 : 0;
                if (_config._dmxOn[i])
                    ++_config._realDMX;
            }

            _config._protocolName = jsonVal["result"]["config"]["pix"]["pixType"].AsString();
            _config._currentDriverSpeed = jsonVal["result"]["config"]["pix"]["freq"].AsInt();
            _config._pixelsCanBeSplit = jsonVal["result"]["config"]["pix"]["pixsSpanUni"].AsBool();
            
            _config._gamma.resize(3);
            for (uint8_t i = 0; i < 3; ++i) {
                if (jsonVal["result"]["config"]["pix"]["gammaOn"].AsBool()) {
                    _config._gamma[i] = jsonVal["result"]["config"]["pix"]["gamma"][i].AsDouble() * 10;
                }
                else
                {
                    _config._gamma[i] = 0;
                }
            }

            request = "{\"req\":\"constantRead\",\"id\":1,\"params\":{\"path\":[\"\"]}}";
            _mk3Constants = Curl::HTTPSPost("http://" + _ip + "/" + _mk3APIVersion, request);

            return true;
        }
    }

    return false;
}

void Pixlite16::PrepareDiscovery(Discovery& discovery)
{
    uint8_t discoveryData[12];
    Pixlite16::CreateDiscovery(discoveryData);

    discovery.AddBroadcast(PIXLITE_PORT, [&discovery](wxDatagramSocket* socket, uint8_t* data, int len) {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.error("    Advatech discovery packet type : %d.", data[10]);
        if (data[10] == 0x02) {
            Pixlite16::Config it;
            memset(&it, 0x00, sizeof(it));
            bool connected = false;
            it._protocolVersion = data[11];
            switch (it._protocolVersion) {
            case 4:
                connected = ParseV4Config(data, it);
                break;
            case 5:
                connected = ParseV5Config(data, it);
                break;
            case 6:
                connected = ParseV6Config(data, it);
                break;
            case 8:
                connected = ParseV8Config(data, it);
                break;
            default:
                logger_base.error("Unsupported protocol : %d.", it._protocolVersion);
                wxASSERT(false);
                break;
            }

            if (connected) {
                wxString rcvIP = wxString::Format("%i.%i.%i.%i", it._currentIP[0], it._currentIP[1], it._currentIP[2], it._currentIP[3]);

                logger_base.debug("Found PixLite/PixCon controller on %s.", (const char*)rcvIP.c_str());
                logger_base.debug("    Model %s %.1f.", (const char*)it._modelName.c_str(), (float)it._hwRevision / 10.0);
                logger_base.debug("    Firmware %s.", (const char*)it._firmwareVersion.c_str());
                logger_base.debug("    Nickname %s.", (const char*)it._nickname.c_str());
                logger_base.debug("    Brand %d.", it._brand);

                auto eth = new ControllerEthernet(discovery.GetOutputManager(), false);
                eth->SetIP(wxString::Format("%i.%i.%i.%i", it._currentIP[0], it._currentIP[1], it._currentIP[2], it._currentIP[3]).ToStdString());
                eth->SetProtocol(OUTPUT_E131);
                eth->SetName(it._nickname);
                eth->EnsureUniqueId();
                bool mkII = Contains(it._modelName, "MkII");
                if (Contains(it._modelName, "PixLite")) {
                    eth->SetVendor("Advatek");
                    if (it._outputPixels.size() >= 16) {
                        if (mkII) {
                            eth->SetModel("PixLite 16 MkII");
                        }
                        else {
                            eth->SetModel("PixLite 16");
                        }
                    }
                    else {
                        if (mkII) {
                            eth->SetModel("PixLite 4 MkII");
                        }
                        else {
                            eth->SetModel("PixLite 4");
                        }
                    }
                }
                else {
                    eth->SetVendor("LOR");
                    eth->SetModel("PixCon 16");
                }
                discovery.AddController(eth);
            }
        }
        });
    discovery.SendBroadcastData(PIXLITE_PORT, discoveryData, sizeof(discoveryData));

    // MK3 Discovery
    uint8_t discoveryDataMK3[34]; // assumes we exclude no MAC Addresses

    discoveryDataMK3[0] = 'D';
    discoveryDataMK3[1] = 'i';
    discoveryDataMK3[2] = 's';
    discoveryDataMK3[3] = 'c';
    discoveryDataMK3[4] = 'P';
    discoveryDataMK3[5] = 'r';
    discoveryDataMK3[6] = 'o';
    discoveryDataMK3[7] = 't';
    discoveryDataMK3[8] = 0x12; // message id
    discoveryDataMK3[9] = 0x01;
    discoveryDataMK3[10] = 0x01; // version 0x0101
    discoveryDataMK3[11] = 0x01;
    discoveryDataMK3[12] = 0xFF; // all product families
    discoveryDataMK3[13] = 0xFF;
    discoveryDataMK3[14] = 0xFF;
    discoveryDataMK3[15] = 0xFF;
    discoveryDataMK3[16] = 0xFF; // OEM
    discoveryDataMK3[17] = 0xFF;
    discoveryDataMK3[18] = 0xFF;
    discoveryDataMK3[19] = 0xFF;
    discoveryDataMK3[20] = 0x00; // MAC Start
    discoveryDataMK3[21] = 0x00;
    discoveryDataMK3[22] = 0x00;
    discoveryDataMK3[23] = 0x00;
    discoveryDataMK3[24] = 0x00;
    discoveryDataMK3[25] = 0x00;
    discoveryDataMK3[26] = 0xFF; // MAC End
    discoveryDataMK3[27] = 0xFF;
    discoveryDataMK3[28] = 0xFF;
    discoveryDataMK3[29] = 0xFF;
    discoveryDataMK3[30] = 0xFF;
    discoveryDataMK3[31] = 0xFF;
    discoveryDataMK3[32] = 0x00; // Exclude MAC count
    discoveryDataMK3[33] = 0x00;

    discovery.AddMulticast("239.255.251.2", 49151, [&discovery](wxDatagramSocket* socket, uint8_t* data, int len) {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        if (len >= 12 && data[0] == 'D' && data[1] == 'i' && data[2] == 's' && data[3] == 'c' && data[4] == 'P' && data[5] == 'r' && data[6] == 'o' && data[7] == 't' && data[8] == 0x21 && data[9] == 0x02) {
            uint16_t ver = (((uint16_t)data[10]) << 8) + data[11];

            // this code was referencing the 0x0101 spec so may not work for other versions
            if (ver != 0x0101) {
                logger_base.debug("MK3 discovery protocol version 0x&04x unknown ... this may cause issues.");
            }

            char cdata[8096];
            memset(cdata, 0x00, sizeof(cdata));
            memcpy(cdata, &data[12], std::min(sizeof(cdata), (size_t)len - 12));

            wxJSONValue jsonVal;
            wxJSONReader reader;
            reader.Parse(cdata, &jsonVal);

            if (jsonVal.IsValid() && !jsonVal.IsNull()) {
                logger_base.debug("Found PixLite MK3 controller on %s.", (const char*)jsonVal["ipAddr"].AsString().c_str());
                logger_base.debug("    Model %s %s.", (const char*)jsonVal["prodName"].AsString().c_str(), (const char*)jsonVal["fwVer"].AsString().c_str());
                logger_base.debug("    Nickname %s.", (const char*)jsonVal["nickname"].AsString().c_str());

                std::string protocol = OUTPUT_E131;

                Pixlite16 p(jsonVal["ipAddr"].AsString());
                if (p.IsConnected()) {
                    if (p._config._protocol == 1) {
                        protocol = OUTPUT_ARTNET;
                    }
                }

                auto eth = new ControllerEthernet(discovery.GetOutputManager(), false);
                eth->SetIP(jsonVal["ipAddr"].AsString());
                eth->SetProtocol(protocol); // this may not be true ... but I need to call a different api to work it out
                eth->SetName(jsonVal["nickname"].AsString());
                eth->EnsureUniqueId();
                eth->SetVendor("Advatek");
                eth->SetModel(jsonVal["prodName"].AsString());
                discovery.AddController(eth);
            }
        }
    });
    discovery.SendData(49151, "239.255.251.1", discoveryDataMK3, sizeof(discoveryDataMK3));
}

bool Pixlite16::SendMk3Config(bool logresult) const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string request = "{\"req\":\"configChange\",\"id\":1,\"params\":{\"action\":\"save\",\"config\":{";

    bool expanded = _config._forceExpanded;
    int pp = 0;
    for (uint8_t i = 0; i < _config._outputPixels.size(); ++i) {
        if (_config._outputPixels[i] > 0) {
            pp = i + 1;
            if (i >= _config._numOutputs / 2 && _config._outputPixels[i] > 0) {
                expanded = true;
            }
        };
    }
    pp = pp <= _config._numOutputs / 2 ? _config._numOutputs / 2 : _config._numOutputs;

    request += wxString::Format("\"dev\": {\"nickname\":\"%s\"},", _config._nickname);

    request += "\"pix\":{";
    request += wxString::Format("\"dataSrc\":\"%s\",", _config._protocol == 0 ? "sACN" : "Art-Net");
    request += wxString::Format("\"pixType\":\"%s\",", wxString(_config._protocolName).Upper());
    if (Mk3FrequencyForProtocol(_config._protocolName) != 0) {
        request += wxString::Format("\"freq\":%d,", Mk3FrequencyForProtocol(_config._protocolName));
    }
    request += wxString::Format("\"expand\":%s,", expanded ? "true" : "false");
    request += "\"inFormat\":\"8Bit\",\"pixsSpanUni\":true},";

    // pix port
    request += "\"pixPort\":{";

    request += "\"pixCount\": [";
    for (uint8_t i = 0; i < pp; ++i) {
        request += wxString::Format("%d", _config._outputPixels[i]);
        if (i != pp - 1) {
            request += ",";
        }
    }
    request += "],";
    request += "\"startUni\": [";
    for (uint8_t i = 0; i < pp; ++i) {
        request += wxString::Format("%d", _config._outputUniverse[i]);
        if (i != pp - 1) {
            request += ",";
        }
    }
    request += "],";

    request += "\"startCh\": [";
    for (uint8_t i = 0; i < pp; ++i) {
        request += wxString::Format("%d", _config._outputStartChannel[i]);
        if (i != pp - 1) {
            request += ",";
        }
    }
    request += "],";

    request += "\"nullPix\": [";
    for (uint8_t i = 0; i < pp; ++i) {
        request += wxString::Format("%d", _config._outputNullPixels[i]);
        if (i != pp - 1) {
            request += ",";
        }
    }
    request += "],";

    request += "\"zigZag\": [";
    for (uint8_t i = 0; i < pp; ++i) {
        request += wxString::Format("%d", _config._outputZigZag[i]);
        if (i != pp - 1) {
            request += ",";
        }
    }
    request += "],";

    request += "\"group\": [";
    for (uint8_t i = 0; i < pp; ++i) {
        request += wxString::Format("%d", _config._outputGrouping[i]);
        if (i != pp - 1) {
            request += ",";
        }
    }
    request += "],";

    request += "\"reverse\": [";
    for (uint8_t i = 0; i < pp; ++i) {
        request += wxString::Format("%s", _config._outputReverse[i] ? "true" : "false");
        if (i != pp - 1) {
            request += ",";
        }
    }
    request += "],";
    
    request += "\"colorOrder\": [";
    for (uint8_t i = 0; i < pp; ++i) {
        request += wxString::Format("\"%s\"", DecodeColourOrder(_config._outputColourOrder[i]));
        if (i != pp - 1) {
            request += ",";
        }
    }
    request += "],";

    request += "\"intensity\": [";
    for (uint8_t i = 0; i < pp; ++i) {
        request += wxString::Format("%d", _config._outputBrightness[i]);
        if (i != pp - 1) {
            request += ",";
        }
    }
    request += "]";

    request += "},";

    // aux port
    request += "\"auxPort\":{";

    request += "\"dataSrc\": [";
    for (uint8_t i = 0; i < _config._dmxOn.size(); ++i) {
        request += _config._protocol == 0 ? "\"sACN\"" : "\"Art-Net\"";
        if (i != _config._dmxOn.size() - 1) {
            request += ",";
        }
    }
    request += "],";

    request += "\"mode\": [";
    for (uint8_t i = 0; i < _config._dmxOn.size(); ++i) {
        if (_config._dmxOn[i]) {
            request += "\"DMX512Out\"";
        } else {
            request += "\"Off\"";
        }
        if (i != _config._dmxOn.size() - 1) {
            request += ",";
        }
    }
    request += "],";

    request += "\"uni\":[";
    for (uint8_t i = 0; i < _config._dmxUniverse.size(); ++i) {
        request += wxString::Format("%d", _config._dmxUniverse[i]);
        if (i != _config._dmxUniverse.size() - 1) {
            request += ",";
        }
    }
    request += "]";

    request += "}";

    request += "}}}";

    logger_base.debug(request);

    auto res = Curl::HTTPSPost("http://" + _ip + "/" + _mk3APIVersion, request);

    logger_base.debug(res);

    wxJSONValue jsonVal;
    wxJSONReader reader;
    reader.Parse(res, &jsonVal);

    bool result = false;
    if (jsonVal.IsValid() && !jsonVal.IsNull()) {
        if (jsonVal.HasMember("result")) {
            result = jsonVal["result"]["saved"].AsBool() == true;
        } else if (jsonVal.HasMember("err")) {
            logger_base.error(jsonVal["err"]["msg"].AsString());
        }
    }
    return result;
}

bool Pixlite16::SendConfig(bool logresult) const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_mk3Ver != "") {
        return SendMk3Config(logresult);
    }

    // broadcast packet to find all of them
    wxIPV4address localAddr;
    localAddr.AnyAddress();
    localAddr.Service(PIXLITE_PORT);

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
    toAddr.Service(PIXLITE_PORT);

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
        size = PrepareV8Config(data); // Yeah this is weird but they seem to prefer the V8 packet
        break;
    case 8:
        size = PrepareV8Config(data);
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

void Pixlite16::DumpConfiguration(Pixlite16::Config& config)
{
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
    logger_base.debug("    Pixels on port can span universe : %d", config._pixelsCanBeSplit);
    logger_base.debug("    Simple Config : %d (0 = simple)", config._simpleConfig);
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
Pixlite16::Pixlite16(const std::string& ip) : BaseController(ip, "")
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _connected = false;
    logger_base.debug("Requesting pixlite configuration.");
    if (GetConfig()) {
        logger_base.debug("*** Success connecting to PixLite/PixCon controller on %s.", (const char*)_ip.c_str());
        _protocolVersion = _config._protocolVersion;
        _model = _config._modelName;
        _version = _config._firmwareVersion;
        _connected = true;
    }
    else {
        logger_base.debug("Requesting pixlite MK3 configuration.");
        if (GetMK3Config()) {
           _model = _config._modelName;
           _version = _config._firmwareVersion;
            _connected = true;
            logger_base.debug(_mk3Ver);
            logger_base.debug(_mk3Config);
            logger_base.debug(_mk3Constants);
        }
    }

    if (!_connected) {
        logger_base.error("Error connecting to PixLite/PixCon controller on %s.", (const char*)_ip.c_str());
    }

    if (_connected) {
        DumpConfiguration(_config);
    }
}
#pragma endregion

#pragma region Getters and Setters
#ifndef DISCOVERYONLY
bool Pixlite16::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent)
{
    //ResetStringOutputs(); // this shouldnt be used normally

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("PixLite/PixCon Outputs Upload: Uploading to %s", (const char*)_ip.c_str());

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);

    auto rules = ControllerCaps::GetControllerConfig(controller);
    bool success = cud.Check(rules, check);

    cud.Dump();

    logger_base.debug(check);

    if (controller->IsFullxLightsControl()) {

        for (auto& it : _config._outputPixels) {
            it = 0;
        }
        for (auto& it : _config._outputNullPixels) {
            it = 0;
        }
        for (auto& it : _config._outputZigZag) {
            it = 1;
        }
        for (auto& it : _config._outputGrouping) {
            it = 1;
        }
        for (auto& it : _config._outputColourOrder) {
            it = 0;
        }
        for (auto& it : _config._outputReverse) {
            it = 0;
        }
        for (auto& it : _config._outputBrightness) {
            it = 100;
        }

        for (auto& it : _config._dmxOn) {
            it = 0x00;
        }
    }

    // Handle varying maximum pixels based on expansion mode
    int maxPixels = 0;
    if (_config._numOutputs == 4 || _config._numOutputs == 8) {
        // 4 board
        if (cud.GetMaxPixelPort() > 4) {
            maxPixels = 510;
        }
        else {
            maxPixels = 1020;
        }
    }
    else if (_config._numOutputs == 16 || _config._numOutputs == 32) {
        // 16 board
        if (cud.GetMaxPixelPort() > 16) {
            maxPixels = 510;
        }
        else {
            maxPixels = 1020;
        }
    }

    std::list<Output*> outputs = controller->GetOutputs();
    _config._maxUsedPixelPort = cud.GetMaxPixelPort();
    _config._nickname = controller->GetName();
    _config._nicknameLen = _config._nickname.size();
    _config._protocolName = std::string("ws2811"); // we do this by default
    if (success && cud.GetMaxPixelPort() > 0) {
        for (int pp = 1; pp <= rules->GetMaxPixelPort(); pp++) {
            if (cud.HasPixelPort(pp)) {

                // always go advanced ... it doesnt hurt and it makes the config always work
                _config._simpleConfig = 1;

                UDControllerPort* port = cud.GetControllerPixelPort(pp);

                if (port->Pixels() > maxPixels) {
                    check += wxString::Format("ERR: String port %d has more pixels than this controller supports (%d when maximum is %d).\n", pp, port->Pixels(), maxPixels);
                    success = false;
                }

                // update the data
                _config._currentDriver = DecodeStringPortProtocol(port->GetProtocol());
                _config._protocolName = port->GetProtocol();
                _config._outputUniverse[pp - 1] = port->GetUniverse();
                _config._outputStartChannel[pp - 1] = port->GetUniverseStartChannel();
                _config._outputPixels[pp - 1] = port->Pixels();
                _config._outputNullPixels[pp - 1] = port->GetFirstModel()->GetStartNullPixels(0);
                _config._outputGrouping[pp - 1] = std::max(1, port->GetFirstModel()->GetGroupCount(1));
                _config._outputBrightness[pp - 1] = port->GetFirstModel()->GetBrightness(100);
                _config._outputColourOrder[pp - 1] = EncodeColourOrder(port->GetFirstModel()->GetColourOrder("RGB"));
                _config._outputZigZag[pp - 1] = port->GetFirstModel()->GetZigZag(0);
                if (port->GetFirstModel()->GetDirection("Forward") == "Reverse") {
                    _config._outputReverse[pp - 1] = 1;
                }
                else {
                    _config._outputReverse[pp - 1] = 0;
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

                    _config._dmxUniverse[sp - 1] = port->GetUniverse();
                    _config._dmxOn[sp - 1] = 0x01; // turn it on
                }
            }
        }
    }

    auto caps = ControllerCaps::GetControllerConfig(controller);
    if (caps != nullptr) {
        _config._forceExpanded = Lower(caps->GetCustomPropertyByPath("ForceExpanded", "false")) == "true";
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
#endif
#pragma endregion

