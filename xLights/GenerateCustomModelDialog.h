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


#include <wx/xml/xml.h>
#include <wx/progdlg.h>
#include <wx/dcmemory.h>

#include <list>
#include <map>

#pragma region xlPoint

class xLightsFrame;

// This is required so I can use points in std::maps
class xlPoint : public wxPoint
{
public:
    xlPoint(int x, int y) : wxPoint(x, y) {}
    bool operator<(const xlPoint& r) const
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

class CustomModelGenerator;
class ProcessedImage;

class GenerateCustomModelDialog: public wxDialog
{
    // variables passed into us
    OutputManager* _outputManager = nullptr;
    CustomModelGenerator* _generator = nullptr;

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
    std::list<std::pair<wxPoint, uint32_t>> _lights; // our lights
    uint32_t _minimumSeparation = 0.0001;
    VideoProcessingStates _state;
    MyGenericStaticBitmap* StaticBitmap_Preview = nullptr;
    bool _busy = false;          // true while busy and re-entrancy possible
    int _draggingedge = 0;
    wxRect _clip;
    xLightsFrame *_parent = nullptr;
    ProcessedImage* _detectedImage = nullptr;
    wxProgressDialog* _pd = nullptr;

	void CreateDetectedImage(ProcessedImage* pi = nullptr, bool drawLines = false);
	void UpdateProgressCallback(float progress);
	void DisplayImageCallbackCMG(ProcessedImage* image);
    void ShowImage(const ProcessedImage* image);
    void SwapPage(int oldp, int newp);
    int GetEdge(int x, int y);
    void ResizeClip(int x, int y);
	void SetGridSizeForFont(const wxFont& font);
    bool ShowPixelLines();

#pragma region Model Type Tab
    void MTTabEntry();
#pragma endregion Model Type Tab

#pragma region Choose Video Tab
    void CVTabEntry();
#pragma endregion Choose Video Tab

#pragma region Start Frame Tab
    void SFTabEntry();
#pragma endregion Start Frame Tab

#pragma region Identify Bulbs Tab
    void DoBulbIdentify();
    void BITabEntry(bool setdefault);
    void SetBIDefault();
    wxString GenerateStats();
    int GetMaxNum();
    int GetBulbCount();
    wxString GetMissingNodes();
	void GuessMissingBulbs();
#pragma endregion Identify Bulbs Tab

    wxString CreateCustomModelData();
    void CMTabEntry();
    wxSize CalcSize(wxPoint* offset = nullptr, float* multiplier = nullptr);
    void DoGenerateCustomModel();
    std::function<void(ProcessedImage*)> DisplayImage(bool show);
    void ShowProgress(bool show);
    std::function<void(float)> Progress();
    void UpdateProgress(float progress);
#pragma endregion Generate Tab

    void ValidateWindow();

    public:
		GenerateCustomModelDialog(xLightsFrame* parent, OutputManager* outputManager, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~GenerateCustomModelDialog();

		//(*Declarations(GenerateCustomModelDialog)
		wxAuiNotebook* AuiNotebook1;
		wxAuiNotebook* AuiNotebook_ProcessSettings;
		wxButton* Button_BI_Back;
		wxButton* Button_BI_Next;
		wxButton* Button_CB_RestoreDefault;
		wxButton* Button_CM_Back;
		wxButton* Button_CM_Save;
		wxButton* Button_CV_Back;
		wxButton* Button_CV_Next;
		wxButton* Button_GCM_SelectFile;
		wxButton* Button_Grow;
		wxButton* Button_MT_Next;
		wxButton* Button_PCM_Run;
		wxButton* Button_SF_Back;
		wxButton* Button_SF_Next;
		wxButton* Button_Shrink;
		wxCheckBox* CheckBox_Advanced;
		wxCheckBox* CheckBox_AdvancedStartScan;
		wxCheckBox* CheckBox_BI_IsSteady;
		wxCheckBox* CheckBox_GuessSingle;
		wxFileDialog* FileDialog1;
		wxFlexGridSizer* FlexGridSizer14;
		wxFlexGridSizer* FlexGridSizer19;
		wxFlexGridSizer* FlexGridSizer5;
		wxGauge* Gauge_Progress1;
		wxGauge* Gauge_Progress2;
		wxGrid* Grid_CM_Result;
		wxPanel* Panel1;
		wxPanel* Panel_BulbIdentify;
		wxPanel* Panel_ChooseVideo;
		wxPanel* Panel_CustomModel;
		wxPanel* Panel_Generate;
		wxPanel* Panel_Prepare;
		wxPanel* Panel_StartFrame;
		wxRadioButton* NodesRadioButton;
		wxRadioButton* NodesRadioButtonPg2;
		wxRadioButton* SLRadioButton;
		wxRadioButton* SingleChannelRadioButton;
		wxSlider* Slider_AdjustBlur;
		wxSlider* Slider_BI_Contrast;
		wxSlider* Slider_BI_MinScale;
		wxSlider* Slider_BI_MinSeparation;
		wxSlider* Slider_BI_Sensitivity;
		wxSlider* Slider_Despeckle;
		wxSlider* Slider_Gamma;
		wxSlider* Slider_Intensity;
		wxSlider* Slider_Saturation;
		wxSpinCtrl* SpinCtrl_NC_Count;
		wxSpinCtrl* SpinCtrl_ProcessNodeCount;
		wxSpinCtrl* SpinCtrl_StartChannel;
		wxStaticText* StaticText11;
		wxStaticText* StaticText17;
		wxStaticText* StaticText19;
		wxStaticText* StaticTextDespeckle;
		wxStaticText* StaticText_BI;
		wxStaticText* StaticText_Blur;
		wxStaticText* StaticText_CM_Request;
		wxStaticText* StaticText_Contrast;
		wxStaticText* StaticText_Gamma;
		wxStaticText* StaticText_MinSeparation;
		wxStaticText* StaticText_ModelScale;
		wxStaticText* StaticText_Saturation;
		wxStaticText* StaticText_Sensitivity;
		wxStaticText* StaticText_StartFrameOk;
		wxStaticText* StaticText_StartTime;
		wxTextCtrl* TextCtrl_BC_Blur;
		wxTextCtrl* TextCtrl_BI_Contrast;
		wxTextCtrl* TextCtrl_BI_MinScale;
		wxTextCtrl* TextCtrl_BI_MinSeparation;
		wxTextCtrl* TextCtrl_BI_Sensitivity;
		wxTextCtrl* TextCtrl_BI_Status;
		wxTextCtrl* TextCtrl_Despeckle;
		wxTextCtrl* TextCtrl_GCM_Filename;
		wxTextCtrl* TextCtrl_Gamma;
		wxTextCtrl* TextCtrl_Saturation;
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
		static const long ID_RADIOBUTTON5;
		static const long ID_BUTTON_MT_Next;
		static const long ID_PANEL1;
		static const long ID_STATICTEXT10;
		static const long ID_TEXTCTRL_GCM_Filename;
		static const long ID_BUTTON_GCM_SelectFile;
		static const long ID_STATICTEXT13;
		static const long ID_SPINCTRL_PROCESSNODECOUNT;
		static const long ID_CHECKBOX_BI_IsSteady;
		static const long ID_CHECKBOX3;
		static const long ID_GAUGE2;
		static const long ID_BUTTON_CV_Back;
		static const long ID_BUTTON_CV_Next;
		static const long ID_PANEL_ChooseVideo;
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT_StartFrameOk;
		static const long ID_STATICTEXT_StartTime;
		static const long ID_BUTTON_SF_Back;
		static const long ID_BUTTON_SF_Next;
		static const long ID_PANEL_StartFrame;
		static const long ID_STATICTEXT5;
		static const long ID_CHECKBOX2;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_AdjustBlur;
		static const long ID_TEXTCTRL_BC_Blur;
		static const long ID_STATICTEXT8;
		static const long ID_SLIDER_BI_Sensitivity;
		static const long ID_TEXTCTRL_BI_Sensitivity;
		static const long ID_STATICTEXT6;
		static const long ID_SLIDER_BI_MinSeparation;
		static const long ID_TEXTCTRL_BI_MinSeparation;
		static const long ID_STATICTEXT4;
		static const long ID_SLIDER1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_BI_Contrast;
		static const long ID_TEXTCTRL_BI_Contrast;
		static const long ID_STATICTEXT11;
		static const long ID_SLIDER2;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT12;
		static const long ID_SLIDER3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT7;
		static const long ID_SLIDER_BI_MinScale;
		static const long ID_TEXTCTRL_BI_MinScale;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON_CB_RestoreDefault;
		static const long ID_TEXTCTRL_BI_Status;
		static const long ID_GAUGE1;
		static const long ID_BUTTON_BI_Back;
		static const long ID_BUTTON_BI_Next;
		static const long ID_PANEL_BulbIdentify;
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
		void OnSlider_BI_MinScaleCmdScrollChanged(wxScrollEvent& event);
		void OnSlider_BI_MinScaleCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_DespeckleCmdScrollChanged(wxScrollEvent& event);
		void OnSlider_DespeckleCmdSliderUpdated(wxScrollEvent& event);
		void OnCheckBox_GuessSingleClick(wxCommandEvent& event);
		void OnSlider_GammaCmdScrollChanged(wxScrollEvent& event);
		void OnSlider_GammaCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_SaturationCmdScrollChanged(wxScrollEvent& event);
		void OnSlider_SaturationCmdSliderUpdated(wxScrollEvent& event);
		void OnCheckBox_AdvancedClick(wxCommandEvent& event);
		//*)

        void OnStaticBitmapLeftUp(wxMouseEvent& event);
        void OnStaticBitmapLeftDClick(wxMouseEvent& event);
        void OnStaticBitmapLeftDown(wxMouseEvent& event);
        void OnStaticBitmapMouseMove(wxMouseEvent& event);
        void OnStaticBitmapMouseLeave(wxMouseEvent& event);
        void OnStaticBitmapMouseEnter(wxMouseEvent& event);

        DECLARE_EVENT_TABLE()
};

