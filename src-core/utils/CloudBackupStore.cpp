/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CloudBackupStore.h"

#include "S3Client.h"
#include "string_utils.h"

#include <algorithm>
#include <map>
#include <set>

namespace {

const std::string MANIFEST_SUFFIX = ".manifest.json";

std::string TrimSlashes(std::string s) {
    while (!s.empty() && (s.back() == '/' || s.back() == '\\')) {
        s.pop_back();
    }
    while (!s.empty() && (s.front() == '/' || s.front() == '\\')) {
        s.erase(0, 1);
    }
    return s;
}

void SortNewestFirst(std::vector<CloudBackup>& backups) {
    std::sort(backups.begin(), backups.end(), [](const CloudBackup& a, const CloudBackup& b) { return a.stamp > b.stamp; });
}

} // namespace

CloudObjectOps CloudObjectOps::For(S3Client& client) {
    CloudObjectOps ops;
    ops.remove = [&client](const std::string& key, std::string& error) { return client.DeleteObject(key, error); };
    ops.read = [&client](const std::string& key, std::string& data, std::string& error) {
        return client.ReadObject(key, data, error);
    };
    ops.write = [&client](const std::string& key, const std::string& data, std::string& error) {
        return client.PutObject(key, data, "application/json", error);
    };
    return ops;
}

namespace CloudBackupStore {

std::string ShowFolderKey(const std::string& prefix, const std::string& show) {
    std::string p = TrimSlashes(prefix);
    return (p.empty() ? "" : p + "/") + show + "/";
}

std::vector<CloudBackup> FromListing(const std::vector<S3Object>& objects, const std::string& folder) {
    std::map<std::string, uint64_t> sizes;
    for (const auto& o : objects) {
        sizes[o.key] = o.size;
    }
    std::vector<CloudBackup> backups;
    for (const auto& o : objects) {
        if (!StartsWith(o.key, folder)) {
            continue;
        }
        std::string name = o.key.substr(folder.size());
        if (!EndsWith(name, MANIFEST_SUFFIX) || name.find('/') != std::string::npos || name == LATEST_MANIFEST) {
            continue;
        }
        CloudBackup b;
        b.stamp = name.substr(0, name.size() - MANIFEST_SUFFIX.size());
        if (b.stamp.empty()) {
            continue;
        }
        b.manifestKey = o.key;
        b.zipKey = folder + b.stamp + ".zip";
        auto z = sizes.find(b.zipKey);
        if (z == sizes.end()) {
            continue;
        }
        b.zipSize = z->second;
        std::string thumb = folder + b.stamp + ".thumbnail.png";
        if (sizes.count(thumb)) {
            b.thumbnailKey = thumb;
        }
        backups.push_back(std::move(b));
    }
    SortNewestFirst(backups);
    return backups;
}

std::vector<CloudBackup> SelectForPruning(const std::vector<CloudBackup>& backups, int keep, const std::string& protectStamp) {
    std::vector<CloudBackup> out;
    if (keep <= 0) {
        return out;
    }
    std::vector<CloudBackup> sorted = backups;
    SortNewestFirst(sorted);
    bool hasProtected = !protectStamp.empty() &&
                        std::any_of(sorted.begin(), sorted.end(), [&](const CloudBackup& b) { return b.stamp == protectStamp; });
    int kept = hasProtected ? 1 : 0;
    for (const auto& b : sorted) {
        if (hasProtected && b.stamp == protectStamp) {
            continue;
        }
        if (kept < keep) {
            ++kept;
        } else {
            out.push_back(b);
        }
    }
    return out;
}

bool Delete(const CloudObjectOps& ops, const std::string& folder, const std::vector<CloudBackup>& all,
            const std::vector<CloudBackup>& toDelete, std::string& error) {
    std::set<std::string> gone;
    bool ok = true;
    for (const auto& b : toDelete) {
        if (!ops.remove(b.manifestKey, error)) {
            ok = false;
            break;
        }
        gone.insert(b.stamp);
        if (!ops.remove(b.zipKey, error) || (!b.thumbnailKey.empty() && !ops.remove(b.thumbnailKey, error))) {
            ok = false;
            break;
        }
    }

    std::vector<CloudBackup> sorted = all;
    SortNewestFirst(sorted);
    if (sorted.empty() || !gone.count(sorted.front().stamp)) {
        return ok;
    }
    const std::string latestKey = folder + LATEST_MANIFEST;
    std::string latestError;
    auto left = std::find_if(sorted.begin(), sorted.end(), [&](const CloudBackup& b) { return !gone.count(b.stamp); });
    bool latestOk;
    if (left == sorted.end()) {
        latestOk = ops.remove(latestKey, latestError);
    } else {
        std::string manifest;
        latestOk = ops.read(left->manifestKey, manifest, latestError) && ops.write(latestKey, manifest, latestError);
    }
    if (!latestOk && ok) {
        error = "Unable to update " + std::string(LATEST_MANIFEST) + ": " + latestError;
        ok = false;
    }
    return ok;
}

} // namespace CloudBackupStore
