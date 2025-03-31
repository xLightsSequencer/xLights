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

#include "aiBase.h"

#include <string>

// https://platform.openai.com/docs/api-reference/introduction
// to get a list of models curl https://api.openai.com/v1/models -H "Authorization: Bearer YOUR_API_KEY"

class chatGPT : public aiBase {

	std::string url = "https://api.openai.com/v1/chat/completions";
	std::string model = "gpt-4o-mini";
	float temperature = 0.0;

	public:

	chatGPT(xLightsFrame* frame) : aiBase(frame) {}
	virtual ~chatGPT() {}

	std::string CallLLM(const std::string& prompt, const std::string& token = "") const override;
    bool TestLLM(const std::string& token = "") const override;
	bool IsAvailable(const std::string& token = "") const override;
    std::string GetLLMName() const override {
        return "ChatGPT";
    }
};
