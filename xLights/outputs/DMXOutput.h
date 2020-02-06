#pragma once

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
    DMXOutput(SerialOutput* output);
    DMXOutput();
    virtual ~DMXOutput() override {};
    #pragma endregion 

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_DMX; }
    virtual int GetMaxChannels() const override { return DMX_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= DMX_MAX_CHANNELS; }
    // virtual std::string GetSetupHelp() const override;
    virtual bool AllowsBaudRateSetting() const override { return false; }
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
