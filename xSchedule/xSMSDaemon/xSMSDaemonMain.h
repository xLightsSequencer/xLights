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

#ifdef _MSC_VER

#include <stdlib.h>

//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#endif

#include "SMSService.h"

//(*Headers(xSMSDaemonFrame)
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/grid.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/timer.h>
//*)

#include "../../xLights/xLightsTimer.h"
#include "SMSDaemonOptions.h"
#include "xSMSDaemonApp.h"

#include <memory>

class wxDebugReportCompress;

class xSMSDaemonFrame : public wxFrame
{
    std::string _showDir;
    //std::string _xScheduleURL;
    SMSDaemonOptions _options;
    std::unique_ptr<SMSService> _smsService;
    p_xSchedule_Action _action;
    bool _suppressGridUpdate = false;
    std::vector<int> _rowIds;

    void RefreshList();
    void ValidateWindow();
    void SendReport(const wxString &loc, wxDebugReportCompress &report);
    std::string xScheduleShowDir();
    std::string xSMSDaemonShowDir();
    void LoadShowDir();
    void SaveShowDir() const;

    void LoadOptions();
    void SaveOptions();
    void Start();
    void Stop();
    bool IsOptionsValid() const;
    bool SetText(const std::string& t, const std::string& text, const std::wstring& wtext = _(""));
    void SetAllText(const std::string& text, const std::wstring& wtext = _(""));

public:

        xSMSDaemonFrame(wxWindow* parent, const std::string& showdir, const std::string& xScheduleURL, p_xSchedule_Action action, wxWindowID id = -1);
        virtual ~xSMSDaemonFrame();
        bool Action(const std::string& command, const std::wstring& parameters, const std::wstring& data, const std::wstring& reference, std::wstring& response);
        void NotifyStatus(const std::string& status);
        void UpdateModeration();
        void FireEvent(const std::string& eventType, const std::string& eventParameter) {}

    private:

        //(*Handlers(xSMSDaemonFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnMenuItem_OptionsSelected(wxCommandEvent& event);
        void OnMenuItem_ViewLogSelected(wxCommandEvent& event);
        void OnButton_PauseClick(wxCommandEvent& event);
        void OnButton_CloseClick(wxCommandEvent& event);
        void OnMenuItem_ShowFolderSelected(wxCommandEvent& event);
        void OnSendTimerTrigger(wxTimerEvent& event);
        void OnMenuItem_InsertTestMessagesSelected(wxCommandEvent& event);
        void OnTimer_SecondTrigger(wxTimerEvent& event);
        void OnClose(wxCloseEvent& event);
        void OnGrid1CellChanged(wxGridEvent& event);
        void OnGrid1CellSelect(wxGridEvent& event);
        //*)

        //(*Identifiers(xSMSDaemonFrame)
        static const long ID_STATICTEXT9;
        static const long ID_STATICTEXT10;
        static const long ID_STATICTEXT3;
        static const long ID_STATICTEXT4;
        static const long ID_STATICTEXT5;
        static const long ID_STATICTEXT6;
        static const long ID_STATICTEXT7;
        static const long ID_STATICTEXT8;
        static const long ID_GRID1;
        static const long ID_BUTTON2;
        static const long ID_MNU_OPTIONS;
        static const long ID_MNU_VIEWLOG;
        static const long ID_MNU_TESTMESSAGES;
        static const long idMenuAbout;
        static const long ID_TIMER2;
        static const long ID_TIMER_SECOND;
        //*)

        //(*Declarations(xSMSDaemonFrame)
        wxButton* Button_Pause;
        wxFlexGridSizer* FlexGridSizer1;
        wxGrid* Grid1;
        wxMenu* Menu1;
        wxMenu* Menu5;
        wxMenuItem* MenuItem_InsertTestMessages;
        wxMenuItem* MenuItem_Options;
        wxMenuItem* MenuItem_ViewLog;
        wxStaticText* StaticText2;
        wxStaticText* StaticText3;
        wxStaticText* StaticText4;
        wxStaticText* StaticText5;
        wxStaticText* StaticText_LastDisplayed;
        wxStaticText* StaticText_LastRetrieved;
        wxStaticText* StaticText_Phone;
        wxStaticText* StaticText_TextItemName;
        wxTimer SendTimer;
        wxTimer Timer_Second;
        //*)

        DECLARE_EVENT_TABLE()
};
