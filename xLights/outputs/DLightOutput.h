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

// This is apparently the same as LOR Output
class DLightOutput : public LOROutput
{
public:

    #pragma region Constructors and Destructors
    DLightOutput(wxXmlNode* node) : LOROutput(node) {};
    DLightOutput(const DLightOutput& from) : LOROutput(from) {};
    DLightOutput() : LOROutput() {};
    virtual ~DLightOutput() override {};
    virtual Output* Copy() override
    {
        return new DLightOutput(*this);
    }
    #pragma endregion 

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_DLIGHT; }
    //virtual std::string GetSetupHelp() const override { return "D-Light controllers attached to a D-Light dongle.\nMax of 8 channels at 9600 baud.\nMax of 48 channels at 57600 baud.\nMax of 96 channels at 115200 baud.\nRun your controllers in DMX mode for higher throughput."; }
    #pragma endregion 
};
