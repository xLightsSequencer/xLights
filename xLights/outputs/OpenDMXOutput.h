#ifndef OPENDMXOUTPUT_H
#define OPENDMXOUTPUT_H

#include "SerialOutput.h"

// ******************************************************
// * This class represents a single DMX universe
// * Compatible with Entec Open DMX, LOR dongle, D-Light dongle,
// * and any other FTDI-based USB to RS-485 converter
// * Methods should be called with: 0 <= chindex <= 511
// ******************************************************

#pragma region Open DMX Constants
#define OPENDMX_MAX_CHANNELS 512
#pragma region Open DMX Constants

class OpenDMXOutput : public SerialOutput
{
    #pragma region Member Variables
    wxByte _data[OPENDMX_MAX_CHANNELS + 1];
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    OpenDMXOutput(wxXmlNode* node);
    OpenDMXOutput(SerialOutput* output);
    OpenDMXOutput();
    virtual ~OpenDMXOutput() override {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_OPENDMX; }
    virtual int GetMaxChannels() const override { return OPENDMX_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(long channelCount) const override { return channelCount > 0 && channelCount <= OPENDMX_MAX_CHANNELS; }
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
    #pragma endregion Data Setting
};

 #endif