#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "OutputProcess.h"

class OutputProcessExcludeDim : public OutputProcess
{
    size_t _channels;

public:

    OutputProcessExcludeDim(OutputManager* outputManager, wxXmlNode* node);
    OutputProcessExcludeDim(OutputManager* outputManager);
    OutputProcessExcludeDim(const OutputProcessExcludeDim& op);
    OutputProcessExcludeDim(OutputManager* outputManager, std::string _startChannel, size_t p1, const std::string& description);
    virtual ~OutputProcessExcludeDim() {}
    virtual wxXmlNode* Save() override;
    virtual void Frame(uint8_t* buffer, size_t size, std::list<OutputProcess*>& processes) override {}
    virtual size_t GetP1() const override { return _channels; }
    virtual size_t GetP2() const override { return 0; }
    virtual std::string GetType() const override { return "Exclude Dim"; }
    size_t GetFirstExcludeChannel() { return GetStartChannelAsNumber(); }
    size_t GetLastExcludeChannel() { return GetStartChannelAsNumber() + _channels - 1; }
    bool Contains(size_t sc, size_t ec);
};

