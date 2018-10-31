/***************************************************************
 * Name:      xCaptureMain.h
 * Purpose:   Defines Application Frame
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#ifndef XCAPTUREMAIN_H
#define XCAPTUREMAIN_H

#ifdef _MSC_VER

#include <stdlib.h>

//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#endif

//(*Headers(xCaptureFrame)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/timer.h>
//*)

#include "../xLights/xLightsTimer.h"
#include <list>
#include <wx/socket.h>

class wxDebugReportCompress;
class wxDatagramSocket;

class PacketData
{
public:
    wxDateTime _timeStamp;
    int _seq;
    int _length;
    wxByte* _pdata;
    int _frameTimeMS;
    virtual ~PacketData() { if (_pdata != nullptr) free(_pdata); }
    PacketData(long type, wxByte* packet, int len);
};

class Collector
{
public:
    int _universe;
    long _protocol;
    long _startChannel; // 1 based start channel
    std::list<PacketData*> _packets;
    virtual ~Collector();
    Collector(long type, int universe) { _startChannel = -1; _universe = universe; _protocol = type; }
    void AddPacket(long type, wxByte* packet, int len) { _packets.push_back(new PacketData(type, packet, len)); }
    void CalculateFrames(wxDateTime startTime, int frameMS);
    PacketData* GetPacket(long ms);
    bool operator<(const Collector& c) const;
};

class xCaptureFrame : public wxFrame
{
    void ValidateWindow();

    std::list<Collector*> _capturedData;
    wxDatagramSocket* _e131Socket;
    wxDatagramSocket* _artNETSocket;
    bool _capturing;
    long _capturedPackets;
    std::string _capturedDesc;
    wxString _localIP;
    wxString _defaultIP;

    void RestartInterfaces();
    void CloseSockets(bool force = false);
    void CreateE131Listener();
    void CreateArtNETListener();
    void AddUniverseRange(int low, int high);
    void PurgeCollectedData();
    void StashPacket(long type, wxByte* packet, int len);
    bool IsUniverseToBeCaptured(int universe, bool ignoreall = false);
    int GuessFrameMS();
    long GetChannelsPerFrame();
    wxDateTime GetStartTime();
    void SaveFSEQ(wxString file, int frameMS, long channelsPerFrame, int frames, wxString& log);
    void SaveESEQ(wxString file, int frameMS, long channelsPerFrame, int frames, wxString& log);
    int GetFrames();
    void UpdateCaptureDesc();
    void LoadState();
    void SaveState();

public:

        xCaptureFrame(wxWindow* parent, const std::string& showdir = "", const std::string& playlist = "", wxWindowID id = -1);
        virtual ~xCaptureFrame();
        void CreateDebugReport(wxDebugReportCompress *report);
        void SendReport(const wxString &loc, wxDebugReportCompress &report);

        static const long ID_E131SOCKET;
        static const long ID_ARTNETSOCKET;

private:

        //(*Handlers(xCaptureFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnResize(wxSizeEvent& event);
        void OnKeyDown(wxKeyEvent& event);
        void OnButton_StartStopClick(wxCommandEvent& event);
        void OnButton_SaveClick(wxCommandEvent& event);
        void OnCheckBox_TriggerOnChannelClick(wxCommandEvent& event);
        void OnListView_UniversesItemSelect(wxListEvent& event);
        void OnListView_UniversesItemActivated(wxListEvent& event);
        void OnButton_AllClick(wxCommandEvent& event);
        void OnButton_ClearClick(wxCommandEvent& event);
        void OnCheckBox_E131Click(wxCommandEvent& event);
        void OnCheckBox_ArtNETClick(wxCommandEvent& event);
        void OnButton_AddClick(wxCommandEvent& event);
        void OnButton_EditClick(wxCommandEvent& event);
        void OnButton_DeleteClick(wxCommandEvent& event);
        void OnUITimerTrigger(wxTimerEvent& event);
        void OnButton_AnalyseClick(wxCommandEvent& event);
        void OnButton1Click(wxCommandEvent& event);
        void OnChoice_TimingSelect(wxCommandEvent& event);
        //*)

        //(*Identifiers(xCaptureFrame)
        static const long ID_CHECKBOX_TRIGGERONCHANNEL;
        static const long ID_STATICTEXT1;
        static const long ID_SPINCTRL_UNIVERSE;
        static const long ID_STATICTEXT2;
        static const long ID_SPINCTRL_CHANNEL;
        static const long ID_STATICTEXT3;
        static const long ID_SPINCTRL_START;
        static const long ID_STATICTEXT4;
        static const long ID_SPINCTRL_END;
        static const long ID_STATICLINE1;
        static const long ID_STATICTEXT5;
        static const long ID_CHECKBOX_E131;
        static const long ID_CHECKBOX_ARTNET;
        static const long ID_STATICTEXT7;
        static const long ID_STATICTEXT8;
        static const long ID_BUTTON9;
        static const long ID_STATICTEXT6;
        static const long ID_STATICLINE2;
        static const long ID_LISTVIEW_UNIVERSES;
        static const long ID_BUTTON6;
        static const long ID_BUTTON3;
        static const long ID_BUTTON4;
        static const long ID_BUTTON5;
        static const long ID_STATICTEXT9;
        static const long ID_CHOICE1;
        static const long ID_SPINCTRL1;
        static const long ID_BUTTON1;
        static const long ID_BUTTON8;
        static const long ID_BUTTON2;
        static const long ID_BUTTON7;
        static const long ID_STATUSBAR1;
        static const long ID_TIMER1;
        //*)

        //(*Declarations(xCaptureFrame)
        wxButton* Button1;
        wxButton* Button_Add;
        wxButton* Button_All;
        wxButton* Button_Analyse;
        wxButton* Button_Clear;
        wxButton* Button_Delete;
        wxButton* Button_Edit;
        wxButton* Button_Save;
        wxButton* Button_StartStop;
        wxCheckBox* CheckBox_ArtNET;
        wxCheckBox* CheckBox_E131;
        wxCheckBox* CheckBox_TriggerOnChannel;
        wxChoice* Choice_Timing;
        wxListView* ListView_Universes;
        wxSpinCtrl* SpinCtrl_Channel;
        wxSpinCtrl* SpinCtrl_ManualTime;
        wxSpinCtrl* SpinCtrl_TriggerStart;
        wxSpinCtrl* SpinCtrl_TriggerStop;
        wxSpinCtrl* SpinCtrl_Universe;
        wxStaticLine* StaticLine1;
        wxStaticLine* StaticLine2;
        wxStaticText* StaticText1;
        wxStaticText* StaticText2;
        wxStaticText* StaticText3;
        wxStaticText* StaticText4;
        wxStaticText* StaticText5;
        wxStaticText* StaticText6;
        wxStaticText* StaticText7;
        wxStaticText* StaticText8;
        wxStaticText* StaticText_IP;
        wxStatusBar* StatusBar1;
        wxTimer UITimer;
        //*)

        DECLARE_EVENT_TABLE()

        void OnE131SocketEvent(wxSocketEvent& event);
        void OnArtNETSocketEvent(wxSocketEvent& event);
};

#endif // xCAPTUREMAIN_H
