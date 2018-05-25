/***************************************************************
 * Name:      xFadeMain.h
 * Purpose:   Defines Application Frame
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#ifndef XFADEMAIN_H
#define XFADEMAIN_H

#ifdef _MSC_VER

#include <stdlib.h>
#include "PacketData.h"

//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#endif

//(*Headers(xFadeFrame)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
//*)

#include "../xLights/xLightsTimer.h"
#include <map>
#include <wx/socket.h>
#include <mutex>
#include "PacketData.h"

class wxDebugReportCompress;
class wxDatagramSocket;
class Emitter;

class xFadeFrame : public wxFrame
{
    const int JUKEBOXBUTTONS = 50;
    const int BUTTONWIDTH = 20;
    const int BUTTONHEIGHT = 20;

    void ValidateWindow();
    void AddFadeTimeButton(std::string label);

    std::map<int, std::string> _targetIP;
    std::map<int, std::string> _targetDesc;
    std::map<int, std::string> _targetProtocol;
    std::map<int, PacketData> _leftData;
    std::map<int, PacketData> _rightData;
    wxString _leftTag;
    wxString _rightTag;
    wxDatagramSocket* _e131SocketReceive;
    wxDatagramSocket* _artNETSocketReceive;
    wxString _localInputIP;
    wxString _localOutputIP;
    wxString _defaultIP;
    Emitter* _emitter;
    wxColor _defaultColour;
    std::mutex _lock;
    int _direction; // auto fade direction

    std::string ExtractE131Tag(wxByte* packet);
    void SetFade();
    void SetTiming();
    void RestartInterfaces();
    void CloseSockets(bool force = false);
    void CreateE131Listener();
    void CreateArtNETListener();
    void AddUniverseRange(int low, int high, std::string ipAddress, std::string desc, std::string protocol);
    void StashPacket(long type, wxByte* packet, int len);
    bool IsLeft(long type, wxByte* packet, int len);
    bool IsRight(long type, wxByte* packet, int len);
    bool IsUniverseToBeCaptured(int universe);
    void LoadState();
    void SaveState();
    void LoadUniverses();
    void SetMIDIForControl(wxString controlName, float parm = 0.0);

public:

        xFadeFrame(wxWindow* parent, wxWindowID id = -1);
        virtual ~xFadeFrame();
        void CreateDebugReport(wxDebugReportCompress *report);
        void SendReport(const wxString &loc, wxDebugReportCompress &report);

        static const long ID_E131SOCKET;
        static const long ID_ARTNETSOCKET;

private:

        //(*Handlers(xFadeFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnResize(wxSizeEvent& event);
        void OnKeyDown(wxKeyEvent& event);
        void OnListView_UniversesItemSelect(wxListEvent& event);
        void OnListView_UniversesItemActivated(wxListEvent& event);
        void OnCheckBox_E131Click(wxCommandEvent& event);
        void OnCheckBox_ArtNETClick(wxCommandEvent& event);
        void OnButton_AddClick(wxCommandEvent& event);
        void OnButton_EditClick(wxCommandEvent& event);
        void OnButton_DeleteClick(wxCommandEvent& event);
        void OnUITimerTrigger(wxTimerEvent& event);
        void OnListView_UniversesItemDeselect(wxListEvent& event);
        void OnChoice_FrameTimingSelect(wxCommandEvent& event);
        void OnButton_ForceInputClick(wxCommandEvent& event);
        void OnButton_ForceOutputClick(wxCommandEvent& event);
        void OnButton_MiddleClick(wxCommandEvent& event);
        void OnButton_LeftClick(wxCommandEvent& event);
        void OnButton_RightClick(wxCommandEvent& event);
        void OnButton_ConnectToxLightsClick(wxCommandEvent& event);
        void OnSlider1CmdSliderUpdated(wxScrollEvent& event);
        //*)

        //(*Identifiers(xFadeFrame)
        static const long ID_STATICTEXT6;
        static const long ID_TEXTCTRL3;
        static const long ID_STATICTEXT10;
        static const long ID_TEXTCTRL4;
        static const long ID_PANEL1;
        static const long ID_BUTTON2;
        static const long ID_STATICTEXT5;
        static const long ID_CHECKBOX_E131;
        static const long ID_CHECKBOX_ARTNET;
        static const long ID_STATICTEXT7;
        static const long ID_STATICTEXT8;
        static const long ID_BUTTON9;
        static const long ID_STATICTEXT2;
        static const long ID_STATICTEXT3;
        static const long ID_BUTTON12;
        static const long ID_STATICTEXT9;
        static const long ID_CHOICE1;
        static const long ID_STATICLINE1;
        static const long ID_STATICTEXT1;
        static const long ID_TEXTCTRL1;
        static const long ID_STATICTEXT14;
        static const long ID_PANEL3;
        static const long ID_BUTTON1;
        static const long ID_BUTTON10;
        static const long ID_SLIDER1;
        static const long ID_BUTTON11;
        static const long ID_STATICLINE2;
        static const long ID_LISTVIEW_UNIVERSES;
        static const long ID_BUTTON3;
        static const long ID_BUTTON4;
        static const long ID_BUTTON5;
        static const long ID_STATICTEXT4;
        static const long ID_TEXTCTRL2;
        static const long ID_STATICTEXT11;
        static const long ID_TEXTCTRL5;
        static const long ID_PANEL2;
        static const long ID_STATUSBAR1;
        static const long ID_TIMER1;
        //*)

        //(*Declarations(xFadeFrame)
        wxButton* Button_Add;
        wxButton* Button_ConnectToxLights;
        wxButton* Button_Delete;
        wxButton* Button_Edit;
        wxButton* Button_ForceInput;
        wxButton* Button_ForceOutput;
        wxButton* Button_Left;
        wxButton* Button_Middle;
        wxButton* Button_Right;
        wxCheckBox* CheckBox_ArtNET;
        wxCheckBox* CheckBox_E131;
        wxChoice* Choice_FrameTiming;
        wxGridSizer* GridSizer_TimePresets;
        wxListView* ListView_Universes;
        wxPanel* Panel_FadeTime;
        wxPanel* Panel_Left;
        wxPanel* Panel_Right;
        wxSlider* Slider1;
        wxStaticLine* StaticLine1;
        wxStaticLine* StaticLine2;
        wxStaticText* StaticText10;
        wxStaticText* StaticText1;
        wxStaticText* StaticText2;
        wxStaticText* StaticText3;
        wxStaticText* StaticText4;
        wxStaticText* StaticText5;
        wxStaticText* StaticText6;
        wxStaticText* StaticText7;
        wxStaticText* StaticText8;
        wxStaticText* StaticText9;
        wxStaticText* StaticText_InputIP;
        wxStaticText* StaticText_OutputIP;
        wxStatusBar* StatusBar1;
        wxTextCtrl* TextCtrl_CrossFadeTime;
        wxTextCtrl* TextCtrl_LeftSequence;
        wxTextCtrl* TextCtrl_LeftTag;
        wxTextCtrl* TextCtrl_RightSequence;
        wxTextCtrl* TextCtrl_RightTag;
        xLightsTimer UITimer;
        //*)

        DECLARE_EVENT_TABLE()

        void OnE131SocketEvent(wxSocketEvent& event);
        void OnArtNETSocketEvent(wxSocketEvent& event);
        void OnButtonClickLeft(wxCommandEvent& event);
        void OnButtonRClickLeft(wxContextMenuEvent& event);
        void OnButtonClickRight(wxCommandEvent& event);
        void OnButtonRClickRight(wxContextMenuEvent& event);
        void OnTextCtrlRClickCrossFadeTime(wxContextMenuEvent& event);

        void OnButtonClickFT(wxCommandEvent& event);
        void OnButtonRClickFT(wxContextMenuEvent& event);

        void OnButtonRClickFadeLeft(wxContextMenuEvent& event);
        void OnButtonRClickFadeMiddle(wxContextMenuEvent& event);
        void OnButtonRClickFadeRight(wxContextMenuEvent& event);
};

#endif // XFADEMAIN_H
