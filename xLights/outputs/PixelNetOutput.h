#ifndef PIXELNETOUTPUT_H
#define PIXELNETOUTPUT_H

#include "SerialOutput.h"

// DIYLightAnimation.com Lynx USB dongle with Pixenet firmware
// Should be called with: 0 <= chindex <= 4095

#pragma region PixelNet Constants
#define PIXELNET_MAX_CHANNELS 4096
#pragma endregion PixelNet Constants

class PixelNetOutput : public SerialOutput
{
    #pragma region Member Variables
    int _datalen;
    uint8_t _data[PIXELNET_MAX_CHANNELS];
    uint8_t _serialBuffer[PIXELNET_MAX_CHANNELS + 1];
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PixelNetOutput(wxXmlNode* node);
    PixelNetOutput(SerialOutput* output);
    PixelNetOutput();
    virtual ~PixelNetOutput() override {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_PIXELNET; }
    virtual int GetMaxChannels() const override { return PIXELNET_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= PIXELNET_MAX_CHANNELS; }
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
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void AllOff() override;
    #pragma endregion Data Setting
};

 #endif
