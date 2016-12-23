#ifndef OPENDMXOUTPUT_H
#define OPENDMXOUTPUT_H

#include "SerialOutput.h"

// ******************************************************
// * This class represents a single DMX universe
// * Compatible with Entec Open DMX, LOR dongle, D-Light dongle,
// * and any other FTDI-based USB to RS-485 converter
// * Methods should be called with: 0 <= chindex <= 511
// ******************************************************

#define OPENDMX_MAX_CHANNELS 512

class OpenDMXOutput : public SerialOutput
{
    wxByte _data[OPENDMX_MAX_CHANNELS + 1];

public:
    OpenDMXOutput(wxXmlNode* node);
    OpenDMXOutput(SerialOutput* output);
    OpenDMXOutput();
    virtual ~OpenDMXOutput() override {};

    virtual wxXmlNode* Save() override;
    virtual bool Open() override;
    virtual void EndFrame() override;
    virtual void SetOneChannel(int channel, unsigned char data) override;
    virtual void SetManyChannels(int channel, unsigned char* data, size_t size) override;
    virtual void AllOff() override;
    virtual std::string GetType() const override { return OUTPUT_OPENDMX; }
    virtual int GetMaxChannels() const override { return OPENDMX_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int channelCount) const override { return channelCount > 0 && channelCount <= OPENDMX_MAX_CHANNELS; }
};

 #endif