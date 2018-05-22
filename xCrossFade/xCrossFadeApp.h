/***************************************************************
 * Name:      xCrossFadeApp.h
 * Purpose:   Defines Application Class
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#ifndef XCROSSFADEAPP_H
#define XCROSSFADEAPP_H

#ifdef _MSC_VER
#include <stdlib.h>
//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#include <wx/app.h>

class xCrossFadeApp : public wxApp
{
    public:
        virtual bool OnInit();
        virtual void OnFatalException();
        void WipeSettings();
        virtual int OnExit();
};

DECLARE_APP(xCrossFadeApp)

#endif // xCrossFadeAPP_H
