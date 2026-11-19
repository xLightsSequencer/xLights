/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowConflictedCopies.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <system_error>

#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <spdlog/spdlog.h>

#include "ExternalHooks.h"
#include "FileUtils.h"
#include "ShowAutosave.h"
#include "globals.h"

namespace ShowConflictedCopies {

namespace {

struct ShowFile {
    FileKind kind;
    const char* name;
    const char* stem;
    const char* ext;
};

constexpr ShowFile kShowFiles[] = {
    { FileKind::RgbEffects, XLIGHTS_RGBEFFECTS_FILE, "xlights_rgbeffects", ".xml" },
    { FileKind::Networks, XLIGHTS_NETWORK_FILE, "xlights_networks", ".xml" },
    { FileKind::Presets, XLIGHTS_PRESETS_FILE, "xlights_effectpresets", ".json" },
};

std::string Lower(std::string s) {
    for (auto& c : s) {
        c = (char)std::tolower((unsigned char)c);
    }
    return s;
}

bool EndsWithNoCase(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() && Lower(s.substr(s.size() - suffix.size())) == suffix;
}

std::string JoinPath(const std::string& dir, const std::string& name) {
    if (dir.empty()) {
        return name;
    }
    char last = dir.back();
    return (last == '/' || last == '\\') ? dir + name : dir + "/" + name;
}

std::string FileNameOf(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

// What follows the stem (before the extension) in a sync client's copy.
const char* ClientForSuffix(const std::string& rest) {
    static const std::regex dropbox(R"(^ \((.*conflicted copy.*)\)$)", std::regex::icase);
    static const std::regex icloud(R"(^ ([2-9]|[1-9][0-9]+)$)");
    static const std::regex google(R"(^ ?\(([1-9][0-9]*)\)$)");
    // A Windows computer name: up to 15 letters, digits and hyphens.
    static const std::regex onedrive(R"(^-[A-Za-z0-9][A-Za-z0-9-]{0,14}$)");
    static const std::regex syncthing(R"(^\.sync-conflict-[0-9]{8}-[0-9]{6}-[A-Z0-9]{7}$)");
    if (std::regex_match(rest, dropbox)) return "Dropbox";
    if (std::regex_match(rest, icloud)) return "iCloud";
    if (std::regex_match(rest, google)) return "Google Drive";
    if (std::regex_match(rest, syncthing)) return "Syncthing";
    if (std::regex_match(rest, onedrive)) return "OneDrive";
    return nullptr;
}

std::optional<std::string> ReadText(const std::string& path) {
    auto r = ShowFileGuard::ReadFile(path);
    if (!r || !r->second.exists) {
        return std::nullopt;
    }
    return r->first;
}

std::time_t ModTime(const std::string& path) {
    auto t = FileUtils::GetFileModTimeTicks(path);
    return t ? (std::time_t)*t : 0;
}

std::string Hex(uint64_t v) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%016llx", (unsigned long long)v);
    return buf;
}

nlohmann::json ReadIgnoreList(const std::string& path) {
    auto text = ReadText(path);
    if (!text) {
        return nlohmann::json::array();
    }
    nlohmann::json j = nlohmann::json::parse(*text, nullptr, false);
    return j.is_array() ? j : nlohmann::json::array();
}

std::string UniqueIn(const std::string& dir, const std::string& name) {
    std::string target = JoinPath(dir, name);
    if (!FileExists(target, false)) {
        return target;
    }
    return ShowAutosave::MakeTaggedCopyPath(target, "copy", std::time(nullptr));
}

} // namespace

std::optional<NameMatch> MatchFileName(const std::string& fileName) {
    for (const auto& f : kShowFiles) {
        const std::string ext = f.ext;
        const std::string stem = f.stem;
        if (fileName == f.name || !EndsWithNoCase(fileName, ext)) {
            continue;
        }
        const std::string middle = fileName.substr(0, fileName.size() - ext.size());
        if (middle.size() <= stem.size() || middle.compare(0, stem.size(), stem) != 0) {
            continue;
        }
        if (const char* client = ClientForSuffix(middle.substr(stem.size()))) {
            return NameMatch{ f.kind, f.name, client };
        }
    }
    return std::nullopt;
}

bool ContentIsKind(FileKind kind, const std::string& text) {
    if (text.empty()) {
        return false;
    }
    if (kind == FileKind::Presets) {
        nlohmann::json j = nlohmann::json::parse(text, nullptr, false);
        return !j.is_discarded() && j.is_object();
    }
    pugi::xml_document doc;
    if (!doc.load_buffer(text.data(), text.size())) {
        return false;
    }
    const std::string root = doc.document_element().name();
    return kind == FileKind::Networks ? root == "Networks" : (root == "xrgb" || root == "xlights");
}

std::vector<Found> Find(const std::string& showDir, const std::string& installId) {
    std::vector<Found> out;
    if (showDir.empty()) {
        return out;
    }
    std::error_code ec;
    std::filesystem::directory_iterator it(std::filesystem::path(showDir), ec);
    if (ec) {
        return out;
    }
    for (; it != std::filesystem::directory_iterator(); it.increment(ec)) {
        if (ec) {
            break;
        }
        std::string name;
        try {
            name = it->path().filename().string();
        } catch (const std::exception&) {
            continue;
        }
        if (name.empty() || name[0] == '.') {
            continue;
        }
        auto match = MatchFileName(name);
        if (!match) {
            continue;
        }
        std::error_code fe;
        if (!it->is_regular_file(fe)) {
            continue;
        }
        Found f;
        f.kind = match->kind;
        f.client = match->client;
        f.copyPath = JoinPath(showDir, name);
        f.realPath = JoinPath(showDir, match->realName);
        if (!FileExists(f.realPath)) {
            continue;
        }
        auto copy = ShowFileGuard::ReadFile(f.copyPath);
        if (!copy || !ContentIsKind(f.kind, copy->first)) {
            spdlog::debug("ShowConflictedCopies: '{}' looks like a conflicted copy but is not a {}", name, match->realName);
            continue;
        }
        f.fingerprint = copy->second;
        if (IsIgnored(showDir, installId, f.fingerprint)) {
            continue;
        }
        if (auto real = ShowFileGuard::FingerprintFile(f.realPath)) {
            f.sameAsCurrent = *real == f.fingerprint;
        }
        f.modified = ModTime(f.copyPath);
        out.push_back(std::move(f));
    }
    std::sort(out.begin(), out.end(), [](const Found& a, const Found& b) { return a.copyPath < b.copyPath; });
    return out;
}

std::string IgnoreListPath(const std::string& showDir, const std::string& installId) {
    if (installId.empty()) {
        return {};
    }
    return JoinPath(ShowAutosave::LayoutAutosaveDir(showDir, installId), "ignored_conflicted_copies.json");
}

bool IsIgnored(const std::string& showDir, const std::string& installId, const ShowFileFingerprint& fp) {
    const std::string path = IgnoreListPath(showDir, installId);
    if (path.empty()) {
        return false;
    }
    for (const auto& e : ReadIgnoreList(path)) {
        if (e.is_object() && e.value("size", (uint64_t)0) == fp.size && e.value("hash", std::string()) == Hex(fp.hash)) {
            return true;
        }
    }
    return false;
}

bool Ignore(const std::string& showDir, const std::string& installId, const Found& copy) {
    const std::string path = IgnoreListPath(showDir, installId);
    if (path.empty()) {
        return false;
    }
    if (IsIgnored(showDir, installId, copy.fingerprint)) {
        return true;
    }
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(ShowAutosave::LayoutAutosaveDir(showDir, installId)), ec);
    nlohmann::json list = ReadIgnoreList(path);
    list.push_back({ { "name", FileNameOf(copy.copyPath) }, { "size", copy.fingerprint.size }, { "hash", Hex(copy.fingerprint.hash) } });
    return ShowFileGuard::WriteFileAtomically(path, list.dump(2));
}

BaseChoice ChooseBase(const Found& copy, const std::string& showDir, const std::string& installId, const std::string& backupRoot) {
    BaseChoice choice;
    const std::string realName = FileNameOf(copy.realPath);
    if (!installId.empty()) {
        const std::string autosave = ShowAutosave::LayoutAutosavePath(showDir, installId, realName);
        if (auto base = ShowAutosave::ReadBaseCopy(autosave); base && ContentIsKind(copy.kind, *base)) {
            choice.source = BaseSource::Autosave;
            choice.text = std::move(*base);
            choice.path = ShowAutosave::BaseCopyPath(autosave);
            return choice;
        }
    }
    if (backupRoot.empty()) {
        return choice;
    }
    const std::time_t limit = std::min(ModTime(copy.realPath), copy.modified ? copy.modified : ModTime(copy.copyPath));
    std::vector<std::pair<std::time_t, std::string>> candidates;
    std::error_code ec;
    std::filesystem::directory_iterator it(std::filesystem::path(JoinPath(backupRoot, "Backup")), ec);
    for (; !ec && it != std::filesystem::directory_iterator(); it.increment(ec)) {
        std::error_code de;
        if (!it->is_directory(de)) {
            continue;
        }
        std::string dir;
        try {
            dir = it->path().string();
        } catch (const std::exception&) {
            continue;
        }
        if (FileNameOf(dir) == FileNameOf(BackupFolder(backupRoot))) {
            continue; // the copies moved aside, not backups
        }
        const std::string candidate = JoinPath(dir, realName);
        std::error_code fe;
        if (!std::filesystem::is_regular_file(std::filesystem::path(candidate), fe)) {
            continue;
        }
        const std::time_t t = ModTime(candidate);
        if (t != 0 && t < limit) {
            candidates.emplace_back(t, candidate);
        }
    }
    std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
    for (const auto& [t, path] : candidates) {
        if (auto text = ReadText(path); text && ContentIsKind(copy.kind, *text)) {
            choice.source = BaseSource::Backup;
            choice.text = std::move(*text);
            choice.path = path;
            break;
        }
    }
    return choice;
}

std::optional<ShowSaveMerge::Plan> PrepareMerge(const Found& copy, const BaseChoice& base, std::string* error) {
    auto mine = ShowFileGuard::ReadFile(copy.copyPath);
    auto theirs = ShowFileGuard::ReadFile(copy.realPath);
    if (!mine || !mine->second.exists || !theirs || !theirs->second.exists) {
        if (error) *error = "the file or its conflicted copy cannot be read";
        return std::nullopt;
    }
    ShowSaveMerge::FileVersions v;
    v.kind = copy.kind;
    v.path = copy.realPath;
    v.mine = std::move(mine->first);
    v.base = base.source == BaseSource::None ? std::string() : base.text;
    v.theirs = std::move(theirs->first);
    v.theirsFingerprint = theirs->second;
    v.changedOnDisk = true;
    v.write = true;
    ShowFileMerge::Options options;
    options.twoWay = base.source == BaseSource::None;
    std::vector<ShowSaveMerge::FileVersions> files;
    files.push_back(std::move(v));
    return ShowSaveMerge::Plan::FromVersions(std::move(files), options);
}

std::string BackupFolder(const std::string& backupRoot) {
    return JoinPath(JoinPath(backupRoot, "Backup"), "ConflictedCopies");
}

std::string MoveToBackup(const std::string& path, const std::string& backupRoot) {
    const std::string dir = BackupFolder(backupRoot);
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(dir), ec);
    const std::string target = UniqueIn(dir, FileNameOf(path));
    std::filesystem::rename(std::filesystem::path(path), std::filesystem::path(target), ec);
    if (!ec) {
        return target;
    }
    // Another volume: copy, check, then remove the original.
    std::error_code ce;
    if (!std::filesystem::copy_file(std::filesystem::path(path), std::filesystem::path(target), ce) || ce) {
        spdlog::warn("ShowConflictedCopies: could not move '{}' to '{}': {}", path, target, ec.message());
        return {};
    }
    auto a = ShowFileGuard::FingerprintFile(path);
    auto b = ShowFileGuard::FingerprintFile(target);
    if (!a || !b || !(*a == *b)) {
        std::filesystem::remove(std::filesystem::path(target), ce);
        return {};
    }
    std::filesystem::remove(std::filesystem::path(path), ce);
    return target;
}

std::string CopyToBackup(const std::string& path, const std::string& backupRoot) {
    const std::string dir = BackupFolder(backupRoot);
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(dir), ec);
    const std::string target = ShowAutosave::MakeTaggedCopyPath(JoinPath(dir, FileNameOf(path)), "before_merge", std::time(nullptr));
    auto text = ShowFileGuard::ReadFile(path);
    if (!text || !text->second.exists || !ShowFileGuard::WriteFileAtomically(target, text->first)) {
        return {};
    }
    return target;
}

FinishResult Finish(const ShowSaveMerge::Plan& plan, const Found& copy, const std::string& backupRoot, ShowFileGuard& guard) {
    FinishResult r;
    if (!plan.CanMerge() || !plan.AllResolved()) {
        r.error = "not every conflict has been resolved";
        return r;
    }
    r.keptCurrent = CopyToBackup(copy.realPath, backupRoot);
    if (r.keptCurrent.empty()) {
        r.error = "could not keep a copy of " + FileNameOf(copy.realPath) + " in the backup folder";
        return r;
    }
    r.writeResult = plan.Write(guard, &r.error);
    if (r.writeResult != ShowSaveMerge::WriteResult::Written) {
        return r;
    }
    r.written = true;
    r.movedCopy = MoveToBackup(copy.copyPath, backupRoot);
    if (r.movedCopy.empty()) {
        r.error = "the merge was saved, but " + FileNameOf(copy.copyPath) + " could not be moved to the backup folder";
    }
    spdlog::info("ShowConflictedCopies: merged '{}' into '{}'; it was moved to '{}', the previous file kept as '{}'", copy.copyPath,
                 copy.realPath, r.movedCopy, r.keptCurrent);
    return r;
}

std::string BackupRootFor(const std::string& showDir) {
    pugi::xml_document doc;
    const std::string path = JoinPath(showDir, XLIGHTS_RGBEFFECTS_FILE);
    if (auto text = ReadText(path); text && doc.load_buffer(text->data(), text->size())) {
        const std::string dir = doc.document_element().child("settings").child("backupDir").attribute("value").as_string();
        std::error_code ec;
        if (!dir.empty() && std::filesystem::is_directory(std::filesystem::path(dir), ec)) {
            return dir;
        }
    }
    return showDir;
}

std::string Describe(const Found& copy) {
    return FileNameOf(copy.copyPath) + " (" + copy.client + ")";
}

} // namespace ShowConflictedCopies
