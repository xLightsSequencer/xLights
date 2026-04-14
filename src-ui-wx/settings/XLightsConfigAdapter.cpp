#include "XLightsConfigAdapter.h"

#include "settings/XLightsSettings.h"
#if __has_include(<wx/config.h>)
#include <wx/config.h>
#endif
#include <filesystem>
#include <log.h>

// ---------------------------------------------------------------------------
// Module-level singleton adapters (one per logical section)
// ---------------------------------------------------------------------------
static XLightsConfigAdapter s_mainConfig("main");
static XLightsConfigAdapter s_todConfig("tod");
static XLightsConfigAdapter s_bookmarksConfig("bookmarks");

XLightsConfigAdapter* GetXLightsConfig()           { return &s_mainConfig;      }
XLightsConfigAdapter* GetXLightsTODConfig()        { return &s_todConfig;       }
XLightsConfigAdapter* GetXLightsBookmarksConfig()  { return &s_bookmarksConfig; }

// ---------------------------------------------------------------------------
// Platform-specific AppData path
// ---------------------------------------------------------------------------
static std::filesystem::path GetSettingsFilePath()
{
    std::filesystem::path dir;

#if defined(_WIN32) || defined(__WXMSW__)
    const char* appData = std::getenv("APPDATA");
    dir = std::filesystem::path(appData && *appData ? appData : ".") / "xLights";
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    dir = std::filesystem::path(home && *home ? home : ".") / "Library" / "Application Support" / "xLights";
#else
    const char* xdgConfig = std::getenv("XDG_CONFIG_HOME");
    if (xdgConfig && *xdgConfig) {
        dir = std::filesystem::path(xdgConfig) / "xLights";
    } else {
        const char* home = std::getenv("HOME");
        dir = std::filesystem::path(home && *home ? home : ".") / ".config" / "xLights";
    }
#endif

    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir / "settings.json";
}

// ---------------------------------------------------------------------------
// One-time import from legacy wxConfig (Registry / plist / .conf)
// ---------------------------------------------------------------------------
static void ImportWxConfigSection(const std::string& wxAppName,
                                   const std::string& targetSection) {
#if __has_include(<wx/config.h>)
    wxConfig cfg(wxAppName);
    wxString key;
    long index = 0;
    bool more = cfg.GetFirstEntry(key, index);
    while (more) {
        const std::string k = key.ToStdString();
        switch (cfg.GetEntryType(key)) {
        case wxConfigBase::EntryType::Type_String: {
            wxString v;
            cfg.Read(key, &v, "");
            XLightsSettings::Get().WriteString(k, v.ToStdString(), targetSection);
            break;
        }
        case wxConfigBase::EntryType::Type_Boolean:
            XLightsSettings::Get().WriteBool(k, cfg.ReadBool(key, false), targetSection);
            break;
        case wxConfigBase::EntryType::Type_Integer:
            XLightsSettings::Get().WriteLong(k, cfg.ReadLong(key, 0), targetSection);
            break;
        case wxConfigBase::EntryType::Type_Float:
            XLightsSettings::Get().WriteDouble(k, cfg.ReadDouble(key, 0.0), targetSection);
            break;
        default:
            break;
        }
        more = cfg.GetNextEntry(key, index);
    }
#endif
}

// ---------------------------------------------------------------------------
// Public lifecycle functions
// ---------------------------------------------------------------------------
void InitializeXLightsConfig()
{
    const auto path = GetSettingsFilePath();
    spdlog::info("XLightsSettings: settings file '{}'", path.string());

    auto& settings = XLightsSettings::Get();
    settings.SetFilePath(path);

    std::error_code ec;
    const bool fileExisted = std::filesystem::exists(path, ec) && !ec;
    settings.Load();

    // One-time migration from wxConfig when the JSON file did not yet exist
    if (!fileExisted && !settings.WasMigrated()) {
        spdlog::info("XLightsSettings: importing legacy settings from system config");
        try {
            ImportWxConfigSection("xLights",           "main");
            ImportWxConfigSection("xLights-TOD",       "tod");
#ifdef __WXOSX__
            ImportWxConfigSection("xLights-Bookmarks", "bookmarks");
#endif
        } catch (...) {
            spdlog::warn("XLightsSettings: exception during legacy import (ignored)");
        }
        settings.SetMigrated();
        settings.Flush();
        spdlog::info("XLightsSettings: legacy import complete");
    }
}

void WipeXLightsConfig()
{
    auto& s = XLightsSettings::Get();
    s.DeleteAll("main");
    s.DeleteAll("tod");
    s.DeleteAll("bookmarks");
    s.Flush();
}

// ---------------------------------------------------------------------------
// XLightsConfigAdapter implementation
// ---------------------------------------------------------------------------
XLightsConfigAdapter::XLightsConfigAdapter(std::string section)
    : _section(std::move(section))
{}

bool XLightsConfigAdapter::Read(const std::string& key, wxString* val,
                                const wxString& def) const {
    const bool exists = XLightsSettings::Get().HasEntry(key, _section);
    *val = wxString(XLightsSettings::Get().ReadString(key, def.ToStdString(), _section));
    return exists;
}
bool XLightsConfigAdapter::Read(const std::string& key, std::string* val,
                                const std::string& def) const {
    const bool exists = XLightsSettings::Get().HasEntry(key, _section);
    *val = XLightsSettings::Get().ReadString(key, def, _section);
    return exists;
}

bool XLightsConfigAdapter::Read(const std::string& key, bool* val, bool def) const
{
    const bool exists = XLightsSettings::Get().HasEntry(key, _section);
    *val = XLightsSettings::Get().ReadBool(key, def, _section);
    return exists;
}

bool XLightsConfigAdapter::Read(const std::string& key, int* val, int def) const
{
    const bool exists = XLightsSettings::Get().HasEntry(key, _section);
    *val = static_cast<int>(
        XLightsSettings::Get().ReadLong(key, static_cast<long>(def), _section));
    return exists;
}

bool XLightsConfigAdapter::Read(const std::string& key, long* val, long def) const
{
    const bool exists = XLightsSettings::Get().HasEntry(key, _section);
    *val = XLightsSettings::Get().ReadLong(key, def, _section);
    return exists;
}

bool XLightsConfigAdapter::Read(const std::string& key, double* val, double def) const {
    const bool exists = XLightsSettings::Get().HasEntry(key, _section);
    *val = XLightsSettings::Get().ReadDouble(key, def, _section);
    return exists;
}

bool XLightsConfigAdapter::Read(const std::string& key, size_t* val, size_t def) const {
    const bool exists = XLightsSettings::Get().HasEntry(key, _section);
    *val = static_cast<size_t>(
        XLightsSettings::Get().ReadLong(key, static_cast<long>(def), _section));
    return exists;
}

std::string XLightsConfigAdapter::Read(const std::string& key, const std::string& def) const {
    return XLightsSettings::Get()
                        .ReadString(key, def, _section);
}

long XLightsConfigAdapter::Read(const std::string& key, long def) const
{
    return XLightsSettings::Get().ReadLong(key, def, _section);
}

bool XLightsConfigAdapter::ReadBool(const std::string& key, bool def) const {
    return XLightsSettings::Get().ReadBool(key, def, _section);
}

long XLightsConfigAdapter::ReadLong(const std::string& key, long def) const {
    return XLightsSettings::Get().ReadLong(key, def, _section);
}

double XLightsConfigAdapter::ReadDouble(const std::string& key, double def) const {
    return XLightsSettings::Get().ReadDouble(key, def, _section);
}

void XLightsConfigAdapter::Write(const std::string& key, const std::string& val) {
    XLightsSettings::Get().WriteString(key, val, _section);
}

void XLightsConfigAdapter::Write(const std::string& key, const wxString& val) {
    XLightsSettings::Get().WriteString(key, val.ToStdString(), _section);
}

void XLightsConfigAdapter::Write(const std::string& key, bool val) {
    XLightsSettings::Get().WriteBool(key, val, _section);
}

void XLightsConfigAdapter::Write(const std::string& key, int val) {
    XLightsSettings::Get().WriteLong(key, static_cast<long>(val), _section);
}

void XLightsConfigAdapter::Write(const std::string& key, unsigned int val) {
    XLightsSettings::Get().WriteLong(key, static_cast<long>(val), _section);
}

void XLightsConfigAdapter::Write(const std::string& key, long val) {
    XLightsSettings::Get().WriteLong(key, val, _section);
}

void XLightsConfigAdapter::Write(const std::string& key, size_t val) {
    XLightsSettings::Get().WriteLong(key, static_cast<long>(val), _section);
}

void XLightsConfigAdapter::Write(const std::string& key, double val) {
    XLightsSettings::Get().WriteDouble(key, val, _section);
}

void XLightsConfigAdapter::DeleteEntry(const std::string& key) {
    XLightsSettings::Get().DeleteEntry(key, _section);
}

void XLightsConfigAdapter::DeleteAll()
{
    XLightsSettings::Get().DeleteAll(_section);
}

void XLightsConfigAdapter::Flush()
{
    XLightsSettings::Get().Flush();
}

void XLightsConfigAdapter::SetPath(const std::string& /*path*/) {
    // JSON store is flat — path navigation is a no-op
}

bool XLightsConfigAdapter::GetFirstEntry(std::string& key, long& index) const {
    _iterKeys = XLightsSettings::Get().GetAllKeys(_section);
    index = 0;
    if (_iterKeys.empty()) {
        return false;
    }
    key = _iterKeys[0];
    return true;
}

bool XLightsConfigAdapter::GetNextEntry(std::string& key, long& index) const {
    ++index;
    if (index >= static_cast<long>(_iterKeys.size())) {
        return false;
    }
    key = _iterKeys[static_cast<size_t>(index)];
    return true;
}

XLightsConfigAdapter::EntryType XLightsConfigAdapter::GetEntryType(const std::string& key) const {
    using VT = XLightsSettings::ValueType;
    switch (XLightsSettings::Get().GetValueType(key, _section)) {
    case VT::Boolean: return EntryType::Boolean;
    case VT::Integer: return EntryType::Integer;
    case VT::Float:   return EntryType::Float;
    case VT::String:  return EntryType::String;
    default:          return EntryType::Unknown;
    }
}

std::string XLightsConfigAdapter::GetAppName() const { return "xLights"; }
std::string XLightsConfigAdapter::GetPath() const { return "/"; }
long     XLightsConfigAdapter::GetStyle()      const { return 0; }
std::string XLightsConfigAdapter::GetVendorName() const { return "xLightsSequencer"; }

int XLightsConfigAdapter::GetNumberOfEntries(bool) const
{
    return static_cast<int>(XLightsSettings::Get().GetAllKeys(_section).size());
}

int XLightsConfigAdapter::GetNumberOfGroups(bool) const
{
    return 0;
}
