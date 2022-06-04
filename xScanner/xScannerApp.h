/***************************************************************
 * Name:      xScannerApp.h
 * Purpose:   Defines Application Class
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#pragma once

#include "../common/xlBaseApp.h"


class xScannerApp : public xlBaseApp
{
    public:
        xScannerApp();

        virtual bool OnInit() override;
        void WipeSettings();
        virtual int OnExit() override;
};

DECLARE_APP(xScannerApp)
