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
#include <wx/filepicker.h>
#include <wx/gbsizer.h>
#include <wx/intl.h>
#include <wx/sizer.h>
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
const long SequenceFileSettingsPanel::ID_CHECKBOX6 = wxNewId();
const long SequenceFileSettingsPanel::ID_DIRPICKERCTRL3 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHECKBOX5 = wxNewId();
const long SequenceFileSettingsPanel::ID_DIRPICKERCTRL2 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHECKBOX4 = wxNewId();
const long SequenceFileSettingsPanel::ID_DIRPICKERCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SequenceFileSettingsPanel,wxPanel)
	//(*EventTable(SequenceFileSettingsPanel)
	//*)
END_EVENT_TABLE()

SequenceFileSettingsPanel::SequenceFileSettingsPanel(wxWindow* parent,xLightsFrame *f,wxWindowID id,const wxPoint& pos,const wxSize& size) : frame(f)
{
	//(*Initialize(SequenceFileSettingsPanel)
	wxGridBagSizer* GridBagSizer1;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer2;
	wxStaticBoxSizer* StaticBoxSizer3;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	RenderOnSaveCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Render on Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	RenderOnSaveCheckBox->SetValue(false);
	GridBagSizer1->Add(RenderOnSaveCheckBox, wxGBPosition(0, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FSEQSaveCheckBox = new wxCheckBox(this, ID_CHECKBOX2, _("Save FSEQ On Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	FSEQSaveCheckBox->SetValue(false);
	GridBagSizer1->Add(FSEQSaveCheckBox, wxGBPosition(6, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ModelBlendCheckBox = new wxCheckBox(this, ID_CHECKBOX3, _("Model Blending Default Off"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	ModelBlendCheckBox->SetValue(false);
	GridBagSizer1->Add(ModelBlendCheckBox, wxGBPosition(1, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Render Cache"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	RenderCacheChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	RenderCacheChoice->Append(_("Enabled"));
	RenderCacheChoice->SetSelection( RenderCacheChoice->Append(_("Locked Effects Only")) );
	RenderCacheChoice->Append(_("Disabled"));
	GridBagSizer1->Add(RenderCacheChoice, wxGBPosition(2, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	GridBagSizer1->Add(StaticText3, wxGBPosition(7, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FSEQVersionChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	FSEQVersionChoice->Append(_("V1"));
	FSEQVersionChoice->SetSelection( FSEQVersionChoice->Append(_("V2 ZSTD (Default)")) );
	FSEQVersionChoice->Append(_("V2 Uncompressed"));
	FSEQVersionChoice->Append(_("V2 ZLIB"));
	GridBagSizer1->Add(FSEQVersionChoice, wxGBPosition(7, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer3 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Render Cache Directory"));
	CheckBox_RenderCache = new wxCheckBox(this, ID_CHECKBOX6, _("Use Show Folder"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
	CheckBox_RenderCache->SetValue(false);
	StaticBoxSizer3->Add(CheckBox_RenderCache, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DirPickerCtrl_RenderCache = new wxDirPickerCtrl(this, ID_DIRPICKERCTRL3, wxEmptyString, wxEmptyString, wxDefaultPosition, wxSize(400,-1), wxDIRP_DIR_MUST_EXIST|wxDIRP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_DIRPICKERCTRL3"));
	StaticBoxSizer3->Add(DirPickerCtrl_RenderCache, 1, wxALL|wxEXPAND, 5);
	GridBagSizer1->Add(StaticBoxSizer3, wxGBPosition(3, 0), wxGBSpan(1, 2), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("FSEQ Directory"));
	CheckBox_FSEQ = new wxCheckBox(this, ID_CHECKBOX5, _("Use Show Folder"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
	CheckBox_FSEQ->SetValue(false);
	StaticBoxSizer2->Add(CheckBox_FSEQ, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DirPickerCtrl_FSEQ = new wxDirPickerCtrl(this, ID_DIRPICKERCTRL2, wxEmptyString, wxEmptyString, wxDefaultPosition, wxSize(400,-1), wxDIRP_DIR_MUST_EXIST|wxDIRP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_DIRPICKERCTRL2"));
	StaticBoxSizer2->Add(DirPickerCtrl_FSEQ, 1, wxALL|wxEXPAND, 5);
	GridBagSizer1->Add(StaticBoxSizer2, wxGBPosition(8, 0), wxGBSpan(1, 2), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Media Directory"));
	CheckBox_Media = new wxCheckBox(this, ID_CHECKBOX4, _("Use Show Folder"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	CheckBox_Media->SetValue(false);
	StaticBoxSizer1->Add(CheckBox_Media, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DirPickerCtrl_Media = new wxDirPickerCtrl(this, ID_DIRPICKERCTRL1, wxEmptyString, wxEmptyString, wxDefaultPosition, wxSize(400,-1), wxDIRP_DIR_MUST_EXIST|wxDIRP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_DIRPICKERCTRL1"));
	StaticBoxSizer1->Add(DirPickerCtrl_Media, 1, wxALL|wxEXPAND, 5);
	GridBagSizer1->Add(StaticBoxSizer1, wxGBPosition(5, 0), wxGBSpan(1, 2), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(GridBagSizer1);
	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);

	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnRenderOnSaveCheckBoxClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnFSEQSaveCheckBoxClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnModelBlendCheckBoxClick);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnRenderCacheChoiceSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnAutoSaveIntervalChoiceSelect);
	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnFSEQVersionChoiceSelect);
	Connect(ID_CHECKBOX6,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnCheckBox_RenderCacheClick);
	Connect(ID_DIRPICKERCTRL3,wxEVT_COMMAND_DIRPICKER_CHANGED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnDirPickerCtrl_RenderCacheDirChanged);
	Connect(ID_CHECKBOX5,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnCheckBox_FSEQClick);
	Connect(ID_DIRPICKERCTRL2,wxEVT_COMMAND_DIRPICKER_CHANGED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnDirPickerCtrl_FSEQDirChanged);
	Connect(ID_CHECKBOX4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnCheckBox_MediaClick);
	Connect(ID_DIRPICKERCTRL1,wxEVT_COMMAND_DIRPICKER_CHANGED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnDirPickerCtrl_MediaDirChanged);
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

    frame->SetMediaFolder(CheckBox_Media->GetValue(), DirPickerCtrl_Media->GetPath());
    frame->SetFSEQFolder(CheckBox_FSEQ->GetValue(), DirPickerCtrl_FSEQ->GetPath());
    frame->SetRenderCacheFolder(CheckBox_RenderCache->GetValue(), DirPickerCtrl_RenderCache->GetPath());

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

    bool cb;
    std::string folder;
    frame->GetMediaFolder(cb, folder);
    CheckBox_Media->SetValue(cb);
    DirPickerCtrl_Media->SetPath(folder);

    frame->GetFSEQFolder(cb, folder);
    CheckBox_FSEQ->SetValue(cb);
    DirPickerCtrl_FSEQ->SetPath(folder);

    frame->GetRenderCacheFolder(cb, folder);
    CheckBox_RenderCache->SetValue(cb);
    DirPickerCtrl_RenderCache->SetPath(folder);

    ValidateWindow();

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

bool SequenceFileSettingsPanel::ValidateWindow()
{
    bool res = true;

    if (CheckBox_Media->GetValue()) {
        DirPickerCtrl_Media->Enable(false);
    }
    else {
        if (!wxDir::Exists(DirPickerCtrl_Media->GetPath())) res = false;
        DirPickerCtrl_Media->Enable(true);
    }

    if (CheckBox_FSEQ->GetValue()) {
        DirPickerCtrl_FSEQ->Enable(false);
    }
    else {
        if (!wxDir::Exists(DirPickerCtrl_FSEQ->GetPath())) res = false;
        DirPickerCtrl_FSEQ->Enable(true);
    }

    if (CheckBox_RenderCache->GetValue()) {
        DirPickerCtrl_RenderCache->Enable(false);
    }
    else {
        if (!wxDir::Exists(DirPickerCtrl_RenderCache->GetPath())) res = false;
        DirPickerCtrl_RenderCache->Enable(true);
    }

    return res;
}
void SequenceFileSettingsPanel::OnCheckBox_RenderCacheClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
    else {
        ValidateWindow();
    }
}

void SequenceFileSettingsPanel::OnCheckBox_MediaClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
    else {
        ValidateWindow();
    }
}

void SequenceFileSettingsPanel::OnCheckBox_FSEQClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
    else {
        ValidateWindow();
    }
}

void SequenceFileSettingsPanel::OnDirPickerCtrl_RenderCacheDirChanged(wxFileDirPickerEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void SequenceFileSettingsPanel::OnDirPickerCtrl_MediaDirChanged(wxFileDirPickerEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void SequenceFileSettingsPanel::OnDirPickerCtrl_FSEQDirChanged(wxFileDirPickerEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
