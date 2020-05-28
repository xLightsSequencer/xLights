/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*AppHeaders
#include "RemoteFalconMain.h"
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

#include "RemoteFalconApp.h"

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Configurator.hh>
#include <wx/file.h>
#include <wx/msgdlg.h>

#include "../../xLights/xLightsVersion.h"
#include "../../xLights/SpecialOptions.h"

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
#pragma comment(lib, "log4cpplib.lib")
#endif
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
#pragma comment(lib, "ImageHlp.Lib")
#pragma comment(lib, "libcurl.lib")
#endif

#ifndef __WXOSX__
IMPLEMENT_APP_NO_MAIN(RemoteFalconApp)
#endif

static std::string __showDir;
static std::string __xScheduleURL;
static bool __started = false;
static p_xSchedule_Action __action;

#ifdef __WXOSX__
static RemoteFalconFrame* _remoteFalconFrame = nullptr;
#endif

static void WipeSettings()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.warn("------ Wiping RemoteFalcon settings ------");

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
    bool RemoteFalcon_xSchedule_Load(char* showDir) {
        __showDir = std::string(showDir);
        SpecialOptions::StashShowDir(showDir);
        return true;
    }

    void RemoteFalcon_xSchedule_GetVirtualWebFolder(char* buffer, size_t bufferSize) {
        memset(buffer, 0x00, bufferSize);
        strncpy(buffer, "RemoteFalcon", bufferSize - 1);
    }

    void RemoteFalcon_xSchedule_GetMenuLabel(char* buffer, size_t bufferSize) {
        memset(buffer, 0x00, bufferSize);
        strncpy(buffer, "Remote Falcon", bufferSize - 1);
    }

    bool RemoteFalcon_xSchedule_HandleWeb(const char* command, const wchar_t* parameters, const wchar_t* data, const wchar_t* reference, wchar_t* response, size_t responseSize) {
        return false;
    }

    // called when we want the plugin to actually interact with the user
    bool RemoteFalcon_xSchedule_Start(char* showDir, char* xScheduleURL, p_xSchedule_Action action) {
        if (__started) return true;

        __action = action;
        __showDir = std::string(showDir);
        __xScheduleURL = std::string(xScheduleURL);

        InitialiseLogging(false);

#ifdef __WXOSX__
        _remoteFalconFrame = new RemoteFalconFrame(0, __showDir, __xScheduleURL, __action);
        _remoteFalconFrame->Show();
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
    void RemoteFalcon_xSchedule_Stop() {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("RemoteFalcon_xSchedule_Stop");

        if (!__started) return;

#ifdef __WXOSX__
        delete _remoteFalconFrame;
        _remoteFalconFrame = nullptr;
#else
        wxEntryCleanup();
#endif

        __started = false;
    }

    void RemoteFalcon_xSchedule_WipeSettings() {
        WipeSettings();
    }

    // called just before xSchedule exits
    void RemoteFalcon_xSchedule_Unload() {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("RemoteFalcon_xSchedule_Unload");
    }

    void RemoteFalcon_xSchedule_NotifyStatus(const char* status) {
        ((RemoteFalconFrame*)wxTheApp->GetTopWindow())->NotifyStatus(status);
    }

    void RemoteFalcon_xSchedule_FireEvent(const char* eventType, const char* eventParameters)
    {
        ((RemoteFalconFrame*)wxTheApp->GetTopWindow())->FireEvent(eventType, eventParameters);
    }

    void RemoteFalcon_xSchedule_ManipulateBuffer(uint8_t* buffer, size_t bufferSize) {
        // we dont manipulate pixel data directly
    }

#ifdef __WXOSX__
#include "../PluginManager.h"
PluginManager::PluginState *CreateRemoteFalconPluginState() {
    PluginManager::PluginState *ret = new PluginManager::PluginState(nullptr);
    ret->_filename = "RemoteFalcon";
    ret->_loadFn = RemoteFalcon_xSchedule_Load;
    ret->_getVirtualWebFolderFn = RemoteFalcon_xSchedule_GetVirtualWebFolder;
    ret->_getMenuLabelFn = RemoteFalcon_xSchedule_GetMenuLabel;
    ret->_handleWebFn = RemoteFalcon_xSchedule_HandleWeb;
    ret->_startFn = RemoteFalcon_xSchedule_Start;
    ret->_stopFn = RemoteFalcon_xSchedule_Stop;
    ret->_wipeFn = RemoteFalcon_xSchedule_WipeSettings;
    ret->_unloadFn = RemoteFalcon_xSchedule_Unload;
    ret->_notifyStatusFn = RemoteFalcon_xSchedule_NotifyStatus;
    ret->_manipulateBufferFn = RemoteFalcon_xSchedule_ManipulateBuffer;
    ret->_fireEventFn = RemoteFalcon_xSchedule_FireEvent;
    return ret;
}
#else
extern "C" {
    bool WXEXPORT xSchedule_Load(char* showDir) {
        return RemoteFalcon_xSchedule_Load(showDir);
    }
    void WXEXPORT xSchedule_GetVirtualWebFolder(char* buffer, size_t bufferSize) {
        RemoteFalcon_xSchedule_GetVirtualWebFolder(buffer, bufferSize);
    }
    void WXEXPORT xSchedule_GetMenuLabel(char* buffer, size_t bufferSize) {
        RemoteFalcon_xSchedule_GetMenuLabel(buffer, bufferSize);
    }
    bool WXEXPORT xSchedule_HandleWeb(const char* command, const wchar_t* parameters, const wchar_t* data, const wchar_t* reference, wchar_t* response, size_t responseSize) {
        return RemoteFalcon_xSchedule_HandleWeb(command, parameters, data, reference, response, responseSize);
    }
    bool WXEXPORT xSchedule_Start(char* showDir, char* xScheduleURL, p_xSchedule_Action action) {
        return RemoteFalcon_xSchedule_Start(showDir, xScheduleURL, action);
    }
    void WXEXPORT xSchedule_Stop() {
        RemoteFalcon_xSchedule_Stop();
    }
    void WXEXPORT xSchedule_WipeSettings() {
        RemoteFalcon_xSchedule_WipeSettings();
    }
    void WXEXPORT xSchedule_Unload() {
        RemoteFalcon_xSchedule_Unload();
    }
    void WXEXPORT xSchedule_NotifyStatus(const char* status) {
        RemoteFalcon_xSchedule_NotifyStatus(status);
    }
    void WXEXPORT xSchedule_ManipulateBuffer(uint8_t* buffer, size_t bufferSize) {
        RemoteFalcon_xSchedule_ManipulateBuffer(buffer, bufferSize);
    }
    void WXEXPORT xSchedule_FireEvent(const char* eventType, const char* eventParameter)
    {
        RemoteFalcon_xSchedule_FireEvent(eventType, eventParameter);
    }
}

int RemoteFalconApp::OnExit()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("RemoteFalconApp::OnExit");

    return 0;
}

bool RemoteFalconApp::OnInit()
{
    InitialiseLogging(false);

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("******* OnInit: RemoteFalcon started.");

    //(*AppInitialize
    RemoteFalconFrame* Frame = new RemoteFalconFrame(0, __showDir, __xScheduleURL, __action);
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
        logger_base.info("RemoteFalcon process attach.");
        wxSetInstance((HINSTANCE)hModule);
        break;
    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    case DLL_PROCESS_DETACH:
        logger_base.info("RemoteFalcon process detach.");
        break;
    }

    return TRUE;
}
#endif
