/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceFaces.h"

#include "SequenceMedia.h"
#include "../utils/ExternalHooks.h"
#include "../utils/FileUtils.h"
#include "pugixml.hpp"

#include <set>

bool SequenceFaces::IsImageKey(const std::string& key)
{
    if (key.compare(0, 5, "Mouth") != 0 && key.compare(0, 4, "Eyes") != 0) {
        return false;
    }
    return key.size() < 6 || key.compare(key.size() - 6, 6, "-Color") != 0;
}

const SequenceFaces::FaceDefinition* SequenceFaces::GetFace(const std::string& name) const
{
    auto it = _faces.find(name);
    return it == _faces.end() ? nullptr : &it->second;
}

void SequenceFaces::SetFace(const std::string& name, const FaceDefinition& def)
{
    if (name.empty()) {
        return;
    }
    FaceDefinition& d = _faces[name] = def;
    d["Type"] = "Matrix";
}

bool SequenceFaces::RemoveFace(const std::string& name)
{
    return _faces.erase(name) != 0;
}

bool SequenceFaces::RenameFace(const std::string& oldName, const std::string& newName)
{
    if (newName.empty() || oldName == newName) {
        return false;
    }
    auto it = _faces.find(oldName);
    if (it == _faces.end() || _faces.find(newName) != _faces.end()) {
        return false;
    }
    _faces[newName] = std::move(it->second);
    _faces.erase(it);
    return true;
}

std::list<std::string> SequenceFaces::GetImagePaths() const
{
    std::set<std::string> seen;
    std::list<std::string> paths;
    for (const auto& [name, def] : _faces) {
        for (const auto& [key, value] : def) {
            if (!value.empty() && IsImageKey(key) && seen.insert(value).second) {
                paths.push_back(value);
            }
        }
    }
    return paths;
}

int SequenceFaces::CountImageReferences(const std::string& path) const
{
    int count = 0;
    for (const auto& [name, def] : _faces) {
        for (const auto& [key, value] : def) {
            if (value == path && IsImageKey(key)) {
                ++count;
            }
        }
    }
    return count;
}

int SequenceFaces::RewriteImagePath(const std::string& from, const std::string& to)
{
    int count = 0;
    for (auto& [name, def] : _faces) {
        for (auto& [key, value] : def) {
            if (value == from && IsImageKey(key)) {
                value = to;
                ++count;
            }
        }
    }
    return count;
}

bool SequenceFaces::LoadFromXml(const pugi::xml_node& node, SequenceMedia& media)
{
    Clear();
    for (auto f : node.children("faceInfo")) {
        std::string name = f.attribute("Name").as_string("");
        if (name.empty()) {
            continue;
        }
        std::string type = f.attribute("Type").as_string("Matrix");
        if (type != "Matrix") {
            continue;
        }
        FaceDefinition& def = _faces[name];
        for (pugi::xml_attribute att = f.first_attribute(); att; att = att.next_attribute()) {
            std::string attName = att.name();
            if (attName == "Name") {
                continue;
            }
            std::string value = att.value();
            if (IsImageKey(attName) && !value.empty()) {
                // Store the path as written and resolve at point of use
                // (SequenceMedia::ResolvePath / render / validation) — never
                // rewrite it here. A FixFile'd path is machine-specific, and
                // FixFile's filename-only fallback could silently repoint the
                // definition at a same-named file elsewhere in the show
                // folder (or repoint an embedded name at a disk file).
                //
                // Register external images so the Media tab lists them for
                // embedding after a reload (only embedded entries persist in
                // the .xsq), and flag them used-by-metadata so unused-media
                // cleanup retains them — face images aren't in any effect's
                // settings, so nothing in the render pipeline marks them used.
                if (!media.HasImage(value) && FileExists(FileUtils::FixFile("", value))) {
                    media.RegisterImage(value);
                }
                media.MarkUsedByMetadata(value);
            }
            def[attName] = value;
        }
        def["Type"] = "Matrix";
    }
    return true;
}

void SequenceFaces::SaveToXml(pugi::xml_node& parent) const
{
    if (_faces.empty()) {
        return;
    }
    auto node = parent.append_child("FaceDefinitions");
    for (const auto& [name, def] : _faces) {
        auto f = node.append_child("faceInfo");
        f.append_attribute("Name") = name;
        for (const auto& [key, value] : def) {
            f.append_attribute(key) = value;
        }
    }
}
