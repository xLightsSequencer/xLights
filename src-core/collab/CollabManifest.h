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

// The show files a live session needs and how a joiner's show folder
// compares with the host's.
//
// The host lists, with size and content hash:
//   - xlights_rgbeffects.xml and xlights_networks.xml (role Layout);
//   - the sequence's audio file and alternate tracks (role Audio);
//   - every file the sequence refers to and doesn't embed: SequenceMedia's
//     entries, each effect's GetFileReferences (images, videos, shaders,
//     glediator / text / SVG files, Matrix face images of the model) and
//     the file references of the models the sequence has rows for (image
//     models) (role Media).
// Embedded media isn't listed: it travels inside the .xsq snapshot. System
// fonts that Text effects name are not show files and aren't listed.
//
// A path is how the host refers to a file: relative to its show folder
// (with '/') when the file is in it, else the reference as the document
// stores it. Each peer resolves it against its own show folder the way a
// render would (the show folder first, then FileUtils::FixFile), so the
// comparison checks exactly the files the joiner would render with.

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "CollabMessages.h"

class SequenceElements;

namespace Collab {

// 64-bit content hash (the same on every platform), streamed.
class ContentHash {
public:
    ContentHash();
    void Update(const void* data, size_t len);
    uint64_t Final() const;

private:
    uint64_t _v[4];
    unsigned char _buf[32];
    size_t _bufLen = 0;
    uint64_t _total = 0;
};

uint64_t HashBytes(const std::string& bytes);
// False when the file can't be read.
bool HashFile(const std::string& path, uint64_t& size, uint64_t& hash);

// Remembers hashes by (path, size, modification time) so a host doesn't
// re-read every video for each joiner.
class HashCache {
public:
    bool Hash(const std::string& path, uint64_t& size, uint64_t& hash);
    size_t FilesRead() const { return _filesRead; }
    // The bytes Hash would read for `path` now: 0 when its size and mtime
    // match what is cached.
    uint64_t BytesToRead(const std::string& path) const;
    // A copy to hash with on another thread (counting its own reads), and
    // taking what it learned back.
    HashCache Fork() const;
    void Merge(const HashCache& other);

private:
    struct Item {
        uint64_t size = 0;
        int64_t mtime = 0;
        uint64_t hash = 0;
    };
    std::map<std::string, Item> _items;
    size_t _filesRead = 0;
};

inline constexpr const char* kLayoutFile = "xlights_rgbeffects.xml";
inline constexpr const char* kNetworksFile = "xlights_networks.xml";

// A file the session needs, with where it is on this machine.
struct SessionFile {
    std::string path;
    FileRole role = FileRole::Media;
    std::string local; // absolute path on this machine
};

// The manifest path for `resolved` (what `reference` resolved to).
std::string ManifestPathFor(const std::string& showDir, const std::string& reference, const std::string& resolved);

// Where `path` lives inside a session copy of a show folder, relative to
// it: the path itself when it is a plain relative path, else its last
// folder and file name (where FixFile looks for a file that has moved).
// Never absolute and never climbs out (no ".."). Empty when unusable.
std::string CachePathFor(const std::string& path);

// The file `path` names on this machine, resolved against `showDir`; empty
// when there is none.
std::string ResolvePath(const std::string& showDir, const std::string& path);

// The files of the open sequence (see the top of this file), resolved on
// this (the host's) machine. Files the host itself doesn't have are left
// out. `audio` is the sequence's audio file and alternate tracks.
std::vector<SessionFile> CollectSessionFiles(const std::string& showDir, SequenceElements& seq, const std::vector<std::string>& audio);

// Hashes the files. Unreadable ones are left out. `files` gets the manifest
// path -> local path of each listed file.
std::vector<FileEntry> BuildManifest(const std::vector<SessionFile>& files, HashCache* cache = nullptr,
                                     std::map<std::string, std::string>* local = nullptr);

enum class FileState { Match, Missing, Different };

struct FileCheck {
    FileEntry entry;
    FileState state = FileState::Match;
    std::string local; // the file this machine has for it (Different / Match)
};

// How `showDir` compares with the host's manifest.
std::vector<FileCheck> CompareManifest(const std::string& showDir, const std::vector<FileEntry>& manifest, HashCache* cache = nullptr);
// CompareManifest in two steps: finding each file on this machine (the main
// thread: it uses the app's search folders), then hashing them (any thread).
std::vector<FileCheck> ResolveManifest(const std::string& showDir, const std::vector<FileEntry>& manifest);
void HashChecks(std::vector<FileCheck>& checks, HashCache* cache);
// Tests: called with each file HashFile reads, on whatever thread reads it.
void SetHashFileHook(std::function<void(const std::string&)> hook);

// A readable account of how two versions of a layout file differ, record by
// record, by the shared-show-folder merge's rules (layout ids, attribute
// order, whitespace and number formatting are not differences).
struct LayoutDifference {
    bool readable = true; // false: one version didn't parse
    size_t changes = 0;   // records changed, added or removed
    std::string text;     // "3 models changed (Arch, Star, Tree); 1 group only on the host (All)"
};
// `file` is kLayoutFile or kNetworksFile.
LayoutDifference DescribeLayoutDifference(const std::string& file, const std::string& mine, const std::string& host);

// Copies (or moves) a file, creating the destination's folders and
// replacing what is there.
bool CopyFileTo(const std::string& from, const std::string& to);
bool MoveFileTo(const std::string& from, const std::string& to);

} // namespace Collab
