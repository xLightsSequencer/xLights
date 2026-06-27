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
#include <vector>

class OpenAIImageGenerator : public aiBase::AIImageGenerator {
private:
    std::string base_url;
    std::string token;
    std::string image_model;
    // Requested output size. "auto" lets the server choose (and is omitted from
    // the request); any concrete WxH is sent as the API "size" parameter.
    std::string image_size = "auto";
    // Diffusion steps (Stable Diffusion / SD-Turbo). "auto" is omitted from the
    // request; a concrete value is sent as the "steps" parameter.
    std::string image_steps = "auto";
    // Models the server reported (from the parent service's cached /models
    // list). When non-empty the image dialog shows a model dropdown; otherwise
    // the model is fixed to whatever was passed in.
    std::vector<std::string> available_models;

public:
    OpenAIImageGenerator(std::string base_url_, std::string token_, std::string modelName,
                         std::vector<std::string> availableModels = {}) :
        base_url(std::move(base_url_)), token(std::move(token_)), image_model(std::move(modelName)),
        available_models(std::move(availableModels)) {
    }

    ~OpenAIImageGenerator() override = default;

    [[nodiscard]] std::vector<ServiceProperty> GetProperties() const override;
    void SetProperty(const std::string& id, const std::string& value) override;

    void generateImage(const std::string& prompt,
                       std::function<void(aiBase::AIImageResult)> callback) override;
};
