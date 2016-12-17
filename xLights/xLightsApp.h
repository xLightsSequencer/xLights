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
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <wx/app.h>
#include <wx/cmdline.h>

class xLightsApp : public wxApp
{
    void WipeSettings();

public:
    virtual bool OnInit();
    static bool WantDebug; //debug flag from command-line -DJ
    static bool RunPrompt; //prompt before running schedule (allows override) -DJ
    static wxString DebugPath; //path name for debug log file -DJ
    static wxString showDir;
    static wxString mediaDir;
    static wxArrayString sequenceFiles;

    virtual void OnFatalException();
};

#endif // XLIGHTSAPP_H
