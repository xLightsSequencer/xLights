#ifndef UTILFUNCTIONS_H
#define UTILFUNCTIONS_H

#include <wx/wx.h>
#include <string>
#include "SequenceData.h"

// Consolidated set of utility functions
bool IsVersionOlder(const std::string compare, const std::string version);
std::string UnXmlSafe(const std::string& s);
std::string XmlSafe(const std::string& s);
wxString FixFile(const wxString& ShowDir, const wxString& file, bool recurse = false);
wxString FixEffectFileParameter(const wxString& paramname, const wxString& parametervalue, const wxString& ShowDir);
int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data);
wxString base64_encode(SequenceData& SeqData);
inline int rountTo4(int i) {
    int remainder = i % 4;
    if (remainder == 0) {
        return i;
    }
    return i + 4 - remainder;
}

inline int rountTo16(int i) {
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

#endif
