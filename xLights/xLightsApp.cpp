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

#ifdef LINUX
#include <GL/glut.h>
#endif

#ifndef __WXMSW__
#include <execinfo.h>
#else
#include <wx/textfile.h>
#include <algorithm>
#include <windows.h>
#include <imagehlp.h>
wxString windows_get_stacktrace(void *data)
{
    wxString trace;
    CONTEXT *context = (CONTEXT*)data;
    SymInitialize(GetCurrentProcess(), 0, true);

    STACKFRAME frame = { 0 };

    wxArrayString mapLines;
    wxFileName name = wxStandardPaths::Get().GetExecutablePath();
    name.SetExt("map");
    wxTextFile mapFile(name.GetFullPath());
    if (mapFile.Exists()) {
        mapFile.Open();
        wxString line = mapFile.GetFirstLine();
        while (!mapFile.Eof()) {
            line = mapFile.GetNextLine();
            line.Trim(true).Trim(false);
            if (line.StartsWith("0x")) {
                mapLines.Add(line);
            }
        }
        mapLines.Sort();
    } else {
        trace += name.GetFullPath() + " does not exist\n";
    }

    // setup initial stack frame
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
            wxString buffer(wxString::Format("0x%08x" , frame.AddrPC.Offset));
            for (int x = 1; x < mapLines.GetCount(); x++) {
                if (wxString(buffer) < mapLines[x]) {
                    buffer += mapLines[x - 1].substr(12).Trim();
                    x = mapLines.GetCount();
                }
            }
            trace += buffer + "\n";
        }
    }

    SymCleanup( GetCurrentProcess() );
    return trace;
}

#endif


IMPLEMENT_APP(xLightsApp)


#include <wx/debugrpt.h>

xLightsFrame *topFrame = NULL;
void handleCrash(void *data) {
    wxDebugReportCompress *report = new wxDebugReportCompress();
    report->SetCompressedFileDirectory(topFrame->CurrentDir);
    report->AddAll(wxDebugReport::Context_Exception);
    report->AddAll(wxDebugReport::Context_Current);
    report->AddFile(wxFileName(topFrame->CurrentDir, "xlights_networks.xml").GetFullPath(), "xlights_networks.xml");
    report->AddFile(wxFileName(topFrame->CurrentDir, "xlights_rgbeffects.xml").GetFullPath(), "xlights_rgbeffects.xml");
    if (topFrame->GetSeqXmlFileName() != "") {
        wxFileName fn(topFrame->GetSeqXmlFileName());
        if (fn.Exists() && !fn.IsDir()) {
            report->AddFile(topFrame->GetSeqXmlFileName(), fn.GetName());
        }
    }
    wxString trace = wxString::Format("xLights version %s\n", xlights_version_string);

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

    int id = (int)wxThread::GetCurrentId();
    trace += wxString::Format("\nCrashed thread id: %X\n", id);
#ifndef LINUX
    trace += topFrame->GetThreadStatusReport();
#endif // LINUX

    report->AddText("backtrace.txt", trace, "Backtrace");
    if (!wxThread::IsMain() && topFrame != nullptr) {
        topFrame->CallAfter(&xLightsFrame::CreateDebugReport, report);
        wxSleep(600000);
    } else {
        topFrame->CreateDebugReport(report);
    }
}
wxString xLightsFrame::GetThreadStatusReport() {
    return jobPool.GetThreadStatus();
}

void xLightsFrame::CreateDebugReport(wxDebugReportCompress *report) {
    if (wxDebugReportPreviewStd().Show(*report)) {
        report->Process();
        wxMessageBox("Crash report saved to " + report->GetCompressedFileName());
    }
    delete report;
    exit(1);
}


#if !(wxUSE_ON_FATAL_EXCEPTION)
#include <windows.h>
//MinGW needs to do this manually
LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS * ExceptionInfo)
{
    handleCrash(ExceptionInfo->ContextRecord);
}
#endif


bool xLightsApp::OnInit()
{
//    heartbeat("init", true); //tell monitor active now -DJ
//check for options on command line: -DJ
//TODO: maybe use wxCmdLineParser instead?
//do this before instantiating xLightsFrame so it can use info gathered here
    wxString unrecog, info;

    static const wxCmdLineEntryDesc cmdLineDesc [] =
    {
        { wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        { wxCMD_LINE_SWITCH, "n", "noauto", "enable auto-run prompt"},
        { wxCMD_LINE_SWITCH, "d", "debug", "enable debug mode"},
        { wxCMD_LINE_OPTION, "m", "media", "specify media directory"},
        { wxCMD_LINE_OPTION, "s", "show", "specify show directory"},
        { wxCMD_LINE_PARAM, "", "", "sequence file", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
        { wxCMD_LINE_NONE }
    };

    wxCmdLineParser parser(cmdLineDesc, argc, argv);
    switch (parser.Parse()) {
    case -1:
        // help was given
        return false;
    case 0:
        WantDebug = parser.Found("d");
        if (WantDebug) {
            info += _("Debug is ON\n");
        }
        RunPrompt = parser.Found("n");
        if (RunPrompt) {
            info += _("Auto-run prompt is ON\n");
        }
        if (parser.Found("s", &showDir)) {
            info += _("Setting show directory to ") + showDir + "\n";
        }
        if (parser.Found("m", &mediaDir)) {
            info += _("Setting media directory to ") + mediaDir + "\n";
        } else if (!showDir.IsNull()) {
            mediaDir = showDir;
        }
        if (parser.GetParamCount()>0) {
            sequenceFile = parser.GetParam(0);
            wxString seqDir =
            info += _("Loading sequence ") + sequenceFile + "\n";
            if (showDir.IsNull()) {
                showDir=wxPathOnly(sequenceFile);
            }
            if (mediaDir.IsNull()) {
                mediaDir=wxPathOnly(sequenceFile);
            }
        }
        if (!info.empty()) wxMessageBox(info, _("Command Line Options")); //give positive feedback*/
        break;
    default:
        wxMessageBox(_("Unrecognized command line parameters"),_("Command Line Error"));
        return false;
    }

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
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    topFrame = (xLightsFrame* )GetTopWindow();

    wxImage::AddHandler(new wxPNGHandler);
    #ifdef LINUX
        glutInit(&(wxApp::argc), wxApp::argv);
    #endif


    return wxsOK;
}


void xLightsApp::OnFatalException() {
    handleCrash(NULL);
}


//global flags from command line:
bool xLightsApp::WantDebug = false;
bool xLightsApp::RunPrompt = false; //prompt before running schedule (allows override) -DJ
wxString xLightsApp::DebugPath;
wxString xLightsApp::mediaDir;
wxString xLightsApp::showDir;
wxString xLightsApp::sequenceFile;
