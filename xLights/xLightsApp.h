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

#include <wx/app.h>

class xLightsApp : public wxApp
{
public:
    virtual bool OnInit();
    static bool WantDebug; //debug flag from command-line -DJ
    static bool RunPrompt; //prompt before running schedule (allows override) -DJ
    static wxString DebugPath; //path name for debug log file -DJ
    
    virtual void OnFatalException();

};

#endif // XLIGHTSAPP_H
