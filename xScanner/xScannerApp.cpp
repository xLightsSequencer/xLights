/***************************************************************
 * Name:      xScannerApp.cpp
 * Purpose:   Code for Application Class
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#include "xScannerApp.h"

//(*AppHeaders
#include "xScannerMain.h"
#include <wx/image.h>
//*)

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Configurator.hh>
#include <wx/file.h>
#include <wx/msgdlg.h>

#include "../xLights/xLightsVersion.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/debugrpt.h>
#include <wx/cmdline.h>
#include <wx/confbase.h>

#ifdef LINUX
    #include <X11/Xlib.h>
#endif

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
        #pragma comment(lib, "log4cpplibd.lib")
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
        #pragma comment(lib, "log4cpplib.lib")
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

IMPLEMENT_APP(xScannerApp)

xScannerApp::xScannerApp() :
    xlBaseApp("xScanner")
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
        std::string initFileName = "xScanner.windows.properties";
#endif
#ifdef __WXOSX__
        std::string initFileName = "xScanner.mac.properties";
        std::string resourceName = wxStandardPaths::Get().GetResourcesDir().ToStdString() + "/xScanner.mac.properties";
        if (!wxFile::Exists(initFileName)) {
            if (fromMain) {
                return;
            } else if (wxFile::Exists(resourceName)) {
                initFileName = resourceName;
            }
        }
        loggingInitialised = true;

#endif
#ifdef LINUX
        std::string initFileName = wxStandardPaths::Get().GetInstallPrefix() + "/bin/xscanner.linux.properties";
        if (!wxFile::Exists(initFileName)) {
            initFileName = wxStandardPaths::Get().GetInstallPrefix() + "/share/xLights/xscanner.linux.properties";
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

void xScannerApp::WipeSettings()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.warn("------ Wiping settings ------");

    wxConfigBase* config = wxConfigBase::Get();
    config->DeleteAll();
}

int xScannerApp::OnExit()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("xScanner exiting.");

    return 0;
}

bool xScannerApp::OnInit()
{
    wxLog::SetLogLevel(wxLOG_FatalError);

#ifdef _MSC_VER
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

    InitialiseLogging(false);
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("******* OnInit: xScanner started.");

    DumpConfig();
#ifdef __WXMSW__
    MSWEnableDarkMode();
#endif
    static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
        { wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        { wxCMD_LINE_SWITCH, "s", "singlethreaded", "run the query process single threaded" },
        { wxCMD_LINE_SWITCH, "w", "wipe", "wipe settings clean" },
        { wxCMD_LINE_NONE }
    };

    bool parmfound = false;
    bool singleThreaded = false;
#ifdef LINUX
    singleThreaded = true;
#endif
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
        if (parser.Found("s")) {
            parmfound = true;
            logger_base.info("-s: Running single threaded");
            singleThreaded = true;
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

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	xScannerFrame* Frame = new xScannerFrame(0, singleThreaded);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;
}
