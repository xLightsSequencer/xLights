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

#include "OutputProcess.h"

class OutputProcessSustain : public OutputProcess
{
    size_t _channels;
    uint8_t* _save;

    public:

        OutputProcessSustain(OutputManager* outputManager, wxXmlNode* node);
        OutputProcessSustain(OutputManager* outputManager);
        OutputProcessSustain(const OutputProcessSustain& op);
        OutputProcessSustain(OutputManager* outputManager, std::string _startChannel, size_t p1, const std::string& description);
        virtual ~OutputProcessSustain();
        virtual wxXmlNode* Save() override;
        virtual void Frame(uint8_t* buffer, size_t size) override;
        virtual size_t GetP1() const override { return _channels; }
        virtual size_t GetP2() const override { return 0; }
        virtual std::string GetType() const override { return "Sustain"; }
};

