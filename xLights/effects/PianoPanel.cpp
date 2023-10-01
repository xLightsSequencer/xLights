/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(PianoPanel)
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

#include <wx/filename.h>
#include <wx/valnum.h>

#include "PianoPanel.h"
#include "EffectPanelUtils.h"
#include "../sequencer/Effect.h"
#include "PianoEffect.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>

//(*IdInit(PianoPanel)
const long PianoPanel::ID_STATICTEXT_Piano_Type = wxNewId();
const long PianoPanel::ID_CHOICE_Piano_Type = wxNewId();
const long PianoPanel::ID_STATICTEXT_Piano_StartMIDI = wxNewId();
const long PianoPanel::ID_SPINCTRL_Piano_StartMIDI = wxNewId();
const long PianoPanel::ID_STATICTEXT_Piano_EndMIDI = wxNewId();
const long PianoPanel::ID_SPINCTRL_Piano_EndMIDI = wxNewId();
const long PianoPanel::ID_CHECKBOX_Piano_ShowSharps = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_ShowSharps = wxNewId();
const long PianoPanel::ID_CHECKBOX_Piano_FadeNotes = wxNewId();
const long PianoPanel::ID_STATICTEXT_Piano_MIDITrack_APPLYLAST = wxNewId();
const long PianoPanel::ID_CHOICE_Piano_MIDITrack_APPLYLAST = wxNewId();
const long PianoPanel::ID_STATICTEXT_Piano_Scale = wxNewId();
const long PianoPanel::ID_SLIDER_Piano_Scale = wxNewId();
const long PianoPanel::ID_VALUECURVE_Piano_Scale = wxNewId();
const long PianoPanel::IDD_TEXTCTRL_Piano_Scale = wxNewId();
const long PianoPanel::ID_STATICTEXT_Piano_XOffset = wxNewId();
const long PianoPanel::ID_SLIDER_Piano_XOffset = wxNewId();
const long PianoPanel::IDD_TEXTCTRL_Piano_XOffset = wxNewId();
//*)

BEGIN_EVENT_TABLE(PianoPanel,wxPanel)
	//(*EventTable(PianoPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_SETTIMINGTRACKS, PianoPanel::SetTimingTracks)
END_EVENT_TABLE()

PianoPanel::PianoPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(PianoPanel)
	wxFlexGridSizer* FlexGridSizer27;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer42;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer42->AddGrowableCol(0);
	FlexGridSizer27 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer27->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Piano_Type, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_Type"));
	FlexGridSizer27->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Piano_Type = new BulkEditChoice(this, ID_CHOICE_Piano_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Piano_Type"));
	Choice_Piano_Type->SetSelection( Choice_Piano_Type->Append(_("True Piano")) );
	Choice_Piano_Type->Append(_("Bars"));
	FlexGridSizer27->Add(Choice_Piano_Type, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT_Piano_StartMIDI, _("Start MIDI Channel [0-127]"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_StartMIDI"));
	FlexGridSizer27->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_Piano_StartMIDI = new BulkEditSpinCtrl(this, ID_SPINCTRL_Piano_StartMIDI, _T("60"), wxDefaultPosition, wxDefaultSize, 0, 0, 127, 60, _T("ID_SPINCTRL_Piano_StartMIDI"));
	SpinCtrl_Piano_StartMIDI->SetValue(_T("60"));
	FlexGridSizer27->Add(SpinCtrl_Piano_StartMIDI, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	StaticText_Piano_NumKeys = new wxStaticText(this, ID_STATICTEXT_Piano_EndMIDI, _("End MIDI Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_EndMIDI"));
	FlexGridSizer27->Add(StaticText_Piano_NumKeys, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_Piano_EndMIDI = new BulkEditSpinCtrl(this, ID_SPINCTRL_Piano_EndMIDI, _T("72"), wxDefaultPosition, wxDefaultSize, 0, 0, 127, 72, _T("ID_SPINCTRL_Piano_EndMIDI"));
	SpinCtrl_Piano_EndMIDI->SetValue(_T("72"));
	FlexGridSizer27->Add(SpinCtrl_Piano_EndMIDI, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	CheckBox_Piano_ShowSharps = new BulkEditCheckBox(this, ID_CHECKBOX_Piano_ShowSharps, _("Show Sharps and Flats"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Piano_ShowSharps"));
	CheckBox_Piano_ShowSharps->SetValue(true);
	FlexGridSizer27->Add(CheckBox_Piano_ShowSharps, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(0,0,1, wxALL|wxEXPAND, 5);
	BitmapButton_Piano_ShowSharps = new xlLockButton(this, ID_BITMAPBUTTON_Piano_ShowSharps, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_ShowSharps"));
	BitmapButton_Piano_ShowSharps->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer27->Add(BitmapButton_Piano_ShowSharps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_FadeNotes = new BulkEditCheckBox(this, ID_CHECKBOX_Piano_FadeNotes, _("Fade notes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Piano_FadeNotes"));
	CheckBox_FadeNotes->SetValue(false);
	FlexGridSizer27->Add(CheckBox_FadeNotes, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(0,0,1, wxALL|wxEXPAND, 5);
	FlexGridSizer27->Add(0,0,1, wxALL|wxEXPAND, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT_Piano_MIDITrack_APPLYLAST, _("Track"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_MIDITrack_APPLYLAST"));
	FlexGridSizer27->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Piano_MIDITrack_APPLYLAST = new BulkEditChoice(this, ID_CHOICE_Piano_MIDITrack_APPLYLAST, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Piano_MIDITrack_APPLYLAST"));
	FlexGridSizer27->Add(Choice_Piano_MIDITrack_APPLYLAST, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	FlexGridSizer42->Add(FlexGridSizer27, 0, wxEXPAND, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT_Piano_Scale, _("Vertical Scale"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_Scale"));
	FlexGridSizer3->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_Piano_Scale = new BulkEditSlider(this, ID_SLIDER_Piano_Scale, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Piano_Scale"));
	FlexGridSizer4->Add(Slider_Piano_Scale, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Piano_ScaleVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Piano_Scale, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Piano_Scale"));
	FlexGridSizer4->Add(BitmapButton_Piano_ScaleVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Piano_Scale = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Piano_Scale, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Piano_Scale"));
	FlexGridSizer3->Add(TextCtrl_Piano_Scale, 1, wxALL, 2);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT_Piano_XOffset, _("Horizontal Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_XOffset"));
	FlexGridSizer3->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Piano_XOffset = new BulkEditSlider(this, ID_SLIDER_Piano_XOffset, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Piano_XOffset"));
	FlexGridSizer3->Add(Slider_Piano_XOffset, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Piano_XOffset = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Piano_XOffset, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Piano_XOffset"));
	FlexGridSizer3->Add(TextCtrl_Piano_XOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer42->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer42);
	FlexGridSizer42->Fit(this);
	FlexGridSizer42->SetSizeHints(this);

	Connect(ID_SPINCTRL_Piano_StartMIDI,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&PianoPanel::OnSpinCtrl_Piano_StartMIDIChange);
	Connect(ID_SPINCTRL_Piano_EndMIDI,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&PianoPanel::OnSpinCtrl_Piano_EndMIDIChange);
	Connect(ID_VALUECURVE_Piano_Scale,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnVCButtonClick);
	//*)

    SetName("ID_PANEL_PIANO");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&PianoPanel::OnVCChanged, nullptr, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&PianoPanel::OnValidateWindow, 0, this);
    
    // Set value curve limits
    BitmapButton_Piano_ScaleVC->GetValue()->SetLimits(PIANO_SCALE_MIN, PIANO_SCALE_MAX);

	ValidateWindow();
}

PianoPanel::~PianoPanel()
{
	//(*Destroy(PianoPanel)
	//*)
}

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

    SpinCtrl_Piano_StartMIDI->SetToolTip(wxString(DecodeMidi(SpinCtrl_Piano_StartMIDI->GetValue()).c_str()));
    SpinCtrl_Piano_EndMIDI->SetToolTip(wxString(DecodeMidi(SpinCtrl_Piano_EndMIDI->GetValue()).c_str()));
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

	if (Choice_Piano_MIDITrack_APPLYLAST->GetCount() > 0 && Choice_Piano_MIDITrack_APPLYLAST->GetSelection() == -1)
	{
        Choice_Piano_MIDITrack_APPLYLAST->SetSelection(0);
	}

    ValidateWindow();
}
