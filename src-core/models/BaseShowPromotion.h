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
#include <vector>

class ModelManager;
class OutputManager;
class ViewObjectManager;

// Copies controllers, models, groups and view objects that live only in the
// current show folder into its base show folder, then links the local copies
// to the base (FromBase) so later base merges keep them in step. The base
// folder's files are loaded, patched and written back in place; nothing else
// in them is touched and the base show never has to be opened.
namespace BaseShowPromotion {

struct Plan {
    // Everything that will be written, including what the selection pulled in.
    std::vector<std::string> controllers;
    std::vector<std::string> models;
    std::vector<std::string> groups;
    std::vector<std::string> objects;

    // Not selected, but promoted because something selected depends on them
    // (a model's controller, a chained start channel, a group member, a
    // controller's layout box).
    std::vector<std::string> pulledIn;
    // Already present in the base folder; promoting replaces the base's copy.
    std::vector<std::string> replaced;
    // Promoted anyway, but worth telling the user about.
    std::vector<std::string> warnings;

    bool empty() const { return controllers.empty() && models.empty() && groups.empty() && objects.empty(); }
    size_t size() const { return controllers.size() + models.size() + groups.size() + objects.size(); }
};

// modelNames may mix models and groups; submodel names promote their parent.
Plan BuildPlan(const OutputManager& om, const ModelManager& models, const ViewObjectManager& objects,
               const std::vector<std::string>& controllerNames,
               const std::vector<std::string>& modelNames,
               const std::vector<std::string>& objectNames);

// Human readable summary of a plan for a confirmation prompt.
std::string Describe(const Plan& plan, const std::string& baseShowDir);

// Writes the plan into the base folder and marks the local copies FromBase.
// The caller must have write access to the base folder and must mark the
// current show's networks/layout dirty afterwards so the FromBase flags save.
bool Apply(const Plan& plan, OutputManager& om, ModelManager& models, ViewObjectManager& objects, std::string& error);

} // namespace BaseShowPromotion
