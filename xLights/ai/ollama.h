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

/*
curl http://localhost:11434/api/generate -d '{
  "model": "llama3.2",
  "prompt":"Why is the sky blue?"
}'
*/

class ollama : public aiBase {

    const std::string api = "/api/generate";
	std::string model = "deepseek-r1";
    std::string host = "localhost";
    //std::string host = "http://localhost:11434";
    int port_num{ 11434 };
	//float temperature = 0.0;
    bool https{ false };

	public:

	explicit ollama(ServiceManager* frame) :
            aiBase(frame) {
        }
	virtual ~ollama() {}

    void SaveSettings() const override;
    void LoadSettings() override;

    void PopulateLLMSettings(wxPropertyGrid* page) override;
    void SetSetting(const std::string& key, const wxVariant& value) override;  

	[[nodiscard]] std::pair<std::string,bool> CallLLM(const std::string& prompt) const override;
    [[nodiscard]] bool IsAvailable() const override;
    [[nodiscard]] std::string GetLLMName() const override {
        return "ollama";
    }

	[[nodiscard]] aiType::TYPE GetLLMType() const override {
        return aiType::TYPE::PROMPT;
    }
};
