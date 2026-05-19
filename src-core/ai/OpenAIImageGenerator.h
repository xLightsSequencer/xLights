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

#include <functional>
#include <string>

class OpenAIImageGenerator : public aiBase::AIImageGenerator {
private:
    std::string base_url;
    std::string token;
    std::string image_model;

public:
    OpenAIImageGenerator(std::string base_url_, std::string token_, std::string modelName) :
        base_url(std::move(base_url_)), token(std::move(token_)), image_model(std::move(modelName)) {
    }

    ~OpenAIImageGenerator() override = default;

    void generateImage(const std::string& prompt,
                       std::function<void(aiBase::AIImageResult)> callback) override;
};
