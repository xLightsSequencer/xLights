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

// Google Gemini API documentation:
// https://ai.google.dev/gemini-api/docs
// OpenAI-compatible chat endpoint: https://generativelanguage.googleapis.com/v1beta/openai/chat/completions

class gemini : public aiBase {

    std::string url = "https://generativelanguage.googleapis.com/v1beta/openai/chat/completions";
    std::string model = "gemini-2.5-flash";
    std::string api_key;

public:

    explicit gemini(ServiceManager* sm) :
        aiBase(sm) {
    }
    virtual ~gemini() {}

    void SaveSettings() const override;
    void LoadSettings() override;

    void PopulateLLMSettings(wxPropertyGrid* page) override;
    void SetSetting(const std::string& key, const wxVariant& value) override;

    [[nodiscard]] std::pair<std::string, bool> CallLLM(const std::string& prompt) const override;
    [[nodiscard]] bool IsAvailable() const override;
    [[nodiscard]] std::string GetLLMName() const override {
        return "Gemini";
    }
    [[nodiscard]] std::list<aiType::TYPE> GetTypes() const override {
        return std::list({ aiType::TYPE::PROMPT });
    }
};