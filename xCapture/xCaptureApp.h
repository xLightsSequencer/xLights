/***************************************************************
 * Name:      xCaptureApp.h
 * Purpose:   Defines Application Class
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#ifndef XCAPTUREAPP_H
#define XCAPTUREAPP_H

#ifdef _MSC_VER
#include <stdlib.h>
//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#include <wx/app.h>

class xCaptureApp : public wxApp
{
    public:
        virtual bool OnInit();
        virtual void OnFatalException();
        void WipeSettings();
        virtual int OnExit();
};

DECLARE_APP(xCaptureApp)

#endif // XCAPTUREAPP_H
