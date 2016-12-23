#ifndef OPENPIXELNETOUTPUT_H
#define OPENPIXELNETOUTPUT_H

#include "SerialOutput.h"

// Generic FTDI-based USB to RS485 dongle transmitting Pixelnet

#define OPENPIXELNET_MAX_CHANNELS 4096

class OpenPixelNetOutput : public SerialOutput
{
    int _datalen;
    wxByte _data[OPENPIXELNET_MAX_CHANNELS];
    wxByte _serialBuffer[OPENPIXELNET_MAX_CHANNELS + 8];

public:
    OpenPixelNetOutput(wxXmlNode* node);
    OpenPixelNetOutput(SerialOutput* output);
    OpenPixelNetOutput(Output* node);
    OpenPixelNetOutput();
    virtual ~OpenPixelNetOutput() override {};

    virtual wxXmlNode* Save() override;
    virtual bool Open() override;
    virtual void EndFrame() override;
    virtual void SetOneChannel(int channel, unsigned char data) override;
    virtual void AllOff() override;
    virtual std::string GetType() const override { return OUTPUT_OPENPIXELNET; }
    virtual int GetMaxChannels() const override { return OPENPIXELNET_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int channelCount) const override { return channelCount > 0 && channelCount <= OPENPIXELNET_MAX_CHANNELS; }
};

 #endif