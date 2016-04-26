#include "GenerateCustomModelDialog.h"
#include "effects/VideoPanel.h"

#include <map>
#include <list>

//(*InternalHeaders(GenerateCustomModelDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/dcmemory.h>

#define GCM_DISPLAYIMAGEWIDTH 400
#define GCM_DISPLAYIMAGEHEIGHT 300

#define PAGE_PREPARE 0
#define PAGE_GENERATE 1

#define PAGE_CHOOSEVIDEO 0
#define PAGE_STARTFRAME 1
#define PAGE_CIRCLEBULB 2
#define PAGE_BULBIDENTIFY 3
#define PAGE_REVIEWMODEL 4

#define STARTSCANSECS 30
#define FRAMEMS 50
#define LEADOFF 3000
#define LEADON 500
#define FLAGON 500
#define FLAGOFF 500
#define NODEON 500
#define NODEOFF 200
#define DELAYMSUNTILSAMPLE 100

#pragma region Flicker Free Static Bitmap

class MyGenericStaticBitmap : public wxGenericStaticBitmap {
    bool _doerasebackground;
public:
    MyGenericStaticBitmap(wxWindow *parent,
        wxWindowID id,
        const wxBitmap& bitmap,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name = wxStaticBitmapNameStr)
        : wxGenericStaticBitmap(parent, id, bitmap, pos, size, style, name)
    {
        _doerasebackground = true;
    };
    void OnEraseBackGround(wxEraseEvent& event)
    {
        if (_doerasebackground)
        {
            event.Skip();
        }
    };
    void SetEraseBackground(bool f)
    {
        _doerasebackground = f;
    }
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MyGenericStaticBitmap, wxGenericStaticBitmap)
EVT_ERASE_BACKGROUND(MyGenericStaticBitmap::OnEraseBackGround)
END_EVENT_TABLE()

#pragma endregion Flicker Free Static Bitmap

#pragma region Constructor

const long GenerateCustomModelDialog::ID_STATICBITMAP_Preview = wxNewId();

//(*IdInit(GenerateCustomModelDialog)
const long GenerateCustomModelDialog::ID_RADIOBOX1 = wxNewId();
const long GenerateCustomModelDialog::ID_SPINCTRL_NC_Count = wxNewId();
const long GenerateCustomModelDialog::ID_SPINCTRL_StartChannel = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_Intensity = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_PCM_Run = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_Prepare = wxNewId();
const long GenerateCustomModelDialog::ID_GAUGE_Progress = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT10 = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_GCM_Filename = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_GCM_SelectFile = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_CV_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_ChooseVideo = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT3 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Back1Frame = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Forward1Frame = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Back10Frames = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Forward10Frames = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT_StartFrameOk = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_SF_Back = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_SF_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_StartFrame = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT4 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT1 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_AdjustBlur = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT2 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_LevelFilterAdjust = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_CB_RestoreDefault = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_BD_Back = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_BD_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_BulbCircle = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT5 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT6 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_BI_MinRadius = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT7 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_BI_MaxRadius = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT8 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_BI_Sensitivity = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_BI_Back = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_BI_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_BulbIdentify = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT9 = wxNewId();
const long GenerateCustomModelDialog::ID_GRID_CM_Result = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_CM_Back = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_CM_Save = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_CustomModel = wxNewId();
const long GenerateCustomModelDialog::ID_AUINOTEBOOK_ProcessSettings = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_Generate = wxNewId();
const long GenerateCustomModelDialog::ID_AUINOTEBOOK1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(GenerateCustomModelDialog,wxDialog)
	//(*EventTable(GenerateCustomModelDialog)
	//*)
END_EVENT_TABLE()

GenerateCustomModelDialog::GenerateCustomModelDialog(wxWindow* parent, wxXmlDocument* network, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _network = network;
    _busy = false;

	//(*Initialize(GenerateCustomModelDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer16;
	wxFlexGridSizer* FlexGridSizer19;
	wxFlexGridSizer* FlexGridSizer23;
	wxStaticText* StaticText2;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer3;
	wxStaticText* StaticText6;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer22;
	wxStaticText* StaticText8;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText1;
	wxStaticText* StaticText3;
	wxFlexGridSizer* FlexGridSizer7;
	wxStaticText* StaticText5;
	wxFlexGridSizer* FlexGridSizer15;
	wxStaticText* StaticText7;
	wxFlexGridSizer* FlexGridSizer18;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer21;
	wxFlexGridSizer* FlexGridSizer14;
	wxFlexGridSizer* FlexGridSizer20;
	wxFlexGridSizer* FlexGridSizer13;
	wxFlexGridSizer* FlexGridSizer12;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer11;
	wxFlexGridSizer* FlexGridSizer17;
	wxStaticText* StaticText4;

	Create(parent, id, _("Generate Custom Models"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	AuiNotebook1 = new wxAuiNotebook(this, ID_AUINOTEBOOK1, wxDefaultPosition, wxSize(800,500), wxTAB_TRAVERSAL);
	Panel_Prepare = new wxPanel(AuiNotebook1, ID_PANEL_Prepare, wxPoint(63,54), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Prepare"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	StaticText1 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Use this page to generate the lights that you need to video before you can proceed to the processing tab to create your custom model."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL, 2);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Using a steady video camera to record a clear video showing all the lights running this test pattern."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(Panel_Prepare, wxID_ANY, _("The test pattern must start within the first 30 seconds of the video."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	StaticText4 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Model details:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	wxFont StaticText4Font(10,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText4->SetFont(StaticText4Font);
	FlexGridSizer6->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	wxString __wxRadioBoxChoices_1[2] =
	{
		_("Nodes"),
		_("Single Channels")
	};
	RadioBox1 = new wxRadioBox(Panel_Prepare, ID_RADIOBOX1, _("Type"), wxDefaultPosition, wxDefaultSize, 2, __wxRadioBoxChoices_1, 1, 0, wxDefaultValidator, _T("ID_RADIOBOX1"));
	FlexGridSizer6->Add(RadioBox1, 1, wxALL|wxEXPAND, 2);
	StaticText5 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Node/Channel Count"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_NC_Count = new wxSpinCtrl(Panel_Prepare, ID_SPINCTRL_NC_Count, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 99999, 1, _T("ID_SPINCTRL_NC_Count"));
	SpinCtrl_NC_Count->SetValue(_T("1"));
	FlexGridSizer6->Add(SpinCtrl_NC_Count, 1, wxALL|wxEXPAND, 2);
	StaticText6 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Start Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_StartChannel = new wxSpinCtrl(Panel_Prepare, ID_SPINCTRL_StartChannel, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 99999, 1, _T("ID_SPINCTRL_StartChannel"));
	SpinCtrl_StartChannel->SetValue(_T("1"));
	FlexGridSizer6->Add(SpinCtrl_StartChannel, 1, wxALL|wxEXPAND, 2);
	StaticText8 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Intensity = new wxSlider(Panel_Prepare, ID_SLIDER_Intensity, 255, 30, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Intensity"));
	FlexGridSizer6->Add(Slider_Intensity, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer2->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_PCM_Run = new wxButton(Panel_Prepare, ID_BUTTON_PCM_Run, _("Run Capture Pattern"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PCM_Run"));
	FlexGridSizer2->Add(Button_PCM_Run, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_Prepare->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel_Prepare);
	FlexGridSizer2->SetSizeHints(Panel_Prepare);
	Panel_Generate = new wxPanel(AuiNotebook1, ID_PANEL_Generate, wxPoint(59,17), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Generate"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(1);
	FlexGridSizer5->AddGrowableRow(0);
	FlexGridSizer14 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer14->AddGrowableCol(0);
	FlexGridSizer14->AddGrowableRow(0);
	Gauge_Progress = new wxGauge(Panel_Generate, ID_GAUGE_Progress, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_GAUGE_Progress"));
	FlexGridSizer14->Add(Gauge_Progress, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer5->Add(FlexGridSizer14, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer7 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	FlexGridSizer7->AddGrowableRow(0);
	AuiNotebook_ProcessSettings = new wxAuiNotebook(Panel_Generate, ID_AUINOTEBOOK_ProcessSettings, wxDefaultPosition, wxDefaultSize, 0);
	Panel_ChooseVideo = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_ChooseVideo, wxPoint(18,15), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_ChooseVideo"));
	FlexGridSizer21 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer21->AddGrowableCol(0);
	FlexGridSizer21->AddGrowableRow(2);
	StaticText18 = new wxStaticText(Panel_ChooseVideo, ID_STATICTEXT10, _("Select you video file you recorded in the prepare step."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer21->Add(StaticText18, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer22 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer22->AddGrowableCol(1);
	StaticText3 = new wxStaticText(Panel_ChooseVideo, wxID_ANY, _("Video File"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer22->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_GCM_Filename = new wxTextCtrl(Panel_ChooseVideo, ID_TEXTCTRL_GCM_Filename, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_GCM_Filename"));
	FlexGridSizer22->Add(TextCtrl_GCM_Filename, 1, wxALL|wxEXPAND, 2);
	Button_GCM_SelectFile = new wxButton(Panel_ChooseVideo, ID_BUTTON_GCM_SelectFile, _("..."), wxDefaultPosition, wxSize(29,28), 0, wxDefaultValidator, _T("ID_BUTTON_GCM_SelectFile"));
	FlexGridSizer22->Add(Button_GCM_SelectFile, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer21->Add(FlexGridSizer22, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer21->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer23 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_CV_Next = new wxButton(Panel_ChooseVideo, ID_BUTTON_CV_Next, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CV_Next"));
	FlexGridSizer23->Add(Button_CV_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer21->Add(FlexGridSizer23, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_ChooseVideo->SetSizer(FlexGridSizer21);
	FlexGridSizer21->Fit(Panel_ChooseVideo);
	FlexGridSizer21->SetSizeHints(Panel_ChooseVideo);
	Panel_StartFrame = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_StartFrame, wxPoint(43,126), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_StartFrame"));
	FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	FlexGridSizer10->AddGrowableRow(3);
	StaticText11 = new wxStaticText(Panel_StartFrame, ID_STATICTEXT3, _("This is the frame the scan has identified as being the most likely to show all your bulbs.\nYou can move it forward or backwards with the buttons below.\n\nClick next when you are happy with it."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer10->Add(StaticText11, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer11 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Back1Frame = new wxButton(Panel_StartFrame, ID_BUTTON_Back1Frame, _("Back 1 Frame"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Back1Frame"));
	FlexGridSizer11->Add(Button_Back1Frame, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Forward1Frame = new wxButton(Panel_StartFrame, ID_BUTTON_Forward1Frame, _("Forward 1 Frame"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Forward1Frame"));
	FlexGridSizer11->Add(Button_Forward1Frame, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Back10Frames = new wxButton(Panel_StartFrame, ID_BUTTON_Back10Frames, _("Back 10 Frames"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Back10Frames"));
	FlexGridSizer11->Add(Button_Back10Frames, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Forward10Frames = new wxButton(Panel_StartFrame, ID_BUTTON_Forward10Frames, _("Forward 10 Frames"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Forward10Frames"));
	FlexGridSizer11->Add(Button_Forward10Frames, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer10->Add(FlexGridSizer11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
	StaticText_StartFrameOk = new wxStaticText(Panel_StartFrame, ID_STATICTEXT_StartFrameOk, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_STATICTEXT_StartFrameOk"));
	wxFont StaticText_StartFrameOkFont(12,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText_StartFrameOk->SetFont(StaticText_StartFrameOkFont);
	FlexGridSizer10->Add(StaticText_StartFrameOk, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_SF_Back = new wxButton(Panel_StartFrame, ID_BUTTON_SF_Back, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SF_Back"));
	FlexGridSizer12->Add(Button_SF_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_SF_Next = new wxButton(Panel_StartFrame, ID_BUTTON_SF_Next, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SF_Next"));
	FlexGridSizer12->Add(Button_SF_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10->Add(FlexGridSizer12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_StartFrame->SetSizer(FlexGridSizer10);
	FlexGridSizer10->Fit(Panel_StartFrame);
	FlexGridSizer10->SetSizeHints(Panel_StartFrame);
	Panel_BulbCircle = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_BulbCircle, wxPoint(71,14), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_BulbCircle"));
	Panel_BulbCircle->Hide();
	FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	FlexGridSizer9->AddGrowableRow(4);
	StaticText12 = new wxStaticText(Panel_BulbCircle, ID_STATICTEXT4, _("We now need to process this image. Use the sliders below to adjust settings to get clear white\nrough circles around all of your bulbs.\n\nClick next when you are happy with it."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer9->Add(StaticText12, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(1);
	StaticText9 = new wxStaticText(Panel_BulbCircle, ID_STATICTEXT1, _("Blur"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer8->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_AdjustBlur = new wxSlider(Panel_BulbCircle, ID_SLIDER_AdjustBlur, 5, 1, 30, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_AdjustBlur"));
	FlexGridSizer8->Add(Slider_AdjustBlur, 1, wxALL|wxEXPAND, 2);
	StaticText10 = new wxStaticText(Panel_BulbCircle, ID_STATICTEXT2, _("Level"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer8->Add(StaticText10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_LevelFilterAdjust = new wxSlider(Panel_BulbCircle, ID_SLIDER_LevelFilterAdjust, 200, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_LevelFilterAdjust"));
	FlexGridSizer8->Add(Slider_LevelFilterAdjust, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer9->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer9->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_CB_RestoreDefault = new wxButton(Panel_BulbCircle, ID_BUTTON_CB_RestoreDefault, _("Restore Default"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CB_RestoreDefault"));
	FlexGridSizer4->Add(Button_CB_RestoreDefault, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer13 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_BD_Back = new wxButton(Panel_BulbCircle, ID_BUTTON_BD_Back, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_BD_Back"));
	FlexGridSizer13->Add(Button_BD_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_BD_Next = new wxButton(Panel_BulbCircle, ID_BUTTON_BD_Next, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_BD_Next"));
	FlexGridSizer13->Add(Button_BD_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9->Add(FlexGridSizer13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_BulbCircle->SetSizer(FlexGridSizer9);
	FlexGridSizer9->Fit(Panel_BulbCircle);
	FlexGridSizer9->SetSizeHints(Panel_BulbCircle);
	Panel_BulbIdentify = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_BulbIdentify, wxPoint(176,18), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_BulbIdentify"));
	FlexGridSizer15 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer15->AddGrowableCol(0);
	FlexGridSizer15->AddGrowableRow(2);
	StaticText13 = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT5, _("We now need to pinpoint exactly where the bulbs are. The magenta circles are what is currently identified.\nIf it has missed some small bulbs or identified multiple where one should be adjust the settings below.\nHaving some false bulbs identified is less important than missing bulbs.\n\nClick next when you are happy that all bulbs have been detected."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer15->Add(StaticText13, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer16 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer16->AddGrowableCol(1);
	StaticText14 = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT6, _("Minimum Radius"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer16->Add(StaticText14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_BI_MinRadius = new wxSlider(Panel_BulbIdentify, ID_SLIDER_BI_MinRadius, 5, 2, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BI_MinRadius"));
	FlexGridSizer16->Add(Slider_BI_MinRadius, 1, wxALL|wxEXPAND, 2);
	StaticText15 = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT7, _("Maximum Radius"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer16->Add(StaticText15, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_BI_MaxRadius = new wxSlider(Panel_BulbIdentify, ID_SLIDER_BI_MaxRadius, 20, 2, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BI_MaxRadius"));
	FlexGridSizer16->Add(Slider_BI_MaxRadius, 1, wxALL|wxEXPAND, 2);
	StaticText16 = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT8, _("Sensitivity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer16->Add(StaticText16, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_BI_Sensitivity = new wxSlider(Panel_BulbIdentify, ID_SLIDER_BI_Sensitivity, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BI_Sensitivity"));
	FlexGridSizer16->Add(Slider_BI_Sensitivity, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer15->Add(FlexGridSizer16, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer15->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer17 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_BI_Back = new wxButton(Panel_BulbIdentify, ID_BUTTON_BI_Back, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_BI_Back"));
	FlexGridSizer17->Add(Button_BI_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_BI_Next = new wxButton(Panel_BulbIdentify, ID_BUTTON_BI_Next, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_BI_Next"));
	FlexGridSizer17->Add(Button_BI_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer15->Add(FlexGridSizer17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_BulbIdentify->SetSizer(FlexGridSizer15);
	FlexGridSizer15->Fit(Panel_BulbIdentify);
	FlexGridSizer15->SetSizeHints(Panel_BulbIdentify);
	Panel_CustomModel = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_CustomModel, wxPoint(259,19), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_CustomModel"));
	FlexGridSizer18 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer18->AddGrowableCol(0);
	FlexGridSizer18->AddGrowableRow(2);
	StaticText17 = new wxStaticText(Panel_CustomModel, ID_STATICTEXT9, _("This is the new custom model. Click save to create a model file that you can then import into your layout."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer18->Add(StaticText17, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer19 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer19->AddGrowableCol(0);
	FlexGridSizer19->AddGrowableRow(0);
	Grid_CM_Result = new wxGrid(Panel_CustomModel, ID_GRID_CM_Result, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID_CM_Result"));
	FlexGridSizer19->Add(Grid_CM_Result, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer18->Add(FlexGridSizer19, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer18->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer20 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_CM_Back = new wxButton(Panel_CustomModel, ID_BUTTON_CM_Back, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CM_Back"));
	FlexGridSizer20->Add(Button_CM_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_CM_Save = new wxButton(Panel_CustomModel, ID_BUTTON_CM_Save, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CM_Save"));
	FlexGridSizer20->Add(Button_CM_Save, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer18->Add(FlexGridSizer20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_CustomModel->SetSizer(FlexGridSizer18);
	FlexGridSizer18->Fit(Panel_CustomModel);
	FlexGridSizer18->SetSizeHints(Panel_CustomModel);
	AuiNotebook_ProcessSettings->AddPage(Panel_ChooseVideo, _("Choose Video"));
	AuiNotebook_ProcessSettings->AddPage(Panel_StartFrame, _("Start Frame"));
	AuiNotebook_ProcessSettings->AddPage(Panel_BulbCircle, _("Bulb Circle"));
	AuiNotebook_ProcessSettings->AddPage(Panel_BulbIdentify, _("Bulb Identify"));
	AuiNotebook_ProcessSettings->AddPage(Panel_CustomModel, _("Custom Model"));
	FlexGridSizer7->Add(AuiNotebook_ProcessSettings, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer5->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 2);
	Panel_Generate->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel_Generate);
	FlexGridSizer3->SetSizeHints(Panel_Generate);
	AuiNotebook1->AddPage(Panel_Prepare, _("Prepare"), true);
	AuiNotebook1->AddPage(Panel_Generate, _("Process"));
	FlexGridSizer1->Add(AuiNotebook1, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 2);
	SetSizer(FlexGridSizer1);
	FileDialog1 = new wxFileDialog(this, _("Select file"), wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
	FileDialog2 = new wxFileDialog(this, _("Save the new model"), wxEmptyString, _("custom"), _("*.xmodel"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT|wxFD_CHANGE_DIR, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_PCM_Run,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_PCM_RunClick);
	Connect(ID_TEXTCTRL_GCM_Filename,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnTextCtrl_GCM_FilenameText);
	Connect(ID_BUTTON_GCM_SelectFile,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_GCM_SelectFileClick);
	Connect(ID_BUTTON_CV_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_CV_NextClick);
	Connect(ID_BUTTON_Back1Frame,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_Back1FrameClick);
	Connect(ID_BUTTON_Forward1Frame,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_Forward1FrameClick);
	Connect(ID_BUTTON_Back10Frames,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_Back10FramesClick);
	Connect(ID_BUTTON_Forward10Frames,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_Forward10FramesClick);
	Connect(ID_BUTTON_SF_Back,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_SF_BackClick);
	Connect(ID_BUTTON_SF_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_SF_NextClick);
	Connect(ID_SLIDER_AdjustBlur,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_AdjustBlurCmdScroll);
	Connect(ID_SLIDER_LevelFilterAdjust,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_LevelFilterAdjustCmdScroll);
	Connect(ID_BUTTON_CB_RestoreDefault,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_CB_RestoreDefaultClick);
	Connect(ID_BUTTON_BD_Back,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BD_BackClick);
	Connect(ID_BUTTON_BD_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BD_NextClick);
	Connect(ID_SLIDER_BI_MinRadius,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_MinRadiusCmdSliderUpdated);
	Connect(ID_SLIDER_BI_MaxRadius,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_MaxRadiusCmdSliderUpdated);
	Connect(ID_SLIDER_BI_Sensitivity,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_SensitivityCmdSliderUpdated);
	Connect(ID_BUTTON_BI_Back,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BI_BackClick);
	Connect(ID_BUTTON_BI_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BI_NextClick);
	Connect(ID_BUTTON_CM_Back,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_CM_BackClick);
	Connect(ID_BUTTON_CM_Save,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_CM_SaveClick);
	Connect(ID_AUINOTEBOOK_ProcessSettings,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING,(wxObjectEventFunction)&GenerateCustomModelDialog::OnAuiNotebook_ProcessSettingsPageChanging);
	//*)

    _displaybmp = wxImage(GCM_DISPLAYIMAGEWIDTH, GCM_DISPLAYIMAGEHEIGHT, true);

    StaticBitmap_Preview = new MyGenericStaticBitmap(Panel_Generate, ID_STATICBITMAP_Preview, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER, _T("ID_STATICBITMAP_Preview"));
    StaticBitmap_Preview->SetScaleMode(wxStaticBitmapBase::ScaleMode::Scale_AspectFit);
    StaticBitmap_Preview->SetSizeHints(wxSize(400, 300), wxSize(400, 300));
    StaticBitmap_Preview->SetBitmap(_displaybmp);
    FlexGridSizer14->Insert(0, StaticBitmap_Preview, 1, wxALL | wxEXPAND, 2);
    FlexGridSizer14->Layout();
    FlexGridSizer5->Layout();

    _vr = NULL;

    CVTabEntry();

    ValidateWindow();
}

GenerateCustomModelDialog::~GenerateCustomModelDialog()
{
	//(*Destroy(GenerateCustomModelDialog)
	//*)

    if (_vr != NULL)
    {
        delete _vr;
    }
}
#pragma endregion Constructor

#pragma region Global Functions

void GenerateCustomModelDialog::ValidateWindow()
{
    if (_state == VideoProcessingStates::CHOOSE_VIDEO)
    {
        wxString file = TextCtrl_GCM_Filename->GetValue();
        if (wxFile::Exists(file))
        {
            TextCtrl_GCM_Filename->SetBackgroundColour(*wxWHITE);
            Button_CV_Next->Enable();
        }
        else
        {
            TextCtrl_GCM_Filename->SetBackgroundColour(*wxRED);
            Button_CV_Next->Disable();
        }
        Button_GCM_SelectFile->Enable();
    }
    else if (_state == VideoProcessingStates::FINDING_START_FRAME)
    {
    }
    else if (_state == VideoProcessingStates::CIRCLING_BULBS)
    {
    }
    else if (_state == VideoProcessingStates::IDENTIFYING_BULBS)
    {
    }
    else if (_state == VideoProcessingStates::REVIEW_CUSTOM_MODEL)
    {
    }
}

#pragma endregion Global Functions

#pragma region Image Functions

inline unsigned char GetPixel(int x, int y, int w3, unsigned char* data) {
    return *(data + y*w3 + x * 3);
}

wxImage GenerateCustomModelDialog::CreateImageFromFrame(AVFrame* frame)
{
    wxImage img(frame->width, frame->height, (unsigned char *)frame->data[0], true);
    img.SetType(wxBitmapType::wxBITMAP_TYPE_BMP);

    return img;
}

void GenerateCustomModelDialog::ShowImage(const wxImage& image)
{
    if (image.IsOk())
    {
        _displaybmp = image.Scale(GCM_DISPLAYIMAGEWIDTH, GCM_DISPLAYIMAGEHEIGHT, wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
    }
    StaticBitmap_Preview->SetBitmap(_displaybmp);
    wxYield();
}

void GenerateCustomModelDialog::ShowFrame(int time)
{
    wxImage image = CreateImageFromFrame(_vr->GetNextFrame(time));
    ShowImage(image);
}
#pragma endregion Image Functions

// ***********************************************************
//
// Prepare tab methods
//
// ***********************************************************

#pragma region Prepare

void GenerateCustomModelDialog::SetBulbs(bool nodes, int count, int startch, int node, int ms, int intensity, xOutput* xout)
{
    // node is out of range ... odd
    if (node > count)
    {
        return;
    }

    wxTimeSpan ts = wxDateTime::UNow() - _starttime;
    long curtime = ts.GetMilliseconds().ToLong();
    xout->TimerStart(curtime);

    if (node != -1)
    {
        if (nodes)
        {
            for (int j = 0; j < count; j++)
            {
                if (node == j)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        xout->SetIntensity(startch + j * 3 + i - 1, intensity);
                    }
                }
                else
                {
                    for (int i = 0; i < 3; i++)
                    {
                        xout->SetIntensity(startch + j * 3 + i - 1, 0);
                    }
                }
            }
        }
        else
        {
            for (int j = 0; j < count; j++)
            {
                if (j == node)
                {
                    xout->SetIntensity(startch + j - 1, intensity);
                }
                else
                {
                    xout->SetIntensity(startch + j - 1, 0);
                }
            }
        }
    }
    else
    {
        if (nodes)
        {
            for (int j = 0; j < count; j++)
            {
                for (int i = 0; i < 3; i++)
                {
                    xout->SetIntensity(startch + j * 3 + i - 1, intensity);
                }
            }
        }
        else
        {
            for (int j = 0; j < count; j++)
            {
                xout->SetIntensity(startch + j - 1, intensity);
            }
        }
    }

    xout->TimerEnd();

    wxTimeSpan tsx = wxDateTime::UNow() - _starttime;
    long curtimex = tsx.GetMilliseconds().ToLong();
    while (curtimex - curtime < ms)
    {
        wxYield();
        wxMicroSleep(5000);
        tsx = wxDateTime::UNow() - _starttime;
        curtimex = tsx.GetMilliseconds().ToLong();
    }
}

bool GenerateCustomModelDialog::InitialiseOutputs(xOutput* xout)
{
    long MaxChan;
    bool ok = true;

    for (wxXmlNode* e = _network->GetRoot()->GetChildren(); e != NULL && ok; e = e->GetNext())
    {
        wxString tagname = e->GetName();
        if (tagname == "network")
        {
            wxString tempstr = e->GetAttribute("MaxChannels", "0");
            tempstr.ToLong(&MaxChan);
            wxString NetworkType = e->GetAttribute("NetworkType", "");
            wxString ComPort = e->GetAttribute("ComPort", "");
            wxString BaudRate = e->GetAttribute("BaudRate", "");
            int baud = (BaudRate == _("n/a")) ? 115200 : wxAtoi(BaudRate);
            bool enabled = e->GetAttribute("Enabled", "Yes") == "Yes";
            wxString Description = e->GetAttribute("Description", "");
            static wxString choices;

            int numU = wxAtoi(e->GetAttribute("NumUniverses", "1"));

#ifdef __WXMSW__ //TODO: enumerate comm ports on all platforms -DJ
            TCHAR valname[32];
            /*byte*/TCHAR portname[32];
            DWORD vallen = sizeof(valname);
            DWORD portlen = sizeof(portname);
            HKEY hkey = NULL;
            DWORD err = 0;

            //enum serial comm ports (more user friendly, especially if USB-to-serial ports change):
            //logic based on http://www.cplusplus.com/forum/windows/73821/
            if (choices.empty()) //should this be cached?  it's not really that expensive
            {
                if (!(err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hkey)))
                    for (DWORD inx = 0; !(err = RegEnumValue(hkey, inx, (LPTSTR)valname, &vallen, NULL, NULL, (LPBYTE)portname, &portlen)) || (err == ERROR_MORE_DATA); ++inx)
                    {
                        if (err == ERROR_MORE_DATA) portname[sizeof(portname) / sizeof(portname[0]) - 1] = '\0'; //need to enlarge read buf if this happens; just truncate string for now
                                                                                                                 //                            debug(3, "found port[%d] %d:'%s' = %d:'%s', err 0x%x", inx, vallen, valname, portlen, portname, err);
                        choices += _(", ") + portname;
                        vallen = sizeof(valname);
                        portlen = sizeof(portname);
                    }
                if (err && (err != /*ERROR_FILE_NOT_FOUND*/ ERROR_NO_MORE_ITEMS)) choices = wxString::Format(", error %d (can't get serial comm ports from registry)", err);
                if (hkey) RegCloseKey(hkey);
                //                    if (err) SetLastError(err); //tell caller about last real error
                if (!choices.empty()) choices = "\n(available ports: " + choices.substr(2) + ")";
                else choices = "\n(no available ports)";
            }
#endif // __WXMSW__
            wxString msg = _("Error occurred while connecting to ") + NetworkType + _(" network on ") + ComPort +
                choices +
                _("\n\nThings to check:\n1. Are all required cables plugged in?") +
                _("\n2. Is there another program running that is accessing the port (like the LOR Control Panel)? If so, then you must close the other program and then restart xLights.") +
                _("\n3. If this is a USB dongle, are the FTDI Virtual COM Port drivers loaded?\n\n");

            try
            {
                xout->addnetwork(NetworkType, MaxChan, ComPort, baud, numU, enabled);
            }
            catch (const char *str)
            {
                wxString errmsg(str, wxConvUTF8);
                if (wxMessageBox(msg + errmsg + _("\nProceed anyway?"), _("Communication Error"), wxYES_NO | wxNO_DEFAULT) != wxYES)
                    ok = false;
            }
        }
    }
    return ok;
}

void GenerateCustomModelDialog::OnButton_PCM_RunClick(wxCommandEvent& event)
{
    wxMessageBox("Please prepare to video the model ... press ok when ready to start.", "", 5L, this);

    wxProgressDialog pd("Running light patterns", "", 100, this);

    int count = SpinCtrl_NC_Count->GetValue();
    int startch = SpinCtrl_StartChannel->GetValue();
    int intensity = Slider_Intensity->GetValue();
    bool nodes = RadioBox1->GetSelection() == 0;

    DisableSleepModes();

    _starttime = wxDateTime::UNow();
    xOutput* xout = new xOutput();
    InitialiseOutputs(xout);

    int totaltime = LEADOFF + LEADON + FLAGOFF + FLAGON + FLAGOFF + count * (NODEON + NODEOFF) + FLAGOFF + FLAGON;

    // 3.0 seconds off 0.5 seconds on ... 0.5 seconds off ... 0.5 second on ... 0.5 seconds off
    SetBulbs(nodes, count, startch, -1, LEADOFF, 0, xout);
    UpdateProgress(pd, totaltime);
    SetBulbs(nodes, count, startch, -1, LEADON, intensity, xout);
    UpdateProgress(pd, totaltime);
    SetBulbs(nodes, count, startch, -1, FLAGOFF, 0, xout);
    UpdateProgress(pd, totaltime);
    SetBulbs(nodes, count, startch, -1, FLAGON, intensity, xout);
    UpdateProgress(pd, totaltime);
    SetBulbs(nodes, count, startch, -1, FLAGOFF, 0, xout);
    UpdateProgress(pd, totaltime);

    // then in turn each node on for 0.5 seconds ... all off for 0.2 seconds
    for (int i = 0; i < count; i++)
    {
        SetBulbs(nodes, count, startch, i, NODEON, intensity, xout);
        UpdateProgress(pd, totaltime);
        SetBulbs(nodes, count, startch, i, NODEOFF, 0, xout);
        UpdateProgress(pd, totaltime);
    }

    pd.Update(100);

    // then turn off for 0.5 seconds ... then on for 0.5 seconds then off
    SetBulbs(nodes, count, startch, -1, FLAGOFF, 0, xout);
    UpdateProgress(pd, totaltime);
    SetBulbs(nodes, count, startch, -1, FLAGON, intensity, xout);
    SetBulbs(nodes, count, startch, -1, 0, 0, xout);

    xout->alloff();
    delete xout;

    pd.Update(100);
    pd.Close();
    SetFocus();

    EnableSleepModes();

    wxMessageBox("Please stop the video.", "", 5L, this);
    ValidateWindow();
}

#pragma endregion Prepare

// ***********************************************************
//
// Generate tab methods
//
// ***********************************************************

#pragma region Generate

#pragma region Generate Tab General Methods

void GenerateCustomModelDialog::UpdateProgress(wxProgressDialog& pd, int totaltime)
{
    wxTimeSpan ts = wxDateTime::UNow() - _starttime;
    int progress = ts.GetMilliseconds().ToLong() * 100 / totaltime;
    if (progress > 100) progress = 100;
    pd.Update(progress);
}

void GenerateCustomModelDialog::OnAuiNotebook_ProcessSettingsPageChanging(wxAuiNotebookEvent& event)
{
    int page = event.GetSelection();
    if (_state == VideoProcessingStates::CHOOSE_VIDEO && page != PAGE_CHOOSEVIDEO)
    {
        event.Veto();
    }
    else if (_state == VideoProcessingStates::FINDING_START_FRAME && page != PAGE_STARTFRAME)
    {
        event.Veto();
    }
    else if (_state == VideoProcessingStates::CIRCLING_BULBS && page != PAGE_CIRCLEBULB)
    {
        event.Veto();
    }
    else if (_state == VideoProcessingStates::IDENTIFYING_BULBS && page != PAGE_BULBIDENTIFY)
    {
        event.Veto();
    }
    else if (_state == VideoProcessingStates::REVIEW_CUSTOM_MODEL && page != PAGE_REVIEWMODEL)
    {
        event.Veto();
    }
}

void GenerateCustomModelDialog::SwapPage(int oldp, const int newp)
{
    AuiNotebook_ProcessSettings->ChangeSelection(newp);
}

#pragma endregion Generate Tab General Methods

// ***********************************************************
// Choose Video tab methods
// ***********************************************************

#pragma region Choose Video

void GenerateCustomModelDialog::CVTabEntry()
{
    Gauge_Progress->SetValue(0);
    _state = VideoProcessingStates::CHOOSE_VIDEO;
    if (wxFile::Exists(std::string(TextCtrl_GCM_Filename->GetValue().c_str())))
    {
        VideoReader vr(std::string(TextCtrl_GCM_Filename->GetValue().c_str()), 800, 600, true);
        ShowImage(CreateImageFromFrame(vr.GetNextFrame(0)));
    }
}

void GenerateCustomModelDialog::OnButton_GCM_SelectFileClick(wxCommandEvent& event)
{
    FileDialog1->SetWildcard(VIDEOWILDCARD);
    if (FileDialog1->ShowModal() == wxID_OK)
    {
        TextCtrl_GCM_Filename->SetValue(FileDialog1->GetDirectory() + "/" + FileDialog1->GetFilename());
        CVTabEntry();
    }
    ValidateWindow();
}

void GenerateCustomModelDialog::OnTextCtrl_GCM_FilenameText(wxCommandEvent& event)
{
    ValidateWindow();
}

void GenerateCustomModelDialog::SetStartFrame(int time)
{
    _startframetime = time;
    _startFrame = CreateImageFromFrame(_vr->GetNextFrame(time));
    ShowImage(_startFrame);
}

void GenerateCustomModelDialog::OnButton_CV_NextClick(wxCommandEvent& event)
{
    Button_CV_Next->Disable();
    Button_GCM_SelectFile->Disable();

    DoStartFrameIdentify();

    SFTabEntry();
    SwapPage(PAGE_CHOOSEVIDEO, PAGE_STARTFRAME);
    ValidateWindow();
}

#pragma endregion Choose Video

// ***********************************************************
// Start Frame tab methods
// ***********************************************************

#pragma region Start Frame

void GenerateCustomModelDialog::DoStartFrameIdentify()
{
    if (_vr != NULL)
    {
        delete _vr;
        _vr = NULL;
    }

    _vr = new VideoReader(std::string(TextCtrl_GCM_Filename->GetValue().c_str()), 800, 600, true);

    if (_vr == NULL)
    {
        wxMessageBox("Unable to process video.");
        ValidateWindow();
        return;
    }

    SetCursor(wxCURSOR_WAIT);

    SetStartFrame(FindStartFrame(_vr));
    ValidateStartFrame();

    SetCursor(wxCURSOR_ARROW);
}

void GenerateCustomModelDialog::SFTabEntry()
{
    Gauge_Progress->SetValue(5);
    _state = VideoProcessingStates::FINDING_START_FRAME;
    ShowImage(_startFrame);
    Button_Back10Frames->Enable();
    Button_Back1Frame->Enable();
    Button_Forward10Frames->Enable();
    Button_Forward1Frame->Enable();
    Button_SF_Next->Enable();
    Button_SF_Back->Enable();
    ValidateWindow();
}

// A frame looks like a valid start frame if another frame LEADON + FLAGOFF MS in the future is about as bright
bool GenerateCustomModelDialog::LooksLikeStartFrame(int candidateframe)
{
    wxImage image = CreateImageFromFrame(_vr->GetNextFrame(candidateframe));
    wxImage nextimage = CreateImageFromFrame(_vr->GetNextFrame(candidateframe + LEADON + FLAGOFF));
    float fimage = CalcFrameBrightness(image);
    float fnextimage = CalcFrameBrightness(nextimage);

    if (fnextimage > fimage * 0.9 && fnextimage < fimage * 1.1)
    {
        // within 10% close enough
        return true;
    }

    return false;
}

float GenerateCustomModelDialog::CalcFrameBrightness(wxImage& image)
{
    wxImage grey = image.ConvertToGreyscale();
    int w = image.GetWidth();
    int h = image.GetHeight();
    int64_t total = 0;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            total += *(grey.GetData() + w * 3 * y + x * 3);
        }
    }

    return (float)((double)total /
        ((double)w * (double)h) / 255.0);
}

// returns the MS of the best start frame - 0.1 MS into what looks like a bright section of the video that lasts about 3 seconds
int GenerateCustomModelDialog::FindStartFrame(VideoReader* vr)
{
    std::list<float> framebrightness;

    StaticBitmap_Preview->SetEraseBackground(false);

    // scan first STARTSCANSECS seconds of video build a list of average frame brightnesses
    for (int s = 0; s < STARTSCANSECS; s++)
    {
        for (int ss = 0; ss < 20; ss++)
        {
            int ms = s * 1000 + ss * FRAMEMS;
            wxImage img = CreateImageFromFrame(vr->GetNextFrame(ms));
            ShowImage(img);
            framebrightness.push_back(CalcFrameBrightness(img));
        }
    }

    StaticBitmap_Preview->SetEraseBackground(true);

    // find the maximum number of frames in the video that it is about a set of brightness thresholds
    std::map<int, int> levelmaxlen;
    std::map<int, int> levelmaxstart;
    float level = 0.1;
    for (int i = 0; i < 9; i++)
    {
        int maxrunlength = 0;
        int currunlength = 0;
        int maxrunstart = 0;
        int currunstart = 0;

        auto it = framebrightness.begin();
        for (int j = 0; j < framebrightness.size(); j++)
        {
            if (*it > level)
            {
                if (currunlength == 0)
                {
                    currunstart = j;
                }
                currunlength++;
            }
            else
            {
                if (currunlength > maxrunlength)
                {
                    maxrunlength = currunlength;
                    maxrunstart = currunstart;
                    currunlength = 0;
                }
            }
            it++;
        }
        if (currunlength > maxrunlength)
        {
            maxrunlength = currunlength;
            maxrunstart = currunstart;
            currunlength = 0;
        }
        levelmaxlen[(int)(level*10.0)] = maxrunlength;
        levelmaxstart[(int)(level*10.0)] = maxrunstart;
        level += 0.1;
    }

    // look for thresholds that are close to LEADON long
    std::map<int, bool> suitable;
    for (int l = 1; l < 10; l++)
    {
        if (levelmaxlen[l] > LEADON / FRAMEMS - 5 && levelmaxlen[l] < LEADON / FRAMEMS + 5)
        {
            suitable[l] = true;
        }
        else
        {
            suitable[l] = false;
        }
    }

    // choose the best threshold to use
    int first = 0;
    int last = 0;
    int curfirst = 0;
    int curlast = 0;
    for (int l = 1; l < 10; l++)
    {
        if (suitable[l])
        {
            curlast = l;
            if (curfirst == 0)
            {
                curfirst = l;
            }
        }
        else
        {
            if (curlast - curfirst > last - first)
            {
                last = curlast;
                first = curfirst;
            }
            curlast = 0;
            curfirst = 0;
        }
    }
    if (curlast - curfirst > last - first)
    {
        last = curlast;
        first = curfirst;
    }

    int bestlevel;
    if (first == 0)
    {
        bestlevel = 7;
    }
    else
    {
        bestlevel = ((int)(((float)last + (float)first) / 2.0 * 10.0)) / 10;
    }

    // pick a point 0.1 secs into the high period as our start frame
    int candidateframe = levelmaxstart[bestlevel] * FRAMEMS;
    candidateframe += DELAYMSUNTILSAMPLE;

    // check the second all on event is there ... if not move up to 10 frames forward looking for it
    for (int i = 0; i < 10; i++)
    {
        if (LooksLikeStartFrame(candidateframe))
        {
            break;
        }
        else
        {
            candidateframe++;
        }
    }

    return candidateframe;
}

void GenerateCustomModelDialog::ValidateStartFrame()
{
    if (LooksLikeStartFrame(_startframetime))
    {
        StaticText_StartFrameOk->SetLabel("Looks ok.");
        StaticText_StartFrameOk->SetForegroundColour(*wxGREEN);
    }
    else
    {
        StaticText_StartFrameOk->SetLabel("Looks wrong.");
        StaticText_StartFrameOk->SetForegroundColour(*wxRED);
    }
}

void GenerateCustomModelDialog::MoveStartFrame(int by)
{
    _startframetime += by * FRAMEMS;

    if (_startframetime < 0)
    {
        _startframetime = 0;
    }

    if (_startframetime > _vr->GetLengthMS())
    {
        _startframetime = _vr->GetLengthMS();
    }
}

void GenerateCustomModelDialog::OnButton_Back1FrameClick(wxCommandEvent& event)
{
    if (!_busy)
    {
        _busy = true;
        SetCursor(wxCURSOR_WAIT);
        MoveStartFrame(-1);
        ValidateStartFrame();
        SetStartFrame(_startframetime);
        ValidateWindow();
        SetCursor(wxCURSOR_ARROW);
        _busy = false;
    }
}

void GenerateCustomModelDialog::OnButton_Forward1FrameClick(wxCommandEvent& event)
{
    if (!_busy)
    {
        _busy = true;
        SetCursor(wxCURSOR_WAIT);
        MoveStartFrame(1);
        ValidateStartFrame();
        SetStartFrame(_startframetime);
        ValidateWindow();
        SetCursor(wxCURSOR_ARROW);
        _busy = false;
    }
}

void GenerateCustomModelDialog::OnButton_Back10FramesClick(wxCommandEvent& event)
{
    if (!_busy)
    {
        _busy = true;
        SetCursor(wxCURSOR_WAIT);
        MoveStartFrame(-10);
        ValidateStartFrame();
        SetStartFrame(_startframetime);
        ValidateWindow();
        SetCursor(wxCURSOR_ARROW);
        _busy = false;
    }
}

void GenerateCustomModelDialog::OnButton_Forward10FramesClick(wxCommandEvent& event)
{
    if (!_busy)
    {
        _busy = true;
        SetCursor(wxCURSOR_WAIT);
        MoveStartFrame(10);
        ValidateStartFrame();
        SetStartFrame(_startframetime);
        ValidateWindow();
        SetCursor(wxCURSOR_ARROW);
        _busy = false;
    }
}

void GenerateCustomModelDialog::OnButton_SF_NextClick(wxCommandEvent& event)
{
    Button_Back10Frames->Disable();
    Button_Back1Frame->Disable();
    Button_Forward10Frames->Disable();
    Button_Forward1Frame->Disable();
    Button_SF_Next->Disable();
    Button_SF_Back->Disable();

    DoBulbCircle();

    CBTabEntry();

    _state = VideoProcessingStates::CIRCLING_BULBS;
    SwapPage(PAGE_STARTFRAME, PAGE_CIRCLEBULB);

    ValidateWindow();
}

void GenerateCustomModelDialog::OnButton_SF_BackClick(wxCommandEvent& event)
{
    SwapPage(PAGE_STARTFRAME, PAGE_CHOOSEVIDEO);
    CVTabEntry();
    ValidateWindow();
}

#pragma endregion Start Frame

// ***********************************************************
// Bulb Circle tab methods
// ***********************************************************

#pragma region Bulb Circle

void GenerateCustomModelDialog::DoBulbCircle()
{
    // store the start frame in greyscale as we only need to do this once
    _greyFrame = _startFrame.ConvertToGreyscale();

    OutlineBulbs();
}

void GenerateCustomModelDialog::CBTabEntry()
{
    Gauge_Progress->SetValue(10);
    _state = VideoProcessingStates::CIRCLING_BULBS;
    ShowImage(_cbFrame);
    Slider_LevelFilterAdjust->Enable();
    Slider_AdjustBlur->Enable();
    Button_BD_Back->Enable();
    Button_BD_Next->Enable();
    Button_CB_RestoreDefault->Enable();
}

wxImage GenerateCustomModelDialog::OutlineBulbs()
{
    if (!_busy)
    {
        _busy = true;

        // blur the photo to eliminate the noise
        wxImage imgblur = _greyFrame.Blur(Slider_AdjustBlur->GetValue());

        // now use a threshold to make it b&w
        _bwFrame = imgblur;
        ApplyThreshold(_bwFrame, Slider_LevelFilterAdjust->GetValue());

        // now find the edges in the image
        _cbFrame = DetectEdges(_bwFrame);
        ShowImage(_cbFrame);
        _busy = false;
    }

    return _cbFrame;
}

inline bool Get3Pixel(int x1, int x2, int x3, int y1, int y2, int y3, int w3, unsigned char* data)
{
    return ((GetPixel(x1, y1, w3, data) == 0) &&
        (GetPixel(x2, y2, w3, data) == 0) &&
        (GetPixel(x3, y3, w3, data) == 0));
}

inline bool IsEdge(int x, int y, int w3, unsigned char* data)
{
    return (GetPixel(x, y, w3, data) > 0) &&
        (Get3Pixel(x - 1, x, x + 1, y - 1, y - 1, y - 1, w3, data) ||
            Get3Pixel(x - 1, x, x + 1, y + 1, y + 1, y + 1, w3, data) ||
            Get3Pixel(x - 1, x - 1, x - 1, y - 1, y, y + 1, w3, data) ||
            Get3Pixel(x + 1, x + 1, x + 1, y - 1, y, y + 1, w3, data) ||
            Get3Pixel(x - 1, x - 1, x, y, y - 1, y - 1, w3, data) ||
            Get3Pixel(x, x + 1, x + 1, y - 1, y - 1, y, w3, data) ||
            Get3Pixel(x + 1, x + 1, x, y, y + 1, y + 1, w3, data) ||
            Get3Pixel(x, x - 1, x - 1, y + 1, y + 1, y, w3, data));
}

wxImage GenerateCustomModelDialog::DetectEdges(wxImage& image)
{
    int w = image.GetWidth();
    int w3 = w * 3;
    int h = image.GetHeight();
    wxImage res(w, h, true);
    unsigned char* data = image.GetData();
    for (int y = 1; y < h - 1; y++)
    {
        for (int x = 1; x < w - 1; x++)
        {
            if (IsEdge(x, y, w3, data))
            {
                unsigned char* p = (res.GetData() + y * w3 + x * 3);
                *p = 255;
                *(p + 1) = 255;
                *(p + 2) = 255;
            }
        }
    }

    return res;
}

void GenerateCustomModelDialog::ApplyThreshold(wxImage& image, int threshold)
{
    for (int i = 0; i < image.GetWidth() * image.GetHeight() * 3; i++)
    {
        if (*(image.GetData() + i) > threshold)
        {
            *(image.GetData() + i) = 255;
        }
        else
        {
            *(image.GetData() + i) = 0;
        }
    }
}

void GenerateCustomModelDialog::OnSlider_LevelFilterAdjustCmdScroll(wxScrollEvent& event)
{
    OutlineBulbs();
    ValidateWindow();
}

void GenerateCustomModelDialog::OnSlider_AdjustBlurCmdScroll(wxScrollEvent& event)
{
    OutlineBulbs();
    ValidateWindow();
}

void GenerateCustomModelDialog::OnButton_BD_BackClick(wxCommandEvent& event)
{
    SFTabEntry();
    SwapPage(PAGE_CIRCLEBULB, PAGE_STARTFRAME);
}

void GenerateCustomModelDialog::OnButton_BD_NextClick(wxCommandEvent& event)
{
    Slider_LevelFilterAdjust->Disable();
    Slider_AdjustBlur->Disable();
    Button_CB_RestoreDefault->Disable();
    Button_BD_Back->Disable();
    Button_BD_Next->Disable();

    DoBulbIdentify();

    BITabEntry();
    SwapPage(PAGE_BULBIDENTIFY, PAGE_CIRCLEBULB);
    ValidateWindow();
}

void GenerateCustomModelDialog::OnButton_CB_RestoreDefaultClick(wxCommandEvent& event)
{
    Slider_AdjustBlur->SetValue(5);
    Slider_BI_Sensitivity->SetValue(200);
    OutlineBulbs();
    ValidateWindow();
}

#pragma endregion Bulb Circle

// ***********************************************************
// Bulb Identify tab methods
// ***********************************************************

#pragma region Bulb Identify

void GenerateCustomModelDialog::DoBulbIdentify()
{

}

void GenerateCustomModelDialog::BITabEntry()
{
    Gauge_Progress->SetValue(15);
    _state = VideoProcessingStates::IDENTIFYING_BULBS;
    ShowImage(_biFrame);
    Slider_BI_MinRadius->Enable();
    Slider_BI_MinRadius->Enable();
    Slider_BI_Sensitivity->Enable();
    Button_BI_Next->Enable();
    Button_BI_Back->Enable();
}

// returns a number between 0-1 saying how much edge exists on the circle edge
// https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
float GenerateCustomModelDialog::CalcPoint(wxImage& edge, int x0, int y0, int radius)
{
    int accum = 0;
    int points = 0;
    int w3 = edge.GetWidth() * 3;
    unsigned char* data = edge.GetData();
    int x = 0, y = radius;
    int dp = 1 - radius;
    do
    {
        if (dp < 0)
        {
            dp = dp + 2 * (++x) + 3;
        }
        else
        {
            dp = dp + 2 * (++x) - 2 * (--y) + 5;
        }

        if (GetPixel(x0 + x, y0 + y, w3, data) > 0)
        {
            accum++;
        }
        if (GetPixel(x0 - x, y0 + y, w3, data) > 0)
        {
            accum++;
        }
        if (GetPixel(x0 + x, y0 - y, w3, data) > 0)
        {
            accum++;
        }
        if (GetPixel(x0 - x, y0 - y, w3, data) > 0)
        {
            accum++;
        }
        if (GetPixel(x0 + y, y0 + x, w3, data) > 0)
        {
            accum++;
        }
        if (GetPixel(x0 - y, y0 + x, w3, data) > 0)
        {
            accum++;
        }
        if (GetPixel(x0 + y, y0 - x, w3, data) > 0)
        {
            accum++;
        }
        if (GetPixel(x0 - y, y0 - x, w3, data) > 0)
        {
            accum++;
        }
        points += 8;
    } while (x < y);

    return (float)accum / float(points);
}

// for a given radius find all the centres where there is a chance there is a circle
// this is based on circles whose edge score > CIRCLETHRESHOLD
#define CIRCLETHRESHOLD 0.5
// This will not detect bulbs hard up against the edge of the video
std::map<xlPoint, int> GenerateCustomModelDialog::CircleDetect(wxImage& mask, wxImage& edge, int radius)
{
    std::map<xlPoint, int> accum;
    int w = edge.GetWidth();
    int h = edge.GetHeight();
    int w3 = w * 3;
    unsigned char* maskdata = mask.GetData();
    for (int x = 0 + radius; x < w - radius; x++)
    {
        for (int y = 0 + radius; y < h - radius; y++)
        {
            // the centre must be white in our mask
            if (GetPixel(x, y, w3, maskdata) > 0)
            {
                xlPoint p(x, y);
                float score = CalcPoint(edge, x, y, radius);
                if (score > CIRCLETHRESHOLD)
                {
                    accum[p] = score * 255;
                }
            }
        }
    }
    return accum;
}

std::list<wxPoint> GenerateCustomModelDialog::CircleDetect(wxImage& mask, wxImage& edge, int minr, int maxr)
{
    std::list<wxPoint> res;
    std::map<int/*radius*/, std::map<xlPoint, int/*score*/>> cone;

    for (int i = minr; i <= maxr; i++)
    {
        cone[i - minr] = CircleDetect(mask, edge, i);
    }

    // reorganise by points
    std::map<xlPoint, std::map<int/*radius*/, int/*score*/>> centres;
    for (auto r = cone.begin(); r != cone.end(); ++r)
    {
        for (auto p = cone[r->first].begin(); p != cone[r->first].end(); ++p)
        {
            centres[p->first][r->first] = p->second;
        }
    }

    // for each point find the best scoring radius
    std::map<xlPoint, int> bestradius;
    for (auto p = centres.begin(); p != centres.end(); ++p)
    {
        int maxscore = 0;
        int maxr = -1;

        for (auto r = centres[p->first].begin(); r != centres[p->first].end(); ++r)
        {
            if (r->second >= maxscore)
            {
                maxscore = r->second;
                maxr = r->first;
            }
        }
        bestradius[p->first] = maxr + minr; // restoring radius to true value
    }

    // coallesc the centres - ie anything centred less than COALLESCSIZE limit away is the same
    //for (int i = max; i >= 0; i++)
    //{
    //    for(auto )
    //}

    for (auto c = bestradius.begin(); c != bestradius.end(); c++)
    {
        res.push_back((wxPoint)c->first);
    }

    return res;
}

wxImage GenerateCustomModelDialog::CreateDetectMask(std::list<wxPoint> centres, wxImage ref, bool includeimage, wxColor col)
{
    wxBitmap bmp(ref.GetWidth(), ref.GetHeight());
    wxMemoryDC* dc = new wxMemoryDC(bmp);

    if (includeimage)
    {
        dc->DrawBitmap(ref, wxPoint(0, 0), false);
    }

    wxBrush b(col, wxBrushStyle::wxBRUSHSTYLE_SOLID);
    wxPen p(col, 1);

    dc->SetBrush(b);
    dc->SetPen(p);

    for (auto c = centres.begin(); c != centres.end(); c++)
    {
        dc->DrawCircle(*c, 3);
    }

    return bmp.ConvertToImage();
}

std::list<wxPoint> GenerateCustomModelDialog::FindLights(wxImage& image)
{
    std::list<wxPoint> res;

    if (!_busy)
    {
        _busy = true;
        // now circle detect
        res = CircleDetect(_bwFrame, _cbFrame, 5, 40);

        wxImage detectmask = CreateDetectMask(res, _cbFrame, true, *wxRED);
        ShowImage(detectmask);
        _busy = false;
    }

    return res;
}

void GenerateCustomModelDialog::OnSlider_BI_MinRadiusCmdSliderUpdated(wxScrollEvent& event)
{
}

void GenerateCustomModelDialog::OnSlider_BI_MaxRadiusCmdSliderUpdated(wxScrollEvent& event)
{
}

void GenerateCustomModelDialog::OnSlider_BI_SensitivityCmdSliderUpdated(wxScrollEvent& event)
{
}

void GenerateCustomModelDialog::OnButton_BI_NextClick(wxCommandEvent& event)
{
}

void GenerateCustomModelDialog::OnButton_BI_BackClick(wxCommandEvent& event)
{
}

#pragma endregion Bulb Identify

// ***********************************************************
// Custom Model tab methods
// ***********************************************************

#pragma region Custom Model

void GenerateCustomModelDialog::OnButton_CM_BackClick(wxCommandEvent& event)
{
}

void GenerateCustomModelDialog::OnButton_CM_SaveClick(wxCommandEvent& event)
{
}

#pragma endregion Custom Model

#pragma endregion Generate



