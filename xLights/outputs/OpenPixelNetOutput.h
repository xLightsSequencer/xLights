#ifndef OPENPIXELNETOUTPUT_H
#define OPENPIXELNETOUTPUT_H

#include "SerialOutput.h"

// Generic FTDI-based USB to RS485 dongle transmitting Pixelnet

#pragma region Open PixelNet Constants
#define OPENPIXELNET_MAX_CHANNELS 4096
#pragma endregion Open PixelNet Constants

class OpenPixelNetOutput : public SerialOutput
{
    #pragma region Member Variables
    int _datalen;
    wxByte _data[OPENPIXELNET_MAX_CHANNELS];
    wxByte _serialBuffer[OPENPIXELNET_MAX_CHANNELS + 8];
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    OpenPixelNetOutput(wxXmlNode* node);
    OpenPixelNetOutput(SerialOutput* output);
    OpenPixelNetOutput();
    virtual ~OpenPixelNetOutput() override {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_OPENPIXELNET; }
    virtual int GetMaxChannels() const override { return OPENPIXELNET_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(long channelCount) const override { return channelCount > 0 && channelCount <= OPENPIXELNET_MAX_CHANNELS; }
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
    virtual void AllOff() override;
    #pragma endregion Data Setting
};

 #endif