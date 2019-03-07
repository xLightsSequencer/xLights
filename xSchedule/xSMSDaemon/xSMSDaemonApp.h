/***************************************************************
 * Name:      xSMSDaemonApp.h
 * Purpose:   Defines Application Class
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#ifndef xSMSDaemonAPP_H
#define xSMSDaemonAPP_H

#ifdef _MSC_VER
#include <stdlib.h>
//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#include <wx/app.h>

class wxSingleInstanceChecker;

class xSMSDaemonApp : public wxApp
{
    wxSingleInstanceChecker* _checker;

    public:
        virtual bool OnInit() override;
        virtual void OnFatalException() override;
        void WipeSettings();
        virtual int OnExit() override;
};

DECLARE_APP(xSMSDaemonApp)

#endif // xSMSDaemonAPP_H
