/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectSymbol.h"
#include "Effect.h"

#include <pugixml.hpp>
#include <cstdlib>
#include <cstring>

EffectSymbol::EffectSymbol()
    : _id("")
    , _name("")
    , _effectType("")
    , _effectIndex(-1)
    , _palette("")
{
}

EffectSymbol::EffectSymbol(const std::string& id, const std::string& name)
    : _id(id)
    , _name(name)
    , _effectType("")
    , _effectIndex(-1)
    , _palette("")
{
}

EffectSymbol::~EffectSymbol()
{
}

void EffectSymbol::SetSettingsFromString(const std::string& settings)
{
    _settings.clear();
    if (settings.empty()) return;

    std::string remaining = settings;
    while (!remaining.empty()) {
        size_t commaPos = remaining.find(',');
        std::string pair;
        if (commaPos == std::string::npos) {
            pair = remaining;
            remaining = "";
        } else {
            pair = remaining.substr(0, commaPos);
            remaining = remaining.substr(commaPos + 1);
        }

        size_t eqPos = pair.find('=');
        if (eqPos != std::string::npos) {
            std::string key = pair.substr(0, eqPos);
            std::string value = pair.substr(eqPos + 1);
            size_t ampPos;
            while ((ampPos = value.find("&comma;")) != std::string::npos) {
                value.replace(ampPos, 7, ",");
            }
            while ((ampPos = value.find("&amp;")) != std::string::npos) {
                value.replace(ampPos, 5, "&");
            }
            _settings[key] = value;
        }
    }
}

void EffectSymbol::CopyFromEffect(const Effect* effect)
{
    if (effect == nullptr) return;

    _effectType = effect->GetEffectName();
    _effectIndex = effect->GetEffectIndex();
    // Strip per-instance X_ keys (X_Effect_Locked, X_Effect_RenderDisabled,
    // X_LinkedSymbolId, etc.) so they don't propagate from the source effect
    // out to every other linked instance when the symbol changes.
    _settings.clear();
    for (const auto& kv : effect->GetSettings()) {
        if (kv.first.compare(0, 2, "X_") == 0) continue;
        _settings[kv.first] = kv.second;
    }
    _palette = effect->GetPaletteAsString();
}

void EffectSymbol::SaveToXml(pugi::xml_node& parent) const
{
    auto node = parent.append_child("Symbol");
    node.append_attribute("id") = _id.c_str();
    node.append_attribute("name") = _name.c_str();
    node.append_attribute("effectType") = _effectType.c_str();
    node.append_attribute("effectIndex") = _effectIndex;

    auto settingsNode = node.append_child("Settings");
    settingsNode.text().set(GetSettingsAsString().c_str());

    auto paletteNode = node.append_child("Palette");
    paletteNode.text().set(_palette.c_str());
}

EffectSymbol* EffectSymbol::FromXml(const pugi::xml_node& node)
{
    if (!node || std::strcmp(node.name(), "Symbol") != 0) {
        return nullptr;
    }

    std::string id = node.attribute("id").as_string("");
    std::string name = node.attribute("name").as_string("");
    std::string effectType = node.attribute("effectType").as_string("");
    int effectIndex = (int)std::strtol(node.attribute("effectIndex").as_string("-1"), nullptr, 10);

    EffectSymbol* symbol = new EffectSymbol(id, name);
    symbol->SetEffectType(effectType);
    symbol->SetEffectIndex(effectIndex);

    for (auto child = node.first_child(); child; child = child.next_sibling()) {
        const char* cname = child.name();
        if (std::strcmp(cname, "Settings") == 0) {
            symbol->SetSettingsFromString(child.text().as_string(""));
        } else if (std::strcmp(cname, "Palette") == 0) {
            symbol->SetPalette(child.text().as_string(""));
        }
    }

    return symbol;
}
