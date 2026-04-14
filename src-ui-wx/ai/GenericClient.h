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

#include "OpenAIAPI.h"
#include "aiType.h"
#include <string>
#include "aiBase.h"

/*
 Generic OpenAI API V1 compatible client, set base URL to http://localhost:8000/api/v1.
 Testing with local lemonade server, https://github.com/lemonade-sdk/lemonade
*/

/*
curl http://localhost:8000/api/v1/completions -d
{'max_tokens':7,'model':'text-davinci-003','prompt':'Say this is a test','temperature':0}'
*/

class GenericClient : public OpenAIAPI {
    public:

	explicit GenericClient(ServiceManager* frame) :
            OpenAIAPI("http://localhost:8000/api/v1", "Llama-3.2-1B-Instruct-GGUF", "SD-Turbo", "Whisper-Large-v3-Turbo", "fake", frame) {
        }
    virtual ~GenericClient() = default; 

    void SaveSettings() const override;
    void LoadSettings() override;

    void PopulateLLMSettings(wxPropertyGrid* page) override;
    void SetSetting(const std::string& key, const wxVariant& value) override;

    [[nodiscard]] bool IsAvailable() const override;
    [[nodiscard]] std::string GetLLMName() const override {
        return "genericClient";
    }

	[[nodiscard]] std::list<aiType::TYPE> GetTypes() const override {
        return std::list({ aiType::TYPE::PROMPT, aiType::TYPE::COLORPALETTES, aiType::TYPE::IMAGES, aiType::TYPE::SPEECH2TEXT });
    }
};
