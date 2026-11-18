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

#include <set>
#include <string>
#include <utility>
#include <vector>

#include <pugixml.hpp>

#include "utils/ShowSaveMerge.h"

class xLightsShowContext;

// After a save merged someone else's changes into the show files, brings what
// is in memory up to the merged files without closing the open sequence, so
// the next save starts from them instead of writing the old state back.
namespace ShowMergeApply {

struct LayoutChanges {
    std::vector<std::string> modelsAdded;   // models and groups, by merged name
    std::vector<std::string> modelsChanged; // replaced (includes renamed)
    std::vector<std::string> modelsRemoved;
    std::vector<std::pair<std::string, std::string>> modelsRenamed; // old, new
    // Of the removed models, those the open sequence had effects on (the
    // effects went with the model).
    std::vector<std::string> removedWithEffects;
    std::vector<std::string> objectsChanged; // added, replaced or renamed
    std::vector<std::string> objectsRemoved;
    bool viewsReloaded = false;
    bool modelSetsReloaded = false;
    // Sections that differ that only the host holds (settings, colors,
    // Viewpoints, perspectives, layoutGroups, palettes, ...).
    std::set<std::string> otherSections;

    bool ModelsChanged() const { return !modelsAdded.empty() || !modelsChanged.empty() || !modelsRemoved.empty(); }
    bool ObjectsChanged() const { return !objectsChanged.empty() || !objectsRemoved.empty(); }
    bool Any() const { return ModelsChanged() || ObjectsChanged() || viewsReloaded || modelSetsReloaded || !otherSections.empty(); }
    // Lines for the save report: what came in, and what needs attention.
    std::vector<std::string> Describe() const;
    std::vector<std::string> Warnings() const;
};

// Updates the models, groups, view objects, model sets and sequencer views
// from `mine` (the layout file this process wanted to save, i.e. what memory
// holds) to `result` (the merged file that was written). Records are matched
// by layout id, else by name; only the ones that differ are rebuilt, through
// ModelManager::ReplaceModel / Delete, so the open sequence keeps its
// elements (a renamed model's element is renamed with it).
//
// The caller must have stopped rendering (AbortRender returned true): this
// frees the models it replaces.
LayoutChanges ApplyLayout(xLightsShowContext& ctx, const pugi::xml_document& mine, const pugi::xml_document& result);

// How the open sequence uses each model and group (for Plan::Prepare): its
// row, and the effects on the row, its layers and its submodel, strand and
// node rows. Group rows count like model rows.
ShowSaveMerge::SequenceUse SequenceUseOf(xLightsShowContext& ctx);

// Applies the decided deletions (Plan::SequenceChanges) to the open sequence
// as one undo step, before the layout reaches memory: Delete removes the row
// with its effects; Map gives the row to the target model or group (rows of
// submodels or strands the target does not have are dropped). Undo puts the
// row back. Returns the number of effects that were dropped by a Map.
int ApplySequenceChanges(xLightsShowContext& ctx, const std::vector<ShowSaveMerge::SequenceChange>& changes);

// Reloads the controllers from the controllers file just written. Output to
// the lights is stopped around the reload and restarted when it was running.
bool ReloadControllers(xLightsShowContext& ctx);
// Same, from the merged text instead of the file. With keepRenameLog the
// controller renames this process made (and has not saved) are kept.
bool ReloadControllers(xLightsShowContext& ctx, const std::string& text, bool keepRenameLog);

// The model check messages (SequenceChecker::RunModelChecks) for the show as
// it is in memory, and those of `after` that `before` did not have.
std::vector<std::string> ModelCheckMessages(xLightsShowContext& ctx);
std::vector<std::string> NewMessages(const std::vector<std::string>& before, const std::vector<std::string>& after);

} // namespace ShowMergeApply
