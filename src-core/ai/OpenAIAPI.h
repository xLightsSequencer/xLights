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

    // Cached model list from GET {base_url}/models. Mutable so the const
    // GetAvailableModels()/GetProperties() can populate/read it. _modelsFetched
    // tracks "we already tried" so a failed/empty fetch isn't retried on every
    // GetProperties() call (iPad rebuilds the settings sheet per redraw).
    mutable std::vector<std::string> _cachedModels;
    mutable bool _modelsFetched = false;

    // Drop the cached model list so the next GetAvailableModels() re-queries.
    // Call whenever a credential/endpoint change invalidates the list.
    void clearModelCache() const {
        _cachedModels.clear();
        _modelsFetched = false;
    }

    // Build a model-picker property: a Choice populated from the cached model
    // list when one is available, otherwise a free-text String so the user can
    // type a model name before the list has been fetched.
    [[nodiscard]] ServiceProperty makeModelProperty(const std::string& id, const std::string& label,
                                                    const std::string& category, const std::string& current) const;

public:
    explicit OpenAIAPI(ServiceManager* sm) :
        aiBase(sm) {
    }
    OpenAIAPI(std::string base_url_, std::string model_, std::string image_model_, std::string transcribe_model_, std::string token_, ServiceManager* sm) :
        aiBase(sm), base_url(std::move(base_url_)), model(std::move(model_)), image_model(std::move(image_model_)), transcribe_model(std::move(transcribe_model_)), token(std::move(token_)) {
    }
    ~OpenAIAPI() override = default;

    [[nodiscard]] bool SupportsModelListing() const override { return true; }
    [[nodiscard]] std::vector<std::string> GetAvailableModels(bool forceRefresh = false) const override;

    [[nodiscard]] std::pair<std::string, bool> CallLLM(const std::string& prompt) const override;

    [[nodiscard]] AIColorPalette GenerateColorPalette(const std::string& prompt) const override;

    [[nodiscard]] AIImageGenerator* createAIImageGenerator() const override;

    [[nodiscard]] AILyricTrack GenerateLyricTrack(const std::string& audioPath) const override;
};
