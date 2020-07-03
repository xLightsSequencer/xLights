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

#include "LOROutput.h"
#include "LorControllers.h"

#include <wx/propgrid/advprops.h>

// Should be called with: 0 <= chindex <= 3839 (max channels=240*16)

class LOROptimisedOutput : public LOROutput
{
    static const unsigned int MAX_BANKS = 64;
    static wxPGChoices __lorDeviceTypes;
    static wxPGChoices __lorAddressModes;

    #pragma region Member Variables
    uint8_t _unit_id = 0x01;
    bool banks_changed[MAX_BANKS];
    bool unit_id_in_use[256];
    uint8_t _curData[LOR_MAX_CHANNELS];
    LorControllers _controllers;
    #pragma endregion Member Variables

    #pragma region Private Functions
    void InitialiseTypes();
    virtual void Save(wxXmlNode* node) override;
	void CalcChannels(int& channel_count, int& channels_per_pass, int& controller_channels_to_process, LorController* cntrl);
    void CalcTotalChannels();
    void GenerateCommand(uint8_t d[], size_t& idx, int unit_id, int bank, bool value_byte, uint8_t dbyte, uint8_t lsb, uint8_t msb);
    #pragma endregion

public:

    #pragma region Constructors and Destructors
    LOROptimisedOutput(wxXmlNode* node);
    LOROptimisedOutput(SerialOutput* output);
    LOROptimisedOutput();
    virtual ~LOROptimisedOutput() override {}
    void SetupHistory();
    #pragma endregion 

    #pragma region Getters and Setters
    LorControllers& GetControllers() { return _controllers; }

    virtual std::string GetType() const override { return OUTPUT_LOR_OPT; }
    #pragma endregion 

    #pragma region Frame Handling
    virtual void EndFrame(int suppressFrames) override;
    #pragma endregion 

    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char* data, size_t size) override;
    virtual void AllOff() override;
    #pragma endregion 

    #pragma region UI
    #ifndef EXCLUDENETWORKUI
    virtual void AddProperties(wxPropertyGrid* propertyGrid, bool allSameSize) override;
    virtual bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) override;
    #endif
    #pragma endregion
};
