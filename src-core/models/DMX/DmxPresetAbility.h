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

#include "../Node.h"

#include <list>
#include <string>

class Model;
class xlColor;

struct PresetSetting {
    PresetSetting(uint8_t channel, uint8_t value, std::string description) :
        DMXChannel(channel), DMXValue(value), Description(description)
    {}
    uint8_t DMXChannel;
    uint8_t DMXValue;
    std::string Description;
};

class DmxPresetAbility
{
public:
    DmxPresetAbility() {};
    virtual ~DmxPresetAbility() = default;

    static constexpr int MAX_PRESETS{ 25 };

    virtual void SetPresetValues(xlColorVector& pixelVector) const;

    [[nodiscard]] virtual std::list<std::string> CheckModelSettings(Model* m) const;
    [[nodiscard]] virtual bool IsValidModelSettings(Model* m) const;
    [[nodiscard]] std::vector<PresetSetting> const& GetPresetSettings() const { return _presets; };

    virtual void SetNodeNames(std::vector<std::string> & names) const;

    void AddPreset(uint8_t chan, uint8_t val, const std::string& desc);
    void PopPreset() { if (!_presets.empty()) _presets.pop_back(); }
    size_t GetPresetsCount() const { return _presets.size(); }
    void SetPresetChannel(size_t idx, uint8_t chan) { _presets[idx].DMXChannel = chan; }
    void SetPresetValue(size_t idx, uint8_t val) { _presets[idx].DMXValue = val; }
    void SetPresetDescription(size_t idx, const std::string& desc) { _presets[idx].Description = desc; }

private:
    std::vector<PresetSetting> _presets;
};
