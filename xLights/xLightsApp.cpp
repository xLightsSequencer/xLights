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


#ifndef __WXMSW__
#include <execinfo.h>
#else

#include <windows.h>
#include <imagehlp.h>
wxString windows_get_stacktrace(void *data)
{
    wxString trace;
    CONTEXT *context = (CONTEXT*)data;

    SymInitialize(GetCurrentProcess(), 0, true);

    STACKFRAME frame = { 0 };

    /* setup initial stack frame */
    frame.AddrPC.Offset         = context->Eip;
    frame.AddrPC.Mode           = AddrModeFlat;
    frame.AddrStack.Offset      = context->Esp;
    frame.AddrStack.Mode        = AddrModeFlat;
    frame.AddrFrame.Offset      = context->Ebp;
    frame.AddrFrame.Mode        = AddrModeFlat;

    while (StackWalk(IMAGE_FILE_MACHINE_I386 ,
                   GetCurrentProcess(),
                   GetCurrentThread(),
                   &frame,
                   context,
                   0,
                   SymFunctionTableAccess,
                   SymGetModuleBase,
                   0 ) )
    {
        static char symbolBuffer[ sizeof(IMAGEHLP_SYMBOL) + 255 ];
        memset( symbolBuffer , 0 , sizeof(IMAGEHLP_SYMBOL) + 255 );
        IMAGEHLP_SYMBOL * symbol = (IMAGEHLP_SYMBOL*) symbolBuffer;

        // Need to set the first two fields of this symbol before obtaining name info:
        symbol->SizeOfStruct    = sizeof(IMAGEHLP_SYMBOL) + 255;
        symbol->MaxNameLength   = 254;

        // The displacement from the beginning of the symbol is stored here: pretty useless
        unsigned displacement = 0;

        // Get the symbol information from the address of the instruction pointer register:
        if (SymGetSymFromAddr(
                    GetCurrentProcess()     ,   // Process to get symbol information for
                    frame.AddrPC.Offset     ,   // Address to get symbol for: instruction pointer register
                    (DWORD*) & displacement ,   // Displacement from the beginning of the symbol: whats this for ?
                    symbol                      // Where to save the symbol
                )) {
            // Add the name of the function to the function list:
            char buffer[2048]; sprintf( buffer , "0x%08x %s\n" ,  frame.AddrPC.Offset , symbol->Name );
            trace += buffer;
        } else {
            // Print an unknown location:
            // functionNames.push_back("unknown location");
            char buffer[64]; sprintf( buffer , "0x%08x\n" ,  frame.AddrPC.Offset );
            trace += buffer;
        }
    }

    SymCleanup( GetCurrentProcess() );
    return trace;
}

#endif

#include <wx/debugrpt.h>

xLightsFrame *topFrame = NULL;
void handleCrash(void *data) {

    wxDebugReportCompress report;
    report.SetCompressedFileDirectory(topFrame->CurrentDir);
    report.AddAll(wxDebugReport::Context_Exception);
    report.AddFile(wxFileName(topFrame->CurrentDir, "xlights_networks.xml").GetFullPath(), "xlights_networks.xml");
    report.AddFile(wxFileName(topFrame->CurrentDir, "xlights_rgbeffects.xml").GetFullPath(), "xlights_rgbeffects.xml");
    if (topFrame->GetSeqXmlFileName() != "") {
        report.AddFile(topFrame->GetSeqXmlFileName(), wxFileName(topFrame->GetSeqXmlFileName()).GetName());
    }
    wxString trace;
#ifndef __WXMSW__
    void* callstack[128];
    int i, frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    for (i = 0; i < frames; ++i) {
        trace += strs[i];
        trace += "\n";
    }
    free(strs);
#else
    trace = windows_get_stacktrace(data);
#endif
    if (!trace.IsEmpty()) {
        report.AddText("backtrace.txt", trace, "Backtrace");
    }
    if (wxDebugReportPreviewStd().Show(report)) {
        report.Process();
    }
}


#if !(wxUSE_ON_FATAL_EXCEPTION)
#include <windows.h>
//MinGW needs to do this manually
LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS * ExceptionInfo)
{
    handleCrash(ExceptionInfo->ContextRecord);
}
#endif

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

#if wxUSE_ON_FATAL_EXCEPTION
    wxHandleFatalExceptions();
#else
    SetUnhandledExceptionFilter(windows_exception_handler);
#endif

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	xLightsFrame* Frame = new xLightsFrame(0);
        topFrame = Frame;
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)

    return wxsOK;
}


void xLightsApp::OnFatalException() {
    handleCrash(NULL);
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
