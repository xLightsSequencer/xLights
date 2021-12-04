#pragma once

#include "IPOutput.h"

class TwinklyOutput : public IPOutput
{
public:
#pragma region Constructors and Destructors
    TwinklyOutput(wxXmlNode* node);
    TwinklyOutput();
    virtual ~TwinklyOutput() override;
    virtual wxXmlNode* Save() override;
#pragma endregion

#pragma region Output overrides
    virtual std::string GetType() const override;
    virtual int GetMaxChannels() const override;
    virtual bool IsValidChannelCount(int32_t channelCount) const override;
    virtual std::string GetLongDescription() const override;
#pragma endregion

#pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
#pragma endregion

#pragma region Frame Handling
    virtual void StartFrame(long msec) override;
    virtual void EndFrame(int suppressFrames) override;
    virtual void ResetFrame() override;
#pragma endregion

#pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char* data, size_t size) override;
    virtual void AllOff() override;
#pragma endregion

private:
    // A single twinkly connection may have unlimited channels
    static const int MAX_CHANNELS = 4 * 2000;
};
