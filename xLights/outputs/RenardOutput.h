#ifndef RENARDOUTPUT_H
#define RENARDOUTPUT_H

#include "SerialOutput.h"

#define RENARD_MAX_CHANNELS 1015

// Should be called with: 0 <= chindex <= 1015 (max channels=127*8)

class RenardOutput : public SerialOutput
{
    int _datalen;
    std::vector<wxByte> _data;

public:
    RenardOutput(SerialOutput* output);
    RenardOutput(wxXmlNode* node);
    RenardOutput();
    virtual ~RenardOutput() override {};

    virtual wxXmlNode* Save() override;
    virtual bool Open() override;
    virtual void EndFrame() override;
    virtual void SetOneChannel(int channel, unsigned char data) override;
    virtual void AllOff() override;
    virtual std::string GetType() const override { return OUTPUT_RENARD; }
    virtual int GetMaxChannels() const override { return RENARD_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int channelCount) const override { return channelCount > 0 && channelCount <= RENARD_MAX_CHANNELS; }
};

 #endif