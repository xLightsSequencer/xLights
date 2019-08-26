#pragma once
#include "SerialOutput.h"

class SyncrolightSerialOutput : public SerialOutput
{
protected:
    #pragma region Syncrolight Constants
    static const unsigned int SYNCROLIGHT_MAX_CHANNELS = 256;
    #pragma endregion Syncrolight Constants

    #pragma region Member Variables
    long _lastheartbeat;
    uint8_t _data[SYNCROLIGHT_MAX_CHANNELS];
    uint8_t _notSentCount[SYNCROLIGHT_MAX_CHANNELS];
    uint8_t _lastSent[SYNCROLIGHT_MAX_CHANNELS];
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    SyncrolightSerialOutput(wxXmlNode* node);
    SyncrolightSerialOutput(SerialOutput* output);
    SyncrolightSerialOutput();
    virtual ~SyncrolightSerialOutput() override {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_SYNCROLIGHTSERIAL; }
    virtual int32_t GetMaxChannels() const override { return SYNCROLIGHT_MAX_CHANNELS; }
    static int GetMaxSyncrolightChannels() { return SYNCROLIGHT_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= SYNCROLIGHT_MAX_CHANNELS; }
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
