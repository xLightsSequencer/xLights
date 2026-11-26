/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "XModelFiles.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>

#include <pugixml.hpp>

#include <log.h>

#include "../graphics/xlMesh.h"
#include "../utils/Base64.h"
#include "../utils/ExternalHooks.h"
#include "../utils/FileUtils.h"

namespace fs = std::filesystem;

namespace XModelFiles {

namespace {

std::string Lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

std::string Trim(std::string s) {
    auto ws = [](unsigned char c) { return std::isspace(c) != 0; };
    while (!s.empty() && ws(s.back())) s.pop_back();
    size_t i = 0;
    while (i < s.size() && ws(s[i])) ++i;
    return s.substr(i);
}

bool ReadFile(const std::string& path, std::string& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    out.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    return !in.bad();
}

// A relative path that stays inside the folder it is resolved against.
bool IsContainedRelative(const std::string& p) {
    if (p.empty() || p[0] == '/' || p[0] == '\\' || p.find(':') != std::string::npos) return false;
    std::string s = p;
    std::replace(s.begin(), s.end(), '\\', '/');
    size_t start = 0;
    while (start <= s.size()) {
        size_t end = s.find('/', start);
        std::string part = s.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (part.empty() || part == "." || part == "..") return false;
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return true;
}

std::string Sanitize(const std::string& name) {
    std::string out;
    for (unsigned char c : name) {
        out += (std::isalnum(c) || c == ' ' || c == '-' || c == '_' || c == '.') ? (char)c : '_';
    }
    out = Trim(out);
    while (!out.empty() && out.front() == '.') out.erase(out.begin());
    while (!out.empty() && out.back() == '.') out.pop_back();
    return out.empty() ? "model" : out;
}

} // namespace

Packer::Packer(const std::string& prop) : _dir(std::string(Folder) + "/" + Sanitize(prop)) {}

std::string Packer::UniquePath(const std::string& dir, const std::string& name) {
    const fs::path n(name);
    const std::string stem = n.stem().string();
    const std::string ext = n.extension().string();
    std::string candidate = dir + "/" + name;
    for (int i = 2; _usedPaths.count(Lower(candidate)) != 0; ++i) {
        candidate = dir + "/" + stem + "-" + std::to_string(i) + ext;
    }
    _usedPaths.insert(Lower(candidate));
    return candidate;
}

bool Packer::Queue(const std::string& source, const std::string& path) {
    std::string bytes;
    if (!ReadFile(source, bytes)) {
        spdlog::warn("xmodel export: could not read '{}', it is not included.", source);
        return false;
    }
    _usedPaths.insert(Lower(path));
    _pending.push_back({ path, Base64::Encode(reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size()) });
    return true;
}

// The loader looks for an .obj's materials, and their textures, in the .obj's
// own folder, so they keep their place relative to it.
void Packer::AddMeshDependencies(const std::string& obj, const std::string& pkgDir) {
    const fs::path objDir = fs::path(obj).parent_path();
    auto addRelative = [&](const std::string& ref) -> std::string {
        std::string rel = Trim(ref);
        if (!IsContainedRelative(rel)) return "";
        const std::string src = (objDir / rel).string();
        if (!FileExists(src, false)) return "";
        std::replace(rel.begin(), rel.end(), '\\', '/');
        const std::string path = pkgDir + "/" + rel;
        if (_usedPaths.count(Lower(path)) != 0) return src;
        return Queue(src, path) ? src : "";
    };
    // The whole line first (a name with spaces), then its words.
    auto refs = [](const std::vector<std::string>& whole, const std::vector<std::string>& words) {
        std::vector<std::string> all = whole;
        all.insert(all.end(), words.begin(), words.end());
        return all;
    };
    for (const auto& mtl : refs(xlMesh::GetMaterialFilenamesFromOBJ(obj, false), xlMesh::GetMaterialFilenamesFromOBJ(obj, true))) {
        const std::string src = addRelative(mtl);
        if (src.empty()) continue;
        for (const auto& tex : refs(xlMesh::GetTextureFilenamesFromMTL(src, false), xlMesh::GetTextureFilenamesFromMTL(src, true))) {
            addRelative(tex);
        }
    }
}

std::string Packer::Add(const std::string& file) {
    if (file.empty()) return file;
    if (auto it = _added.find(file); it != _added.end()) return it->second;

    const std::string resolved = FileUtils::FixFile("", file);
    std::string slashed = resolved;
    std::replace(slashed.begin(), slashed.end(), '\\', '/');
    if (slashed.find("/meshobjects/") != std::string::npos || !FileExists(resolved, false)) {
        return file;
    }
    if (auto it = _added.find(resolved); it != _added.end()) {
        _added[file] = it->second;
        return it->second;
    }

    const fs::path p(resolved);
    std::string path;
    if (Lower(p.extension().string()) == ".obj") {
        // a folder of its own, so its materials and textures can't collide
        // with another mesh's
        const std::string dir = UniquePath(_dir, p.stem().string());
        path = dir + "/" + p.filename().string();
        if (!Queue(resolved, path)) return file;
        AddMeshDependencies(resolved, dir);
    } else {
        path = UniquePath(_dir, p.filename().string());
        if (!Queue(resolved, path)) return file;
    }
    _added[file] = path;
    _added[resolved] = path;
    return path;
}

std::vector<Packer::Entry> Packer::TakePending() {
    std::vector<Entry> out;
    out.swap(_pending);
    return out;
}

namespace {

struct Embedded {
    pugi::xml_node node;
    std::string path; // "modeldownload/<prop>/<rest>"
    std::string prop;
    std::string rest;
    std::vector<uint8_t> bytes;
};

void Collect(pugi::xml_node node, std::vector<Embedded>& out) {
    for (pugi::xml_node c = node.first_child(); c; c = c.next_sibling()) {
        if (c.type() != pugi::node_element) continue;
        if (std::string_view(c.name()) == NodeName) {
            Embedded e;
            e.node = c;
            e.path = c.attribute(PathAttribute).as_string("");
            out.push_back(std::move(e));
        } else {
            Collect(c, out);
        }
    }
}

void Rewrite(pugi::xml_node node, const std::map<std::string, std::string>& placed) {
    for (pugi::xml_attribute a = node.first_attribute(); a; a = a.next_attribute()) {
        auto it = placed.find(a.value());
        if (it != placed.end()) a.set_value(it->second.c_str());
    }
    for (pugi::xml_node c = node.first_child(); c; c = c.next_sibling()) {
        if (c.type() == pugi::node_element) Rewrite(c, placed);
    }
}

bool SameBytes(const fs::path& file, const std::vector<uint8_t>& bytes) {
    std::string have;
    return ReadFile(file.string(), have) && have.size() == bytes.size() &&
           (bytes.empty() || std::memcmp(have.data(), bytes.data(), bytes.size()) == 0);
}

} // namespace

int Unpack(pugi::xml_node root, const std::string& showDir) {
    if (!root) return 0;
    std::vector<Embedded> all;
    Collect(root, all);
    if (all.empty()) return 0;

    // group by prop, keeping only paths that stay inside modeldownload/<prop>
    std::map<std::string, std::vector<Embedded*>> props;
    const std::string prefix = std::string(Folder) + "/";
    for (auto& e : all) {
        if (e.path.compare(0, prefix.size(), prefix) != 0 || !IsContainedRelative(e.path)) {
            spdlog::warn("xmodel import: ignoring embedded file '{}'.", e.path);
            continue;
        }
        const std::string inner = e.path.substr(prefix.size());
        const size_t slash = inner.find('/');
        if (slash == std::string::npos) continue;
        e.prop = inner.substr(0, slash);
        e.rest = inner.substr(slash + 1);
        e.bytes = Base64::Decode(e.node.text().get());
        props[e.prop].push_back(&e);
    }

    std::map<std::string, std::string> placed;
    int count = 0;
    const fs::path base = fs::path(showDir) / Folder;
    if (!showDir.empty()) {
        for (auto& [prop, files] : props) {
            // the first folder where every file is new or already identical
            fs::path dir;
            for (int i = 1;; ++i) {
                dir = base / (i == 1 ? prop : prop + "-" + std::to_string(i));
                bool fits = true;
                for (const Embedded* e : files) {
                    const fs::path target = (dir / fs::path(e->rest)).make_preferred();
                    if (FileExists(target.string(), false) && !SameBytes(target, e->bytes)) {
                        fits = false;
                        break;
                    }
                }
                if (fits) break;
            }
            for (const Embedded* e : files) {
                const fs::path target = (dir / fs::path(e->rest)).make_preferred();
                if (!FileExists(target.string(), false)) {
                    std::error_code ec;
                    fs::create_directories(target.parent_path(), ec);
                    std::ofstream out(target, std::ios::binary);
                    out.write(reinterpret_cast<const char*>(e->bytes.data()), (std::streamsize)e->bytes.size());
                    out.close();
                    if (!out) {
                        spdlog::error("xmodel import: could not write '{}'.", target.string());
                        continue;
                    }
                }
                placed[e->path] = target.string();
                ++count;
            }
        }
    }

    for (auto& e : all) {
        e.node.parent().remove_child(e.node);
    }
    Rewrite(root, placed);
    return count;
}

} // namespace XModelFiles
