/***************************************************************
 * Name:      xCaptureApp.cpp
 * Purpose:   Code for Application Class
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#include "xCaptureApp.h"

//(*AppHeaders
#include "xCaptureMain.h"
#include <wx/image.h>
//*)

#define USE_SPDLOG 1

#ifdef USE_SPDLOG
#include "spdlog/spdlog.h"
#include "spdlog/common.h"
#include "spdlog/sinks/rotating_file_sink.h"

#else

#include <log4cpp/Configurator.hh>
#include <log4cpp/PropertyConfigurator.hh>

#define LOG_INFO LOG_INFO
#define LOG_ERROR LOG_ERROR
#endif

#include <wx/file.h>
#include <wx/msgdlg.h>

#include "../xLights/xLightsVersion.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/debugrpt.h>
#include <wx/cmdline.h>
#include <wx/confbase.h>

#ifdef _MSC_VER
    #ifdef _DEBUG
        #pragma comment(lib, "wxbase"WXWIDGETS_VERSION"ud.lib")
        #pragma comment(lib, "wxbase"WXWIDGETS_VERSION"ud_net.lib")
        #pragma comment(lib, "wxmsw"WXWIDGETS_VERSION"ud_core.lib")
        #pragma comment(lib, "wxscintillad.lib")
        #pragma comment(lib, "wxregexud.lib")
        #pragma comment(lib, "wxbase"WXWIDGETS_VERSION"ud_xml.lib")
        #pragma comment(lib, "wxtiffd.lib")
        #pragma comment(lib, "wxjpegd.lib")
        #pragma comment(lib, "wxpngd.lib")
        #pragma comment(lib, "wxzlibd.lib")
        #pragma comment(lib, "wxmsw"WXWIDGETS_VERSION"ud_qa.lib")
        #pragma comment(lib, "wxexpatd.lib")
        #pragma comment(lib, "msvcprtd.lib")
        #pragma comment(lib, "wxwebpd.lib")
    #else
        #pragma comment(lib, "wxbase"WXWIDGETS_VERSION"u.lib")
        #pragma comment(lib, "wxbase"WXWIDGETS_VERSION"u_net.lib")
        #pragma comment(lib, "wxmsw"WXWIDGETS_VERSION"u_core.lib")
        #pragma comment(lib, "wxscintilla.lib")
        #pragma comment(lib, "wxregexu.lib")
        #pragma comment(lib, "wxbase"WXWIDGETS_VERSION"u_xml.lib")
        #pragma comment(lib, "wxtiff.lib")
        #pragma comment(lib, "wxjpeg.lib")
        #pragma comment(lib, "wxpng.lib")
        #pragma comment(lib, "wxzlib.lib")
        #pragma comment(lib, "wxmsw"WXWIDGETS_VERSION"u_qa.lib")
        #pragma comment(lib, "wxexpat.lib")
        #pragma comment(lib, "msvcprt.lib")
        #pragma comment(lib, "wxwebp.lib")
    #endif  
    #pragma comment(lib, "libcurl.dll.a")
    #pragma comment(lib, "ImageHlp.Lib")
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
#endif

IMPLEMENT_APP(xCaptureApp)

xCaptureApp::xCaptureApp() :
    xlBaseApp("xCapture")
{
}

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
    spdlog::info("Version: " + std::string(xlights_version_string.c_str()));
    spdlog::info("Bits: " + std::string(GetBitness().c_str()));
    spdlog::info("Build Date: " + std::string(xlights_build_date.c_str()));
    spdlog::info("Machine configuration:");
    wxMemorySize s = wxGetFreeMemory();
    if (s != -1)
    {
#if wxUSE_LONGLONG
        wxString msg = wxString::Format(_T("  Free Memory: %" wxLongLongFmtSpec "d."), s);
#else
        wxString msg = wxString::Format(_T("  Free Memory: %ld."), s);
#endif
        spdlog::info( msg.ToStdString());
    }
    spdlog::info("  Current directory: " + std::string(wxGetCwd().c_str()));
    spdlog::info("  Machine name: " + std::string(wxGetHostName().c_str()));
    spdlog::info("  OS: " + std::string(wxGetOsDescription().c_str()));
    int verMaj = -1;
    int verMin = -1;
    wxOperatingSystemId o = wxGetOsVersion(&verMaj, &verMin);
    spdlog::info("  OS: {} {}.{}", DecodeOS(o), verMaj, verMin);
    if (wxIsPlatform64Bit())
    {
        spdlog::info("      64 bit");
    }
    else
    {
        spdlog::info("      NOT 64 bit");
    }
    if (wxIsPlatformLittleEndian())
    {
        spdlog::info("      Little Endian");
    }
    else
    {
        spdlog::info("      Big Endian");
    }
#ifdef LINUX
    wxLinuxDistributionInfo l = wxGetLinuxDistributionInfo();
    spdlog::info("  " + std::string(l.Id.c_str()) \
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
#ifdef USE_SPDLOG

        std::string const logFileName = "xCapture_spdlog.log";
#ifdef __WXMSW__
        wxString dir;
        wxGetEnv("APPDATA", &dir);
        std::string const logFilePath = std::string(dir.c_str()) + "\\" + logFileName;
#endif
#ifdef __WXOSX__
        wxFileName home;
        home.AssignHomeDir();
        wxString const dir = home.GetFullPath();
        std::string const logFilePath = std::string(dir.c_str()) + "/Library/Logs/" + logFileName;
#endif
#ifdef __LINUX__
        std::string const logFilePath = "/tmp/" + logFileName;
#endif

        // wxStandardPaths::Get().Get()

        auto rotating_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFilePath, 1024 * 1024 * 10, 10);

        auto file_logger = std::make_shared<spdlog::logger>("xCapture", rotating_file_sink);
        auto curl_logger = std::make_shared<spdlog::logger>("curl", rotating_file_sink);

        spdlog::register_logger(curl_logger);

        loggingInitialised = true;
        spdlog::initialize_logger(file_logger);
        spdlog::set_default_logger(file_logger);
        spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] %v");
        spdlog::flush_on(spdlog::level::info);
        // wxOperatingSystemId os = wxGetOsVersion();
        // std::string osStr = DecodeOS(os);
        // std::string initFileName;

        wxDateTime now = wxDateTime::Now();
        int millis = wxGetUTCTimeMillis().GetLo() % 1000;
        wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(), now.GetMonth() + 1, now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond(), millis);
        // spdlog::info("Start Time: {}.", ts);

        //auto tt = fmt::sprintf("Start Time: %s.", (const char*)ts.c_str());
        spdlog::info("Start Time: {}.", ts.ToStdString());
        spdlog::info("Current working directory {}.", wxGetCwd().ToStdString());
#else

#ifdef __WXMSW__
        std::string initFileName = "xcapture.windows.properties";
#endif
#ifdef __WXOSX__
        std::string initFileName = "xcapture.mac.properties";
        std::string resourceName = wxStandardPaths::Get().GetResourcesDir().ToStdString() + "/xcapture.mac.properties";
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
        std::string initFileName = wxStandardPaths::Get().GetInstallPrefix() + "/bin/xcapture.linux.properties";
        if (!wxFile::Exists(initFileName)) {
            initFileName = wxStandardPaths::Get().GetInstallPrefix() + "/share/xLights/xcapture.linux.properties";
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
#endif
    }
}

void xCaptureApp::WipeSettings()
{
    spdlog::warn("------ Wiping settings ------");
    wxConfigBase* config = wxConfigBase::Get();
    config->DeleteAll();
}

int xCaptureApp::OnExit()
{
    spdlog::info("xCapture exiting.");
    return 0;
}

bool xCaptureApp::OnInit()
{
    wxLog::SetLogLevel(wxLOG_FatalError);
    SetAppName("xCapture");
#ifdef _MSC_VER
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

    InitialiseLogging(false);
    spdlog::info("******* OnInit: xCapture started.");

    DumpConfig();
#ifdef __WXMSW__
    MSWEnableDarkMode();
#endif
    static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
        { wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        { wxCMD_LINE_OPTION, "s", "show", "specify show directory" },
        { wxCMD_LINE_OPTION, "p", "playlist", "specify the playlist to play" },
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
            spdlog::info("-w: Wiping settings");
            WipeSettings();
        }
        if (!parmfound && parser.GetParamCount() > 0)
        {
            spdlog::info("Unrecognised command line parameter found.");
            wxMessageBox("Unrecognised command line parameter found.", _("Command Line Options")); //give positive feedback*/
        }
        break;
    default:
        wxMessageBox(_("Unrecognized command line parameters"), _("Command Line Error"));
        return false;
    }

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	xCaptureFrame* Frame = new xCaptureFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;
}
