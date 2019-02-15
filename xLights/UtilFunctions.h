#ifndef UTILFUNCTIONS_H
#define UTILFUNCTIONS_H

#include <wx/wx.h>
#include <string>
#include <wx/socket.h>

#define FORMATTIME(ms) (const char *)wxString::Format("%d:%02d.%03d", ((uint32_t)ms) / 60000, (((uint32_t)ms) % 60000) / 1000, ((uint32_t)ms) % 1000).c_str()

// Consolidated set of utility functions
void DisplayError(const std::string& err, wxWindow* win = nullptr);
void DisplayWarning(const std::string& warn, wxWindow* win = nullptr);
void DisplayInfo(const std::string& info, wxWindow* win = nullptr);
void DisplayCrit(const std::string& crit, wxWindow* win = nullptr);
std::string DecodeIPError(wxSocketError err);
std::string Ordinal(int i);
std::string DecodeMidi(int midi);
bool DeleteDirectory(std::string directory);
bool IsIPValid(const std::string &ip);
bool IsIPValidOrHostname(const std::string &ip, bool iponly = false);
bool IsVersionOlder(const std::string &compare, const std::string &version);
std::string UnXmlSafe(const wxString &s);
std::string XmlSafe(const std::string& s);
bool IsFileInShowDir(const wxString& showDir, const std::string filename);
wxString FixFile(const wxString& ShowDir, const wxString& file, bool recurse = false);
wxString FixEffectFileParameter(const wxString& paramname, const wxString& parametervalue, const wxString& ShowDir);
int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data);
int GetxFadePort(int xfp);
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

void SaveWindowPosition(const std::string tag, wxWindow* window);
void LoadWindowPosition(const std::string tag, wxSize& size, wxPoint& position);
int NumberAwareStringCompare(const std::string &a, const std::string &b);
inline int NumberAwareStringCompareRev(const std::string &a, const std::string &b) { return NumberAwareStringCompare(b, a); }
inline int wxStringNumberAwareStringCompare(const wxString& a, const wxString& b) { return NumberAwareStringCompare(a.ToStdString(), b.ToStdString()); }
inline bool stdlistNumberAwareStringCompare(const std::string& a, const std::string& b) { return NumberAwareStringCompare(a, b) == -1; }

double GetSystemContentScaleFactor();
double ScaleWithSystemDPI(double val);
double ScaleWithSystemDPI(double scalingFactor, double val);
double UnScaleWithSystemDPI(double val);
double UnScaleWithSystemDPI(double scalingFactor, double val);

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

inline char HexToChar(char c1, char c2)
{
    return (HexToChar(c1) << 8) + HexToChar(c2);
}

inline bool Contains(const std::string& in, const std::string& contains) noexcept
{
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

inline bool EndsWith(const std::string& in, const std::string& endswith) noexcept
{
    return in.size() >= endswith.size() && in.substr(in.size() - endswith.size()) == endswith;
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

static inline double toRadians(float degrees) 
{
    return 2.0*M_PI*double(degrees) / 360.0;
}

static inline double toDegrees(double radians) {
    return (radians / (2 * M_PI))*360.0;
}

#endif