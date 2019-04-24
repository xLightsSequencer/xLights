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

#include <wx/app.h>

class xSMSDaemonApp : public wxApp
{
    public:
        virtual bool OnInit() override;
        virtual int OnExit() override;
};

DECLARE_APP(xSMSDaemonApp)

#endif // xSMSDaemonAPP_H
