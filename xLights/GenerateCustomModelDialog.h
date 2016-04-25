#ifndef GENERATECUSTOMMODELDIALOG_H
#define GENERATECUSTOMMODELDIALOG_H

#include "VideoReader.h"
#include "xlights_out.h"

//(*Headers(GenerateCustomModelDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobox.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/grid.h>
#include <wx/filedlg.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/gauge.h>
//*)

#ifdef __WXOSX__
#include "osxMacUtils.h"
#endif

#include <wx/xml/xml.h>
#include <wx/progdlg.h>
#include <wx/generic/statbmpg.h>

#include <list>
#include <map>

#pragma region xlPoint

// This is required so I can use points in std::maps
class xlPoint : public wxPoint
{
public:
    xlPoint(int x, int y) : wxPoint(x, y) {}
    bool operator<(const xlPoint& r)
    {
        if (x < r.x) return true;
        if (y < r.y) return true;
        return false;
    }
    friend bool operator<(const xlPoint& l, const xlPoint& r)
    {
        if (l.x < r.x) return true;
        if (l.y < r.y) return true;
        return false;
    }
    operator wxPoint() const { return wxPoint(x,y); }
    operator wxPoint() { return wxPoint(x, y); }
};

#pragma endregion xlPoint

class MyGenericStaticBitmap;

class GenerateCustomModelDialog: public wxDialog
{
    // variables passed into us
    wxXmlDocument* _network;

#pragma region Prepare Tab
    wxDateTime _starttime;

    void SetBulbs(bool nodes, int count, int startch, int node, int ms, int intensity, xOutput* xout);
    bool InitialiseOutputs(xOutput* xOut);
#pragma endregion Prepare Tab

#pragma region Generate Tab
    enum VideoProcessingStates
    {
        CHOOSE_VIDEO,
        FINDING_START_FRAME,
        CIRCLING_BULBS,
        IDENTIFYING_BULBS,
        REVIEW_CUSTOM_MODEL
    };

    wxImage _displaybmp; // the image we are displaying on the screen
    int _startframetime; // time in MS in the video the start frame occurs at
    wxImage _startFrame; // the image of the start frame
    wxImage _greyFrame;  // the greyscale version of the start frame
    wxImage _bwFrame;    // the black and white version of the image after blur & level
    wxImage _cbFrame;    // the Circle bulbs image - edges
    wxImage _biFrame;    // the Bulb identify output - essentially a mask
    VideoProcessingStates _state;
    VideoReader* _vr;
    MyGenericStaticBitmap* StaticBitmap_Preview;
    bool _busy;          // true while busy and re-entrancy possible

    void UpdateProgress(wxProgressDialog& pd, int totaltime);
    void ShowFrame(int time);
    wxImage CreateImageFromFrame(AVFrame* frame);
    void ShowImage(const wxImage& image);
    void SwapPage(int oldp, int newp);

#pragma region Choose Video Tab
    void CVTabEntry();
#pragma endregion Choose Video Tab

#pragma region Start Frame Tab
    void DoStartFrameIdentify();
    void SetStartFrame(int time);
    void SFTabEntry();
    float CalcFrameBrightness(wxImage& image);
    int FindStartFrame(VideoReader* vr);
    void ValidateStartFrame();
    bool LooksLikeStartFrame(int candidateframe);
    void MoveStartFrame(int by);
#pragma endregion Start Frame Tab

#pragma region Circle Bulbs Tab
    void DoBulbCircle();
    wxImage OutlineBulbs();
    void ApplyThreshold(wxImage& image, int threshold);
    wxImage DetectEdges(wxImage& image);
    void CBTabEntry();
#pragma endregion Circle Bulbs Tab

#pragma region Identify Bulbs Tab
    void DoBulbIdentify();
    void BITabEntry();
    float CalcPoint(wxImage& edge, int x0, int y0, int radius);
    std::map<xlPoint, int> CircleDetect(wxImage& mask, wxImage& edge, int radius);
    std::list<wxPoint> CircleDetect(wxImage& mask, wxImage& edge, int minr, int maxr);
    std::list<wxPoint> FindLights(wxImage& image);
    wxImage CreateDetectMask(std::list<wxPoint> centres, wxImage ref, bool includeimage, wxColor col);
#pragma endregion Identify Bulbs Tab

#pragma endregion Generate Tab

    void ValidateWindow();

#ifdef __WXOSX__
    AppNapSuspender _sleepData;
    void EnableSleepModes()
    {
        _sleepData.resume();
    }
    void DisableSleepModes()
    {
        _sleepData.suspend();
    }
#else
    void EnableSleepModes() {}
    void DisableSleepModes() {}
#endif

    public:
		GenerateCustomModelDialog(wxWindow* parent, wxXmlDocument* network, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~GenerateCustomModelDialog();

		//(*Declarations(GenerateCustomModelDialog)
		wxStaticText* StaticText10;
		wxPanel* Panel_BulbIdentify;
		wxStaticText* StaticText9;
		wxButton* Button_Forward10Frames;
		wxSlider* Slider_LevelFilterAdjust;
		wxSlider* Slider_BI_MinRadius;
		wxSlider* Slider_BI_MaxRadius;
		wxButton* Button_BI_Back;
		wxButton* Button_SF_Back;
		wxButton* Button_Back1Frame;
		wxStaticText* StaticText13;
		wxButton* Button_CB_RestoreDefault;
		wxStaticText* StaticText14;
		wxButton* Button_CM_Back;
		wxAuiNotebook* AuiNotebook_ProcessSettings;
		wxSlider* Slider_Intensity;
		wxStaticText* StaticText11;
		wxGrid* Grid_CM_Result;
		wxButton* Button_PCM_Run;
		wxStaticText* StaticText18;
		wxButton* Button_CV_Next;
		wxSpinCtrl* SpinCtrl_NC_Count;
		wxFileDialog* FileDialog1;
		wxButton* Button_Back10Frames;
		wxPanel* Panel_BulbCircle;
		wxGauge* Gauge_Progress;
		wxTextCtrl* TextCtrl_GCM_Filename;
		wxPanel* Panel_ChooseVideo;
		wxSlider* Slider_AdjustBlur;
		wxButton* Button_GCM_SelectFile;
		wxPanel* Panel_Generate;
		wxAuiNotebook* AuiNotebook1;
		wxButton* Button_SF_Next;
		wxStaticText* StaticText_StartFrameOk;
		wxPanel* Panel_CustomModel;
		wxFileDialog* FileDialog2;
		wxButton* Button_BI_Next;
		wxButton* Button_BD_Back;
		wxStaticText* StaticText15;
		wxRadioBox* RadioBox1;
		wxStaticText* StaticText12;
		wxPanel* Panel_Prepare;
		wxSlider* Slider_BI_Sensitivity;
		wxPanel* Panel_StartFrame;
		wxStaticText* StaticText17;
		wxButton* Button_CM_Save;
		wxSpinCtrl* SpinCtrl_StartChannel;
		wxButton* Button_BD_Next;
		wxStaticText* StaticText16;
		wxButton* Button_Forward1Frame;
		//*)

	protected:

        static const long ID_STATICBITMAP_Preview;

		//(*Identifiers(GenerateCustomModelDialog)
		static const long ID_RADIOBOX1;
		static const long ID_SPINCTRL_NC_Count;
		static const long ID_SPINCTRL_StartChannel;
		static const long ID_SLIDER_Intensity;
		static const long ID_BUTTON_PCM_Run;
		static const long ID_PANEL_Prepare;
		static const long ID_GAUGE_Progress;
		static const long ID_STATICTEXT10;
		static const long ID_TEXTCTRL_GCM_Filename;
		static const long ID_BUTTON_GCM_SelectFile;
		static const long ID_BUTTON_CV_Next;
		static const long ID_PANEL_ChooseVideo;
		static const long ID_STATICTEXT3;
		static const long ID_BUTTON_Back1Frame;
		static const long ID_BUTTON_Forward1Frame;
		static const long ID_BUTTON_Back10Frames;
		static const long ID_BUTTON_Forward10Frames;
		static const long ID_STATICTEXT_StartFrameOk;
		static const long ID_BUTTON_SF_Back;
		static const long ID_BUTTON_SF_Next;
		static const long ID_PANEL_StartFrame;
		static const long ID_STATICTEXT4;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_AdjustBlur;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_LevelFilterAdjust;
		static const long ID_BUTTON_CB_RestoreDefault;
		static const long ID_BUTTON_BD_Back;
		static const long ID_BUTTON_BD_Next;
		static const long ID_PANEL_BulbCircle;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT6;
		static const long ID_SLIDER_BI_MinRadius;
		static const long ID_STATICTEXT7;
		static const long ID_SLIDER_BI_MaxRadius;
		static const long ID_STATICTEXT8;
		static const long ID_SLIDER_BI_Sensitivity;
		static const long ID_BUTTON_BI_Back;
		static const long ID_BUTTON_BI_Next;
		static const long ID_PANEL_BulbIdentify;
		static const long ID_STATICTEXT9;
		static const long ID_GRID_CM_Result;
		static const long ID_BUTTON_CM_Back;
		static const long ID_BUTTON_CM_Save;
		static const long ID_PANEL_CustomModel;
		static const long ID_AUINOTEBOOK_ProcessSettings;
		static const long ID_PANEL_Generate;
		static const long ID_AUINOTEBOOK1;
		//*)

	private:

		//(*Handlers(GenerateCustomModelDialog)
		void OnButton_GCM_SelectFileClick(wxCommandEvent& event);
		void OnTextCtrl_GCM_FilenameText(wxCommandEvent& event);
		void OnButton_PCM_RunClick(wxCommandEvent& event);
		void OnButton_ContinueClick(wxCommandEvent& event);
		void OnButton_BackClick(wxCommandEvent& event);
		void OnButton_NextClick(wxCommandEvent& event);
		void OnButton_SF_NextClick(wxCommandEvent& event);
		void OnButton_BD_BackClick(wxCommandEvent& event);
		void OnButton_BD_NextClick(wxCommandEvent& event);
		void OnSlider_LevelFilterAdjustCmdScroll(wxScrollEvent& event);
		void OnButton_Back1FrameClick(wxCommandEvent& event);
		void OnButton_Forward1FrameClick(wxCommandEvent& event);
		void OnButton_Back10FramesClick(wxCommandEvent& event);
		void OnButton_Forward10FramesClick(wxCommandEvent& event);
		void OnPanel_GeneratePaint(wxPaintEvent& event);
		void OnSlider_AdjustBlurCmdScroll(wxScrollEvent& event);
		void OnSlider1CmdScroll(wxScrollEvent& event);
		void OnButton_CM_BackClick(wxCommandEvent& event);
		void OnButton_CM_SaveClick(wxCommandEvent& event);
		void OnSlider_BI_MinRadiusCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_BI_MaxRadiusCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_BI_SensitivityCmdSliderUpdated(wxScrollEvent& event);
		void OnButton_BI_NextClick(wxCommandEvent& event);
		void OnButton_BI_BackClick(wxCommandEvent& event);
		void OnButton_CV_NextClick(wxCommandEvent& event);
		void OnButton_SF_BackClick(wxCommandEvent& event);
		void OnAuiNotebook_ProcessSettingsPageChanging(wxAuiNotebookEvent& event);
		void OnButton_CB_RestoreDefaultClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
