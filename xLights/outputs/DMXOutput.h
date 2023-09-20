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

#include "SerialOutput.h"

// ***************************************************************************************
// * This class represents a single DMX universe
// * Compatible with Entec Pro, Lynx DMX, DIYC RPM, DMXking.com, and DIYblinky.com dongles
// * Universes are exactly 512 bytes, except for DIYblinky, where they can be up to 3036 bytes
// * Methods should be called with: 0 <= chindex < 4800
// ***************************************************************************************

#pragma region DMX Constants
#define DMX_MAX_CHANNELS 4800
#pragma endregion DMX Constants

class DMXOutput : public SerialOutput
{
    #pragma region Member Variables
    int _datalen;
    uint8_t _data[DMX_MAX_CHANNELS + 6];
    #pragma endregion 

public:

    #pragma region Constructors and Destructors
    DMXOutput(wxXmlNode* node);
    DMXOutput(const DMXOutput& from);
    DMXOutput();
    virtual ~DMXOutput() override {};
    virtual Output* Copy() override
    {
        return new DMXOutput(*this);
    }
    #pragma endregion 

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_DMX; }
    virtual int GetMaxChannels() const override { return DMX_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= DMX_MAX_CHANNELS; }
    virtual bool AllowsBaudRateSetting() const override { return false; }
    virtual int GetDefaultBaudRate() const override { return 250000; }
    #pragma endregion 

    #pragma region Start and Stop
    virtual bool Open() override;
    #pragma endregion 
    
    #pragma region Frame Handling
    virtual void EndFrame(int suppressFrames) override;
    #pragma endregion 
    
    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char data[], size_t size) override;
    virtual void AllOff() override;
    #pragma region 
};
