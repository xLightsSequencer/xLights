#include "UtilClasses.h"
#include "../effects/RenderableEffect.h"
#include "../effects/EffectManager.h"
#include <regex>

namespace {
// Detect a setting key embedded inside another setting's value — the result
// of a long-standing missing-comma bug between GetTimingString() and
// GetBufferString() that fused the last T_ setting into the first B_ setting.
// Each save-then-load round appended one more B_/C_ fragment, so a corrupted
// value can contain several embedded keys. We split the value at the first
// embedded key and prepend the rest back onto the unparsed remainder so the
// outer parse loop picks them up as their own tokens.
//
// The regex matches the canonical key shapes the framework writes:
//   B_/C_/T_/E_  +  one of the known control-type prefixes  +  _ID...=
const std::regex kEmbeddedKeyRe(
    R"([BCTE]_(?:SLIDER|VALUECURVE|CHOICE|CHECKBOX|TEXTCTRL|SPINCTRL|RADIOBUTTON|TOGGLEBUTTON|FILEPICKER|0FILEPICKER|FONTPICKER|CUSTOM|NOTEBOOK|PANEL)_[A-Za-z0-9_]+=)");

// If `value` contains an embedded key, truncate value at that point and
// prepend the embedded portion back onto `remainder` (the still-unparsed tail
// of the settings string). Returns true if a split occurred.
bool RepairEmbeddedKey(std::string& value, std::string& remainder) {
    std::smatch m;
    if (!std::regex_search(value, m, kEmbeddedKeyRe)) return false;
    size_t pos = m.position(0);
    if (pos == 0) return false; // nothing legitimate before the embedded key
    std::string tail = value.substr(pos);
    value.resize(pos);
    if (!remainder.empty() && remainder.front() != ',') {
        remainder = tail + "," + remainder;
    } else if (remainder.empty()) {
        remainder = tail;
    } else {
        remainder = tail + remainder;
    }
    return true;
}
} // namespace

void SettingsMap::ParseJson(EffectManager* effectManager, const std::string& str, const std::string& effectName)
{
    _internal.clear();
    std::string before, after, name, value;
    std::string settings(str);
    ReplaceAll(settings, "{", "");
    ReplaceAll(settings, "}", "");
    while (!settings.empty()) {
        size_t start_pos = settings.find(',');
        if (start_pos != std::string::npos) {
            before = settings.substr(0, start_pos);
            settings = settings.substr(start_pos + 1);
        } else {
            before = settings;
            settings = "";
        }

        start_pos = before.find(':');
        name = before.substr(0, start_pos);
        value = before.substr(start_pos + 1);
        ReplaceAll(name, "\"", "");
        ReplaceAll(value, "\"", "");
        Trim(name);
        Trim(value);
        ReplaceAll(value, "&comma;", ","); // unescape the commas
        ReplaceAll(value, "&amp;", "&");   // unescape the amps

        RemapKey(name, value);
        if (effectManager != nullptr) {
            value = RenderableEffect::UpgradeValueCurve(effectManager, name, value, effectName);
        }
        if (!name.empty()) {
            _internal.emplace(name, value);
        }
    }
}

void SettingsMap::Parse(EffectManager* effectManager, const std::string& str, const std::string& effectName)
{
    _internal.clear();
    std::string before, after, name, value;
    std::string settings(str);
    while (!settings.empty()) {
        size_t start_pos = settings.find(',');
        if (start_pos != std::string::npos) {
            before = settings.substr(0, start_pos);
            settings = settings.substr(start_pos + 1);
        } else {
            before = settings;
            settings = "";
        }

        start_pos = before.find('=');
        name = before.substr(0, start_pos);
        value = before.substr(start_pos + 1);
        RepairEmbeddedKey(value, settings);
        ReplaceAll(value, "&comma;", ","); // unescape the commas
        ReplaceAll(value, "&amp;", "&");   // unescape the amps

        RemapKey(name, value);
        if (effectManager != nullptr) {
            value = RenderableEffect::UpgradeValueCurve(effectManager, name, value, effectName);
        }
        if (!name.empty()) {
            _internal.emplace(name, value);
        }
    }
}
