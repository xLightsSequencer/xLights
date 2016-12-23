#ifndef LOROUTPUT_H
#define LOROUTPUT_H

// Should be called with: 0 <= chindex <= 3839 (max channels=240*16)

#define LOR_PACKET_LEN 256
#define LOR_MAX_CHANNELS 3840

#include "SerialOutput.h"

class LOROutput : public SerialOutput
{
    long _lastheartbeat;
    wxByte _data[LOR_PACKET_LEN];

public:
    LOROutput(wxXmlNode* node);
    LOROutput(SerialOutput* output);
    LOROutput();
    virtual ~LOROutput() override {};

    virtual wxXmlNode* Save() override;
    virtual bool Open() override;
    virtual void StartFrame(long msec) override;
    virtual void EndFrame() override;
    virtual void ResetFrame() override;
    virtual void SetOneChannel(int channel, unsigned char data) override;
    virtual void AllOff() override;
    virtual std::string GetType() const override { return OUTPUT_LOR; }
    virtual void SendHeartbeat() const override;
    virtual int GetMaxChannels() const override { return LOR_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int channelCount) const override { return channelCount > 0 && channelCount <= LOR_MAX_CHANNELS; }
};

 #endif