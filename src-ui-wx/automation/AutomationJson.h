/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <map>
#include <string>

#include "nlohmann/json.hpp"

// Builds the "{"res":<code>, ...}" JSON body ProcessAutomation's sendResponse
// callback describes, shared by every caller of ProcessAutomation (the plain
// REST endpoints, POST /xlDoAutomation, and the MCP tools/call bridge) so they
// all render a command's result identically.
inline std::string BuildAutomationResponseJson(int responseCode, const std::string& jsonKey, const std::string& msg, bool isJson) {
    if (isJson) {
        if (jsonKey.empty()) {
            return "{\"res\":" + std::to_string(responseCode) + "," + msg.substr(1);
        }
        return "{\"res\":" + std::to_string(responseCode) + ",\"" + jsonKey + "\":" + msg + "}";
    }
    return "{\"res\":" + std::to_string(responseCode) + ",\"" + jsonKey + "\":\"" + msg + "\"}";
}

// Flattens a JSON object's members into the string/string param map consumed by
// xLightsFrame::ProcessAutomation: strings/numbers/bools stringify as-is, arrays
// become "key_0", "key_1", ... entries (e.g. batchRender's "seqs"), and objects are
// re-serialized to a JSON string -- setEffectSettings/addEffect's "settings" and
// "palette" are real JSON objects on the wire but ProcessAutomation reads them back
// out with nlohmann::json::parse(), so round-tripping through dump() is what those
// call sites already expect. The "cmd" key is skipped; callers pull that out
// separately since it becomes the automation command name, not a param.
inline void FlattenAutomationParams(const nlohmann::json& obj, std::map<std::string, std::string>& paramMap) {
    if (!obj.is_object()) return;
    for (auto& [mn, v] : obj.items()) {
        if (mn == "cmd") continue;
        if (v.is_string()) {
            paramMap[mn] = v.get<std::string>();
        } else if (v.is_number_integer()) {
            paramMap[mn] = std::to_string(v.get<int>());
        } else if (v.is_number_float()) {
            paramMap[mn] = std::to_string(v.get<float>());
        } else if (v.is_boolean()) {
            paramMap[mn] = v.get<bool>() ? "true" : "false";
        } else if (v.is_array()) {
            for (size_t x = 0; x < v.size(); x++) {
                paramMap[mn + "_" + std::to_string(x)] = v[x].get<std::string>();
            }
        } else if (v.is_object()) {
            paramMap[mn] = v.dump();
        }
    }
}
