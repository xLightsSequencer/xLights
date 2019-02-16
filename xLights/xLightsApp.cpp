/***************************************************************
 * Name:      xLightsApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/

//(*AppHeaders
#include "xLightsMain.h"
#include <wx/image.h>
//*)

#include <wx/stdpaths.h>
#include <wx/config.h>

#include <stdlib.h>     /* srand */
#include <time.h>       /* time */
#include <thread>
#include <iomanip>

#include "xLightsApp.h"
#include "xLightsVersion.h"
#include "Parallel.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Configurator.hh>

#ifdef LINUX
#include <GL/glut.h>
#endif

#ifndef __WXMSW__
#include <execinfo.h>
#else
#include "MSWStackWalk.h"
#endif

xLightsFrame* xLightsApp::__frame = nullptr;

void InitialiseLogging(bool fromMain)
{
    static bool loggingInitialised = false;

    if (!loggingInitialised)
    {

#ifdef __WXMSW__
        std::string initFileName = "xlights.windows.properties";
#endif
#ifdef __WXOSX_MAC__
        std::string initFileName = "xlights.mac.properties";
        if (!wxFile::Exists(initFileName)) {
            if (fromMain) {
                return;
            } else if (wxFile::Exists(wxStandardPaths::Get().GetResourcesDir() + "/xlights.mac.properties")) {
                initFileName = wxStandardPaths::Get().GetResourcesDir() + "/xlights.mac.properties";
            }
        }
        loggingInitialised = true;

#endif
#ifdef __LINUX__
        std::string initFileName = wxStandardPaths::Get().GetInstallPrefix() + "/bin/xlights.linux.properties";
        if (!wxFile::Exists(initFileName)) {
            initFileName = wxStandardPaths::Get().GetInstallPrefix() + "/share/xLights/xlights.linux.properties";
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
				static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

                wxDateTime now = wxDateTime::Now();    
                int millis = wxGetUTCTimeMillis().GetLo() % 1000;
                wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(), now.GetMonth(), now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond(), millis);
                logger_base.info("Start Time: %s.", (const char *)ts.c_str());

				logger_base.info("Log4CPP config read from %s.", (const char *)initFileName.c_str());

				auto categories = log4cpp::Category::getCurrentCategories();

				for (auto it = categories->begin(); it != categories->end(); ++it)
				{
					std::string levels = "";

					if ((*it)->isAlertEnabled()) levels += "ALERT ";
					if ((*it)->isCritEnabled()) levels += "CRIT ";
					if ((*it)->isDebugEnabled()) levels += "DEBUG ";
					if ((*it)->isEmergEnabled()) levels += "EMERG ";
					if ((*it)->isErrorEnabled()) levels += "ERROR ";
					if ((*it)->isFatalEnabled()) levels += "FATAL ";
					if ((*it)->isInfoEnabled()) levels += "INFO ";
					if ((*it)->isNoticeEnabled()) levels += "NOTICE ";
					if ((*it)->isWarnEnabled()) levels += "WARN ";

					logger_base.info("    %s : %s", (const char *)(*it)->getName().c_str(), (const char *)levels.c_str());
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
    //     predictable ... as we want the handleCrash function to be able to locate the file to include it in the crash
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("******* XLights main function executing.");

#ifdef LINUX
    XInitThreads();
#endif
    wxDISABLE_DEBUG_SUPPORT();

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

#include <wx/debugrpt.h>

xLightsFrame *topFrame = nullptr;

void handleCrash(void *data) {
    static volatile bool inCrashHandler = false;
    
    if (inCrashHandler) {
        //need to ignore any crashes in the crash handler
        return;
    }
    inCrashHandler = true;
    
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.crit("Crash handler called.");
	wxDebugReportCompress *report = new wxDebugReportCompress();
    report->SetCompressedFileDirectory(topFrame->CurrentDir);

    #ifndef __WXMSW__
        // dont call these for windows as they dont seem to do anything.
        report->AddAll(wxDebugReport::Context_Exception);
    #endif

    wxFileName fn(topFrame->CurrentDir, OutputManager::GetNetworksFileName());
    if (fn.Exists()) {
        report->AddFile(fn.GetFullPath(), OutputManager::GetNetworksFileName());
    }
    if (wxFileName(topFrame->CurrentDir, "xlights_rgbeffects.xml").Exists()) {
        report->AddFile(wxFileName(topFrame->CurrentDir, "xlights_rgbeffects.xml").GetFullPath(), "xlights_rgbeffects.xml");
    }
    if (topFrame->UnsavedRgbEffectsChanges &&  wxFileName(topFrame->CurrentDir, "xlights_rgbeffects.xbkp").Exists()) {
        report->AddFile(wxFileName(topFrame->CurrentDir, "xlights_rgbeffects.xbkp").GetFullPath(), "xlights_rgbeffects.xbkp");
    }

    if (topFrame->GetSeqXmlFileName() != "") {
        wxFileName fn2(topFrame->GetSeqXmlFileName());
        if (fn2.Exists() && !fn2.IsDir()) {
            report->AddFile(topFrame->GetSeqXmlFileName(), fn2.GetName());
            wxFileName fnb(fn2.GetPath() + "/" + fn2.GetName() + ".xbkp");
            if (fnb.Exists())
            {
                report->AddFile(fnb.GetFullPath(), fnb.GetName());
            }
        }
        else
        {
            wxFileName fnb(topFrame->CurrentDir + "/" + "__.xbkp");
            if (fnb.Exists())
            {
                report->AddFile(fnb.GetFullPath(), fnb.GetName());
            }
        }
    }
    else
    {
        wxFileName fnb(topFrame->CurrentDir + "/" + "__.xbkp");
        if (fnb.Exists())
        {
            report->AddFile(fnb.GetFullPath(), fnb.GetName());
        }
    }
    wxString trace = wxString::Format("xLights version %s\n\n", GetDisplayVersionString());

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

    if (wxThread::IsMain()) {
        trace += wxString::Format("\nCrashed thread the Main Thread\n");
    } else {
        
        std::stringstream ret;
        ret << std::showbase // show the 0x prefix
            << std::internal // fill between the prefix and the number
            << std::setfill('0') << std::setw(10)
            << std::hex << std::this_thread::get_id();
        
        std::string id = ret.str();
        trace += wxString::Format("\nCrashed thread id: %s\n", id.c_str());
    }
    //These will be added on the main thread
    //trace += topFrame->GetThreadStatusReport();
    //trace += ParallelJobPool::POOL.GetThreadStatus();

    report->AddText("backtrace.txt", trace, "Backtrace");

    logger_base.crit("%s", (const char *)trace.c_str());

    wxString dir;
#ifdef __WXMSW__
    wxGetEnv("APPDATA", &dir);
    std::string filename = std::string(dir.c_str()) + "/xLights_l4cpp.log";
#endif
#ifdef __WXOSX_MAC__
    wxFileName home;
    home.AssignHomeDir();
    dir = home.GetFullPath();
    std::string filename = std::string(dir.c_str()) + "/Library/Logs/xLights_l4cpp.log";
#endif
#ifdef __LINUX__
    std::string filename = "/tmp/xLights_l4cpp.log";
#endif

    if (wxFile::Exists(filename))
    {
        report->AddFile(filename, "xLights_l4cpp.log");
    }
    else if (wxFile::Exists(wxFileName(topFrame->CurrentDir, "xLights_l4cpp.log").GetFullPath()))
    {
        report->AddFile(wxFileName(topFrame->CurrentDir, "xLights_l4cpp.log").GetFullPath(), "xLights_l4cpp.log");
    }
    else if (wxFile::Exists(wxFileName(wxGetCwd(), "xLights_l4cpp.log").GetFullPath()))
    {
        report->AddFile(wxFileName(wxGetCwd(), "xLights_l4cpp.log").GetFullPath(), "xLights_l4cpp.log");
    }

    if (!wxThread::IsMain() && topFrame != nullptr) 
    {
        topFrame->CallAfter(&xLightsFrame::CreateDebugReport, report);
        wxSleep(600000);
    } 
    else if (topFrame != nullptr)
    {
        topFrame->CreateDebugReport(report);
    }
    else
    {
        // unable to create debug report
        logger_base.crit("Unable to tell user about debug report. Crash report saved to %s.", (const char *)report->GetCompressedFileName().c_str());
    }
}

wxString xLightsFrame::GetThreadStatusReport() {
    return jobPool.GetThreadStatus();
}

void xLightsFrame::CreateDebugReport(wxDebugReportCompress *report) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    static bool inHere = false;

    // if we are in here a second time ... just return
    if (inHere) return;

    inHere = true;

    //add thread status - must be done on main thread
    //due to mutex locks potentially being problematic
    std::string status = "Render Pool:\n";
    status += topFrame->GetThreadStatusReport();
    status += "\nParallel Job Pool:\n";
    status += ParallelJobPool::POOL.GetThreadStatus();

    wxFileName fileName(report->GetDirectory(), "backtrace.txt");
    wxFile file(fileName.GetFullPath(),  wxFile::write_append);
    file.Write("\n");
    file.Write(status);
    file.Flush();
    file.Close();
    logger_base.crit("%s", (const char *)status.c_str());


    if (wxDebugReportPreviewStd().Show(*report)) {
        report->Process();
        SendReport("crashUpload", *report);
        wxMessageBox("Crash report saved to " + report->GetCompressedFileName());
    }
    logger_base.crit("Exiting after creating debug report: %s", (const char *)report->GetCompressedFileName().c_str());
	delete report;

    inHere = false;

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
    InitialiseLogging(false);
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("******* OnInit: XLights started.");

    DumpConfig();

    int id = (int)wxThread::GetCurrentId();
    logger_base.info("Main thread id: 0x%X or %i", id, id);

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

//    heartbeat("init", true); //tell monitor active now -DJ
//check for options on command line: -DJ
//TODO: maybe use wxCmdLineParser instead?
//do this before instantiating xLightsFrame so it can use info gathered here
    wxString unrecog, info;

    static const wxCmdLineEntryDesc cmdLineDesc [] =
    {
        { wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        { wxCMD_LINE_SWITCH, "d", "debug", "enable debug mode"},
        { wxCMD_LINE_SWITCH, "r", "render", "render files and exit"},
        { wxCMD_LINE_OPTION, "m", "media", "specify media directory"},
        { wxCMD_LINE_OPTION, "s", "show", "specify show directory" },
        { wxCMD_LINE_OPTION, "g", "opengl", "specify OpenGL version" },
        { wxCMD_LINE_SWITCH, "w", "wipe", "wipe settings clean" },
        { wxCMD_LINE_SWITCH, "o", "on", "turn on output to lights" },
#ifdef __LINUX__
        { wxCMD_LINE_SWITCH, "x", "xschedule", "run xschedule" },
        { wxCMD_LINE_SWITCH, "a", "xsmsdaemon", "run xsmsdaemon" },
        { wxCMD_LINE_SWITCH, "c", "xcapture", "run xcapture" },
        { wxCMD_LINE_SWITCH, "f", "xfade", "run xfade" },
#endif
        { wxCMD_LINE_PARAM, "", "", "sequence file", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE},
        { wxCMD_LINE_NONE }
    };

// Add option to run xutils via xlights on linux (for AppImage usage)
#ifdef __LINUX__
       int run_xsmsdaemon = FALSE;
       int run_xschedule = FALSE;
       int run_xcapture = FALSE;
       int run_xfade= FALSE;
       wxFileName f(wxStandardPaths::Get().GetExecutablePath());
       wxString appPath(f.GetPath());
       wxString cmdlineC(appPath+wxT("/xCapture"));
       wxString cmdlineS(appPath+wxT("/xSchedule"));
       wxString cmdlineF(appPath+wxT("/xFade"));
       wxString cmdlineM(appPath+wxT("/xSMSDaemon"));
        for (int i=1; i< argc;i++) {
            if (strncmp(argv[i].c_str(), "-x", 2) == 0) {
                run_xschedule = TRUE;
            } else if (strncmp(argv[i].c_str(), "-c", 2) == 0) {
                run_xcapture = TRUE;
            } else if (strncmp(argv[i].c_str(), "-a", 2) == 0) {
                run_xsmsdaemon = TRUE;
            } else if (strncmp(argv[i].c_str(), "-f", 2) == 0) {
                run_xfade = TRUE;
            } else {
                cmdlineS += wxT(" ");
                cmdlineS += wxString::FromUTF8(argv[i]);
                cmdlineC += wxT(" ");
                cmdlineC += wxString::FromUTF8(argv[i]);
                cmdlineF += wxT(" ");
                cmdlineF += wxString::FromUTF8(argv[i]);
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
        }
#endif

    wxCmdLineParser parser(cmdLineDesc, argc, argv);
    switch (parser.Parse()) {
    case -1:
        // help was given
        return false;
    case 0:
        {
            wxString glVersion;
            if (parser.Found("g", &glVersion))
            {
                wxConfigBase* config = wxConfigBase::Get();
                if (glVersion == "" || glVersion.Lower() == "auto")
                {
                    config->Write("ForceOpenGLVer", 99);
                }
                else
                {
                    int gl = wxAtoi(glVersion);
                    if (gl != 0)
                    {
                        config->Write("ForceOpenGLVer", gl);
                    }
                }
                info += _("Forcing open GL version\n");
            }
        }
        if (parser.Found("w"))
        {
            logger_base.info("-w: Wiping settings");
            info += _("Wiping settings\n");
            WipeSettings();
        }
        WantDebug = parser.Found("d");
        if (WantDebug) {
            logger_base.info("-d: Debug is ON");
            info += _("Debug is ON\n");
        }
        if (parser.Found("s", &showDir)) {
            logger_base.info("-s: Show directory set to %s.", (const char *)showDir.c_str());
            info += _("Setting show directory to ") + showDir + "\n";
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
                while (showDir != "" && !wxFile::Exists(showDir + "/" + "xlights_rgbeffects.xml"))
                {
                    auto old = showDir;
                    showDir = wxPathOnly(showDir);
                    if (showDir == old) showDir = "";
                }
            }
            sequenceFiles.push_back(sequenceFile);
        }
        if (!parser.Found("r") && !parser.Found("o") && !info.empty())
        {
            DisplayInfo(info); //give positive feedback*/
        }
        break;
    default:
        DisplayError(_("Unrecognized command line parameters"));
        return false;
    }

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if (wxsOK)
    {
    	xLightsFrame* Frame = new xLightsFrame(nullptr);
        if (Frame->CurrentDir == "")
        {
            return false;
        }
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)

    topFrame = (xLightsFrame*)GetTopWindow();
    __frame = topFrame;

    if (parser.Found("r")) {
        logger_base.info("-r: Render mode is ON");
        topFrame->_renderMode = true;
        topFrame->CallAfter(&xLightsFrame::OpenRenderAndSaveSequences, sequenceFiles, true);
    }

    if (parser.Found("o"))
    {
        logger_base.info("-o: Turning on output to lights");

        // Turn on output to lights - ignore if another xLights/xSchedule is already outputting
        topFrame->CheckBoxLightOutput->SetValue(true);
        topFrame->EnableOutputs(true);
    }

    #ifdef LINUX
        glutInit(&(wxApp::argc), wxApp::argv);
    #endif

    logger_base.info("XLightsApp OnInit Done.");

    return wxsOK;
}


void xLightsApp::OnFatalException() {
    handleCrash(nullptr);
}

void xLightsApp::WipeSettings()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Wiping settings.");

    wxConfigBase* config = wxConfigBase::Get();
    config->DeleteAll();
}

//global flags from command line:
bool xLightsApp::WantDebug = false;
wxString xLightsApp::DebugPath;
wxString xLightsApp::mediaDir;
wxString xLightsApp::showDir;
wxArrayString xLightsApp::sequenceFiles;
