#ifndef NULLOUTPUT_H
#define NULLOUTPUT_H

#include "Output.h"

class NullOutput : public Output
{
public:

    #pragma region Constructors and Destructors
    NullOutput(wxXmlNode* node) : Output(node) {}
    NullOutput() : Output() { _channels = 512; }
    virtual ~NullOutput() override {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_NULL; }
    virtual std::string GetLongDescription() const override;
    virtual bool IsIpOutput() const override { return false; }
    virtual bool IsSerialOutput() const override { return false; }
    virtual bool IsOutputable() const override { return false; }
    virtual std::string GetChannelMapping(long ch) const override;
    virtual int GetMaxChannels() const override { return 9999999; }
    virtual bool IsValidChannelCount(long channelCount) const override { return channelCount > 0; }
    #pragma endregion Getters and Setters

    #pragma region Start and Stop
    virtual bool Open() override { return true; }
    virtual void Close() override {}
    #pragma endregion Start and Stop
    
    #pragma region Frame Handling
    virtual void StartFrame(long msec) override {}
    virtual void EndFrame(int suppressFrames) override {}
    virtual void ResetFrame() override {}
    #pragma endregion Frame Handling
    
    #pragma region Data Setting
    virtual void SetOneChannel(long channel, unsigned char data) override {}
    virtual void SetManyChannels(long channel, unsigned char* data, long size) override {};
    virtual void AllOff() override {}
    #pragma endregion Data Setting

    #pragma region UI
#ifndef EXCLUDENETWORKUI
    virtual Output* Configure(wxWindow* parent, OutputManager* outputManager) override;
#endif
    #pragma endregion UI
};

 #endif