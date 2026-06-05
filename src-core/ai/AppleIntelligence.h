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

#include "ai/aiBase.h"
#include "ai/aiType.h"

#include <list>
#include <string>
#include <utility>
#include <vector>

class AppleIntelligence : public aiBase {
public:

    explicit AppleIntelligence(ServiceManager* frame) :
        aiBase(frame) {
    }
    ~AppleIntelligence() override = default;

    void SaveSettings() const override;
    void LoadSettings() override;

    [[nodiscard]] std::vector<ServiceProperty> GetProperties() const override;
    void SetProperty(const std::string& id, bool value) override;

    [[nodiscard]] std::pair<std::string,bool> CallLLM(const std::string& prompt) const override;
    [[nodiscard]] bool IsAvailable() const override;
    [[nodiscard]] std::string GetLLMName() const override {
        return "AppleIntelligence";
    }

    [[nodiscard]] std::list<aiType::TYPE> GetTypes() const override;

    [[nodiscard]] AIColorPalette GenerateColorPalette(const std::string &prompt) const override;

    [[nodiscard]] AIImageGenerator *createAIImageGenerator() const override;

    [[nodiscard]] AILyricTrack GenerateLyricTrack(const std::string& audioPath) const override;
};
