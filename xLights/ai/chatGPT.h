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
#include "aiType.h"
#include <string>

// https://platform.openai.com/docs/api-reference/introduction
// to get a list of models curl https://api.openai.com/v1/models -H "Authorization: Bearer YOUR_API_KEY"

class chatGPT : public aiBase {

	std::string url = "https://api.openai.com/v1/chat/completions";
	std::string model = "gpt-4o-mini";
    std::string bearer_token;
	float temperature = 0.0;

	public:

	explicit chatGPT(ServiceManager* sm) :
            aiBase(sm) {
        }
	virtual ~chatGPT() {}

    void SaveSettings() const override;
    void LoadSettings() override;

    void PopulateLLMSettings(wxPropertyGrid* page) override;
    void SetSetting(const std::string& key, const wxVariant& value) override;

	[[nodiscard]] std::pair<std::string, bool> CallLLM(const std::string& prompt) const override;

    std::vector<wxColour> CallLLMForColors(const std::string& prompt) const override;
    [[nodiscard]] bool IsAvailable() const override;
    [[nodiscard]] std::string GetLLMName() const override {
        return "ChatGPT";
    }
    [[nodiscard]] aiType::TYPE GetLLMType() const override {
        return aiType::TYPE::PROMPT;
    }
};
