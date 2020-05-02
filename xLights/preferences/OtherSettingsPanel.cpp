/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "OtherSettingsPanel.h"

//(*InternalHeaders(OtherSettingsPanel)
#include <wx/checkbox.h>
#include <wx/gbsizer.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

#include <wx/preferences.h>
#include "../xLightsMain.h"

//(*IdInit(OtherSettingsPanel)
const long OtherSettingsPanel::ID_TEXTCTRL1 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX1 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX2 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(OtherSettingsPanel,wxPanel)
	//(*EventTable(OtherSettingsPanel)
	//*)
END_EVENT_TABLE()

OtherSettingsPanel::OtherSettingsPanel(wxWindow* parent,xLightsFrame *f,wxWindowID id,const wxPoint& pos,const wxSize& size) : frame(f)
{
	//(*Initialize(OtherSettingsPanel)
	wxGridBagSizer* GridBagSizer1;
	wxGridBagSizer* GridBagSizer2;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticText* StaticText1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("eMail Address:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	eMailTextControl = new wxTextCtrl(this, ID_TEXTCTRL1, _("noone@nowhere.xlights.org"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(150,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	GridBagSizer1->Add(eMailTextControl, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxEXPAND, 5);
	HardwareVideoDecodingCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Hardware Video Decoding"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	HardwareVideoDecodingCheckBox->SetValue(false);
	GridBagSizer1->Add(HardwareVideoDecodingCheckBox, wxGBPosition(1, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Packaging Sequences"));
	GridBagSizer2 = new wxGridBagSizer(0, 0);
	ExcludePresetsCheckBox = new wxCheckBox(this, ID_CHECKBOX2, _("Exclude Presets"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	ExcludePresetsCheckBox->SetValue(false);
	GridBagSizer2->Add(ExcludePresetsCheckBox, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ExcludeAudioCheckBox = new wxCheckBox(this, ID_CHECKBOX3, _("Exclude Audio"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	ExcludeAudioCheckBox->SetValue(false);
	GridBagSizer2->Add(ExcludeAudioCheckBox, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1->Add(GridBagSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	GridBagSizer1->Add(StaticBoxSizer1, wxGBPosition(2, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT, 0);
	SetSizer(GridBagSizer1);
	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&OtherSettingsPanel::OneMailTextControlTextEnter);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&OtherSettingsPanel::OneMailTextControlTextEnter);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&OtherSettingsPanel::OnHardwareVideoDecodingCheckBoxClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&OtherSettingsPanel::OnExcludePresetsCheckBoxClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&OtherSettingsPanel::OnExcludeAudioCheckBoxClick);
	//*)
    
    
#ifdef __LINUX__
    HardwareVideoDecodingCheckBox->Hide();
#endif
}

OtherSettingsPanel::~OtherSettingsPanel()
{
	//(*Destroy(OtherSettingsPanel)
	//*)
}
bool OtherSettingsPanel::TransferDataFromWindow() {
    frame->SetExcludeAudioFromPackagedSequences(ExcludeAudioCheckBox->IsChecked());
    frame->SetExcludePresetsFromPackagedSequences(ExcludePresetsCheckBox->IsChecked());
    frame->SetHardwareVideoAccelerated(HardwareVideoDecodingCheckBox->IsChecked());
    frame->SetUserEMAIL(eMailTextControl->GetValue());
    return true;
}
bool OtherSettingsPanel::TransferDataToWindow() {
    ExcludeAudioCheckBox->SetValue(frame->ExcludeAudioFromPackagedSequences());
    ExcludePresetsCheckBox->SetValue(frame->ExcludePresetsFromPackagedSequences());
    HardwareVideoDecodingCheckBox->SetValue(frame->HardwareVideoAccelerated());
    eMailTextControl->SetValue(frame->UserEMAIL());
    return true;
}


void OtherSettingsPanel::OnExcludeAudioCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void OtherSettingsPanel::OnExcludePresetsCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void OtherSettingsPanel::OnHardwareVideoDecodingCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void OtherSettingsPanel::OneMailTextControlTextEnter(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
