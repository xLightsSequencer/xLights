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
#include "ExternalHooks.h"
#include "utils/string_utils.h"

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
    std::filesystem::remove_all(directory, ec);
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

// ---- FixFile and related functions ----

static std::list<std::string> _fixFileSearchDirs;
static std::string _fixFileShowDir;
static std::recursive_mutex _fixFileMutex;
static std::vector<std::string> _fixFileNonExistent;
static std::map<std::string, std::string> _fixFileMap;

void SetFixFileShowDir(const std::string& showDir) {
    _fixFileShowDir = showDir;
}

void SetFixFileDirectories(const std::list<std::string>& dirs) {
    _fixFileSearchDirs = dirs;
}

void ClearNonExistentFiles() {
    std::unique_lock<std::recursive_mutex> lock(_fixFileMutex);
    _fixFileNonExistent.clear();
}

// Get just the filename from a path, handling both / and backslash separators
static std::string GetFilenameFromPath(const std::string& path) {
    auto pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

// Get directory components from a path, splitting on both / and backslash
static std::vector<std::string> GetPathComponents(const std::string& path) {
    std::vector<std::string> components;
    std::string current;
    for (char c : path) {
        if (c == '/' || c == '\\') {
            if (!current.empty()) {
                components.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    // Don't include the filename — only directory components
    return components;
}

// Check if a file exists in a directory with the given filename
static bool doesFileExist(const std::string& dir, const std::string& filename, std::string& resultPath) {
    if (filename.empty()) return false;
    std::filesystem::path p = std::filesystem::path(dir) / filename;
    std::string candidate = p.string();
    if (FileExists(candidate, false)) {
        spdlog::debug("File location fixed: {} -> {}", filename, candidate);
        resultPath = candidate;
        return true;
    }
    return false;
}

// Search dir + append, then all search directories
static bool doesFileExistInDirs(const std::string& baseDir, const std::string& append,
                                const std::string& filename, std::string& resultPath) {
    std::string searchDir = baseDir;
    if (!append.empty()) {
        searchDir = (std::filesystem::path(baseDir) / append).string();
    }
    if (doesFileExist(searchDir, filename, resultPath)) return true;
    for (const auto& fd : _fixFileSearchDirs) {
        std::string sd = fd;
        if (!append.empty()) {
            sd = (std::filesystem::path(fd) / append).string();
        }
        if (doesFileExist(sd, filename, resultPath)) return true;
    }
    return false;
}

std::string FixFile(const std::string& showDir, const std::string& file) {
    if (showDir != _fixFileShowDir && !showDir.empty() && _fixFileShowDir.empty()) {
        _fixFileShowDir = showDir;
    }

    if (file.empty()) return file;

    if (FileExists(file, false)) return file;

    // Handle meshobjects special case
    auto meshPos = file.find("/meshobjects/");
    if (meshPos != std::string::npos) {
        return GetResourcesDir() + file.substr(meshPos);
    }

    std::unique_lock<std::recursive_mutex> lock(_fixFileMutex);

    // Check cache
    auto it = _fixFileMap.find(file);
    if (it != _fixFileMap.end()) return it->second;

    if (std::find(_fixFileNonExistent.begin(), _fixFileNonExistent.end(), file) != _fixFileNonExistent.end()) {
        return file;
    }

    std::string sd = showDir.empty() ? _fixFileShowDir : showDir;
    lock.unlock();

    spdlog::debug("File not found ... attempting to fix location ({}) : {}", sd, file);

    // Extract filename using both Unix and Windows separators
    std::string filename = GetFilenameFromPath(file);
    std::string resultPath;

    // Search show dir and search dirs for the file directly
    if (doesFileExistInDirs(sd, "", filename, resultPath)) {
        lock.lock();
        _fixFileMap[file] = resultPath;
        return resultPath;
    }

    // Search subdirectories of the show dir
    {
        std::error_code ec;
        if (std::filesystem::is_directory(sd, ec)) {
            for (const auto& entry : std::filesystem::directory_iterator(sd, ec)) {
                if (entry.is_directory()) {
                    std::string folderName = entry.path().filename().string();
                    std::string folderLower = folderName;
                    std::transform(folderLower.begin(), folderLower.end(), folderLower.begin(), ::tolower);
                    if (folderLower != "backup") {
                        if (doesFileExist(entry.path().string(), filename, resultPath)) {
                            lock.lock();
                            _fixFileMap[file] = resultPath;
                            return resultPath;
                        }
                    }
                }
            }
        }
    }

    // Try to match directory structure from the file path
    std::string sdLower = std::filesystem::path(sd).filename().string();
    std::transform(sdLower.begin(), sdLower.end(), sdLower.begin(), ::tolower);

    auto components = GetPathComponents(file);

    // Forward search: find the show folder name in the path components and use everything after it
    std::string appendPath;
    bool appending = false;
    for (const auto& comp : components) {
        std::string compLower = comp;
        std::transform(compLower.begin(), compLower.end(), compLower.begin(), ::tolower);
        if (compLower == sdLower) {
            appending = true;
        } else if (appending) {
            if (!appendPath.empty()) appendPath += std::filesystem::path::preferred_separator;
            appendPath += comp;
        }
    }
    if (!appendPath.empty()) {
        if (doesFileExistInDirs(sd, appendPath, filename, resultPath)) {
            lock.lock();
            _fixFileMap[file] = resultPath;
            return resultPath;
        }
    }

    // Check if file contains the show folder name and try the relative portion
    std::string fileLower = file;
    std::transform(fileLower.begin(), fileLower.end(), fileLower.begin(), ::tolower);
    auto sdPos = fileLower.find(sdLower);
    if (sdPos != std::string::npos) {
        size_t offset = sdPos + sdLower.length();
        std::string relative = file.substr(offset);
        if (FileExists(relative, false)) {
            lock.lock();
            _fixFileMap[file] = relative;
            return relative;
        }
    }

    // Reverse search: traverse up from the end of the file's directory components
    for (int x = (int)components.size() - 1; x >= 0; x--) {
        std::string revPath;
        for (int y = x; y < (int)components.size(); y++) {
            if (!revPath.empty()) revPath += std::filesystem::path::preferred_separator;
            revPath += components[y];
        }
        if (doesFileExistInDirs(sd, revPath, filename, resultPath)) {
            lock.lock();
            _fixFileMap[file] = resultPath;
            return resultPath;
        }
    }

    // Last resort: try with the last directory component of the file path as a subdirectory
    if (showDir.empty() && !components.empty()) {
        std::string lastDir = sd + std::string(1, std::filesystem::path::preferred_separator) + components.back();
        return FixFile(lastDir, file);
    }

    spdlog::debug("   could not find a fixed file location for : {}", file);
    spdlog::debug("   We will not look for this file again until a new sequence is loaded.");
    lock.lock();
    _fixFileNonExistent.push_back(file);
    return file;
}

std::string MakeRelativeFile(const std::string& file) {
    if (file.empty()) return {};
    if (!std::filesystem::path(file).is_absolute()) return {};

    // Normalize separators to /
    std::string f = file;
    std::replace(f.begin(), f.end(), '\\', '/');

    auto stripPrefix = [&](std::string base) -> std::string {
        std::replace(base.begin(), base.end(), '\\', '/');
        std::string baseCmp = base;
        std::string fCmp = f;
#ifdef _WIN32
        std::transform(baseCmp.begin(), baseCmp.end(), baseCmp.begin(), ::tolower);
        std::transform(fCmp.begin(), fCmp.end(), fCmp.begin(), ::tolower);
#endif
        if (!baseCmp.empty() && baseCmp.back() != '/') baseCmp += '/';
        if (fCmp.substr(0, baseCmp.size()) == baseCmp)
            return f.substr(baseCmp.size());
        return {};
    };

    std::string rel = stripPrefix(_fixFileShowDir);
    if (!rel.empty()) return rel;

    for (const auto& dir : _fixFileSearchDirs) {
        rel = stripPrefix(dir);
        if (!rel.empty()) return rel;
    }

    return {};
}

bool IsFileInShowDir(const std::string& showDir, const std::string& filename) {
    std::string sd = showDir.empty() ? _fixFileShowDir : showDir;
    if (sd.empty()) return false;
    std::string fixedFile = FixFile(sd, filename);

#ifdef _WIN32
    std::string fixedLower = fixedFile;
    std::transform(fixedLower.begin(), fixedLower.end(), fixedLower.begin(), ::tolower);
    std::string sdLower = sd;
    std::transform(sdLower.begin(), sdLower.end(), sdLower.begin(), ::tolower);
    if (fixedLower.substr(0, sdLower.size()) == sdLower) return true;
    for (auto d : _fixFileSearchDirs) {
        std::transform(d.begin(), d.end(), d.begin(), ::tolower);
        if (fixedLower.substr(0, d.size()) == d) return true;
    }
#else
    if (fixedFile.substr(0, sd.size()) == sd) return true;
    for (const auto& d : _fixFileSearchDirs) {
        if (fixedFile.substr(0, d.size()) == d) return true;
    }
#endif
    return false;
}

std::string FixEffectFileParameter(const std::string& paramname, const std::string& parametervalue, const std::string& showDir) {
    auto startparamname = parametervalue.find(paramname);
    if (startparamname == std::string::npos) return parametervalue;
    auto endparamname = parametervalue.find("=", startparamname);
    if (endparamname == std::string::npos) return parametervalue;
    auto startvalue = endparamname + 1;
    auto endvalue = parametervalue.find(",", startvalue);
    if (endvalue == std::string::npos) endvalue = parametervalue.size();
    std::string file = parametervalue.substr(startvalue, endvalue - startvalue);
    std::string newfile = FixFile(showDir, file);
    return parametervalue.substr(0, startvalue) + newfile + parametervalue.substr(endvalue);
}
