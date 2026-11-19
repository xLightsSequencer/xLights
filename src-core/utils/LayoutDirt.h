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

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>

// What an app that saves the layout by load-modify-write (the iPad) has
// changed in memory since xlights_rgbeffects.xml was loaded or saved: the
// records a save patches into the file, plus whether the controllers need
// saving.
//
// Memory is the base plus these records. The base is normally the file as it
// was loaded (ShowFileGuard's recorded text). When memory is brought to a
// whole layout that is not on disk (a live session applied someone else's
// edits), that layout becomes the base (AdoptBase): the per-record sets were
// relative to the old base and the new one already holds them, so they go,
// and the layout counts as changed until it is saved.
//
// Generation() moves on every change, including marking a record that is
// already marked, so an observer can tell that memory may differ from what it
// last read.
class LayoutDirt {
public:
    uint64_t Generation() const { return _generation; }

    // ---- Models and groups ----
    void MarkModel(const std::string& name) {
        if (name.empty()) return;
        Bump();
        _models.insert(name);
    }
    // A model (not a group) left memory. The file loses the record it holds
    // under the name it had there.
    void MarkModelDeleted(const std::string& name) {
        if (name.empty()) return;
        Bump();
        _models.erase(name);
        std::string disk = name;
        if (auto it = _renamedModels.find(name); it != _renamedModels.end()) {
            disk = it->second;
            _renamedModels.erase(it);
        }
        _deletedModels.insert(disk);
    }
    void MarkModelRenamed(const std::string& oldName, const std::string& newName) {
        if (oldName.empty() || newName.empty() || oldName == newName) return;
        Bump();
        RecordRename(_renamedModels, oldName, newName);
    }
    void MarkGroupCreated(const std::string& name) {
        if (name.empty()) return;
        Bump();
        _createdGroups.insert(name);
        // Deleted then created again under the same name.
        _deletedGroups.erase(name);
    }
    void MarkGroupDeleted(const std::string& name) {
        if (name.empty()) return;
        Bump();
        // Created and never saved: nothing in the file to remove.
        if (_createdGroups.erase(name) > 0) {
            _models.erase(name);
            _renamedGroups.erase(name);
            return;
        }
        _deletedGroups.insert(name);
        _models.erase(name);
        if (auto it = _renamedGroups.find(name); it != _renamedGroups.end()) {
            _deletedGroups.insert(it->second);
            _deletedGroups.erase(name);
            _renamedGroups.erase(it);
        }
    }
    void MarkGroupRenamed(const std::string& oldName, const std::string& newName) {
        if (oldName.empty() || newName.empty() || oldName == newName) return;
        Bump();
        if (_createdGroups.erase(oldName) > 0) {
            _createdGroups.insert(newName);
            return;
        }
        RecordRename(_renamedGroups, oldName, newName);
    }

    // ---- View objects ----
    void MarkViewObject(const std::string& name) {
        if (name.empty()) return;
        Bump();
        _viewObjects.insert(name);
    }
    void MarkViewObjectCreated(const std::string& name) {
        if (name.empty()) return;
        Bump();
        _createdViewObjects.insert(name);
        _deletedViewObjects.erase(name);
    }
    void MarkViewObjectDeleted(const std::string& name) {
        if (name.empty()) return;
        Bump();
        if (_createdViewObjects.erase(name) > 0) {
            _viewObjects.erase(name);
            _renamedViewObjects.erase(name);
            return;
        }
        _deletedViewObjects.insert(name);
        _viewObjects.erase(name);
        if (auto it = _renamedViewObjects.find(name); it != _renamedViewObjects.end()) {
            _deletedViewObjects.insert(it->second);
            _deletedViewObjects.erase(name);
            _renamedViewObjects.erase(it);
        }
    }
    void MarkViewObjectRenamed(const std::string& oldName, const std::string& newName) {
        if (oldName.empty() || newName.empty() || oldName == newName) return;
        Bump();
        if (_createdViewObjects.erase(oldName) > 0) {
            _createdViewObjects.insert(newName);
            return;
        }
        RecordRename(_renamedViewObjects, oldName, newName);
    }

    // ---- Layout groups (previews) and their backgrounds ----
    // "Default" is the top-level <settings> background.
    void MarkBackground(const std::string& layoutGroup) {
        if (layoutGroup.empty()) return;
        Bump();
        _backgrounds.insert(layoutGroup);
    }
    void MarkLayoutGroupDeleted(const std::string& name) {
        if (name.empty()) return;
        Bump();
        _backgrounds.erase(name);
        _deletedLayoutGroups.insert(name);
    }

    // ---- Whole sections the file rewrites from memory ----
    void MarkViews() {
        Bump();
        _views = true;
    }
    void MarkViewpoints() {
        Bump();
        _viewpoints = true;
    }

    // ---- Controllers (xlights_networks.xml, always saved whole) ----
    void MarkControllers() {
        Bump();
        _controllers = true;
    }
    bool Controllers() const { return _controllers; }
    void ClearControllers() { _controllers = false; }

    // ---- The base ----
    // Memory now holds `layout`, which is not the file on disk.
    void AdoptBase(std::string layout) {
        Bump();
        ClearRecords();
        _base = std::move(layout);
    }
    // The layout memory is patched from instead of the file as loaded; empty
    // when it is the file.
    const std::optional<std::string>& Base() const { return _base; }

    // Memory differs from the layout file (not counting the controllers).
    bool HasLayoutChanges() const {
        return _base.has_value() || !_models.empty() || !_deletedModels.empty() || !_renamedModels.empty() ||
               !_createdGroups.empty() || !_deletedGroups.empty() || !_renamedGroups.empty() || !_viewObjects.empty() ||
               !_createdViewObjects.empty() || !_deletedViewObjects.empty() || !_renamedViewObjects.empty() ||
               !_backgrounds.empty() || !_deletedLayoutGroups.empty() || _views || _viewpoints;
    }
    bool HasChanges() const { return HasLayoutChanges() || _controllers; }

    // The layout file now holds memory (saved, or memory reloaded from it).
    void ClearLayout() {
        ClearRecords();
        _base.reset();
    }
    void ClearAll() {
        ClearLayout();
        _controllers = false;
    }

    // ---- What a save patches in ----
    const std::set<std::string>& Models() const { return _models; }
    const std::set<std::string>& DeletedModels() const { return _deletedModels; }
    // New name -> the name in the base.
    const std::map<std::string, std::string>& RenamedModels() const { return _renamedModels; }
    const std::set<std::string>& CreatedGroups() const { return _createdGroups; }
    const std::set<std::string>& DeletedGroups() const { return _deletedGroups; }
    const std::map<std::string, std::string>& RenamedGroups() const { return _renamedGroups; }
    const std::set<std::string>& ViewObjects() const { return _viewObjects; }
    const std::set<std::string>& CreatedViewObjects() const { return _createdViewObjects; }
    const std::set<std::string>& DeletedViewObjects() const { return _deletedViewObjects; }
    const std::map<std::string, std::string>& RenamedViewObjects() const { return _renamedViewObjects; }
    const std::set<std::string>& Backgrounds() const { return _backgrounds; }
    const std::set<std::string>& DeletedLayoutGroups() const { return _deletedLayoutGroups; }
    bool Views() const { return _views; }
    bool Viewpoints() const { return _viewpoints; }

private:
    void Bump() { ++_generation; }

    // `renames` maps new name -> base name; chains collapse to the base name
    // and renaming back drops the entry.
    static void RecordRename(std::map<std::string, std::string>& renames, const std::string& oldName, const std::string& newName) {
        std::string disk = oldName;
        if (auto it = renames.find(disk); it != renames.end()) {
            disk = it->second;
            renames.erase(it);
        }
        if (disk == newName) {
            renames.erase(newName);
            return;
        }
        renames[newName] = disk;
    }

    void ClearRecords() {
        _models.clear();
        _deletedModels.clear();
        _renamedModels.clear();
        _createdGroups.clear();
        _deletedGroups.clear();
        _renamedGroups.clear();
        _viewObjects.clear();
        _createdViewObjects.clear();
        _deletedViewObjects.clear();
        _renamedViewObjects.clear();
        _backgrounds.clear();
        _deletedLayoutGroups.clear();
        _views = false;
        _viewpoints = false;
    }

    uint64_t _generation = 0;
    std::optional<std::string> _base;
    std::set<std::string> _models; // models and groups to rewrite from memory
    std::set<std::string> _deletedModels;
    std::map<std::string, std::string> _renamedModels;
    std::set<std::string> _createdGroups;
    std::set<std::string> _deletedGroups;
    std::map<std::string, std::string> _renamedGroups;
    std::set<std::string> _viewObjects;
    std::set<std::string> _createdViewObjects;
    std::set<std::string> _deletedViewObjects;
    std::map<std::string, std::string> _renamedViewObjects;
    std::set<std::string> _backgrounds;
    std::set<std::string> _deletedLayoutGroups;
    bool _views = false;
    bool _viewpoints = false;
    bool _controllers = false;
};
