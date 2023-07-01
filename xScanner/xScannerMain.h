/***************************************************************
 * Name:      xScannerMain.h
 * Purpose:   Defines Application Frame
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#pragma once

#ifdef _MSC_VER

#include <stdlib.h>

//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#endif

//(*Headers(xScannerFrame)
#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/statusbr.h>
#include <wx/timer.h>
//*)

#include <list>
#include <wx/socket.h>
#include <wx/treelist.h>
#include <wx/file.h>

#include "../common/xlBaseApp.h"
#include "ScanWork.h"

class wxDebugReportCompress;
class wxTreeListCtrl;
class wxProgressDialog;

wxDECLARE_EVENT(EVT_SCANPROGRESS, wxCommandEvent);

enum class TITLE_PRIORITY {
    TP_XSCHEDULE,
    TP_COMPUTER_NAME,
    TP_CONTROLLER_NAME,
    TP_CONTROLLER_FALCON,
    TP_CONTROLLER_VMV,
    TP_DISCOVER,
    TP_CONTROLLER_FPP,
    TP_HTTP_CONTROLLER,
    TP_HTTP_TITLE,
    TP_MAC,
    TP_NONE
};

class xScannerFrame : public xlFrame
{
    std::pair<std::string, std::string> nullPair = { "","" };
    wxTreeListCtrl* _tree = nullptr;
    wxTreeListItem _item;
    WorkManager _workManager;
    void Scan();
    std::string GetItem(std::list<std::pair<std::string, std::string>>& res, const std::string& label);
    std::string GetIPSubnet(const std::string& ip);
    wxTreeListItem GetSubnetItem(const std::string& subnet);
    wxTreeListItem GetIPItem(const std::string& ip, bool create = true);
    wxString GetItemUnderParent(wxTreeListItem& parent, const std::string& label) const;
    wxTreeListItem AddItemUnderParent(wxTreeListItem& parent, const std::string& label, const std::string& value);
    void AddItemUnderParentIfNotBlank(wxTreeListItem& item, const std::string& label, const std::string& value);
    std::list<std::string> GetStartsWith(std::list<std::pair<std::string, std::string>>& res, const std::string& prefix);

    void ValidateWindow();
    void UpdateDeviceTitle(wxTreeListCtrl* tree, wxTreeListItem& ti, TITLE_PRIORITY tp, const std::string& name);
        //void AddIP(wxTreeListItem ti, const IPObject& ip);

    void AddtoxLights(wxTreeListItem& item);

public:

        xScannerFrame(wxWindow* parent, bool singleThreaded, wxWindowID id = -1);
        virtual ~xScannerFrame();
        virtual void CreateDebugReport(xlCrashHandler* crashHandler) override;
        void ExportItem(int skip, wxTreeListItem& item, wxFile& f);
        void ProcessScanResult(std::list<std::pair<std::string, std::string>>& res);
        void ProcessScanResults();
        void ProcessComputerResult(std::list<std::pair<std::string, std::string>>& res);
        void ProcessPingResult(std::list<std::pair<std::string, std::string>>& res);
        void ProcessHTTPResult(std::list<std::pair<std::string, std::string>>& res);
        void ProcessFPPResult(std::list<std::pair<std::string, std::string>>& res);
        void ProcessFalconResult(std::list<std::pair<std::string, std::string>>& res);
        void ProcessMACResult(std::list<std::pair<std::string, std::string>>& res);
        void ProcessDiscoverResult(std::list<std::pair<std::string, std::string>>& res);
        void ProcessxScheduleResult(std::list<std::pair<std::string, std::string>>& res);
        void ProcessControllerResult(std::list<std::pair<std::string, std::string>>& res);
        void ProcessProxiedResult(std::list<std::pair<std::string, std::string>>& res);

        static const long ID_E131SOCKET;
        static const long ID_ARTNETSOCKET;

private:

        //(*Handlers(xScannerFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnResize(wxSizeEvent& event);
        void OnKeyDown(wxKeyEvent& event);
        void OnMenuItemScanSelected(wxCommandEvent& event);
        void OnTimer1Trigger(wxTimerEvent& event);
        //*)

        void OnTreeItemActivated(wxTreeListEvent& event);
        void OnTreeRClick(wxTreeListEvent& event);
        void OnPopup(wxCommandEvent& event);

        //(*Identifiers(xScannerFrame)
        static const long ID_STATUSBAR1;
        static const long Network;
        static const long ID_TIMER1;
        //*)

        static const long ID_MNU_EXPORT;
        static const long ID_MNU_RESCAN;
        static const long ID_MNU_ADDTOXLIGHTS;

        //(*Declarations(xScannerFrame)
        wxMenu Menu1;
        wxMenuItem* MenuItemScan;
        wxStatusBar* StatusBar1;
        wxTimer Timer1;
        //*)

        DECLARE_EVENT_TABLE()
};
