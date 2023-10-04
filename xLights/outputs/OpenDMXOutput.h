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

// ******************************************************
// * This class represents a single DMX universe
// * Compatible with Entec Open DMX, LOR dongle, D-Light dongle,
// * and any other FTDI-based USB to RS-485 converter
// * Methods should be called with: 0 <= chindex <= 511
// ******************************************************

#pragma region Open DMX Constants
#define OPENDMX_MAX_CHANNELS 512
#pragma region 

class OpenDMXOutput : public SerialOutput
{
    #pragma region Member Variables
    uint8_t _data[OPENDMX_MAX_CHANNELS + 1];
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    OpenDMXOutput(wxXmlNode* node);
    OpenDMXOutput();
    OpenDMXOutput(const OpenDMXOutput& from);
    virtual ~OpenDMXOutput() override {};
    virtual Output* Copy() override
    {
        return new OpenDMXOutput(*this);
    }
#pragma endregion 

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_OPENDMX; }

    virtual int GetMaxChannels() const override { return OPENDMX_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= OPENDMX_MAX_CHANNELS; }

    virtual bool AllowsBaudRateSetting() const override { return false; }
    virtual int GetDefaultBaudRate() const override
    {
        return 250000;
    }
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
    #pragma endregion 
};