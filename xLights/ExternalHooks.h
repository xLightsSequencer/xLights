#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 **************************************************************/


#if defined(__WXOSX__)
#if __has_include("ExternalHooksMacOS.h")
#include "osxUtils/ExternalHooksMacOS.h"
#endif
#elif defined(__WXMSW__)
#if __has_include("ExternalHooksMSW.h")
#include "mswUtils/ExternalHooksMSW.h"
#endif
#else
#if __has_include("ExternalHooksLinux.h")
#include "linuxUtils/ExternalHooksLinux.h"
#endif
#endif

#ifndef __XL_EXTERNAL_HOOKS__

#include <functional>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/button.h>
#include "Color.h"

#define EnableSleepModes()
#define DisableSleepModes()
#define AddAudioDeviceChangeListener(a)
#define RemoveAudioDeviceChangeListener()
#define AdjustModalDialogParent(par)
#define DoInAppPurchases(w)
#define WXGLUnsetCurrentContext()
#define GetOSFormattedClipboardData() ""
#define StartGraphicsSyncPoint()
#define EndGraphicsSyncPoint()
#define SetThreadQOS(a)

inline double xlOSGetMainScreenContentScaleFactor() { return 1.0; }
inline bool ObtainAccessToURL(const std::string &path) { return true; }
inline bool IsFromAppStore() { return false; }
inline void AdjustColorToDeviceColorspace(const wxColor& c, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) { r = c.Red(); g = c.Green(); b = c.Blue(); a = c.Alpha();}
inline bool IsMouseEventFromTouchpad() { return false; }
inline void RunInAutoReleasePool(std::function<void()> &&f) { f(); }

inline bool FileExists(const std::string &s, bool waitForDownload = true) {
    return wxFile::Exists(s);
}
inline bool FileExists(const wxString &s, bool waitForDownload = true) {
    return wxFile::Exists(s);
}
inline bool FileExists(const wxFileName &fn, bool waitForDownload = true) {
    return fn.FileExists();
}
inline void GetAllFilesInDir(const wxString& dir, wxArrayString& files, const wxString& filespec, int flags = wxDIR_FILES)
{
    wxDir::GetAllFiles(dir, &files, filespec, flags);
}
inline void SetButtonBackground(wxButton *b, const wxColour &c, int bgType) {
    b->SetBackgroundColour(c);
    b->Refresh();
}

#endif
