/***************************************************************
 * Name:      xScheduleApp.h
 * Purpose:   Defines Application Class
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#ifndef XSCHEDULEAPP_H
#define XSCHEDULEAPP_H

#include <wx/app.h>

class xScheduleApp : public wxApp
{
    public:
        virtual bool OnInit();
};

DECLARE_APP(xScheduleApp)

#endif // XSCHEDULEAPP_H
