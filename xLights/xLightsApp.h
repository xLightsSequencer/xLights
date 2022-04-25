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

class xLightsFrame;

class xLightsApp : public xlGLBaseApp
{
    void WipeSettings();

public:
    xLightsApp();

    virtual bool OnInit() override;
    static xLightsFrame* GetFrame() { return __frame; }
    static wxString showDir;
    static wxString mediaDir;
    static wxArrayString sequenceFiles;
    static xLightsFrame* __frame;

    #ifdef __WXOSX__
    virtual void MacOpenFiles(wxArrayString const& fileNames) override;
    #endif
    virtual bool ProcessIdle() override;
    uint64_t _nextIdleTime = 0;
};
