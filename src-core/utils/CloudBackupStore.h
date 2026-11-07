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

// The backups of one show in the bucket. Each backup is a set of objects in
// "<prefix>/<show>/" named after its UTC stamp:
//   <stamp>.zip, <stamp>.manifest.json, <stamp>.thumbnail.png (optional)
// plus latest.manifest.json, a copy of the newest manifest.

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class S3Client;
struct S3Object;

struct CloudBackup {
    std::string stamp; // 2026-11-06-143000 (UTC)
    std::string manifestKey;
    std::string zipKey;
    std::string thumbnailKey; // empty when there is none
    uint64_t zipSize = 0;
};

// The bucket calls CloudBackupStore needs, so it can run against a fake in tests.
struct CloudObjectOps {
    std::function<bool(const std::string& key, std::string& error)> remove;
    std::function<bool(const std::string& key, std::string& data, std::string& error)> read;
    std::function<bool(const std::string& key, const std::string& data, std::string& error)> write;

    static CloudObjectOps For(S3Client& client);
};

namespace CloudBackupStore {

constexpr const char* LATEST_MANIFEST = "latest.manifest.json";

// "<prefix>/<show>/", ignoring stray slashes around the prefix.
std::string ShowFolderKey(const std::string& prefix, const std::string& show);

// Complete backups (manifest and archive both present) in a listing of
// folder, newest first.
std::vector<CloudBackup> FromListing(const std::vector<S3Object>& objects, const std::string& folder);

// The backups to remove so that only the newest keep are left. keep <= 0 keeps
// everything. protectStamp (the backup just uploaded) is never selected and
// counts as one of the kept backups.
std::vector<CloudBackup> SelectForPruning(const std::vector<CloudBackup>& backups, int keep,
                                          const std::string& protectStamp = {});

// Deletes toDelete, out of all the backups of folder. The manifest goes first,
// so a backup that is only partly deleted no longer lists as complete. When the
// newest backup goes, latest.manifest.json is pointed at the newest one left,
// or removed when none is left. Stops at the first failure.
bool Delete(const CloudObjectOps& ops, const std::string& folder, const std::vector<CloudBackup>& all,
            const std::vector<CloudBackup>& toDelete, std::string& error);

} // namespace CloudBackupStore
