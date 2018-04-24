/***************************************************************
 * Name:      xLightsApp.h
 * Purpose:   Defines Application Class
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/

#ifndef XLIGHTSAPP_H
#define XLIGHTSAPP_H

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
    virtual bool OnInit();
    static xLightsFrame* GetFrame() { return __frame; }
    static bool WantDebug; //debug flag from command-line -DJ
    static wxString DebugPath; //path name for debug log file -DJ
    static wxString showDir;
    static wxString mediaDir;
    static wxString fseqDir;
    static wxString backupDir;
    static wxArrayString sequenceFiles;
    static xLightsFrame* __frame;

    virtual void OnFatalException();
};

#endif // XLIGHTSAPP_H
