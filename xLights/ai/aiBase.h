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

#include "aiType.h"

#include <list>
#include <string>
#include <utility>
#include <vector>

class ServiceManager;
class wxPropertyGrid;
class wxVariant;

class aiBase {
protected:
    ServiceManager* _sm = nullptr;
    bool _enabled{ false };

public:
    explicit aiBase(ServiceManager* sm);
    virtual ~aiBase() {
    }

    virtual void SaveSettings() const = 0;
    virtual void LoadSettings() = 0;

    [[nodiscard]] virtual std::string GetLLMName() const = 0;
    [[nodiscard]] virtual bool IsEnabled() const {
        return _enabled;
    };
    virtual void SetEnabled(bool enabled) {
        _enabled = enabled;
    }
    [[nodiscard]] virtual bool IsAvailable() const = 0;

    [[nodiscard]] virtual std::list<aiType::TYPE> GetTypes() const = 0;

    [[nodiscard]] virtual std::pair<std::string, bool> CallLLM(const std::string& prompt) const = 0;
    [[nodiscard]] virtual std::pair<std::string, bool> TestLLM() const;

    virtual void PopulateLLMSettings(wxPropertyGrid* page) = 0;
    virtual void SetSetting(const std::string& key, const wxVariant& value) = 0;

    // For ColorPalette generation:
    struct AIColor {
        std::string hexValue;
        std::string name;
        std::string description;
    };

    struct AIColorPalette {
        std::string description;
        std::vector<AIColor> colors;

        std::string error;
    };

    virtual AIColorPalette GenerateColorPalette(const std::string& /* prompt*/) const {
        return AIColorPalette();
    }
};
