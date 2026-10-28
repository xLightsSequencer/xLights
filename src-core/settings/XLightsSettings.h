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

#include <filesystem>
#include <map>
#include <set>
#include <shared_mutex>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

/// wx-free persistent settings store backed by a JSON file in AppData.
/// Three logical namespaces (sections): "main", "tod", "bookmarks".
/// Thread-safe for concurrent reads; writes are serialised internally.
class XLightsSettings {
public:
    static XLightsSettings& Get();

    // --- Lifecycle -------------------------------------------------------

    /// Set the path of the JSON settings file (call before Load).
    void SetFilePath(const std::filesystem::path& path);

    /// Load settings from disk. No-op if path not set or file not found.
    void Load();

    /// Write this process's changed keys to disk if dirty (atomic rename).
    /// Keys changed by other running instances since our last load are kept.
    void Flush();

    /// Pick up changes other running instances wrote to disk, keeping any
    /// of our own changes that have not been flushed yet.
    void Reload();

    // --- Section-level operations ----------------------------------------

    /// Remove all keys in the given section.
    void DeleteAll(const std::string& section = "main");

    /// Remove a single key from the given section.
    void DeleteEntry(const std::string& key, const std::string& section = "main");

    /// Return true if the key exists in the section.
    bool HasEntry(const std::string& key, const std::string& section = "main") const;

    // --- Read ------------------------------------------------------------

    std::string ReadString(const std::string& key, const std::string& def = "",
                           const std::string& section = "main") const;
    bool        ReadBool  (const std::string& key, bool   def = false,
                           const std::string& section = "main") const;
    long        ReadLong  (const std::string& key, long   def = 0,
                           const std::string& section = "main") const;
    double      ReadDouble(const std::string& key, double def = 0.0,
                           const std::string& section = "main") const;

    // --- Write -----------------------------------------------------------

    void WriteString(const std::string& key, const std::string& val,
                     const std::string& section = "main");
    void WriteBool  (const std::string& key, bool   val,
                     const std::string& section = "main");
    void WriteLong  (const std::string& key, long   val,
                     const std::string& section = "main");
    void WriteDouble(const std::string& key, double val,
                     const std::string& section = "main");

    // --- Iteration -------------------------------------------------------

    /// Return all keys present in the given section.
    std::vector<std::string> GetAllKeys(const std::string& section = "main") const;

    // --- Value type (for debug dump) ------------------------------------

    enum class ValueType { Unknown, String, Boolean, Integer, Float };
    ValueType GetValueType(const std::string& key,
                           const std::string& section = "main") const;

    // --- Migration marker -----------------------------------------------

    bool WasMigrated() const;
    void SetMigrated();

private:
    XLightsSettings() = default;
    XLightsSettings(const XLightsSettings&) = delete;
    XLightsSettings& operator=(const XLightsSettings&) = delete;

    mutable std::shared_mutex _mutex;
    nlohmann::json _data;
    std::filesystem::path _filePath;
    bool _dirty { false };
    std::map<std::string, std::set<std::string>> _changedKeys;
    std::set<std::string> _clearedSections;

    void EnsureSections();
    void MarkChanged(const std::string& section, const std::string& key);
    void MergeFromDisk();
};
