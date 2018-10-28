#ifndef DMXOUTPUT_H
#define DMXOUTPUT_H

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
    wxByte _data[DMX_MAX_CHANNELS + 6];
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    DMXOutput(wxXmlNode* node);
    DMXOutput(SerialOutput* output);
    DMXOutput();
    virtual ~DMXOutput() override {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_DMX; }
    virtual int GetMaxChannels() const override { return DMX_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(long channelCount) const override { return channelCount > 0 && channelCount <= DMX_MAX_CHANNELS; }
    virtual std::string GetSetupHelp() const override;
    virtual bool AllowsBaudRateSetting() const override { return false; }
    #pragma endregion Getters and Setters

    #pragma region Start and Stop
    virtual bool Open() override;
    #pragma endregion Start and Stop
    
    #pragma region Frame Handling
    virtual void EndFrame(int suppressFrames) override;
    #pragma endregion Frame Handling
    
    #pragma region Data Setting
    virtual void SetOneChannel(long channel, unsigned char data) override;
    virtual void SetManyChannels(long channel, unsigned char data[], long size) override;
    virtual void AllOff() override;
    #pragma region Data Setting
};

 #endif
