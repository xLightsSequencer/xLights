#ifndef XSCHEDULE_PLUGIN_H
#define XSCHEDULE_PLUGIN_H

class wxWindow;

#include <string>

extern "C" {

    typedef bool (*p_xSchedule_Action)(const char* command, const wchar_t* parameters, const char* data, char* buffer, size_t bufferSize);

    // always called when the dll is found ... should not actually do anything
    bool __declspec(dllimport) xSchedule_Load(char* showDir);
    typedef bool (*p_xSchedule_Load)(char* showDir);

    bool __declspec(dllimport) xSchedule_WipeSettings();
    typedef bool (*p_xSchedule_WipeSettings)();

    void __declspec(dllimport) xSchedule_GetVirtualWebFolder(char* buffer, size_t bufferSize);
    typedef void (*p_xSchedule_GetVirtualWebFolder)(char* buffer, size_t bufferSize);

    void __declspec(dllimport) xSchedule_GetMenuLabel(char* buffer, size_t bufferSize);
    typedef void (*p_xSchedule_GetMenuLabel)(char* buffer, size_t bufferSize);

    bool __declspec(dllimport) xSchedule_HandleWeb(const char* action, const wchar_t* parms, const wchar_t* data, const wchar_t* reference, wchar_t* response, size_t responseSize);
    typedef bool (*p_xSchedule_HandleWeb)(const char* action, const wchar_t* parms, const wchar_t* data, const wchar_t* reference, wchar_t* response, size_t responseSize);

    // called when we want the plugin to actually interact with the user
    bool __declspec(dllimport) xSchedule_Start(char* showDir, char* xScheduleURL, p_xSchedule_Action action);
    typedef bool (*p_xSchedule_Start)(char* showDir, char* xScheduleURL, p_xSchedule_Action action);

    // called when we want the plugin to exit
    void __declspec(dllimport) xSchedule_Stop();
    typedef void (*p_xSchedule_Stop)();

    // called just before xSchedule exits
    void __declspec(dllimport) xSchedule_Unload();
    typedef void (*p_xSchedule_Unload)();
}
#endif