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

#include "VideoReader.h"

class OutputManager;

//(*Headers(GenerateCustomModelDialog)
#include <wx/aui/aui.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/filedlg.h>
#include <wx/gauge.h>
#include <wx/grid.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#ifdef __WXOSX__
#include "osxMacUtils.h"
#endif

#include <wx/xml/xml.h>
#include <wx/progdlg.h>
#include <wx/dcmemory.h>

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
};

#pragma endregion xlPoint

class MyGenericStaticBitmap;

class GCMBulb
{
    wxPoint _location;
    int _num;
    int _brightness;
    bool _suppressoutsideclip;
    bool _suppressduplicate;
    bool _suppresstooclose;

public:
    GCMBulb(wxPoint pt, int num, int brightness) {
        _location = pt;
        _num = num;
        _brightness = brightness;
        _suppressoutsideclip = false;
        _suppressduplicate = false;
        _suppresstooclose = false;
    }
    void TooClose()
    {
        _suppresstooclose = true;
    }
    void Duplicate()
    {
        _suppressduplicate = true;
    }
    void OutsideClip()
    {
        _suppressoutsideclip = true;
    }
    void Reset()
    {
        _suppressoutsideclip = false;
        _suppressduplicate = false;
        _suppresstooclose = false;
    }
    bool isSupressed()
    {
        return _suppressoutsideclip || _suppressduplicate || _suppresstooclose;
    }
    bool isSupressedButDraw()
    {
        return _suppressoutsideclip || _suppresstooclose;
    }
    void SetLocation(int x, int y) { _location = wxPoint(x, y); }
    void Draw(wxMemoryDC& dc, float factor)
    {
        int diameter = 2 * factor;
        if (isSupressedButDraw())
        {
            wxBrush b(*wxBLUE, wxBrushStyle::wxBRUSHSTYLE_SOLID);
            dc.SetBrush(b);
            wxPen p(*wxBLUE, 1);
            dc.SetPen(p);
            dc.DrawCircle(_location, diameter);
        }
        else if (!isSupressed())
        {
            wxBrush b(*wxRED, wxBrushStyle::wxBRUSHSTYLE_SOLID);
            dc.SetBrush(b);
            wxPen p(*wxRED, 1);
            dc.SetPen(p);
            dc.DrawCircle(_location, diameter);
        }
    }
    int GetBrightness() {
        return _brightness;
    }
    int GetNum() {
        return _num;
    }
    inline wxPoint GetLocation(float scale = 1.0, wxPoint trim = wxPoint(0, 0))
    {
        return wxPoint((_location.x - trim.x) * scale, (_location.y - trim.y) * scale);
    }
    inline bool IsSameLocation(GCMBulb& r, float scale = 1.0, wxPoint trim = wxPoint(0,0))
    {
        wxPoint rloc = r.GetLocation(scale, trim);
        wxPoint lloc = GetLocation(scale, trim);
        return lloc.x == rloc.x && lloc.y == rloc.y;
    }
};

class GenerateCustomModelDialog: public wxDialog
{
    // variables passed into us
    OutputManager* _outputManager;

#pragma region Prepare Tab
    wxDateTime _starttime;

    void SetBulbs(bool nodes, int count, int startch, int node, int ms, int intensity);
#pragma endregion Prepare Tab

#pragma region Generate Tab
    enum VideoProcessingStates
    {
        CHOOSE_MODELTYPE,
        CHOOSE_VIDEO,
        FINDING_START_FRAME,
        CIRCLING_BULBS,
        IDENTIFYING_BULBS,
        IDENTIFYING_MANUAL,
        REVIEW_CUSTOM_MODEL
    };

    wxImage _displaybmp; // the image we are displaying on the screen
    int _startframetime; // time in MS in the video the start frame occurs at
    float _startframebrightness;
    wxImage _startFrame; // the image of the start frame
    wxImage _darkFrame; // an image with no lights on
    wxImage _biFrame;    // the Bulb identify output - essentially a mask
    std::list<GCMBulb> _lights; // our lights
    VideoProcessingStates _state;
    VideoReader* _vr;
    MyGenericStaticBitmap* StaticBitmap_Preview;
    bool _busy;          // true while busy and re-entrancy possible
    wxPoint _trim;
    float _scale;
    wxSize _size;
    bool _warned;
    int _draggingedge;
    wxRect _clip;
    float _overallmaxbrightness;
    float _overallaveragebrightness;
    bool _manual = false;
    int _MI_CurrentNode;
    wxImage _MI_CurrentFrame;
    int _MI_CurrentTime;

    void UpdateProgress(wxProgressDialog& pd, int totaltime);
    wxImage CreateImageFromFrame(AVFrame* frame);
    void ShowImage(const wxImage& image);
    void SwapPage(int oldp, int newp);
    int GetEdge(int x, int y);
    void ResizeClip(int x, int y);

#pragma region Model Type Tab
    void MTTabEntry();
#pragma endregion Model Type Tab

#pragma region Choose Video Tab
    void CVTabEntry();
#pragma endregion Choose Video Tab

#pragma region Start Frame Tab
    void DoStartFrameIdentify();
    void SetStartFrame(int time);
    void SFTabEntry();
    float CalcFrameBrightness(const wxImage& image);
    int FindStartFrame(VideoReader* vr);
    void ValidateStartFrame();
    bool LooksLikeStartFrame(int candidateframe);
    void MoveStartFrame(int by);
#pragma endregion Start Frame Tab

#pragma region Identify Bulbs Tab
    void ApplyThreshold(wxImage& image, int threshold);
    void DoBulbIdentify();
    void BITabEntry(bool setdefault);
    void SetBIDefault();
    void FindLights(const wxImage& bwimage, int num, const wxImage& greyimage, const wxImage& frame);
    wxImage CreateDetectMask(wxImage ref, bool includeimage, wxRect rect);
    void WalkPixels(int x, int y, int w, int h, int w3, unsigned char *data, int& totalX, int& totalY, int& pixelCount);
    GCMBulb FindCenter(int x, int y, int w, int h, int w3, unsigned char *data, int num, const wxImage& grey);
    void SubtractImage(wxImage& from, wxImage& tosubtract);
    void ApplyContrast(wxImage& grey, int contrast);
    int CountWhite(wxImage& image);
    int ApplyMinimumSeparation(std::list<GCMBulb>& clipped, int minseparation);
    wxString GenerateStats(int minseparation);
    int GetMaxNum();
    int GetBulbCount();
    wxString GetMissingNodes();
    wxString GetMultiBulbNodes();
#pragma endregion Identify Bulbs Tab

    wxString CreateCustomModelData();
    wxPoint CalcTrim(std::list<GCMBulb>& lights);
    bool TestScale(std::list<GCMBulb>& lights, std::list<GCMBulb>::iterator it, float scale, wxPoint trim);
    void CMTabEntry();
    wxSize CalcSize(float min);
    void DoGenerateCustomModel();
    void RemoveClippedLights(std::list<GCMBulb>& lights, wxRect& clip);
    void MITabEntry(bool erase);
    wxImage CreateManualMask(wxImage ref);
    void AdvanceFrame();
    void ReverseFrame();
    void MIValidateWindow();
#pragma endregion Generate Tab

    void ValidateWindow();

    public:
		GenerateCustomModelDialog(wxWindow* parent, OutputManager* outputManager, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~GenerateCustomModelDialog();

		//(*Declarations(GenerateCustomModelDialog)
		wxAuiNotebook* AuiNotebook1;
		wxAuiNotebook* AuiNotebook_ProcessSettings;
		wxButton* ButtonBumpBack;
		wxButton* ButtonBumpFwd;
		wxButton* Button_BI_Back;
		wxButton* Button_BI_Next;
		wxButton* Button_BI_Update;
		wxButton* Button_Back10Frames;
		wxButton* Button_Back1Frame;
		wxButton* Button_CB_RestoreDefault;
		wxButton* Button_CM_Back;
		wxButton* Button_CM_Save;
		wxButton* Button_CV_Back;
		wxButton* Button_CV_Manual;
		wxButton* Button_CV_Next;
		wxButton* Button_Forward10Frames;
		wxButton* Button_Forward1Frame;
		wxButton* Button_GCM_SelectFile;
		wxButton* Button_Grow;
		wxButton* Button_MI_Back;
		wxButton* Button_MI_Next;
		wxButton* Button_MI_NextFrame;
		wxButton* Button_MI_PriorFrame;
		wxButton* Button_MI_UndoBulb;
		wxButton* Button_MT_Next;
		wxButton* Button_PCM_Run;
		wxButton* Button_SF_Back;
		wxButton* Button_SF_Manual;
		wxButton* Button_SF_Next;
		wxButton* Button_Shrink;
		wxCheckBox* CheckBox_BI_IsSteady;
		wxCheckBox* CheckBox_BI_ManualUpdate;
		wxFileDialog* FileDialog1;
		wxFlexGridSizer* FlexGridSizer14;
		wxFlexGridSizer* FlexGridSizer19;
		wxFlexGridSizer* FlexGridSizer5;
		wxGauge* Gauge_Progress;
		wxGrid* Grid_CM_Result;
		wxPanel* Panel1;
		wxPanel* Panel2;
		wxPanel* Panel_BulbIdentify;
		wxPanel* Panel_ChooseVideo;
		wxPanel* Panel_CustomModel;
		wxPanel* Panel_Generate;
		wxPanel* Panel_Prepare;
		wxPanel* Panel_StartFrame;
		wxRadioButton* NodesRadioButton;
		wxRadioButton* NodesRadioButtonPg2;
		wxRadioButton* SCRadioButton;
		wxRadioButton* SLRadioButton;
		wxRadioButton* SingleChannelRadioButton;
		wxSlider* Slider_AdjustBlur;
		wxSlider* Slider_BI_Contrast;
		wxSlider* Slider_BI_MinScale;
		wxSlider* Slider_BI_MinSeparation;
		wxSlider* Slider_BI_Sensitivity;
		wxSlider* Slider_Intensity;
		wxSpinCtrl* SpinCtrl_MissingBulbLimit;
		wxSpinCtrl* SpinCtrl_NC_Count;
		wxSpinCtrl* SpinCtrl_StartChannel;
		wxStaticText* StaticText10;
		wxStaticText* StaticText11;
		wxStaticText* StaticText12;
		wxStaticText* StaticText13;
		wxStaticText* StaticText15;
		wxStaticText* StaticText16;
		wxStaticText* StaticText17;
		wxStaticText* StaticText18;
		wxStaticText* StaticText9;
		wxStaticText* StaticText_BI;
		wxStaticText* StaticText_BI_Slider;
		wxStaticText* StaticText_CM_Request;
		wxStaticText* StaticText_StartFrameOk;
		wxStaticText* StaticText_StartTime;
		wxTextCtrl* TextCtrl_BC_Blur;
		wxTextCtrl* TextCtrl_BI_Contrast;
		wxTextCtrl* TextCtrl_BI_MinScale;
		wxTextCtrl* TextCtrl_BI_MinSeparation;
		wxTextCtrl* TextCtrl_BI_Sensitivity;
		wxTextCtrl* TextCtrl_BI_Status;
		wxTextCtrl* TextCtrl_GCM_Filename;
		//*)

	protected:

        static const long ID_STATICBITMAP_Preview;

		//(*Identifiers(GenerateCustomModelDialog)
		static const long ID_RADIOBUTTON1;
		static const long ID_RADIOBUTTON2;
		static const long ID_SPINCTRL_NC_Count;
		static const long ID_SPINCTRL_StartChannel;
		static const long ID_SLIDER_Intensity;
		static const long ID_BUTTON_PCM_Run;
		static const long ID_PANEL_Prepare;
		static const long ID_RADIOBUTTON3;
		static const long ID_RADIOBUTTON4;
		static const long ID_RADIOBUTTON5;
		static const long ID_BUTTON_MT_Next;
		static const long ID_PANEL1;
		static const long ID_STATICTEXT10;
		static const long ID_TEXTCTRL_GCM_Filename;
		static const long ID_BUTTON_GCM_SelectFile;
		static const long ID_BUTTON_CV_Back;
		static const long ID_BUTTON_CV_Manual;
		static const long ID_BUTTON_CV_Next;
		static const long ID_PANEL_ChooseVideo;
		static const long ID_STATICTEXT3;
		static const long ID_BUTTON_Back1Frame;
		static const long ID_BUTTON_Forward1Frame;
		static const long ID_BUTTON_Back10Frames;
		static const long ID_BUTTON_Forward10Frames;
		static const long ID_STATICTEXT_StartFrameOk;
		static const long ID_STATICTEXT_StartTime;
		static const long ID_BUTTON_SF_Back;
		static const long ID_BUTTON6;
		static const long ID_BUTTON_SF_Next;
		static const long ID_PANEL_StartFrame;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_AdjustBlur;
		static const long ID_TEXTCTRL_BC_Blur;
		static const long ID_STATICTEXT8;
		static const long ID_SLIDER_BI_Sensitivity;
		static const long ID_TEXTCTRL_BI_Sensitivity;
		static const long ID_STATICTEXT6;
		static const long ID_SLIDER_BI_MinSeparation;
		static const long ID_TEXTCTRL_BI_MinSeparation;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_BI_Contrast;
		static const long ID_TEXTCTRL_BI_Contrast;
		static const long ID_STATICTEXT7;
		static const long ID_SLIDER_BI_MinScale;
		static const long ID_TEXTCTRL_BI_MinScale;
		static const long ID_CHECKBOX_BI_IsSteady;
		static const long ID_CHECKBOX_BI_ManualUpdate;
		static const long ID_STATICTEXT12;
		static const long ID_SPINCTRL1;
		static const long ID_GAUGE1;
		static const long ID_BUTTON_BI_Update;
		static const long ID_BUTTON_CB_RestoreDefault;
		static const long ID_TEXTCTRL_BI_Status;
		static const long ID_BUTTON_BI_Back;
		static const long ID_BUTTON_BI_Next;
		static const long ID_PANEL_BulbIdentify;
		static const long ID_STATICTEXT11;
		static const long ID_BUTTON3;
		static const long ID_BUTTON1;
		static const long ID_BUTTON7;
		static const long ID_BUTTON8;
		static const long ID_STATICTEXT4;
		static const long ID_BUTTON2;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		static const long ID_PANEL2;
		static const long ID_STATICTEXT9;
		static const long ID_GRID_CM_Result;
		static const long ID_BUTTON_Shrink;
		static const long ID_BUTTON_Grow;
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
		void OnButton_SF_NextClick(wxCommandEvent& event);
		void OnButton_Back1FrameClick(wxCommandEvent& event);
		void OnButton_Forward1FrameClick(wxCommandEvent& event);
		void OnButton_Back10FramesClick(wxCommandEvent& event);
		void OnButton_Forward10FramesClick(wxCommandEvent& event);
		void OnSlider_AdjustBlurCmdScroll(wxScrollEvent& event);
		void OnButton_CM_BackClick(wxCommandEvent& event);
		void OnButton_CM_SaveClick(wxCommandEvent& event);
		void OnSlider_BI_SensitivityCmdSliderUpdated(wxScrollEvent& event);
		void OnButton_BI_NextClick(wxCommandEvent& event);
		void OnButton_BI_BackClick(wxCommandEvent& event);
		void OnButton_CV_NextClick(wxCommandEvent& event);
		void OnButton_SF_BackClick(wxCommandEvent& event);
		void OnAuiNotebook_ProcessSettingsPageChanging(wxAuiNotebookEvent& event);
		void OnButton_MT_NextClick(wxCommandEvent& event);
		void OnButton_CV_BackClick(wxCommandEvent& event);
		void OnButton_ShrinkClick(wxCommandEvent& event);
		void OnButton_GrowClick(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnSlider_BI_MinSeparationCmdSliderUpdated(wxScrollEvent& event);
		void OnButton_BI_RestoreDefaultClick(wxCommandEvent& event);
		void OnSlider_AdjustBlurCmdScrollChanged(wxScrollEvent& event);
		void OnSlider_BI_SensitivityCmdScrollChanged(wxScrollEvent& event);
		void OnSlider_BI_MinSeparationCmdScrollChanged(wxScrollEvent& event);
		void OnSlider_BI_ContrastCmdScrollChanged(wxScrollEvent& event);
		void OnSlider_BI_ContrastCmdSliderUpdated(wxScrollEvent& event);
		void OnCheckBox_BI_IsSteadyClick(wxCommandEvent& event);
		void OnCheckBox_BI_ManualUpdateClick(wxCommandEvent& event);
		void OnButton_BI_UpdateClick(wxCommandEvent& event);
		void OnButton_MI_PriorFrameClick(wxCommandEvent& event);
		void OnButton_MI_NextFrameClick(wxCommandEvent& event);
		void OnButton_MI_UndoBulbClick(wxCommandEvent& event);
		void OnButton_MI_BackClick(wxCommandEvent& event);
		void OnButton_MI_NextClick(wxCommandEvent& event);
		void OnButton_CV_ManualClick(wxCommandEvent& event);
		void OnButton_SF_ManualClick(wxCommandEvent& event);
		void OnButtonBumpBackClick(wxCommandEvent& event);
		void OnButtonBumpFwdClick(wxCommandEvent& event);
		void OnSlider_BI_MinScaleCmdScrollChanged(wxScrollEvent& event);
		void OnSlider_BI_MinScaleCmdSliderUpdated(wxScrollEvent& event);
		//*)

        void OnStaticBitmapLeftUp(wxMouseEvent& event);
        void OnStaticBitmapLeftDown(wxMouseEvent& event);
        void OnStaticBitmapMouseMove(wxMouseEvent& event);
        void OnStaticBitmapMouseLeave(wxMouseEvent& event);
        void OnStaticBitmapMouseEnter(wxMouseEvent& event);

        DECLARE_EVENT_TABLE()
};

