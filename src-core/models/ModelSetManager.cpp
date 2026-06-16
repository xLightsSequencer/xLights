/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ModelSetManager.h"

#include <algorithm>
#include <cstdlib>
#include <string_view>
#include <unordered_set>

void ModelSetManager::RebuildIndex()
{
    _memberIndex.clear();
    for (auto& s : _sets) {
        for (const auto& m : s->GetMembers()) {
            _memberIndex[m] = s.get();
        }
    }
}

std::string ModelSetManager::GenerateUniqueName(const std::string& candidate) const
{
    if (!candidate.empty() && GetSetByName(candidate) == nullptr) {
        return candidate;
    }
    // Auto-name: pick lowest unused "Set N".
    for (int i = 1; i < 100000; ++i) {
        std::string n = "Set " + std::to_string(i);
        if (GetSetByName(n) == nullptr) {
            return n;
        }
    }
    return "Set"; // unreachable in practice
}

ModelSet* ModelSetManager::CreateSet(const std::vector<std::string>& memberModels,
                                      const std::string& suggestedName)
{
    if (memberModels.size() < 2) {
        return nullptr;
    }
    auto set = std::make_unique<ModelSet>(GenerateUniqueName(suggestedName));
    for (const auto& m : memberModels) {
        set->AddMember(m);
    }
    ModelSet* raw = set.get();
    _sets.push_back(std::move(set));
    RebuildIndex();
    return raw;
}

void ModelSetManager::DeleteSet(ModelSet* set)
{
    if (set == nullptr) return;
    _sets.erase(std::remove_if(_sets.begin(), _sets.end(),
                               [set](const std::unique_ptr<ModelSet>& s) { return s.get() == set; }),
                _sets.end());
    RebuildIndex();
}

bool ModelSetManager::RenameSet(ModelSet* set, const std::string& newName)
{
    if (set == nullptr || newName.empty()) return false;
    // Name uniqueness - if another Set already has this name, reject.
    ModelSet* existing = GetSetByName(newName);
    if (existing != nullptr && existing != set) {
        return false;
    }
    set->SetName(newName);
    return true;
}

void ModelSetManager::AddMember(ModelSet* set, const std::string& modelName)
{
    if (set == nullptr || modelName.empty()) return;
    // If the model is in a different Set, remove it from that Set first -
    // and dissolve that Set if the removal leaves it under 2 members, the
    // same invariant RemoveMember() enforces.
    auto it = _memberIndex.find(modelName);
    if (it != _memberIndex.end() && it->second != set) {
        ModelSet* old = it->second;
        old->RemoveMember(modelName);
        if (old->GetMembers().size() < 2) {
            DeleteSet(old);
        }
    }
    set->AddMember(modelName);
    RebuildIndex();
}

bool ModelSetManager::RemoveMember(const std::string& modelName)
{
    auto it = _memberIndex.find(modelName);
    if (it == _memberIndex.end()) return false;
    ModelSet* set = it->second;
    set->RemoveMember(modelName);
    bool autoDeleted = false;
    if (set->GetMembers().size() < 2) {
        DeleteSet(set);
        autoDeleted = true;
    }
    RebuildIndex();
    return autoDeleted;
}

ModelSet* ModelSetManager::GetSetContaining(const std::string& modelName) const
{
    auto it = _memberIndex.find(modelName);
    return (it == _memberIndex.end()) ? nullptr : it->second;
}

ModelSet* ModelSetManager::GetSetByName(const std::string& setName) const
{
    for (const auto& s : _sets) {
        if (s->GetName() == setName) return s.get();
    }
    return nullptr;
}

void ModelSetManager::OnModelRenamed(const std::string& oldName, const std::string& newName)
{
    if (oldName == newName) return;
    bool changed = false;
    for (auto& s : _sets) {
        if (s->HasMember(oldName)) {
            s->RenameMember(oldName, newName);
            changed = true;
        }
    }
    if (changed) {
        RebuildIndex();
    }
}

void ModelSetManager::OnModelDeleted(const std::string& modelName)
{
    RemoveMember(modelName);
}

void ModelSetManager::Load(pugi::xml_node setsNode)
{
    _sets.clear();
    if (!setsNode) {
        RebuildIndex();
        return;
    }
    std::unordered_set<std::string> claimed;
    for (pugi::xml_node e = setsNode.first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "modelSet") {
            auto set = std::make_unique<ModelSet>();
            set->Load(e);
            // Drop members already claimed by a previously-loaded Set.
            // O(members) per Set using the claimed set instead of a nested loop.
            std::vector<std::string> toRemove;
            for (const auto& m : set->GetMembers()) {
                if (claimed.count(m)) toRemove.push_back(m);
            }
            for (const auto& m : toRemove) set->RemoveMember(m);
            if (set->GetMembers().size() >= 2 && !set->GetName().empty()) {
                for (const auto& m : set->GetMembers()) claimed.insert(m);
                _sets.push_back(std::move(set));
            }
        }
    }
    RebuildIndex();
}

void ModelSetManager::Save(pugi::xml_node setsNode) const
{
    if (!setsNode) return;
    // Wipe and rewrite. Caller guarantees setsNode is the <modelSets> node.
    setsNode.remove_children();
    for (const auto& s : _sets) {
        if (s->GetMembers().size() < 2) continue;
        pugi::xml_node node = setsNode.append_child("modelSet");
        s->Save(node);
    }
}

void ModelSetManager::Clear()
{
    _sets.clear();
    _memberIndex.clear();
}
