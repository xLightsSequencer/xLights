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
#include <utility>

class OpenAIAPI : public aiBase {

protected:
	std::string base_url = "https://api.openai.com/v1";
    std::string model = "gpt-4o-mini";
    std::string image_model = "gpt-image-1";
    std::string transcribe_model = "gpt-4o-transcribe";
    std::string token;

public:
    OpenAIAPI(ServiceManager* sm) :
        aiBase(sm) {
    }
    OpenAIAPI(std::string base_url_, std::string model_, std::string image_model_, std::string transcribe_model_, std::string token_, ServiceManager* sm) :
        aiBase(sm), base_url(std::move(base_url_)), model(std::move(model_)), image_model(std::move(image_model_)), transcribe_model(std::move(transcribe_model_)), token(std::move(token_)) {
    }
    virtual ~OpenAIAPI() {
    }

    [[nodiscard]] std::pair<std::string, bool> CallLLM(const std::string& prompt) const override;

    [[nodiscard]] virtual AIColorPalette GenerateColorPalette(const std::string& prompt) const override;

    [[nodiscard]] virtual AIImageGenerator* createAIImageGenerator() const override;

    [[nodiscard]] AILyricTrack GenerateLyricTrack(const std::string& audioPath) const override;
};