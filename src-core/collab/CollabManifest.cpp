/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CollabManifest.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <set>
#include <system_error>

#include <pugixml.hpp>

#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/RenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceMedia.h"
#include "utils/ExternalHooks.h"
#include "utils/FileUtils.h"
#include "utils/LayoutRecords.h"
#include "utils/ShowFileMerge.h"

namespace fs = std::filesystem;

namespace Collab {

// ---------------------------------------------------------------------------
// Content hash (XXH64 layout, little-endian reads on every platform)
// ---------------------------------------------------------------------------

namespace {

constexpr uint64_t P1 = 0x9E3779B185EBCA87ull;
constexpr uint64_t P2 = 0xC2B2AE3D27D4EB4Full;
constexpr uint64_t P3 = 0x165667B19E3779F9ull;
constexpr uint64_t P4 = 0x85EBCA77C2B2AE63ull;
constexpr uint64_t P5 = 0x27D4EB2F165667C5ull;

inline uint64_t Rotl(uint64_t x, int r) {
    return (x << r) | (x >> (64 - r));
}

inline uint64_t Read64(const unsigned char* p) {
    uint64_t v = 0;
    for (int i = 7; i >= 0; --i) {
        v = (v << 8) | p[i];
    }
    return v;
}

inline uint64_t Read32(const unsigned char* p) {
    return (uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16) | ((uint64_t)p[3] << 24);
}

inline uint64_t Round(uint64_t acc, uint64_t in) {
    acc += in * P2;
    acc = Rotl(acc, 31);
    return acc * P1;
}

inline uint64_t MergeRound(uint64_t acc, uint64_t v) {
    acc ^= Round(0, v);
    return acc * P1 + P4;
}

} // namespace

ContentHash::ContentHash() {
    _v[0] = P1 + P2;
    _v[1] = P2;
    _v[2] = 0;
    _v[3] = 0 - P1;
}

void ContentHash::Update(const void* data, size_t len) {
    const unsigned char* p = static_cast<const unsigned char*>(data);
    _total += len;
    if (_bufLen > 0) {
        size_t take = std::min(len, sizeof(_buf) - _bufLen);
        std::memcpy(_buf + _bufLen, p, take);
        _bufLen += take;
        p += take;
        len -= take;
        if (_bufLen < sizeof(_buf)) {
            return;
        }
        for (int i = 0; i < 4; ++i) {
            _v[i] = Round(_v[i], Read64(_buf + 8 * i));
        }
        _bufLen = 0;
    }
    while (len >= 32) {
        for (int i = 0; i < 4; ++i) {
            _v[i] = Round(_v[i], Read64(p + 8 * i));
        }
        p += 32;
        len -= 32;
    }
    if (len > 0) {
        std::memcpy(_buf, p, len);
        _bufLen = len;
    }
}

uint64_t ContentHash::Final() const {
    uint64_t h;
    if (_total >= 32) {
        h = Rotl(_v[0], 1) + Rotl(_v[1], 7) + Rotl(_v[2], 12) + Rotl(_v[3], 18);
        for (int i = 0; i < 4; ++i) {
            h = MergeRound(h, _v[i]);
        }
    } else {
        h = P5;
    }
    h += _total;
    const unsigned char* p = _buf;
    size_t len = _bufLen;
    while (len >= 8) {
        h ^= Round(0, Read64(p));
        h = Rotl(h, 27) * P1 + P4;
        p += 8;
        len -= 8;
    }
    if (len >= 4) {
        h ^= Read32(p) * P1;
        h = Rotl(h, 23) * P2 + P3;
        p += 4;
        len -= 4;
    }
    while (len > 0) {
        h ^= (*p) * P5;
        h = Rotl(h, 11) * P1;
        ++p;
        --len;
    }
    h ^= h >> 33;
    h *= P2;
    h ^= h >> 29;
    h *= P3;
    h ^= h >> 32;
    return h;
}

uint64_t HashBytes(const std::string& bytes) {
    ContentHash h;
    h.Update(bytes.data(), bytes.size());
    return h.Final();
}

namespace {
std::function<void(const std::string&)> gHashFileHook;
}

void SetHashFileHook(std::function<void(const std::string&)> hook) {
    gHashFileHook = std::move(hook);
}

bool HashFile(const std::string& path, uint64_t& size, uint64_t& hash) {
    if (path.empty()) {
        return false;
    }
    if (gHashFileHook) {
        gHashFileHook(path);
    }
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return false;
    }
    ContentHash h;
    std::vector<char> buf(1 << 20);
    uint64_t total = 0;
    while (f) {
        f.read(buf.data(), (std::streamsize)buf.size());
        std::streamsize n = f.gcount();
        if (n <= 0) {
            break;
        }
        h.Update(buf.data(), (size_t)n);
        total += (uint64_t)n;
    }
    if (f.bad()) {
        return false;
    }
    size = total;
    hash = h.Final();
    return true;
}

uint64_t HashCache::BytesToRead(const std::string& path) const {
    std::error_code ec;
    auto fsize = fs::file_size(path, ec);
    if (ec) {
        return 0;
    }
    auto mtime = fs::last_write_time(path, ec);
    if (ec) {
        return 0;
    }
    auto it = _items.find(path);
    bool cached = it != _items.end() && it->second.size == (uint64_t)fsize && it->second.mtime == (int64_t)mtime.time_since_epoch().count();
    return cached ? 0 : (uint64_t)fsize;
}

HashCache HashCache::Fork() const {
    HashCache c = *this;
    c._filesRead = 0;
    return c;
}

void HashCache::Merge(const HashCache& other) {
    for (const auto& [path, item] : other._items) {
        _items[path] = item;
    }
    _filesRead += other._filesRead;
}

bool HashCache::Hash(const std::string& path, uint64_t& size, uint64_t& hash) {
    std::error_code ec;
    auto fsize = fs::file_size(path, ec);
    if (ec) {
        return false;
    }
    auto mtime = fs::last_write_time(path, ec);
    if (ec) {
        return false;
    }
    int64_t t = (int64_t)mtime.time_since_epoch().count();
    auto it = _items.find(path);
    if (it != _items.end() && it->second.size == (uint64_t)fsize && it->second.mtime == t) {
        size = it->second.size;
        hash = it->second.hash;
        return true;
    }
    ++_filesRead;
    if (!HashFile(path, size, hash)) {
        _items.erase(path);
        return false;
    }
    _items[path] = { size, t, hash };
    return true;
}

// ---------------------------------------------------------------------------
// Paths
// ---------------------------------------------------------------------------

namespace {

std::string ForwardSlashes(std::string s) {
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}

std::vector<std::string> Components(const std::string& path) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : path) {
        if (c == '/' || c == '\\') {
            if (!cur.empty() && cur != ".") {
                out.push_back(cur);
            }
            cur.clear();
        } else {
            cur += c;
        }
    }
    if (!cur.empty() && cur != ".") {
        out.push_back(cur);
    }
    return out;
}

bool SafeComponent(const std::string& c) {
    return !c.empty() && c != ".." && c.find(':') == std::string::npos;
}

fs::path Canonical(const std::string& p) {
    std::error_code ec;
    fs::path c = fs::weakly_canonical(fs::path(p), ec);
    return ec ? fs::path(p) : c;
}

bool Hash(HashCache* cache, const std::string& path, uint64_t& size, uint64_t& hash) {
    return cache != nullptr ? cache->Hash(path, size, hash) : HashFile(path, size, hash);
}

} // namespace

std::string ManifestPathFor(const std::string& showDir, const std::string& reference, const std::string& resolved) {
    if (!showDir.empty() && !resolved.empty()) {
        try {
            fs::path rel = Canonical(resolved).lexically_relative(Canonical(showDir));
            std::string s = rel.generic_string();
            if (!s.empty() && !rel.is_absolute() && s != "." && s.rfind("..", 0) != 0) {
                return s;
            }
        } catch (const std::exception&) {
        }
    }
    if (!FileUtils::IsAbsoluteOrRootedPath(reference)) {
        return ForwardSlashes(reference);
    }
    return reference;
}

std::string CachePathFor(const std::string& path) {
    std::vector<std::string> comps = Components(path);
    if (comps.empty()) {
        return "";
    }
    bool plain = !FileUtils::IsAbsoluteOrRootedPath(path) && std::all_of(comps.begin(), comps.end(), SafeComponent);
    if (plain) {
        std::string out;
        for (const auto& c : comps) {
            if (!out.empty()) {
                out += '/';
            }
            out += c;
        }
        return out;
    }
    const std::string& file = comps.back();
    if (!SafeComponent(file)) {
        return "";
    }
    if (comps.size() >= 2 && SafeComponent(comps[comps.size() - 2])) {
        return comps[comps.size() - 2] + "/" + file;
    }
    return file;
}

std::string SessionSequenceFileName(const std::string& advertised) {
    static const std::string kDefault = "Session.xsq";
    std::vector<std::string> comps = Components(advertised);
    if (comps.empty()) {
        return kDefault;
    }
    std::string name = comps.back();
    if (!SafeComponent(name)) {
        return kDefault;
    }
    for (unsigned char c : name) {
        if (c < 0x20 || c == 0x7f || std::strchr("<>\"|?*", c) != nullptr) {
            return kDefault;
        }
    }
    // Windows drops trailing dots and spaces, so "..." would name the folder.
    while (!name.empty() && (name.back() == '.' || name.back() == ' ')) {
        name.pop_back();
    }
    if (name.empty()) {
        return kDefault;
    }
    auto lower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
        return s;
    };
    std::string stem = lower(name.substr(0, name.find('.')));
    while (!stem.empty() && stem.back() == ' ') {
        stem.pop_back();
    }
    static const std::set<std::string> kReserved = { "con", "prn", "aux", "nul",
                                                     "com1", "com2", "com3", "com4", "com5", "com6", "com7", "com8", "com9",
                                                     "lpt1", "lpt2", "lpt3", "lpt4", "lpt5", "lpt6", "lpt7", "lpt8", "lpt9" };
    if (kReserved.count(stem) != 0) {
        return kDefault;
    }
    std::string l = lower(name);
    if (l.size() < 4 || l.compare(l.size() - 4, 4, ".xsq") != 0) {
        name += ".xsq";
    }
    return name;
}

namespace {
fs::path Comparable(const std::string& p) {
    std::error_code ec;
    fs::path abs = fs::absolute(fs::path(p), ec);
    if (ec) {
        abs = fs::path(p);
    }
    fs::path c = fs::weakly_canonical(abs, ec);
    if (ec) {
        c = abs.lexically_normal();
    }
    // "dir/" and "dir" name the same folder.
    std::string s = c.generic_string();
    while (s.size() > 1 && s.back() == '/') {
        s.pop_back();
    }
#ifdef _WIN32
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) { return (char)std::tolower(ch); });
#endif
    return fs::path(s);
}
} // namespace

bool PathIsInside(const std::string& path, const std::string& folder) {
    if (path.empty() || folder.empty()) {
        return false;
    }
    fs::path p = Comparable(path);
    fs::path f = Comparable(folder);
    auto pi = p.begin();
    for (auto fi = f.begin(); fi != f.end(); ++fi, ++pi) {
        if (pi == p.end() || *pi != *fi) {
            return false;
        }
    }
    return true;
}

std::string SessionSaveRefusal(const std::string& target, const std::vector<std::string>& temporaryFolders) {
    if (target.empty()) {
        return "Choose where to save the sequence.";
    }
    for (const auto& t : temporaryFolders) {
        if (PathIsInside(target, t)) {
            return "This folder belongs to the collaboration session and is deleted when the session ends:\n" + t +
                   "\n\nChoose a folder outside it to keep your copy of the sequence.";
        }
    }
    return "";
}

std::string SessionSaveStartFolder(const std::vector<std::string>& candidates, const std::vector<std::string>& temporaryFolders) {
    for (const auto& c : candidates) {
        std::error_code ec;
        if (c.empty() || !fs::is_directory(fs::path(c), ec)) {
            continue;
        }
        bool temporary = std::any_of(temporaryFolders.begin(), temporaryFolders.end(), [&](const std::string& t) { return PathIsInside(c, t); });
        if (!temporary) {
            return c;
        }
    }
    return "";
}

std::string ShowCopyFolderRefusal(const std::string& folder, const std::vector<std::string>& temporaryFolders) {
    if (folder.empty()) {
        return "Choose a folder for the copy of the show.";
    }
    if (std::string r = SessionSaveRefusal((fs::path(folder) / "x").string(), temporaryFolders); !r.empty()) {
        return r;
    }
    std::error_code ec;
    fs::path p(folder);
    if (!fs::exists(p, ec)) {
        return "";
    }
    if (!fs::is_directory(p, ec)) {
        return folder + " is not a folder.";
    }
    if (!fs::is_empty(p, ec) || ec) {
        return "Choose an empty folder for the copy of the show: " + folder +
               " already has files in it, which the copy could overwrite.";
    }
    return "";
}

bool CopyFolderContents(const std::string& from, const std::string& to, std::string& error) {
    std::error_code ec;
    fs::create_directories(fs::path(to), ec);
    if (ec) {
        error = "Could not create " + to + ": " + ec.message();
        return false;
    }
    fs::recursive_directory_iterator it(fs::path(from), fs::directory_options::skip_permission_denied, ec);
    if (ec) {
        error = "Could not read " + from + ": " + ec.message();
        return false;
    }
    for (const fs::recursive_directory_iterator end; it != end; it.increment(ec)) {
        if (ec) {
            error = "Could not read " + from + ": " + ec.message();
            return false;
        }
        fs::path rel = it->path().lexically_relative(fs::path(from));
        fs::path dest = fs::path(to) / rel;
        if (it->is_directory(ec)) {
            fs::create_directories(dest, ec);
        } else if (it->is_regular_file(ec)) {
            fs::copy_file(it->path(), dest, fs::copy_options::none, ec);
        } else {
            continue; // links and special files are not part of a show copy
        }
        if (ec) {
            error = "Could not copy " + it->path().string() + " to " + dest.string() + ": " + ec.message();
            return false;
        }
    }
    return true;
}

std::string ResolvePath(const std::string& showDir, const std::string& path) {
    if (path.empty()) {
        return "";
    }
    try {
        bool rooted = FileUtils::IsAbsoluteOrRootedPath(path);
        if (!rooted && !showDir.empty()) {
            // Native separators, like FixFile's results: appending a "/" path
            // keeps its slashes on Windows, and the same file then had two
            // spellings (hash cache keys, comparisons).
            std::string direct = (fs::path(showDir) / fs::path(ForwardSlashes(path))).make_preferred().string();
            if (FileExists(direct)) {
                return direct;
            }
        } else if (rooted && FileExists(path)) {
            return path;
        }
        std::string fixed = FileUtils::FixFile(showDir, path);
        // FixFile hands back the path unchanged when it finds nothing; a
        // relative one would then name a file in the process's directory.
        if (fixed.empty() || !FileUtils::IsAbsoluteOrRootedPath(fixed) || !FileExists(fixed)) {
            return "";
        }
        return fixed;
    } catch (const std::exception&) {
        return "";
    }
}

// ---------------------------------------------------------------------------
// Collecting and comparing
// ---------------------------------------------------------------------------

std::vector<SessionFile> CollectSessionFiles(const std::string& showDir, SequenceElements& seq, const std::vector<std::string>& audio) {
    std::vector<SessionFile> out;
    std::set<std::string> seen;
    std::set<std::string> paths;
    auto add = [&](const std::string& reference, const std::string& resolved, FileRole role) {
        if (resolved.empty()) {
            return;
        }
        if (!seen.insert(Canonical(resolved).string()).second) {
            return;
        }
        std::string path = role == FileRole::Layout ? reference : ManifestPathFor(showDir, reference, resolved);
        // Two references to different files can't share one manifest path.
        if (path.empty() || !paths.insert(path).second) {
            return;
        }
        out.push_back({ path, role, resolved });
    };

    if (!showDir.empty()) {
        ObtainAccessToURL(showDir);
        for (const char* name : { kLayoutFile, kNetworksFile }) {
            std::string p = (fs::path(showDir) / name).string();
            if (FileExists(p)) {
                add(name, p, FileRole::Layout);
            }
        }
    }
    for (const auto& a : audio) {
        add(a, ResolvePath(showDir, a), FileRole::Audio);
    }

    const SequenceMedia& media = seq.GetSequenceMedia();
    auto embedded = [&](const std::string& ref) { return media.GetMediaEmbedState(ref).first; };
    for (const auto& [key, type] : media.GetAllMediaPaths()) {
        if (!embedded(key)) {
            add(key, ResolvePath(showDir, key), FileRole::Media);
        }
    }

    RenderContext* ctx = seq.GetRenderContext();
    if (ctx == nullptr) {
        return out;
    }
    EffectManager& em = ctx->GetEffectManager();
    auto layerRefs = [&](EffectLayer* layer, Model* model) {
        for (int k = 0; k < layer->GetEffectCount(); ++k) {
            Effect* e = layer->GetEffect(k);
            int idx = e->GetEffectIndex();
            if (idx < 0 || idx >= (int)em.size() || em[idx] == nullptr) {
                continue;
            }
            for (const auto& ref : em[idx]->GetFileReferences(ctx, model, e->GetSettings())) {
                if (!embedded(ref)) {
                    add(ref, ResolvePath(showDir, ref), FileRole::Media);
                }
            }
        }
    };
    auto modelRefs = [&](Model* m) {
        if (m == nullptr) {
            return;
        }
        std::vector<Model*> models{ m };
        if (auto* g = dynamic_cast<ModelGroup*>(m)) {
            for (Model* member : g->GetFlatModels(true, false)) {
                models.push_back(member);
            }
        }
        for (Model* mm : models) {
            for (const auto& ref : mm->GetFileReferences()) {
                add(ref, ResolvePath(showDir, ref), FileRole::Media);
            }
        }
    };
    for (size_t i = 0; i < seq.GetElementCount(); ++i) {
        Element* el = seq.GetElement(i);
        if (el == nullptr || el->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            continue;
        }
        Model* model = ctx->GetModel(el->GetModelName());
        modelRefs(model);
        for (size_t l = 0; l < el->GetEffectLayerCount(); ++l) {
            layerRefs(el->GetEffectLayer((int)l), model);
        }
        auto* me = dynamic_cast<ModelElement*>(el);
        if (me == nullptr) {
            continue;
        }
        for (int s = 0; s < me->GetSubModelAndStrandCount(); ++s) {
            SubModelElement* sub = me->GetSubModel(s);
            Model* subModel = ctx->GetModel(sub->GetFullName());
            if (subModel == nullptr) {
                subModel = model;
            }
            for (size_t l = 0; l < sub->GetEffectLayerCount(); ++l) {
                layerRefs(sub->GetEffectLayer((int)l), subModel);
            }
            if (auto* strand = dynamic_cast<StrandElement*>(sub)) {
                for (int n = 0; n < strand->GetNodeLayerCount(); ++n) {
                    layerRefs(strand->GetNodeLayer(n), subModel);
                }
            }
        }
    }
    return out;
}

std::vector<FileEntry> BuildManifest(const std::vector<SessionFile>& files, HashCache* cache, std::map<std::string, std::string>* local) {
    std::vector<FileEntry> out;
    for (const auto& f : files) {
        FileEntry e;
        e.path = f.path;
        e.role = f.role;
        if (!Hash(cache, f.local, e.size, e.hash)) {
            continue;
        }
        out.push_back(e);
        if (local != nullptr) {
            (*local)[f.path] = f.local;
        }
    }
    return out;
}

std::vector<FileCheck> ResolveManifest(const std::string& showDir, const std::vector<FileEntry>& manifest) {
    std::vector<FileCheck> out;
    if (!showDir.empty()) {
        ObtainAccessToURL(showDir);
    }
    for (const auto& e : manifest) {
        FileCheck c;
        c.entry = e;
        // A layout file is the one at the top of the show folder, never one
        // FixFile turns up somewhere else.
        if (e.role == FileRole::Layout) {
            std::string direct = showDir.empty() ? std::string() : (fs::path(showDir) / CachePathFor(e.path)).make_preferred().string();
            c.local = !direct.empty() && FileExists(direct) ? direct : std::string();
        } else {
            c.local = ResolvePath(showDir, e.path);
        }
        out.push_back(std::move(c));
    }
    return out;
}

void HashChecks(std::vector<FileCheck>& checks, HashCache* cache) {
    for (auto& c : checks) {
        uint64_t size = 0;
        uint64_t hash = 0;
        if (c.local.empty() || !Hash(cache, c.local, size, hash)) {
            c.state = FileState::Missing;
            c.local.clear();
        } else {
            c.state = size == c.entry.size && hash == c.entry.hash ? FileState::Match : FileState::Different;
        }
    }
}

std::vector<FileCheck> CompareManifest(const std::string& showDir, const std::vector<FileEntry>& manifest, HashCache* cache) {
    std::vector<FileCheck> out = ResolveManifest(showDir, manifest);
    HashChecks(out, cache);
    return out;
}

// ---------------------------------------------------------------------------
// Layout differences
// ---------------------------------------------------------------------------

namespace {

struct DiffCounts {
    std::vector<std::string> changed;
    std::vector<std::string> mineOnly;
    std::vector<std::string> hostOnly;
};

std::string NameList(const std::vector<std::string>& names) {
    std::string out;
    size_t shown = 0;
    for (const auto& n : names) {
        if (n.empty()) {
            continue;
        }
        if (shown == 5) {
            out += ", +" + std::to_string(names.size() - shown) + " more";
            break;
        }
        out += (shown ? ", " : "") + n;
        ++shown;
    }
    return out.empty() ? out : " (" + out + ")";
}

void DiffChildren(ShowFileMerge::FileKind kind, pugi::xml_node mine, pugi::xml_node host, int depth, std::map<std::string, DiffCounts>& counts) {
    // Children keyed by tag and name; unnamed ones by tag and position.
    auto keyed = [](pugi::xml_node parent) {
        std::map<std::pair<std::string, std::string>, pugi::xml_node> out;
        std::map<std::string, int> unnamed;
        for (pugi::xml_node c : parent.children()) {
            if (c.type() != pugi::node_element) {
                continue;
            }
            std::string name = LayoutRecords::NameOf(c);
            std::string key = name.empty() ? "#" + std::to_string(unnamed[c.name()]++) : name;
            out.emplace(std::make_pair(std::string(c.name()), key), c);
        }
        return out;
    };
    auto a = keyed(mine);
    auto b = keyed(host);
    auto label = [](const std::pair<std::string, std::string>& k) { return k.second[0] == '#' ? std::string() : k.second; };
    for (const auto& [k, node] : a) {
        auto it = b.find(k);
        if (it == b.end()) {
            counts[k.first].mineOnly.push_back(label(k));
            continue;
        }
        bool named = k.second[0] != '#';
        if (!named && depth == 0) {
            DiffChildren(kind, node, it->second, 1, counts);
        } else if (!ShowFileMerge::RecordsEqual(kind, node, it->second)) {
            counts[k.first].changed.push_back(label(k));
        }
    }
    for (const auto& [k, node] : b) {
        if (a.find(k) == a.end()) {
            counts[k.first].hostOnly.push_back(label(k));
        }
    }
}

} // namespace

LayoutDifference DescribeLayoutDifference(const std::string& file, const std::string& mine, const std::string& host) {
    LayoutDifference out;
    pugi::xml_document a;
    pugi::xml_document b;
    if (!a.load_buffer(mine.data(), mine.size()) || !b.load_buffer(host.data(), host.size()) || !a.document_element() ||
        !b.document_element()) {
        out.readable = false;
        out.changes = 1;
        out.text = file + " could not be read";
        return out;
    }
    auto kind = file == kNetworksFile ? ShowFileMerge::FileKind::Networks : ShowFileMerge::FileKind::RgbEffects;
    std::map<std::string, DiffCounts> counts;
    DiffChildren(kind, a.document_element(), b.document_element(), 0, counts);
    auto part = [&](const std::string& tag, const std::vector<std::string>& names, const char* what) {
        if (names.empty()) {
            return;
        }
        out.changes += names.size();
        if (!out.text.empty()) {
            out.text += "; ";
        }
        out.text += std::to_string(names.size()) + " " + tag + (names.size() == 1 ? "" : "s") + " " + what + NameList(names);
    };
    for (const auto& [tag, c] : counts) {
        part(tag, c.changed, "changed");
        part(tag, c.mineOnly, "only on this device");
        part(tag, c.hostOnly, "only on the host");
    }
    return out;
}

// ---------------------------------------------------------------------------
// Files
// ---------------------------------------------------------------------------

bool CopyFileTo(const std::string& from, const std::string& to) {
    std::error_code ec;
    fs::create_directories(fs::path(to).parent_path(), ec);
    fs::copy_file(from, to, fs::copy_options::overwrite_existing, ec);
    return !ec;
}

bool MoveFileTo(const std::string& from, const std::string& to) {
    std::error_code ec;
    fs::create_directories(fs::path(to).parent_path(), ec);
    fs::remove(to, ec);
    fs::rename(from, to, ec);
    if (!ec) {
        return true;
    }
    // Across volumes: copy, then drop the original.
    if (!CopyFileTo(from, to)) {
        return false;
    }
    fs::remove(from, ec);
    return true;
}

} // namespace Collab
