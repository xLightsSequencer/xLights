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
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/gbsizer.h>
#include <wx/intl.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

#include <wx/preferences.h>
#include "../xLightsMain.h"

//(*IdInit(SequenceFileSettingsPanel)
const long SequenceFileSettingsPanel::ID_CHECKBOX1 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHECKBOX3 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHECKBOX2 = wxNewId();
const long SequenceFileSettingsPanel::ID_STATICTEXT1 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHOICE4 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHOICE1 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHOICE2 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHOICE3 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHECKBOX6 = wxNewId();
const long SequenceFileSettingsPanel::ID_DIRPICKERCTRL3 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHECKBOX5 = wxNewId();
const long SequenceFileSettingsPanel::ID_DIRPICKERCTRL2 = wxNewId();
const long SequenceFileSettingsPanel::ID_LISTBOX_MEDIA = wxNewId();
const long SequenceFileSettingsPanel::ID_BUTTON_ADDMEDIA = wxNewId();
const long SequenceFileSettingsPanel::ID_BUTTON_REMOVE_MEDIA = wxNewId();
const long SequenceFileSettingsPanel::ID_STATICTEXT2 = wxNewId();
const long SequenceFileSettingsPanel::ID_CHOICE_VIEW_DEFAULT = wxNewId();
//*)

BEGIN_EVENT_TABLE(SequenceFileSettingsPanel,wxPanel)
	//(*EventTable(SequenceFileSettingsPanel)
	//*)
END_EVENT_TABLE()

SequenceFileSettingsPanel::SequenceFileSettingsPanel(wxWindow* parent,xLightsFrame *f,wxWindowID id,const wxPoint& pos,const wxSize& size) : frame(f)
{
	//(*Initialize(SequenceFileSettingsPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
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
	CheckBox_LowDefinitionRender = new wxCheckBox(this, ID_CHECKBOX3, _("Low Definition Render"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_LowDefinitionRender->SetValue(false);
	GridBagSizer1->Add(CheckBox_LowDefinitionRender, wxGBPosition(1, 0), wxGBSpan(1, 2), wxALL|wxEXPAND, 5);
	FSEQSaveCheckBox = new wxCheckBox(this, ID_CHECKBOX2, _("Save FSEQ File On Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	FSEQSaveCheckBox->SetValue(false);
	GridBagSizer1->Add(FSEQSaveCheckBox, wxGBPosition(9, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT1, _("Default Model Blending for New Sequences"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	GridBagSizer1->Add(StaticText4, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ModelBlendDefaultChoice = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
	ModelBlendDefaultChoice->SetSelection( ModelBlendDefaultChoice->Append(_("Enabled")) );
	ModelBlendDefaultChoice->Append(_("Disabled"));
	GridBagSizer1->Add(ModelBlendDefaultChoice, wxGBPosition(2, 1), wxDefaultSpan, wxALL, 5);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Render Cache"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(4, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	RenderCacheChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	RenderCacheChoice->Append(_("Enabled"));
	RenderCacheChoice->SetSelection( RenderCacheChoice->Append(_("Locked Effects Only")) );
	RenderCacheChoice->Append(_("Disabled"));
	GridBagSizer1->Add(RenderCacheChoice, wxGBPosition(4, 1), wxDefaultSpan, wxALL, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Auto Save Interval"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText2, wxGBPosition(6, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	AutoSaveIntervalChoice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	AutoSaveIntervalChoice->Append(_("Disabled"));
	AutoSaveIntervalChoice->Append(_("3 Minutes"));
	AutoSaveIntervalChoice->SetSelection( AutoSaveIntervalChoice->Append(_("5 Minutes")) );
	AutoSaveIntervalChoice->Append(_("10 Minutes"));
	AutoSaveIntervalChoice->Append(_("15 Minutes"));
	AutoSaveIntervalChoice->Append(_("30 Minutes"));
	GridBagSizer1->Add(AutoSaveIntervalChoice, wxGBPosition(6, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("FSEQ Version"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText3, wxGBPosition(8, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FSEQVersionChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	FSEQVersionChoice->Append(_("V1"));
	FSEQVersionChoice->SetSelection( FSEQVersionChoice->Append(_("V2 ZSTD (Default)")) );
	FSEQVersionChoice->Append(_("V2 Uncompressed"));
	FSEQVersionChoice->Append(_("V2 ZLIB"));
	FSEQVersionChoice->Append(_("V2 ZSTD/sparse"));
	GridBagSizer1->Add(FSEQVersionChoice, wxGBPosition(8, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer3 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Render Cache Directory"));
	CheckBox_RenderCache = new wxCheckBox(this, ID_CHECKBOX6, _("Use Show Folder"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
	CheckBox_RenderCache->SetValue(false);
	StaticBoxSizer3->Add(CheckBox_RenderCache, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DirPickerCtrl_RenderCache = new wxDirPickerCtrl(this, ID_DIRPICKERCTRL3, wxEmptyString, wxEmptyString, wxDefaultPosition, wxSize(400,-1), wxDIRP_DIR_MUST_EXIST|wxDIRP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_DIRPICKERCTRL3"));
	StaticBoxSizer3->Add(DirPickerCtrl_RenderCache, 1, wxALL|wxEXPAND, 5);
	GridBagSizer1->Add(StaticBoxSizer3, wxGBPosition(5, 0), wxGBSpan(1, 2), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("FSEQ Directory"));
	CheckBox_FSEQ = new wxCheckBox(this, ID_CHECKBOX5, _("Use Show Folder"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
	CheckBox_FSEQ->SetValue(false);
	StaticBoxSizer2->Add(CheckBox_FSEQ, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DirPickerCtrl_FSEQ = new wxDirPickerCtrl(this, ID_DIRPICKERCTRL2, wxEmptyString, wxEmptyString, wxDefaultPosition, wxSize(400,-1), wxDIRP_DIR_MUST_EXIST|wxDIRP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_DIRPICKERCTRL2"));
	StaticBoxSizer2->Add(DirPickerCtrl_FSEQ, 1, wxALL|wxEXPAND, 5);
	GridBagSizer1->Add(StaticBoxSizer2, wxGBPosition(10, 0), wxGBSpan(1, 2), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Media/Resource Directories"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	MediaDirectoryList = new wxListBox(this, ID_LISTBOX_MEDIA, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX_MEDIA"));
	MediaDirectoryList->SetMinSize(wxDLG_UNIT(this,wxSize(-1,50)));
	FlexGridSizer1->Add(MediaDirectoryList, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	AddMediaButton = new wxButton(this, ID_BUTTON_ADDMEDIA, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADDMEDIA"));
	FlexGridSizer2->Add(AddMediaButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RemoveMediaButton = new wxButton(this, ID_BUTTON_REMOVE_MEDIA, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_REMOVE_MEDIA"));
	FlexGridSizer2->Add(RemoveMediaButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	GridBagSizer1->Add(StaticBoxSizer1, wxGBPosition(7, 0), wxGBSpan(1, 2), wxALL|wxEXPAND, 2);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT2, _("Default View for New Sequences"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	GridBagSizer1->Add(StaticText5, wxGBPosition(3, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ViewDefaultChoice = new wxChoice(this, ID_CHOICE_VIEW_DEFAULT, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_VIEW_DEFAULT"));
	ViewDefaultChoice->SetToolTip(_("This option is used to select which models will populate the master view when a new sequence is created."));
	GridBagSizer1->Add(ViewDefaultChoice, wxGBPosition(3, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(GridBagSizer1);
	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);

	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnRenderOnSaveCheckBoxClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnCheckBox_LowDefinitionRenderClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnFSEQSaveCheckBoxClick);
	Connect(ID_CHOICE4,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnModelBlendDefaultChoiceSelect);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnRenderCacheChoiceSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnAutoSaveIntervalChoiceSelect);
	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnFSEQVersionChoiceSelect);
	Connect(ID_CHECKBOX6,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnCheckBox_RenderCacheClick);
	Connect(ID_DIRPICKERCTRL3,wxEVT_COMMAND_DIRPICKER_CHANGED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnDirPickerCtrl_RenderCacheDirChanged);
	Connect(ID_CHECKBOX5,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnCheckBox_FSEQClick);
	Connect(ID_DIRPICKERCTRL2,wxEVT_COMMAND_DIRPICKER_CHANGED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnDirPickerCtrl_FSEQDirChanged);
	Connect(ID_LISTBOX_MEDIA,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnMediaDirectoryListSelect);
	Connect(ID_BUTTON_ADDMEDIA,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnAddMediaButtonClick);
	Connect(ID_BUTTON_REMOVE_MEDIA,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnRemoveMediaButtonClick);
	Connect(ID_CHOICE_VIEW_DEFAULT,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SequenceFileSettingsPanel::OnViewDefaultChoiceSelect);
	//*)

	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);
    #ifdef _MSC_VER
    MSWDisableComposited();
    #endif
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
    frame->SetModelBlendDefaultOff(ModelBlendDefaultChoice->GetSelection());
    frame->SetLowDefinitionRender(CheckBox_LowDefinitionRender->IsChecked());

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

    std::list<std::string> mediaFolders;
    wxArrayString dirs =  MediaDirectoryList->GetStrings();
    for (auto &a : dirs) {
        mediaFolders.push_back(a.ToStdString());
    }
    frame->SetMediaFolders(mediaFolders);

    frame->SetFSEQFolder(CheckBox_FSEQ->GetValue(), DirPickerCtrl_FSEQ->GetPath());
    frame->SetRenderCacheFolder(CheckBox_RenderCache->GetValue(), DirPickerCtrl_RenderCache->GetPath());

    frame->SetDefaultSeqView(ViewDefaultChoice->GetStringSelection());

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
    ModelBlendDefaultChoice->SetSelection(frame->ModelBlendDefaultOff());
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
    frame->GetFSEQFolder(cb, folder);
    CheckBox_FSEQ->SetValue(cb);
    DirPickerCtrl_FSEQ->SetPath(folder);


    folder = frame->GetShowDirectory();
    MediaDirectoryList->Clear();
    for (auto &a : frame->GetMediaFolders()) {
        if (a != folder) {
            MediaDirectoryList->Append(a);
        }
    }
    AddMediaButton->Enable();
    RemoveMediaButton->Disable();

    frame->GetRenderCacheFolder(cb, folder);
    CheckBox_RenderCache->SetValue(cb);
    DirPickerCtrl_RenderCache->SetPath(folder);
    CheckBox_LowDefinitionRender->SetValue(frame->IsLowDefinitionRender());

    ViewDefaultChoice->Clear();
    ViewDefaultChoice->Append(wxString());
    ViewDefaultChoice->Append(frame->GetSequenceViews());
    auto const& view = ViewDefaultChoice->FindString(frame->GetDefaultSeqView());
    if (wxNOT_FOUND != view) {
        ViewDefaultChoice->SetSelection(view);
    }

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
    RemoveMediaButton->Enable(MediaDirectoryList->GetSelection() != wxNOT_FOUND);

    if (CheckBox_FSEQ->GetValue()) {
        DirPickerCtrl_FSEQ->Enable(false);
    } else {
        if (!wxDir::Exists(DirPickerCtrl_FSEQ->GetPath())) res = false;
        DirPickerCtrl_FSEQ->Enable(true);
    }

    if (CheckBox_RenderCache->GetValue()) {
        DirPickerCtrl_RenderCache->Enable(false);
    } else {
        if (!wxDir::Exists(DirPickerCtrl_RenderCache->GetPath())) res = false;
        DirPickerCtrl_RenderCache->Enable(true);
    }

    return res;
}
void SequenceFileSettingsPanel::OnCheckBox_RenderCacheClick(wxCommandEvent& event)
{
    ValidateWindow();
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void SequenceFileSettingsPanel::OnCheckBox_MediaClick(wxCommandEvent& event)
{
    ValidateWindow();
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void SequenceFileSettingsPanel::OnCheckBox_FSEQClick(wxCommandEvent& event)
{
    ValidateWindow();
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
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

void SequenceFileSettingsPanel::OnAddMediaButtonClick(wxCommandEvent& event)
{
    wxDirDialog dlg(NULL, "Choose media directory", "",
                    wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST | wxDD_NEW_DIR_BUTTON );
    if (dlg.ShowModal() == wxID_OK) {
        wxString d = dlg.GetPath();
        if (MediaDirectoryList->FindString(d) == wxNOT_FOUND) {
            MediaDirectoryList->Append(d);
            if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
                TransferDataFromWindow();
            }
        }
    }
}

void SequenceFileSettingsPanel::OnRemoveMediaButtonClick(wxCommandEvent& event)
{
    int i = MediaDirectoryList->GetSelection();
    if (i != wxNOT_FOUND) {
        MediaDirectoryList->Delete(i);
    }
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void SequenceFileSettingsPanel::OnMediaDirectoryListSelect(wxCommandEvent& event)
{
    RemoveMediaButton->Enable(MediaDirectoryList->GetSelection() != wxNOT_FOUND);
}

void SequenceFileSettingsPanel::OnModelBlendDefaultChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void SequenceFileSettingsPanel::OnViewDefaultChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void SequenceFileSettingsPanel::OnCheckBox_LowDefinitionRenderClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
