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
//*)

#include <list>
#include <wx/socket.h>
#include <wx/treelist.h>

#include "Scanner.h"

class wxDebugReportCompress;
class wxTreeListCtrl;
class wxProgressDialog;

wxDECLARE_EVENT(EVT_SCANPROGRESS, wxCommandEvent);

class ScanThread : public wxThread
{
    virtual ExitCode Entry();
    wxWindow* _frame;
public:
    ScanThread(wxWindow* frame)
        : wxThread(wxTHREAD_DETACHED)
    {
        _frame = frame;
    }
    ~ScanThread() {}
};

class xScannerFrame : public wxFrame
{
    wxProgressDialog* _progress = nullptr;
    Scanner _scanner;
    wxTreeListCtrl* _tree = nullptr;
    ScanThread* _thread = nullptr;

    void ValidateWindow();
    void PurgeCollectedData();
    void Scan();
    void AddIP(wxTreeListItem ti, const IPObject& ip);
    void LoadScanResults();

public:

        xScannerFrame(wxWindow* parent, const std::string& showdir = "", const std::string& playlist = "", wxWindowID id = -1);
        virtual ~xScannerFrame();
        void CreateDebugReport(wxDebugReportCompress *report);
        void SendReport(const wxString &loc, wxDebugReportCompress &report);
        void ExportItem(int skip, wxTreeListItem& item, wxFile& f);
        void ScanUpdate(wxCommandEvent& event);
        void DoScan();

        static const long ID_E131SOCKET;
        static const long ID_ARTNETSOCKET;

private:

        //(*Handlers(xScannerFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnResize(wxSizeEvent& event);
        void OnKeyDown(wxKeyEvent& event);
        void OnMenuItemScanSelected(wxCommandEvent& event);
        //*)

        void OnTreeItemActivated(wxTreeListEvent& event);
        void OnTreeRClick(wxTreeListEvent& event);
        void OnPopup(wxCommandEvent& event);

        //(*Identifiers(xScannerFrame)
        static const long ID_STATUSBAR1;
        static const long Network;
        //*)

        static const long ID_MNU_EXPORT;

        //(*Declarations(xScannerFrame)
        wxMenu Menu1;
        wxMenuItem* MenuItemScan;
        wxStatusBar* StatusBar1;
        //*)

        DECLARE_EVENT_TABLE()
};
