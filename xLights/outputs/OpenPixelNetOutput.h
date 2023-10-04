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

// Generic FTDI-based USB to RS485 dongle transmitting Pixelnet

#pragma region Open PixelNet Constants
#define OPENPIXELNET_MAX_CHANNELS 4096
#pragma endregion Open PixelNet Constants

class OpenPixelNetOutput : public SerialOutput
{
    #pragma region Member Variables
    int _datalen = 0;
    uint8_t _data[OPENPIXELNET_MAX_CHANNELS];
    uint8_t _serialBuffer[OPENPIXELNET_MAX_CHANNELS + 8];
    #pragma endregion

public:

    #pragma region Constructors and Destructors
    OpenPixelNetOutput(wxXmlNode* node);
    OpenPixelNetOutput(const OpenPixelNetOutput& from);
    OpenPixelNetOutput();
    virtual ~OpenPixelNetOutput() override {};
    virtual Output* Copy() override
    {
        return new OpenPixelNetOutput(*this);
    }
#pragma endregion 

    #pragma region Getters and Setters
    
    virtual std::string GetType() const override { return OUTPUT_OPENPIXELNET; }
    
    virtual int GetMaxChannels() const override { return OPENPIXELNET_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= OPENPIXELNET_MAX_CHANNELS; }
    
    virtual bool AllowsBaudRateSetting() const override { return false; }
    virtual int GetDefaultBaudRate() const override { return 1000000;}
    #pragma endregion

    #pragma region Start and Stop
    virtual bool Open() override;
    #pragma endregion 
    
    #pragma region Frame Handling
    virtual void EndFrame(int suppressFrames) override;
    #pragma endregion 

    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void AllOff() override;
    #pragma endregion 
};
