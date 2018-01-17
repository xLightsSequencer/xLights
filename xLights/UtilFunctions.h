#ifndef UTILFUNCTIONS_H
#define UTILFUNCTIONS_H

#include <wx/wx.h>
#include <string>
#include "SequenceData.h"

// Consolidated set of utility functions
bool IsVersionOlder(const std::string compare, const std::string version);
std::string UnXmlSafe(wxString s);
std::string XmlSafe(const std::string& s);
bool IsFileInShowDir(const wxString& showDir, const std::string filename);
wxString FixFile(const wxString& ShowDir, const wxString& file, bool recurse = false);
wxString FixEffectFileParameter(const wxString& paramname, const wxString& parametervalue, const wxString& ShowDir);
int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data);
wxString base64_encode(SequenceData& SeqData);
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
int NumberAwareStringCompare(std::string a, std::string b);
inline int NumberAwareStringCompareRev(std::string a, std::string b) { return NumberAwareStringCompare(b, a); }
inline int wxStringNumberAwareStringCompare(const wxString& a, const wxString& b) { return NumberAwareStringCompare(a.ToStdString(), b.ToStdString()); }
inline bool stdlistNumberAwareStringCompare(const std::string& a, const std::string& b) { return NumberAwareStringCompare(a, b) == -1; }
#endif
