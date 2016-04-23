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

//(*IdInit(GenerateCustomModelDialog)
const long GenerateCustomModelDialog::ID_RADIOBOX1 = wxNewId();
const long GenerateCustomModelDialog::ID_SPINCTRL_NC_Count = wxNewId();
const long GenerateCustomModelDialog::ID_SPINCTRL_StartChannel = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_Intensity = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_PCM_Run = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_Prepare = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_GCM_Filename = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_GCM_SelectFile = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_GCM_Generate = wxNewId();
const long GenerateCustomModelDialog::ID_STATICBITMAP_Preview = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Back1Frame = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Forward1Frame = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Back10Frames = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Forward10Frames = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT_StartFrameOk = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_SF_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_StartFrame = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT1 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_LevelFilterAdjust = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_BD_Back = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_BD_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_BulbDetect = wxNewId();
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

	//(*Initialize(GenerateCustomModelDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxStaticText* StaticText2;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer3;
	wxStaticText* StaticText6;
	wxFlexGridSizer* FlexGridSizer5;
	wxStaticText* StaticText8;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText1;
	wxStaticText* StaticText3;
	wxFlexGridSizer* FlexGridSizer7;
	wxStaticText* StaticText5;
	wxStaticText* StaticText7;
	wxFlexGridSizer* FlexGridSizer8;
	wxButton* Button_GCM_SelectFile;
	wxFlexGridSizer* FlexGridSizer13;
	wxFlexGridSizer* FlexGridSizer12;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer11;
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
	FlexGridSizer3->AddGrowableRow(1);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText3 = new wxStaticText(Panel_Generate, wxID_ANY, _("Video File"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_GCM_Filename = new wxTextCtrl(Panel_Generate, ID_TEXTCTRL_GCM_Filename, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_GCM_Filename"));
	FlexGridSizer4->Add(TextCtrl_GCM_Filename, 1, wxALL|wxEXPAND, 2);
	Button_GCM_SelectFile = new wxButton(Panel_Generate, ID_BUTTON_GCM_SelectFile, _("..."), wxDefaultPosition, wxSize(29,28), 0, wxDefaultValidator, _T("ID_BUTTON_GCM_SelectFile"));
	FlexGridSizer4->Add(Button_GCM_SelectFile, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_GCM_Generate = new wxButton(Panel_Generate, ID_BUTTON_GCM_Generate, _("Generate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_GCM_Generate"));
	FlexGridSizer4->Add(Button_GCM_Generate, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer5->AddGrowableRow(0);
	StaticBitmap_Preview = new wxStaticBitmap(Panel_Generate, ID_STATICBITMAP_Preview, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER|wxCLIP_CHILDREN, _T("ID_STATICBITMAP_Preview"));
	FlexGridSizer5->Add(StaticBitmap_Preview, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer7 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	FlexGridSizer7->AddGrowableRow(0);
	AuiNotebook_ProcessSettings = new wxAuiNotebook(Panel_Generate, ID_AUINOTEBOOK_ProcessSettings, wxDefaultPosition, wxDefaultSize, 0);
	Panel_StartFrame = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_StartFrame, wxPoint(43,126), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_StartFrame"));
	FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	FlexGridSizer10->AddGrowableRow(2);
	FlexGridSizer11 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Back1Frame = new wxButton(Panel_StartFrame, ID_BUTTON_Back1Frame, _("Back 1 Frame"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Back1Frame"));
	FlexGridSizer11->Add(Button_Back1Frame, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Forward1Frame = new wxButton(Panel_StartFrame, ID_BUTTON_Forward1Frame, _("Forward 1 Frame"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Forward1Frame"));
	FlexGridSizer11->Add(Button_Forward1Frame, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Back10Frames = new wxButton(Panel_StartFrame, ID_BUTTON_Back10Frames, _("Back 10 Frames"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Back10Frames"));
	FlexGridSizer11->Add(Button_Back10Frames, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Forward10Frames = new wxButton(Panel_StartFrame, ID_BUTTON_Forward10Frames, _("Forward 10 Frames"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Forward10Frames"));
	FlexGridSizer11->Add(Button_Forward10Frames, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer10->Add(FlexGridSizer11, 1, wxALL|wxEXPAND, 5);
	StaticText_StartFrameOk = new wxStaticText(Panel_StartFrame, ID_STATICTEXT_StartFrameOk, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_StartFrameOk"));
	wxFont StaticText_StartFrameOkFont(12,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText_StartFrameOk->SetFont(StaticText_StartFrameOkFont);
	FlexGridSizer10->Add(StaticText_StartFrameOk, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer12->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_SF_Next = new wxButton(Panel_StartFrame, ID_BUTTON_SF_Next, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SF_Next"));
	FlexGridSizer12->Add(Button_SF_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10->Add(FlexGridSizer12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_StartFrame->SetSizer(FlexGridSizer10);
	FlexGridSizer10->Fit(Panel_StartFrame);
	FlexGridSizer10->SetSizeHints(Panel_StartFrame);
	Panel_BulbDetect = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_BulbDetect, wxPoint(71,14), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_BulbDetect"));
	Panel_BulbDetect->Hide();
	FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	FlexGridSizer9->AddGrowableRow(1);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(1);
	StaticText9 = new wxStaticText(Panel_BulbDetect, ID_STATICTEXT1, _("Adjust"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer8->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_LevelFilterAdjust = new wxSlider(Panel_BulbDetect, ID_SLIDER_LevelFilterAdjust, 200, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_LevelFilterAdjust"));
	FlexGridSizer8->Add(Slider_LevelFilterAdjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer13 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_BD_Back = new wxButton(Panel_BulbDetect, ID_BUTTON_BD_Back, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_BD_Back"));
	FlexGridSizer13->Add(Button_BD_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_BD_Next = new wxButton(Panel_BulbDetect, ID_BUTTON_BD_Next, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_BD_Next"));
	FlexGridSizer13->Add(Button_BD_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9->Add(FlexGridSizer13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_BulbDetect->SetSizer(FlexGridSizer9);
	FlexGridSizer9->Fit(Panel_BulbDetect);
	FlexGridSizer9->SetSizeHints(Panel_BulbDetect);
	AuiNotebook_ProcessSettings->AddPage(Panel_StartFrame, _("Start Frame"));
	AuiNotebook_ProcessSettings->AddPage(Panel_BulbDetect, _("Bulb Detect"));
	FlexGridSizer7->Add(AuiNotebook_ProcessSettings, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 2);
	Panel_Generate->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel_Generate);
	FlexGridSizer3->SetSizeHints(Panel_Generate);
	AuiNotebook1->AddPage(Panel_Prepare, _("Prepare"), true);
	AuiNotebook1->AddPage(Panel_Generate, _("Process"));
	FlexGridSizer1->Add(AuiNotebook1, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 2);
	SetSizer(FlexGridSizer1);
	FileDialog1 = new wxFileDialog(this, _("Select file"), wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_PCM_Run,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_PCM_RunClick);
	Connect(ID_TEXTCTRL_GCM_Filename,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnTextCtrl_GCM_FilenameText);
	Connect(ID_BUTTON_GCM_SelectFile,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_GCM_SelectFileClick);
	Connect(ID_BUTTON_GCM_Generate,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_GCM_GenerateClick);
	Connect(ID_BUTTON_SF_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_SF_NextClick);
	Connect(ID_SLIDER_LevelFilterAdjust,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_LevelFilterAdjustCmdScroll);
	Connect(ID_BUTTON_BD_Back,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BD_BackClick);
	Connect(ID_BUTTON_BD_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BD_NextClick);
	//*)

    StaticBitmap1->SetScaleMode(wxStaticBitmapBase::ScaleMode::Scale_AspectFill);

    ValidateWindow();
}

GenerateCustomModelDialog::~GenerateCustomModelDialog()
{
	//(*Destroy(GenerateCustomModelDialog)
	//*)
}

void GenerateCustomModelDialog::ValidateWindow()
{
    wxString file = TextCtrl_GCM_Filename->GetValue();
    if (file == "" || wxFile::Exists(file))
    {
        TextCtrl_GCM_Filename->SetBackgroundColour(*wxWHITE);
        Button_GCM_Generate->Enable();
    }
    else
    {
        TextCtrl_GCM_Filename->SetBackgroundColour(*wxRED);
        Button_GCM_Generate->Disable();
    }
}

void GenerateCustomModelDialog::OnButton_GCM_SelectFileClick(wxCommandEvent& event)
{
    FileDialog1->SetWildcard(VIDEOWILDCARD);
    if (FileDialog1->ShowModal() == wxID_OK)
    {
        TextCtrl_GCM_Filename->SetValue(FileDialog1->GetDirectory() + "/" + FileDialog1->GetFilename());
    }
    ValidateWindow();
}

void GenerateCustomModelDialog::OnTextCtrl_GCM_FilenameText(wxCommandEvent& event)
{
    ValidateWindow();
}

#define AVFrameRed(d, w,x,y) (int)*(d + w * 3 * y + x * 3)
#define AVFrameGreen(d, w,x,y) (int)*(d + w * 3 * y + x * 3 + 1)
#define AVFrameBlue(d, w,x,y) (int)*(d + w * 3 * y + x * 3 + 2)

float GenerateCustomModelDialog::CalcFrameBrightness(AVFrame* image)
{
    //wxImage img(image->width, image->height, true);
    //img.SetData((unsigned char*)(image->data[0]));
    //wxImage grey = img.ConvertToGreyscale();
    //StaticBitmap1->SetBitmap(grey);

    int w = image->width;
    int h = image->height;
    int64_t total = 0;
    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            total += (int)((float)AVFrameRed(image->data[0], w, x, y) * 0.299 + (float)AVFrameGreen(image->data[0], w, x, y) * 0.587 + (float)AVFrameBlue(image->data[0], w, x, y) * 0.114);
        }
    }

    return (float)((double)total / ((double)w * (double)h) / 255.0);
}

#define STARTSCANSECS 30
#define FRAMEMS 50
#define LEADOFF 3000
#define LEADON 500
#define FLAGON 500
#define FLAGOFF 500
#define NODEON 500
#define NODEOFF 200


// returns the MS of the best start frame - 0.1 MS into what looks like a bright section of the video that lasts about 3 seconds
int GenerateCustomModelDialog::FindStartFrame(VideoReader* vr)
{
    std::list<float> framebrightness;

    // scan first STARTSCANSECS seconds of video
    for (int s = 0; s < STARTSCANSECS; s++)
    {
        // calculate each frame average brightness
        for (int ss = 0; ss < 20; ss++)
        {
            int ms = s * 1000 + ss * FRAMEMS;
            AVFrame* image = vr->GetNextFrame(ms);

            framebrightness.push_back(CalcFrameBrightness(image));
        }
    }

    // find a LEADON millisecondsecond high ... this will be our start frame
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

    int first = 0;
    int last = 0;
    int curfirst = 0;
    int curlast = 0;
    for (int l = 1; l < 10; l++)
    {
        curlast = l;
        if (suitable[l])
        {
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
        bestlevel = (float)((int)((last - first) / 2 * 10)) / 10.0;
    }

    // pick a point 0.1 secs into the high period as our start frame
    return levelmaxstart[bestlevel] * FRAMEMS;
}

std::list<wxPoint> GenerateCustomModelDialog::FindLights(wxImage& image)
{
    std::list<wxPoint> res;

    // Greyscale the image
    wxImage grey = image.ConvertToGreyscale();

    StaticBitmap1->SetBitmap(grey);

    TextCtrl_Message->SetValue("Grey. Continue?");
    _continue = false;
    while (!_continue) wxYield();
    TextCtrl_Message->SetValue("");

    // blur the photo to eliminate the noise
    wxImage blur = grey.Blur(15);

    StaticBitmap1->SetBitmap(blur);

    TextCtrl_Message->SetValue("Blur. Continue?");
    _continue = false;
    while (!_continue) wxYield();
    TextCtrl_Message->SetValue("");

    // sharpen the image

    // take out any pixel about threshold not surrounded by at least 3 bright pixels?

    // take out the bottom 15%

    // emphasise the top 15%

    return res;
}

void GenerateCustomModelDialog::OnButton_GCM_GenerateClick(wxCommandEvent& event)
{
    VideoReader vr(std::string(TextCtrl_GCM_Filename->GetValue().c_str()), 800, 600, false);

    wxProgressDialog pd("Analysing video ...", "", 100, this);
    pd.Update(1);

    int starttime = FindStartFrame(&vr);
    pd.Update(15, wxString::Format("Start frame found at %dms", starttime));

    AVFrame* image;
    image = vr.GetNextFrame(starttime);
    wxImage bmp(image->width, image->height, true);
    bmp.SetData((unsigned char*)(image->data[0]));
    StaticBitmap1->SetBitmap(bmp);

    TextCtrl_Message->SetValue("Start Frame. Continue?");
    _continue = false;
    while (!_continue) wxYield();
    TextCtrl_Message->SetValue("");

    // confirm the start frame with the user ... let them bump it forward or back

    // look at this start frame and identify all the light locations
    std::list<wxPoint> points = FindLights(bmp);
    pd.Update(30, wxString::Format("Start frame analysed for lights. %d found.", points.size()));

    // confirm with the user that i identified them all ... let them manuall add them/move them around?

    // generate a grid with just x's to show where the lights are
    // grid should be minimal. narrow and short and as little resolution as possible

    // confirm with the user as well

    int currpixelframe = starttime + LEADON + FLAGOFF + FLAGON + FLAGOFF;

    // now go through all the frames at the expected points and check only which of the light locations is on so I can number them

    // now regenerate the grid with the x's replaced by numbers

    // export the model to an xmodel file ... ready for use

    pd.Update(100);
    pd.Close();
    SetFocus();
}

void GenerateCustomModelDialog::UpdateProgress(wxProgressDialog& pd, int totaltime)
{
    wxTimeSpan ts = wxDateTime::UNow() - _starttime;
    int progress = ts.GetMilliseconds().ToLong() * 100 / totaltime;
    if (progress > 100) progress = 100;
    pd.Update(progress);
}

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
            for (int j = 0; j < count; j ++)
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
    while(curtimex - curtime < ms)
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
}

void GenerateCustomModelDialog::OnButton_ContinueClick(wxCommandEvent& event)
{
    _continue = true;
}

void GenerateCustomModelDialog::OnButton_BackClick(wxCommandEvent& event)
{
}

void GenerateCustomModelDialog::OnButton_NextClick(wxCommandEvent& event)
{
}

void GenerateCustomModelDialog::OnButton_SF_NextClick(wxCommandEvent& event)
{
}

void GenerateCustomModelDialog::OnButton_BD_BackClick(wxCommandEvent& event)
{
}

void GenerateCustomModelDialog::OnButton_BD_NextClick(wxCommandEvent& event)
{
}

void GenerateCustomModelDialog::OnSlider_LevelFilterAdjustCmdScroll(wxScrollEvent& event)
{
}
