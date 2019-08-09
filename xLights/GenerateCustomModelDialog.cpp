//(*InternalHeaders(GenerateCustomModelDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/dcmemory.h>
#include <wx/log.h>
#include <wx/generic/statbmpg.h>

#include <map>
#include <list>

#include "GenerateCustomModelDialog.h"
#include "effects/VideoPanel.h"
#include "xLightsVersion.h"
#include "xLightsXmlFile.h"
#include "outputs/OutputManager.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>

#define GCM_DISPLAYIMAGEWIDTH 800
#define GCM_DISPLAYIMAGEHEIGHT 600

#define PAGE_PREPARE 0
#define PAGE_GENERATE 1

#define PAGE_MODELTYPE 0
#define PAGE_CHOOSEVIDEO 1
#define PAGE_STARTFRAME 2
#define PAGE_BULBIDENTIFY 3
#define PAGE_MANUALIDENTIFY 4
#define PAGE_REVIEWMODEL 5

#define STARTSCANSECS 15
#define FRAMEMS 50
#define LEADOFF 3000
#define LEADON 500
#define FLAGON 500
#define FLAGOFF 500
#define NODEON 500
#define NODEOFF 200
#define DELAYMSUNTILSAMPLE 0

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
const long GenerateCustomModelDialog::ID_RADIOBUTTON1 = wxNewId();
const long GenerateCustomModelDialog::ID_RADIOBUTTON2 = wxNewId();
const long GenerateCustomModelDialog::ID_SPINCTRL_NC_Count = wxNewId();
const long GenerateCustomModelDialog::ID_SPINCTRL_StartChannel = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_Intensity = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_PCM_Run = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_Prepare = wxNewId();
const long GenerateCustomModelDialog::ID_RADIOBUTTON3 = wxNewId();
const long GenerateCustomModelDialog::ID_RADIOBUTTON4 = wxNewId();
const long GenerateCustomModelDialog::ID_RADIOBUTTON5 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_MT_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL1 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT10 = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_GCM_Filename = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_GCM_SelectFile = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_CV_Back = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_CV_Manual = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_CV_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_ChooseVideo = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT3 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Back1Frame = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Forward1Frame = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Back10Frames = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Forward10Frames = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT_StartFrameOk = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT_StartTime = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_SF_Back = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON6 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_SF_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_StartFrame = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT5 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT1 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_AdjustBlur = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_BC_Blur = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT8 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_BI_Sensitivity = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_BI_Sensitivity = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT6 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_BI_MinSeparation = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_BI_MinSeparation = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT2 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_BI_Contrast = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_BI_Contrast = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT7 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_BI_MinScale = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_BI_MinScale = wxNewId();
const long GenerateCustomModelDialog::ID_CHECKBOX_BI_IsSteady = wxNewId();
const long GenerateCustomModelDialog::ID_CHECKBOX_BI_ManualUpdate = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT12 = wxNewId();
const long GenerateCustomModelDialog::ID_SPINCTRL1 = wxNewId();
const long GenerateCustomModelDialog::ID_GAUGE1 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_BI_Update = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_CB_RestoreDefault = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_BI_Status = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_BI_Back = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_BI_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_BulbIdentify = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT11 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON3 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON1 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON7 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON8 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT4 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON2 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON4 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON5 = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL2 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT9 = wxNewId();
const long GenerateCustomModelDialog::ID_GRID_CM_Result = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Shrink = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Grow = wxNewId();
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

GenerateCustomModelDialog::GenerateCustomModelDialog(wxWindow* parent, OutputManager* outputManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;
    _busy = false;

	//(*Initialize(GenerateCustomModelDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer11;
	wxFlexGridSizer* FlexGridSizer12;
	wxFlexGridSizer* FlexGridSizer13;
	wxFlexGridSizer* FlexGridSizer15;
	wxFlexGridSizer* FlexGridSizer16;
	wxFlexGridSizer* FlexGridSizer17;
	wxFlexGridSizer* FlexGridSizer18;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer20;
	wxFlexGridSizer* FlexGridSizer21;
	wxFlexGridSizer* FlexGridSizer22;
	wxFlexGridSizer* FlexGridSizer23;
	wxFlexGridSizer* FlexGridSizer24;
	wxFlexGridSizer* FlexGridSizer25;
	wxFlexGridSizer* FlexGridSizer26;
	wxFlexGridSizer* FlexGridSizer27;
	wxFlexGridSizer* FlexGridSizer28;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer30;
	wxFlexGridSizer* FlexGridSizer31;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer9;
	wxStaticText* StaticText14;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText4;
	wxStaticText* StaticText5;
	wxStaticText* StaticText6;
	wxStaticText* StaticText7;
	wxStaticText* StaticText8;

	Create(parent, id, _("Generate Custom Models"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	AuiNotebook1 = new wxAuiNotebook(this, ID_AUINOTEBOOK1, wxDefaultPosition, wxSize(1200,700), wxTAB_TRAVERSAL);
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
	StaticText14 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	NodesRadioButton = new wxRadioButton(Panel_Prepare, ID_RADIOBUTTON1, _("Nodes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
	NodesRadioButton->SetValue(true);
	FlexGridSizer6->Add(NodesRadioButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SingleChannelRadioButton = new wxRadioButton(Panel_Prepare, ID_RADIOBUTTON2, _("Single Channel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
	FlexGridSizer6->Add(SingleChannelRadioButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Node/Channel Count"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_NC_Count = new wxSpinCtrl(Panel_Prepare, ID_SPINCTRL_NC_Count, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 99999, 1, _T("ID_SPINCTRL_NC_Count"));
	SpinCtrl_NC_Count->SetValue(_T("1"));
	FlexGridSizer6->Add(SpinCtrl_NC_Count, 1, wxALL|wxEXPAND, 2);
	StaticText6 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Start Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_StartChannel = new wxSpinCtrl(Panel_Prepare, ID_SPINCTRL_StartChannel, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 999999, 1, _T("ID_SPINCTRL_StartChannel"));
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
	FlexGridSizer14 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer14->AddGrowableCol(0);
	FlexGridSizer14->AddGrowableRow(0);
	FlexGridSizer5->Add(FlexGridSizer14, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer7 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	FlexGridSizer7->AddGrowableRow(0);
	AuiNotebook_ProcessSettings = new wxAuiNotebook(Panel_Generate, ID_AUINOTEBOOK_ProcessSettings, wxDefaultPosition, wxDefaultSize, 0);
	Panel1 = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL1, wxPoint(73,8), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer24 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer24->AddGrowableCol(0);
	FlexGridSizer24->AddGrowableRow(1);
	FlexGridSizer25 = new wxFlexGridSizer(0, 1, 0, 0);
	NodesRadioButtonPg2 = new wxRadioButton(Panel1, ID_RADIOBUTTON3, _("Nodes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON3"));
	NodesRadioButtonPg2->SetValue(true);
	FlexGridSizer25->Add(NodesRadioButtonPg2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SCRadioButton = new wxRadioButton(Panel1, ID_RADIOBUTTON4, _("Single Channels"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON4"));
	FlexGridSizer25->Add(SCRadioButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SLRadioButton = new wxRadioButton(Panel1, ID_RADIOBUTTON5, _("Static Lights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON5"));
	FlexGridSizer25->Add(SLRadioButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer24->Add(FlexGridSizer25, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer24->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer26 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_MT_Next = new wxButton(Panel1, ID_BUTTON_MT_Next, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MT_Next"));
	FlexGridSizer26->Add(Button_MT_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer24->Add(FlexGridSizer26, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel1->SetSizer(FlexGridSizer24);
	FlexGridSizer24->Fit(Panel1);
	FlexGridSizer24->SetSizeHints(Panel1);
	Panel_ChooseVideo = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_ChooseVideo, wxPoint(18,15), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_ChooseVideo"));
	FlexGridSizer21 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer21->AddGrowableCol(0);
	FlexGridSizer21->AddGrowableRow(2);
	StaticText_CM_Request = new wxStaticText(Panel_ChooseVideo, ID_STATICTEXT10, _("Insert text here"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer21->Add(StaticText_CM_Request, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer22 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer22->AddGrowableCol(1);
	StaticText3 = new wxStaticText(Panel_ChooseVideo, wxID_ANY, _("Media File"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer22->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_GCM_Filename = new wxTextCtrl(Panel_ChooseVideo, ID_TEXTCTRL_GCM_Filename, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_GCM_Filename"));
	FlexGridSizer22->Add(TextCtrl_GCM_Filename, 1, wxALL|wxEXPAND, 2);
	Button_GCM_SelectFile = new wxButton(Panel_ChooseVideo, ID_BUTTON_GCM_SelectFile, _("..."), wxDefaultPosition, wxSize(29,28), 0, wxDefaultValidator, _T("ID_BUTTON_GCM_SelectFile"));
	FlexGridSizer22->Add(Button_GCM_SelectFile, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer21->Add(FlexGridSizer22, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer21->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer23 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_CV_Back = new wxButton(Panel_ChooseVideo, ID_BUTTON_CV_Back, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CV_Back"));
	FlexGridSizer23->Add(Button_CV_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_CV_Manual = new wxButton(Panel_ChooseVideo, ID_BUTTON_CV_Manual, _("Manual"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CV_Manual"));
	FlexGridSizer23->Add(Button_CV_Manual, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_CV_Next = new wxButton(Panel_ChooseVideo, ID_BUTTON_CV_Next, _("Automatic"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CV_Next"));
	FlexGridSizer23->Add(Button_CV_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer21->Add(FlexGridSizer23, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_ChooseVideo->SetSizer(FlexGridSizer21);
	FlexGridSizer21->Fit(Panel_ChooseVideo);
	FlexGridSizer21->SetSizeHints(Panel_ChooseVideo);
	Panel_StartFrame = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_StartFrame, wxPoint(43,126), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_StartFrame"));
	FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	FlexGridSizer10->AddGrowableRow(4);
	StaticText11 = new wxStaticText(Panel_StartFrame, ID_STATICTEXT3, _("This is the frame the scan has identified as being the most likely to show all your bulbs. It should be a frame showing all the bulbs on and it should be near the start of the first flash of all the bulbs before each bulb was lit in turn.\nYou can move it forward or backwards with the buttons below.\n\nClick next when you are happy with it."), wxDefaultPosition, wxSize(658,99), 0, _T("ID_STATICTEXT3"));
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
	StaticText_StartTime = new wxStaticText(Panel_StartFrame, ID_STATICTEXT_StartTime, wxEmptyString, wxDefaultPosition, wxSize(95,16), 0, _T("ID_STATICTEXT_StartTime"));
	FlexGridSizer10->Add(StaticText_StartTime, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_SF_Back = new wxButton(Panel_StartFrame, ID_BUTTON_SF_Back, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SF_Back"));
	FlexGridSizer12->Add(Button_SF_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_SF_Manual = new wxButton(Panel_StartFrame, ID_BUTTON6, _("Manual"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	FlexGridSizer12->Add(Button_SF_Manual, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_SF_Next = new wxButton(Panel_StartFrame, ID_BUTTON_SF_Next, _("Automatic"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SF_Next"));
	FlexGridSizer12->Add(Button_SF_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10->Add(FlexGridSizer12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_StartFrame->SetSizer(FlexGridSizer10);
	FlexGridSizer10->Fit(Panel_StartFrame);
	FlexGridSizer10->SetSizeHints(Panel_StartFrame);
	Panel_BulbIdentify = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_BulbIdentify, wxPoint(176,18), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_BulbIdentify"));
	FlexGridSizer15 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer15->AddGrowableCol(0);
	FlexGridSizer15->AddGrowableRow(5);
	StaticText_BI = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT5, _("The red circles on the image show the bulbs we have identify. Adjust the sensitivity if there are bulbs missing or phantom bulbs identified.\n\nClick next when you are happy that all bulbs have been detected."), wxDefaultPosition, wxSize(652,75), 0, _T("ID_STATICTEXT5"));
	FlexGridSizer15->Add(StaticText_BI, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer16 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer16->AddGrowableCol(1);
	StaticText9 = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT1, _("Blur"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer16->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_AdjustBlur = new wxSlider(Panel_BulbIdentify, ID_SLIDER_AdjustBlur, 1, 1, 30, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_AdjustBlur"));
	FlexGridSizer16->Add(Slider_AdjustBlur, 1, wxALL|wxEXPAND, 2);
	TextCtrl_BC_Blur = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL_BC_Blur, _("1"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_BC_Blur"));
	FlexGridSizer16->Add(TextCtrl_BC_Blur, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText_BI_Slider = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT8, _("Sensitivity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer16->Add(StaticText_BI_Slider, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_BI_Sensitivity = new wxSlider(Panel_BulbIdentify, ID_SLIDER_BI_Sensitivity, 127, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BI_Sensitivity"));
	FlexGridSizer16->Add(Slider_BI_Sensitivity, 1, wxALL|wxEXPAND, 2);
	TextCtrl_BI_Sensitivity = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL_BI_Sensitivity, _("127"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_BI_Sensitivity"));
	FlexGridSizer16->Add(TextCtrl_BI_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText13 = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT6, _("Minimum Separation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer16->Add(StaticText13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_BI_MinSeparation = new wxSlider(Panel_BulbIdentify, ID_SLIDER_BI_MinSeparation, 100, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BI_MinSeparation"));
	FlexGridSizer16->Add(Slider_BI_MinSeparation, 1, wxALL|wxEXPAND, 2);
	TextCtrl_BI_MinSeparation = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL_BI_MinSeparation, _("100"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_BI_MinSeparation"));
	FlexGridSizer16->Add(TextCtrl_BI_MinSeparation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText10 = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT2, _("Contrast"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer16->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_BI_Contrast = new wxSlider(Panel_BulbIdentify, ID_SLIDER_BI_Contrast, 0, -255, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BI_Contrast"));
	FlexGridSizer16->Add(Slider_BI_Contrast, 1, wxALL|wxEXPAND, 2);
	TextCtrl_BI_Contrast = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL_BI_Contrast, _("0"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_BI_Contrast"));
	FlexGridSizer16->Add(TextCtrl_BI_Contrast, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText16 = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT7, _("Model Scale"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer16->Add(StaticText16, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_BI_MinScale = new wxSlider(Panel_BulbIdentify, ID_SLIDER_BI_MinScale, 1, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BI_MinScale"));
	FlexGridSizer16->Add(Slider_BI_MinScale, 1, wxALL|wxEXPAND, 2);
	TextCtrl_BI_MinScale = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL_BI_MinScale, _("1"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_BI_MinScale"));
	FlexGridSizer16->Add(TextCtrl_BI_MinScale, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer15->Add(FlexGridSizer16, 1, wxALL|wxEXPAND, 2);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	CheckBox_BI_IsSteady = new wxCheckBox(Panel_BulbIdentify, ID_CHECKBOX_BI_IsSteady, _("Video is steady"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_BI_IsSteady"));
	CheckBox_BI_IsSteady->SetValue(true);
	BoxSizer1->Add(CheckBox_BI_IsSteady, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_BI_ManualUpdate = new wxCheckBox(Panel_BulbIdentify, ID_CHECKBOX_BI_ManualUpdate, _("Manual Update"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_BI_ManualUpdate"));
	CheckBox_BI_ManualUpdate->SetValue(true);
	BoxSizer1->Add(CheckBox_BI_ManualUpdate, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer30 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer30->AddGrowableCol(1);
	StaticText18 = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT12, _("Blank Frames Limit"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
	FlexGridSizer30->Add(StaticText18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_MissingBulbLimit = new wxSpinCtrl(Panel_BulbIdentify, ID_SPINCTRL1, _T("30"), wxDefaultPosition, wxDefaultSize, 0, 0, 300, 30, _T("ID_SPINCTRL1"));
	SpinCtrl_MissingBulbLimit->SetValue(_T("30"));
	FlexGridSizer30->Add(SpinCtrl_MissingBulbLimit, 1, wxALL|wxEXPAND, 5);
	BoxSizer1->Add(FlexGridSizer30, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer15->Add(BoxSizer1, 1, wxALL|wxEXPAND, 2);
	Gauge_Progress = new wxGauge(Panel_BulbIdentify, ID_GAUGE1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_GAUGE1"));
	FlexGridSizer15->Add(Gauge_Progress, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer27 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_BI_Update = new wxButton(Panel_BulbIdentify, ID_BUTTON_BI_Update, _("Update"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_BI_Update"));
	FlexGridSizer27->Add(Button_BI_Update, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_CB_RestoreDefault = new wxButton(Panel_BulbIdentify, ID_BUTTON_CB_RestoreDefault, _("Restore Default"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CB_RestoreDefault"));
	FlexGridSizer27->Add(Button_CB_RestoreDefault, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer15->Add(FlexGridSizer27, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_BI_Status = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL_BI_Status, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_BI_Status"));
	FlexGridSizer15->Add(TextCtrl_BI_Status, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer17 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_BI_Back = new wxButton(Panel_BulbIdentify, ID_BUTTON_BI_Back, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_BI_Back"));
	FlexGridSizer17->Add(Button_BI_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_BI_Next = new wxButton(Panel_BulbIdentify, ID_BUTTON_BI_Next, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_BI_Next"));
	FlexGridSizer17->Add(Button_BI_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer15->Add(FlexGridSizer17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_BulbIdentify->SetSizer(FlexGridSizer15);
	FlexGridSizer15->Fit(Panel_BulbIdentify);
	FlexGridSizer15->SetSizeHints(Panel_BulbIdentify);
	Panel2 = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL2, wxPoint(482,20), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(4);
	StaticText15 = new wxStaticText(Panel2, ID_STATICTEXT11, _("Click the image to place bulbs."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer4->Add(StaticText15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_MI_PriorFrame = new wxButton(Panel2, ID_BUTTON3, _("Prior Frame"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer8->Add(Button_MI_PriorFrame, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_MI_NextFrame = new wxButton(Panel2, ID_BUTTON1, _("Next Frame"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer8->Add(Button_MI_NextFrame, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer31 = new wxFlexGridSizer(0, 3, 0, 0);
	ButtonBumpBack = new wxButton(Panel2, ID_BUTTON7, _("< Bump"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	FlexGridSizer31->Add(ButtonBumpBack, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonBumpFwd = new wxButton(Panel2, ID_BUTTON8, _("Bump >"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
	FlexGridSizer31->Add(ButtonBumpFwd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer31, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer28 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText12 = new wxStaticText(Panel2, ID_STATICTEXT4, _("Current Number: "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer28->Add(StaticText12, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4->Add(FlexGridSizer28, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
	FlexGridSizer9 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_MI_UndoBulb = new wxButton(Panel2, ID_BUTTON2, _("Undo Bulb"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer9->Add(Button_MI_UndoBulb, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer13 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_MI_Back = new wxButton(Panel2, ID_BUTTON4, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer13->Add(Button_MI_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_MI_Next = new wxButton(Panel2, ID_BUTTON5, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	FlexGridSizer13->Add(Button_MI_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel2->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(Panel2);
	FlexGridSizer4->SetSizeHints(Panel2);
	Panel_CustomModel = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_CustomModel, wxPoint(259,19), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_CustomModel"));
	FlexGridSizer18 = new wxFlexGridSizer(4, 1, 0, 0);
	FlexGridSizer18->AddGrowableCol(0);
	FlexGridSizer18->AddGrowableRow(1);
	StaticText17 = new wxStaticText(Panel_CustomModel, ID_STATICTEXT9, _("This is the new custom model. Click save to create a model file that you can then import into your layout."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer18->Add(StaticText17, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer19 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer19->AddGrowableCol(0);
	FlexGridSizer19->AddGrowableRow(0);
	Grid_CM_Result = new wxGrid(Panel_CustomModel, ID_GRID_CM_Result, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID_CM_Result"));
	FlexGridSizer19->Add(Grid_CM_Result, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer18->Add(FlexGridSizer19, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer18->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer20 = new wxFlexGridSizer(0, 5, 0, 0);
	Button_Shrink = new wxButton(Panel_CustomModel, ID_BUTTON_Shrink, _("-"), wxDefaultPosition, wxSize(26,28), 0, wxDefaultValidator, _T("ID_BUTTON_Shrink"));
	FlexGridSizer20->Add(Button_Shrink, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Grow = new wxButton(Panel_CustomModel, ID_BUTTON_Grow, _("+"), wxDefaultPosition, wxSize(26,28), 0, wxDefaultValidator, _T("ID_BUTTON_Grow"));
	FlexGridSizer20->Add(Button_Grow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer20->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_CM_Back = new wxButton(Panel_CustomModel, ID_BUTTON_CM_Back, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CM_Back"));
	FlexGridSizer20->Add(Button_CM_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_CM_Save = new wxButton(Panel_CustomModel, ID_BUTTON_CM_Save, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CM_Save"));
	FlexGridSizer20->Add(Button_CM_Save, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer18->Add(FlexGridSizer20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_CustomModel->SetSizer(FlexGridSizer18);
	FlexGridSizer18->Fit(Panel_CustomModel);
	FlexGridSizer18->SetSizeHints(Panel_CustomModel);
	AuiNotebook_ProcessSettings->AddPage(Panel1, _("Model Type"));
	AuiNotebook_ProcessSettings->AddPage(Panel_ChooseVideo, _("Choose Media"));
	AuiNotebook_ProcessSettings->AddPage(Panel_StartFrame, _("Start Frame"));
	AuiNotebook_ProcessSettings->AddPage(Panel_BulbIdentify, _("Bulb Identify"));
	AuiNotebook_ProcessSettings->AddPage(Panel2, _("Manual Identify"));
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
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_PCM_Run,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_PCM_RunClick);
	Connect(ID_BUTTON_MT_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_MT_NextClick);
	Connect(ID_TEXTCTRL_GCM_Filename,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnTextCtrl_GCM_FilenameText);
	Connect(ID_BUTTON_GCM_SelectFile,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_GCM_SelectFileClick);
	Connect(ID_BUTTON_CV_Back,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_CV_BackClick);
	Connect(ID_BUTTON_CV_Manual,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_CV_ManualClick);
	Connect(ID_BUTTON_CV_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_CV_NextClick);
	Connect(ID_BUTTON_Back1Frame,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_Back1FrameClick);
	Connect(ID_BUTTON_Forward1Frame,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_Forward1FrameClick);
	Connect(ID_BUTTON_Back10Frames,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_Back10FramesClick);
	Connect(ID_BUTTON_Forward10Frames,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_Forward10FramesClick);
	Connect(ID_BUTTON_SF_Back,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_SF_BackClick);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_SF_ManualClick);
	Connect(ID_BUTTON_SF_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_SF_NextClick);
	Connect(ID_SLIDER_AdjustBlur,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_AdjustBlurCmdScrollChanged);
	Connect(ID_SLIDER_AdjustBlur,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_AdjustBlurCmdScroll);
	Connect(ID_SLIDER_BI_Sensitivity,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_SensitivityCmdScrollChanged);
	Connect(ID_SLIDER_BI_Sensitivity,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_SensitivityCmdSliderUpdated);
	Connect(ID_SLIDER_BI_MinSeparation,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_MinSeparationCmdScrollChanged);
	Connect(ID_SLIDER_BI_MinSeparation,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_MinSeparationCmdSliderUpdated);
	Connect(ID_SLIDER_BI_Contrast,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_ContrastCmdScrollChanged);
	Connect(ID_SLIDER_BI_Contrast,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_ContrastCmdSliderUpdated);
	Connect(ID_SLIDER_BI_MinScale,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_MinScaleCmdScrollChanged);
	Connect(ID_SLIDER_BI_MinScale,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_MinScaleCmdSliderUpdated);
	Connect(ID_CHECKBOX_BI_IsSteady,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnCheckBox_BI_IsSteadyClick);
	Connect(ID_CHECKBOX_BI_ManualUpdate,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnCheckBox_BI_ManualUpdateClick);
	Connect(ID_BUTTON_BI_Update,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BI_UpdateClick);
	Connect(ID_BUTTON_CB_RestoreDefault,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BI_RestoreDefaultClick);
	Connect(ID_BUTTON_BI_Back,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BI_BackClick);
	Connect(ID_BUTTON_BI_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BI_NextClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_MI_PriorFrameClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_MI_NextFrameClick);
	Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButtonBumpBackClick);
	Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButtonBumpFwdClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_MI_UndoBulbClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_MI_BackClick);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_MI_NextClick);
	Connect(ID_BUTTON_Shrink,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_ShrinkClick);
	Connect(ID_BUTTON_Grow,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_GrowClick);
	Connect(ID_BUTTON_CM_Back,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_CM_BackClick);
	Connect(ID_BUTTON_CM_Save,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_CM_SaveClick);
	Connect(ID_AUINOTEBOOK_ProcessSettings,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING,(wxObjectEventFunction)&GenerateCustomModelDialog::OnAuiNotebook_ProcessSettingsPageChanging);
	//*)

    _displaybmp = wxImage(GCM_DISPLAYIMAGEWIDTH, GCM_DISPLAYIMAGEHEIGHT, true);

    StaticBitmap_Preview = new MyGenericStaticBitmap(Panel_Generate, ID_STATICBITMAP_Preview, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER, _T("ID_STATICBITMAP_Preview"));
    StaticBitmap_Preview->SetScaleMode(wxStaticBitmapBase::ScaleMode::Scale_AspectFit);
    StaticBitmap_Preview->SetSizeHints(wxSize(800, 600), wxSize(800, 600));
    StaticBitmap_Preview->SetBitmap(_displaybmp);
    FlexGridSizer14->Insert(0, StaticBitmap_Preview, 1, wxALL | wxEXPAND, 2);
    FlexGridSizer14->Layout();
    FlexGridSizer5->Layout();

    FlexGridSizer19->Fit(Grid_CM_Result);
    FlexGridSizer19->SetSizeHints(Grid_CM_Result);
    FlexGridSizer19->Layout();

    StaticBitmap_Preview->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&GenerateCustomModelDialog::OnStaticBitmapLeftDown, 0, this);
    StaticBitmap_Preview->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&GenerateCustomModelDialog::OnStaticBitmapLeftUp, 0, this);
    StaticBitmap_Preview->Connect(wxEVT_MOTION, (wxObjectEventFunction)&GenerateCustomModelDialog::OnStaticBitmapMouseMove, 0, this);
    StaticBitmap_Preview->Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&GenerateCustomModelDialog::OnStaticBitmapMouseLeave, 0, this);
    StaticBitmap_Preview->Connect(wxEVT_ENTER_WINDOW, (wxObjectEventFunction)&GenerateCustomModelDialog::OnStaticBitmapMouseEnter, 0, this);

    _vr = nullptr;
    _draggingedge = -1;

    MTTabEntry();

    ValidateWindow();
}

GenerateCustomModelDialog::~GenerateCustomModelDialog()
{
	//(*Destroy(GenerateCustomModelDialog)
	//*)

    if (_vr != nullptr)
    {
        delete _vr;
    }
}
#pragma endregion Constructor

#pragma region Global Functions

void GenerateCustomModelDialog::OnResize(wxSizeEvent& event)
{
    event.Skip();
}

void GenerateCustomModelDialog::ValidateWindow()
{
    if (_state == VideoProcessingStates::CHOOSE_VIDEO)
    {
        wxString file = TextCtrl_GCM_Filename->GetValue();
        if (wxFile::Exists(file))
        {
            TextCtrl_GCM_Filename->SetBackgroundColour(*wxWHITE);
            Button_CV_Next->Enable();
            Button_CV_Manual->Enable();
        }
        else
        {
            TextCtrl_GCM_Filename->SetBackgroundColour(*wxRED);
            Button_CV_Next->Disable();
            Button_CV_Manual->Disable();
        }
        Button_GCM_SelectFile->Enable();
    }
    else if (_state == VideoProcessingStates::CHOOSE_MODELTYPE)
    {
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

inline void SetPixel(int x, int y, int w3, unsigned char* data, unsigned char c) {
    *(data + y*w3 + x * 3) = c;
}

wxImage GenerateCustomModelDialog::CreateImageFromFrame(AVFrame* frame)
{
    if (frame != nullptr)
    {
        wxImage img(frame->width, frame->height, (unsigned char *)frame->data[0], true);
        img.SetType(wxBitmapType::wxBITMAP_TYPE_BMP);
        return img;
    }
    else
    {
        static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
        logger_gcm.info("Video returned no frame.");
        if (_startFrame.IsOk())
        {
            wxImage img(_startFrame.GetWidth(), _startFrame.GetHeight(), true);
            return img;
        }
        else
        {
            wxImage img(800, 600, true);
            return img;
        }
    }
}

void GenerateCustomModelDialog::ShowImage(const wxImage& image)
{
    if (image.IsOk())
    {
        wxSize s = StaticBitmap_Preview->GetSize();
        _displaybmp = image.Scale(s.GetWidth() , s.GetHeight(), wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
    }
    StaticBitmap_Preview->SetBitmap(_displaybmp);
    wxYield();
}
#pragma endregion Image Functions

// ***********************************************************
//
// Prepare tab methods
//
// ***********************************************************

#pragma region Prepare

void GenerateCustomModelDialog::SetBulbs(bool nodes, int count, int startch, int node, int ms, int intensity)
{
    static log4cpp::Category &logger_pcm = log4cpp::Category::getInstance(std::string("log_prepcustommodel"));

    // node is out of range ... odd
    if (node > count)
    {
        logger_pcm.debug("SetBulbs failed. Node %d is greater than number of nodes %d", node, count);
        return;
    }

    wxTimeSpan ts = wxDateTime::UNow() - _starttime;
    long curtime = ts.GetMilliseconds().ToLong();
    _outputManager->StartFrame(curtime);

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
                        _outputManager->SetOneChannel(startch + j * 3 + i - 1, intensity);
                    }
                }
                else
                {
                    for (int i = 0; i < 3; i++)
                    {
                        _outputManager->SetOneChannel(startch + j * 3 + i - 1, 0);
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
                    _outputManager->SetOneChannel(startch + j - 1, intensity);
                }
                else
                {
                    _outputManager->SetOneChannel(startch + j - 1, 0);
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
                    _outputManager->SetOneChannel(startch + j * 3 + i - 1, intensity);
                }
            }
        }
        else
        {
            for (int j = 0; j < count; j++)
            {
                _outputManager->SetOneChannel(startch + j - 1, intensity);
            }
        }
    }

    _outputManager->EndFrame();

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

void GenerateCustomModelDialog::OnButton_PCM_RunClick(wxCommandEvent& event)
{
    DisplayInfo("Please prepare to video the model ... press ok when ready to start.", this);

    static log4cpp::Category &logger_pcm = log4cpp::Category::getInstance(std::string("log_prepcustommodel"));
    logger_pcm.info("Running lights to be videoed.");

    wxProgressDialog pd("Running light patterns", "", 100, this);

    int count = SpinCtrl_NC_Count->GetValue();
    int startch = SpinCtrl_StartChannel->GetValue();
    int intensity = Slider_Intensity->GetValue();
    bool nodes = NodesRadioButton->GetValue();

    logger_pcm.info("   Count: %d.", count);
    logger_pcm.info("   Start Channel: %d.", startch);
    logger_pcm.info("   Intensity: %d.", intensity);
    if (nodes)
    {
        logger_pcm.info("   Nodes.");
        logger_pcm.info("   Channels that will be affected %ld-%ld of %ld channels", startch, startch + (count * 3) - 1, _outputManager->GetTotalChannels());
    }
    else
    {
        logger_pcm.info("   Channels.");
        logger_pcm.info("   Channels that will be affected %ld-%ld of %ld channels", startch, startch + count - 1, _outputManager->GetTotalChannels());
    }

    _starttime = wxDateTime::UNow();

    // Remember our outputting state
    bool outputting = _outputManager->IsOutputting();

    if (_outputManager->IsOutputOpenInAnotherProcess())
    {
        DisplayWarning("Another process seems to be outputing to lights right now. This may not generate the result expected.", this);
    }

    _outputManager->StartOutput();

    int totaltime = LEADOFF + LEADON + FLAGOFF + FLAGON + FLAGOFF + count * (NODEON + NODEOFF);

    // 3.0 seconds off 0.5 seconds on ... 0.5 seconds off ... 0.5 second on ... 0.5 seconds off
    SetBulbs(nodes, count, startch, -1, LEADOFF, 0);
    UpdateProgress(pd, totaltime);
    SetBulbs(nodes, count, startch, -1, LEADON, intensity);
    UpdateProgress(pd, totaltime);
    SetBulbs(nodes, count, startch, -1, FLAGOFF, 0);
    UpdateProgress(pd, totaltime);
    SetBulbs(nodes, count, startch, -1, FLAGON, intensity);
    UpdateProgress(pd, totaltime);
    SetBulbs(nodes, count, startch, -1, FLAGOFF, 0);
    UpdateProgress(pd, totaltime);

    // then in turn each node on for 0.5 seconds ... all off for 0.2 seconds
    for (int i = 0; i < count && !wxGetKeyState(WXK_ESCAPE); i++)
    {
        //logger_pcm.debug("%d of %d", i, count);
        SetBulbs(nodes, count, startch, i, NODEON, intensity);
        UpdateProgress(pd, totaltime);
        SetBulbs(nodes, count, startch, i, NODEOFF, 0);
        UpdateProgress(pd, totaltime);
    }
    SetBulbs(nodes, count, startch, -1, 0, 0);

    pd.Update(100);

    _outputManager->AllOff();

    if (!outputting)
    {
        _outputManager->StopOutput();
    }

    pd.Update(100);
    pd.Close();
    SetFocus();

    logger_pcm.info("   Done.");

    DisplayInfo("Please stop the video.", this);
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
    if (_state == VideoProcessingStates::CHOOSE_MODELTYPE && page != PAGE_MODELTYPE)
    {
        event.Veto();
    }
    else if (_state == VideoProcessingStates::CHOOSE_VIDEO && page != PAGE_CHOOSEVIDEO)
    {
        event.Veto();
    }
    else if (_state == VideoProcessingStates::FINDING_START_FRAME && page != PAGE_STARTFRAME)
    {
        event.Veto();
    }
    else if (_state == VideoProcessingStates::IDENTIFYING_BULBS && page != PAGE_BULBIDENTIFY)
    {
        event.Veto();
    }
    else if (_state == VideoProcessingStates::IDENTIFYING_MANUAL && page != PAGE_MANUALIDENTIFY)
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
// Model Type tab methods
// ***********************************************************

#pragma region Model Type

void GenerateCustomModelDialog::MTTabEntry()
{
    _state = VideoProcessingStates::CHOOSE_MODELTYPE;
    _displaybmp = wxImage(GCM_DISPLAYIMAGEWIDTH, GCM_DISPLAYIMAGEHEIGHT, true);
    StaticBitmap_Preview->SetBitmap(_displaybmp);
    if (_vr != nullptr)
    {
        delete _vr;
        _vr = nullptr;
    }
    _draggingedge = -1;
}

void GenerateCustomModelDialog::OnButton_MT_NextClick(wxCommandEvent& event)
{
    static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
    logger_gcm.info("Generating custom model.");

    TextCtrl_GCM_Filename->SetValue(""); // clear the filename in case the type has changed
    CVTabEntry();
    SwapPage(PAGE_MODELTYPE, PAGE_CHOOSEVIDEO);
    ValidateWindow();
}

#pragma endregion Model Type

// ***********************************************************
// Choose Video tab methods
// ***********************************************************

#pragma region Choose Video

void GenerateCustomModelDialog::CVTabEntry()
{
    Button_CV_Next->Enable();
    Button_CV_Back->Enable();
    if (SLRadioButton->GetValue())
    {
        Button_CV_Manual->Show();
        Button_CV_Manual->Enable();
        Button_CV_Next->SetLabel("Automatic");
    }
    else
    {
        Button_CV_Manual->Hide();
        Button_CV_Next->SetLabel("Next");
    }
    Button_GCM_SelectFile->Enable();
    TextCtrl_GCM_Filename->Enable();
    _manual = false;

    if (SLRadioButton->GetValue())
    {
        StaticText_CM_Request->SetLabel("Select a picture of your static lights model.");
    }
    else
    {
        StaticText_CM_Request->SetLabel("Select the video you recorded of your model using the prepare tab.");
    }
    _state = VideoProcessingStates::CHOOSE_VIDEO;
    if (wxFile::Exists(std::string(TextCtrl_GCM_Filename->GetValue().c_str())))
    {
        if (SLRadioButton->GetValue())
        {
            _startFrame = wxImage(TextCtrl_GCM_Filename->GetValue());
            _clip = wxRect(0, 0, _startFrame.GetWidth()-1, _startFrame.GetHeight()-1);
            ShowImage(_startFrame);
        }
        else
        {
            VideoReader vr(std::string(TextCtrl_GCM_Filename->GetValue().c_str()), 800, 600, true);
            wxImage frm = CreateImageFromFrame(vr.GetNextFrame(0)).Copy();
            ShowImage(frm);
        }
    }
}

void GenerateCustomModelDialog::OnButton_GCM_SelectFileClick(wxCommandEvent& event)
{
    if (SLRadioButton->GetValue())
    {
        FileDialog1->SetWildcard(wxImage::GetImageExtWildcard());
    }
    else
    {
        FileDialog1->SetWildcard(VIDEOWILDCARD);
    }
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
    StaticText_StartTime->SetLabel(wxString::Format("%dms", time));
    _startFrame = CreateImageFromFrame(_vr->GetNextFrame(time)).Copy();
    _startframebrightness = CalcFrameBrightness(_startFrame);
    ShowImage(_startFrame);
    _clip = wxRect(0, 0, _startFrame.GetWidth()-1, _startFrame.GetHeight()-1);
    int darkframetime = time + FLAGON + (FLAGOFF / 2);
    wxImage df = CreateImageFromFrame(_vr->GetNextFrame(darkframetime)).Copy();
    _darkFrame = df.ConvertToGreyscale();
    static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
    logger_gcm.info("Start frame set to time %dms brightness %f.", time, _startframebrightness);
    logger_gcm.info("   Dark frame set to time %dms.", darkframetime);
}

void GenerateCustomModelDialog::OnButton_CV_NextClick(wxCommandEvent& event)
{
    Button_CV_Manual->Disable();
    Button_CV_Next->Disable();
    Button_GCM_SelectFile->Disable();
    Button_CV_Back->Disable();
    TextCtrl_GCM_Filename->Disable();

    static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
    logger_gcm.info("File: %s.", (const char *)TextCtrl_GCM_Filename->GetValue().c_str());

    if (SLRadioButton->GetValue())
    {
        CheckBox_BI_ManualUpdate->SetValue(false);
        Button_BI_Update->Hide();
        CheckBox_BI_IsSteady->Hide();
        CheckBox_BI_ManualUpdate->Hide();
        DoBulbIdentify();
        BITabEntry(true);
        SwapPage(PAGE_CHOOSEVIDEO, PAGE_BULBIDENTIFY);
    }
    else
    {
        DoStartFrameIdentify();
        SFTabEntry();
        SwapPage(PAGE_CHOOSEVIDEO, PAGE_STARTFRAME);
    }
    ValidateWindow();
}

void GenerateCustomModelDialog::OnButton_CV_BackClick(wxCommandEvent& event)
{
    MTTabEntry();
    SwapPage(PAGE_CHOOSEVIDEO, PAGE_MODELTYPE);
    ValidateWindow();
}

#pragma endregion Choose Video

// ***********************************************************
// Start Frame tab methods
// ***********************************************************

#pragma region Start Frame

void GenerateCustomModelDialog::DoStartFrameIdentify()
{
    if (_vr != nullptr)
    {
        delete _vr;
        _vr = nullptr;
    }

    _vr = new VideoReader(std::string(TextCtrl_GCM_Filename->GetValue().c_str()), 800, 600, true);

    if (_vr == nullptr)
    {
        DisplayError("Unable to process video.", this);
        ValidateWindow();
        return;
    }

    SetCursor(wxCURSOR_WAIT);

    FindStartFrame(_vr);

    SetCursor(wxCURSOR_ARROW);
}

void GenerateCustomModelDialog::SFTabEntry()
{
    _state = VideoProcessingStates::FINDING_START_FRAME;
    _manual = false;
    ShowImage(_startFrame);
    Button_Back10Frames->Enable();
    Button_Back1Frame->Enable();
    Button_Forward10Frames->Enable();
    Button_Forward1Frame->Enable();
    Button_SF_Next->Enable();
    Button_SF_Manual->Enable();
    Button_SF_Back->Enable();
    ValidateWindow();
}

#define ALLOWABLEDIFFERENCE 0.2
// A frame looks like a valid start frame if another frame LEADON + FLAGOFF MS in the future is about as bright
bool GenerateCustomModelDialog::LooksLikeStartFrame(int candidateframe)
{
    static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
    wxImage image = CreateImageFromFrame(_vr->GetNextFrame(candidateframe)).Copy();
    float fimage = CalcFrameBrightness(image);

    if (fimage < 1.25 * _overallaveragebrightness)
    {
        logger_gcm.info("       Frame %d (%f) NOT large enough over average brightness %f to be considered start frame.", candidateframe, fimage, _overallaveragebrightness);
        return false;
    }

    int testframe = candidateframe + LEADON + FLAGOFF;
    wxImage nextimage = CreateImageFromFrame(_vr->GetNextFrame(testframe)).Copy();
    float fnextimage = CalcFrameBrightness(nextimage);

    // within +/-10% close enough
    if (fnextimage > fimage * (1.0 - ALLOWABLEDIFFERENCE) && fnextimage < fimage * (1.0 + ALLOWABLEDIFFERENCE))
    {
        logger_gcm.info("       Second Flash frame %d (%f) and %d (%f) close enough to look like start frame.", candidateframe, fimage, testframe, fnextimage);
        return true;
    }

    logger_gcm.info("       Frame %d (%f) and %d (%f) NOT close enough to look like start frame.", candidateframe, fimage, testframe, fnextimage);
    return false;
}

float GenerateCustomModelDialog::CalcFrameBrightness(const wxImage& image)
{
    wxImage grey = image.ConvertToGreyscale();
    int w = image.GetWidth();
    int h = image.GetHeight();
    unsigned char * data = grey.GetData();
    int64_t total = 0;
    for (int i = 0; i < w * h * 3; i = i + 3)
    {
        total += *(data + i);
    }

    return (float)((double)total /
        ((double)w * (double)h) / 255.0);
}

// returns the MS of the best start frame - 0.1 MS into what looks like a bright section of the video that lasts about LEADON seconds
#define EXTRABRIGHTTHRESHOLD 0.2
int GenerateCustomModelDialog::FindStartFrame(VideoReader* vr)
{
    static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
    std::list<float> framebrightness;

    StaticBitmap_Preview->SetEraseBackground(false);

    // skip over any leading blankness
    int start = 0;
    wxImage img = CreateImageFromFrame(vr->GetNextFrame(start)).Copy();
    while (CalcFrameBrightness(img) == 0.0)
    {
        start += FRAMEMS;
        img = CreateImageFromFrame(vr->GetNextFrame(start)).Copy();
    }
    logger_gcm.info("Skipped first %dms due to black lead in.", start);
    logger_gcm.info("Finding start frame in first %dms.", (STARTSCANSECS * 1000) - start);

    // scan first STARTSCANSECS seconds of video build a list of average frame brightnesses
    _overallaveragebrightness = 0.0;
    int samples = 0;
    for (int ms = start; ms < STARTSCANSECS * 1000 && !wxGetKeyState(WXK_ESCAPE); ms+=FRAMEMS)
    {
        wxImage img1 = CreateImageFromFrame(vr->GetNextFrame(ms)).Copy();
        ShowImage(img1);
        float b = CalcFrameBrightness(img1);
        _overallaveragebrightness += b;
        samples++;
        framebrightness.push_back(b);
        logger_gcm.info("   Frame %d brightness %f.", ms, b);
    }
    _overallaveragebrightness /= samples;

    StaticBitmap_Preview->SetEraseBackground(true);

    // find the maximum number of frames in the video that it is about a set of brightness thresholds
    _overallmaxbrightness = 0.0;
    std::map<int, int> levelmaxlen;
    std::map<int, int> levelmaxstart;
    float level = 0.05f;
    for (size_t i = 0; i < 19; i++)
    {
        int maxrunlength = 0;
        int currunlength = 0;
        int maxrunstart = 0;
        int currunstart = start / FRAMEMS;
        float maxrunbrightness = 0.0;
        float curmaxbrightness = 0.0;

        auto it = framebrightness.begin();
        for (size_t j = 0; j < framebrightness.size(); j++)
        {
            if (*it > level)
            {
                if (currunlength == 0)
                {
                    currunstart = j;
                }
                currunlength++;
                if (*it > curmaxbrightness)
                {
                    curmaxbrightness = *it;
                }
            }
            else
            {
                // take this run if it is closer to the right length
                //if (currunlength > maxrunlength || curmaxbrightness > maxrunbrightness * (1.0 + EXTRABRIGHTTHRESHOLD))
                if (abs(LEADON/FRAMEMS - currunlength) < abs(LEADON/FRAMEMS - maxrunlength))
                {
                    maxrunlength = currunlength;
                    maxrunstart = currunstart;
                    maxrunbrightness = curmaxbrightness;
                    if (maxrunbrightness > _overallmaxbrightness)
                    {
                        _overallmaxbrightness = maxrunbrightness;
                    }
                }
                currunlength = 0;
                curmaxbrightness = 0.0;
            }
            ++it;
        }
        // take this run if it is closer to the right length
        //if (currunlength > maxrunlength || curmaxbrightness > maxrunbrightness * (1.0 + EXTRABRIGHTTHRESHOLD))
        if (abs(LEADON / FRAMEMS - currunlength) < abs(LEADON / FRAMEMS - maxrunlength))
        {
            maxrunlength = currunlength;
            maxrunstart = currunstart;
        }
        levelmaxlen[(int)(level*20.0)] = maxrunlength;
        levelmaxstart[(int)(level*20.0)] = maxrunstart;
        logger_gcm.info("   For level %f maxrunstarts at %dms and goes for %dms with max brightness %f.", level, maxrunstart * FRAMEMS, maxrunlength * FRAMEMS, maxrunbrightness);
        level += 0.05f;
    }

    // look for thresholds that are close to LEADON long
    std::map<int, bool> suitable;
    for (int l = 1; l < 20; l++)
    {
        if (levelmaxlen[l] > LEADON / FRAMEMS - 1 && levelmaxlen[l] < LEADON / FRAMEMS + 1)
        {
            logger_gcm.info("   Level %f looks suitable from a length perspective.", (float)l/20.0);
            if (LooksLikeStartFrame(levelmaxstart[l] * FRAMEMS))
            {
                logger_gcm.info("       And looking forward the second flash also seems to be there.");
                suitable[l] = true;
            }
            else
            {
                // check just one more frame
                if (LooksLikeStartFrame((levelmaxstart[l] + 1) * FRAMEMS))
                {
                    logger_gcm.info("       And looking forward the second flash also seems to be there ... but only once I looked forward one frame.");
                    suitable[l] = true;
                }
                else
                {
                    suitable[l] = false;
                }
            }
        }
        else
        {
            suitable[l] = false;
        }
    }

    // choose the best threshold to use
    int first = -1;
    int last = -2;
    int curfirst = -1;
    int curlast = -2;
    for (int l = 1; l < 20; l++)
    {
        if (suitable[l])
        {
            curlast = l;
            if (curfirst == -1)
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
            curlast = -2;
            curfirst = -1;
        }
    }
    if (curlast - curfirst > last - first)
    {
        last = curlast;
        first = curfirst;
    }

    int bestlevel;
    if (first == -1)
    {
        logger_gcm.info("    No great match found.");
        bestlevel = 7;
        while (bestlevel > 1 && levelmaxstart[bestlevel] == 0)
        {
            bestlevel--;
        }
    }
    else
    {
        bestlevel = ((((float)last + (float)first + 1.0) * 20.0) / 2.0) / 20;
        logger_gcm.info("    Level chosen: halfway between %f and %f ... %f.", (float)first/20.0, (float)last/20.0, (float)bestlevel / 20.0);
    }

    // pick a point 0.1 secs into the high period as our start frame
    int candidateframe = levelmaxstart[bestlevel] * FRAMEMS + start;
    logger_gcm.info("    Selected start frame %d.", candidateframe);
    candidateframe += DELAYMSUNTILSAMPLE;
    logger_gcm.info("    After adding delay Selected start frame %d.", candidateframe);

    // check the second all on event is there ... if not move up to 10 frames forward looking for it
    bool found = false;
    for (int i = 0; i < 10; i++)
    {
        if (LooksLikeStartFrame(candidateframe))
        {
            found = true;
            break;
        }
        else
        {
            candidateframe+=FRAMEMS;
        }
    }

    // if no better one found then go back to our original guess
    if (!found)
    {
        candidateframe -= FRAMEMS * 10;
    }

    logger_gcm.info("    After scanning forward the best start frame is %d.", candidateframe);

    SetStartFrame(candidateframe);
    ValidateStartFrame();

    return candidateframe;
}

void GenerateCustomModelDialog::ValidateStartFrame()
{
    static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
    if (LooksLikeStartFrame(_startframetime))
    {
        logger_gcm.info("Start frame look ok.");
        StaticText_StartFrameOk->SetLabel("Looks ok.");
        StaticText_StartFrameOk->SetForegroundColour(*wxGREEN);
    }
    else
    {
        logger_gcm.info("Start frame does NOT look ok.");
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

    static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
    logger_gcm.info("Start frame moved manually to %d.", _startframetime);
}

void GenerateCustomModelDialog::OnButton_Back1FrameClick(wxCommandEvent& event)
{
    if (!_busy)
    {
        _busy = true;
        SetCursor(wxCURSOR_WAIT);
        MoveStartFrame(-1);
        SetStartFrame(_startframetime);
        ValidateStartFrame();
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
        SetStartFrame(_startframetime);
        ValidateStartFrame();
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
        SetStartFrame(_startframetime);
        ValidateStartFrame();
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
        SetStartFrame(_startframetime);
        ValidateStartFrame();
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
    Button_SF_Manual->Disable();
    Button_SF_Back->Disable();

    CheckBox_BI_ManualUpdate->SetValue(true);
    Button_BI_Update->Show();
    CheckBox_BI_IsSteady->Show();
    CheckBox_BI_ManualUpdate->Show();
    _biFrame = _startFrame.Copy();

    BITabEntry(true);
    SwapPage(PAGE_STARTFRAME, PAGE_BULBIDENTIFY);
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
// Bulb Identify tab methods
// ***********************************************************

#pragma region Bulb Identify

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

void GenerateCustomModelDialog::SubtractImage(wxImage& from, wxImage& tosubtract)
{
    int b = from.GetWidth() * 3 * from.GetHeight();
    unsigned char * datafrom = from.GetData();
    unsigned char * datatosubtract = tosubtract.GetData();
    for (int i = 0; i < b; i++)
    {
        if (*(datatosubtract + i) > *(datafrom + i))
        {
            *(datafrom + i) = 0;
        }
        else
        {
            *(datafrom + i) = *(datafrom + i) - *(datatosubtract + i);
        }
    }
}

int GenerateCustomModelDialog::CountWhite(wxImage& image)
{
    int res = 0;
    int b = image.GetWidth() * 3 * image.GetHeight();
    unsigned char * data = image.GetData();
    for (int i = 0; i < b; i+=3)
    {
        if (*(data + i) == 255)
        {
            res++;
        }
    }

    return res;
}

void GenerateCustomModelDialog::ApplyContrast(wxImage& grey, int contrast)
{
    // Dont need to do anything if zero
    if (contrast == 0)
    {
        return;
    }

    float factor = (259.0 * ((float)contrast + 255.0)) / (255.0 * (259.0 - (float)contrast));

    int w = grey.GetWidth();
    int h = grey.GetHeight();
    int w3 = w * 3;
    unsigned char* data = grey.GetData();
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            SetPixel(x, y, w3, data, (unsigned char)(factor * (GetPixel(x,y,w3,data) - 128) + 128));
        }
    }
}

void GenerateCustomModelDialog::DoBulbIdentify()
{
    static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));

    if (!_busy)
    {
        _busy = true;

        Slider_AdjustBlur->Disable();
        Slider_BI_Sensitivity->Disable();
        Slider_BI_MinSeparation->Disable();
        Slider_BI_MinScale->Disable();
        Slider_BI_Contrast->Disable();
        CheckBox_BI_ManualUpdate->Disable();
        CheckBox_BI_IsSteady->Disable();
        Button_BI_Update->Disable();
        Button_CB_RestoreDefault->Disable();
        Button_BI_Next->Disable();
        Button_BI_Back->Disable();
        Gauge_Progress->SetValue(0);
        Gauge_Progress->Show();
        Panel_BulbIdentify->Layout();
        SetCursor(wxCURSOR_WAIT);
        StaticBitmap_Preview->SetEraseBackground(false);

        wxYield(); // let them update

        logger_gcm.info("Executing bulb identify.");
        logger_gcm.info("   Image Size: %dx%d.", _startFrame.GetWidth(), _startFrame.GetHeight());
        logger_gcm.info("   Blur: %d.", Slider_AdjustBlur->GetValue());
        logger_gcm.info("   Sensitivity: %d.", Slider_BI_Sensitivity->GetValue());
        logger_gcm.info("   Contrast: %d.", Slider_BI_Contrast->GetValue());
        logger_gcm.info("   Minimum Separation: %d.", Slider_BI_MinSeparation->GetValue());
        logger_gcm.info("   Minimum Scale: %d.", Slider_BI_MinScale->GetValue());
        logger_gcm.info("   Clip Rectangle: (%d,%d)-(%d,%d).", _clip.GetLeft(), _clip.GetTop(), _clip.GetRight(), _clip.GetBottom());
        if (CheckBox_BI_IsSteady->GetValue())
        {
            logger_gcm.info("   Is Steady: TRUE.");
        }
        else
        {
            logger_gcm.info("   Is Steady: FALSE.");
        }
        _warned = false;
        _lights.clear();
        if (SLRadioButton->GetValue())
        {
            _startFrame.LoadFile(TextCtrl_GCM_Filename->GetValue());
            wxImage bwFrame;
            wxImage grey = _startFrame.ConvertToGreyscale();
            ApplyContrast(grey, Slider_BI_Contrast->GetValue());
            wxImage imgblur = grey.Blur(Slider_AdjustBlur->GetValue());
            bwFrame = imgblur.Copy();
            ApplyThreshold(bwFrame, Slider_BI_Sensitivity->GetValue());
            FindLights(bwFrame.Copy(), 1, grey, _startFrame.Copy());
            _biFrame = CreateDetectMask(bwFrame, true, _clip);
        }
        else
        {
            // handle videos here
            int zerotime = _startframetime + LEADON + FLAGOFF + FLAGON + (0.9 * (float)FLAGOFF);
            int currentTime = zerotime;
            int n = 1;
            wxImage frame;

            int sincefound = 0;
            while (currentTime < _vr->GetLengthMS() && !_warned && sincefound < SpinCtrl_MissingBulbLimit->GetValue() && !wxGetKeyState(WXK_ESCAPE))
            {
                Gauge_Progress->SetValue((currentTime * 100) / _vr->GetLengthMS());
                logger_gcm.info("   Looking for frame at %d for node %d.", currentTime, n);
                wxImage bwFrame;
                wxImage grey;
                int advance = 0;
                bool toobright = false;

                while ((!bwFrame.IsOk() || CountWhite(bwFrame) < 50 || toobright) &&
                       currentTime < _vr->GetLengthMS() &&
                       sincefound < SpinCtrl_MissingBulbLimit->GetValue())
                {
                    toobright = false;
                    sincefound++;
                    if (bwFrame.IsOk())
                    {
                        advance++;
                        if (advance > 4)
                        {
                            logger_gcm.info("   No bulb found so assuming bulb %d is not visible.", n);
                            advance = 0;
                            n++;
                            currentTime = zerotime + (n - 1)*(NODEON + NODEOFF) - FRAMEMS;
                        }
                        else
                        {
                            currentTime += FRAMEMS;
                            logger_gcm.info("   No frame found so now trying %d.", currentTime);
                        }
                    }

                    frame = CreateImageFromFrame(_vr->GetNextFrame(currentTime)).Copy();
                    float brightness = CalcFrameBrightness(frame);
                    logger_gcm.info("       Frame %d brightness %f.", currentTime, brightness);

                    if (brightness > 0.9 * _startframebrightness)
                    {
                        logger_gcm.info("       Frame too bright so we will skip it.");
                        toobright = true;
                    }
                    else
                    {
                        grey = frame.ConvertToGreyscale();
                        if (CheckBox_BI_IsSteady->GetValue())
                        {
                            SubtractImage(grey, _darkFrame);
                        }
                        ApplyContrast(grey, Slider_BI_Contrast->GetValue());
                        wxImage imgblur = grey.Blur(Slider_AdjustBlur->GetValue());
                        bwFrame = imgblur;
                        ApplyThreshold(bwFrame, Slider_BI_Sensitivity->GetValue());
                    }
                }

                if (sincefound < SpinCtrl_MissingBulbLimit->GetValue())
                {
                    int delta = currentTime - (zerotime + (n - 1)*(NODEON + NODEOFF));

                    sincefound = 0;
                    FindLights(bwFrame, n++, grey, frame.Copy());

                    if (n == 1)
                    {
                        zerotime = currentTime - FRAMEMS;
                    }
                    else
                    {
                        logger_gcm.info("   Video drift %d.", delta);
                        // This helps correct for video drift
                        if (abs(delta) > 2 * FRAMEMS)
                        {
                            if (delta < 0)
                            {
                                logger_gcm.info("       *** Adjusting by %d.", -1 * FRAMEMS);
                                zerotime -= FRAMEMS;
                            }
                            else
                            {
                                logger_gcm.info("       *** Adjusting by %d.", delta);
                                zerotime += delta;
                            }
                        }
                    }

                    currentTime = zerotime + (n - 1)*(NODEON + NODEOFF) - FRAMEMS;
                }
            }

            if (sincefound >= SpinCtrl_MissingBulbLimit->GetValue())
            {
                DisplayError("Too many frames with no lights spotted. Aborting scan.", this);
            }

            _biFrame = CreateDetectMask(_startFrame, true, _clip);
        }
        ShowImage(_biFrame);
        StaticBitmap_Preview->SetEraseBackground(true);
        Slider_AdjustBlur->Enable();
        Slider_BI_Sensitivity->Enable();
        Slider_BI_MinSeparation->Enable();
        Slider_BI_MinScale->Enable();
        Slider_BI_Contrast->Enable();
        CheckBox_BI_ManualUpdate->Enable();
        CheckBox_BI_IsSteady->Enable();
        Button_BI_Update->Enable();
        Button_CB_RestoreDefault->Enable();
        Button_BI_Next->Enable();
        Button_BI_Back->Enable();
        Gauge_Progress->Hide();
        Panel_BulbIdentify->Layout();
        SetCursor(wxCURSOR_ARROW);
        logger_gcm.info("Result: %s.", (const char *)TextCtrl_BI_Status->GetValue().c_str());
        _busy = false;
    }
}

void GenerateCustomModelDialog::BITabEntry(bool setdefault)
{
    _state = VideoProcessingStates::IDENTIFYING_BULBS;
    if (setdefault)
    {
        TextCtrl_BI_Status->SetValue("");
        SetBIDefault();
    }
    _biFrame = CreateDetectMask(_biFrame, true, _clip);
    ShowImage(_biFrame);
    StaticText_BI->SetLabel("The red circles on the image show the bulbs we have identified. Adjust the sensitivity if there are bulbs missing or phantom bulbs identified.\n\nClick next when you are happy that all bulbs have been detected.");
    Slider_BI_Sensitivity->Enable();
    Slider_AdjustBlur->Enable();
    Slider_BI_MinSeparation->Enable();
    Slider_BI_MinScale->Enable();
    Slider_BI_Contrast->Enable();
    Button_CB_RestoreDefault->Enable();
    Button_BI_Next->Enable();
    Button_BI_Back->Enable();
}

int GenerateCustomModelDialog::GetMaxNum()
{
    int max = -1;
    for (auto it = _lights.begin(); it != _lights.end(); ++it)
    {
        if (!it->isSupressed() && it->GetNum() > max)
        {
            max = it->GetNum();
        }
    }

    return max;
}

int GenerateCustomModelDialog::GetBulbCount()
{
    int count = 0;
    for (auto it = _lights.begin(); it != _lights.end(); ++it)
    {
        if (!it->isSupressed())
        {
            count++;
        }
    }

    return count;
}

// Assumes nodes are in order
wxString GenerateCustomModelDialog::GetMissingNodes()
{
    wxString res;
    int current = 0;
    for (auto it = _lights.begin(); it != _lights.end(); ++it)
    {
        if (!it->isSupressed())
        {
            if (it->GetNum() == current)
            {
                // this is ok ... a second bulb for this node
            }
            else if (it->GetNum() == current + 1)
            {
                // this is ok ... we have moved on to next node
                current++;
            }
            else
            {
                // this is a problem
                for (int i = current + 1; i < it->GetNum(); i++)
                {
                    if (res != "")
                    {
                        res += ", ";
                    }
                    res += wxString::Format(wxT("%i"), i);
                }
                current = it->GetNum();
            }
        }
    }

    return res;
}

// Assumes nodes are in order
wxString GenerateCustomModelDialog::GetMultiBulbNodes()
{
    wxString res;
    int current = -1;
    int last = -2;
    for (auto it = _lights.begin(); it != _lights.end(); ++it)
    {
        if (!it->isSupressed())
        {
            if (it->GetNum() == current && it->GetNum() != last)
            {
                // this is ok ... a second bulb for this node
                if (res != "")
                {
                    res += ", ";
                }
                res += wxString::Format(wxT("%i"), it->GetNum());
                last = it->GetNum();
            }
            else if (it->GetNum() > current)
            {
                // this is ok ... we have moved on to next node
                current = it->GetNum();
            }
        }
    }

    return res;
}

wxString GenerateCustomModelDialog::GenerateStats(int minseparation)
{
    wxString res;

    int n = GetMaxNum();
    if (n < 1)
    {
        res += "Nodes: None\n";
    }
    else
    {
        res += wxString::Format("Nodes: %d\n", n);
    }
    res += wxString::Format("Bulbs: %d\n", GetBulbCount());
    if (minseparation == 9999999)
    {
        res += "Minimum Bulb Separation: N/A\n";
    }
    else
    {
        res += wxString::Format("Minimum Bulb Separation: %d\n", minseparation);
    }
    wxString mn = GetMissingNodes();
    if (mn == "")
    {
        res += "Missing Nodes: N/A\n";
    }
    else
    {
        res += wxString::Format("Missing Nodes: %s\n", mn);
    }
    wxString mbn = GetMultiBulbNodes();
    if (mbn == "")
    {
        res += "Nodes with more than 1 bulb: N/A\n";
    }
    else
    {
        res += wxString::Format("Nodes with more than 1 bulb: %s\n", mbn);
    }
    wxSize p = CalcSize((float)Slider_BI_MinScale->GetValue() / 100.0);
    res += wxString::Format("Model size: %dx%d\n", p.x, p.y);

    return res;
}

wxImage GenerateCustomModelDialog::CreateDetectMask(wxImage ref, bool includeimage, wxRect clip)
{
    for (auto it = _lights.begin(); it != _lights.end(); ++it)
    {
        it->Reset();
    }

    RemoveClippedLights(_lights, _clip);
    int min = ApplyMinimumSeparation(_lights, Slider_BI_MinSeparation->GetValue());

    TextCtrl_BI_Status->SetValue(GenerateStats(min));

    wxBitmap bmp(ref.GetWidth(), ref.GetHeight());
    wxMemoryDC dc(bmp);

    if (includeimage)
    {
        dc.DrawBitmap(ref, wxPoint(0, 0), false);
    }

    wxSize displaysize = StaticBitmap_Preview->GetSize();
    float factor = std::max((float)_startFrame.GetWidth() / (float)displaysize.GetWidth(),
        (float)_startFrame.GetHeight() / (float)displaysize.GetHeight());

    // draw grey out clipped area
    dc.SetPen(*wxTRANSPARENT_PEN);
    wxBrush shade(*wxLIGHT_GREY, wxBRUSHSTYLE_BDIAGONAL_HATCH);
    dc.SetBrush(shade);
    if (_clip.GetLeft() > 0)
    {
        dc.DrawRectangle(wxRect(0, 0, _clip.GetLeft(), _startFrame.GetHeight()));
    }
    if (_clip.GetRight() < _startFrame.GetWidth())
    {
        dc.DrawRectangle(wxRect(_clip.GetRight(), 0, _startFrame.GetWidth() - _clip.GetRight(), _startFrame.GetHeight()));
    }
    if (_clip.GetTop() > 0)
    {
        dc.DrawRectangle(wxRect(0, 0, _startFrame.GetWidth(), _clip.GetTop()));
    }
    if (_clip.GetBottom() < _startFrame.GetHeight())
    {
        dc.DrawRectangle(wxRect(0, _clip.GetBottom(), _startFrame.GetWidth(), _startFrame.GetHeight() - _clip.GetBottom()));
    }

    // Draw clip rectangle
    int penw = 2 * factor;
    wxPen p2(*wxGREEN, penw, wxPENSTYLE_LONG_DASH);
    dc.SetPen(p2);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(_clip);

    // draw blue first
    for (auto c = _lights.begin(); c != _lights.end(); ++c)
    {
        if (c->isSupressedButDraw())
        {
            c->Draw(dc, factor);
        }
    }

    // now red so they are easy to see
    for (auto c = _lights.begin(); c != _lights.end(); ++c)
    {
        if (!c->isSupressed())
        {
            c->Draw(dc, factor);
        }
    }

    return bmp.ConvertToImage();
}

void GenerateCustomModelDialog::WalkPixels(int x, int y, int w, int h, int w3, unsigned char *data, int& totalX, int& totalY, int& pixelCount)
{
    std::list<wxPoint> pixels;
    pixels.push_back(wxPoint(x, y));

    while(pixels.size() != 0 && pixels.size() < 1000)
    {
        std::list<wxPoint>::iterator it = pixels.begin();

        if (GetPixel(it->x, it->y, w3, data) > 0)
        {
            SetPixel(it->x, it->y, w3, data, 0);
            pixelCount++;
            totalX += it->x;
            totalY += it->y;

            if (it->x > 0 && it->y > 0 && GetPixel(it->x - 1, it->y - 1, w3, data) > 0) {
                pixels.push_back(wxPoint(it->x - 1, it->y - 1));
            }
            if (it->x > 0 && it->y < h-1 && GetPixel(it->x - 1, it->y + 1, w3, data) > 0) {
                pixels.push_back(wxPoint(it->x - 1, it->y + 1));
            }
            if (it->x < w-1 && it->y > 0 && GetPixel(it->x + 1, it->y - 1, w3, data) > 0) {
                pixels.push_back(wxPoint(it->x + 1, it->y - 1));
            }
            if (it->x < w-1 && it->y < h-1 && GetPixel(it->x + 1, it->y + 1, w3, data) > 0) {
                pixels.push_back(wxPoint(it->x + 1, it->y + 1));
            }
            if (it->y > 0 && GetPixel(it->x, it->y - 1, w3, data) > 0) {
                pixels.push_back(wxPoint(it->x, it->y - 1));
            }
            if (it->y < h-1 && GetPixel(it->x, it->y + 1, w3, data) > 0) {
                pixels.push_back(wxPoint(it->x, it->y + 1));
            }
            if (it->x > 0 && GetPixel(it->x - 1, it->y, w3, data) > 0) {
                pixels.push_back(wxPoint(it->x - 1, it->y));
            }
            if (it->x < w-1 && GetPixel(it->x + 1, it->y, w3, data) > 0) {
                pixels.push_back(wxPoint(it->x + 1, it->y));
            }
        }
        pixels.pop_front();
    }

    if (pixels.size() != 0)
    {
        if (!_warned)
        {
            _warned = true;
            DisplayError("Too many pixels are looking like bulbs ... this could take forever ... you need to change your settings ... maybe increase sensitivity.", this);
        }
    }
}

GCMBulb GenerateCustomModelDialog::FindCenter(int x, int y, int w, int h, int w3, unsigned char *data, int num, const wxImage& grey)
{
    int totalX = 0;
    int totalY = 0;
    int pixelCount = 0;
    WalkPixels(x, y, w, h, w3, data, totalX, totalY, pixelCount);

    return GCMBulb(wxPoint(totalX / pixelCount, totalY / pixelCount), num, GetPixel(totalX / pixelCount, totalY / pixelCount, w3, grey.GetData()));
}

void GenerateCustomModelDialog::FindLights(const wxImage& bwimage, int num, const wxImage& greyimage, const wxImage& frame)
{
    static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));

    wxImage temp = bwimage;
    int w = temp.GetWidth();
    int w3 = w * 3;
    int h = temp.GetHeight();
    unsigned char * data = temp.GetData();
    std::list<GCMBulb> found;

    for (int y = 0; y < temp.GetHeight() && !_warned; y++)
    {
        for (int x = 0; x < temp.GetWidth() && !_warned; x++)
        {
            if (GetPixel(x, y, w3, data) > 0)
            {
                found.push_back(FindCenter(x, y, w, h, w3, data, num, greyimage));
            }
        }
    }

    // only add them if we didnt warn the user
    if (!_warned)
    {
        logger_gcm.info("    Node %d found %d bulbs.", num, found.size());
        _lights.splice(_lights.end(), found);
    }
    else
    {
        logger_gcm.info("    Node %d found %d bulbs ... but not added.", num, found.size());
    }

    _biFrame = CreateDetectMask(frame, true, _clip);
    ShowImage(_biFrame);
}

void GenerateCustomModelDialog::OnSlider_BI_SensitivityCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_BI_Sensitivity->SetValue(wxString::Format(wxT("%i"), Slider_BI_Sensitivity->GetValue()));
}

void GenerateCustomModelDialog::OnSlider_AdjustBlurCmdScroll(wxScrollEvent& event)
{
    TextCtrl_BC_Blur->SetValue(wxString::Format(wxT("%i"), Slider_AdjustBlur->GetValue()));
}

void GenerateCustomModelDialog::OnSlider_BI_MinSeparationCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_BI_MinSeparation->SetValue(wxString::Format(wxT("%i"), Slider_BI_MinSeparation->GetValue()));
}

void GenerateCustomModelDialog::SetBIDefault()
{
    Slider_BI_MinSeparation->SetValue(100);
    TextCtrl_BI_MinSeparation->SetValue("100");
    Slider_BI_MinScale->SetValue(1);
    TextCtrl_BI_MinScale->SetValue("1");
    Slider_AdjustBlur->SetValue(1);
    TextCtrl_BC_Blur->SetValue("1");
    Slider_BI_Sensitivity->SetValue(127);
    TextCtrl_BI_Sensitivity->SetValue("127");
    Slider_BI_Contrast->SetValue(0);
    TextCtrl_BI_Contrast->SetValue("0");
}

void GenerateCustomModelDialog::OnButton_BI_RestoreDefaultClick(wxCommandEvent& event)
{
    if (!_busy)
    {
        SetBIDefault();
        if (!CheckBox_BI_ManualUpdate->GetValue())
        {
            DoBulbIdentify();
        }
    }
}

void GenerateCustomModelDialog::OnSlider_AdjustBlurCmdScrollChanged(wxScrollEvent& event)
{
    if (!CheckBox_BI_ManualUpdate->GetValue())
    {
        if (!_busy)
        {
            DoBulbIdentify();
        }
    }
}

void GenerateCustomModelDialog::OnSlider_BI_SensitivityCmdScrollChanged(wxScrollEvent& event)
{
    if (!CheckBox_BI_ManualUpdate->GetValue())
    {
        if (!_busy)
        {
            DoBulbIdentify();
        }
    }
}

void GenerateCustomModelDialog::OnSlider_BI_MinSeparationCmdScrollChanged(wxScrollEvent& event)
{
    if (!CheckBox_BI_ManualUpdate->GetValue())
    {
        if (!_busy)
        {
            DoBulbIdentify();
        }
    }
}

void GenerateCustomModelDialog::OnSlider_BI_ContrastCmdScrollChanged(wxScrollEvent& event)
{
    if (!CheckBox_BI_ManualUpdate->GetValue())
    {
        if (!_busy)
        {
            DoBulbIdentify();
        }
    }
}

void GenerateCustomModelDialog::OnCheckBox_BI_IsSteadyClick(wxCommandEvent& event)
{
    if (!CheckBox_BI_ManualUpdate->GetValue())
    {
        if (!_busy)
        {
            DoBulbIdentify();
        }
    }
}

void GenerateCustomModelDialog::OnCheckBox_BI_ManualUpdateClick(wxCommandEvent& event)
{
    if (CheckBox_BI_ManualUpdate->GetValue())
    {
        Button_BI_Update->Show();
    }
    else
    {
        Button_BI_Update->Hide();
        if (!_busy)
        {
            DoBulbIdentify();
        }
    }
}

void GenerateCustomModelDialog::OnButton_BI_UpdateClick(wxCommandEvent& event)
{
    if (!_busy)
    {
        DoBulbIdentify();
    }
}

void GenerateCustomModelDialog::OnSlider_BI_ContrastCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_BI_Contrast->SetValue(wxString::Format(wxT("%i"), Slider_BI_Contrast->GetValue()));
}

void GenerateCustomModelDialog::OnButton_BI_NextClick(wxCommandEvent& event)
{
    if (!_busy)
    {
        DoGenerateCustomModel();
        CMTabEntry();
        SwapPage(PAGE_BULBIDENTIFY, PAGE_REVIEWMODEL);
    }
}

void GenerateCustomModelDialog::OnButton_BI_BackClick(wxCommandEvent& event)
{
    if (!_busy)
    {
        if (SLRadioButton->GetValue())
        {
            CVTabEntry();
            SwapPage(PAGE_BULBIDENTIFY, PAGE_CHOOSEVIDEO);
        }
        else
        {
            SFTabEntry();
            SwapPage(PAGE_BULBIDENTIFY, PAGE_STARTFRAME);
        }
    }
}

#pragma endregion Bulb Identify

// ***********************************************************
// Custom Model tab methods
// ***********************************************************

#pragma region Custom Model

void GenerateCustomModelDialog::CMTabEntry()
{
    _state = VideoProcessingStates::REVIEW_CUSTOM_MODEL;
    ShowImage(_biFrame);
}

bool GenerateCustomModelDialog::TestScale(std::list<GCMBulb>& lights, std::list<GCMBulb>::iterator it, float scale, wxPoint trim)
{
    GCMBulb b = *it;
    ++it;
    if (it != lights.end() && !it->isSupressed())
    {
        if (!TestScale(lights, it, scale, trim))
        {
            return false;
        }
        while (it != lights.end())
        {
            if (!it->isSupressed())
            {
                if (b.IsSameLocation(*it, scale, trim))
                {
                    return false;
                }
            }
            ++it;
        }
    }
    return true;
}

wxPoint GenerateCustomModelDialog::CalcTrim(std::list<GCMBulb>& lights)
{
    int x = 999999;
    int y = 999999;

    for (auto it = lights.begin(); it != lights.end(); ++it)
    {
        if (!it->isSupressed())
        {
            wxPoint loc = it->GetLocation();
            if (loc.x < x)
            {
                x = loc.x;
            }
            if (loc.y < y)
            {
                y = loc.y;
            }
        }
    }

    return wxPoint(x, y);
}

wxSize GenerateCustomModelDialog::CalcSize(float min)
{
    if (_lights.size() == 0)
    {
        return wxSize(0, 0);
    }

    _trim = CalcTrim(_lights);

    float best = 1.0f;
    float curr = 0.9f;

    while (curr >= min && TestScale(_lights, _lights.begin(), curr, _trim))
    {
        best = curr;
        curr = curr - 0.1f;
    }
    curr = best;
    float start = curr;
    curr = curr - 0.01;
    while (curr > start && curr >= min && TestScale(_lights, _lights.begin(), curr, _trim))
    {
        best = curr;
        curr = curr - 0.01;
    }
    _scale = best;

    int x = 0;
    int y = 0;

    for (auto it = _lights.begin(); it != _lights.end(); ++it)
    {
        if (!it->isSupressed())
        {
            wxPoint loc = it->GetLocation(_scale, _trim);
            if (loc.x > x)
            {
                x = loc.x;
            }
            if (loc.y > y)
            {
                y = loc.y;
            }
        }
    }

    return wxSize(x+1, y+1);
}

void GenerateCustomModelDialog::RemoveClippedLights(std::list<GCMBulb>& lights, wxRect& clip)
{
    for (auto it = lights.begin(); it != lights.end(); ++it)
    {
        if (!it->isSupressed())
        {
            int x = it->GetLocation().x;
            int y = it->GetLocation().y;
            if (x < _clip.GetLeft() || x > _clip.GetRight() ||
                y < _clip.GetTop() || y > _clip.GetBottom())
            {
                it->OutsideClip();
            }
        }
    }
}

inline int GetSeparation(int x1, int y1, int x2, int y2)
{
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}
inline bool IsWithin(int x1, int y1, int x2, int y2, int d)
{
    return d > GetSeparation(x1, y1, x2, y2);
}

// return minimum actual separation
int GenerateCustomModelDialog::ApplyMinimumSeparation(std::list<GCMBulb>& lights, int minseparation)
{
    int min = 9999999;
    for (auto it = lights.begin(); it != lights.end(); ++it)
    {
        // No point looking at this light as it is already suppressed
        if (!it->isSupressed())
        {
            for (auto it2 = it; it2 != lights.end(); ++it2)
            {
                // No point looking at this light as it is already suppressed
                if (!it2->isSupressed())
                {
                    // only do suppression if they are both tagged with the same number
                    if (it->GetNum() == it2->GetNum())
                    {
                        // and they are not the same light
                        if (it != it2)
                        {
                            // If it is within our minimum separation distance
                            if (IsWithin(it->GetLocation().x, it->GetLocation().y, it2->GetLocation().x, it2->GetLocation().y, minseparation))
                            {
                                // Suppress the dimmer of the two under the assumption the reflection is more likely to be dimmer
                                if (it->GetBrightness() >= it2->GetBrightness())
                                {
                                    it2->TooClose();
                                }
                                else
                                {
                                    it->TooClose();
                                }
                            }
                        }
                    }

                    // If both are not suppressed update the minimum separation
                    // we need minimum separation so we can work out the minimal grid that puts each node in its
                    // own cell
                    if (it != it2 && !it->isSupressed() && !it2->isSupressed())
                    {
                        int d = GetSeparation(it->GetLocation().x, it->GetLocation().y, it2->GetLocation().x, it2->GetLocation().y);
                        if (d < min)
                        {
                            min = d;
                        }
                    }
                }
            }
        }
    }

    return min;
}

// this will find the best scale to 1/100th of the imput size
void GenerateCustomModelDialog::DoGenerateCustomModel()
{
    if (_lights.size() == 0)
    {
        return;
    }

    for (auto it = _lights.begin(); it != _lights.end(); ++it)
    {
        it->Reset();
    }

    RemoveClippedLights(_lights, _clip);
    ApplyMinimumSeparation(_lights, Slider_BI_MinSeparation->GetValue());

    _size = CalcSize((float)Slider_BI_MinScale->GetValue() / 100.0);

    Grid_CM_Result->ClearGrid();
    if (Grid_CM_Result->GetNumberCols() > 0)
    {
        Grid_CM_Result->DeleteCols(0, Grid_CM_Result->GetNumberCols());
        if (Grid_CM_Result->GetNumberRows() > 0)
        {
            Grid_CM_Result->DeleteRows(0, Grid_CM_Result->GetNumberRows());
        }
        Grid_CM_Result->AppendCols(_size.x);
        Grid_CM_Result->AppendRows(_size.y);
    }
    else
    {
        Grid_CM_Result->CreateGrid(_size.y, _size.x);
    }

    for (auto it = _lights.begin(); it != _lights.end(); ++it)
    {
        if (!it->isSupressed())
        {
            wxPoint p = it->GetLocation(_scale, _trim);
            Grid_CM_Result->SetCellValue(p.y, p.x, wxString::Format(wxT("%i"), it->GetNum()));
            Grid_CM_Result->SetCellBackgroundColour(p.y, p.x, *wxGREEN);
        }
    }

    wxFont font = Grid_CM_Result->GetDefaultCellFont();
    Grid_CM_Result->SetRowMinimalAcceptableHeight(5); //don't need to read text, just see the shape
    Grid_CM_Result->SetColMinimalAcceptableWidth(5); //don't need to read text, just see the shape
    for (int c = 0; c < Grid_CM_Result->GetNumberCols(); ++c)
        Grid_CM_Result->SetColSize(c, 2 * font.GetPixelSize().y); //Grid_CM_Result->GetColSize(c) * 4/5);
    for (int r = 0; r < Grid_CM_Result->GetNumberRows(); ++r)
        Grid_CM_Result->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //Grid_CM_Result->GetRowSize(r) * 4/5);
    font = Grid_CM_Result->GetLabelFont();
    Grid_CM_Result->SetColLabelSize(int(1.5 * (float)font.GetPixelSize().y));
    FlexGridSizer19->Layout();
    Layout();
}

void GenerateCustomModelDialog::OnButton_CM_BackClick(wxCommandEvent& event)
{
    if (_manual)
    {
        MITabEntry(false);
        SwapPage(PAGE_REVIEWMODEL, PAGE_MANUALIDENTIFY);
    }
    else
    {
        BITabEntry(false);
        SwapPage(PAGE_REVIEWMODEL, PAGE_BULBIDENTIFY);
    }
}

wxString GenerateCustomModelDialog::CreateCustomModelData()
{
    wxString res = "";
    for (int y = 0; y < Grid_CM_Result->GetNumberRows(); y++)
    {
        for (int x = 0; x < Grid_CM_Result->GetNumberCols(); x++)
        {
            res += Grid_CM_Result->GetCellValue(y, x);

            if (x + 1 != Grid_CM_Result->GetNumberCols())
            {
                res += ",";
            }
        }

        if (y + 1 != Grid_CM_Result->GetNumberRows())
        {
            res += ";";
        }
    }

    return res;
}

void GenerateCustomModelDialog::OnButton_CM_SaveClick(wxCommandEvent& event)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, "NewCustomModel", wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
    logger_gcm.info("Saving to xmodel file %s.", (const char *)filename.c_str());
    if (!f.Create(filename, true) || !f.IsOpened())
    {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }
    wxString name = wxFileName(filename).GetName();
    wxString cm = CreateCustomModelData();
    wxString p1 = wxString::Format(wxT("%i"),Grid_CM_Result->GetNumberCols());
    wxString p2 = wxString::Format(wxT("%i"), Grid_CM_Result->GetNumberRows());
    wxString st;
    if (SLRadioButton->GetValue())
    {
        st = "Single Color White";
    }
    else if (SCRadioButton->GetValue())
    {
        st = "Single Color White";
    }
    else
    {
        st = "RGB Nodes";
    }
    wxString ps = "2";
    wxString t = "0";
    wxString mb = "";
    wxString a = "1";
    wxString sn = "";
    wxString nn = "";
    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<custommodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write(wxString::Format("StringType=\"%s\" ", st));
    f.Write(wxString::Format("Transparency=\"%s\" ", t));
    f.Write(wxString::Format("PixelSize=\"%s\" ", ps));
    f.Write(wxString::Format("ModelBrightness=\"%s\" ", mb));
    f.Write(wxString::Format("Antialias=\"%s\" ", a));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    f.Write("CustomModel=\"");
    f.Write(cm);
    f.Write("\" ");
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(" >\n");
    f.Write("</custommodel>");
    f.Close();
}

void GenerateCustomModelDialog::OnButton_GrowClick(wxCommandEvent& event)
{
    Grid_CM_Result->BeginBatch();
    wxFont font = Grid_CM_Result->GetLabelFont();
    font.MakeLarger();
    Grid_CM_Result->SetLabelFont(font);
    font = Grid_CM_Result->GetDefaultCellFont();
    font.MakeLarger();
    Grid_CM_Result->SetDefaultCellFont(font);
    for (int c = 0; c < Grid_CM_Result->GetNumberCols(); ++c)
        Grid_CM_Result->SetColSize(c, 2 * font.GetPixelSize().y); //Grid_CM_Result->GetColSize(c) * 5/4);
    for (int r = 0; r < Grid_CM_Result->GetNumberRows(); ++r)
        Grid_CM_Result->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //Grid_CM_Result->GetRowSize(r) * 5/4);
    Grid_CM_Result->EndBatch();
}

void GenerateCustomModelDialog::OnButton_ShrinkClick(wxCommandEvent& event)
{
    Grid_CM_Result->BeginBatch();
    wxFont font = Grid_CM_Result->GetLabelFont();
    font.MakeSmaller();
    Grid_CM_Result->SetLabelFont(font);
    font = Grid_CM_Result->GetDefaultCellFont();
    font.MakeSmaller();
    Grid_CM_Result->SetDefaultCellFont(font);
    Grid_CM_Result->SetRowMinimalAcceptableHeight(5); //don't need to read text, just see the shape
    Grid_CM_Result->SetColMinimalAcceptableWidth(5); //don't need to read text, just see the shape
    for (int c = 0; c < Grid_CM_Result->GetNumberCols(); ++c)
        Grid_CM_Result->SetColSize(c, 2 * font.GetPixelSize().y); //Grid_CM_Result->GetColSize(c) * 4/5);
    for (int r = 0; r < Grid_CM_Result->GetNumberRows(); ++r)
        Grid_CM_Result->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //Grid_CM_Result->GetRowSize(r) * 4/5);
    Grid_CM_Result->EndBatch();
}
#pragma endregion Custom Model

#pragma endregion Generate

int GenerateCustomModelDialog::GetEdge(int x, int y)
{
    wxSize displaysize = StaticBitmap_Preview->GetSize();
    float xf = (float)_startFrame.GetWidth() / (float)displaysize.GetWidth();
    float yf = (float)_startFrame.GetHeight() / (float)displaysize.GetHeight();
    int edge = -1;

    if (std::abs(xf*x - _clip.GetLeft()) < 3 * xf)
    {
        if (yf*y >= _clip.GetTop() && yf*y <= _clip.GetBottom())
        {
            edge = 3;
        }
    }
    else if (std::abs(xf*x - _clip.GetRight()) < 5 * xf)
    {
        if (yf*y >= _clip.GetTop() && yf*y <= _clip.GetBottom())
        {
            edge = 1;
        }
    }
    else if (std::abs(yf*y - _clip.GetTop()) < 5*yf)
    {
        if (xf*x >= _clip.GetLeft() && xf*x <= _clip.GetRight())
        {
            edge = 0;
        }
    }
    else if (std::abs(yf*y - _clip.GetBottom()) < 5*yf)
    {
        if (xf*x >= _clip.GetLeft() && xf*x <= _clip.GetRight())
        {
            edge = 2;
        }
    }

    return edge;
}

void GenerateCustomModelDialog::ResizeClip(int x, int y)
{
    if (_draggingedge == -1)
    {
        return;
    }
    else
    {
        wxSize displaysize = StaticBitmap_Preview->GetSize();
        int w = _startFrame.GetWidth();
        int h = _startFrame.GetHeight();
        float xf = (float)w / (float)displaysize.GetWidth() * (float)x;
        if (xf < 0)
        {
            xf = 0;
        }
        else if (xf > w-1)
        {
            xf = w-1;
        }
        float yf = (float)h / (float)displaysize.GetHeight() * (float)y;
        if (yf < 0)
        {
            yf = 0;
        }
        else if (yf > h - 1)
        {
            yf = h - 1;
        }

        if (_draggingedge == 0)
        {
            int b = _clip.GetBottom();
            _clip.SetTop(yf);
            _clip.SetBottom(b);
        }
        else if (_draggingedge == 1)
        {
            _clip.SetRight(xf);
        }
        else if (_draggingedge == 2)
        {
            _clip.SetBottom(yf);
        }
        else if (_draggingedge == 3)
        {
            int r = _clip.GetRight();
            _clip.SetLeft(xf);
            _clip.SetRight(r);
        }
    }
    StaticBitmap_Preview->SetEraseBackground(false);
    _biFrame = CreateDetectMask(_startFrame, true, _clip);
    ShowImage(_biFrame);
    StaticBitmap_Preview->SetEraseBackground(true);
}

void GenerateCustomModelDialog::OnStaticBitmapLeftDown(wxMouseEvent& event)
{
    // only doing dragging on one page
    if (_state == VideoProcessingStates::IDENTIFYING_BULBS)
    {
        _draggingedge = GetEdge(event.GetX(), event.GetY());
        if (_draggingedge >= 0)
        {
            ResizeClip(event.GetX(), event.GetY());
            if (_draggingedge % 2 == 0)
            {
                SetCursor(wxCURSOR_SIZENS);
            }
            else
            {
                SetCursor(wxCURSOR_SIZEWE);
            }
        }
    }
}

void GenerateCustomModelDialog::OnStaticBitmapLeftUp(wxMouseEvent& event)
{
    if (_state == VideoProcessingStates::IDENTIFYING_BULBS)
    {
        if (_draggingedge >= 0)
        {
            ResizeClip(event.GetX(), event.GetY());
        }
        _draggingedge = -1;
        SetCursor(wxCURSOR_ARROW);
    }
    else if (_state == VideoProcessingStates::IDENTIFYING_MANUAL)
    {
        wxSize displaysize = StaticBitmap_Preview->GetSize();
        int w = _startFrame.GetWidth();
        int h = _startFrame.GetHeight();
        float xf = (float)w / (float)displaysize.GetWidth() * (float)event.GetX();
        float yf = (float)h / (float)displaysize.GetHeight() * (float)event.GetY();
        if (xf < 0 || xf >= w || yf < 0 || yf >= h)
        {
            // outside image bounds
            return;
        }
        _lights.push_back(GCMBulb(wxPoint(xf, yf), _MI_CurrentNode, 255));
        _biFrame = CreateManualMask(_MI_CurrentFrame);
        ShowImage(_biFrame);
        MIValidateWindow();
    }
}


void GenerateCustomModelDialog::OnStaticBitmapMouseLeave(wxMouseEvent& event)
{
    if (_state == VideoProcessingStates::IDENTIFYING_BULBS)
    {
        if (_draggingedge >= 0)
        {
            ResizeClip(std::min(event.GetX(),_startFrame.GetWidth() - 1), std::min(event.GetY(), _startFrame.GetHeight() - 1));
        }
        _draggingedge = -1;
        SetCursor(wxCURSOR_ARROW);
    }
    else if (_state == VideoProcessingStates::IDENTIFYING_MANUAL)
    {
        SetCursor(wxCURSOR_ARROW);
    }
}

void GenerateCustomModelDialog::OnStaticBitmapMouseEnter(wxMouseEvent& event)
{
    if (_state == VideoProcessingStates::IDENTIFYING_MANUAL)
    {
        SetCursor(wxCURSOR_CROSS);
    }
}

void GenerateCustomModelDialog::OnStaticBitmapMouseMove(wxMouseEvent& event)
{
    if (_state == VideoProcessingStates::IDENTIFYING_BULBS)
    {
        if (_draggingedge >= 0)
        {
            ResizeClip(event.GetX(), event.GetY());
        }
        else
        {
            int e = GetEdge(event.GetX(), event.GetY());
            if (e == -1)
            {
                SetCursor(wxCURSOR_ARROW);
            }
            else if (e % 2 == 0)
            {
                SetCursor(wxCURSOR_SIZENS);
            }
            else
            {
                SetCursor(wxCURSOR_SIZEWE);
            }
        }
    }
}

void GenerateCustomModelDialog::AdvanceFrame()
{
    _MI_CurrentTime += (NODEON + NODEOFF);
    if (_MI_CurrentTime > _vr->GetLengthMS())
    {
        _MI_CurrentTime -= (NODEON + NODEOFF);
    }
    _MI_CurrentFrame = CreateImageFromFrame(_vr->GetNextFrame(_MI_CurrentTime)).Copy();
}

void GenerateCustomModelDialog::ReverseFrame()
{
    _MI_CurrentTime -= ((float)(NODEON + NODEOFF));
    if (_MI_CurrentTime < _startframetime + LEADON + FLAGOFF + FLAGON + FLAGOFF + (NODEON / 2))
    {
        _MI_CurrentTime = _startframetime + LEADON + FLAGOFF + FLAGON + FLAGOFF + (NODEON / 2);
    }
    _MI_CurrentFrame = CreateImageFromFrame(_vr->GetNextFrame(_MI_CurrentTime)).Copy();
}

void GenerateCustomModelDialog::OnButton_MI_PriorFrameClick(wxCommandEvent& event)
{
    if (_MI_CurrentNode > 1)
    {
        while (_lights.size() > 0 && _lights.back().GetNum() == _MI_CurrentNode)
        {
            _lights.pop_back();
        }
        _MI_CurrentNode--;
        StaticText12->SetLabel("Current: " + wxString::Format(wxT("%i"), _MI_CurrentNode));
        ReverseFrame();
    }
    _biFrame = CreateManualMask(_MI_CurrentFrame);
    ShowImage(_biFrame);
    MIValidateWindow();
}

void GenerateCustomModelDialog::OnButton_MI_NextFrameClick(wxCommandEvent& event)
{
    _MI_CurrentNode++;
    StaticText12->SetLabel("Current: " + wxString::Format(wxT("%i"), _MI_CurrentNode));
    AdvanceFrame();
    _biFrame = CreateManualMask(_MI_CurrentFrame);
    ShowImage(_biFrame);
    MIValidateWindow();
}

void GenerateCustomModelDialog::OnButton_MI_UndoBulbClick(wxCommandEvent& event)
{
    if (_lights.size() > 0 && _lights.back().GetNum() == _MI_CurrentNode)
    {
        _lights.pop_back();
    }
    _biFrame = CreateManualMask(_MI_CurrentFrame);
    ShowImage(_biFrame);
    MIValidateWindow();
}

void GenerateCustomModelDialog::OnButton_MI_BackClick(wxCommandEvent& event)
{
    if (SLRadioButton->GetValue())
    {
        CVTabEntry();
        SwapPage(PAGE_MANUALIDENTIFY, PAGE_CHOOSEVIDEO);
    }
    else
    {
        SFTabEntry();
        SwapPage(PAGE_MANUALIDENTIFY, PAGE_STARTFRAME);
    }
}

void GenerateCustomModelDialog::OnButton_MI_NextClick(wxCommandEvent& event)
{
    DoGenerateCustomModel();
    CMTabEntry();
    SwapPage(PAGE_MANUALIDENTIFY, PAGE_REVIEWMODEL);
}

void GenerateCustomModelDialog::MITabEntry(bool erase)
{
    if (erase)
    {
        _lights.clear();
        _MI_CurrentNode = 1;
    }
    _state = VideoProcessingStates::IDENTIFYING_MANUAL;

    if (SLRadioButton->GetValue())
    {
        Button_MI_NextFrame->Hide();
        Button_MI_PriorFrame->Hide();
        ButtonBumpBack->Hide();
        ButtonBumpFwd->Hide();
        // static bitmap
        _MI_CurrentFrame = _startFrame;
        StaticText12->Hide();
    }
    else
    {
        Button_MI_NextFrame->Show();
        Button_MI_PriorFrame->Show();
        ButtonBumpBack->Show();
        ButtonBumpFwd->Show();
        StaticText12->Show();
        StaticText12->SetLabel("Current: " + wxString::Format(wxT("%i"), _MI_CurrentNode));
        _MI_CurrentTime = _startframetime + LEADON + FLAGOFF + FLAGON + FLAGOFF + (NODEON / 2) - (NODEON + NODEOFF) + (_MI_CurrentNode - 1) * (NODEON + NODEOFF);

        // video ... need to move to first frame
        AdvanceFrame();
    }
    _biFrame = CreateManualMask(_MI_CurrentFrame);
    ShowImage(_biFrame);
    MIValidateWindow();
}

void GenerateCustomModelDialog::OnButton_CV_ManualClick(wxCommandEvent& event)
{
    Button_CV_Manual->Disable();
    Button_CV_Next->Disable();
    Button_GCM_SelectFile->Disable();
    Button_CV_Back->Disable();
    TextCtrl_GCM_Filename->Disable();
    _manual = true;

    static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
    logger_gcm.info("File: %s.", (const char *)TextCtrl_GCM_Filename->GetValue().c_str());

    MITabEntry(true);
    SwapPage(PAGE_CHOOSEVIDEO, PAGE_MANUALIDENTIFY);
    ValidateWindow();
}

wxImage GenerateCustomModelDialog::CreateManualMask(wxImage ref)
{
    wxBitmap bmp(ref.GetWidth(), ref.GetHeight());
    wxMemoryDC dc(bmp);

    dc.DrawBitmap(ref, wxPoint(0, 0), false);

    wxSize displaysize = StaticBitmap_Preview->GetSize();
    float factor = std::max((float)_startFrame.GetWidth() / (float)displaysize.GetWidth(),
        (float)_startFrame.GetHeight() / (float)displaysize.GetHeight());

    // now red so they are easy to see
    for (auto c = _lights.begin(); c != _lights.end(); c++)
    {
        c->Draw(dc, factor);
    }

    return bmp.ConvertToImage();
}

void GenerateCustomModelDialog::OnButton_SF_ManualClick(wxCommandEvent& event)
{
    Button_Back10Frames->Disable();
    Button_Back1Frame->Disable();
    Button_Forward10Frames->Disable();
    Button_Forward1Frame->Disable();
    Button_SF_Next->Disable();
    Button_SF_Manual->Disable();
    Button_SF_Back->Disable();
    _manual = true;

    _biFrame = _startFrame.Copy();

    MITabEntry(true);
    SwapPage(PAGE_STARTFRAME, PAGE_MANUALIDENTIFY);
    ValidateWindow();
}

void GenerateCustomModelDialog::MIValidateWindow()
{
    if (_MI_CurrentNode == 1)
    {
        Button_MI_PriorFrame->Disable();
    }
    else
    {
        Button_MI_PriorFrame->Enable();
    }
    if (_lights.size() == 0)
    {
        Button_MI_Next->Disable();
    }
    else
    {
        Button_MI_Next->Enable();
    }
    if (_lights.size() > 0 && _lights.back().GetNum() == _MI_CurrentNode)
    {
        Button_MI_UndoBulb->Enable();
    }
    else
    {
        Button_MI_UndoBulb->Disable();
    }
    if (!SLRadioButton->GetValue())
    {
        if (_MI_CurrentTime + NODEON + NODEOFF > _vr->GetLengthMS())
        {
            Button_MI_NextFrame->Disable();
        }
        else
        {
            Button_MI_NextFrame->Enable();
        }
    }
}

void GenerateCustomModelDialog::OnButtonBumpBackClick(wxCommandEvent& event)
{
    _MI_CurrentTime -= FRAMEMS;
    if (_MI_CurrentTime < _startframetime + LEADON + FLAGOFF + FLAGON + FLAGOFF + (NODEON / 2))
    {
        _MI_CurrentTime = _startframetime + LEADON + FLAGOFF + FLAGON + FLAGOFF + (NODEON / 2);
    }
    _MI_CurrentFrame = CreateImageFromFrame(_vr->GetNextFrame(_MI_CurrentTime)).Copy();
    _biFrame = CreateManualMask(_MI_CurrentFrame);
    ShowImage(_biFrame);
    MIValidateWindow();
}

void GenerateCustomModelDialog::OnButtonBumpFwdClick(wxCommandEvent& event)
{
    _MI_CurrentTime += FRAMEMS;
    if (_MI_CurrentTime > _vr->GetLengthMS())
    {
        _MI_CurrentTime -= FRAMEMS;
    }
    _MI_CurrentFrame = CreateImageFromFrame(_vr->GetNextFrame(_MI_CurrentTime)).Copy();
    _biFrame = CreateManualMask(_MI_CurrentFrame);
    ShowImage(_biFrame);
    MIValidateWindow();
}

void GenerateCustomModelDialog::OnSlider_BI_MinScaleCmdScrollChanged(wxScrollEvent& event)
{
    if (!CheckBox_BI_ManualUpdate->GetValue())
    {
        if (!_busy)
        {
            DoBulbIdentify();
        }
    }
}

void GenerateCustomModelDialog::OnSlider_BI_MinScaleCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_BI_MinScale->SetValue(wxString::Format(wxT("%i"), Slider_BI_MinScale->GetValue()));
}
