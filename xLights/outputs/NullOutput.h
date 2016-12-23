#ifndef NULLOUTPUT_H
#define NULLOUTPUT_H

#include "Output.h"

class NullOutput : public Output
{
public:
    NullOutput(wxXmlNode* node) : Output(node) {}
    NullOutput() : Output() { _channels = 512; }
    virtual ~NullOutput() override {};

    virtual wxXmlNode* Save() override;
    virtual bool Open() override { return true; }
    virtual void Close() override {}
    virtual void StartFrame(long msec) override {}
    virtual void EndFrame() override {}
    virtual void ResetFrame() override {}
    virtual void SetOneChannel(int channel, unsigned char data) override {}
    virtual void SetManyChannels(int channel, unsigned char* data, size_t size) override {};
    virtual void AllOff() override {}
    virtual std::string GetType() const override { return OUTPUT_NULL; }
    virtual std::string GetLongDescription() const override;
    virtual bool IsIpOutput() const override { return false; }
    virtual bool IsOutputable() const override { return false; }
    virtual std::string GetChannelMapping(int ch) const override;
    virtual size_t TxNonEmptyCount() const override { return 0; }
    virtual bool TxEmpty() const override { return true; }
    virtual int GetMaxChannels() const override { return 9999999; }
    virtual bool IsValidChannelCount(int channelCount) const override { return channelCount > 0; }

#ifndef EXCLUDENETWORKUI
    virtual int Configure(wxWindow* parent, OutputManager& outputManager) override;
#endif
};

 #endif