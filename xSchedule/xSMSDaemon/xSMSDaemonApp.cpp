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

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/debugrpt.h>
#include <wx/cmdline.h>
#include <wx/confbase.h>
#include <wx/snglinst.h>

#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#include "xSMSDaemonApp.h"

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Configurator.hh>
#include <wx/file.h>
#include <wx/msgdlg.h>

#include "../../xLights/xLightsVersion.h"

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
//        #pragma comment(lib, "wxmsw31ud_adv.lib")
//        #pragma comment(lib, "wxmsw31ud_aui.lib")
//        #pragma comment(lib, "wxmsw31ud_gl.lib")
#pragma comment(lib, "wxzlibd.lib")
//        #pragma comment(lib, "wxmsw31ud_xrc.lib")
//        #pragma comment(lib, "wxmsw31ud_stc.lib")
#pragma comment(lib, "wxmsw31ud_qa.lib")
//        #pragma comment(lib, "wxmsw31ud_html.lib")
//        #pragma comment(lib, "wxmsw31ud_richtext.lib")
//        #pragma comment(lib, "wxmsw31ud_propgrid.lib")
//        #pragma comment(lib, "wxmsw31ud_ribbon.lib")
//        #pragma comment(lib, "wxmsw31ud_webview.lib")
//        #pragma comment(lib, "wxmsw31ud_media.lib")
#pragma comment(lib, "wxexpatd.lib")
#endif
#pragma comment(lib, "msvcprtd.lib")
#pragma comment(lib, "iphlpapi.lib")
//    #pragma comment(lib, "libdbghelp.a")
//    #pragma comment(lib, "log4cppd.lib")
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
#endif

#ifndef __WXOSX__
IMPLEMENT_APP_NO_MAIN(xSMSDaemonApp)
#endif

static std::string __showDir;
static std::string __xScheduleURL;
static bool __started = false;
static p_xSchedule_Action __action;

#ifdef __WXOSX__
static xSMSDaemonFrame* _smsFrame = nullptr;
#endif

static void WipeSettings()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.warn("------ Wiping xSMSDaemon settings ------");

    wxConfigBase* config = wxConfigBase::Get();
    config->DeleteAll();
}

static void InitialiseLogging(bool fromMain)
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
            }
            else if (wxFile::Exists(resourceName)) {
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
            }
            catch (log4cpp::ConfigureFailure & e) {
                // ignore config failure ... but logging wont work
                printf("Log issue:  %s\n", e.what());
            }
            catch (const std::exception & ex) {
                printf("Log issue: %s\n", ex.what());
            }
        }
    }
}

    // always called when the dll is found ... should not actually do anything
    bool xSMSDaemon_xSchedule_Load(char* showDir) {
        __showDir = std::string(showDir);
        return true;
    }

    void xSMSDaemon_xSchedule_GetVirtualWebFolder(char* buffer, size_t bufferSize) {
        memset(buffer, 0x00, bufferSize);
        strncpy(buffer, "xSMSDaemon", bufferSize - 1);
    }

    void xSMSDaemon_xSchedule_GetMenuLabel(char* buffer, size_t bufferSize) {
        memset(buffer, 0x00, bufferSize);
        strncpy(buffer, "SMS", bufferSize - 1);
    }

    bool xSMSDaemon_xSchedule_HandleWeb(const char* command, const wchar_t* parameters, const wchar_t* data, const wchar_t* reference, wchar_t* response, size_t responseSize) {
        std::wstring resp;
        memset(response, 0x00, responseSize);
        bool res = ((xSMSDaemonFrame*)wxTheApp->GetTopWindow())->Action(std::string(command), std::wstring(parameters), std::wstring(data), std::wstring(reference), resp);
        wchar_t* pr = (wchar_t*)resp.c_str();
        wcsncpy(response, pr, (responseSize / 2) - 1); // divide by 2 as 2 byte characters
        return res;
    }

    // called when we want the plugin to actually interact with the user
    bool xSMSDaemon_xSchedule_Start(char* showDir, char* xScheduleURL, p_xSchedule_Action action) {
        if (__started) return true;

        __action = action;
        __showDir = std::string(showDir);
        __xScheduleURL = std::string(xScheduleURL);

        InitialiseLogging(false);

#ifdef __WXOSX__
        _smsFrame = new xSMSDaemonFrame(0, __showDir, __xScheduleURL, __action);
        _smsFrame->Show();
#else
        int argc = 0;
        char** argv = NULL;
        if (!wxEntryStart(argc, argv) || !wxTheApp || !wxTheApp->CallOnInit())
            return false;
#endif

        __started = true;
        return true;
    }

    // called when we want the plugin to exit
    void xSMSDaemon_xSchedule_Stop() {
        //static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        if (!__started) return;

#ifdef __WXOSX__
        delete _smsFrame;
        _smsFrame = nullptr;
#else
        wxEntryCleanup();
#endif

        __started = false;
    }

    void xSMSDaemon_xSchedule_WipeSettings() {
        WipeSettings();
    }

    // called just before xSchedule exits
    void xSMSDaemon_xSchedule_Unload() {
    }

    void xSMSDaemon_xSchedule_NotifyStatus(const char* status) {
        // we dont care about status
    }

    void xSMSDaemon_xSchedule_ManipulateBuffer(uint8_t* buffer, size_t bufferSize) {
        // we dont manipulate pixel data directly
    }

#ifdef __WXOSX__
#include "../PluginManager.h"
PluginManager::PluginState *CreateSMSPluginState() {
    PluginManager::PluginState *ret = new PluginManager::PluginState(nullptr);
    ret->_filename = "SMSDaemon";
    ret->_loadFn = xSMSDaemon_xSchedule_Load;
    ret->_unloadFn = xSMSDaemon_xSchedule_Unload;
    ret->_startFn = xSMSDaemon_xSchedule_Start;
    ret->_stopFn = xSMSDaemon_xSchedule_Stop;
    ret->_handleWebFn = xSMSDaemon_xSchedule_HandleWeb;
    ret->_wipeFn = xSMSDaemon_xSchedule_WipeSettings;
    ret->_manipulateBufferFn = xSMSDaemon_xSchedule_ManipulateBuffer;
    ret->_notifyStatusFn = xSMSDaemon_xSchedule_NotifyStatus;
    ret->_getVirtualWebFolderFn = xSMSDaemon_xSchedule_GetVirtualWebFolder;
    ret->_getMenuLabelFn = xSMSDaemon_xSchedule_GetMenuLabel;
    return ret;
}
#else
extern "C" {
    bool WXEXPORT xSchedule_Load(char* showDir) {
        return xSMSDaemon_xSchedule_Load(showDir);
    }
    void WXEXPORT xSchedule_GetVirtualWebFolder(char* buffer, size_t bufferSize) {
        xSMSDaemon_xSchedule_GetVirtualWebFolder(buffer, bufferSize);
    }
    void WXEXPORT xSchedule_GetMenuLabel(char* buffer, size_t bufferSize) {
        xSMSDaemon_xSchedule_GetMenuLabel(buffer, bufferSize);
    }
    bool WXEXPORT xSchedule_HandleWeb(const char* command, const wchar_t* parameters, const wchar_t* data, const wchar_t* reference, wchar_t* response, size_t responseSize) {
        return xSMSDaemon_xSchedule_HandleWeb(command, parameters, data, reference, response, responseSize);
    }
    bool WXEXPORT xSchedule_Start(char* showDir, char* xScheduleURL, p_xSchedule_Action action) {
        return xSMSDaemon_xSchedule_Start(showDir, xScheduleURL, action);
    }
    void WXEXPORT xSchedule_Stop() {
        xSMSDaemon_xSchedule_Stop();
    }
    void WXEXPORT xSchedule_WipeSettings() {
        xSMSDaemon_xSchedule_WipeSettings();
    }
    void WXEXPORT xSchedule_Unload() {
        xSMSDaemon_xSchedule_Unload();
    }
    void WXEXPORT Schedule_NotifyStatus(const char* status) {
        xSMSDaemon_xSchedule_NotifyStatus(status);
    }
    void WXEXPORT xSchedule_ManipulateBuffer(uint8_t* buffer, size_t bufferSize) {
        xSMSDaemon_xSchedule_ManipulateBuffer(buffer, bufferSize);
    }
}

int xSMSDaemonApp::OnExit()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("xSMSDaemon exiting.");

    return 0;
}

bool xSMSDaemonApp::OnInit()
{
    InitialiseLogging(false);

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("******* OnInit: xSMSDaemon started.");

    //(*AppInitialize
    xSMSDaemonFrame* Frame = new xSMSDaemonFrame(0, __showDir, __xScheduleURL, __action);
    Frame->Show();

    return true;
}

#endif

#ifdef __WXMSW__
BOOL APIENTRY DllMain(HANDLE hModule,
    DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        InitialiseLogging(false);
        logger_base.info("xSMSDaemon process attach.");
        wxSetInstance((HINSTANCE)hModule);
        break;
    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    case DLL_PROCESS_DETACH:
        logger_base.info("xSMSDaemon process detach.");
        break;
    }

    return TRUE;
}
#endif
