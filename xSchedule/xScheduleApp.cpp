/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
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
#include <wx/cmdline.h>
#include <wx/confbase.h>
#include <wx/snglinst.h>

#ifdef _MSC_VER
#ifdef _DEBUG
    #pragma comment(lib, "wxbase31ud.lib")
    #pragma comment(lib, "wxbase31ud_net.lib")
    #pragma comment(lib, "wxmsw31ud_core.lib")
    #pragma comment(lib, "wxscintillad.lib")
    #pragma comment(lib, "wxregexud.lib")
    #pragma comment(lib, "wxbase31ud_xml.lib")
    #pragma comment(lib, "wxtiffd.lib")
    #pragma comment(lib, "wxjpegd.lib")
    #pragma comment(lib, "wxpngd.lib")
    #pragma comment(lib, "wxzlibd.lib")
    #pragma comment(lib, "wxmsw31ud_qa.lib")
    #pragma comment(lib, "wxexpatd.lib")
    #pragma comment(lib, "log4cpplibd.lib")
    //#pragma comment(lib, "log4cppd.lib")
    #pragma comment(lib, "portmidid.lib")
    #pragma comment(lib, "msvcprtd.lib")
    #pragma comment(lib, "libzstdd_static_VS.lib")
    #pragma comment(lib, "libltcd.lib")
#else
    #pragma comment(lib, "wxbase31u.lib")
    #pragma comment(lib, "wxbase31u_net.lib")
    #pragma comment(lib, "wxmsw31u_core.lib")
    #pragma comment(lib, "wxscintilla.lib")
    #pragma comment(lib, "wxregexu.lib")
    #pragma comment(lib, "wxbase31u_xml.lib")
    #pragma comment(lib, "wxtiff.lib")
    #pragma comment(lib, "wxjpeg.lib")
    #pragma comment(lib, "wxpng.lib")
    #pragma comment(lib, "wxzlib.lib")
    #pragma comment(lib, "wxmsw31u_qa.lib")
    #pragma comment(lib, "wxexpat.lib")
    #pragma comment(lib, "libzstd_static_VS.lib")
    #pragma comment(lib, "log4cpplib.lib")
    //#pragma comment(lib, "log4cpp.lib")
    #pragma comment(lib, "portmidi.lib")
    #pragma comment(lib, "msvcprt.lib")
    #pragma comment(lib, "libltc.lib")
#endif
#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "z.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "WS2_32.Lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "odbc32.lib") 
#pragma comment(lib, "odbccp32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ImageHlp.Lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "swscale.lib")
#endif

IMPLEMENT_APP(xScheduleApp)

std::string DecodeOS(wxOperatingSystemId o)  {
    switch (o) {
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
        std::string initFileName = "xschedule.windows.properties";
#endif
#ifdef __WXOSX_MAC__
        std::string initFileName = "xschedule.mac.properties";
        std::string resourceName = wxStandardPaths::Get().GetResourcesDir().ToStdString() + "/xschedule.mac.properties";
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
        std::string initFileName = wxStandardPaths::Get().GetInstallPrefix() + "/bin/xschedule.linux.properties";
        if (!wxFile::Exists(initFileName)) {
            initFileName = wxStandardPaths::Get().GetInstallPrefix() + "/share/xLights/xschedule.linux.properties";
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
                static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

                wxDateTime now = wxDateTime::Now();
                int millis = wxGetUTCTimeMillis().GetLo() % 1000;
                wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(), now.GetMonth(), now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond(), millis);
                logger_base.info("Start Time: %s.", (const char*)ts.c_str());

                logger_base.info("Current Working Directory: " + wxGetCwd());
                logger_base.info("Log4cpp config read from %s.", (const char*)initFileName.c_str());

                auto categories = log4cpp::Category::getCurrentCategories();
                for (const auto& it : *categories)
                {
                    std::string apps = "";
                    auto appenders = it->getAllAppenders();
                    for (const auto& it2 : appenders)
                    {
                        if (apps != "") apps += ", ";
                        apps += it2->getName();
                    }

                    std::string levels = "";
                    if (it->isAlertEnabled()) levels += "ALERT ";
                    if (it->isCritEnabled()) levels += "CRIT ";
                    if (it->isDebugEnabled()) levels += "DEBUG ";
                    if (it->isEmergEnabled()) levels += "EMERG ";
                    if (it->isErrorEnabled()) levels += "ERROR ";
                    if (it->isFatalEnabled()) levels += "FATAL ";
                    if (it->isInfoEnabled()) levels += "INFO ";
                    if (it->isNoticeEnabled()) levels += "NOTICE ";
                    if (it->isWarnEnabled()) levels += "WARN ";

                    logger_base.info("    %s : %s", (const char*)it->getName().c_str(), (const char*)levels.c_str());
                    if (apps != "")
                    {
                        logger_base.info("         " + apps);
                    }
                }
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
#include "../xLights/MSWStackWalk.h"
#endif

void handleCrash(void *data) {

    // if we crash while in here we dont want to report again.
    static bool reentry = false;
    if (reentry) return;
    reentry = true;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.crit("Crash handler called.");
    wxDebugReportCompress *report = new wxDebugReportCompress();
    if (xScheduleFrame::GetScheduleManager() != nullptr)
    {
        report->SetCompressedFileDirectory(xScheduleFrame::GetScheduleManager()->GetShowDir());
    }

#ifndef __WXMSW__
    // dont call these for windows as they dont seem to do anything.
    report->AddAll(wxDebugReport::Context_Exception);
    //report->AddAll(wxDebugReport::Context_Current);
#endif

    if (xScheduleFrame::GetScheduleManager() != nullptr)
    {
        wxFileName fn(xScheduleFrame::GetScheduleManager()->GetShowDir(), OutputManager::GetNetworksFileName());
        if (fn.Exists()) {
            report->AddFile(fn.GetFullPath(), OutputManager::GetNetworksFileName());
        }

        if (wxFileName(xScheduleFrame::GetScheduleManager()->GetShowDir(), ScheduleManager::GetScheduleFile()).Exists()) {
            report->AddFile(wxFileName(xScheduleFrame::GetScheduleManager()->GetShowDir(), ScheduleManager::GetScheduleFile()).GetFullPath(), ScheduleManager::GetScheduleFile());
        }
    }

    wxString trace = wxString::Format("xSchedule version %s\n\n", GetDisplayVersionString());

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

    if (xScheduleFrame::GetScheduleManager() != nullptr)
    {
        xScheduleFrame::GetScheduleManager()->CheckScheduleIntegrity(false);
    }

    if (!wxThread::IsMain() && topFrame != nullptr) {
        topFrame->CallAfter(&xScheduleFrame::CreateDebugReport, report);
        wxSleep(600000);
    }
    else {
        topFrame->CreateDebugReport(report);
    }

    reentry = false;
}

#if !(wxUSE_ON_FATAL_EXCEPTION)
#include <windows.h>
//MinGW needs to do this manually
LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS * ExceptionInfo)
{
    handleCrash(ExceptionInfo->ContextRecord);
	return 0;
}
#endif

void xScheduleApp::WipeSettings()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.warn("------ Wiping settings ------");

    wxConfigBase* config = wxConfigBase::Get();
    config->DeleteAll();
}

int xScheduleApp::OnExit()
{
    if (_checker != nullptr)
    {
        delete _checker;
        _checker = nullptr;
    }

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("xSchedule exiting.");

    return 0;
}

bool xScheduleApp::OnInit()
{
    _checker = nullptr;

    // seed the random number generator
    srand(wxGetLocalTimeMillis().GetLo());

    wxLog::SetLogLevel(wxLOG_FatalError);

#if wxUSE_ON_FATAL_EXCEPTION
    #if !defined(_DEBUG) || !defined(_MSC_VER)
        wxHandleFatalExceptions();
    #endif
#else
    SetUnhandledExceptionFilter(windows_exception_handler);
#endif

    //curl_global_init(CURL_GLOBAL_SSL);

    InitialiseLogging(false);
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("******* OnInit: xSchedule started.");

#ifdef __WXMSW__
    logger_base.debug("xSchedule module handle 0x%llx", ::GetModuleHandle(nullptr));
    logger_base.debug("xSchedule wxTheApp 0x%llx", wxTheApp);
#endif

    DumpConfig();

    static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
        { wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        { wxCMD_LINE_OPTION, "s", "show", "specify show directory" },
        { wxCMD_LINE_OPTION, "p", "playlist", "specify the playlist to play" },
        { wxCMD_LINE_SWITCH, "w", "wipe", "wipe settings clean" },
        { wxCMD_LINE_NONE }
    };

    bool parmfound = false;
    bool wipeSettings = false;
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
            wipeSettings = true;
        }
        if (parser.Found("s", &showDir)) {
            parmfound = true;
            logger_base.info("-s: Show directory set to %s.", (const char*)showDir.c_str());
        }
        if (parser.Found("p", &playlist)) {
            parmfound = true;
            logger_base.info("-p: Playlist to play %s.", (const char*)playlist.c_str());
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
            logger_base.info("Another instance of xSchedule is running.");
            delete _checker; // OnExit() won't be called if we return false
            _checker = nullptr;

            // WOuld be nice to switch focuse here to the existing instance ... but that doesnt work ... this only sees windows in this process
            //wxWindow* x = FindWindowByLabel(_("xLights Scheduler"));

            wxMessageBox("Another instance of xSchedule is already running. A second instance not allowed. Exiting.");

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
    if (wxsOK)
    {
        xScheduleFrame* Frame = new xScheduleFrame(0, showDir, playlist);
        Frame->Show();
        SetTopWindow(Frame);
        if (wipeSettings) Frame->GetPluginManager().WipeSettings();
    }
    //*)
    return wxsOK;
}

// CODE COPIED FROM XLIGHTS TO DUMP STACK TRACES

void xScheduleApp::OnFatalException() {
    handleCrash(nullptr);
}

