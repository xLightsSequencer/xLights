/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <random>
#include <thread>
#include <time.h>
#include <filesystem>
#include <regex>
#include <cstring>
#include <fstream>

#include <pugixml.hpp>

#include "UtilFunctions.h"
#include "utils/ExternalHooks.h"
#include "string_utils.h"

#include <string_view>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

#ifdef __linux__
#include <sys/sysinfo.h>
#endif

#include <log.h>

#if defined(_MSC_VER) // Visual studio
#define thread_local __declspec(thread)
#elif defined(__GCC__) // GCC
#define thread_local __thread
#endif

pugi::xml_node FindXmlNode(pugi::xml_node parent, const std::string& tag, const std::string& attr, const std::string& value, bool create)
{
    for (pugi::xml_node node = parent.first_child(); node; node = node.next_sibling()) {
        if (!tag.empty() && (node.name() != tag))
            continue;
        if (!value.empty() && (node.attribute(attr.c_str()).as_string() != value))
            continue;
        return node;
    }
    if (!create)
        return pugi::xml_node();
    pugi::xml_node retnode = parent.append_child(tag.c_str());
    if (!value.empty()) {
        retnode.remove_attribute(attr.c_str());
        retnode.append_attribute(attr.c_str()) = value.c_str();
    }
    return retnode;
}

std::string DecodeMidi(int midi) {
    int n = midi % 12;
    int o = midi / 12 - 1;
    // dont go below zero ... if so move it up an octave ... the user will never know
    while (o < 0) {
        o++;
    }

    bool sharp = false;
    char note = '?';
    switch (n) {
    case 9:
        note = 'A';
        break;
    case 10:
        note = 'A';
        sharp = true;
        break;
    case 11:
        note = 'B';
        break;
    case 0:
        note = 'C';
        break;
    case 1:
        note = 'C';
        sharp = true;
        break;
    case 2:
        note = 'D';
        break;
    case 3:
        note = 'D';
        sharp = true;
        break;
    case 4:
        note = 'E';
        break;
    case 5:
        note = 'F';
        break;
    case 6:
        note = 'F';
        sharp = true;
        break;
    case 7:
        note = 'G';
        break;
    case 8:
        note = 'G';
        sharp = true;
        break;
    default:
        break;
    }

    char buf[16];
    if (sharp) {
        snprintf(buf, sizeof(buf), "%c#%d", note, o);
    } else {
        snprintf(buf, sizeof(buf), "%c%d", note, o);
    }
    return std::string(buf);
}

std::string UnXmlSafe(const std::string& res) {
    if (Contains(res, "&")) {
        std::string r2(res);
        for (int i = 0; i < 32; ++i) {
            std::string ss = "&#" + std::to_string(i);
            char buf[2] = { (char)i, 0 };
            Replace(r2, ss, buf);
        }
        Replace(r2, "&lt;", "<");
        Replace(r2, "&gt;", ">");
        Replace(r2, "&apos;", "'");
        Replace(r2, "&quot;", "\"");
        Replace(r2, "&amp;", "&");
        return r2;
    }
    return res;
}

std::string XmlSafe(const std::string& s) {
    std::string res = "";
    for (auto c = s.begin(); c != s.end(); ++c) {
        if ((int)(*c) < 32) {
            int cc = (int)*c;
            if (cc == 9 || cc == 10 || cc == 13) {
                char buf[8];
                snprintf(buf, sizeof(buf), "&#%d;", cc);
                res += buf;
            }
        } else if (*c == '&') {
            res += "&amp;";
        } else if (*c == '<') {
            res += "&lt;";
        } else if (*c == '>') {
            res += "&gt;";
        } else if (*c == '\'') {
            res += "&apos;";
        } else if (*c == '\"') {
            res += "&quot;";
        } else {
            res += (*c);
        }
    }

    return res;
}

bool IsXmlSafe(const std::string& s) {
    bool res = true;
    for (auto c = s.begin(); c != s.end(); ++c) {
        if ((int)(*c) < 32 || (int)(*c) > 127) {
            res = false;
        } else if (*c == ',') {
            res = false;
        } else if (*c == '\'') {
            res = false;
        }
    }
    return res;
}

int GetExifOrientation(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        spdlog::debug("Failed to open file: {}", filename);
        file.close();
        return 1; // Default orientation
    }

    unsigned char byte1, byte2;
    file.read(reinterpret_cast<char*>(&byte1), 1);
    file.read(reinterpret_cast<char*>(&byte2), 1);
    if (byte1 != 0xFF || byte2 != 0xD8) {
        file.close();
        return 1;
    }

    while (file) {
        file.read(reinterpret_cast<char*>(&byte1), 1);
        if (byte1 != 0xFF) break;
        file.read(reinterpret_cast<char*>(&byte2), 1);
        if (byte2 == 0xD9 || byte2 == 0xDA) break;

        unsigned short len;
        file.read(reinterpret_cast<char*>(&len), 2);
        len = ((len >> 8) & 0xFF) | ((len << 8) & 0xFF00); // big-endian
        if (len < 2) break;

        if (byte2 == 0xE1) { // APP1 segment
            std::vector<char> data(len - 2);
            file.read(data.data(), len - 2);

            if (data.size() < 14) continue; // too small to hold Exif header

            if (memcmp(data.data(), "Exif\0\0", 6) != 0) {
                continue;
            }

            size_t tiff_header = 6; // TIFF header starts right after Exif\0\0
            bool littleEndian = (data[tiff_header] == 'I' && data[tiff_header + 1] == 'I');
            unsigned short fortytwo = littleEndian ?
                ((unsigned char)data[tiff_header + 3] << 8) | (unsigned char)data[tiff_header + 2] :
                ((unsigned char)data[tiff_header + 2] << 8) | (unsigned char)data[tiff_header + 3];

            if (fortytwo != 42) {
                spdlog::debug("Invalid TIFF header identifier in {}", filename);
                return 1;
            }

            // Read offset to IFD0
            unsigned int ifd_offset;
            if (littleEndian) {
                ifd_offset =  (unsigned char)data[tiff_header + 4] |
                    ((unsigned char)data[tiff_header + 5] << 8) |
                    ((unsigned char)data[tiff_header + 6] << 16)|
                    ((unsigned char)data[tiff_header + 7] << 24);
            } else {
                ifd_offset = ((unsigned char)data[tiff_header + 4] << 24)|
                    ((unsigned char)data[tiff_header + 5] << 16)|
                    ((unsigned char)data[tiff_header + 6] << 8) |
                    (unsigned char)data[tiff_header + 7];
            }

            size_t pos = tiff_header + ifd_offset;
            if (pos + 2 > data.size()) return 1;

            unsigned short num_entries = littleEndian ?
                ((unsigned char)data[pos + 1] << 8) | (unsigned char)data[pos] :
                ((unsigned char)data[pos] << 8) | (unsigned char)data[pos + 1];
            pos += 2;

            for (unsigned short i = 0; i < num_entries; ++i) {
                if (pos + 12 > data.size()) break;

                unsigned short tag = littleEndian ?
                    ((unsigned char)data[pos + 1] << 8) | (unsigned char)data[pos] :
                    ((unsigned char)data[pos] << 8) | (unsigned char)data[pos + 1];

                if (tag == 0x0112) { // Orientation
                    unsigned short orient = littleEndian ?
                        ((unsigned char)data[pos + 9] << 8) | (unsigned char)data[pos + 8] :
                        ((unsigned char)data[pos + 8] << 8) | (unsigned char)data[pos + 9];
                    return static_cast<int>(orient);
                }
                pos += 12;
            }
        } else {
            file.seekg(len - 2, std::ios::cur);
        }
    }

    return 1; // default
}

// This takes a string and removes all problematic characters from it for an XML file
std::string RemoveUnsafeXmlChars(const std::string& s) {
    std::string res;

    for (size_t i = 0; i < s.size(); i++) {
        if ((int)s[i] < 32 || s[i] > 127) {
            // strip it
        } else if (s[i] == '=' || s[i] == '<' || s[i] == '>' || s[i] == '&' || s[i] == '"' || s[i] == '\'') {
            // strip them too
        } else
            res += s[i];
    }

    return res;
}

std::string EscapeCSV(const std::string& s) {
    std::string res = "";
    for (auto c : s) {
        if (c == '\"') {
            res += "\"\"";
        } else {
            res += c;
        }
    }

    return res;
}

std::string EscapeRegex(const std::string& s) {
    // \,*,+,?,|,{,[, (,),^,$,.,#,
    constexpr std::string_view BADREX{ R"(\*+?|{[()^$,#)" };
    std::string safe;
    for (auto& c : s) {
        if (BADREX.find(c) != std::string::npos) {
            safe += "\\";
        }
        safe += c;
    }
    return safe;
}


bool DoesXmlNodeExist(const pugi::xml_node &parent, const std::string& path) {
    pugi::xml_node curr = parent;
    std::vector<std::string> pe;
    Split(path, '/', pe);

    for (const auto& it : pe) {
        bool found = false;
        for (pugi::xml_node n = curr.first_child(); n; n = n.next_sibling()) {
            if (std::string_view(n.name()) == it) {
                if (it == pe.back())
                    return true;

                curr = n;
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    return false;
}

std::string GetXmlNodeContent(const pugi::xml_node &parent, const std::string& path, const std::string& def) {
    pugi::xml_node curr = parent;
    std::vector<std::string> pe;
    Split(path, '/', pe);

    for (const auto& it : pe) {
        bool found = false;
        for (pugi::xml_node n = curr.first_child(); n; n = n.next_sibling()) {
            if (std::string_view(n.name()) == it) {
                if (it == pe.back()) {
                    const char* text = n.child_value();
                    return (text && text[0]) ? text : def;
                }
                curr = n;
                found = true;
                break;
            }
        }
        if (!found) return def;
    }
    return def;
}

std::string GetXmlNodeAttribute(const pugi::xml_node &parent, const std::string& path, const std::string& attribute, const std::string& def) {
    pugi::xml_node curr = parent;
    std::vector<std::string> pe;
    Split(path, '/', pe);

    for (const auto& it : pe) {
        bool found = false;
        for (pugi::xml_node n = curr.first_child(); n; n = n.next_sibling()) {
            if (std::string_view(n.name()) == it) {
                if (it == pe.back()) {
                    pugi::xml_attribute a = n.attribute(attribute);
                    return a.empty() ? def : a.as_string();
                }
                curr = n;
                found = true;
                break;
            }
        }
        if (!found) return def;
    }
    return def;
}

std::vector<std::string> GetXmlNodeListContent(const pugi::xml_node &parent, const std::string& path, const std::string& listNodeName) {
    std::vector<std::string> res;
    pugi::xml_node curr = parent;
    std::vector<std::string> pe;
    Split(path, '/', pe);

    for (const auto& it : pe) {
        bool found = false;
        for (pugi::xml_node n = curr.first_child(); n; n = n.next_sibling()) {
            if (std::string_view(n.name()) == it) {
                curr = n;
                found = true;
                break;
            }
        }
        if (!found) return res;
    }

    for (pugi::xml_node n = curr.first_child(); n; n = n.next_sibling()) {
        if (std::string_view(n.name()) == listNodeName) {
            const char* text = n.child_value();
            if (text && text[0]) {
                res.push_back(text);
            }
        }
    }
    return res;
}


void SetXmlNodeAttribute(pugi::xml_node &node, const std::string& property, const std::string& value) {
    auto attr = node.attribute(property);
    if (!attr) {
        attr = node.append_attribute(property);
    }
    attr.set_value(value);
}

bool IsVersionOlder(const std::string& compare, const std::string& version) {
    std::vector<std::string> compare_parts = Split(compare, '.');
    std::vector<std::string> version_parts = Split(version, '.');

    auto safeInt = [](const std::string& s) -> int {
        return (int)std::strtol(s.c_str(), nullptr, 10);
    };

    if (safeInt(version_parts[0]) < safeInt(compare_parts[0]))
        return true;
    if (safeInt(version_parts[0]) > safeInt(compare_parts[0]))
        return false;
    if (safeInt(version_parts[1]) < safeInt(compare_parts[1]))
        return true;
    if (safeInt(version_parts[1]) > safeInt(compare_parts[1]))
        return false;
    if (version_parts.size() > 2 && compare_parts.size() == 2)
        return false; // remote version has 2 components but local has three so local must be newer
    if (version_parts.size() == 2 && compare_parts.size() > 2)
        return true; // remote version has 3 components but local has two so remote must be newer
    if (version_parts.size() > 2 && compare_parts.size() > 2) {
        if (safeInt(version_parts[2]) <= safeInt(compare_parts[2]))
            return true;
        if (safeInt(version_parts[2]) > safeInt(compare_parts[2]))
            return false;
    }
    return false;
}

std::string JSONSafe(const std::string& s) {
    std::string safe;
    for (auto& c : s) {
        if (c == '\\')
            safe += "\\\\";
        else if (c == '"')
            safe += "\\\"";
        else
            safe += c;
    }
    return safe;
}

// Extract all chars before the first number in the string ... strip it from the input string
static std::string BeforeInt(std::string& s) {
    size_t i = 0;
    while (i < s.size() && (s[i] > '9' || s[i] < '0')) {
        i++;
    }
    if (i == 0) {
        return "";
    }

    std::string res = s.substr(0, i);
    s = s.substr(i);
    return res;
}

int intRand(const int& min, const int& max) {
    static thread_local std::mt19937* generator = nullptr;
    if (!generator)
        generator = new std::mt19937(
    static_cast<std::mt19937::result_type>(
        clock() + std::hash<std::thread::id>{}(std::this_thread::get_id())
    )
);
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(*generator);
}

// Extract any leading number ... strip it from the input string
int ExtractInt(std::string& s) {
    size_t i = 0;
    while (i < s.size() && s[i] <= '9' && s[i] >= '0') {
        i++;
    }

    if (i == 0) {
        return -1;
    }

    int res = (int)std::strtol(s.substr(0, i).c_str(), nullptr, 10);
    s = s.substr(i);
    return res;
}

// Extract an integer from the end of a string
int ExtractTrailingInt(const std::string& s) {
    size_t i = 0;
    while (i < s.size() && (s[i] > '9' || s[i] < '0')) {
        i++;
    }
    if (i == 0) {
        return -1;
    }
    return (int)std::strtol(s.substr(i).c_str(), nullptr, 10);
}

int NumberAwareStringCompare(const std::string& a, const std::string& b) {
    std::string aa = a;
    std::string bb = b;

    while (true) {
        std::string abi = BeforeInt(aa);
        std::string bbi = BeforeInt(bb);

        if (abi == bbi) {
            int ia = ExtractInt(aa);
            int ib = ExtractInt(bb);

            if (ia == ib) {
                if (aa == bb) {
                    return 0;
                }
            } else {
                if (ia < ib) {
                    return -1;
                }
                return 1;
            }
        } else {
            if (abi < bbi) {
                return -1;
            }
            return 1;
        }
    }
}

bool IsExcessiveMemoryUsage(double physicalMultiplier) {
#if defined(_WIN32) && defined(_WIN64)

    ULONGLONG physical;
    if (GetPhysicallyInstalledSystemMemory(&physical) != 0) {
        PROCESS_MEMORY_COUNTERS_EX mc;
        if (::GetProcessMemoryInfo(::GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&mc, sizeof(mc)) != 0) {
            // if we are using more ram than the machine has times the multiplier
            if (mc.PagefileUsage / 1024 > physicalMultiplier * physical) {
                return true;
            }
        }
    }
    return false;
#elif defined(__APPLE__)
    // max of 24G of swap used
    constexpr size_t MAX = 24l * 1024 * 1024 * 1024;
    std::error_code ec;
    size_t totalSize = 0;
    for (const auto& entry : std::filesystem::directory_iterator("/System/Volumes/VM", ec)) {
        if (ec) return false;
        auto sz = std::filesystem::file_size(entry.path(), ec);
        if (!ec) {
            totalSize += sz;
        }
        ec.clear();
    }
    return totalSize > MAX;
#else
    return false;
#endif
}

std::string Ordinal(int i) {
    std::string ii = std::to_string(i);

    if (EndsWith(ii, "11") || EndsWith(ii, "12") || EndsWith(ii, "13")) {
        return ii + "th";
    } else if (EndsWith(ii, "1")) {
        return ii + "st";
    } else if (EndsWith(ii, "2")) {
        return ii + "nd";
    } else if (EndsWith(ii, "3")) {
        return ii + "rd";
    } else {
        return ii + "th";
    }
}

bool IsEmailValid(const std::string& email) {
    std::string e = Trim(email);
    if (e.empty()) {
        return false;
    }
    static const std::regex regxEmail("^([a-zA-Z0-9\\.!#$%&+\\/=?^_`{|}~\\-]*@[a-zA-Z0-9\\-]+\\.[a-zA-Z0-9\\=\\.]+)$");
    return std::regex_match(e, regxEmail);
}

int GetxFadePort(int xfp) {
    if (xfp == 0)
        return 0;
    return xfp + 49912;
}

uint64_t GetPhysicalMemorySizeMB() {
    uint64_t ret = 0;
#if defined(__APPLE__)
    int mib[] = { CTL_HW, HW_MEMSIZE };
    size_t length = sizeof(ret);
    sysctl(mib, 2, &ret, &length, NULL, 0);
    ret /= 1024; // -> KB
#elif defined(_WIN32)
    GetPhysicallyInstalledSystemMemory(&ret);
    // already in KB
#else
    ret = get_phys_pages();
    ret *= getpagesize();
    ret /= 1024; // -> KB
#endif
    ret /= 1024; // -> MB
    return ret;
}

void CheckMemoryUsage(const std::string& reason, bool onchangeOnly) {
#if defined(TURN_THIS_OFF) && defined(_WIN32)

    static long lastPrivate = 0;
    static long lastWorking = 0;
    PROCESS_MEMORY_COUNTERS_EX memoryCounters;
    memoryCounters.cb = sizeof(memoryCounters);
    ::GetProcessMemoryInfo(::GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&memoryCounters, sizeof(memoryCounters));
    long privateMem = (long)(memoryCounters.PrivateUsage / 1024);
    long workingMem = (long)(memoryCounters.WorkingSetSize / 1024);
    if (!onchangeOnly || privateMem != lastPrivate) {
        spdlog::debug("Memory Usage: {} : private {}KB ({}KB) working {}KB ({}KB).",
                          reason,
                          privateMem,
                          privateMem - lastPrivate,
                          workingMem,
                          workingMem - lastWorking);
    }
    lastPrivate = privateMem;
    lastWorking = workingMem;
#endif
}

static bool _isxLights = false;
bool IsxLights() {
    return _isxLights;
}
void SetIsxLights(bool val) {
    _isxLights = val;
}



std::string ReverseCSV(const std::string& csv) {
    std::string res;
    std::vector<std::string> a = Split(csv, ',');
    for (auto it = a.rbegin(); it != a.rend(); ++it) {
        if (!res.empty())
            res += ",";
        res += *it;
    }
    return res;
}

void DumpBinary(uint8_t* buffer, size_t sz) {

    for (size_t i = 0; i < (sz + 15) / 16; i++) {
        std::string out;
        char hex[4];
        for (size_t j = i * 16; j < (std::min)(sz, (i + 1) * 16); j++) {
            snprintf(hex, sizeof(hex), "%02x ", buffer[j]);
            out += hex;
        }
        out += "    ";
        for (size_t j = i * 16; j < (std::min)(sz, (i + 1) * 16); j++) {
            if (buffer[j] < 32 || buffer[j] > 126) {
                out += '.';
            } else {
                out += char(buffer[j]);
            }
        }
        spdlog::debug(out);
    }
}



// returns true if the string contains what looks like a floating point number
bool IsFloat(const std::string& number) {
    // it cant be blank
    if (number == "")
        return false;
    // if it contains a - it must be the first character and there must only be one of them
    if (CountChar(number, '-') > 1 || (Contains(number, "-") && number[0] != '-'))
        return false;
    // it must contain zero or 1 '.'
    if (CountChar(number, '.') > 1)
        return false;
    // all other characters must be 0-9
    for (const auto it : number) {
        if (it != '.' && it != '-' && (it < '0' || it > '9'))
            return false;
    }
    return true;
}