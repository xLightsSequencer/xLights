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
#include <variant>
#include <vector>

// Pure-data description of a user-editable property exposed by an AI service
// (or an image generator). The host app walks the vector returned by
// GetProperties() and renders each entry with its own widget toolkit:
//   - Desktop: wxPropertyGrid rows (see PropertyGridBuilder)
//   - iPad:    SwiftUI Form controls
//
// Ids are dot-qualified ("Claude.Key", "OpenAIImage.Style"); the first
// component must match the service name returned by GetLLMName() so the UI
// can route property changes back to the right service.
struct ServiceProperty {
    enum class Kind {
        Category, // Visual grouping header. Only label/category fields used.
        Bool,
        String,
        Secret,   // String with a masked/password editor. Persisted via IServiceSettingsStore::setSecret.
        Int,
        Choice    // A string chosen from a fixed list (choices).
    };

    Kind kind = Kind::String;
    std::string id;
    std::string label;
    std::string category;
    std::string help;
    std::vector<std::string> choices;
    std::variant<std::monostate, bool, int, std::string> value;
};
