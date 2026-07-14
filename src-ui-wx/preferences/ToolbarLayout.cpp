/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ToolbarLayout.h"

#include "settings/XLightsConfigAdapter.h"

#include <nlohmann/json.hpp>

#include <algorithm>

std::vector<std::pair<std::string, bool>> LoadToolbarLayout(XLightsConfigAdapter* cfg, const std::string& key,
                                                              const std::vector<std::string>& currentNames)
{
    std::vector<std::pair<std::string, bool>> saved;

    std::string raw;
    if (cfg != nullptr && cfg->Read(key, &raw) && !raw.empty()) {
        nlohmann::json j = nlohmann::json::parse(raw, nullptr, false);
        if (!j.is_discarded() && j.is_array()) {
            for (const auto& entry : j) {
                if (!entry.is_object()) continue;
                saved.emplace_back(entry.value("name", std::string()), entry.value("visible", true));
            }
        }
    }

    std::vector<std::pair<std::string, bool>> result;
    for (const auto& entry : saved) {
        bool alreadyAdded = std::any_of(result.begin(), result.end(),
                                         [&entry](const auto& e) { return e.first == entry.first; });
        if (alreadyAdded) continue; // de-dupe a malformed/hand-edited config, keep first occurrence
        if (std::find(currentNames.begin(), currentNames.end(), entry.first) != currentNames.end()) {
            result.push_back(entry);
        }
    }
    for (const auto& name : currentNames) {
        bool alreadyPresent = std::any_of(result.begin(), result.end(),
                                           [&name](const auto& e) { return e.first == name; });
        if (!alreadyPresent) {
            result.emplace_back(name, true);
        }
    }
    return result;
}

void SaveToolbarLayout(XLightsConfigAdapter* cfg, const std::string& key,
                        const std::vector<std::pair<std::string, bool>>& layout)
{
    if (cfg == nullptr) return;

    nlohmann::json j = nlohmann::json::array();
    for (const auto& entry : layout) {
        j.push_back({ { "name", entry.first }, { "visible", entry.second } });
    }
    cfg->Write(key, j.dump());
}
