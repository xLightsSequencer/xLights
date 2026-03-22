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

#include <memory>
#include <string>
#include <vector>
#include <pugixml.hpp>

class BaseSerializingVisitor;

// ---------------------------------------------------------------------------
// EffectPresetItem — abstract base for items in the preset tree
// ---------------------------------------------------------------------------
class EffectPresetGroup; // forward

class EffectPresetItem {
public:
    virtual ~EffectPresetItem() = default;

    const std::string& GetName() const { return _name; }
    void SetName(const std::string& name) { _name = name; }

    EffectPresetGroup* GetParent() const { return _parent; }
    void SetParent(EffectPresetGroup* parent) { _parent = parent; }

    virtual bool IsGroup() const = 0;

    virtual void Save(BaseSerializingVisitor& visitor) const = 0;

protected:
    EffectPresetItem(const std::string& name, EffectPresetGroup* parent)
        : _name(name), _parent(parent) {}

    std::string _name;
    EffectPresetGroup* _parent = nullptr;
};

// ---------------------------------------------------------------------------
// EffectPreset — a leaf node holding a single saved effect preset
// ---------------------------------------------------------------------------
class EffectPreset : public EffectPresetItem {
public:
    EffectPreset(const std::string& name, const std::string& settings,
                 const std::string& version, const std::string& xLightsVersion,
                 EffectPresetGroup* parent = nullptr);
    explicit EffectPreset(pugi::xml_node node, EffectPresetGroup* parent = nullptr);

    bool IsGroup() const override { return false; }
    void Save(BaseSerializingVisitor& visitor) const override;

    const std::string& GetSettings() const { return _settings; }
    void SetSettings(const std::string& settings) { _settings = settings; }

    const std::string& GetVersion() const { return _version; }
    void SetVersion(const std::string& version) { _version = version; }

    const std::string& GetXLightsVersion() const { return _xLightsVersion; }
    void SetXLightsVersion(const std::string& ver) { _xLightsVersion = ver; }

private:
    std::string _settings;
    std::string _version;
    std::string _xLightsVersion;
};

// ---------------------------------------------------------------------------
// EffectPresetGroup — a container node holding children
// ---------------------------------------------------------------------------
class EffectPresetGroup : public EffectPresetItem {
public:
    explicit EffectPresetGroup(const std::string& name, EffectPresetGroup* parent = nullptr);
    explicit EffectPresetGroup(pugi::xml_node node, EffectPresetGroup* parent = nullptr);

    bool IsGroup() const override { return true; }
    void Save(BaseSerializingVisitor& visitor) const override;

    // Child access
    const std::vector<std::unique_ptr<EffectPresetItem>>& GetChildren() const { return _children; }

    // Add a child at the end.  Returns non-owning pointer.
    EffectPresetItem* AddChild(std::unique_ptr<EffectPresetItem> child);

    // Insert a child after the given sibling (nullptr → prepend).
    EffectPresetItem* InsertChildAfter(std::unique_ptr<EffectPresetItem> child,
                                       EffectPresetItem* after);

    // Remove and return ownership of a child.
    std::unique_ptr<EffectPresetItem> RemoveChild(EffectPresetItem* child);

    // Query
    EffectPresetItem* FindChildByName(const std::string& name) const;
    bool HasChildNamed(const std::string& name) const;

private:
    void LoadChildren(pugi::xml_node node);

    std::vector<std::unique_ptr<EffectPresetItem>> _children;
};

// ---------------------------------------------------------------------------
// EffectPresetManager — top-level manager owning the preset tree
// ---------------------------------------------------------------------------
class EffectPresetManager {
public:
    EffectPresetManager();
    ~EffectPresetManager() = default;

    // Lifecycle
    void Load(pugi::xml_node effectsNode);
    void Save(BaseSerializingVisitor& visitor) const;
    void Reset();

    // Version of the effects block
    const std::string& GetVersion() const { return _version; }
    void SetVersion(const std::string& v) { _version = v; }

    // Root group access
    EffectPresetGroup& GetRoot() { return _root; }
    const EffectPresetGroup& GetRoot() const { return _root; }

    // Queries
    std::vector<std::string> GetAllPresetPaths(const std::string& separator = "\\") const;
    EffectPresetItem* FindItemByPath(const std::string& path, char separator = '\\') const;
    EffectPreset* FindPresetByPath(const std::string& path, char separator = '\\') const;

    // Mutations
    EffectPreset* AddPreset(EffectPresetGroup* parent, const std::string& name,
                            const std::string& settings, const std::string& version,
                            const std::string& xLightsVersion);
    EffectPresetGroup* AddGroup(EffectPresetGroup* parent, const std::string& name);
    void Remove(EffectPresetItem* item);
    void MoveItem(EffectPresetItem* item, EffectPresetGroup* newParent,
                  EffectPresetItem* insertAfter = nullptr);
    void RenameItem(EffectPresetItem* item, const std::string& newName);
    void UpdatePresetSettings(EffectPreset* preset, const std::string& settings,
                              const std::string& xLightsVersion);

    // Import from XML (for .xpreset files or another show's effects node)
    void ImportFromXml(pugi::xml_node node, EffectPresetGroup* parent);

    // Name fixup (migrated from EffectTreeDialog::FixRgbEffects)
    bool FixRgbEffects();

private:
    void CollectPresetPaths(const EffectPresetGroup& group, const std::string& prefix,
                            const std::string& separator,
                            std::vector<std::string>& result) const;
    bool FixGroupNames(EffectPresetGroup& group);

    EffectPresetGroup _root;
    std::string _version;
};
