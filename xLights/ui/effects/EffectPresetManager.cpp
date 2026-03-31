/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ui/effects/EffectPresetManager.h"
#include "UtilFunctions.h"
#include "ui/wxUtilities.h"
#include "utils/ExternalHooks.h"
#include "XmlSerializer/BaseSerializingVisitor.h"

#include <wx/tokenzr.h>
#include <wx/filename.h>

#include <algorithm>
#include <fstream>
#include <list>

// Local helper — strips forbidden filename characters from a name.
// Mirrors EffectTreeDialog::FixName; will be unified in a later step.
static bool FixPresetName(std::string& name)
{
    std::string forbiddenChars = wxFileName::GetForbiddenChars(wxPATH_WIN).ToStdString();
    wxString wxName(name);
    for (const auto& ch : forbiddenChars) {
        wxName.Replace(wxString(ch), wxEmptyString);
    }
    bool changed = (wxName != name);
    name = wxName.ToStdString();
    return changed;
}

// ===========================================================================
// EffectPreset
// ===========================================================================

EffectPreset::EffectPreset(const std::string& name, const std::string& settings,
                           const std::string& version, const std::string& xLightsVersion,
                           EffectPresetGroup* parent)
    : EffectPresetItem(name, parent)
    , _settings(settings)
    , _version(version)
    , _xLightsVersion(xLightsVersion)
{
}

EffectPreset::EffectPreset(pugi::xml_node node, EffectPresetGroup* parent)
    : EffectPresetItem(node.attribute("name").as_string(""), parent)
    , _settings(node.attribute("settings").as_string(""))
    , _version(node.attribute("version").as_string(""))
    , _xLightsVersion(node.attribute("xLightsVersion").as_string("4.0"))
{
}

void EffectPreset::Save(BaseSerializingVisitor& visitor) const
{
    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("name", _name);
    attrs.Add("settings", _settings);
    attrs.Add("version", _version);
    attrs.Add("xLightsVersion", _xLightsVersion);
    visitor.WriteOpenTag("effect", attrs, /*selfClose=*/true);
}

nlohmann::json EffectPreset::ToJson() const
{
    return {
        {"type", "effect"},
        {"name", _name},
        {"settings", _settings},
        {"version", _version},
        {"xLightsVersion", _xLightsVersion}
    };
}

// ===========================================================================
// EffectPresetGroup
// ===========================================================================

EffectPresetGroup::EffectPresetGroup(const std::string& name, EffectPresetGroup* parent)
    : EffectPresetItem(name, parent)
{
}

EffectPresetGroup::EffectPresetGroup(pugi::xml_node node, EffectPresetGroup* parent)
    : EffectPresetItem(node.attribute("name").as_string(""), parent)
{
    LoadChildren(node);
}

void EffectPresetGroup::LoadChildren(pugi::xml_node node)
{
    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        std::string_view childName = child.name();
        if (childName == "effectGroup") {
            _children.push_back(std::make_unique<EffectPresetGroup>(child, this));
        } else if (childName == "effect") {
            // Only load presets that have tab-separated settings (valid format)
            std::string settings = child.attribute("settings").as_string("");
            if (settings.find('\t') != std::string::npos) {
                _children.push_back(std::make_unique<EffectPreset>(child, this));
            }
        }
    }
}

void EffectPresetGroup::Save(BaseSerializingVisitor& visitor) const
{
    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("name", _name);
    visitor.WriteOpenTag("effectGroup", attrs);
    for (const auto& child : _children) {
        child->Save(visitor);
    }
    visitor.WriteCloseTag();
}

nlohmann::json EffectPresetGroup::ToJson() const
{
    nlohmann::json childrenArr = nlohmann::json::array();
    for (const auto& child : _children) {
        childrenArr.push_back(child->ToJson());
    }
    return {
        {"type", "group"},
        {"name", _name},
        {"children", childrenArr}
    };
}

void EffectPresetGroup::LoadChildrenFromJson(const nlohmann::json& j)
{
    if (!j.contains("children") || !j["children"].is_array())
        return;

    for (const auto& child : j["children"]) {
        std::string type = child.value("type", "");
        if (type == "group") {
            auto group = std::make_unique<EffectPresetGroup>(child.value("name", ""), this);
            group->LoadChildrenFromJson(child);
            _children.push_back(std::move(group));
        } else if (type == "effect") {
            std::string settings = child.value("settings", "");
            if (settings.find('\t') != std::string::npos) {
                _children.push_back(std::make_unique<EffectPreset>(
                    child.value("name", ""),
                    settings,
                    child.value("version", ""),
                    child.value("xLightsVersion", "4.0"),
                    this));
            }
        }
    }
}

EffectPresetItem* EffectPresetGroup::AddChild(std::unique_ptr<EffectPresetItem> child)
{
    child->SetParent(this);
    _children.push_back(std::move(child));
    return _children.back().get();
}

EffectPresetItem* EffectPresetGroup::InsertChildAfter(std::unique_ptr<EffectPresetItem> child,
                                                       EffectPresetItem* after)
{
    child->SetParent(this);
    if (after == nullptr) {
        // Prepend
        auto* ptr = child.get();
        _children.insert(_children.begin(), std::move(child));
        return ptr;
    }
    for (auto it = _children.begin(); it != _children.end(); ++it) {
        if (it->get() == after) {
            ++it;
            auto* ptr = child.get();
            _children.insert(it, std::move(child));
            return ptr;
        }
    }
    // after not found — append
    return AddChild(std::move(child));
}

std::unique_ptr<EffectPresetItem> EffectPresetGroup::RemoveChild(EffectPresetItem* child)
{
    for (auto it = _children.begin(); it != _children.end(); ++it) {
        if (it->get() == child) {
            std::unique_ptr<EffectPresetItem> removed = std::move(*it);
            _children.erase(it);
            removed->SetParent(nullptr);
            return removed;
        }
    }
    return nullptr;
}

EffectPresetItem* EffectPresetGroup::FindChildByName(const std::string& name) const
{
    for (const auto& child : _children) {
        if (child->GetName() == name) {
            return child.get();
        }
    }
    return nullptr;
}

bool EffectPresetGroup::HasChildNamed(const std::string& name) const
{
    return FindChildByName(name) != nullptr;
}

// ===========================================================================
// EffectPresetManager
// ===========================================================================

EffectPresetManager::EffectPresetManager()
    : _root("", nullptr)
{
}


void EffectPresetManager::Load(pugi::xml_node effectsNode)
{
    Reset();
    if (!effectsNode)
        return;

    _version = effectsNode.attribute("version").as_string("0000");

    for (pugi::xml_node child = effectsNode.first_child(); child; child = child.next_sibling()) {
        std::string_view childName = child.name();
        if (childName == "effectGroup") {
            _root.AddChild(std::make_unique<EffectPresetGroup>(child, &_root));
        } else if (childName == "effect") {
            std::string settings = child.attribute("settings").as_string("");
            if (settings.find('\t') != std::string::npos) {
                _root.AddChild(std::make_unique<EffectPreset>(child, &_root));
            }
        }
    }
}

void EffectPresetManager::Save(BaseSerializingVisitor& visitor) const
{
    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("version", _version);
    visitor.WriteOpenTag("effects", attrs);
    for (const auto& child : _root.GetChildren()) {
        child->Save(visitor);
    }
    visitor.WriteCloseTag();
}

void EffectPresetManager::LoadFromJson(const nlohmann::json& j)
{
    Reset();
    if (!j.is_object())
        return;

    _version = j.value("version", "0000");
    _root.LoadChildrenFromJson(j);
}

nlohmann::json EffectPresetManager::SaveToJson() const
{
    nlohmann::json childrenArr = nlohmann::json::array();
    for (const auto& child : _root.GetChildren()) {
        childrenArr.push_back(child->ToJson());
    }
    return {
        {"version", _version},
        {"children", childrenArr}
    };
}

bool EffectPresetManager::LoadJsonFile(const std::string& filepath)
{
    if (!FileExists(filepath))
        return false;

    ObtainAccessToURL(filepath);
    std::ifstream ifs(filepath);
    if (!ifs.is_open())
        return false;

    try {
        nlohmann::json j = nlohmann::json::parse(ifs);
        LoadFromJson(j);
        return true;
    } catch (...) {
        return false;
    }
}

bool EffectPresetManager::SaveJsonFile(const std::string& filepath) const
{
    ObtainAccessToURL(filepath, true);
    std::ofstream ofs(filepath);
    if (!ofs.is_open())
        return false;

    try {
        nlohmann::json j = SaveToJson();
        ofs << j.dump(2);
        return ofs.good();
    } catch (...) {
        return false;
    }
}

void EffectPresetManager::Reset()
{
    _root = EffectPresetGroup("", nullptr);
    _version.clear();
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

std::vector<std::string> EffectPresetManager::GetAllPresetPaths(const std::string& separator) const
{
    std::vector<std::string> result;
    CollectPresetPaths(_root, "", separator, result);
    return result;
}

void EffectPresetManager::CollectPresetPaths(const EffectPresetGroup& group,
                                              const std::string& prefix,
                                              const std::string& separator,
                                              std::vector<std::string>& result) const
{
    for (const auto& child : group.GetChildren()) {
        if (child->IsGroup()) {
            auto newPrefix = prefix.empty() ? child->GetName()
                                            : prefix + separator + child->GetName();
            CollectPresetPaths(static_cast<const EffectPresetGroup&>(*child),
                               newPrefix, separator, result);
        } else {
            auto path = prefix.empty() ? child->GetName()
                                       : prefix + separator + child->GetName();
            result.push_back(path);
        }
    }
}

EffectPresetItem* EffectPresetManager::FindItemByPath(const std::string& path,
                                                       char separator) const
{
    wxArrayString parts = wxSplit(path, separator);
    if (parts.empty())
        return nullptr;

    const EffectPresetGroup* current = &_root;
    for (size_t i = 0; i < parts.size(); ++i) {
        std::string partName = parts[i].ToStdString();
        EffectPresetItem* found = current->FindChildByName(partName);
        if (found == nullptr)
            return nullptr;
        if (i == parts.size() - 1)
            return found;
        if (!found->IsGroup())
            return nullptr;
        current = static_cast<const EffectPresetGroup*>(found);
    }
    return nullptr;
}

EffectPreset* EffectPresetManager::FindPresetByPath(const std::string& path,
                                                     char separator) const
{
    EffectPresetItem* item = FindItemByPath(path, separator);
    if (item != nullptr && !item->IsGroup())
        return static_cast<EffectPreset*>(item);
    return nullptr;
}

// ---------------------------------------------------------------------------
// Mutations
// ---------------------------------------------------------------------------

EffectPreset* EffectPresetManager::AddPreset(EffectPresetGroup* parent,
                                              const std::string& name,
                                              const std::string& settings,
                                              const std::string& version,
                                              const std::string& xLightsVersion)
{
    if (parent == nullptr)
        parent = &_root;
    auto preset = std::make_unique<EffectPreset>(name, settings, version, xLightsVersion, parent);
    return static_cast<EffectPreset*>(parent->AddChild(std::move(preset)));
}

EffectPresetGroup* EffectPresetManager::AddGroup(EffectPresetGroup* parent,
                                                  const std::string& name)
{
    if (parent == nullptr)
        parent = &_root;
    auto group = std::make_unique<EffectPresetGroup>(name, parent);
    return static_cast<EffectPresetGroup*>(parent->AddChild(std::move(group)));
}

void EffectPresetManager::Remove(EffectPresetItem* item)
{
    if (item == nullptr || item->GetParent() == nullptr)
        return;
    item->GetParent()->RemoveChild(item);
    // unique_ptr destruction handles cleanup
}

void EffectPresetManager::MoveItem(EffectPresetItem* item, EffectPresetGroup* newParent,
                                    EffectPresetItem* insertAfter)
{
    if (item == nullptr || newParent == nullptr)
        return;
    EffectPresetGroup* oldParent = item->GetParent();
    if (oldParent == nullptr)
        return;

    auto owned = oldParent->RemoveChild(item);
    if (owned) {
        if (insertAfter != nullptr) {
            newParent->InsertChildAfter(std::move(owned), insertAfter);
        } else {
            newParent->AddChild(std::move(owned));
        }
    }
}

void EffectPresetManager::RenameItem(EffectPresetItem* item, const std::string& newName)
{
    if (item != nullptr)
        item->SetName(newName);
}

void EffectPresetManager::UpdatePresetSettings(EffectPreset* preset,
                                                const std::string& settings,
                                                const std::string& xLightsVersion)
{
    if (preset == nullptr)
        return;
    preset->SetSettings(settings);
    preset->SetXLightsVersion(xLightsVersion);
}

// ---------------------------------------------------------------------------
// Import
// ---------------------------------------------------------------------------

void EffectPresetManager::ImportFromXml(pugi::xml_node node, EffectPresetGroup* parent)
{
    if (!node)
        return;
    if (parent == nullptr)
        parent = &_root;

    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        std::string_view childName = child.name();
        if (childName == "effectGroup") {
            parent->AddChild(std::make_unique<EffectPresetGroup>(child, parent));
        } else if (childName == "effect") {
            parent->AddChild(std::make_unique<EffectPreset>(child, parent));
        }
    }
}

// ---------------------------------------------------------------------------
// FixRgbEffects — migrated from EffectTreeDialog
// ---------------------------------------------------------------------------

bool EffectPresetManager::FixRgbEffects()
{
    return FixGroupNames(_root);
}

bool EffectPresetManager::FixGroupNames(EffectPresetGroup& group)
{
    bool anyFixed = false;
    std::list<std::pair<std::string, int>> children;

    for (const auto& child : group.GetChildren()) {
        std::string name = child->GetName();

        // Fix illegal characters in name
        if (FixPresetName(name)) {
            child->SetName(name);
            anyFixed = true;
        }

        // Fix duplicate names at this level
        auto existingItem = std::find_if(children.begin(), children.end(),
            [&name](const std::pair<std::string, int>& b) {
                return b.first == name;
            });

        if (existingItem == children.end()) {
            children.push_back(std::make_pair(name, 0));
        } else {
            int childUniqueIdx = existingItem->second + 1;
            std::string newName = wxString::Format("%s %d", name, childUniqueIdx).ToStdString();
            child->SetName(newName);
            existingItem->second = childUniqueIdx;
            anyFixed = true;
        }

        // Recurse into subgroups
        if (child->IsGroup()) {
            if (FixGroupNames(static_cast<EffectPresetGroup&>(*child))) {
                anyFixed = true;
            }
        }
    }

    return anyFixed;
}
