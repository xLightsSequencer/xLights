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

#include <functional>
#include <string>
#include <string_view>
#include <unordered_set>

#include <pugixml.hpp>

// Permanent ids for the records of a show's layout files (models, submodels,
// groups, view objects, views, perspectives and layout groups in
// xlights_rgbeffects.xml; controllers in xlights_networks.xml).
//
// A record keeps its id for its whole life - through renames and through any
// rebuild of the in-memory object from its XML - so two copies of a layout file
// can be matched record by record even when names differ. Anything that makes a
// COPY of a record gets a new id. Ids only have to be unique within one file.
//
// Older versions ignore the attribute on load. The older desktop drops it on
// save because it rebuilds the file from memory; anything matching records
// must therefore fall back to the name when one side has no id.
namespace LayoutObjectId {

inline constexpr char const* ATTRIBUTE = "xlId";

// A new RFC 4122 v4 UUID, lower-case, 36 characters.
std::string Generate();

// True for the id attribute. Code comparing two versions of a record for
// changes must skip it: two shows hold different ids for the same base record.
inline bool IsIdAttribute(std::string_view attrName) { return attrName == ATTRIBUTE; }

// Makes `id` usable: a fresh id when it is empty or already in `used`. The
// final id is added to `used`. Returns true when a new id was assigned.
bool Claim(std::string& id, std::unordered_set<std::string>& used);

// The id stored on a record node, empty when there is none.
std::string Read(pugi::xml_node node);

// Sets (or, for an empty id, removes) the id attribute on a record node.
void Write(pugi::xml_node node, const std::string& id);

// Removes the id from `node` and every element below it. Used on XML that is
// about to become a new record (a paste, an import), so it cannot carry the
// source record's identity.
void StripFromXml(pugi::xml_node node);

// For a node that is replacing `local` as the same record (a base show update):
// gives `replacement` the local record's id, and each of its sub-records the id
// of the local sub-record with the same name (or none, so one is assigned on
// load). Ids that came with `replacement` are dropped.
void CopyLocalIds(pugi::xml_node local, pugi::xml_node replacement,
                  char const* subRecordName, char const* nameAttribute = "name");

// For each `childName` element of `parent`: when `idForName` knows the
// element's name, writes that id onto it. Then assigns a fresh id to any
// element whose id is still empty or repeats an earlier one in `used`.
// `idForName` may be empty to only fill and de-duplicate.
void StampChildren(pugi::xml_node parent, char const* childName,
                   std::function<std::string(std::string const&)> const& idForName,
                   std::unordered_set<std::string>& used,
                   char const* nameAttribute = "name");

} // namespace LayoutObjectId
