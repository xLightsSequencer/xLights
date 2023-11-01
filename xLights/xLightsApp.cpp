/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#ifdef _DEBUG
//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#endif

//(*AppHeaders
#include "xLightsMain.h"
#include <wx/image.h>
//*)

#include <wx/stdpaths.h>
#include <wx/config.h>
#include <wx/cmdline.h>
#include <wx/debugrpt.h>
#include <wx/version.h>
#include <wx/dirdlg.h>

#include <stdlib.h>     /* srand */
#include <time.h>       /* time */
#include <thread>
#include <iomanip>
#include <curl/curl.h>

#include "xLightsApp.h"
#include "xLightsVersion.h"
#include "UtilFunctions.h"
#include "TraceLog.h"
#include "ExternalHooks.h"
#include "BitmapCache.h"
#include "utils/CurlManager.h"

#ifndef __WXMSW__
#include "automation/automation.h"
#endif

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Configurator.hh>

#ifdef LINUX
#include <GL/glut.h>
#endif

#ifdef _MSC_VER
#ifdef _DEBUG
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "ud.lib")
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "ud_net.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_core.lib")
    #pragma comment(lib, "wxscintillad.lib")
    #pragma comment(lib, "wxregexud.lib")
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "ud_xml.lib")
    #pragma comment(lib, "wxtiffd.lib")
    #pragma comment(lib, "wxjpegd.lib")
    #pragma comment(lib, "wxpngd.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_aui.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_gl.lib")
    #pragma comment(lib, "wxzlibd.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_qa.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_html.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_propgrid.lib")
    #pragma comment(lib, "wxexpatd.lib")
    #pragma comment(lib, "log4cppLIBd.lib")
    #pragma comment(lib, "msvcprtd.lib")
    #pragma comment(lib, "liquidfund.lib")
    #pragma comment(lib, "libzstdd_static_VS.lib")
    #pragma comment(lib, "xlsxwriterd.lib")
#else
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "u.lib")
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "u_net.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_core.lib")
    #pragma comment(lib, "wxscintilla.lib")
    #pragma comment(lib, "wxregexu.lib")
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "u_xml.lib")
    #pragma comment(lib, "wxtiff.lib")
    #pragma comment(lib, "wxjpeg.lib")
    #pragma comment(lib, "wxpng.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_aui.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_gl.lib")
    #pragma comment(lib, "wxzlib.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_qa.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_html.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_propgrid.lib")
    #pragma comment(lib, "wxexpat.lib")
    #pragma comment(lib, "log4cppLIB.lib")
    #pragma comment(lib, "msvcprt.lib")
    #pragma comment(lib, "liquidfun.lib")
    #pragma comment(lib, "libzstd_static_VS.lib")
    #pragma comment(lib, "xlsxwriter.lib")
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
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "z.lib")
#pragma comment(lib, "lua5.3.5-static.lib")
#pragma comment(lib, "libwebp.lib")
#pragma comment(lib, "libwebpdecoder.lib")
#pragma comment(lib, "libwebpdemux.lib")
#endif

xLightsFrame* xLightsApp::__frame = nullptr;

void InitialiseLogging(bool fromMain)
{
    static bool loggingInitialised = false;

    if (!loggingInitialised) {
#ifdef __WXMSW__
        std::string initFileName = "xlights.windows.properties";
#endif
#ifdef __WXOSX__
        std::string initFileName = "xlights.mac.properties";
        if (!FileExists(initFileName)) {
            if (fromMain) {
                return;
            } else if (FileExists(wxStandardPaths::Get().GetResourcesDir() + "/xlights.mac.properties")) {
                initFileName = wxStandardPaths::Get().GetResourcesDir() + "/xlights.mac.properties";
            }
        }
        loggingInitialised = true;
        //make sure the default logging location is actually created
        std::string ld = std::getenv("HOME");
        ld += "/Library/Logs/";
        wxDir logDir(ld);
        if (!wxDir::Exists(ld)) {
            wxDir::Make(ld);
        }
#endif
#ifdef __LINUX__
        std::string initFileName = wxStandardPaths::Get().GetInstallPrefix() + "/bin/xlights.linux.properties";
        if (!FileExists(initFileName)) {
            initFileName = wxStandardPaths::Get().GetInstallPrefix() + "/share/xLights/xlights.linux.properties";
        }
#endif

#ifdef _MSC_VER
        if (!FileExists(initFileName)) {
            wxFileName f(wxStandardPaths::Get().GetExecutablePath());
            wxString appPath(f.GetPath());
            initFileName = appPath + "\\" + initFileName;
        }
#endif

        if (!FileExists(initFileName)) {
#ifdef _MSC_VER
            // the app is not initialized so GUI is not available and no event loop.
            wxMessageBox(initFileName + " not found in " + wxGetCwd() + ". Logging disabled.");
#endif
        } else {
            try {
                log4cpp::PropertyConfigurator::configure(initFileName);
                static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

                wxDateTime now = wxDateTime::Now();
                int millis = wxGetUTCTimeMillis().GetLo() % 1000;
                wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(), now.GetMonth() + 1, now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond(), millis);
                logger_base.info("Start Time: %s.", (const char*)ts.c_str());

                logger_base.info("Log4CPP config read from %s.", (const char*)initFileName.c_str());
                logger_base.info("Current working directory %s.", (const char*)wxGetCwd().c_str());

                auto categories = log4cpp::Category::getCurrentCategories();

                for (auto it = categories->begin(); it != categories->end(); ++it) {
                    std::string levels = "";

                    if ((*it)->isAlertEnabled())
                        levels += "ALERT ";
                    if ((*it)->isCritEnabled())
                        levels += "CRIT ";
                    if ((*it)->isDebugEnabled())
                        levels += "DEBUG ";
                    if ((*it)->isEmergEnabled())
                        levels += "EMERG ";
                    if ((*it)->isErrorEnabled())
                        levels += "ERROR ";
                    if ((*it)->isFatalEnabled())
                        levels += "FATAL ";
                    if ((*it)->isInfoEnabled())
                        levels += "INFO ";
                    if ((*it)->isNoticeEnabled())
                        levels += "NOTICE ";
                    if ((*it)->isWarnEnabled())
                        levels += "WARN ";

                    logger_base.info("    %s : %s", (const char*)(*it)->getName().c_str(), (const char*)levels.c_str());
                }
                delete categories;
            } catch (log4cpp::ConfigureFailure& e) {
                // ignore config failure ... but logging wont work
                printf("Log issue:  %s\n", e.what());
            } catch (const std::exception& ex) {
                printf("Log issue: %s\n", ex.what());
            }
        }
    }
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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::string versionStr = "Version: " + xlights_version_string.c_str();
    if (IsFromAppStore()) {
        versionStr += " - App Store";
    }
    logger_base.info(versionStr);
    logger_base.info("Bits: " + std::string(GetBitness().c_str()));
    logger_base.info("Build Date: " + std::string(xlights_build_date.c_str()));
    logger_base.info("WX Version: " + std::string(wxString( wxVERSION_STRING).c_str()));

    logger_base.info("Machine configuration:");
    logger_base.info("  Total memory: " + std::to_string(GetPhysicalMemorySizeMB()) + " MB");
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
    logger_base.info("  CPU Arch: " + std::string(wxGetCpuArchitectureName().c_str()));

#ifdef LINUX
    wxLinuxDistributionInfo l = wxGetLinuxDistributionInfo();
    logger_base.info("  " + std::string(l.Id.c_str()) \
        + " " + std::string(l.Release.c_str()) \
        + " " + std::string(l.CodeName.c_str()) \
        + " " + std::string(l.Description.c_str()));
#endif
}

#ifdef LINUX
    #include <X11/Xlib.h>
#endif // LINUX
//IMPLEMENT_APP(xLightsApp)
int main(int argc, char **argv)
{
    srand(time(nullptr));
    InitialiseLogging(true);
    // Dan/Chris ... if you get an exception here the most likely reason is the line
    // appender.A1.fileName= in the xlights.xxx.properties file
    // it seems to need to be a folder that exists ... ideally it would create it but it doesnt seem to
    // it needs to be:
    //     a folder the user can write to
    //     predictable ... as we want the HandleCrash function to be able to locate the file to include it in the crash
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("******* XLights main function executing.");

#ifdef LINUX
    XInitThreads();
#endif
    wxDISABLE_DEBUG_SUPPORT();

    curl_global_init(CURL_GLOBAL_DEFAULT);

    #ifndef __WXMSW__
    wxApp::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "program");
    wxString binName(argv[0]);

    if (binName.EndsWith("xlDo")) {
        //symlink xlDo to xLights
        return DoXLDoCommands(argc, argv);
    } else if (argc > 1) {
        std::string argv1 = &argv[1][1];
        if (argv1 == "xlDo") {
            return DoXLDoCommands(argc - 1, &argv[1]);
        }
    }
    #endif

    logger_base.info("Main: Starting wxWidgets ...");
    int rc =  wxEntry(argc, argv);
    logger_base.info("Main: wxWidgets exited with rc=" + wxString::Format("%d", rc));
    return rc;
}

#ifdef _MSC_VER
IMPLEMENT_APP(xLightsApp);
#else
wxIMPLEMENT_APP_NO_MAIN(xLightsApp);
#endif

xLightsApp::xLightsApp() :
    xLightsAppBaseClass("xLights")
{
}

wxString xLightsFrame::GetThreadStatusReport() {
    return jobPool.GetThreadStatus();
}
void xLightsFrame::PushTraceContext() {
    TraceLog::PushTraceContext();
}
void xLightsFrame::PopTraceContext() {
    TraceLog::PopTraceContext();
}

void xLightsFrame::AddTraceMessage(const std::string &trc) {
    TraceLog::AddTraceMessage(trc);
}

void xLightsFrame::ClearTraceMessages() {
    TraceLog::ClearTraceMessages();
}


#ifdef __WXOSX__
void xLightsApp::MacOpenFiles(const wxArrayString &fileNames) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (fileNames.empty()) {
        return;
    }
    wxString fileName = fileNames[0];
    logger_base.info("******* MacOpenFiles: %s", (const char*)fileName.c_str());
    ObtainAccessToURL(fileName);

    wxString showDir = wxPathOnly(fileName);
    while (showDir != "" && !FileExists(showDir + "/" + "xlights_rgbeffects.xml")) {
        auto old = showDir;
        showDir = wxPathOnly(showDir);
        if (showDir == old) showDir = "";
    }
    
    if (__frame) {
        xLightsFrame* frame = __frame;
        frame->CallAfter([showDir, fileName, frame] {
            if (showDir != "" && showDir != frame->showDirectory) {
                wxString nsd = showDir;
                if (!ObtainAccessToURL(nsd)) {
                    wxDirDialog dlg(frame, "Select Show Directory", nsd,  wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
                    if (dlg.ShowModal() == wxID_OK) {
                        nsd = dlg.GetPath();
                    }
                    if (!ObtainAccessToURL(nsd)) {
                        return;
                    }
                }
                frame->SetDir(nsd, false);
            }
            frame->OpenSequence(fileName, nullptr);
        });
    } else {
        logger_base.info("       No xLightsFrame");
    }
}
#endif

bool xLightsApp::OnInit()
{
    InitialiseLogging(false);
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("******* OnInit: XLights started.");
#ifdef __WXMSW__
    if (!IsSuppressDarkMode()) {
        MSWEnableDarkMode();
    }
#endif
#if wxUSE_GLCANVAS_EGL
    // this is only needed if using the EGL canvas as it's necessary to initialize the
    // GL attributes and pixel formats.  Likely a bug in the EGL implementation,
    // but not really sure.   In anycase, it's not needed on any of our "normal"
    // platforms.  It's only needed if building on an EGL based Linux distribution.
    wxGLApp::OnInit();
#endif

    wxTheApp->SetAppName("xLights");
    DumpConfig();

    int id = (int)wxThread::GetCurrentId();
    logger_base.info("Main thread id: 0x%X or %i", id, id);

#ifdef _MSC_VER
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

//check for options on command line: -DJ
//TODO: maybe use wxCmdLineParser instead?
//do this before instantiating xLightsFrame so it can use info gathered here
    wxString unrecog, info;

    static const wxCmdLineEntryDesc cmdLineDesc [] =
    {
        { wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        { wxCMD_LINE_SWITCH, "r", "render", "render files and exit"},
        { wxCMD_LINE_SWITCH, "cs", "checksequence", "run check sequence and exit" },
        { wxCMD_LINE_OPTION, "m", "media", "specify media directory"},
        { wxCMD_LINE_OPTION, "s", "show", "specify show directory" },
        { wxCMD_LINE_SWITCH, "w", "wipe", "wipe settings clean" },
        { wxCMD_LINE_SWITCH, "o", "on", "turn on output to lights" },
        { wxCMD_LINE_SWITCH, "a", "aport", "turn on xFade A port" },
        { wxCMD_LINE_SWITCH, "b", "bport", "turn on xFade B port" },
#ifdef __LINUX__
        { wxCMD_LINE_SWITCH, "x", "xschedule", "run xschedule" },
        { wxCMD_LINE_SWITCH, "xs", "xsmsdaemon", "run xsmsdaemon" },
        { wxCMD_LINE_SWITCH, "c", "xcapture", "run xcapture" },
        { wxCMD_LINE_SWITCH, "f", "xfade", "run xfade" },
        { wxCMD_LINE_SWITCH, "n", "xscanner", "run xscanner" },
#endif
        { wxCMD_LINE_PARAM, "", "", "sequence file", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE},
        { wxCMD_LINE_NONE }
    };

// Add option to run xutils via xlights on linux (for AppImage usage)
#ifdef __LINUX__
       int run_xsmsdaemon = FALSE;
       int run_xschedule = FALSE;
       int run_xcapture = FALSE;
       int run_xfade = FALSE;
       int run_xscanner = FALSE;
       wxFileName f(wxStandardPaths::Get().GetExecutablePath());
       wxString appPath(f.GetPath());
       wxString cmdlineC(appPath + wxT("/xCapture"));
       wxString cmdlineS(appPath + wxT("/xSchedule"));
       wxString cmdlineF(appPath + wxT("/xFade"));
       wxString cmdlineM(appPath + wxT("/xSMSDaemon"));
       wxString cmdlineN(appPath + wxT("/xScanner"));
        for (int i=1; i< argc;i++) {
            if ((strncmp(argv[i].c_str(), "-xs", 3) == 0 && argv[i].size() == 3)|| strncmp(argv[i].c_str(), "-xsmsdaemon", 11) == 0) {
                run_xsmsdaemon = TRUE;
            } else if ((strncmp(argv[i].c_str(), "-x", 2) == 0 && argv[i].size() == 2) || strncmp(argv[i].c_str(), "-xschedule", 10) == 0) {
                run_xschedule = TRUE;
            } else if ((strncmp(argv[i].c_str(), "-c", 2) == 0 && argv[i].size() == 2) || strncmp(argv[i].c_str(), "-xcapture", 9) == 0) {
                run_xcapture = TRUE;
            } else if ((strncmp(argv[i].c_str(), "-f", 2) == 0 && argv[i].size() == 2) || strncmp(argv[i].c_str(), "-xfade", 6) == 0) {
                run_xfade = TRUE;
            } else if ((strncmp(argv[i].c_str(), "-n", 2) == 0 && argv[i].size() == 2) || strncmp(argv[i].c_str(), "-xscanner", 9) == 0) {
                run_xscanner = TRUE;
            } else {
                cmdlineS += wxT(" ");
                cmdlineS += wxString::FromUTF8(argv[i]);
                cmdlineC += wxT(" ");
                cmdlineC += wxString::FromUTF8(argv[i]);
                cmdlineF += wxT(" ");
                cmdlineF += wxString::FromUTF8(argv[i]);
                cmdlineM += wxT(" ");
                cmdlineM += wxString::FromUTF8(argv[i]);
                cmdlineN += wxT(" ");
                cmdlineN += wxString::FromUTF8(argv[i]);
            }
        }
        if (run_xschedule) {
            wxExecute(cmdlineS, wxEXEC_BLOCK,NULL,NULL);
            exit(0);
        } else if (run_xcapture) {
            wxExecute(cmdlineC, wxEXEC_BLOCK,NULL,NULL);
            exit(0);
        } else if (run_xfade) {
            wxExecute(cmdlineF, wxEXEC_BLOCK,NULL,NULL);
            exit(0);
        } else if (run_xsmsdaemon) {
            wxExecute(cmdlineM, wxEXEC_BLOCK,NULL,NULL);
            exit(0);
        } else if (run_xscanner) {
            wxExecute(cmdlineN, wxEXEC_BLOCK,NULL,NULL);
            exit(0);
        }
       // Set App Name for when running via appimage
       SetAppName(wxT("xLights"));
#endif

    int ab = 0;

    wxCmdLineParser parser(cmdLineDesc, argc, argv);
    switch (parser.Parse()) {
    case -1:
        // help was given
        return false;
    case 0:
        if (parser.Found("w"))
        {
            logger_base.info("-w: Wiping settings");
            info += _("Wiping settings\n");
            WipeSettings();
        }
        if (parser.Found("s", &showDir)) {
            logger_base.info("-s: Show directory set to %s.", (const char *)showDir.c_str());
            info += _("Setting show directory to ") + showDir + "\n";
        }

        if (parser.Found("a")) {
            logger_base.info("-a: A port enabled.");
            ab = 1;
        } else if (parser.Found("b")) {
            logger_base.info("-b: B port enabled.");
            ab = 2;
        }

        if (parser.Found("m", &mediaDir)) {
            logger_base.info("-m: Media directory set to %s.", (const char *)mediaDir.c_str());
            info += _("Setting media directory to ") + mediaDir + "\n";
        } else if (!showDir.IsNull()) {
            mediaDir = showDir;
        }
        for (size_t x = 0; x < parser.GetParamCount(); x++) {
            wxString sequenceFile = parser.GetParam(x);
            if (x == 0) {
                logger_base.info("Sequence file passed on command line: %s.", (const char *)sequenceFile.c_str());
                info += _("Loading sequence ") + sequenceFile + "\n";
            }
            if (showDir.IsNull()) {
                showDir=wxPathOnly(sequenceFile);
                while (showDir != "" && !FileExists(showDir + "/" + "xlights_rgbeffects.xml"))
                {
                    wxString old = showDir;
                    showDir = wxPathOnly(showDir);
                    if (showDir == old) showDir = "";
                }
            }
            sequenceFiles.push_back(sequenceFile);
        }
        if (!parser.Found("cs") && !parser.Found("r") && !parser.Found("o") && !info.empty())
        {
            DisplayInfo(info); //give positive feedback*/
        }
        break;
    default:
        DisplayError(_("Unrecognized command line parameters"));
        return false;
    }

    bool renderOnlyMode = false;
    if (parser.Found("r")) {
        logger_base.info("-r: Render mode is ON");
        renderOnlyMode = true;
    }

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    BitmapCache::SetupArtProvider();
    if (wxsOK)
    {
    	xLightsFrame* Frame = new xLightsFrame(nullptr, ab, -1, renderOnlyMode);
        if (Frame->CurrentDir == "") {
            logger_base.info("Show directory not set");
        }
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)

    xLightsFrame* const topFrame = (xLightsFrame*)GetTopWindow();
    __frame = topFrame;

    if (renderOnlyMode) {
        topFrame->CallAfter(&xLightsFrame::OpenRenderAndSaveSequencesF, sequenceFiles, xLightsFrame::RENDER_EXIT_ON_DONE);
    }

    if (parser.Found("cs")) {
        logger_base.info("-v: Check sequence mode is ON");
        topFrame->_checkSequenceMode = true;
        topFrame->CallAfter(&xLightsFrame::OpenAndCheckSequence, sequenceFiles, true);
    }

    if (parser.Found("o")) {
        logger_base.info("-o: Turning on output to lights");
        // Turn on output to lights - ignore if another xLights/xSchedule is already outputting
        topFrame->EnableOutputs(true);
    }

    #ifdef LINUX
        glutInit(&(wxApp::argc), wxApp::argv);
    #endif

    logger_base.info("XLightsApp OnInit Done.");

    return wxsOK;
}

void xLightsApp::WipeSettings()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Wiping settings.");

    wxConfigBase* config = wxConfigBase::Get();
    config->DeleteAll();
}

bool xLightsApp::ProcessIdle() {
    uint64_t now = wxGetLocalTimeMillis().GetValue();
    if (now > _nextIdleTime) {
        _nextIdleTime = now + 100;
        return wxApp::ProcessIdle();
    }
    CurlManager::INSTANCE.processCurls();
    return false;
}

//global flags from command line:
wxString xLightsApp::mediaDir;
wxString xLightsApp::showDir;
wxArrayString xLightsApp::sequenceFiles;
