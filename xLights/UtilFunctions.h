#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/socket.h>
#include <wx/xml/xml.h>

#include <string>
#include <algorithm>

#define AMPS_PER_PIXEL (0.055f)
#define FORMATTIME(ms) (const char *)wxString::Format("%d:%02d.%03d", ((uint32_t)ms) / 60000, (((uint32_t)ms) % 60000) / 1000, ((uint32_t)ms) % 1000).c_str()

// Consolidated set of utility functions
void CleanupIpAddress(wxString& IpAddr);
void DisplayError(const std::string& err, wxWindow* win = nullptr);
void DisplayWarning(const std::string& warn, wxWindow* win = nullptr);
void DisplayInfo(const std::string& info, wxWindow* win = nullptr);
void DisplayCrit(const std::string& crit, wxWindow* win = nullptr);
std::string DecodeIPError(wxSocketError err);
std::string Ordinal(int i);
std::string DecodeMidi(int midi);
wxArrayString Split(const wxString& s, const std::vector<char>& delimiters);
bool DeleteDirectory(std::string directory);
bool IsIPValid(const std::string& ip);
bool IsEmailValid(const std::string& email);
bool IsIPValidOrHostname(const std::string &ip, bool iponly = false);
std::string CleanupIP(const std::string& ip);
std::string ResolveIP(const std::string& ip);
bool IsVersionOlder(const std::string &compare, const std::string &version);
std::string UnXmlSafe(const wxString &s);
std::string XmlSafe(const std::string& s);
std::string EscapeCSV(const std::string& s);
inline bool isOdd(int n) { return n % 2 != 0; }
wxString GetXmlNodeAttribute(wxXmlNode* parent, const std::string& path, const std::string& attribute, const std::string& def = "");
wxString GetXmlNodeContent(wxXmlNode* parent, const std::string& path, const std::string& def = "");
std::list<std::string> GetXmlNodeListContent(wxXmlNode* parent, const std::string& path, const std::string& listNodeName);
bool DoesXmlNodeExist(wxXmlNode* parent, const std::string& path);
void DownloadVamp();
bool IsFileInShowDir(const wxString& showDir, const std::string filename);
void SetFixFileDirectories(const std::list<std::string>& dirs);
void SetFixFileShowDir(const wxString& ShowDir);
wxString FixFile(const wxString& ShowDir, const wxString& file, bool recurse = false);
void ClearNonExistentFiles();
wxString FixEffectFileParameter(const wxString& paramname, const wxString& parametervalue, const wxString& ShowDir);
int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data);
int GetxFadePort(int xfp);
void OptimiseDialogPosition(wxDialog* dlg);
wxString xLightsRequest(int xFadePort, wxString message, wxString ipAddress = "127.0.0.1");
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

void SaveWindowPosition(const std::string& tag, wxWindow* window);
void LoadWindowPosition(const std::string& tag, wxSize& size, wxPoint& position);
void SaveInt(const std::string& tag, int value);
int LoadInt(const std::string& tag, int defaultValue);
int NumberAwareStringCompare(const std::string &a, const std::string &b);
inline int NumberAwareStringCompareRev(const std::string &a, const std::string &b) { return NumberAwareStringCompare(b, a); }
inline int wxStringNumberAwareStringCompare(const wxString& a, const wxString& b) { return NumberAwareStringCompare(a.ToStdString(), b.ToStdString()); }
inline bool stdlistNumberAwareStringCompare(const std::string& a, const std::string& b) { return NumberAwareStringCompare(a, b) == -1; }

double GetSystemContentScaleFactor();
double ScaleWithSystemDPI(double val);
double ScaleWithSystemDPI(double scalingFactor, double val);
double UnScaleWithSystemDPI(double val);
double UnScaleWithSystemDPI(double scalingFactor, double val);
void EnsureWindowHeaderIsOnScreen(wxWindow* win);

inline bool isHexChar(char c)
{
    return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

inline char HexToChar(char c)
{
    if (c >= '0' && c <= '9')
    {
        return (int)c - 0x30;
    }
    if (c >= 'A' && c <= 'F')
    {
        return (int)c - 65 + 10;
    }
    return (int)c - 97 + 10;
}

inline int CountChar(const std::string& s, char c) {
    return std::count(begin(s), end(s), c);
}

inline size_t CountStrings(const std::string& what, const std::string& in)
{
    size_t count = 0;
    size_t pos = 0;
    while ((pos = in.find(what, pos)) != std::string::npos) {
        count++; 
        pos += what.size();
    }
    return count;
}

inline char HexToChar(char c1, char c2) {
    return (HexToChar(c1) << 4) + HexToChar(c2);
}

inline bool Contains(const std::string& in, const std::string& contains) noexcept {
    return in.find(contains) != std::string::npos;
}

inline bool Contains(const std::wstring& in, const std::wstring& contains) noexcept
{
    return in.find(contains) != std::wstring::npos;
}

inline bool StartsWith(const std::string& in, const std::string& startswith) noexcept
{
    return in.size() >= startswith.size() && in.substr(0, startswith.size()) == startswith;
}

inline std::string AfterFirst(const std::string& in, char c)
{
    auto pos = in.find(c);
    if (pos == std::string::npos) return "";
    return in.substr(pos+1);
}

inline std::string BeforeFirst(const std::string& in, char c)
{
    auto pos = in.find(c);
    if (pos == std::string::npos) return in;
    return in.substr(0, pos);
}

inline bool EndsWith(const std::string& in, const std::string& endswith) noexcept
{
    return in.size() >= endswith.size() && in.substr(in.size() - endswith.size()) == endswith;
}

inline std::string AfterLast(const std::string& in, char c)
{
    auto pos = in.find_last_of(c);
    if (pos == std::string::npos) return "";
    return in.substr(pos+1);
}

inline void Replace(std::string& in, const std::string& what, const std::string& with)
{
    auto pos = in.find(what);

    // Repeat till end is reached
    while (pos != std::string::npos)
    {
        // Replace this occurrence of Sub String
        in.replace(pos, what.size(), with);
        // Get the next occurrence from the current position
        pos = in.find(what, pos + with.size());
    }
}

inline void Replace(std::wstring& in, const std::wstring& what, const std::wstring& with)
{
    auto pos = in.find(what);

    // Repeat till end is reached
    while (pos != std::wstring::npos)
    {
        // Replace this occurrence of Sub String
        in.replace(pos, what.size(), with);
        // Get the next occurrence from the current position
        pos = in.find(what, pos + with.size());
    }
}

inline std::string Capitalise(const std::string& input) noexcept
{
    std::string res = "";
    char last = ' ';
    for (const auto c : input)
    {
        if (last == ' ' || last == '\t' || last == '\n' || last == '\r')
        {
            res += ::toupper(c);
        }
        else
        {
            res += ::tolower(c);
        }

        last = c;
    }
    return res;
}

inline std::string Lower(const std::string& input) noexcept
{
    std::string res = "";
    for (const auto c : input)
    {
        res += ::tolower(c);
    }
    return res;
}

inline std::string Trim(const std::string& input)
{
    if (input == "") return "";

    int firstnonblank = 0;
    int lastnonblank = input.size()-1;

    while (firstnonblank < input.size() && (input[firstnonblank] == ' ' || input[firstnonblank] == '\t')) { firstnonblank++; }
    while (lastnonblank > 0 && (input[lastnonblank] == ' ' || input[lastnonblank] == '\t')) { lastnonblank--; }
    if (lastnonblank < firstnonblank) return "";
    return input.substr(firstnonblank, lastnonblank - firstnonblank + 1);
}

static inline double toRadians(float degrees)
{
    return 2.0 * M_PI * double(degrees) / 360.0;
}

static inline double toDegrees(double radians) {
    return (radians / (2 * M_PI)) * 360.0;
}

bool IsExcessiveMemoryUsage(double physicalMultiplier = 0.95);
std::list<std::string> GetLocalIPs();
bool IsValidLocalIP(const std::string& ip);

void ViewTempFile(const wxString& content, const wxString& name = "temp", const wxString& type = "txt");
void CheckMemoryUsage(const std::string& reason, bool onchangeOnly = false);
uint64_t GetPhysicalMemorySizeMB();


bool IsxLights();
std::string ReverseCSV(const std::string& csv);
void DumpBinary(uint8_t* buffer, size_t read);
