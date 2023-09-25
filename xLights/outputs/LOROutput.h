#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SerialOutput.h"

// Should be called with: 0 <= chindex <= 3839 (max channels=240*16)
class LOROutput : public SerialOutput
{
protected:
    #pragma region LOR Constants
    static const unsigned int LOR_PACKET_LEN = 256;
    static const unsigned int LOR_MAX_CHANNELS = 20480;
    #pragma endregion 

    #pragma region Member Variables
    long _lastheartbeat = -1;
    uint8_t _data[LOR_PACKET_LEN];
    uint8_t _notSentCount[LOR_PACKET_LEN];
    uint8_t _lastSent[LOR_MAX_CHANNELS];
    #pragma endregion 

public:

    #pragma region Constructors and Destructors
    LOROutput(wxXmlNode* node);
    LOROutput(const LOROutput& from);
    LOROutput();
    virtual ~LOROutput() override {};
    virtual Output* Copy() override
    {
        return new LOROutput(*this);
    }
#pragma endregion 

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_LOR; }

    virtual int32_t GetMaxChannels() const override { return LOR_MAX_CHANNELS; }
    static int GetMaxLORChannels() { return LOR_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= LOR_MAX_CHANNELS; }
    #pragma endregion 

    #pragma region Start and Stop
    virtual bool Open() override;
    #pragma endregion Start and Stop

    #pragma region Frame Handling
    virtual void EndFrame(int suppressFrames) override;
    virtual void ResetFrame() override;
    virtual void SendHeartbeat() const override;
    #pragma endregion 

    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void AllOff() override;
    #pragma endregion 
};
