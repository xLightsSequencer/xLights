#include "XLightsSettings.h"

#include <fstream>
#include <log.h>

XLightsSettings& XLightsSettings::Get()
{
    static XLightsSettings inst;
    return inst;
}

void XLightsSettings::EnsureSections()
{
    if (!_data.is_object())
        _data = nlohmann::json::object();
    for (const auto& sec : {"main", "tod", "bookmarks", "_meta"}) {
        if (!_data.contains(sec))
            _data[sec] = nlohmann::json::object();
    }
}

void XLightsSettings::SetFilePath(const std::filesystem::path& path)
{
    std::unique_lock lock(_mutex);
    _filePath = path;
}

void XLightsSettings::Load()
{
    std::unique_lock lock(_mutex);
    if (_filePath.empty()) {
        EnsureSections();
        return;
    }

    std::error_code ec;
    if (!std::filesystem::exists(_filePath, ec) || ec) {
        EnsureSections();
        return;
    }

    std::ifstream f(_filePath);
    if (!f.is_open()) {
        spdlog::warn("XLightsSettings: unable to open '{}'", _filePath.string());
        EnsureSections();
        return;
    }
    try {
        _data = nlohmann::json::parse(f);
    } catch (const std::exception& e) {
        spdlog::error("XLightsSettings: parse error '{}' — starting fresh", e.what());
        _data = nlohmann::json::object();
    }
    EnsureSections();
    _dirty = false;
}

void XLightsSettings::Flush()
{
    std::unique_lock lock(_mutex);
    if (_filePath.empty() || !_dirty)
        return;

    // Write atomically via temp file + rename
    auto tmp = _filePath;
    tmp += ".tmp";
    std::ofstream f(tmp);
    if (!f.is_open()) {
        spdlog::error("XLightsSettings: cannot write '{}'", tmp.string());
        return;
    }
    f << _data.dump(2);
    f.close();

    std::error_code ec;
    std::filesystem::rename(tmp, _filePath, ec);
    if (ec) {
        spdlog::error("XLightsSettings: rename failed: {}", ec.message());
    } else {
        _dirty = false;
    }
}

void XLightsSettings::DeleteAll(const std::string& section)
{
    std::unique_lock lock(_mutex);
    _data[section] = nlohmann::json::object();
    _dirty = true;
}

void XLightsSettings::DeleteEntry(const std::string& key, const std::string& section)
{
    std::unique_lock lock(_mutex);
    if (_data.contains(section) && _data[section].contains(key)) {
        _data[section].erase(key);
        _dirty = true;
    }
}

bool XLightsSettings::HasEntry(const std::string& key, const std::string& section) const
{
    std::shared_lock lock(_mutex);
    return _data.contains(section) && _data.at(section).contains(key);
}

std::string XLightsSettings::ReadString(const std::string& key, const std::string& def,
                                         const std::string& section) const
{
    std::shared_lock lock(_mutex);
    if (!_data.contains(section)) return def;
    const auto& sec = _data.at(section);
    if (!sec.contains(key)) return def;
    const auto& v = sec.at(key);
    if (v.is_string()) return v.get<std::string>();
    if (v.is_boolean()) return v.get<bool>() ? "1" : "0";
    if (v.is_number_integer()) return std::to_string(v.get<long long>());
    if (v.is_number_float()) return std::to_string(v.get<double>());
    return def;
}

bool XLightsSettings::ReadBool(const std::string& key, bool def,
                                const std::string& section) const
{
    std::shared_lock lock(_mutex);
    if (!_data.contains(section)) return def;
    const auto& sec = _data.at(section);
    if (!sec.contains(key)) return def;
    const auto& v = sec.at(key);
    if (v.is_boolean()) return v.get<bool>();
    if (v.is_number_integer()) return v.get<long long>() != 0;
    if (v.is_string()) {
        const auto s = v.get<std::string>();
        return (s == "1" || s == "true" || s == "True" || s == "yes");
    }
    return def;
}

long XLightsSettings::ReadLong(const std::string& key, long def,
                                const std::string& section) const
{
    std::shared_lock lock(_mutex);
    if (!_data.contains(section)) return def;
    const auto& sec = _data.at(section);
    if (!sec.contains(key)) return def;
    const auto& v = sec.at(key);
    if (v.is_number_integer()) return static_cast<long>(v.get<long long>());
    if (v.is_boolean()) return v.get<bool>() ? 1L : 0L;
    if (v.is_string()) {
        const std::string s = v.get<std::string>();
        char* end = nullptr;
        long result = std::strtol(s.c_str(), &end, 10);
        return (end != s.c_str()) ? result : def;
    }
    return def;
}

double XLightsSettings::ReadDouble(const std::string& key, double def,
                                    const std::string& section) const
{
    std::shared_lock lock(_mutex);
    if (!_data.contains(section)) return def;
    const auto& sec = _data.at(section);
    if (!sec.contains(key)) return def;
    const auto& v = sec.at(key);
    if (v.is_number()) return v.get<double>();
    if (v.is_string()) {
        const std::string s = v.get<std::string>();
        char* end = nullptr;
        double result = std::strtod(s.c_str(), &end);
        return (end != s.c_str()) ? result : def;
    }
    return def;
}

void XLightsSettings::WriteString(const std::string& key, const std::string& val,
                                   const std::string& section)
{
    std::unique_lock lock(_mutex);
    _data[section][key] = val;
    _dirty = true;
}

void XLightsSettings::WriteBool(const std::string& key, bool val,
                                 const std::string& section)
{
    std::unique_lock lock(_mutex);
    _data[section][key] = val;
    _dirty = true;
}

void XLightsSettings::WriteLong(const std::string& key, long val,
                                 const std::string& section)
{
    std::unique_lock lock(_mutex);
    _data[section][key] = static_cast<long long>(val);
    _dirty = true;
}

void XLightsSettings::WriteDouble(const std::string& key, double val,
                                   const std::string& section)
{
    std::unique_lock lock(_mutex);
    _data[section][key] = val;
    _dirty = true;
}

std::vector<std::string> XLightsSettings::GetAllKeys(const std::string& section) const
{
    std::shared_lock lock(_mutex);
    std::vector<std::string> keys;
    if (!_data.contains(section)) return keys;
    for (const auto& [k, _v] : _data.at(section).items())
        keys.push_back(k);
    return keys;
}

XLightsSettings::ValueType XLightsSettings::GetValueType(const std::string& key,
                                                           const std::string& section) const
{
    std::shared_lock lock(_mutex);
    if (!_data.contains(section)) return ValueType::Unknown;
    const auto& sec = _data.at(section);
    if (!sec.contains(key)) return ValueType::Unknown;
    const auto& v = sec.at(key);
    if (v.is_boolean()) return ValueType::Boolean;
    if (v.is_number_integer()) return ValueType::Integer;
    if (v.is_number_float()) return ValueType::Float;
    if (v.is_string()) return ValueType::String;
    return ValueType::Unknown;
}

bool XLightsSettings::WasMigrated() const
{
    std::shared_lock lock(_mutex);
    if (!_data.contains("_meta")) return false;
    const auto& m = _data.at("_meta");
    if (!m.contains("migrated")) return false;
    const auto& v = m.at("migrated");
    return v.is_boolean() && v.get<bool>();
}

void XLightsSettings::SetMigrated()
{
    std::unique_lock lock(_mutex);
    _data["_meta"]["migrated"] = true;
    _dirty = true;
}
