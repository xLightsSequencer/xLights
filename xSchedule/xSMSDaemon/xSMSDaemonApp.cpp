/***************************************************************
 * Name:      xSMSDaemonApp.cpp
 * Purpose:   Code for Application Class
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

//(*AppHeaders
#include "xSMSDaemonMain.h"
#include <wx/image.h>
//*)

#include "xSMSDaemonApp.h"

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Configurator.hh>
#include <wx/file.h>
#include <wx/msgdlg.h>

#include "../../xLights/xLightsVersion.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/debugrpt.h>
#include <wx/cmdline.h>
#include <wx/confbase.h>
#include <wx/snglinst.h>

IMPLEMENT_APP(xSMSDaemonApp)

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
    logger_base.info("Bits: " + std::string(GetBitness().c_str()));
    logger_base.info("Build Date: " + std::string(xlights_build_date.c_str()));
    logger_base.info("Machine configuration:");
    wxMemorySize s = wxGetFreeMemory();
    if (s != -1)
    {
#if wxUSE_LONGLONG
        wxString msg = wxString::Format(_T("  Free Memory: %" wxLongLongFmtSpec "d."), s);
#else
        wxString msg = wxString::Format(_T("  Free Memory: %ld."), s);
#endif
        logger_base.info("%s", (const char *)msg.c_str());
    }
    logger_base.info("  Current directory: " + std::string(wxGetCwd().c_str()));
    logger_base.info("  Machine name: " + std::string(wxGetHostName().c_str()));
    logger_base.info("  OS: " + std::string(wxGetOsDescription().c_str()));
    int verMaj = -1;
    int verMin = -1;
    wxOperatingSystemId o = wxGetOsVersion(&verMaj, &verMin);
    logger_base.info("  OS: %s %d.%d", (const char *)DecodeOS(o).c_str(), verMaj, verMin);
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
        std::string initFileName = "xSMSDaemon.windows.properties";
#endif
#ifdef __WXOSX_MAC__
        std::string initFileName = "xSMSDaemon.mac.properties";
        std::string resourceName = wxStandardPaths::Get().GetResourcesDir().ToStdString() + "/xSMSDaemon.mac.properties";
        if (!wxFile::Exists(initFileName)) {
            if (fromMain) {
                return;
            } else if (wxFile::Exists(resourceName)) {
                initFileName = resourceName;
            }
        }
        loggingInitialised = true;

#endif
#ifdef __LINUX__
        std::string initFileName = wxStandardPaths::Get().GetInstallPrefix() + "/bin/xSMSDaemon.linux.properties";
        if (!wxFile::Exists(initFileName)) {
            initFileName = wxStandardPaths::Get().GetInstallPrefix() + "/share/xLights/xSMSDaemon.linux.properties";
        }
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

xSMSDaemonFrame *topFrame = nullptr;

#ifndef __WXMSW__
#include <execinfo.h>
#else
#include "../xLights/MSWStackWalk.h"
#endif

void handleCrash(void *data) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.crit("Crash handler called.");
    wxDebugReportCompress *report = new wxDebugReportCompress();

#ifndef __WXMSW__
    // dont call these for windows as they dont seem to do anything.
    report->AddAll(wxDebugReport::Context_Exception);
    //report->AddAll(wxDebugReport::Context_Current);
#endif

    wxString trace = wxString::Format("xSMSDaemon version %s\n\n", GetDisplayVersionString());

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
    trace += wxString::Format("\nCrashed thread id: 0x%X or %d\n", id, id);

    logger_base.crit(trace);

    report->AddText("backtrace.txt", trace, "Backtrace");

    wxString dir;
#ifdef __WXMSW__
    wxGetEnv("APPDATA", &dir);
    std::string filename = std::string(dir.c_str()) + "/xSMSDaemon_l4cpp.log";
#endif
#ifdef __WXOSX_MAC__
    wxFileName home;
    home.AssignHomeDir();
    dir = home.GetFullPath();
    std::string filename = std::string(dir.c_str()) + "/Library/Logs/xSMSDaemon_l4cpp.log";
#endif
#ifdef __LINUX__
    std::string filename = "/tmp/xSMSDaemon_l4cpp.log";
#endif

    if (wxFile::Exists(filename))
    {
        report->AddFile(filename, "xSMSDaemon_l4cpp.log");
    }
    else if (wxFile::Exists(wxFileName(wxGetCwd(), "xSMSDaemon_l4cpp.log").GetFullPath()))
    {
        report->AddFile(wxFileName(wxGetCwd(), "xSMSDaemon_l4cpp.log").GetFullPath(), "xSMSDaemon_l4cpp.log");
    }

    if (!wxThread::IsMain() && topFrame != nullptr) {
        topFrame->CallAfter(&xSMSDaemonFrame::CreateDebugReport, report);
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

void xSMSDaemonApp::WipeSettings()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.warn("------ Wiping settings ------");

    wxConfigBase* config = wxConfigBase::Get();
    config->DeleteAll();
}

int xSMSDaemonApp::OnExit()
{
    if (_checker != nullptr)
    {
        delete _checker;
        _checker = nullptr;
    }

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("xSMSDaemon exiting.");

    return 0;
}

bool xSMSDaemonApp::OnInit()
{
    _checker = nullptr;

    // seed the random number generator
    srand(wxGetLocalTimeMillis().GetLo());

    wxLog::SetLogLevel(wxLOG_FatalError);

#ifdef _MSC_VER
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

#if wxUSE_ON_FATAL_EXCEPTION
    wxHandleFatalExceptions();
#else
    SetUnhandledExceptionFilter(windows_exception_handler);
#endif

    InitialiseLogging(false);
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("******* OnInit: xSMSDaemon started.");

    DumpConfig();

    //curl_global_init(CURL_GLOBAL_SSL);

    static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
        { wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        { wxCMD_LINE_SWITCH, "w", "wipe", "wipe settings clean" },
        { wxCMD_LINE_NONE }
    };

    bool parmfound = false;
    wxString showDir;
    wxString playlist;
    wxCmdLineParser parser(cmdLineDesc, argc, argv);
    switch (parser.Parse()) {
    case -1:
        // help was given
        return false;
    case 0:
        if (parser.Found("w"))
        {
            parmfound = true;
            logger_base.info("-w: Wiping settings");
            WipeSettings();
        }
        if (!parmfound && parser.GetParamCount() > 0)
        {
            logger_base.info("Unrecognised command line parameter found.");
            wxMessageBox("Unrecognised command line parameter found.", _("Command Line Options")); //give positive feedback*/
        }
        break;
    default:
        wxMessageBox(_("Unrecognized command line parameters"), _("Command Line Error"));
        return false;
    }

    _checker = new wxSingleInstanceChecker();
    if (showDir == "")
    {
        if (_checker->IsAnotherRunning())
        {
            logger_base.info("Another instance of xSMSDaemon is running.");
            delete _checker; // OnExit() won't be called if we return false
            _checker = nullptr;

            // WOuld be nice to switch focuse here to the existing instance ... but that doesnt work ... this only sees windows in this process
            //wxWindow* x = FindWindowByLabel(_("xLights Scheduler"));

            wxMessageBox("Another instance of xSMSDaemon is already running. A second instance not allowed. Exiting.");

            return false;
        }
    }
    else
    {
        _checker->CreateDefault();
    }

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	xSMSDaemonFrame* Frame = new xSMSDaemonFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;
}

// CODE COPIED FROM XLIGHTS TO DUMP STACK TRACES

void xSMSDaemonApp::OnFatalException() {
    handleCrash(nullptr);
}
