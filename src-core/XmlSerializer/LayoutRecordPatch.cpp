/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LayoutRecordPatch.h"

#include <pugixml.hpp>

#include <string_view>

namespace LayoutRecordPatch {

void MarkDeleted(const std::string& name, std::set<std::string>& deleted,
                 std::map<std::string, std::string>& renamed, std::set<std::string>& dirty) {
    if (name.empty()) {
        return;
    }
    dirty.erase(name);
    // `name` itself is not un-deleted here: it may also be an on-disk record
    // deleted earlier whose name a rename then reused.
    if (auto it = renamed.find(name); it != renamed.end()) {
        deleted.insert(it->second);
        renamed.erase(it);
    } else {
        deleted.insert(name);
    }
}

int RemoveNamed(pugi::xml_node parent, const char* element, const std::set<std::string>& names) {
    if (!parent || names.empty()) {
        return 0;
    }
    int removed = 0;
    for (pugi::xml_node n = parent.first_child(); n;) {
        pugi::xml_node next = n.next_sibling();
        if (std::string_view(n.name()) == element && names.contains(n.attribute("name").as_string())) {
            parent.remove_child(n);
            ++removed;
        }
        n = next;
    }
    return removed;
}

} // namespace LayoutRecordPatch
