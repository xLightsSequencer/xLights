#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "../common/xlBaseApp.h"
#include "../common/xlGLBaseApp.h"

class xLightsFrame;

#include "graphics/xlGraphicsBase.h"

#if defined(XL_DRAWING_WITH_OPENGL)
#define xLightsAppBaseClass xlGLBaseApp
#else
#define xLightsAppBaseClass xlBaseApp
#endif



class xLightsApp : public xLightsAppBaseClass
{
    void WipeSettings();

public:
    xLightsApp();

    virtual bool OnInit() override;
    static xLightsFrame* GetFrame() { return __frame; }
    static wxString showDir;
    static wxString mediaDir;
    static wxString cleanupDir;
    static wxArrayString sequenceFiles;
    static xLightsFrame* __frame;

    #ifdef __WXOSX__
    virtual void MacOpenFiles(wxArrayString const& fileNames) override;
    #endif
    virtual bool ProcessIdle() override;
    uint64_t _nextIdleTime = 0;
};

#ifdef __WXOSX__
// Spawn a new xLights process via `/usr/bin/open -n -a <bundle> [--args <file>]`.
// Pass an empty fileToOpen to launch a blank sibling instance. Returns false if
// the running app's .app bundle cannot be resolved (unbundled / dev runs).
bool SpawnNewXLightsInstance(const wxString& fileToOpen);
#endif
