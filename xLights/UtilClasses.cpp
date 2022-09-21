#include "UtilClasses.h"
#include "effects/RenderableEffect.h"
#include "effects/EffectManager.h"

void MapStringString::ParseJson(EffectManager* effectManager, const std::string& str, const std::string& effectName)
{
    clear();
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
        if (effectManager != nullptr)
            value = RenderableEffect::UpgradeValueCurve(effectManager, name, value, effectName);
        if (!name.empty()) {
            (*this)[name] = value;
        }
    }
}

void MapStringString::Parse(EffectManager* effectManager, const std::string& str, const std::string& effectName)
{
    clear();
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
        ReplaceAll(value, "&comma;", ","); // unescape the commas
        ReplaceAll(value, "&amp;", "&");   // unescape the amps

        RemapKey(name, value);
        if (effectManager != nullptr)
            value = RenderableEffect::UpgradeValueCurve(effectManager, name, value, effectName);
        if (!name.empty()) {
            (*this)[name] = value;
        }
    }
}
