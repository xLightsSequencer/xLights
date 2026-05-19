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

#include <list>
#include <string>
#include <utility>
#include <vector>

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
    int port_num{ 11434 };
    bool https{ false };

	public:

	explicit ollama(ServiceManager* frame) :
            aiBase(frame) {
        }
	~ollama() override = default;

    void SaveSettings() const override;
    void LoadSettings() override;

    [[nodiscard]] std::vector<ServiceProperty> GetProperties() const override;
    void SetProperty(const std::string& id, bool value) override;
    void SetProperty(const std::string& id, int value) override;
    void SetProperty(const std::string& id, const std::string& value) override;

	[[nodiscard]] std::pair<std::string,bool> CallLLM(const std::string& prompt) const override;
    [[nodiscard]] bool IsAvailable() const override;
    [[nodiscard]] std::string GetLLMName() const override {
        return "ollama";
    }

	[[nodiscard]] std::list<aiType::TYPE> GetTypes() const override {
        return std::list({ aiType::TYPE::PROMPT, aiType::TYPE::COLORPALETTES });
    }

    [[nodiscard]] AIColorPalette GenerateColorPalette(const std::string& prompt) const override;
};
