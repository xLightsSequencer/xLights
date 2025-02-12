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

#include <string>

class xLightsFrame;

class aiBase {

protected:
    xLightsFrame* _frame = nullptr;

public:
    aiBase(xLightsFrame* frame) : _frame(frame) {}
	virtual ~aiBase() {}

    virtual std::string CallLLM(const std::string& prompt, const std::string& token = "") const = 0;
    virtual bool TestLLM(const std::string& token = "") const = 0;
    virtual bool IsAvailable(const std::string& token = "") const = 0;
    virtual std::string GetLLMName() const = 0;
};
