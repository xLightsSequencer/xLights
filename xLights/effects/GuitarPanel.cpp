/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(GuitarPanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

#include <wx/filename.h>
#include <wx/valnum.h>

#include "GuitarPanel.h"
#include "EffectPanelUtils.h"
#include "../sequencer/Effect.h"
#include "GuitarEffect.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>

//(*IdInit(GuitarPanel)
const long GuitarPanel::ID_STATICTEXT_Guitar_Type = wxNewId();
const long GuitarPanel::ID_CHOICE_Guitar_Type = wxNewId();
const long GuitarPanel::ID_STATICTEXT_Guitar_MIDITrack_APPLYLAST = wxNewId();
const long GuitarPanel::ID_CHOICE_Guitar_MIDITrack_APPLYLAST = wxNewId();
const long GuitarPanel::ID_STATICTEXT1 = wxNewId();
const long GuitarPanel::ID_CHOICE_StringAppearance = wxNewId();
const long GuitarPanel::ID_STATICTEXT_Piano_Scale = wxNewId();
const long GuitarPanel::ID_SLIDER_MaxFrets = wxNewId();
const long GuitarPanel::IDD_TEXTCTRL_MaxFrets = wxNewId();
const long GuitarPanel::ID_STATICTEXT2 = wxNewId();
const long GuitarPanel::ID_SLIDER_BaseWaveFactor = wxNewId();
const long GuitarPanel::IDD_TEXTCTRL_BaseWaveFactor = wxNewId();
const long GuitarPanel::ID_STATICTEXT3 = wxNewId();
const long GuitarPanel::ID_SLIDER_StringWaveFactor = wxNewId();
const long GuitarPanel::IDD_TEXTCTRL_StringWaveFactor = wxNewId();
const long GuitarPanel::ID_CHECKBOX_Fade = wxNewId();
const long GuitarPanel::ID_CHECKBOX_Collapse = wxNewId();
const long GuitarPanel::ID_CHECKBOX_ShowStrings = wxNewId();
const long GuitarPanel::ID_CHECKBOX_VaryWaveLengthOnFret = wxNewId();
//*)

BEGIN_EVENT_TABLE(GuitarPanel,wxPanel)
	//(*EventTable(GuitarPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_SETTIMINGTRACKS, GuitarPanel::SetTimingTracks)
END_EVENT_TABLE()

GuitarPanel::GuitarPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(GuitarPanel)
	wxFlexGridSizer* FlexGridSizer27;
	wxFlexGridSizer* FlexGridSizer42;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer42->AddGrowableCol(0);
	FlexGridSizer27 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer27->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Guitar_Type, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Guitar_Type"));
	FlexGridSizer27->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Guitar_Type = new BulkEditChoice(this, ID_CHOICE_Guitar_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Guitar_Type"));
	Choice_Guitar_Type->SetSelection( Choice_Guitar_Type->Append(_("Guitar")) );
	Choice_Guitar_Type->Append(_("Bass Guitar"));
	Choice_Guitar_Type->Append(_("Banjo"));
	FlexGridSizer27->Add(Choice_Guitar_Type, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT_Guitar_MIDITrack_APPLYLAST, _("Track"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Guitar_MIDITrack_APPLYLAST"));
	FlexGridSizer27->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Guitar_MIDITrack_APPLYLAST = new BulkEditChoice(this, ID_CHOICE_Guitar_MIDITrack_APPLYLAST, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Guitar_MIDITrack_APPLYLAST"));
	FlexGridSizer27->Add(Choice_Guitar_MIDITrack_APPLYLAST, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT1, _("String Appearance"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer27->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Choice_StringAppearance = new wxChoice(this, ID_CHOICE_StringAppearance, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_StringAppearance"));
	Choice_StringAppearance->SetSelection( Choice_StringAppearance->Append(_("On")) );
	Choice_StringAppearance->Append(_("Wave"));
	FlexGridSizer27->Add(Choice_StringAppearance, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT_Piano_Scale, _("Fret Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_Scale"));
	FlexGridSizer27->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_MaxFrets = new BulkEditSlider(this, ID_SLIDER_MaxFrets, 19, 8, 30, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MaxFrets"));
	FlexGridSizer27->Add(Slider_MaxFrets, 1, wxALL|wxEXPAND, 2);
	TextCtrl_MaxFrets = new BulkEditTextCtrl(this, IDD_TEXTCTRL_MaxFrets, _("19"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_MaxFrets"));
	FlexGridSizer27->Add(TextCtrl_MaxFrets, 1, wxALL|wxEXPAND, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT2, _("Base Wavelength"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer27->Add(StaticText3, 1, wxALL|wxEXPAND, 2);
	Slider_BaseWaveFactor = new BulkEditSliderF1(this, ID_SLIDER_BaseWaveFactor, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BaseWaveFactor"));
	FlexGridSizer27->Add(Slider_BaseWaveFactor, 1, wxALL|wxEXPAND, 2);
	TextCtrl_BaseWaveFactor = new BulkEditTextCtrlF1(this, IDD_TEXTCTRL_BaseWaveFactor, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_BaseWaveFactor"));
	TextCtrl_BaseWaveFactor->SetMaxLength(5);
	FlexGridSizer27->Add(TextCtrl_BaseWaveFactor, 1, wxALL|wxEXPAND, 2);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT3, _("Vary Wavelength By String"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer27->Add(StaticText4, 1, wxALL|wxEXPAND, 2);
	Slider_StringWaveFactor = new BulkEditSliderF1(this, ID_SLIDER_StringWaveFactor, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_StringWaveFactor"));
	FlexGridSizer27->Add(Slider_StringWaveFactor, 1, wxALL|wxEXPAND, 2);
	TextCtrl_StringWaveFactor = new BulkEditTextCtrlF1(this, IDD_TEXTCTRL_StringWaveFactor, _("0.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_StringWaveFactor"));
	TextCtrl_StringWaveFactor->SetMaxLength(5);
	FlexGridSizer27->Add(TextCtrl_StringWaveFactor, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fade = new BulkEditCheckBox(this, ID_CHECKBOX_Fade, _("Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fade"));
	CheckBox_Fade->SetValue(false);
	FlexGridSizer27->Add(CheckBox_Fade, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	CheckBox_Collapse = new BulkEditCheckBox(this, ID_CHECKBOX_Collapse, _("Collapse"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Collapse"));
	CheckBox_Collapse->SetValue(false);
	FlexGridSizer27->Add(CheckBox_Collapse, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	CheckBox_ShowStrings = new BulkEditCheckBox(this, ID_CHECKBOX_ShowStrings, _("Show Strings"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ShowStrings"));
	CheckBox_ShowStrings->SetValue(false);
	FlexGridSizer27->Add(CheckBox_ShowStrings, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	CheckBox_VaryWaveLengthOnFret = new BulkEditCheckBox(this, ID_CHECKBOX_VaryWaveLengthOnFret, _("Vary Wavelength By Fret"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_VaryWaveLengthOnFret"));
	CheckBox_VaryWaveLengthOnFret->SetValue(true);
	FlexGridSizer27->Add(CheckBox_VaryWaveLengthOnFret, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	FlexGridSizer42->Add(FlexGridSizer27, 0, wxEXPAND, 2);
	SetSizer(FlexGridSizer42);
	FlexGridSizer42->Fit(this);
	FlexGridSizer42->SetSizeHints(this);
	//*)

    SetName("ID_PANEL_Guitar");

	Choice_Guitar_Type->Append(_("Violin"));

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&GuitarPanel::OnVCChanged, nullptr, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&GuitarPanel::OnValidateWindow, 0, this);

	ValidateWindow();
}

GuitarPanel::~GuitarPanel()
{
	//(*Destroy(GuitarPanel)
	//*)
}

void GuitarPanel::ValidateWindow()
{
    if (Choice_Guitar_MIDITrack_APPLYLAST->GetCount() > 0) {
        Choice_Guitar_MIDITrack_APPLYLAST->Enable(true);
    } else {
        Choice_Guitar_MIDITrack_APPLYLAST->Enable(false);
    }

	if (Choice_StringAppearance->GetStringSelection() == "Wave")
	{
        Slider_BaseWaveFactor->Enable();
        TextCtrl_BaseWaveFactor->Enable();
        Slider_StringWaveFactor->Enable();
        TextCtrl_StringWaveFactor->Enable();
        CheckBox_VaryWaveLengthOnFret->Enable();
	}
	else
	{
        Slider_BaseWaveFactor->Disable();
        TextCtrl_BaseWaveFactor->Disable();
        Slider_StringWaveFactor->Disable();
        TextCtrl_StringWaveFactor->Disable();
        CheckBox_VaryWaveLengthOnFret->Disable();
    }
}

void GuitarPanel::SetTimingTracks(wxCommandEvent& event)
{
    auto timingtracks = wxSplit(event.GetString(), '|');

    wxString selection = Choice_Guitar_MIDITrack_APPLYLAST->GetStringSelection();

    // check if anything has been removed ... if it has clear the list and we will have to rebuild it as you cant delete items from a combo box
    bool removed = false;
    for (size_t i = 0; i < Choice_Guitar_MIDITrack_APPLYLAST->GetCount(); i++)
    {
        bool found = false;
        for (auto it = timingtracks.begin(); it != timingtracks.end(); ++it)
        {
            if (*it == Choice_Guitar_MIDITrack_APPLYLAST->GetString(i))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Choice_Guitar_MIDITrack_APPLYLAST->Clear();
            removed = true;
            break;
        }
    }

    // add any new timing tracks
    for (auto it = timingtracks.begin(); it != timingtracks.end(); ++it)
    {
        bool found = false;
        for (size_t i = 0; i < Choice_Guitar_MIDITrack_APPLYLAST->GetCount(); i++)
        {
            if (*it == Choice_Guitar_MIDITrack_APPLYLAST->GetString(i))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Choice_Guitar_MIDITrack_APPLYLAST->Append(*it);
        }
    }

    if (removed && Choice_Guitar_MIDITrack_APPLYLAST->GetCount() > 0)
    {
        // go through the list and see if our selected item is there
        bool found = false;
        for (size_t i = 0; i < Choice_Guitar_MIDITrack_APPLYLAST->GetCount(); i++)
        {
            if (selection == Choice_Guitar_MIDITrack_APPLYLAST->GetString(i))
            {
                found = true;
                Choice_Guitar_MIDITrack_APPLYLAST->SetSelection(i);
                break;
            }
        }
        if (!found)
        {
            Choice_Guitar_MIDITrack_APPLYLAST->SetSelection(0);
        }
    }
    ValidateWindow();
}

void GuitarPanel::OnChoice_StringAppearanceSelect(wxCommandEvent& event)
{
    ValidateWindow();
}
