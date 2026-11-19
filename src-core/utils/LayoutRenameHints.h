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

#include <ctime>
#include <functional>
#include <string>
#include <vector>

#include <pugixml.hpp>

#include "LayoutRenameLog.h"

class BaseSerializingVisitor;

// Renames of layout records, kept in xlights_rgbeffects.xml as
//
//   <renames>
//     <rename type="model" from="Arch 1" to="Arch Left" time="1763500000" xlId="..."/>
//   </renames>
//
// so that a sequence saved before a rename (here, or by someone sharing the
// show folder) can follow it when it is opened later. Models and groups are
// type "model", view objects "viewObject", controllers "controller". The
// controllers live in xlights_networks.xml, but their renames are kept here
// too: the layout file is the one every host rewrites with its own sections
// intact, and the models naming a controller live in it.
//
// Older versions ignore the section; an older desktop drops it on save, which
// only loses the hints. The show-file merge keeps the union of both sides'
// entries (minus the ones a side pruned) and never reports a conflict for it.
class LayoutRenameHints {
public:
    static constexpr const char* SECTION = "renames";
    static constexpr const char* TYPE_MODEL = "model";
    static constexpr const char* TYPE_VIEW_OBJECT = "viewObject";
    static constexpr const char* TYPE_CONTROLLER = "controller";
    // Entries older than this are dropped when the file is written.
    static constexpr int MAX_AGE_DAYS = 180;
    // The newest entries kept, so a show renamed in bulk cannot grow the file
    // without bound.
    static constexpr size_t MAX_ENTRIES = 1000;

    struct Entry {
        std::string type;
        std::string from;
        std::string to;
        std::time_t time = 0;
        std::string id;

        // Two entries are the same rename when type, from and to match.
        bool SameRename(const Entry& o) const { return type == o.type && from == o.from && to == o.to; }
        bool operator==(const Entry&) const = default;
    };

    void Clear() { _entries.clear(); }
    bool Empty() const { return _entries.empty(); }
    const std::vector<Entry>& Entries() const { return _entries; }

    // Replaces the entries with the <renames> node's (a null node clears).
    void Load(pugi::xml_node renamesNode);
    // Adds the rename, or updates the time (and id, when given) of the same
    // rename already held.
    void Add(const Entry& e);
    // Adds a manager's queued renames (LayoutRenameLog::TakeUnpersisted).
    void Absorb(const std::string& type, const std::vector<LayoutRenameLog::TimedEntry>& renames);
    // Drops entries older than maxAgeDays before `now`, then the oldest beyond
    // maxEntries. Returns how many were dropped.
    size_t Prune(std::time_t now, int maxAgeDays = MAX_AGE_DAYS, size_t maxEntries = MAX_ENTRIES);

    // Writes <renames> (nothing when empty).
    void Serialize(BaseSerializingVisitor& visitor) const;
    // Replaces the <renames> child of `root` with these entries (removes it
    // when empty).
    void WriteTo(pugi::xml_node root) const;
    // Reads every entry of a <renames> node.
    static std::vector<Entry> Read(pugi::xml_node renamesNode);

    // The three-way merge of the section: the entries either side has, less
    // those that were in base and one side no longer has (it pruned them).
    // Duplicates of one rename keep the newest time. Sorted by time.
    static std::vector<Entry> Merge(const std::vector<Entry>& base, const std::vector<Entry>& mine, const std::vector<Entry>& theirs);

    enum class Outcome {
        NotRenamed, // no hint for the name
        Mapped,     // `target` is the record the name became
        Ambiguous,  // the name was renamed to more than one thing
        Broken,     // the hints lead to nothing that exists now, or loop
    };
    struct Resolution {
        Outcome outcome = Outcome::NotRenamed;
        std::string target;
        // The names followed, from the name asked about to the target.
        std::vector<std::string> chain;
    };
    // Where a record of `type` that was called `name` is now. Follows chains in
    // time order (A->B, then B->C gives C; a B->C older than A->B renamed a
    // different B and is not followed). `exists` says whether a record of that
    // name exists now; `nameForId` (optional) gives the current name of the
    // record with a layout id (empty when none has it), which settles a step
    // whose hint carries one.
    Resolution Resolve(const std::string& type, const std::string& name, const std::function<bool(const std::string&)>& exists,
                       const std::function<std::string(const std::string&)>& nameForId = {}) const;

private:
    std::vector<Entry> _entries;
};
