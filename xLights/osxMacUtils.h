//
//  osMacUtils.h
//  xLights
//
//  Created by Daniel Kulp on 8/17/15.
//  Copyright (c) 2015 Daniel Kulp. All rights reserved.
//

#ifndef xLights_osxMacUtils_h
#define xLights_osxMacUtils_h

#ifdef __WXOSX__
class xlGLCanvas;
class wxMenu;

void xlSetOpenGLRetina(xlGLCanvas &win);
void xlSetRetinaCanvasViewport(xlGLCanvas &win, int &x, int &y, int &x2, int&y2);
double xlTranslateToRetina(xlGLCanvas &win, double x);
void ObtainAccessToURL(const std::string &path);
void EnableSleepModes();
void DisableSleepModes();

void ModalPopup(wxWindow *p, wxMenu &menu);

#define WINDOW_LOCKER(a, b)

#else
#define xlSetOpenGLRetina(a)
#define xlSetRetinaCanvasViewport(w,a,b,c,d)
#define xlTranslateToRetina(a, x) x
#define ObtainAccessToURL(x)
#define EnableSleepModes()
#define DisableSleepModes()
#define ModalPopup(p, a) p->PopupMenu(&a)
#define WINDOW_LOCKER(a, b) wxWindowUpdateLocker b(a)
#endif


#endif
