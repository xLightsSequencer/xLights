/***************************************************************
 * Name:      xScheduleApp.cpp
 * Purpose:   Code for Application Class
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#include "xScheduleApp.h"

//(*AppHeaders
#include "xScheduleMain.h"
#include <wx/image.h>
//*)

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Configurator.hh>
#include <wx/file.h>
#include <wx/msgdlg.h>

#include "../xLights/xLightsVersion.h"
#include <wx/filename.h>
#include "ScheduleManager.h"
#include "../xLights/outputs/OutputManager.h"
#include <wx/stdpaths.h>
#include <wx/debugrpt.h>

IMPLEMENT_APP(xScheduleApp)

std::string DecodeOS(wxOperatingSystemId o)
{
    switch (o)
    {
    case wxOS_UNKNOWN:
        return "Call get get operating system failed.";
    case wxOS_MAC_OS:
        return "Apple Mac OS 8 / 9 / X with Mac paths.";
    case wxOS_MAC_OSX_DARWIN:
        return "Apple OS X with Unix paths.";
    case wxOS_MAC:
        return "An Apple Mac of some type.";
    case wxOS_WINDOWS_NT:
        return "Windows NT family(XP / Vista / 7 / 8 / 10).";
    case wxOS_WINDOWS:
        return "A Windows system of some type.";
    case wxOS_UNIX_LINUX:
        return "Linux.";
    case wxOS_UNIX_FREEBSD:
        return "FreeBSD.";
    case wxOS_UNIX_OPENBSD:
        return "OpenBSD.";
    case wxOS_UNIX_NETBSD:
        return "NetBSD.";
    case wxOS_UNIX_SOLARIS:
        return "Solaris.";
    case wxOS_UNIX_AIX:
        return "AIX.";
    case wxOS_UNIX_HPUX:
        return "HP / UX.";
    case wxOS_UNIX:
        return "Some flavour of Unix.";
    default:
        break;
    }

    return "Unknown Operating System.";
}

void DumpConfig()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Version: " + std::string(xlights_version_string.c_str()));
    logger_base.info("Build Date: " + std::string(xlights_build_date.c_str()));
    logger_base.info("Machine configuration:");
    wxMemorySize s = wxGetFreeMemory();
    if (s != -1)
    {
        logger_base.info("  Free Memory:%ld.", s);
    }
    logger_base.info("  Current directory: " + std::string(wxGetCwd().c_str()));
    logger_base.info("  Machine name: " + std::string(wxGetHostName().c_str()));
    logger_base.info("  OS: " + std::string(wxGetOsDescription().c_str()));
    int verMaj = -1;
    int verMin = -1;
    wxOperatingSystemId o = wxGetOsVersion(&verMaj, &verMin);
    logger_base.info("  OS: %s %d.%d.%d", (const char *)DecodeOS(o).c_str(), verMaj, verMin);
    if (wxIsPlatform64Bit())
    {
        logger_base.info("      64 bit");
    }
    else
    {
        logger_base.info("      NOT 64 bit");
    }
    if (wxIsPlatformLittleEndian())
    {
        logger_base.info("      Little Endian");
    }
    else
    {
        logger_base.info("      Big Endian");
    }
#ifdef LINUX
    wxLinuxDistributionInfo l = wxGetLinuxDistributionInfo();
    logger_base.info("  " + std::string(l.Id.c_str()) \
        + " " + std::string(l.Release.c_str()) \
        + " " + std::string(l.CodeName.c_str()) \
        + " " + std::string(l.Description.c_str()));
#endif
}

void InitialiseLogging(bool fromMain)
{
    static bool loggingInitialised = false;

    if (!loggingInitialised)
    {

#ifdef __WXMSW__
        std::string initFileName = "xschedule.windows.properties";
#endif
#ifdef __WXOSX_MAC__
        std::string initFileName = "xschedule.mac.properties";
        if (!wxFile::Exists(initFileName)) {
            if (fromMain) {
                return;
            }
            else if (wxFile::Exists(wxStandardPaths::Get().GetResourcesDir() + "/xlights.mac.properties")) {
                initFileName = wxStandardPaths::Get().GetResourcesDir() + "/xlights.mac.properties";
            }
        }
        loggingInitialised = true;

#endif
#ifdef __LINUX__
        std::string initFileName = "/usr/share/xLights/xschedule.linux.properties";
#endif

        if (!wxFile::Exists(initFileName))
        {
#ifdef _MSC_VER
            // the app is not initialized so GUI is not available and no event loop.
            wxMessageBox(initFileName + " not found in " + wxGetCwd() + ". Logging disabled.");
#endif
        }
        else
        {
            try
            {
                log4cpp::PropertyConfigurator::configure(initFileName);
            }
            catch (log4cpp::ConfigureFailure& e) {
                // ignore config failure ... but logging wont work
                printf("Log issue:  %s\n", e.what());
            }
            catch (const std::exception& ex) {
                printf("Log issue: %s\n", ex.what());
            }
        }
    }
}

xScheduleFrame *topFrame = nullptr;

#ifndef __WXMSW__
#include <execinfo.h>
#else
#include <wx/textfile.h>
#include <windows.h>
#include <imagehlp.h>

#ifdef _WIN64
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
    }
    else {
        trace += name.GetFullPath() + " does not exist\n";
    }

    // setup initial stack frame
    frame.AddrPC.Offset = context->Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context->Rsp;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context->Rbp;
    frame.AddrFrame.Mode = AddrModeFlat;

    while (StackWalk64(IMAGE_FILE_MACHINE_AMD64,
        GetCurrentProcess(),
        GetCurrentThread(),
        &frame,
        context,
        0,
        SymFunctionTableAccess64,
        SymGetModuleBase64,
        0))
    {
        static char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255];
        memset(symbolBuffer, 0, sizeof(IMAGEHLP_SYMBOL) + 255);
        IMAGEHLP_SYMBOL * symbol = (IMAGEHLP_SYMBOL*)symbolBuffer;

        // Need to set the first two fields of this symbol before obtaining name info:
        symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL) + 255;
        symbol->MaxNameLength = 254;

        // The displacement from the beginning of the symbol is stored here: pretty useless
        unsigned displacement = 0;

        // Get the symbol information from the address of the instruction pointer register:
        if (SymGetSymFromAddr64(
            GetCurrentProcess(),   // Process to get symbol information for
            frame.AddrPC.Offset,   // Address to get symbol for: instruction pointer register
            (DWORD64*)& displacement,   // Displacement from the beginning of the symbol: whats this for ?
            symbol                      // Where to save the symbol
        )) {
            // Add the name of the function to the function list:
            char buffer[2048]; sprintf(buffer, "0x%08x %s\n", frame.AddrPC.Offset, symbol->Name);
            trace += buffer;
        }
        else {
            // Print an unknown location:
            // functionNames.push_back("unknown location");
            wxString buffer(wxString::Format("0x%08x", frame.AddrPC.Offset));
            for (size_t x = 1; x < mapLines.GetCount(); x++) {
                if (wxString(buffer) < mapLines[x]) {
                    buffer += mapLines[x - 1].substr(12).Trim();
                    x = mapLines.GetCount();
                }
            }
            trace += buffer + "\n";
        }
    }

    SymCleanup(GetCurrentProcess());
    return trace;
}
#else
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
    }
    else {
        trace += name.GetFullPath() + " does not exist\n";
    }

    // setup initial stack frame
    frame.AddrPC.Offset = context->Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context->Esp;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context->Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;

    while (StackWalk(IMAGE_FILE_MACHINE_I386,
        GetCurrentProcess(),
        GetCurrentThread(),
        &frame,
        context,
        0,
        SymFunctionTableAccess,
        SymGetModuleBase,
        0))
    {
        static char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255];
        memset(symbolBuffer, 0, sizeof(IMAGEHLP_SYMBOL) + 255);
        IMAGEHLP_SYMBOL * symbol = (IMAGEHLP_SYMBOL*)symbolBuffer;

        // Need to set the first two fields of this symbol before obtaining name info:
        symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL) + 255;
        symbol->MaxNameLength = 254;

        // The displacement from the beginning of the symbol is stored here: pretty useless
        unsigned displacement = 0;

        // Get the symbol information from the address of the instruction pointer register:
        if (SymGetSymFromAddr(
            GetCurrentProcess(),   // Process to get symbol information for
            frame.AddrPC.Offset,   // Address to get symbol for: instruction pointer register
            (DWORD*)& displacement,   // Displacement from the beginning of the symbol: whats this for ?
            symbol                      // Where to save the symbol
        )) {
            // Add the name of the function to the function list:
            char buffer[2048]; sprintf(buffer, "0x%08x %s\n", frame.AddrPC.Offset, symbol->Name);
            trace += buffer;
        }
        else {
            // Print an unknown location:
            // functionNames.push_back("unknown location");
            wxString buffer(wxString::Format("0x%08x", frame.AddrPC.Offset));
            for (size_t x = 1; x < mapLines.GetCount(); x++) {
                if (wxString(buffer) < mapLines[x]) {
                    buffer += mapLines[x - 1].substr(12).Trim();
                    x = mapLines.GetCount();
                }
            }
            trace += buffer + "\n";
        }
    }

    SymCleanup(GetCurrentProcess());
    return trace;
}
#endif // _WIN64

#endif

void handleCrash(void *data) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.crit("Crash handler called.");
    wxDebugReportCompress *report = new wxDebugReportCompress();
    report->SetCompressedFileDirectory(xScheduleFrame::GetScheduleManager()->GetShowDir());
    report->AddAll(wxDebugReport::Context_Exception);
    report->AddAll(wxDebugReport::Context_Current);

    wxFileName fn(xScheduleFrame::GetScheduleManager()->GetShowDir(), OutputManager::GetNetworksFileName());
    if (fn.Exists()) {
        report->AddFile(fn.GetFullPath(), OutputManager::GetNetworksFileName());
    }
    if (wxFileName(xScheduleFrame::GetScheduleManager()->GetShowDir(), ScheduleManager::GetScheduleFile()).Exists()) {
        report->AddFile(wxFileName(xScheduleFrame::GetScheduleManager()->GetShowDir(), ScheduleManager::GetScheduleFile()).GetFullPath(), ScheduleManager::GetScheduleFile());
    }

    wxString dir;
#ifdef __WXMSW__
    wxGetEnv("APPDATA", &dir);
    std::string filename = std::string(dir.c_str()) + "/xSchedule_l4cpp.log";
#endif
#ifdef __WXOSX_MAC__
    wxFileName home;
    home.AssignHomeDir();
    dir = home.GetFullPath();
    std::string filename = std::string(dir.c_str()) + "/Library/Logs/xSchedule_l4cpp.log";
#endif
#ifdef __LINUX__
    std::string filename = "/tmp/xSchedule_l4cpp.log";
#endif

    if (wxFile::Exists(filename))
    {
        report->AddFile(filename, "xSchedule_l4cpp.log");
    }
    else if (wxFile::Exists(wxFileName(xScheduleFrame::GetScheduleManager()->GetShowDir(), "xSchedule_l4cpp.log").GetFullPath()))
    {
        report->AddFile(wxFileName(xScheduleFrame::GetScheduleManager()->GetShowDir(), "xSchedule_l4cpp.log").GetFullPath(), "xSchedule_l4cpp.log");
    }
    else if (wxFile::Exists(wxFileName(wxGetCwd(), "xSchedule_l4cpp.log").GetFullPath()))
    {
        report->AddFile(wxFileName(wxGetCwd(), "xSchedule_l4cpp.log").GetFullPath(), "xSchedule_l4cpp.log");
    }

    wxString trace = wxString::Format("xSchedule version %s\n\n", xlights_version_string);

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
    trace += windows_get_stacktrace(data);
#endif

    int id = (int)wxThread::GetCurrentId();
    trace += wxString::Format("\nCrashed thread id: %X\n", id);

    logger_base.crit(trace);

    report->AddText("backtrace.txt", trace, "Backtrace");
    if (!wxThread::IsMain() && topFrame != nullptr) {
        topFrame->CallAfter(&xScheduleFrame::CreateDebugReport, report);
        wxSleep(600000);
    }
    else {
        topFrame->CreateDebugReport(report);
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

bool xScheduleApp::OnInit()
{
    wxLog::SetLogLevel(wxLOG_FatalError);

#ifdef _MSC_VER
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

#if wxUSE_ON_FATAL_EXCEPTION
    wxHandleFatalExceptions();
#else
    SetUnhandledExceptionFilter(windows_exception_handler);
#endif

    InitialiseLogging(false);
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("******* OnInit: xSchedule started.");

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	xScheduleFrame* Frame = new xScheduleFrame(0);
        topFrame = Frame;
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;
}

// CODE COPIED FROM XLIGHTS TO DUMP STACK TRACES

void xScheduleApp::OnFatalException() {
    handleCrash(nullptr);
}

