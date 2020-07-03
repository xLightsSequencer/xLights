#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/app.h>

class wxSingleInstanceChecker;
class xScheduleFrame;

class xScheduleApp : public wxApp
{
    wxSingleInstanceChecker* _checker = nullptr;

    public:
        virtual bool OnInit() override;
        virtual void OnFatalException() override;
        void WipeSettings();
        virtual int OnExit() override;
        xScheduleFrame* GetFrame() { return (xScheduleFrame*)GetTopWindow(); }
};

DECLARE_APP(xScheduleApp)
