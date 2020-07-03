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

class OutputProcessDeadChannel : public OutputProcess
{
    int _channel;

    public:

        OutputProcessDeadChannel(OutputManager* outputManager, wxXmlNode* node);
        OutputProcessDeadChannel(OutputManager* outputManager);
        OutputProcessDeadChannel(const OutputProcessDeadChannel& op);
        OutputProcessDeadChannel(OutputManager* outputManager, std::string _startChannel, size_t p1, const std::string& description);
        virtual ~OutputProcessDeadChannel() {}
        virtual wxXmlNode* Save() override;
        virtual void Frame(uint8_t* buffer, size_t size) override;
        virtual size_t GetP1() const override { return _channel; }
        virtual size_t GetP2() const override { return 0; }
        virtual std::string GetType() const override { return "Dead Channel"; }
};

