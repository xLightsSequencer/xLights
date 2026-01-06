/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xScheduleApp.h"

//(*AppHeaders
#include "xScheduleMain.h"
#include <wx/image.h>
//*)

#include "spdlog/spdlog.h"
#include "spdlog/common.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include <wx/file.h>
#include <wx/msgdlg.h>

#include "ScheduleManager.h"
#include "../xLights/outputs/OutputManager.h"
#include "../xLights/xLightsVersion.h"
#include "../xLights/UtilFunctions.h"
#include <wx/cmdline.h>
#include <wx/confbase.h>
#include <wx/debugrpt.h>
#include <wx/filename.h>
#include <wx/snglinst.h>
#include <wx/stdpaths.h>

#ifdef _MSC_VER

    #pragma warning(push)
    #pragma warning(disable : 4081)

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
        #pragma comment(lib, "wxwebpd.lib")
        #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_qa.lib")
        #pragma comment(lib, "wxexpatd.lib")
        #pragma comment(lib, "portmidid.lib")
        #pragma comment(lib, "msvcprtd.lib")
        #pragma comment(lib, "libzstdd_static_VS.lib")

        #if !defined(SKIP_SMPTE)
            #pragma comment(lib, "libltcd.lib")
        #endif

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
        #pragma comment(lib, "wxwebp.lib")
        #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_qa.lib")
        #pragma comment(lib, "wxexpat.lib")
        #pragma comment(lib, "libzstd_static_VS.lib")
        #pragma comment(lib, "portmidi.lib")
        #pragma comment(lib, "msvcprt.lib")
        #if !defined(SKIP_SMPTE)
            #pragma comment(lib, "libltc.lib")
        #endif
    #endif

    #pragma comment(lib, "libcurl.dll.a")
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
    #pragma comment(lib, "avdevice.lib")
    #pragma comment(lib, "avfilter.lib")
    #pragma comment(lib, "avformat.lib")
    #pragma comment(lib, "avutil.lib")
    #pragma comment(lib, "postproc.lib")
    #pragma comment(lib, "swresample.lib")
    #pragma comment(lib, "swscale.lib")
    #pragma comment(lib, "SDL2.lib")

    #pragma warning(pop)

#endif

IMPLEMENT_APP(xScheduleApp)

xScheduleApp::xScheduleApp() :
    xlBaseApp("xSchedule")
{
}

std::string DecodeOS(wxOperatingSystemId o)
{
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
    spdlog::info("Version: " + std::string(xlights_version_string.c_str()));
    spdlog::info("Bits: " + std::string(GetBitness().c_str()));
    spdlog::info("Build Date: " + std::string(xlights_build_date.c_str()));
    spdlog::info("Machine configuration:");
    wxMemorySize s = wxGetFreeMemory();
    if (s != -1) {
#if wxUSE_LONGLONG
        wxString msg = wxString::Format(_T("  Free Memory: %" wxLongLongFmtSpec "d."), s);
#else
        wxString msg = wxString::Format(_T("  Free Memory: %ld."), s);
#endif
        spdlog::info(msg.ToStdString());
    }
    spdlog::info("  Current directory: " + std::string(wxGetCwd().c_str()));
    spdlog::info("  Machine name: " + std::string(wxGetHostName().c_str()));
    spdlog::info("  OS: " + std::string(wxGetOsDescription().c_str()));
    int verMaj = -1;
    int verMin = -1;
    wxOperatingSystemId o = wxGetOsVersion(&verMaj, &verMin);
    spdlog::info("  OS: {} {}.{}", DecodeOS(o), verMaj, verMin);
    if (wxIsPlatform64Bit()) {
        spdlog::info("      64 bit");
    } else {
        spdlog::info("      NOT 64 bit");
    }
    if (wxIsPlatformLittleEndian()) {
        spdlog::info("      Little Endian");
    } else {
        spdlog::info("      Big Endian");
    }
#ifdef LINUX
    wxLinuxDistributionInfo l = wxGetLinuxDistributionInfo();
    spdlog::info("  " + std::string(l.Id.c_str()) + " " + std::string(l.Release.c_str()) + " " + std::string(l.CodeName.c_str()) + " " + std::string(l.Description.c_str()));
#endif
}

void InitialiseLogging(bool fromMain)
{
    static bool loggingInitialised = false;

    if (!loggingInitialised) {

        std::string const logFileName = "xSchedule_spdlog.log";
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

        auto file_logger = std::make_shared<spdlog::logger>("xSchedule", rotating_file_sink);
        auto curl_logger = std::make_shared<spdlog::logger>("curl", rotating_file_sink);
        auto frame_logger = std::make_shared<spdlog::logger>("frame", rotating_file_sink);
        auto sync_logger = std::make_shared<spdlog::logger>("sync", rotating_file_sink);
        spdlog::register_logger(curl_logger);
        spdlog::register_logger(frame_logger);
        spdlog::register_logger(sync_logger);

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

        spdlog::info("Start Time: {}.", ts.ToStdString());
        spdlog::info("Current working directory {}.", wxGetCwd().ToStdString());

    }
}

void xScheduleApp::WipeSettings()
{
    spdlog::warn("------ Wiping settings ------");

    wxConfigBase* config = wxConfigBase::Get();
    config->DeleteAll();
}

int xScheduleApp::OnExit()
{
    if (_checker != nullptr) {
        delete _checker;
        _checker = nullptr;
    }
    spdlog::info("xSchedule exiting.");

    return 0;
}

bool xScheduleApp::OnInit()
{
    _checker = nullptr;

    // seed the random number generator
    srand(wxGetLocalTimeMillis().GetLo());

    wxLog::SetLogLevel(wxLOG_FatalError);

    // curl_global_init(CURL_GLOBAL_SSL);

    InitialiseLogging(false);
    spdlog::info("******* OnInit: xSchedule started.");

#ifdef __WXMSW__
    spdlog::debug("xSchedule module handle 0x%llx",  static_cast<void*>(::GetModuleHandle(nullptr)));
    spdlog::debug("xSchedule wxTheApp 0x%llx", fmt::ptr(wxTheApp));
    if (!IsSuppressDarkMode()) {
        MSWEnableDarkMode();
    }
#endif

    DumpConfig();

    static const wxCmdLineEntryDesc cmdLineDesc[] = {
        { wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        { wxCMD_LINE_OPTION, "s", "show", "specify show directory" },
        { wxCMD_LINE_OPTION, "p", "playlist", "specify the playlist to play" },
        { wxCMD_LINE_SWITCH, "w", "wipe", "wipe settings clean" },
        { wxCMD_LINE_SWITCH, "x", "exit", "exit silently if another instance is runnning" },
        { wxCMD_LINE_NONE }
    };

    bool parmfound = false;
    bool wipeSettings = false;
    bool exitIfRunning = false;
    wxString showDir;
    wxString playlist;
    wxCmdLineParser parser(cmdLineDesc, argc, argv);
    switch (parser.Parse()) {
    case -1:
        // help was given
        return false;
    case 0:
        if (parser.Found("w")) {
            parmfound = true;
            spdlog::info("-w: Wiping settings.");
            WipeSettings();
            wipeSettings = true;
        }
        if (parser.Found("x")) {
            spdlog::info("-x: Exit silently if another instance is running.");
            exitIfRunning = true;
        }
        if (parser.Found("s", &showDir)) {
            parmfound = true;
            spdlog::info("-s: Show directory set to %s.", (const char*)showDir.c_str());
        }
        if (parser.Found("p", &playlist)) {
            parmfound = true;
            spdlog::info("-p: Playlist to play %s.", (const char*)playlist.c_str());
        }
        if (!parmfound && parser.GetParamCount() > 0) {
            spdlog::info("Unrecognised command line parameter found.");
            wxMessageBox("Unrecognised command line parameter found.", _("Command Line Options")); // give positive feedback*/
        }
        break;
    default:
        wxMessageBox(_("Unrecognized command line parameters"), _("Command Line Error"));
        return false;
    }

    _checker = new wxSingleInstanceChecker();
    if (showDir == "") {
        if (_checker->IsAnotherRunning()) {
            spdlog::info("Another instance of xSchedule is running. Exiting.");
            delete _checker; // OnExit() won't be called if we return false
            _checker = nullptr;

            // WOuld be nice to switch focuse here to the existing instance ... but that doesnt work ... this only sees windows in this process
            // wxWindow* x = FindWindowByLabel(_("xLights Scheduler"));

            if (!exitIfRunning) {
                wxMessageBox("Another instance of xSchedule is already running. A second instance not allowed. Exiting.");
            }

            return false;
        }
    } else {
        _checker->CreateDefault();
    }

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if (wxsOK) {
        xScheduleFrame* Frame = new xScheduleFrame(0, showDir, playlist);
        Frame->Show();
        SetTopWindow(Frame);
        if (wipeSettings)
            Frame->GetPluginManager().WipeSettings();
    }
    //*)
    return wxsOK;
}
