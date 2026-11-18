/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowAutosave.h"

#include "ExternalHooks.h"
#include "FileUtils.h"
#include "string_utils.h"

#include <cctype>
#include <cinttypes>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <system_error>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace {

std::mutex s_identityLock;
std::string s_installId;
std::string s_deviceName;
std::string s_appVersion;

std::optional<std::filesystem::path> ToPath(const std::string& s) {
    try {
        return std::filesystem::path(s);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::string JoinPath(const std::string& dir, const std::string& name) {
    if (dir.empty()) {
        return name;
    }
    char last = dir.back();
    if (last == '/' || last == '\\') {
        return dir + name;
    }
    return dir + "/" + name;
}

std::optional<std::string> ReadFile(const std::string& path) {
    auto p = ToPath(path);
    if (!p || !FileExists(path)) {
        return std::nullopt;
    }
    std::ifstream ifs(*p, std::ios::binary);
    if (!ifs.is_open()) {
        return std::nullopt;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    if (ifs.bad()) {
        return std::nullopt;
    }
    return ss.str();
}

std::string HashToHex(uint64_t h) {
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%016" PRIx64, h);
    return buf;
}

uint64_t HexToHash(const std::string& s) {
    return std::strtoull(s.c_str(), nullptr, 16);
}

nlohmann::json FingerprintToJson(const ShowFileFingerprint& fp) {
    return nlohmann::json{ { "exists", fp.exists }, { "size", fp.size }, { "hash", HashToHex(fp.hash) } };
}

ShowFileFingerprint FingerprintFromJson(const nlohmann::json& j) {
    ShowFileFingerprint fp;
    if (!j.is_object()) {
        return fp;
    }
    fp.exists = j.value("exists", false);
    fp.size = j.value("size", (uint64_t)0);
    fp.hash = HexToHash(j.value("hash", std::string()));
    return fp;
}

bool SameContent(const ShowFileFingerprint& a, const ShowFileFingerprint& b) {
    return a.exists && b.exists && a.size == b.size && a.hash == b.hash;
}

} // namespace

namespace ShowAutosave {

void SetIdentity(const std::string& installId, const std::string& deviceName, const std::string& appVersion) {
    std::lock_guard<std::mutex> l(s_identityLock);
    s_installId = installId;
    s_deviceName = deviceName;
    s_appVersion = appVersion;
}

std::string InstallId() {
    std::lock_guard<std::mutex> l(s_identityLock);
    return s_installId;
}

std::string DeviceName() {
    std::lock_guard<std::mutex> l(s_identityLock);
    return s_deviceName;
}

std::string NewInstallId() {
    return GenerateGuid();
}

bool IsValidInstallId(const std::string& id) {
    // Used as a folder name, so only a plain UUID is accepted.
    if (id.size() != 36) {
        return false;
    }
    for (size_t i = 0; i < id.size(); ++i) {
        const char c = id[i];
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (c != '-') {
                return false;
            }
        } else if (!std::isxdigit((unsigned char)c)) {
            return false;
        }
    }
    return true;
}

std::string ValidInstallIdOr(const std::string& stored) {
    return IsValidInstallId(stored) ? stored : NewInstallId();
}

std::string ShowStateDir(const std::string& showDir) {
    return JoinPath(showDir, ".xlights");
}

std::string LayoutAutosaveDir(const std::string& showDir, const std::string& installId) {
    return JoinPath(JoinPath(ShowStateDir(showDir), "autosave"), installId);
}

static std::string BackupNameFor(const std::string& realFileName) {
    auto p = ToPath(realFileName);
    if (!p) {
        return realFileName + ".xbkp";
    }
    std::string ext = p->extension().string();
    for (auto& c : ext) {
        c = (char)std::tolower((unsigned char)c);
    }
    return p->stem().string() + (ext == ".json" ? ".jbkp" : ".xbkp");
}

std::string LayoutAutosavePath(const std::string& showDir, const std::string& installId, const std::string& realFileName) {
    return JoinPath(LayoutAutosaveDir(showDir, installId), BackupNameFor(realFileName));
}

std::string LayoutAutosavePath(const std::string& showDir, const std::string& realFileName) {
    return LayoutAutosavePath(showDir, InstallId(), realFileName);
}

std::string PrepareLayoutAutosavePath(const std::string& showDir, const std::string& realFileName) {
    const std::string id = InstallId();
    if (showDir.empty() || !IsValidInstallId(id)) {
        return std::string();
    }
    const std::string dir = LayoutAutosaveDir(showDir, id);
    ObtainAccessToURL(showDir, true);
    auto p = ToPath(dir);
    if (!p) {
        return std::string();
    }
    std::error_code ec;
    std::filesystem::create_directories(*p, ec);
    if (!std::filesystem::is_directory(*p, ec)) {
        spdlog::warn("ShowAutosave: cannot create the autosave folder {}", dir);
        return std::string();
    }
    return LayoutAutosavePath(showDir, id, realFileName);
}

std::string LegacyLayoutAutosavePath(const std::string& showDir, const std::string& realFileName) {
    return JoinPath(showDir, BackupNameFor(realFileName));
}

std::string SidecarPath(const std::string& backupPath) {
    return backupPath + ".json";
}

bool IsShowStateDirName(const std::string& name) {
    return name == ".xlights";
}

std::string ToJson(const ShowAutosaveInfo& info) {
    nlohmann::json j;
    j["installId"] = info.installId;
    j["device"] = info.deviceName;
    j["appVersion"] = info.appVersion;
    j["savedAt"] = info.savedAt;
    j["base"] = FingerprintToJson(info.base);
    j["backup"] = FingerprintToJson(info.backup);
    if (info.declined) {
        j["declined"] = true;
    }
    return j.dump(2);
}

std::optional<ShowAutosaveInfo> FromJson(const std::string& json) {
    nlohmann::json j = nlohmann::json::parse(json, nullptr, false);
    if (j.is_discarded() || !j.is_object() || !j.contains("backup")) {
        return std::nullopt;
    }
    try {
        ShowAutosaveInfo info;
        info.installId = j.value("installId", std::string());
        info.deviceName = j.value("device", std::string());
        info.appVersion = j.value("appVersion", std::string());
        info.savedAt = j.value("savedAt", (int64_t)0);
        info.base = FingerprintFromJson(j.value("base", nlohmann::json::object()));
        info.backup = FingerprintFromJson(j["backup"]);
        info.declined = j.value("declined", false);
        return info;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<ShowAutosaveInfo> ReadSidecar(const std::string& backupPath) {
    auto text = ReadFile(SidecarPath(backupPath));
    if (!text) {
        return std::nullopt;
    }
    return FromJson(*text);
}

bool WriteSidecar(const std::string& backupPath, const ShowFileFingerprint& base) {
    ShowAutosaveInfo identity;
    {
        std::lock_guard<std::mutex> l(s_identityLock);
        identity.installId = s_installId;
        identity.deviceName = s_deviceName;
        identity.appVersion = s_appVersion;
    }
    return WriteSidecar(backupPath, base, identity);
}

bool WriteSidecar(const std::string& backupPath, const ShowFileFingerprint& base, const ShowAutosaveInfo& identity) {
    auto backupFp = ShowFileGuard::FingerprintFile(backupPath);
    if (!backupFp || !backupFp->exists) {
        return false;
    }
    ShowAutosaveInfo info = identity;
    info.declined = false;
    info.savedAt = (int64_t)std::time(nullptr);
    info.base = base;
    info.backup = *backupFp;
    if (!ShowFileGuard::WriteFileAtomically(SidecarPath(backupPath), ToJson(info))) {
        spdlog::warn("ShowAutosave: could not write {}", SidecarPath(backupPath));
        return false;
    }
    return true;
}

ShowFileFingerprint BaseFor(const std::string& realPath, const ShowFileGuard& guard) {
    if (auto recorded = guard.RecordedFingerprint(realPath)) {
        return *recorded;
    }
    auto now = ShowFileGuard::FingerprintFile(realPath);
    return now ? *now : ShowFileFingerprint{};
}

AutosaveCheck Check(const std::string& realPath, const std::string& backupPath) {
    return Check(realPath, backupPath, InstallId());
}

AutosaveCheck Check(const std::string& realPath, const std::string& backupPath, const std::string& myInstallId) {
    AutosaveCheck result;
    if (backupPath.empty() || !FileExists(backupPath)) {
        return result;
    }
    auto backupFp = ShowFileGuard::FingerprintFile(backupPath);
    auto realFp = ShowFileGuard::FingerprintFile(realPath);
    if (!backupFp || !backupFp->exists || !realFp) {
        return result;
    }
    auto backupTicks = FileUtils::GetFileModTimeTicks(backupPath);
    result.backupTime = backupTicks ? (int64_t)*backupTicks : 0;

    auto info = ReadSidecar(backupPath);
    if (info && info->backup == *backupFp) {
        result.hasInfo = true;
        result.info = *info;
    }

    if (result.hasInfo && !result.info.installId.empty()) {
        if (result.info.installId != myInstallId) {
            return result;
        }
        if (SameContent(*backupFp, *realFp)) {
            result.offer = AutosaveOffer::Stale;
            return result;
        }
        if (result.info.declined) {
            return result;
        }
        result.offer = (result.info.base == *realFp) ? AutosaveOffer::Offer : AutosaveOffer::OfferConflict;
        return result;
    }

    // An older version's autosave, or one whose sidecar no longer describes it.
    // Whose it is and what it was based on are unknown, so it keeps the old
    // "newer than the real file" rule as well as having to differ from it.
    if (result.hasInfo && result.info.declined) {
        return result;
    }
    if (SameContent(*backupFp, *realFp)) {
        return result;
    }
    if (realFp->exists) {
        auto realTicks = FileUtils::GetFileModTimeTicks(realPath);
        if (!backupTicks || !realTicks || *backupTicks <= *realTicks) {
            return result;
        }
    }
    result.offer = AutosaveOffer::OfferLegacy;
    return result;
}

std::string MakeTaggedCopyPath(const std::string& path, const std::string& tag, std::time_t when) {
    std::tm tmv{};
#ifdef _WIN32
    localtime_s(&tmv, &when);
#else
    localtime_r(&when, &tmv);
#endif
    char stamp[32];
    std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &tmv);

    auto p = ToPath(path);
    if (!p) {
        return path + "_" + tag + "_" + stamp;
    }
    std::filesystem::path dir = p->parent_path();
    std::string base = p->stem().string() + "_" + tag + "_" + stamp;
    std::string ext = p->extension().string();
    std::filesystem::path candidate = dir / (base + ext);
    for (int n = 2; FileExists(candidate.string(), false) && n < 1000; ++n) {
        candidate = dir / (base + "_" + std::to_string(n) + ext);
    }
    return candidate.string();
}

bool Restore(const std::string& realPath, const std::string& backupPath, bool keepCurrentAsCopy,
             ShowFileGuard& guard, std::string* keptCopyPath) {
    ObtainAccessToURL(realPath, true);
    auto bytes = ReadFile(backupPath);
    if (!bytes) {
        spdlog::error("ShowAutosave: cannot read the autosave {}", backupPath);
        return false;
    }
    if (keepCurrentAsCopy && FileExists(realPath)) {
        auto current = ReadFile(realPath);
        if (!current) {
            spdlog::error("ShowAutosave: cannot read {} to keep a copy of it; not restoring", realPath);
            return false;
        }
        const std::string copy = MakeTaggedCopyPath(realPath, "before_restore", std::time(nullptr));
        if (!ShowFileGuard::WriteFileAtomically(copy, *current)) {
            spdlog::error("ShowAutosave: cannot keep a copy of {} at {}; not restoring", realPath, copy);
            return false;
        }
        spdlog::info("ShowAutosave: kept the current {} as {}", realPath, copy);
        if (keptCopyPath) {
            *keptCopyPath = copy;
        }
    }
    if (!guard.SaveFile(realPath, *bytes)) {
        spdlog::error("ShowAutosave: restoring {} from {} failed", realPath, backupPath);
        return false;
    }
    spdlog::info("ShowAutosave: restored {} from {}", realPath, backupPath);
    Remove(backupPath);
    return true;
}

std::string SaveAsCopy(const std::string& realPath, const std::string& backupPath) {
    ObtainAccessToURL(realPath, true);
    auto bytes = ReadFile(backupPath);
    if (!bytes) {
        spdlog::error("ShowAutosave: cannot read the autosave {}", backupPath);
        return std::string();
    }
    const std::string copy = ShowFileGuard::MakeCopyPath(realPath);
    if (!ShowFileGuard::WriteFileAtomically(copy, *bytes)) {
        spdlog::error("ShowAutosave: cannot write the autosave {} to {}", backupPath, copy);
        return std::string();
    }
    spdlog::info("ShowAutosave: saved the autosave {} as {}", backupPath, copy);
    Remove(backupPath);
    return copy;
}

bool Decline(const std::string& backupPath) {
    auto backupFp = ShowFileGuard::FingerprintFile(backupPath);
    if (!backupFp || !backupFp->exists) {
        return false;
    }
    ShowAutosaveInfo info;
    auto existing = ReadSidecar(backupPath);
    if (existing && existing->backup == *backupFp) {
        info = *existing;
    } else {
        info.savedAt = (int64_t)std::time(nullptr);
        info.backup = *backupFp;
    }
    info.declined = true;
    return ShowFileGuard::WriteFileAtomically(SidecarPath(backupPath), ToJson(info));
}

void Remove(const std::string& backupPath) {
    if (backupPath.empty()) {
        return;
    }
    ObtainAccessToURL(backupPath, true);
    for (const std::string& f : { backupPath, SidecarPath(backupPath) }) {
        auto p = ToPath(f);
        if (!p) {
            continue;
        }
        std::error_code ec;
        std::filesystem::remove(*p, ec);
    }
}

bool RemoveIfMine(const std::string& backupPath) {
    if (backupPath.empty() || !FileExists(backupPath, false)) {
        return false;
    }
    auto info = ReadSidecar(backupPath);
    if (!info || info->installId.empty() || info->installId != InstallId()) {
        return false;
    }
    auto backupFp = ShowFileGuard::FingerprintFile(backupPath);
    if (!backupFp || !(info->backup == *backupFp)) {
        return false;
    }
    Remove(backupPath);
    return true;
}

} // namespace ShowAutosave
