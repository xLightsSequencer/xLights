#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

class wxWindow;

#include <wx/wx.h>
#include <string>

extern "C" {

    typedef bool (*p_xSchedule_Action)(const char* command, const wchar_t* parameters, const char* data, char* buffer, size_t bufferSize);

    // always called when the dll is found ... should not actually do anything
    bool WXIMPORT xSchedule_Load(char* showDir);
    typedef bool (*p_xSchedule_Load)(char* showDir);

    void WXIMPORT xSchedule_WipeSettings();
    typedef void (*p_xSchedule_WipeSettings)();

    void WXIMPORT xSchedule_GetVirtualWebFolder(char* buffer, size_t bufferSize);
    typedef void (*p_xSchedule_GetVirtualWebFolder)(char* buffer, size_t bufferSize);

    void WXIMPORT xSchedule_GetMenuLabel(char* buffer, size_t bufferSize);
    typedef void (*p_xSchedule_GetMenuLabel)(char* buffer, size_t bufferSize);

    bool WXIMPORT xSchedule_HandleWeb(const char* action, const wchar_t* parms, const wchar_t* data, const wchar_t* reference, wchar_t* response, size_t responseSize);
    typedef bool (*p_xSchedule_HandleWeb)(const char* action, const wchar_t* parms, const wchar_t* data, const wchar_t* reference, wchar_t* response, size_t responseSize);

    // called when we want the plugin to actually interact with the user
    bool WXIMPORT xSchedule_Start(char* showDir, char* xScheduleURL, p_xSchedule_Action action);
    typedef bool (*p_xSchedule_Start)(char* showDir, char* xScheduleURL, p_xSchedule_Action action);

    void WXIMPORT xSchedule_ManipulateBuffer(uint8_t* buffer, size_t bufferSize);
    typedef void (*p_xSchedule_ManipulateBuffer)(uint8_t* buffer, size_t bufferSize);

    void WXIMPORT xSchedule_NotifyStatus(const char* statusJSON);
    typedef void (*p_xSchedule_NotifyStatus)(const char* statusJSON);

    // called when we want the plugin to exit
    void WXIMPORT xSchedule_Stop();
    typedef void (*p_xSchedule_Stop)();

    // called just before xSchedule exits
    void WXIMPORT xSchedule_Unload();
    typedef void (*p_xSchedule_Unload)();

    // called when an event occurs
    void WXIMPORT xSchedule_FireEvent(const char* eventType, const char* eventParam);
    typedef bool (*p_xSchedule_FireEvent)(const char* eventType, const char* eventParam);
}
