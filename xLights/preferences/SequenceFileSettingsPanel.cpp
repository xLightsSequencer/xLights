/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceFileSettingsPanel.h"

//(*InternalHeaders(SequenceFileSettingsPanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

#include <wx/preferences.h>
#include "../xLightsMain.h"

//(*IdInit(SequenceFileSettingsPanel)
const long SequenceFileSettingsPanel::ID_CHECKBOX1 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHECKBOX2 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHECKBOX3 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHOICE1 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHOICE2 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHOICE3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SequenceFileSettingsPanel,wxPanel)
	//(*EventTable(SequenceFileSettingsPanel)
	//*)
END_EVENT_TABLE()

SequenceFileSettingsPanel::SequenceFileSettingsPanel(wxWindow* parent,xLightsFrame *f,wxWindowID id,const wxPoint& pos,const wxSize& size) : frame(f)
{
	//(*Initialize(SequenceFileSettingsPanel)
	wxGridBagSizer* GridBagSizer1;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	RenderOnSaveCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Render on Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	RenderOnSaveCheckBox->SetValue(false);
	GridBagSizer1->Add(RenderOnSaveCheckBox, wxGBPosition(0, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FSEQSaveCheckBox = new wxCheckBox(this, ID_CHECKBOX2, _("Save FSEQ On Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	FSEQSaveCheckBox->SetValue(false);
	GridBagSizer1->Add(FSEQSaveCheckBox, wxGBPosition(1, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ModelBlendCheckBox = new wxCheckBox(this, ID_CHECKBOX3, _("Model Blending Default Off"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	ModelBlendCheckBox->SetValue(false);
	GridBagSizer1->Add(ModelBlendCheckBox, wxGBPosition(2, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Render Cache"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(3, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	RenderCacheChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	RenderCacheChoice->Append(_("Enabled"));
	RenderCacheChoice->SetSelection( RenderCacheChoice->Append(_("Locked Effects Only")) );
	RenderCacheChoice->Append(_("Disabled"));
	GridBagSizer1->Add(RenderCacheChoice, wxGBPosition(3, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Auto Save Interval"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText2, wxGBPosition(4, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	AutoSaveIntervalChoice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	AutoSaveIntervalChoice->Append(_("Disabled"));
	AutoSaveIntervalChoice->Append(_("3 Minutes"));
	AutoSaveIntervalChoice->SetSelection( AutoSaveIntervalChoice->Append(_("5 Minutes")) );
	AutoSaveIntervalChoice->Append(_("10 Minutes"));
	AutoSaveIntervalChoice->Append(_("15 Minutes"));
	AutoSaveIntervalChoice->Append(_("30 Minutes"));
	GridBagSizer1->Add(AutoSaveIntervalChoice, wxGBPosition(4, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("FSEQ Version"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText3, wxGBPosition(5, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FSEQVersionChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	FSEQVersionChoice->Append(_("V1"));
	FSEQVersionChoice->SetSelection( FSEQVersionChoice->Append(_("V2 ZSTD (Default)")) );
	FSEQVersionChoice->Append(_("V2 Uncompressed"));
	FSEQVersionChoice->Append(_("V2 ZLIB"));
	GridBagSizer1->Add(FSEQVersionChoice, wxGBPosition(5, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(GridBagSizer1);
	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);

	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnRenderOnSaveCheckBoxClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnFSEQSaveCheckBoxClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnModelBlendCheckBoxClick);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnRenderCacheChoiceSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnAutoSaveIntervalChoiceSelect);
	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnFSEQVersionChoiceSelect);
	//*)
}

SequenceFileSettingsPanel::~SequenceFileSettingsPanel()
{
	//(*Destroy(SequenceFileSettingsPanel)
	//*)
}

bool SequenceFileSettingsPanel::TransferDataFromWindow() {
    frame->SetSaveFSEQVersion(FSEQVersionChoice->GetSelection() + 1);
    frame->SetEnableRenderCache(RenderCacheChoice->GetStringSelection());
    frame->SetRenderOnSave(RenderOnSaveCheckBox->IsChecked());
    frame->SetSaveFseqOnSave(FSEQSaveCheckBox->IsChecked());
    switch (AutoSaveIntervalChoice->GetSelection()) {
        case 0:
            frame->SetAutoSaveInterval(0);
            break;
        case 1:
            frame->SetAutoSaveInterval(3);
            break;
        case 2:
            frame->SetAutoSaveInterval(5);
            break;
        case 3:
            frame->SetAutoSaveInterval(10);
            break;
        case 4:
            frame->SetAutoSaveInterval(15);
            break;
        case 5:
            frame->SetAutoSaveInterval(30);
            break;
        default:
            frame->SetAutoSaveInterval(5);
            break;
    }
    
    return true;
}
bool SequenceFileSettingsPanel::TransferDataToWindow() {
    FSEQVersionChoice->SetSelection(frame->SaveFSEQVersion() - 1);
    wxString rc = frame->EnableRenderCache();
    if (rc == "Locked Only") {
        rc = "Locked Effects Only";
    }
    RenderCacheChoice->SetStringSelection(rc);
    FSEQSaveCheckBox->SetValue(frame->SaveFseqOnSave());
    RenderOnSaveCheckBox->SetValue(frame->RenderOnSave());
    switch (frame->AutoSaveInterval()) {
        case 30:
            AutoSaveIntervalChoice->SetSelection(5);
            break;
        case 15:
            AutoSaveIntervalChoice->SetSelection(4);
            break;
        case 10:
            AutoSaveIntervalChoice->SetSelection(3);
            break;
        case 3:
            AutoSaveIntervalChoice->SetSelection(1);
            break;
        case 0:
            AutoSaveIntervalChoice->SetSelection(0);
            break;
        case 5:
        default:
            AutoSaveIntervalChoice->SetSelection(2);
            break;
    }
    
    return true;
}
void SequenceFileSettingsPanel::OnRenderOnSaveCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void SequenceFileSettingsPanel::OnFSEQSaveCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void SequenceFileSettingsPanel::OnModelBlendCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void SequenceFileSettingsPanel::OnRenderCacheChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void SequenceFileSettingsPanel::OnAutoSaveIntervalChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void SequenceFileSettingsPanel::OnFSEQVersionChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void SequenceFileSettingsPanel::OnRenderModeChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
