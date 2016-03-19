#include "PianoPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(PianoPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/spinctrl.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)


#include "wx/filedlg.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>

//(*IdInit(PianoPanel)
const long PianoPanel::ID_STATICTEXT1 = wxNewId();
const long PianoPanel::ID_SPINCTRL_Piano_StartMIDI = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_SLIDER_Piano_Style = wxNewId();
const long PianoPanel::ID_STATICTEXT_Piano_NumKeys = wxNewId();
const long PianoPanel::ID_SPINCTRL_Piano_EndMIDI = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_NumKeys = wxNewId();
const long PianoPanel::ID_CHECKBOX_Piano_ShowSharps = wxNewId();
const long PianoPanel::ID_STATICTEXT_Piano_NumRows = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_NumRows = wxNewId();
//*)

BEGIN_EVENT_TABLE(PianoPanel,wxPanel)
	//(*EventTable(PianoPanel)
	//*)
END_EVENT_TABLE()

PianoPanel::PianoPanel(wxWindow* parent)
{
	//(*Initialize(PianoPanel)
	wxFlexGridSizer* FlexGridSizer27;
	wxFlexGridSizer* FlexGridSizer5;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer27 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer27->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Start MIDI Channel [0-127]"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer27->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Piano_StartMIDI = new wxSpinCtrl(this, ID_SPINCTRL_Piano_StartMIDI, _T("60"), wxDefaultPosition, wxDefaultSize, 0, 0, 127, 60, _T("ID_SPINCTRL_Piano_StartMIDI"));
	SpinCtrl_Piano_StartMIDI->SetValue(_T("60"));
	FlexGridSizer27->Add(SpinCtrl_Piano_StartMIDI, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Piano_Style = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Piano_Style, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Piano_Style"));
	BitmapButton_Piano_Style->SetDefault();
	FlexGridSizer27->Add(BitmapButton_Piano_Style, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText_Piano_NumKeys = new wxStaticText(this, ID_STATICTEXT_Piano_NumKeys, _("End MIDI Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_NumKeys"));
	FlexGridSizer27->Add(StaticText_Piano_NumKeys, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Piano_EndMIDI = new wxSpinCtrl(this, ID_SPINCTRL_Piano_EndMIDI, _T("72"), wxDefaultPosition, wxDefaultSize, 0, 0, 127, 72, _T("ID_SPINCTRL_Piano_EndMIDI"));
	SpinCtrl_Piano_EndMIDI->SetValue(_T("72"));
	FlexGridSizer27->Add(SpinCtrl_Piano_EndMIDI, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Piano_NumKeys = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_NumKeys, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_NumKeys"));
	BitmapButton_Piano_NumKeys->SetDefault();
	FlexGridSizer27->Add(BitmapButton_Piano_NumKeys, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Piano_ShowSharps = new wxCheckBox(this, ID_CHECKBOX_Piano_ShowSharps, _("Show Sharps and Flats"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Piano_ShowSharps"));
	CheckBox_Piano_ShowSharps->SetValue(true);
	FlexGridSizer27->Add(CheckBox_Piano_ShowSharps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Piano_NumRows = new wxStaticText(this, ID_STATICTEXT_Piano_NumRows, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_NumRows"));
	FlexGridSizer27->Add(StaticText_Piano_NumRows, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Piano_NumRows = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_NumRows, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_NumRows"));
	BitmapButton_Piano_NumRows->SetDefault();
	FlexGridSizer27->Add(BitmapButton_Piano_NumRows, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(FlexGridSizer27, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(this);
	FlexGridSizer5->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Piano_Style,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_Piano_NumKeys,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_Piano_NumRows,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_PIANO");

}

PianoPanel::~PianoPanel()
{
	//(*Destroy(PianoPanel)
	//*)
}

PANEL_EVENT_HANDLERS(PianoPanel)
void PianoPanel::OnSpinCtrl_Piano_StartMIDIChange(wxSpinEvent& event)
{
	int start = SpinCtrl_Piano_StartMIDI->GetValue();
	int end = SpinCtrl_Piano_EndMIDI->GetValue();

	if (start > end)
	{
		SpinCtrl_Piano_EndMIDI->SetValue(start);
	}
}

void PianoPanel::OnSpinCtrl_Piano_EndMIDIChange(wxSpinEvent& event)
{
	int start = SpinCtrl_Piano_StartMIDI->GetValue();
	int end = SpinCtrl_Piano_EndMIDI->GetValue();

	if (end < start)
	{
		SpinCtrl_Piano_StartMIDI->SetValue(end);
	}
}
