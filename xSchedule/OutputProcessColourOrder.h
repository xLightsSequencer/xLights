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

class OutputProcessColourOrder : public OutputProcess
{
    size_t _nodes;
    int _colourOrder;

    public:

        OutputProcessColourOrder(OutputManager* outputManager, wxXmlNode* node);
        OutputProcessColourOrder(OutputManager* outputManager);
        OutputProcessColourOrder(const OutputProcessColourOrder& op);
        OutputProcessColourOrder(OutputManager* outputManager, std::string _startChannel, size_t p1, size_t p2, const std::string& description);
        virtual ~OutputProcessColourOrder() {}
        virtual wxXmlNode* Save() override;
        virtual void Frame(uint8_t* buffer, size_t size) override;
        virtual size_t GetP1() const override { return _nodes; }
        virtual size_t GetP2() const override { return _colourOrder; }
        virtual std::string GetType() const override { return "Color Order"; }
};

