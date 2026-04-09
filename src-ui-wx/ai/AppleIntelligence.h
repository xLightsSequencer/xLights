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

class AppleIntelligence : public aiBase {
public:
    
    AppleIntelligence(ServiceManager* frame) :
    aiBase(frame) {
    }
    virtual ~AppleIntelligence() {}
    
    void SaveSettings() const override;
    void LoadSettings() override;
    
    void PopulateLLMSettings(wxPropertyGrid* page) override;
    void SetSetting(const std::string& key, const wxVariant& value) override;
    
    [[nodiscard]] std::pair<std::string,bool> CallLLM(const std::string& prompt) const override;
    [[nodiscard]] bool IsAvailable() const override;
    [[nodiscard]] std::string GetLLMName() const override {
        return "AppleIntelligence";
    }
    
    [[nodiscard]] std::list<aiType::TYPE> GetTypes() const override;
    
    virtual AIColorPalette GenerateColorPalette(const std::string &prompt) const override;
    
    virtual AIImageGenerator *createAIImageGenerator() const override;
};
