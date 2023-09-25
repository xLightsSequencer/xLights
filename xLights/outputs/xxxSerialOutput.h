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

class wxXmlNode;

class xxxSerialOutput : public SerialOutput
{
protected:
    #pragma region xxx Constants
    static const unsigned int xxx_MAX_CHANNELS = 256;
    #pragma endregion 

    #pragma region Member Variables
    long _lastheartbeat = -1;
    uint8_t _data[xxx_MAX_CHANNELS];
    uint8_t _notSentCount[xxx_MAX_CHANNELS];
    uint8_t _lastSent[xxx_MAX_CHANNELS];
    #pragma endregion 

    #pragma region Private Functions
    virtual void SendHeartbeat() const override;
    #pragma endregion 

public:

    #pragma region Constructors and Destructors
    xxxSerialOutput(wxXmlNode* node);
    xxxSerialOutput(const xxxSerialOutput& from);
    xxxSerialOutput();
    virtual ~xxxSerialOutput() override {};
    virtual Output* Copy() override
    {
        return new xxxSerialOutput(*this);
    }
#pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_xxxSERIAL; }

    virtual int32_t GetMaxChannels() const override { return xxx_MAX_CHANNELS; }
    static int GetMaxxxxChannels() { return xxx_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= xxx_MAX_CHANNELS; }
    #pragma endregion 

    #pragma region Start and Stop
    virtual bool Open() override;
    #pragma endregion 

    #pragma region Frame Handling
    virtual void EndFrame(int suppressFrames) override;
    virtual void ResetFrame() override;
    #pragma endregion 

    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char data[], size_t size) override;
    virtual void AllOff() override;
    #pragma endregion 
};
