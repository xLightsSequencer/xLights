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

#include <string>
#include "../utils/UtilClasses.h"

namespace pugi { class xml_node; }
class Effect;

// An EffectSymbol is a reusable effect definition that can be linked to multiple Effect instances.
// When the symbol is modified, all linked effects automatically inherit the changes.
// Symbols store the effect type, settings, and palette - but NOT timing (start/end time).
class EffectSymbol
{
public:
    EffectSymbol();
    EffectSymbol(const std::string& id, const std::string& name);
    ~EffectSymbol();

    const std::string& GetId() const { return _id; }
    const std::string& GetName() const { return _name; }
    void SetName(const std::string& name) { _name = name; }

    const std::string& GetEffectType() const { return _effectType; }
    void SetEffectType(const std::string& type) { _effectType = type; }
    int GetEffectIndex() const { return _effectIndex; }
    void SetEffectIndex(int index) { _effectIndex = index; }

    const SettingsMap& GetSettings() const { return _settings; }
    SettingsMap& GetSettings() { return _settings; }
    void SetSettings(const SettingsMap& settings) { _settings = settings; }
    std::string GetSettingsAsString() const { return _settings.AsString(); }
    void SetSettingsFromString(const std::string& settings);

    const std::string& GetPalette() const { return _palette; }
    void SetPalette(const std::string& palette) { _palette = palette; }

    void CopyFromEffect(const Effect* effect);

    void SaveToXml(pugi::xml_node& parent) const;
    static EffectSymbol* FromXml(const pugi::xml_node& node);

private:
    std::string _id;
    std::string _name;
    std::string _effectType;
    int _effectIndex;
    SettingsMap _settings;
    std::string _palette;
};
