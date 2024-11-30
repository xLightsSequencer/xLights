#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
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
    std::string _deviceChannels;
    #pragma endregion 

    #pragma region Private Functions
    virtual void SendHeartbeat() const override;
    uint8_t GetDeviceFromChannel(uint32_t channel) const;
    uint8_t GetChannelOnDevice(uint32_t channel) const;
    uint8_t GetDeviceChannels(uint8_t device) const;
    uint8_t PopulateBuffer(uint8_t* buffer, int32_t channel, uint8_t value) const;
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
    std::string GetDeviceChannels() const {
        return _deviceChannels;
    }
    void SetDeviceChannels(const std::string& deviceChannels) {
        if (_deviceChannels != deviceChannels) {
            _deviceChannels = deviceChannels;
            _dirty = true;
        }
        auto ch = wxSplit(_deviceChannels, ',');
        uint8_t channels = 0;
        for (const auto& it : ch) {
            channels += wxAtoi(it);
        }
        SetChannels(channels);
    }
    virtual wxXmlNode* Save() override;

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

    #pragma region UI
#ifndef EXCLUDENETWORKUI
    virtual void UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    virtual void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) override;
    virtual void RemoveProperties(wxPropertyGrid* propertyGrid) override;
    virtual bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) override;
#endif
#pragma endregion
};
