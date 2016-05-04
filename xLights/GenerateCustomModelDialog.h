#ifndef GENERATECUSTOMMODELDIALOG_H
#define GENERATECUSTOMMODELDIALOG_H

#include "VideoReader.h"
#include "xlights_out.h"

//(*Headers(GenerateCustomModelDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobox.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/grid.h>
#include <wx/filedlg.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#ifdef __WXOSX__
#include "osxMacUtils.h"
#endif

#include <wx/xml/xml.h>
#include <wx/progdlg.h>
#include <wx/generic/statbmpg.h>
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
    wxXmlDocument* _network;

#pragma region Prepare Tab
    wxDateTime _starttime;

    void SetBulbs(bool nodes, int count, int startch, int node, int ms, int intensity, xOutput* xout);
    bool InitialiseOutputs(xOutput* xOut);
#pragma endregion Prepare Tab

#pragma region Generate Tab
    enum VideoProcessingStates
    {
        CHOOSE_MODELTYPE,
        CHOOSE_VIDEO,
        FINDING_START_FRAME,
        CIRCLING_BULBS,
        IDENTIFYING_BULBS,
        REVIEW_CUSTOM_MODEL
    };

    wxImage _displaybmp; // the image we are displaying on the screen
    int _startframetime; // time in MS in the video the start frame occurs at
    float _startframebrightness;
    wxImage _startFrame; // the image of the start frame
    wxImage _darkFrame; // an image with no lights on
    //wxImage _greyFrame;  // the greyscale version of the start frame
    //wxImage _bwFrame;    // the black and white version of the image after blur & level
    //wxImage _cbFrame;    // the Circle bulbs image - edges
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

    void UpdateProgress(wxProgressDialog& pd, int totaltime);
    void ShowFrame(int time);
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
    void BITabEntry();
    //float CalcPoint(wxImage& edge, int x0, int y0, int radius);
    //std::map<xlPoint, int> CircleDetect(wxImage& mask, wxImage& edge, int radius);
    //std::list<wxPoint> CircleDetect(wxImage& mask, wxImage& edge, int minr, int maxr);
    void FindLights(wxImage& bwimage, int num, wxImage& greyimage);
    wxImage CreateDetectMask(wxImage ref, bool includeimage, wxRect rect);
    void WalkPixels(int x, int y, int w, int h, int w3, unsigned char *data, int& totalX, int& totalY, int& pixelCount);
    GCMBulb FindCenter(int x, int y, int w, int h, int w3, unsigned char *data, int num, wxImage& grey);
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
    wxSize CalcSize();
    void DoGenerateCustomModel();
    void RemoveClippedLights(std::list<GCMBulb>& lights, wxRect& clip);

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
		wxButton* Button_Shrink;
		wxButton* Button_Forward10Frames;
		wxTextCtrl* TextCtrl_BI_Contrast;
		wxStaticText* StaticText_StartTime;
		wxFlexGridSizer* FlexGridSizer19;
		wxStaticText* StaticText_CM_Request;
		wxTextCtrl* TextCtrl_BI_Sensitivity;
		wxSlider* Slider_BI_MinSeparation;
		wxButton* Button_BI_Back;
		wxButton* Button_SF_Back;
		wxButton* Button_Back1Frame;
		wxStaticText* StaticText13;
		wxButton* Button_CB_RestoreDefault;
		wxButton* Button_CM_Back;
		wxRadioBox* RadioBox2;
		wxAuiNotebook* AuiNotebook_ProcessSettings;
		wxButton* Button_BI_Update;
		wxSlider* Slider_Intensity;
		wxStaticText* StaticText_BI;
		wxStaticText* StaticText11;
		wxGrid* Grid_CM_Result;
		wxStaticText* StaticText_BI_Slider;
		wxButton* Button_PCM_Run;
		wxButton* Button_CV_Next;
		wxPanel* Panel1;
		wxSpinCtrl* SpinCtrl_NC_Count;
		wxFileDialog* FileDialog1;
		wxTextCtrl* TextCtrl_BI_Status;
		wxButton* Button_Back10Frames;
		wxButton* Button_CV_Back;
		wxButton* Button_MT_Next;
		wxTextCtrl* TextCtrl_BC_Blur;
		wxSlider* Slider_BI_Contrast;
		wxTextCtrl* TextCtrl_BI_MinSeparation;
		wxTextCtrl* TextCtrl_GCM_Filename;
		wxPanel* Panel_ChooseVideo;
		wxCheckBox* CheckBox_BI_ManualUpdate;
		wxSlider* Slider_AdjustBlur;
		wxButton* Button_GCM_SelectFile;
		wxPanel* Panel_Generate;
		wxAuiNotebook* AuiNotebook1;
		wxButton* Button_SF_Next;
		wxStaticText* StaticText_StartFrameOk;
		wxPanel* Panel_CustomModel;
		wxButton* Button_BI_Next;
		wxCheckBox* CheckBox_BI_IsSteady;
		wxRadioBox* RadioBox1;
		wxPanel* Panel_Prepare;
		wxSlider* Slider_BI_Sensitivity;
		wxPanel* Panel_StartFrame;
		wxButton* Button_Grow;
		wxStaticText* StaticText17;
		wxButton* Button_CM_Save;
		wxSpinCtrl* SpinCtrl_StartChannel;
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
		static const long ID_RADIOBOX2;
		static const long ID_BUTTON_MT_Next;
		static const long ID_PANEL1;
		static const long ID_STATICTEXT10;
		static const long ID_TEXTCTRL_GCM_Filename;
		static const long ID_BUTTON_GCM_SelectFile;
		static const long ID_BUTTON_CV_Back;
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
		static const long ID_CHECKBOX_BI_IsSteady;
		static const long ID_CHECKBOX_BI_ManualUpdate;
		static const long ID_BUTTON_BI_Update;
		static const long ID_BUTTON_CB_RestoreDefault;
		static const long ID_TEXTCTRL_BI_Status;
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
		void OnButton1Click(wxCommandEvent& event);
		void OnCheckBox_BI_ManualUpdateClick(wxCommandEvent& event);
		void OnButton_BI_UpdateClick(wxCommandEvent& event);
		//*)

        void OnStaticBitmapLeftUp(wxMouseEvent& event);
        void OnStaticBitmapLeftDown(wxMouseEvent& event);
        void OnStaticBitmapMouseMove(wxMouseEvent& event);
        void OnStaticBitmapMouseLeave(wxMouseEvent& event);

        DECLARE_EVENT_TABLE()
};

#endif
