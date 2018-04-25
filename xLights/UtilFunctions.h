#ifndef UTILFUNCTIONS_H
#define UTILFUNCTIONS_H

#include <wx/wx.h>
#include <string>
#include "SequenceData.h"

#define FORMATTIME(ms) (const char *)wxString::Format("%d:%02d.%03d", (ms) / 60000, ((ms) % 60000) / 1000, (ms) % 1000).c_str()

// Consolidated set of utility functions
bool IsIPValid(const std::string &ip);
bool IsIPValidOrHostname(const std::string &ip, bool iponly = false);
bool IsVersionOlder(const std::string &compare, const std::string &version);
std::string UnXmlSafe(const wxString &s);
std::string XmlSafe(const std::string& s);
bool IsFileInShowDir(const wxString& showDir, const std::string filename);
wxString FixFile(const wxString& ShowDir, const wxString& file, bool recurse = false);
wxString FixEffectFileParameter(const wxString& paramname, const wxString& parametervalue, const wxString& ShowDir);
int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data);
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
#endif
