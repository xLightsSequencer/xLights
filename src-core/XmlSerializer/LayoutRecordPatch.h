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
#include <set>
#include <string>

namespace pugi {
class xml_node;
}

// Bookkeeping for saving layout edits by patching the records of a
// xlights_rgbeffects.xml loaded from disk, rather than rewriting the file.
namespace LayoutRecordPatch {

// Records the in-memory record `name` as deleted. `renamed` maps current names to
// the name the record still has on disk; a renamed record is recorded under its
// on-disk name and its pending rename is dropped. Pending edits to it are dropped
// from `dirty`.
void MarkDeleted(const std::string& name, std::set<std::string>& deleted,
                 std::map<std::string, std::string>& renamed, std::set<std::string>& dirty);

// Removes the direct `element` children of `parent` whose name attribute is in
// `names`. Returns the number removed.
int RemoveNamed(pugi::xml_node parent, const char* element, const std::set<std::string>& names);

} // namespace LayoutRecordPatch
