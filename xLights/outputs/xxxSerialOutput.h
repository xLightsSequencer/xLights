#pragma once

#include "SerialOutput.h"

class wxXmlNode;

class xxxSerialOutput : public SerialOutput
{
protected:
    #pragma region xxx Constants
    static const unsigned int xxx_MAX_CHANNELS = 256;
    #pragma endregion xxx Constants

    #pragma region Member Variables
    long _lastheartbeat;
    uint8_t _data[xxx_MAX_CHANNELS];
    uint8_t _notSentCount[xxx_MAX_CHANNELS];
    uint8_t _lastSent[xxx_MAX_CHANNELS];
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    xxxSerialOutput(wxXmlNode* node);
    xxxSerialOutput(SerialOutput* output);
    xxxSerialOutput();
    virtual ~xxxSerialOutput() override {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_xxxSERIAL; }
    virtual int32_t GetMaxChannels() const override { return xxx_MAX_CHANNELS; }
    static int GetMaxxxxChannels() { return xxx_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= xxx_MAX_CHANNELS; }
    virtual std::string GetSetupHelp() const override;
    #pragma endregion Getters and Setters

    #pragma region Start and Stop
    virtual bool Open() override;
    #pragma endregion Start and Stop

    #pragma region Frame Handling
    virtual void EndFrame(int suppressFrames) override;
    virtual void ResetFrame() override;
    #pragma endregion Frame Handling

    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char data[], size_t size) override;
    virtual void AllOff() override;
    #pragma endregion Data Setting

    virtual void SendHeartbeat() const override;
};
