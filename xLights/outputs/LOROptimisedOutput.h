#ifndef LOROPTIMISEDOUTPUT_H
#define LOROPTIMISEDOUTPUT_H

#include "LOROutput.h"
#include "LorControllers.h"

// Should be called with: 0 <= chindex <= 3839 (max channels=240*16)

struct LORDataPair {
    uint8_t value;
    wxWord bits;
    LORDataPair( uint8_t value_, wxWord bits_ )
    : value(value_), bits(bits_)
    {
    }
};

class LOROptimisedOutput : public LOROutput
{
    static const unsigned int MAX_BANKS = 64;
    #pragma region Member Variables
    uint8_t _unit_id = 0x01;
    bool banks_changed[MAX_BANKS];
    bool unit_id_in_use[256];
    uint8_t _curData[LOR_MAX_CHANNELS];
    LorControllers _controllers;
    #pragma endregion Member Variables

    virtual void Save(wxXmlNode* node) override;
    void CalcTotalChannels();
	void CalcChannels(int& channel_count, int& channels_per_pass, int& controller_channels_to_process, LorController* cntrl);

public:

    #pragma region Constructors and Destructors
    LOROptimisedOutput(wxXmlNode* node);
    LOROptimisedOutput(SerialOutput* output);
    LOROptimisedOutput();
    virtual ~LOROptimisedOutput() override;
    void SetupHistory();
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_LOR_OPT; }
    virtual std::string GetSetupHelp() const override;
    virtual std::string GetExport() const override;
    #pragma endregion Getters and Setters

    #pragma region Frame Handling
    virtual void EndFrame(int suppressFrames) override;
    #pragma endregion Frame Handling

    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char data[], size_t size) override;
    virtual void AllOff() override;
    void GenerateCommand(uint8_t d[], size_t& idx, int unit_id, int bank, bool value_byte, uint8_t dbyte, uint8_t lsb, uint8_t msb);
    #pragma endregion Data Setting

#ifndef EXCLUDENETWORKUI
    virtual Output* Configure(wxWindow* parent, OutputManager* outputManager, ModelManager* modelManager) override;
#endif
};

#endif
