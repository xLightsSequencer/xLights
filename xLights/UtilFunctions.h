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
#include <wx/filename.h>
#include <wx/xml/xml.h>
#include "../xSchedule/wxJSON/json_defs.h"
#include "../xSchedule/wxJSON/jsonval.h"
#include "../xSchedule/wxJSON/jsonreader.h"

#include "utils/string_utils.h"

#include <string>
#include <algorithm>
#include <map>

#include <locale>
#include <codecvt>

#define AMPS_PER_PIXEL (0.055f)
#define FORMATTIME(ms) (const char *)wxString::Format("%d:%02d.%03d", ((uint32_t)ms) / 60000, (((uint32_t)ms) % 60000) / 1000, ((uint32_t)ms) % 1000).c_str()
#define INTROUNDUPDIV(a, b) (((a) + (b) - 1) / (b))
constexpr double PI = 3.141592653589793238463;

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
bool IsEmailValid(const std::string& email);
bool IsVersionOlder(const std::string &compare, const std::string &version);
std::string JSONSafe(const std::string& s);
std::string UnXmlSafe(const std::string &s);
std::string XmlSafe(const std::string& s);
std::string RemoveUnsafeXmlChars(const std::string& s);
std::string EscapeCSV(const std::string& s);
std::string EscapeRegex(const std::string& s);
inline bool isOdd(int n) { return n % 2 != 0; }
wxString GetXmlNodeAttribute(wxXmlNode* parent, const std::string& path, const std::string& attribute, const std::string& def = "");
wxString GetXmlNodeContent(wxXmlNode* parent, const std::string& path, const std::string& def = "");
std::vector<std::string> GetXmlNodeListContent(wxXmlNode* parent, const std::string& path, const std::string& listNodeName);
bool DoesXmlNodeExist(wxXmlNode* parent, const std::string& path);
void SetXmlNodeAttribute(wxXmlNode* node, wxString const& property, wxString const& value);
void DownloadVamp();
bool IsFileInShowDir(const wxString& showDir, const std::string filename);
void SetFixFileDirectories(const std::list<std::string>& dirs);
void SetFixFileShowDir(const wxString& ShowDir);
wxString FixFile(const wxString& ShowDir, const wxString& file);
void ClearNonExistentFiles();
wxString FixEffectFileParameter(const wxString& paramname, const wxString& parametervalue, const wxString& ShowDir);
int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data);
int GetxFadePort(int xfp);
void OptimiseDialogPosition(wxDialog* dlg);
wxJSONValue xLightsRequest(int xFadePort, const wxString& request, const wxString& ipAddress = "127.0.0.1");
bool xLightsRequest(std::string &result, int xFadePort, const wxString& request, const wxString& ipAddress = "127.0.0.1");

wxString ExpandNodes(const wxString& nodes);
wxString CompressNodes(const wxString& nodes);

//shift nodes  numbering 1->21, 50->70
void ShiftNodes(std::map<std::string, std::string> & nodes, int shift, int min, int max);
//reverse nodes, numbering 1->100, 100->1
void ReverseNodes(std::map<std::string, std::string> & nodes, int max);

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
int intRand(const int& min, const int& max);
int ExtractInt(std::string& s);
void SaveInt(const std::string& tag, int value);
int LoadInt(const std::string& tag, int defaultValue);
int NumberAwareStringCompare(const std::string &a, const std::string &b);
inline int NumberAwareStringCompareRev(const std::string &a, const std::string &b) { return NumberAwareStringCompare(b, a); }
inline int wxStringNumberAwareStringCompare(const wxString& a, const wxString& b) { return NumberAwareStringCompare(a.ToStdString(), b.ToStdString()); }
inline bool stdlistNumberAwareStringCompare(const std::string& a, const std::string& b) { return NumberAwareStringCompare(a, b) == -1; }

void EnsureWindowHeaderIsOnScreen(wxWindow* win);

static inline double toRadians(float degrees)
{
    return 2.0 * M_PI * double(degrees) / 360.0;
}

static inline double toDegrees(double radians) {
    return (radians / (2 * M_PI)) * 360.0;
}

inline const char* const toStr(bool b)
{
    return b ? "true" : "false";
}

inline std::string ToStdString(const wxString& wxstr)
{
#if defined(__WXOSX__)
    //OSX is native utf-8
    return wxstr.ToStdString();
#else
    return wxstr.utf8_str().data();
#endif
}
inline std::string ToUTF8(const wxString& wxstr) {
    return ToStdString(wxstr);
}
inline wxString ToWXString(const std::string& stdstr)
{
#if defined(__WXOSX__)
    return stdstr;
#else
    return wxString::FromUTF8(stdstr);
#endif
}
inline wxString FromUTF8(const std::string& str) {
    return ToWXString(str);
}
inline const std::string &GetPathSeparator() {
    static std::string PATHSEP = ToUTF8(wxFileName::GetPathSeparator());
    return PATHSEP;
}

bool IsExcessiveMemoryUsage(double physicalMultiplier = 0.95);
std::list<std::string> GetLocalIPs();
bool IsValidLocalIP(const std::string& ip);
bool IsValidLocalIP(const wxIPV4address& ip);
bool IsInSameSubnet(const std::string& ip1, const std::string& ip2, const std::string& mask = "255.255.255.0");

void ViewTempFile(const wxString& content, const wxString& name = "temp", const wxString& type = "txt");
void CheckMemoryUsage(const std::string& reason, bool onchangeOnly = false);
uint64_t GetPhysicalMemorySizeMB();


bool IsxLights();
std::string ReverseCSV(const std::string& csv);
void DumpBinary(uint8_t* buffer, size_t read);
wxColor CyanOrBlue();
wxColor LightOrMediumGrey();
bool IsFloat(const std::string& number);
bool IsDarkMode();
void SetSuppressDarkMode(bool suppress);
bool IsSuppressDarkMode();
