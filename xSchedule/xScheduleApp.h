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

class wxSingleInstanceChecker;
class xScheduleFrame;

class xScheduleApp : public wxApp
{
    wxSingleInstanceChecker* _checker;

    public:
        virtual bool OnInit() override;
        virtual void OnFatalException() override;
        void WipeSettings();
        virtual int OnExit() override;
        xScheduleFrame* GetFrame() { return (xScheduleFrame*)GetTopWindow(); }
};

DECLARE_APP(xScheduleApp)

#endif // XSCHEDULEAPP_H
