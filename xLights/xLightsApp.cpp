/***************************************************************
 * Name:      xLightsApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/

#include "xLightsApp.h"
//#include "heartbeat.h" //DJ

//(*AppHeaders
#include "xLightsMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(xLightsApp)

bool xLightsApp::OnInit()
{
//    heartbeat("init", true); //tell monitor active now -DJ
//check for options on command line: -DJ
//TODO: maybe use wxCmdLineParser instead?
//do this before instantiating xLightsFrame so it can use info gathered here
    wxString unrecog, info;
    for (int i = 1; i < wxApp::argc; ++i)
        if (!strcasecmp(wxApp::argv[i], "/debug"))
        {
            WantDebug = true;
            info += _("Debug is ON\n");
        }
        else if (!strcasecmp(wxApp::argv[i], "/noauto"))
        {
            RunPrompt = true;
            info += _("Auto-run prompt is ON\n");
        }
//        else if ... //check for other options
        else unrecog += wxString::Format("\narg[%d/%d]: '%s'", i, wxApp::argc, wxApp::argv[i]);

    if (!unrecog.empty()) wxMessageBox(info + _("Unrecognized command line parameters:") + unrecog, _("Command Line Error"));
    else if (!info.empty()) wxMessageBox(info, _("Command Line Options")); //give positive feedback

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	xLightsFrame* Frame = new xLightsFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)

    return wxsOK;
}
//global flags from command line:
bool xLightsApp::WantDebug = false;
bool xLightsApp::RunPrompt = false; //prompt before running schedule (allows override) -DJ

//re-added global keyboard handler: -DJ
//couldn't get PanelSequence2 keyboard handler to work, so just use a global filter as suggested in
// http://wiki.wxwidgets.org/Catching_key_events_globally
//also, there are some issues with controls in other panels

int xLightsApp::FilterEvent(wxEvent& event)
{
    if (event.GetEventType() == wxEVT_CHAR)
    {
        xLightsFrame* Frame = (xLightsFrame*)GetTopWindow();
        if (Frame->HotKey((wxKeyEvent&)event)) return Event_Processed; //true;
    }
    return Event_Skip; //-1;
}
