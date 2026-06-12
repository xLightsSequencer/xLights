#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Owns the collection of Model Sets and provides CRUD + lookup. Lives
// alongside ModelManager (not a member, to avoid pulling Model* into Set
// data). Persistence is the <modelSets> XML node in
// xlights_rgbeffects.xml.

#include "ModelSet.h"

#include <memory>
#include <pugixml.hpp>
#include <string>
#include <unordered_map>
#include <vector>

class ModelSetManager {
public:
    ModelSetManager() = default;
    ModelSetManager(const ModelSetManager&) = delete;
    ModelSetManager& operator=(const ModelSetManager&) = delete;

    // CRUD
    ModelSet* CreateSet(const std::vector<std::string>& memberModels,
                        const std::string& suggestedName = "");
    // Next free auto-name ("Set N") for pre-filling a naming prompt.
    std::string SuggestName() const { return GenerateUniqueName(""); }
    void DeleteSet(ModelSet* set);
    bool RenameSet(ModelSet* set, const std::string& newName);

    // Membership
    void AddMember(ModelSet* set, const std::string& modelName);
    // Returns true if the Set is now too small (<2 members) and was auto-deleted.
    bool RemoveMember(const std::string& modelName);

    // Lookup
    ModelSet* GetSetContaining(const std::string& modelName) const;
    ModelSet* GetSetByName(const std::string& setName) const;
    const std::vector<std::unique_ptr<ModelSet>>& GetAllSets() const { return _sets; }

    // Model lifecycle hooks - call these when a model is renamed / deleted
    // in ModelManager so Sets stay coherent.
    void OnModelRenamed(const std::string& oldName, const std::string& newName);
    void OnModelDeleted(const std::string& modelName);

    // Persistence. Load reads child <modelSet> elements from setsNode.
    // Save clears setsNode's children and writes the current state.
    void Load(pugi::xml_node setsNode);
    void Save(pugi::xml_node setsNode) const;

    void Clear();

private:
    std::vector<std::unique_ptr<ModelSet>> _sets;
    // model name -> Set pointer. Rebuilt on every mutation.
    mutable std::unordered_map<std::string, ModelSet*> _memberIndex;
    void RebuildIndex();
    std::string GenerateUniqueName(const std::string& candidate) const;
};
