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

#include <wx/wx.h>
#include <wx/button.h>
#include <wx/colour.h>
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filepicker.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/socket.h>

#include "../Color.h"
#include "../UtilFunctions.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

// wx<->xlColor conversions
inline wxColour xlColorToWxColour(const xlColor& c) {
    return wxColour(c.red, c.green, c.blue, c.alpha);
}

inline xlColor wxColourToXlColor(const wxColour& c) {
    return xlColor(c.Red(), c.Green(), c.Blue());
}

// wx<->std string conversion helpers
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

inline int wxStringNumberAwareStringCompare(const wxString& a, const wxString& b) { return NumberAwareStringCompare(a.ToStdString(), b.ToStdString()); }

// wxString extern
extern const wxString xlEMPTY_WXSTRING;

// Consolidated set of wx-dependent utility functions
void CleanupIpAddress(wxString& IpAddr);
void DisplayError(const std::string& err, wxWindow* win = nullptr);
void DisplayWarning(const std::string& warn, wxWindow* win = nullptr);
void DisplayInfo(const std::string& info, wxWindow* win = nullptr);
void DisplayCrit(const std::string& crit, wxWindow* win = nullptr);
std::string DecodeIPError(wxSocketError err);
wxArrayString Split(const wxString& s, const std::vector<char>& delimiters);
bool IsFileInShowDir(const wxString& showDir, const std::string filename);
void SetFixFileShowDir(const wxString& ShowDir);
void SetFixFileDirectories(const std::list<std::string>& dirs);
wxString FixFile(const wxString& ShowDir, const wxString& file);
wxString MakeRelativeFile(const wxString& file);
void ClearNonExistentFiles();
wxString FixEffectFileParameter(const wxString& paramname, const wxString& parametervalue, const wxString& ShowDir);
int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data);
void OptimiseDialogPosition(wxDialog* dlg);
nlohmann::json xLightsRequest(int xFadePort, const wxString& request, const std::string& ipAddress = "127.0.0.1");
bool xLightsRequest(std::string& result, int xFadePort, const wxString& request, const std::string& ipAddress = "127.0.0.1");

wxImage ApplyOrientation(const wxImage& img, int orient);
int GetExifOrientation(const std::string& filename);

std::string GetResourcesDirectory();

void SaveWindowPosition(const std::string& tag, wxWindow* window);
void LoadWindowPosition(const std::string& tag, wxSize& size, wxPoint& position);
void SaveInt(const std::string& tag, int value);
int LoadInt(const std::string& tag, int defaultValue);
void EnsureWindowHeaderIsOnScreen(wxWindow* win);

bool IsValidLocalIP(const wxIPV4address& ip);

void ViewTempFile(const wxString& content, const wxString& name = "temp", const wxString& type = "txt");

void DownloadVamp();

wxColor CyanOrBlue();
wxColor LightOrMediumGrey();
wxColor BlueOrLightBlue();
wxColor RedOrLightRed();
bool IsDarkMode();
#ifdef __WXMSW__
void SetSuppressDarkMode(bool suppress);
bool IsSuppressDarkMode();
#endif

// ExternalHooks wx-dependent functions (moved from ExternalHooks.h)
bool FileExists(const wxString& s, bool waitForDownload = true);
bool FileExists(const wxFileName& fn, bool waitForDownload = true);
void GetAllFilesInDir(const wxString& dir, wxArrayString& files, const wxString& filespec, int flags = wxDIR_FILES);
void SetButtonBackground(wxButton* b, const wxColour& c, int bgType = 0);
void AdjustColorToDeviceColorspace(const wxColor& c, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);
bool DoInAppPurchases(wxWindow* w);
wxString GetOSFormattedClipboardData();

#if defined(__APPLE__)
#define AdjustModalDialogParent(par) par = nullptr
#else
#define AdjustModalDialogParent(par)
#endif

// ImageFilePickerCtrl
class ImageFilePickerCtrl : public wxFilePickerCtrl
{
public:
    ImageFilePickerCtrl(wxWindow *parent, wxWindowID id, const wxString& path, const wxString& message, const wxString& wildcard, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) :
        wxFilePickerCtrl(parent, id, path, message, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                                    ";*.webp"
                                                    "|All files (*.*)|*.*",
                         pos, size, style, validator, name)
    {
    }
};
