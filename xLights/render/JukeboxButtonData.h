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

#include <map>
#include <memory>
#include <string>
#include <pugixml.hpp>

// Pure-data representation of a jukebox button — no UI dependencies.
// Lives in render/ so SequenceFile can load/save jukebox state without
// pulling in any wxWidgets headers.

struct JukeboxButtonData
{
    enum class LookupType { DESCRIPTION, MLT, DISABLED };

    int number = -1;
    std::string description;
    LookupType type = LookupType::DISABLED;
    std::string element;
    std::string tooltip;
    int layer = -1;
    int time = -1;
    bool loop = true;

    JukeboxButtonData() = default;

    explicit JukeboxButtonData(const pugi::xml_node& n) {
        if (std::string_view(n.name()) == "Button") {
            element     = n.attribute("Element").as_string("");
            layer       = n.attribute("Layer").as_int(-1);
            time        = n.attribute("Time").as_int(-1);
            number      = n.attribute("Number").as_int(-1);
            description = n.attribute("Description").as_string("");
            tooltip     = n.attribute("Tooltip").as_string("");
            loop        = n.attribute("Loop").as_bool(true);
            type = (std::string_view(n.attribute("Type").as_string("")) == "DESCRIPTION")
                       ? LookupType::DESCRIPTION : LookupType::MLT;
        }
    }

    void Save(pugi::xml_node& parent) const {
        auto res = parent.append_child("Button");
        res.append_attribute("Type")        = (type == LookupType::DESCRIPTION ? "DESCRIPTION" : "MLT");
        res.append_attribute("Description") = description;
        res.append_attribute("Tooltip")     = tooltip;
        res.append_attribute("Element")     = element;
        res.append_attribute("Layer")       = layer;
        res.append_attribute("Time")        = time;
        res.append_attribute("Number")      = number;
        res.append_attribute("Loop")        = loop;
    }
};

// Convenience alias for the map that both SequenceFile and JukeboxPanel use.
using JukeboxButtonMap = std::map<int, std::unique_ptr<JukeboxButtonData>>;

// Free functions for bulk load/save of the <Jukebox> XML node.
inline void LoadJukeboxButtons(const pugi::xml_node& node, JukeboxButtonMap& buttons) {
    buttons.clear();
    if (std::string_view(node.name()) == "Jukebox") {
        for (auto n : node.children("Button")) {
            auto b = std::make_unique<JukeboxButtonData>(n);
            int num = b->number;
            buttons[num] = std::move(b);
        }
    }
}

inline void SaveJukeboxButtons(pugi::xml_node& parent, const JukeboxButtonMap& buttons) {
    auto res = parent.append_child("Jukebox");
    for (const auto& [num, btn] : buttons) {
        btn->Save(res);
    }
}
