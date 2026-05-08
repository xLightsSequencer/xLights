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

#include <string>
#include <vector>
#include <filesystem>

#if __has_include(<wx/string.h>)
#include <wx/string.h>
#endif

/// Drop-in replacement for wxConfigBase* that delegates to XLightsSettings.
/// All call sites replace  wxConfigBase* config = wxConfigBase::Get();
/// with                    auto* config = GetXLightsConfig();
class XLightsConfigAdapter {
public:
    explicit XLightsConfigAdapter(std::string section = "main");

    // --- Read helpers (output-parameter form, returns true if key existed) ---
#if __has_include(<wx/string.h>)
    bool Read(const std::string& key, wxString* val,
              const wxString& def = wxString()) const;
#endif
    bool Read(const std::string& key, std::string*      val,
              const std::string& def = std::string()) const;
    bool Read(const std::string& key, bool*          val, bool     def = false) const;
    bool Read(const std::string& key, int*           val, int      def = 0)     const;
    bool Read(const std::string& key, long*          val, long     def = 0)     const;
    bool Read(const std::string& key, double*        val, double   def = 0.0)   const;
    bool Read(const std::string& key, size_t*        val, size_t   def = 0)     const;

    // --- Read helpers (return-value form) ---
    // NOTE: bool/double return-value overloads are intentionally omitted.
    // const char* literals would match bool over wxString, breaking all string-default reads.
    // Use ReadBool()/ReadDouble() for typed reads without a pointer.
    std::string Read(const std::string& key, const std::string& def = std::string()) const;
    long     Read      (const std::string& key, long   def) const;
    bool     ReadBool  (const std::string& key, bool   def) const;
    long     ReadLong  (const std::string& key, long   def) const;
    double   ReadDouble(const std::string& key, double def) const;

    // --- Write helpers --------------------------------------------------
    void Write(const std::string& key, const std::string& val);
    void Write(const std::string& key, const char* val) { Write(key, std::string(val ? val : "")); }
#if __has_include(<wx/string.h>)
    void Write(const std::string& key, const wxString&    val);
#endif
    void Write(const std::string& key, bool         val);
    void Write(const std::string& key, int          val);
    void Write(const std::string& key, unsigned int val);
    void Write(const std::string& key, long         val);
    void Write(const std::string& key, size_t       val);
    void Write(const std::string& key, double       val);

    // --- Lifecycle ------------------------------------------------------
    void DeleteEntry(const std::string& key);
    void DeleteAll();
    void Flush();
    void SetPath(const std::string& path); // no-op: JSON store is flat

    // --- Iteration (used for config dump in debug reports) --------------
    enum class EntryType { Unknown, String, Boolean, Integer, Float };
    bool GetFirstEntry(std::string& key, long& index) const;
    bool GetNextEntry(std::string& key, long& index) const;
    EntryType GetEntryType(const std::string& key) const;

    // --- Informational (mirroring wxConfigBase debug-log calls) --------
    std::string GetAppName() const;
    std::string GetPath() const;
    int      GetNumberOfEntries(bool recurse = false) const;
    int      GetNumberOfGroups (bool recurse = false) const;
    long     GetStyle()              const;
    std::string GetVendorName() const;

private:
    std::string _section;
    mutable std::vector<std::string> _iterKeys;  // cached key list for iteration
};

// ---------------------------------------------------------------------------
// Global accessor functions — replace wxConfigBase::Get()
// ---------------------------------------------------------------------------
XLightsConfigAdapter* GetXLightsConfig();
XLightsConfigAdapter* GetXLightsTODConfig();
XLightsConfigAdapter* GetXLightsBookmarksConfig();

/// Call once early in xLightsApp::OnInit (before xLightsFrame is created).
/// Determines AppData path, loads JSON, and imports legacy wxConfig on first run.
void InitializeXLightsConfig();

/// Returns the path to the xLights log file.
std::filesystem::path GetLogFilePath();
std::filesystem::path GetLogFileFolder();
std::string GetLogFileName();

/// Wipe all settings to factory defaults (called from the -w command-line flag).
void WipeXLightsConfig();
