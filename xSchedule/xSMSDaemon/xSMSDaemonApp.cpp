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

#include "xSMSDaemonApp.h"

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Configurator.hh>
#include <wx/file.h>
#include <wx/msgdlg.h>

#include "../../xLights/xLightsVersion.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/debugrpt.h>
#include <wx/cmdline.h>
#include <wx/confbase.h>
#include <wx/snglinst.h>

#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

IMPLEMENT_APP_NO_MAIN(xSMSDaemonApp)

HANDLE ThreadId;
std::string __showDir;
xSMSDaemonApp* _app = nullptr;

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    _app = new xSMSDaemonApp();
    wxApp::SetInstance(_app);

    wxEntry(GetModuleHandle(NULL), NULL, NULL, SW_SHOW);

    delete _app;
    _app = nullptr;
    return true;
}

void WipeSettings()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.warn("------ Wiping xSMSDaemon settings ------");

    wxConfigBase* config = wxConfigBase::Get();
    config->DeleteAll();
}

extern "C" {
    // always called when the dll is found ... should not actually do anything
    bool __declspec(dllexport) xSchedule_Load(char* showDir)
    {
        __showDir = std::string(showDir);
        return true;
    }

    void __declspec(dllexport) xSchedule_GetVirtualWebFolder(char* buffer, size_t bufferSize)
    {
        memset(buffer, 0x00, bufferSize);
        strncpy(buffer, "xSMSDaemon", bufferSize - 1);
    }

    void __declspec(dllexport) xSchedule_GetMenuLabel(char* buffer, size_t bufferSize)
    {
        memset(buffer, 0x00, bufferSize);
        strncpy(buffer, "SMS", bufferSize - 1);
    }

    bool __declspec(dllexport) xSchedule_HandleWeb(const std::string& action, const std::string& parms)
    {
        wxMessageBox("xSchedule_HandleWeb");
        return false;
    }

    // called when we want the plugin to actually interact with the user
    bool __declspec(dllexport) xSchedule_Start(char* showDir)
    {
        if (_app != nullptr) return true; // already started

        __showDir = std::string(showDir);

        ThreadId = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);

        return true;
    }

    // called when we want the plugin to exit
    void __declspec(dllexport) xSchedule_Stop()
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        if (_app != nullptr)
        {
            wxEntryCleanup();
        }
    }

    void __declspec(dllexport) xSchedule_WipeSettings()
    {
        WipeSettings();
    }

    // called just before xSchedule exits
    void __declspec(dllexport) xSchedule_Unload()
    {
    }
}

#ifdef __WXMSW__
BOOL APIENTRY DllMain(HANDLE hModule,
    DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}
#endif

int xSMSDaemonApp::OnExit()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("xSMSDaemon exiting.");

    return 0;
}

bool xSMSDaemonApp::OnInit()
{
    static log4cpp::Category & logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("******* OnInit: xSMSDaemon started.");

    //(*AppInitialize
#ifdef __WXMSW__
    // This is here because the instance is inherited from xSchedule which then causes
    // the wxWindow class registration to fail
    // This code temporarily forces the hinstance to be different
    // I am assuming a similar trick is not needed on OSX/Linux
    auto inst = wxGetInstance();
    wxSetInstance((HINSTANCE)this);
#endif
    xSMSDaemonFrame* Frame = new xSMSDaemonFrame(0, __showDir);
    Frame->Show();
#ifdef __WXMSW__
    // set it back as it probably matters
    wxSetInstance(inst);
#endif
    return true;
}