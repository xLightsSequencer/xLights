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

#include "Output.h"

class NullOutput : public Output
{

public:

    #pragma region Constructors and Destructors
    NullOutput(wxXmlNode* node);
    NullOutput() : Output() { _channels = 512; SetUniverse(64001); }
    NullOutput(const NullOutput& from);
    virtual ~NullOutput() override {};
    virtual wxXmlNode* Save() override;
    virtual Output* Copy() override
    {
        return new NullOutput(*this);
    }
    #pragma endregion 

    #pragma region Getters and Setters
    int GetId() const { return GetUniverse(); }
    void SetId(int id) { SetUniverse(id); }

    virtual std::string GetType() const override { return OUTPUT_NULL; }

    virtual std::string GetLongDescription() const override;
    
    virtual bool IsIpOutput() const override { return false; }
    virtual bool IsSerialOutput() const override { return false; }
    virtual bool IsOutputable() const override { return false; }
    
    virtual int32_t GetMaxChannels() const override { return 9999999; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0; }
    
    virtual std::string GetSortName() const override;
    #pragma endregion 

    #pragma region Start and Stop
    virtual bool Open() override { return true; }
    virtual void Close() override {}
    #pragma endregion 
    
    #pragma region Frame Handling
    virtual void StartFrame(long msec) override {}
    virtual void EndFrame(int suppressFrames) override {}
    virtual void ResetFrame() override {}
    #pragma endregion 
    
    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override {}
    virtual void SetManyChannels(int32_t channel, unsigned char* data, size_t size) override {};
    virtual void AllOff() override {}
    #pragma endregion 
};
