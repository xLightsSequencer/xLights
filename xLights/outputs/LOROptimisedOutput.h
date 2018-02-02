#ifndef LOROPTIMISEDOUTPUT_H
#define LOROPTIMISEDOUTPUT_H

#include "LOROutput.h"
#include "LorControllers.h"

// Should be called with: 0 <= chindex <= 3839 (max channels=240*16)

struct LORDataPair {
    wxByte value;
    wxWord bits;
    LORDataPair( wxByte value_, wxWord bits_ )
    : value(value_), bits(bits_)
    {
    }
};

class LOROptimisedOutput : public LOROutput
{
    #pragma region Member Variables
    wxByte _unit_id = 0x01;
    bool banks_changed[32];
    LorControllers _controllers;
    #pragma endregion Member Variables

    virtual void Save(wxXmlNode* node) override;
    void CalcTotalChannels();

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
    #pragma endregion Getters and Setters

    #pragma region Data Setting
    virtual void SetManyChannels(long channel, unsigned char data[], long size) override;
    virtual void AllOff() override;
    void GenerateCommand(wxByte d[], size_t& idx, int unit_id, int bank, bool value_byte, wxByte dbyte, wxByte lsb, wxByte msb);
    #pragma endregion Data Setting

#ifndef EXCLUDENETWORKUI
    virtual Output* Configure(wxWindow* parent, OutputManager* outputManager) override;
#endif
};

#endif
