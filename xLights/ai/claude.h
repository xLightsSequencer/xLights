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

// https://docs.anthropic.com/en/docs/about-claude/models
// https://docs.anthropic.com/en/api/messages

class claude : public aiBase {

    std::string base_url = "https://api.anthropic.com/v1";
    std::string claudeModel = "claude-sonnet-4-6";
    std::string api_key;

public:
    explicit claude(ServiceManager* sm) :
        aiBase(sm) {
    }
    virtual ~claude() {}

    void SaveSettings() const override;
    void LoadSettings() override;

    void PopulateLLMSettings(wxPropertyGrid* page) override;
    void SetSetting(const std::string& key, const wxVariant& value) override;

    [[nodiscard]] std::pair<std::string, bool> CallLLM(const std::string& prompt) const override;
    [[nodiscard]] bool IsAvailable() const override;
    [[nodiscard]] std::string GetLLMName() const override {
        return "Claude";
    }
    [[nodiscard]] std::list<aiType::TYPE> GetTypes() const override {
        return std::list({ aiType::TYPE::PROMPT, aiType::TYPE::COLORPALETTES, aiType::TYPE::MAPPING });
    }

    [[nodiscard]] AIColorPalette GenerateColorPalette(const std::string& prompt) const override;
    [[nodiscard]] AIModelMappingResult GenerateModelMapping(
        const std::vector<MappingModelInfo>& sourceModels,
        const std::vector<MappingModelInfo>& targetModels,
        const std::map<std::string, std::string>& existingMappings) const override;
};
