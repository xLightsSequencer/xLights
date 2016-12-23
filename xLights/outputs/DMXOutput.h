#ifndef DMXOUTPUT_H
#define DMXOUTPUT_H

#include "SerialOutput.h"

// ***************************************************************************************
// * This class represents a single DMX universe
// * Compatible with Entec Pro, Lynx DMX, DIYC RPM, DMXking.com, and DIYblinky.com dongles
// * Universes are exactly 512 bytes, except for DIYblinky, where they can be up to 3036 bytes
// * Methods should be called with: 0 <= chindex < 3036
// ***************************************************************************************

#define DMX_MAX_CHANNELS 3036

class DMXOutput : public SerialOutput
{
    int _datalen;
    wxByte _data[DMX_MAX_CHANNELS + 6];

public:
    DMXOutput(wxXmlNode* node);
    DMXOutput(SerialOutput* output);
    DMXOutput();
    virtual ~DMXOutput() override {};

    virtual wxXmlNode* Save() override;
    virtual bool Open() override;
    virtual void EndFrame() override;
    virtual void SetOneChannel(int channel, unsigned char data) override;
    virtual void SetManyChannels(int channel, unsigned char* data, size_t size) override;
    virtual void AllOff() override;
    virtual std::string GetType() const override { return OUTPUT_DMX; }
    virtual int GetMaxChannels() const override { return DMX_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int channelCount) const override { return channelCount > 0 && channelCount <= DMX_MAX_CHANNELS; }
};

 #endif