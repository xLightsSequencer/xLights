#pragma once

#include "Output.h"

class NullOutput : public Output
{
    int _id;

    virtual void Save(wxXmlNode* node) override;

public:

    #pragma region Constructors and Destructors
    NullOutput(wxXmlNode* node);
    NullOutput() : Output() { _channels = 512; _id = 64001; }
    virtual ~NullOutput() override {};
    #pragma endregion Constructors and Destructors

    virtual wxXmlNode* Save() override;

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_NULL; }
    virtual std::string GetLongDescription() const override;
    virtual std::string GetPingDescription() const override { return ""; }
    virtual bool IsIpOutput() const override { return false; }
    virtual bool IsSerialOutput() const override { return false; }
    virtual bool IsOutputable() const override { return false; }
    virtual int32_t GetMaxChannels() const override { return 9999999; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0; }
    int GetId() const { return _universe; }
    void SetId(int id) { _universe = id; _dirty = true;}
    virtual std::string GetSortName() const override;
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
    virtual void SetOneChannel(int32_t channel, unsigned char data) override {}
    virtual void SetManyChannels(int32_t channel, unsigned char* data, size_t size) override {};
    virtual void AllOff() override {}
    #pragma endregion Data Setting

    #pragma region UI
#ifndef EXCLUDENETWORKUI
//    virtual Output* Configure(wxWindow* parent, OutputManager* outputManager, ModelManager *modelManager) override;
    virtual void AddProperties(wxPropertyGrid* propertyGrid, bool allSameSize) override;
    virtual bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) override;
    #endif
    #pragma endregion UI
};
