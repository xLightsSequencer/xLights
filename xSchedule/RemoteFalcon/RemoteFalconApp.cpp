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

#include "spdlog/spdlog.h"
#include "spdlog/common.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include <wx/file.h>
#include <wx/msgdlg.h>

#include "../../xLights/xLightsVersion.h"
#include "../../xLights/SpecialOptions.h"

#include "../../xLights/UtilFunctions.h"

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

IMPLEMENT_APP_NO_MAIN(RemoteFalconApp)

static std::string __showDir;
static std::string __xScheduleURL;
static bool __started = false;
static p_xSchedule_Action __action;

RemoteFalconApp::RemoteFalconApp() :
    xlBaseApp("RemoteFalcon")
{
}

static void WipeSettings()
{
    spdlog::warn("------ Wiping RemoteFalcon settings ------");

    wxConfigBase* config = wxConfigBase::Get();
    config->DeleteAll();
}

static void InitialiseLogging(bool fromMain)
{
    static bool loggingInitialised = false;

    if (!loggingInitialised)
    {

std::string const logFileName = "xschedule_spdlog.log";
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

        auto file_logger = std::make_shared<spdlog::logger>("xschedule", rotating_file_sink);
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

    bool RemoteFalcon_xSchedule_SendCommand(const char* command, const char* parameters, char* msg, size_t bufferSize)
    {
        memset(msg, 0x00, bufferSize);
        std::string c(command);
        std::string p(parameters);
        std::string m;
        bool rc = ((RemoteFalconFrame*)wxTheApp->GetTopWindow())->SendCommand(c, p, m);
        strncpy(msg, m.c_str(), bufferSize - 1);
        return rc;
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

        int argc = 0;
        char** argv = NULL;
        if (!wxEntryStart(argc, argv) || !wxTheApp || !wxTheApp->CallOnInit())
            return false;

        __started = true;
        return true;
    }

    // called when we want the plugin to exit
    void RemoteFalcon_xSchedule_Stop() {
        spdlog::debug("RemoteFalcon_xSchedule_Stop");

        if (!__started) return;

        wxEntryCleanup();

        __started = false;
    }

    void RemoteFalcon_xSchedule_WipeSettings() {
        WipeSettings();
    }

    // called just before xSchedule exits
    void RemoteFalcon_xSchedule_Unload() {
        spdlog::debug("RemoteFalcon_xSchedule_Unload");
    }

    void RemoteFalcon_xSchedule_NotifyStatus(const char* status) {
        ((RemoteFalconFrame*)wxTheApp->GetTopWindow())->NotifyStatus(status);
    }

    bool RemoteFalcon_xSchedule_FireEvent(const char* eventType, const char* eventParameters)
    {
        return ((RemoteFalconFrame*)wxTheApp->GetTopWindow())->FireEvent(eventType, eventParameters);
    }

    void RemoteFalcon_xSchedule_ManipulateBuffer(uint8_t* buffer, size_t bufferSize) {
        // we dont manipulate pixel data directly
    }

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
    bool WXEXPORT xSchedule_FireEvent(const char* eventType, const char* eventParameter)
    {
        return RemoteFalcon_xSchedule_FireEvent(eventType, eventParameter);
    }
    bool WXEXPORT xSchedule_SendCommand(const char* command, const char* parameters, char* msg, size_t bufferSize)
    {
        return RemoteFalcon_xSchedule_SendCommand(command, parameters, msg, bufferSize);
    }
}

int RemoteFalconApp::OnExit()
{
    spdlog::info("RemoteFalconApp::OnExit");

    return 0;
}

bool RemoteFalconApp::OnInit()
{
    InitialiseLogging(false);

    spdlog::info("******* OnInit: RemoteFalcon started.");

    //(*AppInitialize
    RemoteFalconFrame* Frame = new RemoteFalconFrame(0, __showDir, __xScheduleURL, __action);
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
        spdlog::info("RemoteFalcon process attach.");
        wxSetInstance((HINSTANCE)hModule);
        break;
    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    case DLL_PROCESS_DETACH:
        spdlog::info("RemoteFalcon process detach.");
        break;
    }

    return TRUE;
}
#endif
