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

#include <string>
#include <vector>

#include <wx/string.h>
#include <wx/event.h>    // wxDECLARE_EVENT, wxCommandEvent
#include <wx/gdicmn.h>   // wxPoint, wxSize

#include "../utils/Color.h"
#include "../utils/UtilFunctions.h"
#include "../utils/xlImage.h"

#include <nlohmann/json.hpp>

// Forward declarations — avoid pulling in full wx headers
class wxWindow;
class wxButton;
class wxDialog;
class wxImage;
class wxColour;
class wxFileName;
class wxIPV4address;
class wxArrayString;
typedef wxColour wxColor;

// wx<->xlColor conversions (implementations in wxUtilities.cpp)
wxColour xlColorToWxColour(const xlColor& c);
xlColor wxColourToXlColor(const wxColour& c);

// wx<->xlImage conversions (implementations in wxUtilities.cpp)
xlImage wxImageToXlImage(const wxImage& img);
wxImage xlImageToWxImage(const xlImage& img);

// UI events used by effect panels (moved from render/Effect.h to keep render core wx-free)
wxDECLARE_EVENT(EVT_SETTIMINGTRACKS, wxCommandEvent);

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
std::string DecodeIPError(int err);
wxArrayString Split(const wxString& s, const std::vector<char>& delimiters);
inline bool IsFileInShowDir(const wxString& showDir, const std::string filename) { return IsFileInShowDir(showDir.ToStdString(), filename); }
// wxString wrappers around std::string versions in UtilFunctions.h
inline void SetFixFileShowDir(const wxString& ShowDir) { SetFixFileShowDir(ShowDir.ToStdString()); }
inline wxString FixFile(const wxString& ShowDir, const wxString& file) { return wxString(FixFile(ShowDir.ToStdString(), file.ToStdString())); }
inline wxString MakeRelativeFile(const wxString& file) { return wxString(MakeRelativeFile(file.ToStdString())); }
inline wxString FixEffectFileParameter(const wxString& paramname, const wxString& parametervalue, const wxString& ShowDir) {
    return wxString(FixEffectFileParameter(paramname.ToStdString(), parametervalue.ToStdString(), ShowDir.ToStdString()));
}
int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data);
void OptimiseDialogPosition(wxDialog* dlg);
nlohmann::json xLightsRequest(int xFadePort, const wxString& request, const std::string& ipAddress = "127.0.0.1");
bool xLightsRequest(std::string& result, int xFadePort, const wxString& request, const std::string& ipAddress = "127.0.0.1");

wxImage ApplyOrientation(const wxImage& img, int orient);
AnimatedImageData LoadGIFAnimationDataWx(const std::string& filename);

std::string GetResourcesDirectory();

void SaveWindowPosition(const std::string& tag, wxWindow* window);
void LoadWindowPosition(const std::string& tag, wxSize& size, wxPoint& position);
void SaveInt(const std::string& tag, int value);
int LoadInt(const std::string& tag, int defaultValue);
void EnsureWindowHeaderIsOnScreen(wxWindow* win);

bool IsValidLocalIP(const wxIPV4address& ip);

void ViewTempFile(const wxString& content, const wxString& name = "temp", const wxString& type = "txt");

void DownloadVamp();

// wxConfig wrapper for cross-process communication
void SetConfigBool(const std::string& key, bool value);
bool GetConfigBool(const std::string& key, bool defaultValue = false);

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
void GetAllFilesInDir(const wxString& dir, wxArrayString& files, const wxString& filespec, int flags = 0x0001 /*wxDIR_FILES*/);
void SetButtonBackground(wxButton* b, const wxColour& c, int bgType = 0);
void AdjustColorToDeviceColorspace(const wxColor& c, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);
bool DoInAppPurchases(wxWindow* w);
wxString GetOSFormattedClipboardData();

#if defined(__APPLE__)
#define AdjustModalDialogParent(par) par = nullptr
#else
#define AdjustModalDialogParent(par)
#endif

// ImageFilePickerCtrl — needs full wx/filepicker.h for inheritance
#include <wx/filepicker.h>

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
