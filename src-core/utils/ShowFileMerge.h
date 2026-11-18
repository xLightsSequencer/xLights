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
#include <optional>
#include <string>
#include <vector>

#include <pugixml.hpp>

class LayoutRenameLog;

// Three-way merge of the show-wide files that several people can save into
// one shared show folder: xlights_rgbeffects.xml, xlights_networks.xml and the
// effect presets JSON.
//
//   base   - the file as this process last loaded or saved it
//   mine   - what this process wants to save now
//   theirs - what is on disk now
//
// The result is theirs plus everything mine changed since base, merged per
// record:
//   - records (models, groups, view objects, controllers, views, ...) are
//     matched by layout id when both versions carry one, otherwise by name
//     (an older desktop drops ids on save), so a rename is a rename and not a
//     delete plus an add;
//   - models, view objects, groups, model sets and controllers merge each
//     attribute on its own, and each keyed child (submodel, face, state,
//     dimming curve, controller connection, aliases, the controller's output
//     list) as a whole;
//   - group, view-set and model-set member lists merge as lists: adds and
//     removes from both sides apply, order follows theirs with my inserts after
//     their predecessor;
//   - views, perspectives, layout groups, viewpoints, settings, colours and
//     presets are replaced whole; palettes are matched by content;
//   - controller order (which drives start channels) merges as a list;
//   - a section one version does not write at all (an older version, or one
//     that does not know it) is kept, never deleted;
//   - the rename hints (<renames>, LayoutRenameHints) merge as a set: the
//     entries of both sides, less the ones a side pruned; never a conflict;
//   - layout ids, attribute order, whitespace and number formatting
//     ("6.0" vs "6.000000") are not changes.
// References to a renamed model, group or controller (group members, views,
// model sets, "chained" start channels, shadow models, the model's controller)
// are rewritten on every side to the merged name before merging, so a rename
// on one side and an edit that refers to the old name on the other both land.
//
// When both sides changed the same thing differently the result keeps theirs
// and a Conflict is reported; nothing is decided silently. Resolve each
// conflict (or all of them) and read the result again. When theirs equals base
// the result is mine exactly (the same bytes, for text input); when mine
// equals base it is theirs exactly.
namespace ShowFileMerge {

enum class FileKind { RgbEffects, Networks, Presets };

enum class Side { Base, Mine, Theirs };

enum class ConflictKind {
    AttributeChanged, // both changed one attribute of a record to different values
    ChildChanged,     // both changed one keyed child of a record (a submodel, a face...) differently
    RecordChanged,    // both changed a whole-record item (view, perspective, preset...) differently
    DeletedByMine,    // I deleted a record they changed
    DeletedByTheirs,  // they deleted a record I changed
    BothAdded,        // both added a record with the same name and different content
    BothRenamed,      // both renamed one record, to different names
    OrderChanged,     // both reordered the controllers, differently
    // Not found by the merge itself: the merged layout no longer has a model
    // or group the open sequence has effects on (ShowSaveMerge::Plan).
    DeletedWithEffects,
};

const char* ToString(ConflictKind kind);

enum class Resolution {
    Unresolved, // the result holds theirs
    Mine,
    Theirs,
    KeepBoth, // BothAdded only: keep theirs and add mine under a new name
    Map,      // DeletedWithEffects only: move the effects to Conflict::mapTarget
};

struct Conflict {
    // Identifies the conflict across re-runs of the same merge.
    std::string key;
    ConflictKind kind = ConflictKind::AttributeChanged;
    // The section ("models", "modelGroups", "views", "Controller", "presets"...).
    std::string section;
    // The record the conflict is in: its layout id (empty when it has none)
    // and its name on each side (empty where that side has no such record).
    std::string recordId;
    std::string baseName;
    std::string mineName;
    std::string theirsName;
    // What inside the record: the attribute name, the keyed child
    // ("subModel[Left]"), "name" for BothRenamed, empty for the whole record.
    std::string detail;
    // The three versions: attribute values, or the XML of the child / record.
    // nullopt where that side has none.
    std::optional<std::string> baseValue;
    std::optional<std::string> mineValue;
    std::optional<std::string> theirsValue;
    bool keepBothAllowed = false;
    bool mapAllowed = false;
    // Resolution::Map: the model or group the effects move to.
    std::string mapTarget;
    Resolution resolution = Resolution::Unresolved;
};

// A record that was renamed on at least one side.
struct RecordRename {
    std::string section;
    std::string id;
    std::string baseName;
    std::string mineName;
    std::string theirsName;
    std::string mergedName; // the name it has in the result
};

// A problem the merged result has even though it merged cleanly: a start
// channel chained to a model that no longer exists, a group member that is
// gone, two records with one name, ... Channel overlap needs the models
// loaded; run SequenceChecker::RunModelChecks on the reloaded result for it.
struct ReferenceIssue {
    std::string section;
    std::string record;
    std::string message;
};

// Renames of one kind of record, as old name -> merged name for each side.
struct NameMaps {
    std::map<std::string, std::string> base;
    std::map<std::string, std::string> mine;
    std::map<std::string, std::string> theirs;
    bool Empty() const { return base.empty() && mine.empty() && theirs.empty(); }
};

struct Options {
    // Renames made in this process (mine), used to match a renamed record
    // when one version has no ids. Optional.
    const LayoutRenameLog* modelRenames = nullptr;      // models and groups
    const LayoutRenameLog* viewObjectRenames = nullptr; // view objects
    const LayoutRenameLog* controllerRenames = nullptr; // controllers (networks file)
    // For the rgbeffects merge: the controller renames the networks merge
    // found (Merge::ControllerNameMaps()), so models and controller objects
    // that name a renamed controller follow it.
    NameMaps controllerNameMaps;
    // The base is not known (pass an empty one): a record only one version
    // has may have been added there or deleted from the other, so it is a
    // conflict (DeletedByTheirs when only mine has it, DeletedByMine when
    // only theirs has it) instead of being kept. For merging a copy a sync
    // client made of a file with no common version to start from.
    bool twoWay = false;
};

class Merge {
public:
    // Text input: the files' contents. A version that does not parse makes
    // the merge fail (Ok() false, Error() says which). An empty base means
    // the file did not exist when it was loaded.
    static Merge Run(FileKind kind, const std::string& base, const std::string& mine, const std::string& theirs,
                     const Options& options = {});
    // Document input (XML kinds only).
    static Merge Run(FileKind kind, const pugi::xml_document& base, const pugi::xml_document& mine,
                     const pugi::xml_document& theirs, const Options& options = {});

    Merge(Merge&&) noexcept;
    Merge& operator=(Merge&&) noexcept;
    ~Merge();

    bool Ok() const;
    const std::string& Error() const;
    FileKind Kind() const;

    // theirs == base: the result is mine as given.
    bool TheirsUnchanged() const;
    // mine == base: the result is theirs as given.
    bool MineUnchanged() const;

    const std::vector<Conflict>& Conflicts() const;
    bool HasConflicts() const { return !Conflicts().empty(); }
    bool AllResolved() const;

    // Returns false for an unknown key or a resolution the conflict does not
    // allow (KeepBoth on anything but BothAdded). The result is recomputed on
    // the next read.
    bool Resolve(const std::string& key, Resolution resolution);
    bool Resolve(size_t index, Resolution resolution);
    // Mine or Theirs for every conflict ("all mine" / "all theirs").
    void ResolveAll(Resolution resolution);

    // The merged file. For text input with TheirsUnchanged()/MineUnchanged()
    // these are the given bytes.
    std::string ResultText() const;
    // XML kinds: the merged document.
    const pugi::xml_document& ResultXml() const;

    const std::vector<RecordRename>& Renames() const;
    // Networks merge: controller renames, for Options::controllerNameMaps of
    // the rgbeffects merge.
    NameMaps ControllerNameMaps() const;

    // Structural checks on the merged result (see ReferenceIssue).
    std::vector<ReferenceIssue> ReferenceIssues() const;

private:
    struct Impl;
    explicit Merge(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> _impl;
};

// Whether two versions of a record (or section) hold the same thing, by the
// merge's rules: layout ids, attribute order, whitespace and number
// formatting are not differences.
bool RecordsEqual(FileKind kind, pugi::xml_node a, pugi::xml_node b);

// Checks a merged layout (and, when given, the merged controllers) for
// references that no longer resolve: chained start channels, shadow models,
// model chains and group members naming a missing model, models and
// controller objects naming a missing controller, duplicate model/group names,
// duplicate controller names and ids.
std::vector<ReferenceIssue> CheckReferences(const pugi::xml_document& rgbEffects, const pugi::xml_document* networks);

} // namespace ShowFileMerge
