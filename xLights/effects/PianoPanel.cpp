#include "PianoPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"
#include <wx/textfile.h>
#include "../sequencer/Effect.h"
#include "PianoEffect.h"

//(*InternalHeaders(PianoPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/spinctrl.h>
#include <wx/slider.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

#include <wx/filename.h>
#include <wx/valnum.h>

#include "../xLightsMain.h"
#include <log4cpp/Category.hh>

//(*IdInit(PianoPanel)
const long PianoPanel::ID_STATICTEXT1 = wxNewId();
const long PianoPanel::ID_CHOICE_Piano_Type = wxNewId();
const long PianoPanel::ID_STATICTEXT2 = wxNewId();
const long PianoPanel::ID_SPINCTRL_Piano_StartMIDI = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_StartMIDI = wxNewId();
const long PianoPanel::ID_STATICTEXT_Piano_NumKeys = wxNewId();
const long PianoPanel::ID_SPINCTRL_Piano_EndMIDI = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_EndMIDI = wxNewId();
const long PianoPanel::ID_CHECKBOX_Piano_ShowSharps = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_ShowSharps = wxNewId();
const long PianoPanel::ID_STATICTEXT8 = wxNewId();
const long PianoPanel::ID_CHOICE_Piano_MIDITrack_APPLYLAST = wxNewId();
const long PianoPanel::ID_STATICTEXT7 = wxNewId();
const long PianoPanel::ID_SLIDER_Piano_Scale = wxNewId();
const long PianoPanel::ID_VALUECURVE_Piano_Scale = wxNewId();
const long PianoPanel::IDD_TEXTCTRL_Piano_Scale = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_Scale = wxNewId();
const long PianoPanel::ID_STATICTEXT9 = wxNewId();
const long PianoPanel::ID_SLIDER_Piano_XOffset = wxNewId();
const long PianoPanel::IDD_TEXTCTRL_Piano_XOffset = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_XOffset = wxNewId();
//*)

BEGIN_EVENT_TABLE(PianoPanel,wxPanel)
	//(*EventTable(PianoPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_SETTIMINGTRACKS, PianoPanel::SetTimingTracks)
END_EVENT_TABLE()

PianoPanel::PianoPanel(wxWindow* parent)
{
	//(*Initialize(PianoPanel)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer27;
	wxFlexGridSizer* FlexGridSizer42;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer42->AddGrowableCol(0);
	FlexGridSizer27 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer27->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer27->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Piano_Type = new wxChoice(this, ID_CHOICE_Piano_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Piano_Type"));
	Choice_Piano_Type->SetSelection( Choice_Piano_Type->Append(_("True Piano")) );
	Choice_Piano_Type->Append(_("Bars"));
	FlexGridSizer27->Add(Choice_Piano_Type, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Start MIDI Channel [0-127]"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer27->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_Piano_StartMIDI = new wxSpinCtrl(this, ID_SPINCTRL_Piano_StartMIDI, _T("60"), wxDefaultPosition, wxDefaultSize, 0, 0, 127, 60, _T("ID_SPINCTRL_Piano_StartMIDI"));
	SpinCtrl_Piano_StartMIDI->SetValue(_T("60"));
	FlexGridSizer27->Add(SpinCtrl_Piano_StartMIDI, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Piano_StartMIDI = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_StartMIDI, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_StartMIDI"));
	FlexGridSizer27->Add(BitmapButton_Piano_StartMIDI, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText_Piano_NumKeys = new wxStaticText(this, ID_STATICTEXT_Piano_NumKeys, _("End MIDI Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_NumKeys"));
	FlexGridSizer27->Add(StaticText_Piano_NumKeys, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_Piano_EndMIDI = new wxSpinCtrl(this, ID_SPINCTRL_Piano_EndMIDI, _T("72"), wxDefaultPosition, wxDefaultSize, 0, 0, 127, 72, _T("ID_SPINCTRL_Piano_EndMIDI"));
	SpinCtrl_Piano_EndMIDI->SetValue(_T("72"));
	FlexGridSizer27->Add(SpinCtrl_Piano_EndMIDI, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Piano_EndMIDI = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_EndMIDI, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_EndMIDI"));
	FlexGridSizer27->Add(BitmapButton_Piano_EndMIDI, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Piano_ShowSharps = new wxCheckBox(this, ID_CHECKBOX_Piano_ShowSharps, _("Show Sharps and Flats"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Piano_ShowSharps"));
	CheckBox_Piano_ShowSharps->SetValue(true);
	FlexGridSizer27->Add(CheckBox_Piano_ShowSharps, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Piano_ShowSharps = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_ShowSharps, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_ShowSharps"));
	FlexGridSizer27->Add(BitmapButton_Piano_ShowSharps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Track"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer27->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Piano_MIDITrack_APPLYLAST = new wxChoice(this, ID_CHOICE_Piano_MIDITrack_APPLYLAST, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Piano_MIDITrack_APPLYLAST"));
	FlexGridSizer27->Add(Choice_Piano_MIDITrack_APPLYLAST, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer42->Add(FlexGridSizer27, 0, wxEXPAND, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Vertical Scale"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer3->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_Piano_Scale = new wxSlider(this, ID_SLIDER_Piano_Scale, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Piano_Scale"));
	FlexGridSizer4->Add(Slider_Piano_Scale, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Piano_ScaleVC = new ValueCurveButton(this, ID_VALUECURVE_Piano_Scale, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Piano_Scale"));
	FlexGridSizer4->Add(BitmapButton_Piano_ScaleVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Piano_Scale = new wxTextCtrl(this, IDD_TEXTCTRL_Piano_Scale, _("100"), wxDefaultPosition, wxSize(44,24), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Piano_Scale"));
	FlexGridSizer3->Add(TextCtrl_Piano_Scale, 1, wxALL, 2);
	BitmapButton_Piano_scale = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_Scale, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_Scale"));
	FlexGridSizer3->Add(BitmapButton_Piano_scale, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Horizontal Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer3->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Piano_XOffset = new wxSlider(this, ID_SLIDER_Piano_XOffset, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Piano_XOffset"));
	FlexGridSizer3->Add(Slider_Piano_XOffset, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Piano_XOffset = new wxTextCtrl(this, IDD_TEXTCTRL_Piano_XOffset, _("0"), wxDefaultPosition, wxSize(44,24), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Piano_XOffset"));
	FlexGridSizer3->Add(TextCtrl_Piano_XOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Piano_XOffset = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_XOffset, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_XOffset"));
	FlexGridSizer3->Add(BitmapButton_Piano_XOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer42->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer42);
	FlexGridSizer42->Fit(this);
	FlexGridSizer42->SetSizeHints(this);

	Connect(ID_SPINCTRL_Piano_StartMIDI,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&PianoPanel::OnSpinCtrl_Piano_StartMIDIChange);
	Connect(ID_BITMAPBUTTON_Piano_StartMIDI,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_SPINCTRL_Piano_EndMIDI,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&PianoPanel::OnSpinCtrl_Piano_EndMIDIChange);
	Connect(ID_BITMAPBUTTON_Piano_EndMIDI,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_Piano_ShowSharps,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Piano_Scale,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&PianoPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Piano_Scale,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Piano_Scale,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PianoPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_Piano_Scale,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Piano_XOffset,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&PianoPanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_Piano_XOffset,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PianoPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_Piano_XOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	//*)

    SetName("ID_PANEL_PIANO");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&PianoPanel::OnVCChanged, nullptr, this);
    
    // Set value curve limits
    BitmapButton_Piano_ScaleVC->GetValue()->SetLimits(PIANO_SCALE_MIN, PIANO_SCALE_MAX);

	ValidateWindow();
}

PianoPanel::~PianoPanel()
{
	//(*Destroy(PianoPanel)
	//*)
}

PANEL_EVENT_HANDLERS(PianoPanel)

void PianoPanel::OnSpinCtrl_Piano_StartMIDIChange(wxSpinEvent& event)
{
    // ensure start is never greater than end
	int start = SpinCtrl_Piano_StartMIDI->GetValue();
	int end = SpinCtrl_Piano_EndMIDI->GetValue();

	if (start > end)
	{
		SpinCtrl_Piano_EndMIDI->SetValue(start);
	}
    ValidateWindow();
}

void PianoPanel::OnSpinCtrl_Piano_EndMIDIChange(wxSpinEvent& event)
{
    // ensure end is never less than start
	int start = SpinCtrl_Piano_StartMIDI->GetValue();
	int end = SpinCtrl_Piano_EndMIDI->GetValue();

	if (end < start)
	{
		SpinCtrl_Piano_StartMIDI->SetValue(end);
	}
    ValidateWindow();
}

void PianoPanel::ValidateWindow()
{
    if (Choice_Piano_MIDITrack_APPLYLAST->GetCount() > 0)
    {
        Choice_Piano_MIDITrack_APPLYLAST->Enable(true);
    }
    else
    {
        Choice_Piano_MIDITrack_APPLYLAST->Enable(false);
    }

    SpinCtrl_Piano_StartMIDI->SetToolTip(wxString(xLightsFrame::DecodeMidi(SpinCtrl_Piano_StartMIDI->GetValue()).c_str()));
    SpinCtrl_Piano_EndMIDI->SetToolTip(wxString(xLightsFrame::DecodeMidi(SpinCtrl_Piano_EndMIDI->GetValue()).c_str()));
}

void PianoPanel::SetTimingTracks(wxCommandEvent& event)
{
    auto timingtracks = wxSplit(event.GetString(), '|');

    wxString selection = Choice_Piano_MIDITrack_APPLYLAST->GetStringSelection();

    // check if anything has been removed ... if it has clear the list and we will have to rebuild it as you cant delete items from a combo box
    bool removed = false;
    for (size_t i = 0; i < Choice_Piano_MIDITrack_APPLYLAST->GetCount(); i++)
    {
        bool found = false;
        for (auto it = timingtracks.begin(); it != timingtracks.end(); ++it)
        {
            if (*it == Choice_Piano_MIDITrack_APPLYLAST->GetString(i))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Choice_Piano_MIDITrack_APPLYLAST->Clear();
            removed = true;
            break;
        }
    }

    // add any new timing tracks
    for (auto it = timingtracks.begin(); it != timingtracks.end(); ++it)
    {
        bool found = false;
        for (size_t i = 0; i < Choice_Piano_MIDITrack_APPLYLAST->GetCount(); i++)
        {
            if (*it == Choice_Piano_MIDITrack_APPLYLAST->GetString(i))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Choice_Piano_MIDITrack_APPLYLAST->Append(*it);
        }
    }

    if (removed && Choice_Piano_MIDITrack_APPLYLAST->GetCount() > 0)
    {
        // go through the list and see if our selected item is there
        bool found = false;
        for (size_t i = 0; i < Choice_Piano_MIDITrack_APPLYLAST->GetCount(); i++)
        {
            if (selection == Choice_Piano_MIDITrack_APPLYLAST->GetString(i))
            {
                found = true;
                Choice_Piano_MIDITrack_APPLYLAST->SetSelection(i);
                break;
            }
        }
        if (!found)
        {
            Choice_Piano_MIDITrack_APPLYLAST->SetSelection(0);
        }
    }
    ValidateWindow();
}
