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

#include "Color.h"

#ifdef __WXOSX__
class xlGLCanvas;
class wxMenu;
class wxWindow;

void xlSetOpenGLRetina(xlGLCanvas &win);
void xlSetRetinaCanvasViewport(xlGLCanvas &win, int &x, int &y, int &x2, int&y2);
double xlTranslateToRetina(xlGLCanvas &win, double x);
bool ObtainAccessToURL(const std::string &path);
void EnableSleepModes();
void DisableSleepModes();

void ModalPopup(wxWindow *p, wxMenu &menu);

bool IsMouseEventFromTouchpad();
class AudioManager;
void AddAudioDeviceChangeListener(AudioManager *am);
void RemoveAudioDeviceChangeListener(AudioManager *am);

void AdjustColorToDeviceColorspace(const wxColor &c, xlColor &xlc);
bool IsFromAppStore();

bool DoInAppPurchases(wxWindow *w);


#define AdjustModalDialogParent(par) par = nullptr

#define WINDOW_LOCKER(a, b)

#else
#define xlSetOpenGLRetina(a)
#define xlSetRetinaCanvasViewport(w,a,b,c,d)
#define xlTranslateToRetina(a, x) x
inline bool ObtainAccessToURL(const std::string &path) { return true; }
#define EnableSleepModes()
#define DisableSleepModes()
#define ModalPopup(p, a) p->PopupMenu(&a)
#define WINDOW_LOCKER(a, b) wxWindowUpdateLocker b(a)
#define AddAudioDeviceChangeListener(a)
#define RemoveAudioDeviceChangeListener(a)
#define AdjustModalDialogParent(par)
inline bool IsFromAppStore() { return false; }
inline void AdjustColorToDeviceColorspace(const wxColor& c, xlColor& xlc) {xlc = c;}

inline bool IsMouseEventFromTouchpad() {
    return false;
}
#endif
