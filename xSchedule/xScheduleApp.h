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

#include "../common/xlBaseApp.h"


class wxSingleInstanceChecker;
class xScheduleFrame;

class xScheduleApp : public xlBaseApp
{
    wxSingleInstanceChecker* _checker = nullptr;

    public:
        xScheduleApp();

        virtual bool OnInit() override;
        void WipeSettings();
        virtual int OnExit() override;
        xScheduleFrame* GetFrame() { return (xScheduleFrame*)GetTopWindow(); }
};

DECLARE_APP(xScheduleApp)
