#ifndef PIXELNETOUTPUT_H
#define PIXELNETOUTPUT_H

#include "SerialOutput.h"

// DIYLightAnimation.com Lynx USB dongle with Pixenet firmware
// Should be called with: 0 <= chindex <= 4095

#define PIXELNET_MAX_CHANNELS 4095

class PixelNetOutput : public SerialOutput
{
    int _datalen;
    wxByte _data[PIXELNET_MAX_CHANNELS + 1];
    wxByte _serialBuffer[PIXELNET_MAX_CHANNELS + 2];

public:
    PixelNetOutput(wxXmlNode* node);
    PixelNetOutput(SerialOutput* output);
    PixelNetOutput(Output* node);
    PixelNetOutput();
    virtual ~PixelNetOutput() override {};

    virtual wxXmlNode* Save() override;
    virtual bool Open() override;
    virtual void EndFrame() override;
    virtual void SetOneChannel(int channel, unsigned char data) override;
    virtual void AllOff() override;
    virtual std::string GetType() const override { return OUTPUT_PIXELNET; }
    virtual int GetMaxChannels() const override { return PIXELNET_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int channelCount) const override { return channelCount > 0 && channelCount <= PIXELNET_MAX_CHANNELS; }
};

 #endif