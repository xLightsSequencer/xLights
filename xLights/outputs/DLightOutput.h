#ifndef DLIGHTOUTPUT_H
#define DLIGHTOUTPUT_H

#include "LOROutput.h"

// This is aparently the same as LOR Output

class DLightOutput : public LOROutput
{
public:
    DLightOutput(wxXmlNode* node) : LOROutput(node) {};
    DLightOutput(SerialOutput* output) : LOROutput(output) {};
    DLightOutput() : LOROutput() {};
    virtual ~DLightOutput() override {};
    virtual std::string GetType() const override { return OUTPUT_DLIGHT; }
};

 #endif