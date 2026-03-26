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

#include <pugixml.hpp>

#include "UtilFunctions.h"
#include "utils/string_utils.h"
#include "utils/xlImage.h"

#include <mutex>
#include <string_view>

#ifdef _WIN32
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <psapi.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
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

// This takes a string and removes all problematic characters from it for an XML file
std::string RemoveUnsafeXmlChars(const std::string& s) {
    std::string res;

    for (int i = 0; i < s.size(); i++) {
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
        generator = new std::mt19937(clock() + std::hash<std::thread::id>{}(std::this_thread::get_id()));
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

std::list<std::string> GetLocalIPs() {
    std::list<std::string> res;

#ifdef _WIN32

    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    if (pAdapterInfo == nullptr) {
        spdlog::error("Error getting adapter info.");
        return res;
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
        if (pAdapterInfo == nullptr) {
            spdlog::error("Error getting adapter info.");
            return res;
        }
    }

    PIP_ADAPTER_INFO pAdapter = nullptr;
    DWORD dwRetVal = 0;

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
            auto ip = &pAdapter->IpAddressList;
            while (ip != nullptr) {
                if (strcmp(ip->IpAddress.String, "0.0.0.0") != 0) {
                    res.push_back(std::string(ip->IpAddress.String));
                }
                ip = ip->Next;
            }

            pAdapter = pAdapter->Next;
        }
    }
    free(pAdapterInfo);
#else
    struct ifaddrs *interfaces, *tmp;
    getifaddrs(&interfaces);
    tmp = interfaces;
    // loop through all the interfaces
    while (tmp) {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* address = (struct sockaddr_in*)tmp->ifa_addr;
            std::string ip = inet_ntoa(address->sin_addr);
            if (ip != "0.0.0.0") {
                res.push_back(ip);
            }
        } else if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET6) {
            // LogDebug(VB_SYNC, "   Inet6 interface %s\n", tmp->ifa_name);
        }
        tmp = tmp->ifa_next;
    }
    freeifaddrs(interfaces);
#endif

    return res;
}

bool IsValidLocalIP(const std::string& ip) {
    for (const auto& it : GetLocalIPs()) {
        if (it == ip)
            return true;
    }

    return false;
}

bool IsInSameSubnet(const std::string& ip1, const std::string& ip2, const std::string& mask) {
    if (ip1 == "" || ip2 == "" || mask == "")
        return false;

    struct in_addr a1, a2, m;
    if (inet_pton(AF_INET, ip1.c_str(), &a1) != 1) return false;
    if (inet_pton(AF_INET, ip2.c_str(), &a2) != 1) return false;
    if (inet_pton(AF_INET, mask.c_str(), &m) != 1) return false;

    return (a1.s_addr & m.s_addr) == (a2.s_addr & m.s_addr);
}

bool DeleteDirectory(std::string directory) {
    spdlog::debug("  Processing directory: {}.", directory);
    std::error_code ec;
    if (!std::filesystem::exists(directory, ec)) {
        spdlog::error("  Thats odd ... the directory cannot be found: {}.", directory);
        return false;
    }
    auto removed = std::filesystem::remove_all(directory, ec);
    if (ec) {
        spdlog::error("  Could not delete folder {}: {}.", directory, ec.message());
        return false;
    }
    return true;
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

static std::string _resourcesDir;
std::string GetResourcesDir() {
    return _resourcesDir;
}
void SetResourcesDir(const std::string& dir) {
    _resourcesDir = dir;
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
        for (size_t j = i * 16; j < std::min(sz, (i + 1) * 16); j++) {
            snprintf(hex, sizeof(hex), "%02x ", buffer[j]);
            out += hex;
        }
        out += "    ";
        for (size_t j = i * 16; j < std::min(sz, (i + 1) * 16); j++) {
            if (buffer[j] < 32 || buffer[j] > 126) {
                out += '.';
            } else {
                out += char(buffer[j]);
            }
        }
        spdlog::debug(out);
    }
}

std::string CompressNodes(const std::string& nodes) {
    std::string res;
    if (nodes.empty())
        return res;
    // make sure it is fully expanded first
    auto s = ExpandNodes(nodes);
    int dir = 0;
    int start = -1;
    int last = -1;
    std::vector<std::string> as;
    Split(s, ',', as);

    // There is no difference between empty row and row with one blank pixel (shrug)
    // We will take removal of the last comma approach

    for (const auto& i : as) {
        if (i.empty() || i == "0") {
            // Flush out start/last if any
            if (start != -1) {
                if (last != start) {
                    res += std::to_string(start) + "-" + std::to_string(last) + ",";
                } else {
                    res += std::to_string(start) + ",";
                }
            }
            // Add empty and separator
            res += i + ",";
            start = last = -1;
            dir = 0;
            continue;
        }
        if (start == -1) {
            start = (int)std::strtol(i.c_str(), nullptr, 10);
            last = start;
            dir = 0;
        } else {
            int j = (int)std::strtol(i.c_str(), nullptr, 10);
            if (dir == 0) {
                if (j == last + 1) {
                    dir = 1;
                } else if (j == last - 1) {
                    dir = -1;
                } else {
                    res += std::to_string(start) + ",";
                    start = j;
                    dir = 0;
                }
                last = j;
            } else {
                if (j == last + dir) {
                } else {
                    res += std::to_string(start) + "-" + std::to_string(last) + ",";
                    start = j;
                    dir = 0;
                }
                last = j;
            }
        }
    }

    if (start == -1) {
        // nothing to do
    } else if (start == last) {
        res += std::to_string(start) + ",";
    } else {
        res += std::to_string(start) + "-" + std::to_string(last) + ",";
    }

    if (!res.empty())
        res = res.substr(0, res.length() - 1); // Chop last comma

    return res;
}

std::string ExpandNodes(const std::string& nodes) {
    std::string res;

    std::vector<std::string> as;
    Split(nodes, ',', as);

    bool first = true;
    for (const auto& i : as) {
        if (i.find('-') != std::string::npos) {
            std::vector<std::string> as2;
            Split(i, '-', as2);
            if (as2.size() == 2) {
                int start = (int)std::strtol(as2[0].c_str(), nullptr, 10);
                int end = (int)std::strtol(as2[1].c_str(), nullptr, 10);
                if (start < end) {
                    for (int j = start; j <= end; j++) {
                        if (!first || !res.empty())
                            res += ",";
                        res += std::to_string(j);
                    }
                } else if (start == end) {
                    if (!first || !res.empty())
                        res += ",";
                    res += std::to_string(start);
                } else {
                    for (int j = start; j >= end; j--) {
                        if (!first || !res.empty())
                            res += ",";
                        res += std::to_string(j);
                    }
                }
            }
        } else {
            if (!first || !res.empty())
                res += ",";
            res += i;
        }
        first = false;
    }
    return res;
}

void ShiftNodes(std::map<std::string, std::string>& nodes, int shift, int min, int max) {
    for (auto& line : nodes) {
        if (line.second.empty())
            continue;
        if (Contains(line.first, "Color"))
            continue;
        if (Contains(line.first, "Name"))
            continue;
        if (Contains(line.first, "Type"))
            continue;
        auto const oldnodes = ExpandNodes(line.second);
        std::vector<std::string> oldNodeArray;
        Split(oldnodes, ',', oldNodeArray);
        std::string newNodes;
        for (auto const& node : oldNodeArray) {
            long val = std::strtol(node.c_str(), nullptr, 10);
            if (val != 0 || node == "0") {
                long newVal = val + shift;
                if (newVal > max) {
                    newVal -= max;
                } else if (newVal < min) {
                    newVal += max;
                }
                if (!newNodes.empty()) newNodes += ",";
                newNodes += std::to_string(newVal);
            }
        }
        if (!newNodes.empty())
            line.second = CompressNodes(newNodes);
    }
}

void ReverseNodes(std::map<std::string, std::string>& nodes, int max) {
    for (auto& line : nodes) {
        if (line.second.empty())
            continue;
        if (Contains(line.first, "Color"))
            continue;
        if (Contains(line.first, "Name"))
            continue;
        if (Contains(line.first, "Type"))
            continue;
        auto const oldnodes = ExpandNodes(line.second);
        std::vector<std::string> oldNodeArray;
        Split(oldnodes, ',', oldNodeArray);
        std::string newNodes;
        for (auto const& node : oldNodeArray) {
            long val = std::strtol(node.c_str(), nullptr, 10);
            if (val != 0 || node == "0") {
                long newVal = max - val;
                if (!newNodes.empty()) newNodes += ",";
                newNodes += std::to_string(newVal);
            }
        }
        if (!newNodes.empty())
            line.second = CompressNodes(newNodes);
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

xlImage ApplyOrientation(const xlImage& img, int orient) {
    switch (orient) {
    case 2: return img.Mirror(true);  // horizontal flip
    case 3: return img.Rotate180();
    case 4: return img.Mirror(false); // vertical flip
    case 5: return img.Mirror(true).Rotate90(false); // horizontal flip + 90 CCW
    case 6: return img.Rotate90(true);  // 90 CW
    case 7: return img.Mirror(true).Rotate90(true);  // horizontal flip + 90 CW
    case 8: return img.Rotate90(false); // 90 CCW
    default: return img.Copy();
    }
}
