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

class OutputProcessSet : public OutputProcess
{
    size_t _channels;
    int _value;

    public:

        OutputProcessSet(OutputManager* outputManager, wxXmlNode* node);
        OutputProcessSet(OutputManager* outputManager);
        OutputProcessSet(const OutputProcessSet& op);
        OutputProcessSet(OutputManager* outputManager, std::string _startChannel, size_t p1, size_t p2, const std::string& description);
        virtual ~OutputProcessSet() {}
        virtual wxXmlNode* Save() override;
        virtual void Frame(uint8_t* buffer, size_t size) override;
        virtual size_t GetP1() const override { return _channels; }
        virtual size_t GetP2() const override { return _value; }
        virtual std::string GetType() const override { return "Set"; }
};
