/***************************************************************
 * Name:      xFadeApp.h
 * Purpose:   Defines Application Class
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#ifndef XFADEAPP_H
#define XFADEAPP_H

#ifdef _MSC_VER
#include <stdlib.h>
//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#include "../common/xlBaseApp.h"

class xFadeApp : public xlBaseApp
{
    public:
        xFadeApp();

        virtual bool OnInit() override;
        void WipeSettings();
        virtual int OnExit() override;
};

DECLARE_APP(xFadeApp)

#endif // XFADEAPP_H
