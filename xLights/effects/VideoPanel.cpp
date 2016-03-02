#include "VideoPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(VideoPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/filepicker.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(VideoPanel)
const long VideoPanel::ID_FILEPICKERCTRL_Video_Filename = wxNewId();
const long VideoPanel::ID_BITMAPBUTTON_Video_Filename = wxNewId();
const long VideoPanel::ID_STATICTEXT8 = wxNewId();
const long VideoPanel::ID_SLIDER_Video_Starttime = wxNewId();
const long VideoPanel::ID_TEXTCTRL_Video_Starttime = wxNewId();
const long VideoPanel::ID_BITMAPBUTTON_Video_Starttime = wxNewId();
const long VideoPanel::ID_CHECKBOX_Video_AspectRatio = wxNewId();
//*)

BEGIN_EVENT_TABLE(VideoPanel,wxPanel)
	//(*EventTable(VideoPanel)
	//*)
END_EVENT_TABLE()

VideoPanel::VideoPanel(wxWindow* parent)
{
	//(*Initialize(VideoPanel)
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer42;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer42->AddGrowableCol(0);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FilePicker_Video_Filename = new xlVideoFilePickerCtrl(this, ID_FILEPICKERCTRL_Video_Filename, wxEmptyString, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL_Video_Filename"));
	FlexGridSizer1->Add(FilePicker_Video_Filename, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Filename = new wxBitmapButton(this, ID_BITMAPBUTTON_Video_Filename, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Video_Filename"));
	BitmapButton_Filename->SetDefault();
	BitmapButton_Filename->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer1->Add(BitmapButton_Filename, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer42->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Start Time"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer2->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Video_Starttime = new wxSlider(this, ID_SLIDER_Video_Starttime, 0, 0, 20, wxDefaultPosition, wxSize(200,-1), 0, wxDefaultValidator, _T("ID_SLIDER_Video_Starttime"));
	FlexGridSizer2->Add(Slider_Video_Starttime, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Video_Starttime = new wxTextCtrl(this, ID_TEXTCTRL_Video_Starttime, _("0.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(50,-1)), wxTE_READONLY|wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL_Video_Starttime"));
	FlexGridSizer2->Add(TextCtrl_Video_Starttime, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Video_Starttime = new wxBitmapButton(this, ID_BITMAPBUTTON_Video_Starttime, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Video_Starttime"));
	BitmapButton_Video_Starttime->SetDefault();
	BitmapButton_Video_Starttime->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer2->Add(BitmapButton_Video_Starttime, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer42->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_Video_AspectRatio = new wxCheckBox(this, ID_CHECKBOX_Video_AspectRatio, _("Maintain Aspect Ratio"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Video_AspectRatio"));
	CheckBox_Video_AspectRatio->SetValue(false);
	FlexGridSizer42->Add(CheckBox_Video_AspectRatio, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer42);
	FlexGridSizer42->Fit(this);
	FlexGridSizer42->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_Video_Filename,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Video_Starttime,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&VideoPanel::OnSlider_Video_StarttimeCmdSliderUpdated);
	Connect(ID_TEXTCTRL_Video_Starttime,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&VideoPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_Video_Starttime,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_Video");
	TextCtrl_Video_Starttime->SetValue("0.000");
	CheckBox_Video_AspectRatio->SetValue(false);
}

VideoPanel::~VideoPanel()
{
	//(*Destroy(VideoPanel)
	//*)
}

PANEL_EVENT_HANDLERS(VideoPanel)

static inline void EnableControl(wxWindow *w, int id, bool e) {
    wxWindow *c = w->FindWindowById(id);
    if (c) c->Enable(e);
}

void VideoPanel::OnSlider_Video_StarttimeCmdSliderUpdated(wxScrollEvent& event)
{
	int ms = Slider_Video_Starttime->GetValue();
	int seconds = ms / 1000;
	ms = ms - seconds * 1000;
	TextCtrl_Video_Starttime->SetValue(wxString::Format("%d.%03d", seconds, ms));
}
