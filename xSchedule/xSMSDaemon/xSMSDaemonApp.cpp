/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
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

#include "spdlog/spdlog.h"
#include "spdlog/common.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <wx/file.h>
#include <wx/msgdlg.h>

#include "../../xLights/xLightsVersion.h"

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
#pragma comment(lib, "wxzlibd.lib")
#pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_qa.lib")
#pragma comment(lib, "wxexpatd.lib")
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
#pragma comment(lib, "wxzlib.lib")
#pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_qa.lib")
#pragma comment(lib, "wxexpat.lib")
#endif
#pragma comment(lib, "libcurl.dll.a")
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
#endif

IMPLEMENT_APP_NO_MAIN(xSMSDaemonApp)

static std::string __showDir;
static std::string __xScheduleURL;
static bool __started = false;
static p_xSchedule_Action __action;

xSMSDaemonApp::xSMSDaemonApp() :
    xlBaseApp("xSMSDaemon")
{
}

p_xSchedule_Action xSMSDaemonApp::GetAction()
{
    return __action;
}

static void WipeSettings()
{
    spdlog::warn("------ Wiping xSMSDaemon settings ------");

    wxConfigBase* config = wxConfigBase::Get();
    config->DeleteAll();
}

static void InitialiseLogging(bool fromMain)
{
    static bool loggingInitialised = false;

    if (!loggingInitialised)
    {
        std::string const logFileName = "xSMSDaemon_spdlog.log";
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

        auto file_logger = std::make_shared<spdlog::logger>("xsmsdaemon", rotating_file_sink);
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

        spdlog::info("Start Time: {}.", ts.ToStdString());
        spdlog::info("Current working directory {}.", wxGetCwd().ToStdString());
        
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

        int argc = 0;
        char** argv = NULL;
        if (!wxEntryStart(argc, argv) || !wxTheApp || !wxTheApp->CallOnInit())
            return false;

        __started = true;
        return true;
    }

    // called when we want the plugin to exit
    void xSMSDaemon_xSchedule_Stop() {
        spdlog::debug("xSMSDaemon_xSchedule_Stop");

        if (!__started) return;

        wxEntryCleanup();

        __started = false;
    }

    void xSMSDaemon_xSchedule_WipeSettings() {
        WipeSettings();
    }

    // called just before xSchedule exits
    void xSMSDaemon_xSchedule_Unload() {
        spdlog::debug("xSMSDaemon_xSchedule_Unload");
    }

    void xSMSDaemon_xSchedule_NotifyStatus(const char* status) {
        // we dont care about status
    }

    bool xSMSDaemon_xSchedule_FireEvent(const char* eventType, const char* eventParameters)
    {
        // we dont care about events
		return false;
    }

    bool xSMSDaemon_xSchedule_SendCommand(const char* command, const char* parameters, char* msg, size_t bufferSize)
    {
        // we dont care about command
        memset(msg, 0x00, bufferSize);
        strncpy(msg, "Commands not supported by xSMSDaemon", bufferSize - 1);
        return false;
    }

    void xSMSDaemon_xSchedule_ManipulateBuffer(uint8_t* buffer, size_t bufferSize) {
        // we dont manipulate pixel data directly
    }

    extern "C" {
        bool WXEXPORT xSchedule_Load(char* showDir)
        {
            return xSMSDaemon_xSchedule_Load(showDir);
        }
        void WXEXPORT xSchedule_GetVirtualWebFolder(char* buffer, size_t bufferSize)
        {
            xSMSDaemon_xSchedule_GetVirtualWebFolder(buffer, bufferSize);
        }
        void WXEXPORT xSchedule_GetMenuLabel(char* buffer, size_t bufferSize)
        {
            xSMSDaemon_xSchedule_GetMenuLabel(buffer, bufferSize);
        }
        bool WXEXPORT xSchedule_HandleWeb(const char* command, const wchar_t* parameters, const wchar_t* data, const wchar_t* reference, wchar_t* response, size_t responseSize)
        {
            return xSMSDaemon_xSchedule_HandleWeb(command, parameters, data, reference, response, responseSize);
        }
        bool WXEXPORT xSchedule_Start(char* showDir, char* xScheduleURL, p_xSchedule_Action action)
        {
            return xSMSDaemon_xSchedule_Start(showDir, xScheduleURL, action);
        }
        void WXEXPORT xSchedule_Stop()
        {
            xSMSDaemon_xSchedule_Stop();
        }
        void WXEXPORT xSchedule_WipeSettings()
        {
            xSMSDaemon_xSchedule_WipeSettings();
        }
        void WXEXPORT xSchedule_Unload()
        {
            xSMSDaemon_xSchedule_Unload();
        }
        void WXEXPORT xSchedule_NotifyStatus(const char* status)
        {
            xSMSDaemon_xSchedule_NotifyStatus(status);
        }
        void WXEXPORT xSchedule_ManipulateBuffer(uint8_t* buffer, size_t bufferSize)
        {
            xSMSDaemon_xSchedule_ManipulateBuffer(buffer, bufferSize);
        }
        bool WXEXPORT xSchedule_FireEvent(const char* eventType, const char* eventParameter)
        {
            return xSMSDaemon_xSchedule_FireEvent(eventType, eventParameter);
        }
        bool WXEXPORT xSchedule_SendCommand(const char* command, const char* parameters, char* msg, size_t bufferSize)
        {
            return xSMSDaemon_xSchedule_SendCommand(command, parameters, msg, bufferSize);
        }
    }

int xSMSDaemonApp::OnExit()
{
    spdlog::info("xSMSDaemonApp::OnExit");

    return 0;
}

bool xSMSDaemonApp::OnInit()
{
    InitialiseLogging(false);

    spdlog::info("******* OnInit: xSMSDaemon started.");

    //(*AppInitialize
    xSMSDaemonFrame* Frame = new xSMSDaemonFrame(0, __showDir, __xScheduleURL, __action);
    Frame->Show();

    return true;
}

#ifdef __WXMSW__
BOOL APIENTRY DllMain(HANDLE hModule,
    DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        InitialiseLogging(false);
        spdlog::info("xSMSDaemon process attach.");
        wxSetInstance((HINSTANCE)hModule);
        break;
    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    case DLL_PROCESS_DETACH:
        spdlog::info("xSMSDaemon process detach.");
        break;
    }

    return TRUE;
}
#endif
