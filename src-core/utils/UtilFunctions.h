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

#include "globals.h"
#include "string_utils.h"

#include <chrono>
#include <format>
#include <string>
#include <algorithm>
#include <map>
#include <limits>
#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <list>
#include <vector>

#define AMPS_PER_PIXEL (0.055f)

inline std::string FormatTime(uint32_t ms) {
    return std::format("{}:{:02}.{:03}", ms / 60000, (ms % 60000) / 1000, ms % 1000);
}
inline std::string FormatTimeHMS(uint32_t ms) {
    return std::format("{:02}:{:02}:{:02}.{:03}", ms / 3600000, (ms % 3600000) / 60000, (ms % 60000) / 1000, ms % 1000);
}
#define FORMATTIME(ms) FormatTime(static_cast<uint32_t>(ms)).c_str()
#define FORMATTIME_H_M_S(ms) FormatTimeHMS(static_cast<uint32_t>(ms)).c_str()

// Monotonic millisecond timestamp for timing comparisons (replaces wxGetUTCTimeMillis)
inline int64_t GetCurrentTimeMillis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

#define INTROUNDUPDIV(a, b) (((a) + (b) - 1) / (b))
#define UNUSED(x) (void)(x)
constexpr double PI = 3.141592653589793238463;

namespace pugi {
    class xml_node;
}

// XML utility: find (or optionally create) a child node matching tag/attr/value
pugi::xml_node FindXmlNode(pugi::xml_node parent, const std::string& tag, const std::string& attr, const std::string& value, bool create = false);

// Consolidated set of wx-free utility functions
std::string Ordinal(int i);
std::string DecodeMidi(int midi);

bool IsEmailValid(const std::string& email);
bool IsVersionOlder(const std::string &compare, const std::string &version);
std::string JSONSafe(const std::string& s);
std::string UnXmlSafe(const std::string &s);
std::string XmlSafe(const std::string& s);
bool IsXmlSafe(const std::string& s);
std::string RemoveUnsafeXmlChars(const std::string& s);
std::string EscapeCSV(const std::string& s);
std::string EscapeRegex(const std::string& s);
inline bool isOdd(int n) { return n % 2 != 0; }
bool DoesXmlNodeExist(const pugi::xml_node &parent, const std::string& path);
std::string GetXmlNodeContent(const pugi::xml_node &parent, const std::string& path, const std::string& def = "");
std::string GetXmlNodeAttribute(const pugi::xml_node &parent, const std::string& path, const std::string& attribute, const std::string& def = "");
std::vector<std::string> GetXmlNodeListContent(const pugi::xml_node &parent, const std::string& path, const std::string& listNodeName);
void SetXmlNodeAttribute(pugi::xml_node &node, const std::string& property, const std::string& value);
int GetxFadePort(int xfp);

inline long roundTo4(long i) {
    long remainder = i % 4;
    if (remainder == 0) {
        return i;
    }
    return i + 4 - remainder;
}
inline int roundTo16(int i) {
    int remainder = i % 16;
    if (remainder == 0) {
        return i;
    }
    return i + 16 - remainder;
}

inline double rand01()
{
    return (double)rand() / (double)RAND_MAX;
}

// checks if two floating point numbers are equal
template<class Ty>
    requires std::is_floating_point_v<Ty>
[[nodiscard]] constexpr bool
fp_equal(Ty a, Ty b, const Ty eps = std::numeric_limits<Ty>::epsilon()) {
    return std::abs(a - b) <= (std::min(abs(a), std::abs(b)) * eps);
}

int intRand(const int& min, const int& max);
int ExtractInt(std::string& s);
int ExtractTrailingInt(const std::string& s);
int NumberAwareStringCompare(const std::string &a, const std::string &b);
inline int NumberAwareStringCompareRev(const std::string &a, const std::string &b) { return NumberAwareStringCompare(b, a); }
inline bool stdlistNumberAwareStringCompare(const std::string& a, const std::string& b) { return NumberAwareStringCompare(a, b) == -1; }

static inline double toRadians(float degrees)
{
    return 2.0 * M_PI * double(degrees) / 360.0;
}

static inline double toDegrees(double radians) {
    return (radians / (2 * M_PI)) * 360.0;
}

inline const std::string& GetPathSeparator() {
    static const std::string sep(1, std::filesystem::path::preferred_separator);
    return sep;
}

bool IsExcessiveMemoryUsage(double physicalMultiplier = 0.95);

void CheckMemoryUsage(const std::string& reason, bool onchangeOnly = false);
uint64_t GetPhysicalMemorySizeMB();

bool IsxLights();
void SetIsxLights(bool val);
std::string ReverseCSV(const std::string& csv);
void DumpBinary(uint8_t* buffer, size_t read);
bool IsFloat(const std::string& number);
int GetExifOrientation(const std::string& filename);
