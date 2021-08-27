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
#include <wx/choice.h>
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
const long OtherSettingsPanel::ID_STATICTEXT1 = wxNewId();
const long OtherSettingsPanel::ID_CHOICE1 = wxNewId();
const long OtherSettingsPanel::ID_STATICTEXT2 = wxNewId();
const long OtherSettingsPanel::ID_TEXTCTRL1 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX1 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX2 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX3 = wxNewId();
const long OtherSettingsPanel::ID_CHOICE2 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX4 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX_WARN_GROUP_ISSUES = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX5 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX6 = wxNewId();
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
	StaticText2 = new wxStaticText(this, ID_STATICTEXT1, _("Link save:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	GridBagSizer1->Add(StaticText2, wxGBPosition(3, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	Choice_LinkSave = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_LinkSave->SetSelection( Choice_LinkSave->Append(_("None")) );
	Choice_LinkSave->Append(_("Controllers and Layout Tab"));
	GridBagSizer1->Add(Choice_LinkSave, wxGBPosition(3, 1), wxDefaultSpan, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT2, _("Link controller upload:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	GridBagSizer1->Add(StaticText3, wxGBPosition(4, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("eMail Address:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	eMailTextControl = new wxTextCtrl(this, ID_TEXTCTRL1, _("noone@nowhere.xlights.org"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(150,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	GridBagSizer1->Add(eMailTextControl, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxEXPAND, 5);
	HardwareVideoDecodingCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Hardware Video Decoding"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	HardwareVideoDecodingCheckBox->SetValue(false);
	GridBagSizer1->Add(HardwareVideoDecodingCheckBox, wxGBPosition(1, 0), wxGBSpan(1, 2), wxALL|wxEXPAND, 5);
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
	Choice_LinkControllerUpload = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	Choice_LinkControllerUpload->SetSelection( Choice_LinkControllerUpload->Append(_("None")) );
	Choice_LinkControllerUpload->Append(_("Inputs and Outputs"));
	GridBagSizer1->Add(Choice_LinkControllerUpload, wxGBPosition(4, 1), wxDefaultSpan, wxALL|wxEXPAND, 5);
	CheckBox_BatchRenderPromptIssues = new wxCheckBox(this, ID_CHECKBOX4, _("Prompt issues during batch render"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	CheckBox_BatchRenderPromptIssues->SetValue(true);
	GridBagSizer1->Add(CheckBox_BatchRenderPromptIssues, wxGBPosition(5, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_WarnGroupIssues = new wxCheckBox(this, ID_CHECKBOX_WARN_GROUP_ISSUES, _("Warn for Missing Model in Groups"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_WARN_GROUP_ISSUES"));
	CheckBox_WarnGroupIssues->SetValue(true);
	GridBagSizer1->Add(CheckBox_WarnGroupIssues, wxGBPosition(6, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_IgnoreVendorModelRecommendations = new wxCheckBox(this, ID_CHECKBOX5, _("Ignore vendor model recommendations"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
	CheckBox_IgnoreVendorModelRecommendations->SetValue(false);
	GridBagSizer1->Add(CheckBox_IgnoreVendorModelRecommendations, wxGBPosition(7, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	CheckBox_PurgeDownloadCache = new wxCheckBox(this, ID_CHECKBOX6, _("Purge download cache at startup"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
	CheckBox_PurgeDownloadCache->SetValue(false);
	GridBagSizer1->Add(CheckBox_PurgeDownloadCache, wxGBPosition(8, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	SetSizer(GridBagSizer1);
	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&OtherSettingsPanel::OnChoice_LinkSaveSelect);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&OtherSettingsPanel::OneMailTextControlTextEnter);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&OtherSettingsPanel::OneMailTextControlTextEnter);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&OtherSettingsPanel::OnHardwareVideoDecodingCheckBoxClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&OtherSettingsPanel::OnExcludePresetsCheckBoxClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&OtherSettingsPanel::OnExcludeAudioCheckBoxClick);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&OtherSettingsPanel::OnChoice_LinkControllerUploadSelect);
	Connect(ID_CHECKBOX4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&OtherSettingsPanel::OnCheckBox_BatchRenderPromptIssuesClick);
	Connect(ID_CHECKBOX_WARN_GROUP_ISSUES,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&OtherSettingsPanel::OnCheckBox_WarnGroupIssuesClick);
	Connect(ID_CHECKBOX5,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&OtherSettingsPanel::OnCheckBox_IgnoreVendorModelRecommendationsClick);
	Connect(ID_CHECKBOX6,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&OtherSettingsPanel::OnCheckBox_PurgeDownloadCacheClick);
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
	frame->SetLinkedSave(Choice_LinkSave->GetStringSelection());
	frame->SetLinkedControllerUpload(Choice_LinkControllerUpload->GetStringSelection());
	frame->SetPromptBatchRenderIssues(CheckBox_BatchRenderPromptIssues->GetValue());
	frame->SetWarnGroupIssues(CheckBox_WarnGroupIssues->GetValue());
	frame->SetIgnoreVendorModelRecommendations(CheckBox_IgnoreVendorModelRecommendations->GetValue());
	frame->SetPurgeDownloadCacheOnStart(CheckBox_PurgeDownloadCache->GetValue());
    return true;
}

bool OtherSettingsPanel::TransferDataToWindow() {
    ExcludeAudioCheckBox->SetValue(frame->ExcludeAudioFromPackagedSequences());
    ExcludePresetsCheckBox->SetValue(frame->ExcludePresetsFromPackagedSequences());
    HardwareVideoDecodingCheckBox->SetValue(frame->HardwareVideoAccelerated());
    eMailTextControl->SetValue(frame->UserEMAIL());
	Choice_LinkSave->SetStringSelection(frame->LinkedSave());
	Choice_LinkControllerUpload->SetStringSelection(frame->LinkedControllerUpload());
	CheckBox_BatchRenderPromptIssues->SetValue(frame->GetPromptBatchRenderIssues());
	CheckBox_WarnGroupIssues->SetValue(frame->GetWarnGroupIssues());
	CheckBox_IgnoreVendorModelRecommendations->SetValue(frame->GetIgnoreVendorModelRecommendations());
	CheckBox_PurgeDownloadCache->SetValue(frame->GetPurgeDownloadCacheOnStart());

// Remove attempt to sneak functionality into the windows build
#ifndef __WXMSW__
#ifndef IGNORE_VENDORS
    CheckBox_IgnoreVendorModelRecommendations->SetValue(false);
    CheckBox_IgnoreVendorModelRecommendations->Hide();
    CheckBox_WarnGroupIssues->SetValue(true);
    CheckBox_WarnGroupIssues->Hide();
#endif
#endif
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

void OtherSettingsPanel::OnChoice_LinkSaveSelect(wxCommandEvent& event)
{
	if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
		TransferDataFromWindow();
	}
}

void OtherSettingsPanel::OnChoice_LinkControllerUploadSelect(wxCommandEvent& event)
{
	if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
		TransferDataFromWindow();
	}
}

void OtherSettingsPanel::OnCheckBox_WarnGroupIssuesClick(wxCommandEvent& event)
{
	if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
		TransferDataFromWindow();
	}
}

void OtherSettingsPanel::OnCheckBox_BatchRenderPromptIssuesClick(wxCommandEvent& event)
{
	if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
		TransferDataFromWindow();
	}
}

void OtherSettingsPanel::OnCheckBox_IgnoreVendorModelRecommendationsClick(wxCommandEvent& event)
{
	if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
		TransferDataFromWindow();
	}
}

void OtherSettingsPanel::OnCheckBox_PurgeDownloadCacheClick(wxCommandEvent& event)
{
	if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
		TransferDataFromWindow();
	}
}
