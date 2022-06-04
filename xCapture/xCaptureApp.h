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

#include "../common/xlBaseApp.h"

class xCaptureApp : public xlBaseApp
{
    public:
        xCaptureApp();

        virtual bool OnInit() override;
        void WipeSettings();
        virtual int OnExit() override;
};

DECLARE_APP(xCaptureApp)

#endif // XCAPTUREAPP_H
