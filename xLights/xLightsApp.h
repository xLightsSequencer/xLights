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

#ifdef _MSC_VER
#include <stdlib.h>
//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#include <wx/app.h>
#include <wx/cmdline.h>

class xLightsFrame;

class xLightsApp : public wxApp
{
    void WipeSettings();

public:
    virtual bool OnInit() override;
    static xLightsFrame* GetFrame() { return __frame; }
    static bool WantDebug; //debug flag from command-line -DJ
    static wxString DebugPath; //path name for debug log file -DJ
    static wxString showDir;
    static wxString mediaDir;
    static wxArrayString sequenceFiles;
    static xLightsFrame* __frame;

    #ifdef __WXOSX__
    virtual void MacOpenFiles(const wxArrayString &fileNames) override;
    #endif

    virtual void OnFatalException() override;
    
    virtual bool ProcessIdle() override;
    uint64_t _nextIdleTime = 0;
};
