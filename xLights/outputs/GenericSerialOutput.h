#ifndef GENERICSERIALOUTPUT_H
#define GENERICSERIALOUTPUT_H

#include "SerialOutput.h"

#pragma region GenericSerial Constants
#define GENERICSERIAL_MAX_CHANNELS 8192
#pragma endregion GenericSerial Constants

class GenericSerialOutput : public SerialOutput
{
    #pragma region Member Variables
    int _datalen;
    std::vector<uint8_t> _data;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    GenericSerialOutput(wxXmlNode* node);
    GenericSerialOutput(const GenericSerialOutput& from);
    GenericSerialOutput();
    virtual ~GenericSerialOutput() override {};
    virtual Output* Copy() override
    {
        return new GenericSerialOutput(*this);
    }
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_GENERICSERIAL; }
    virtual int GetMaxChannels() const override { return GENERICSERIAL_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= GENERICSERIAL_MAX_CHANNELS; }
    //virtual std::string GetSetupHelp() const override;
    #pragma endregion Getters and Setters

    #pragma region Start and Stop
    virtual bool Open() override;
    #pragma endregion Start and Stop

    #pragma region Frame Handling
    virtual void EndFrame(int suppressFrames) override;
    #pragma endregion Frame Handling

    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void AllOff() override;
    #pragma endregion Data Setting
};

 #endif
