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

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;
class Model;
class xlColor;
class wxFile;

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

    virtual void AddProperties(wxPropertyGridInterface* grid, int num_channels);
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, int num_channels, BaseObject* base);
    [[nodiscard]] virtual std::list<std::string> CheckModelSettings(Model* m) const;
    [[nodiscard]] virtual bool IsValidModelSettings(Model* m) const;
    [[nodiscard]] std::vector<PresetSetting> const& GetPresetSettings() const { return _presets; };

    virtual void SetNodeNames(std::vector<std::string> & names) const;

    void AddPreset(uint8_t chan, uint8_t val, const std::string& desc);

private:
    std::vector<PresetSetting> _presets;
    void WriteXMLSettings(wxXmlNode* ModelXml) const;
};
