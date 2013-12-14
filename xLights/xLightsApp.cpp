/***************************************************************
 * Name:      xLightsApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/

#include "xLightsApp.h"

//(*AppHeaders
#include "xLightsMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(xLightsApp)

bool xLightsApp::OnInit()
{
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

//check for options on command line: -DJ
//TODO: maybe use wxCmdLineParser instead?
    wxString unrecog, info;
    for (int i = 1; i < wxApp::argc; ++i)
        if (!stricmp(wxApp::argv[i], "/debug"))
        {
            WantDebug = true;
            info += _("Debug in ON\n");
        }
//        else if ... //check for other options
        else unrecog += wxString::Format(wxT("\narg[%d/%d]: '%s'"), i, wxApp::argc, wxApp::argv[i]);

    if (!unrecog.empty()) wxMessageBox(info + _("Unrecognized command line parameters:") + unrecog, _("Command Line Error"));
    else if (!info.empty()) wxMessageBox(info, _("Command Line Options")); //give positive feedback
    return wxsOK;
}
bool xLightsApp::WantDebug = false; //global debug flag from command line -DJ

//re-added global keyboard handler: -DJ
//couldn't get PanelSequence2 keyboard handler to work, so just use a global filter as suggested in
// http://wiki.wxwidgets.org/Catching_key_events_globally
//also, there are some issues with controls in other panels

int xLightsApp::FilterEvent(wxEvent& event)
{
    if (event.GetEventType() == wxEVT_CHAR)
    {
        xLightsFrame* Frame = (xLightsFrame*)GetTopWindow();
        if (Frame->HotKey((wxKeyEvent&)event)) return true;
    }
    return -1;
}
