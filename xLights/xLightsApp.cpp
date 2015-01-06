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

#include <wx/debugrpt.h>


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
        else if (wxApp::argv[i].StartsWith(wxT("/debug=")))
        {
            WantDebug = true;
            DebugPath = wxApp::argv[i].Mid(7);
            info += _("Debug is ON, path = '") + DebugPath + _("'");
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
    wxHandleFatalExceptions();
    if ( wxsOK )
    {
    	xLightsFrame* Frame = new xLightsFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)

    return wxsOK;
}

#ifndef __WXMSW__
#include <execinfo.h>
#endif

void xLightsApp::OnFatalException() {
    xLightsFrame *frame = (xLightsFrame*)GetTopWindow();
    wxDebugReportCompress report;
    report.SetCompressedFileDirectory(frame->CurrentDir);
    report.AddAll(wxDebugReport::Context_Exception);
    report.AddFile(wxFileName(frame->CurrentDir, "xlights_networks.xml").GetFullPath(), "xlights_networks.xml");
    report.AddFile(wxFileName(frame->CurrentDir, "xlights_rgbeffects.xml").GetFullPath(), "xlights_rgbeffects.xml");
    if (frame->SeqXmlFileName != "") {
        report.AddFile(frame->SeqXmlFileName, wxFileName(frame->SeqXmlFileName).GetName());
    }
#ifndef __WXMSW__
    void* callstack[128];
    int i, frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    wxString backtrace;
    for (i = 0; i < frames; ++i) {
        backtrace += strs[i];
        backtrace += "\n";
    }
    free(strs);
    report.AddText("backtrace.txt", backtrace, "Backtrace");
#endif
    if (wxDebugReportPreviewStd().Show(report)) {
        report.Process();
    }
}


//global flags from command line:
bool xLightsApp::WantDebug = false;
bool xLightsApp::RunPrompt = false; //prompt before running schedule (allows override) -DJ
wxString xLightsApp::DebugPath;

//re-added global keyboard handler: -DJ
//couldn't get PaneNutcracker keyboard handler to work, so just use a global filter as suggested in
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
