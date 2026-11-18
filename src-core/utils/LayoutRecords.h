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
#include <string_view>
#include <vector>

#include <pugixml.hpp>

// The record-level rules shared by everything that compares or merges two
// versions of a layout file: the base show folder update
// (ModelManager / ViewObjectManager::MergeFromBase) and the three-way merge of
// a shared show folder (ShowFileMerge).
namespace LayoutRecords {

// --- names ------------------------------------------------------------------

// A record's or child's name: its "name" attribute, else its "Name" (faces,
// states, controllers).
std::string NameOf(pugi::xml_node node);
// The attribute NameOf reads ("name" when present, else "Name").
const char* NameAttributeOf(pugi::xml_node node);
// The child of `parent` with element name `tag` and "name" attribute `name`.
pugi::xml_node FindByName(pugi::xml_node parent, std::string_view tag, std::string_view name);
// For matching children: the same "name" (or, when `a` has none, the same
// "Name") as `a`; true when `a` has neither.
bool SameName(pugi::xml_node a, pugi::xml_node b);

void SetAttribute(pugi::xml_node node, const char* name, const std::string& value);

// --- values -----------------------------------------------------------------

// Attributes that never make two versions of a record different: the layout
// id (each show holds its own) and the version of xLights that saved it.
bool IsBookkeepingAttribute(std::string_view name);

// Position, rotation, scale and length attributes. Older versions wrote them
// to 4 decimal places, so they are equal when equal at 4 decimals.
bool IsPositionAttribute(std::string_view name);
bool PositionsEqual(std::string_view a, std::string_view b);

// --- member lists (groups, views, model sets) --------------------------------

std::vector<std::string> SplitMembers(const std::string& list);
std::string JoinMembers(const std::vector<std::string>& members);
// Members of `first` then those only `second` has, each once.
std::string UnionMembers(const std::string& first, const std::string& second);
// Three-way: adds and removes from both sides apply; the order follows
// `theirs` (or `mine` when only mine reordered), with the other side's
// additions after their predecessor there, or at the end when appended.
// Absent lists are empty.
std::string MergeMembers(const std::string* base, const std::string* mine, const std::string* theirs);

// The order of a merged list of entries (indexes). `primary` is the order
// kept; an entry only `secondary` has goes after its predecessor there, or at
// the end when nothing that `primary` or `base` knows follows it. `keep` says
// which entries survive.
std::vector<int> MergeOrder(const std::vector<int>& primary, const std::vector<int>& secondary, const std::vector<int>& base,
                            const std::vector<char>& keep);

// --- base show folder updates ------------------------------------------------

enum class BaseCompare {
    Model,      // models and groups (see BaseRecordChanged)
    ViewObject, // view objects
};

// Whether the base show folder's version of a record differs from the show's
// copy of it, ignoring what is the show's own business. One-directional: an
// attribute only the show has is not a difference.
//   Model: bookkeeping attributes; positions at 4 decimals; a base with no
//     controller vs a local one; the start channel of a model wired to a local
//     controller; empty attributes old versions wrote (NodeNames, ...); the
//     show's controller connection. Children matched by name, compared with
//     the same rules, and a child the base removed is a difference.
//   ViewObject: only the id is ignored; children's attributes are compared
//     one level down; a child only the show has is not a difference.
bool BaseRecordChanged(pugi::xml_node local, pugi::xml_node base, BaseCompare rules);

// Attributes of a base model or group that belong to the show, not the base:
// they survive a base update and are the show's own in a three-way merge too.
//   model: Controller, StartChannel, ModelChain (and the ControllerConnection child)
//   modelGroup: GridCentreX, GridCentreY, selected
bool IsShowOwnedAttribute(std::string_view recordTag, std::string_view attr);
bool IsShowOwnedChild(std::string_view recordTag, std::string_view childTag);

// Copies the show's controller wiring (controller, start channel, model
// chain, port) from `local` onto `updated`, the base's version replacing it.
void KeepLocalControllerWiring(pugi::xml_node local, pugi::xml_node updated);
// Copies the show's group settings (centre offsets) from `local` onto `updated`.
void KeepLocalGroupSettings(pugi::xml_node local, pugi::xml_node updated);
void RemoveControllerConnection(pugi::xml_node node);

} // namespace LayoutRecords
