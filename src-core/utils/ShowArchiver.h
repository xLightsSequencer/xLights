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

// Builds a single Zip64 archive of a whole show folder (media included) plus
// the rendered .fseq of every sequence, for the cloud backup, and extracts it
// back for a restore. Works purely on files on disk so it can run on a worker
// thread while the user keeps editing.
//
// Archive layout:
//   show/<path relative to the show folder>   everything under the show folder
//   fseq/<name>.fseq                          .fseq kept in a separate FSEQ folder
//   media/<name>                              sequence audio living outside the show
//   thumbnail.png                             optional picture of the layout
//   manifest.json                             same content as the uploaded manifest,
//                                             minus the "archive" section

#include <cstdint>
#include <ctime>
#include <filesystem>
#include <functional>
#include <list>
#include <string>
#include <utility>
#include <vector>

struct ShowArchiveOptions {
    std::string showDir;
    // FSEQ output folder; empty or equal to showDir means next to each .xsq.
    std::string fseqDir;
    // Extra folders searched for sequence audio referenced by a relative path.
    std::list<std::string> mediaDirs;
    // Folder names skipped at any depth (case-insensitive).
    std::vector<std::string> excludeDirNames{ "Backup", "RenderCache" };
    bool includeFseq = true;
};

struct ShowArchiveEntry {
    std::filesystem::path source;
    std::string zipPath;
    uint64_t size = 0;
    long long mtime = 0;
    bool hash = false;    // SHA-256 recorded in the manifest
    std::string sha256;   // filled by Create() when hash is set
    bool written = false; // filled by Create()
};

struct ShowArchiveSequence {
    std::string name;       // .xsq path relative to the show folder, without extension
    std::string xsqZipPath;
    int fseqEntry = -1;     // index into ShowArchivePlan::entries, -1 when not rendered
    int mediaEntry = -1;    // -1 when the sequence has no audio or it was not found
    std::string mediaRef;   // media file as written in the .xsq
    bool fseqUpToDate = false;
};

struct ShowArchivePlan {
    ShowArchiveOptions options;
    std::vector<ShowArchiveEntry> entries;
    std::vector<ShowArchiveSequence> sequences;
    std::vector<std::string> warnings;
    uint64_t totalBytes = 0;
    // Written as is at the root of the archive, e.g. thumbnail.png.
    std::vector<std::pair<std::string, std::string>> extraFiles;

    // Sequences whose .fseq is missing or older than the .xsq; always empty
    // when .fseq files are not included.
    std::vector<std::string> StaleOrMissingFseq() const;
};

struct ShowArchiveResult {
    bool ok = false;
    bool cancelled = false;
    std::string error;
    std::vector<std::string> warnings;
    uint64_t archiveSize = 0;
};

// Return false to cancel.
using ShowArchiveProgress = std::function<bool(uint64_t doneBytes, uint64_t totalBytes, const std::string& currentFile)>;

namespace ShowArchiver {

ShowArchivePlan Scan(const ShowArchiveOptions& options);

// Writes the archive to outZip. On failure or cancel the partial file is removed.
ShowArchiveResult Create(ShowArchivePlan& plan, const std::filesystem::path& outZip, const ShowArchiveProgress& progress = nullptr);

struct ManifestInfo {
    std::string showName;
    std::string createdUtc;
    std::string archiveKey;      // empty: no "archive" section
    uint64_t archiveSize = 0;
    std::string archiveSha256;
    std::string thumbnailKey;    // uploaded next to the manifest, empty if none
};
std::string BuildManifest(const ShowArchivePlan& plan, const ManifestInfo& info);

// What a restore needs from an uploaded manifest. ok is false when the text
// is not a show backup manifest.
struct BackupSummary {
    bool ok = false;
    std::string showName;
    std::string createdUtc;
    std::string xlightsVersion;
    std::string archiveKey;
    uint64_t archiveSize = 0;
    std::string archiveSha256;
    std::string thumbnailKey;
    int fileCount = 0;
    uint64_t totalBytes = 0;
    int sequenceCount = 0;
    bool fseqIncluded = true;
};
BackupSummary ParseManifest(const std::string& json);

struct ShowRestoreResult {
    bool ok = false;
    bool cancelled = false;
    std::string error;
    std::vector<std::string> warnings;
    int filesWritten = 0;
};

// Extracts an archive made by Create() into destDir, which should be empty:
// show/ goes to the root, each .fseq from a separate FSEQ folder next to its
// sequence, and audio from outside the show into "External Media". Entries
// that would land outside destDir are refused. On failure or cancel the files
// written so far are removed.
ShowRestoreResult Extract(const std::filesystem::path& zip, const std::filesystem::path& destDir,
                          const ShowArchiveProgress& progress = nullptr);

// "2026-11-06T14:30:00Z" and "2026-11-06-143000" (UTC).
std::string FormatUtcIso(time_t t);
std::string FormatUtcForKey(time_t t);

} // namespace ShowArchiver
