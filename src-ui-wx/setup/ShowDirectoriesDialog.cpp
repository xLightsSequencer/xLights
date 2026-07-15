/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowDirectoriesDialog.h"
#include "xLightsMain.h"
#include "layout/LayoutPanel.h"
#include "layout/ViewsModelsPanel.h"
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/gbsizer.h>
#include <wx/dirdlg.h>
#include <wx/settings.h>

const wxWindowID ShowDirectoriesDialog::ID_BUTTON_CHANGE_PERMANENT = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_BUTTON_CHANGE_TEMPORARY = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_BUTTON_CHANGE_TEMP_AGAIN = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_STATICTEXT_SHOWDIR = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_STATICTEXT_BASE_LABEL = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_BUTTON_CHANGE_BASE = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_BUTTON_OPEN_BASE = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_BUTTON_CLEAR_BASE = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_STATICTEXT_BASE_PATH = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_CHECKBOX_AUTO_UPDATE = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_BUTTON_UPDATE_BASE = wxNewId();

wxBEGIN_EVENT_TABLE(ShowDirectoriesDialog, wxDialog)
    EVT_BUTTON(ID_BUTTON_CHANGE_PERMANENT, ShowDirectoriesDialog::OnButtonChangeShowDirPermanently)
    EVT_BUTTON(ID_BUTTON_CHANGE_TEMPORARY, ShowDirectoriesDialog::OnButtonCheckShowFolderTemporarily)
    EVT_BUTTON(ID_BUTTON_CHANGE_TEMP_AGAIN, ShowDirectoriesDialog::OnButtonChangeTemporarilyAgain)
    EVT_BUTTON(ID_BUTTON_CHANGE_BASE, ShowDirectoriesDialog::OnButtonChangeBaseShowDir)
    EVT_BUTTON(ID_BUTTON_OPEN_BASE, ShowDirectoriesDialog::OnButtonOpenBaseShowDir)
    EVT_BUTTON(ID_BUTTON_CLEAR_BASE, ShowDirectoriesDialog::OnButtonClearBaseShowDir)
    EVT_CHECKBOX(ID_CHECKBOX_AUTO_UPDATE, ShowDirectoriesDialog::OnCheckBoxAutoUpdateBase)
    EVT_BUTTON(ID_BUTTON_UPDATE_BASE, ShowDirectoriesDialog::OnButtonUpdateBase)
wxEND_EVENT_TABLE()

ShowDirectoriesDialog::ShowDirectoriesDialog(xLightsFrame* parent)
    : wxDialog(parent, wxID_ANY, _("Directories"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    _xLights = parent;

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBoxSizer* staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Directories"));
    wxGridBagSizer* gridBagSizer = new wxGridBagSizer(10, 10);

    // Row 0: Show Directory Header & Path
    wxStaticText* labelShowDir = new wxStaticText(this, wxID_ANY, _("Show Directory:"), wxDefaultPosition, wxDefaultSize, 0);
    gridBagSizer->Add(labelShowDir, wxGBPosition(0, 0), wxDefaultSpan, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    ShowDirectoryLabel = new wxStaticText(this, ID_STATICTEXT_SHOWDIR, _("{Show Directory not set}"), wxDefaultPosition, wxDefaultSize, 0);
    gridBagSizer->Add(ShowDirectoryLabel, wxGBPosition(0, 1), wxDefaultSpan, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    // Row 1: Show Directory Buttons (aligned under path)
    wxBoxSizer* showButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    Button_ChangeShowDirPermanently = new wxButton(this, ID_BUTTON_CHANGE_PERMANENT, _("Change Permanently"), wxDefaultPosition, wxDefaultSize, 0);
    showButtonsSizer->Add(Button_ChangeShowDirPermanently, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 10);

    Button_CheckShowFolderTemporarily = new wxButton(this, ID_BUTTON_CHANGE_TEMPORARY, _("Change Temporarily"), wxDefaultPosition, wxDefaultSize, 0);
    showButtonsSizer->Add(Button_CheckShowFolderTemporarily, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 10);

    Button_ChangeTemporarilyAgain = new wxButton(this, ID_BUTTON_CHANGE_TEMP_AGAIN, _("Change Temporarily Again"), wxDefaultPosition, wxDefaultSize, 0);
    Button_ChangeTemporarilyAgain->Hide();
    showButtonsSizer->Add(Button_ChangeTemporarilyAgain, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 10);

    gridBagSizer->Add(showButtonsSizer, wxGBPosition(1, 1), wxDefaultSpan, wxTOP | wxBOTTOM | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    // Row 2: Base Show Directory Header & Path
    StaticText_BaseShowDirLabel = new wxStaticText(this, ID_STATICTEXT_BASE_LABEL, _("Base Show Directory:"), wxDefaultPosition, wxDefaultSize, 0);
    StaticText_BaseShowDir = new wxStaticText(this, ID_STATICTEXT_BASE_PATH, _("No base show directory"), wxDefaultPosition, wxDefaultSize, 0);

    // Row 3: Base Show Directory Buttons (aligned under base path)
    wxBoxSizer* baseButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    Button_ChangeBaseShowDir = new wxButton(this, ID_BUTTON_CHANGE_BASE, _("Change"), wxDefaultPosition, wxDefaultSize, 0);
    baseButtonsSizer->Add(Button_ChangeBaseShowDir, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 10);

    Button_OpenBaseShowDir = new wxButton(this, ID_BUTTON_OPEN_BASE, _("Open"), wxDefaultPosition, wxDefaultSize, 0);
    Button_OpenBaseShowDir->SetToolTip(_("Open the base show folder"));
    baseButtonsSizer->Add(Button_OpenBaseShowDir, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 10);

    Button_ClearBaseShowDir = new wxButton(this, ID_BUTTON_CLEAR_BASE, _("Clear"), wxDefaultPosition, wxDefaultSize, 0);
    baseButtonsSizer->Add(Button_ClearBaseShowDir, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 10);

    CheckBox_AutoUpdateBase = new wxCheckBox(this, ID_CHECKBOX_AUTO_UPDATE, _("Auto Update On Load"), wxDefaultPosition, wxDefaultSize, 0);
    CheckBox_AutoUpdateBase->SetValue(false);
    baseButtonsSizer->Add(CheckBox_AutoUpdateBase, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 10);

    Button_UpdateBase = new wxButton(this, ID_BUTTON_UPDATE_BASE, _("Update"), wxDefaultPosition, wxDefaultSize, 0);
    baseButtonsSizer->Add(Button_UpdateBase, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 10);

    if (_xLights->IsShowBaseShowFolder()) {
        gridBagSizer->Add(StaticText_BaseShowDirLabel, wxGBPosition(2, 0), wxDefaultSpan, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
        gridBagSizer->Add(StaticText_BaseShowDir, wxGBPosition(2, 1), wxDefaultSpan, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
        gridBagSizer->Add(baseButtonsSizer, wxGBPosition(3, 1), wxDefaultSpan, wxTOP | wxBOTTOM | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    } else {
        StaticText_BaseShowDirLabel->Hide();
        StaticText_BaseShowDir->Hide();
        Button_ChangeBaseShowDir->Hide();
        Button_OpenBaseShowDir->Hide();
        Button_ClearBaseShowDir->Hide();
        CheckBox_AutoUpdateBase->Hide();
        Button_UpdateBase->Hide();
        delete baseButtonsSizer;
    }

    gridBagSizer->AddGrowableCol(1);
    staticBoxSizer->Add(gridBagSizer, 1, wxALL | wxEXPAND, 5);
    mainSizer->Add(staticBoxSizer, 1, wxALL | wxEXPAND, 10);

    // Dialog buttons (Close)
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* closeButton = new wxButton(this, wxID_CANCEL, _("Close"));
    buttonSizer->Add(closeButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_RIGHT | wxALL, 10);

    SetSizerAndFit(mainSizer);

    // Apply minimum width and center
    int w, h;
    GetSize(&w, &h);
    if (w < 1000) {
        SetSize(1000, h);
    }
    SetMinSize(wxSize(1000, h));
    CenterOnParent();

    UpdateControlsState();
}

ShowDirectoriesDialog::~ShowDirectoriesDialog()
{
}

void ShowDirectoriesDialog::UpdateControlsState()
{
    wxString currentDir = _xLights->CurrentDir;
    std::string baseShowDir = _xLights->_outputManager.GetBaseShowDir();
    bool permanent = (_xLights->_permanentShowFolder.empty() || _xLights->_permanentShowFolder == currentDir.ToStdString());
    bool readOnly = _xLights->IsReadOnlyMode();

    ShowDirectoryLabel->SetLabel(currentDir);

    CheckBox_AutoUpdateBase->SetValue(_xLights->_outputManager.IsAutoUpdateFromBaseShowDir());
    CheckBox_AutoUpdateBase->Enable(!readOnly);

    if (baseShowDir.empty()) {
        StaticText_BaseShowDir->SetLabel(_("No Base Show Directory"));
        Button_UpdateBase->Disable();
        Button_ClearBaseShowDir->Disable();
    } else {
        StaticText_BaseShowDir->SetLabel(baseShowDir);
        Button_UpdateBase->Enable(!readOnly);
        Button_ClearBaseShowDir->Enable(!readOnly);
    }

    Button_ChangeShowDirPermanently->Enable(!readOnly);
    Button_CheckShowFolderTemporarily->Enable(!readOnly);
    Button_ChangeTemporarilyAgain->Enable(!readOnly);
    Button_ChangeBaseShowDir->Enable(!readOnly);

    if (permanent) {
        ShowDirectoryLabel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
        wxFont font = ShowDirectoryLabel->GetFont();
        font.SetWeight(wxFONTWEIGHT_NORMAL);
        ShowDirectoryLabel->SetFont(font);
        Button_CheckShowFolderTemporarily->SetLabelText(_("Change Temporarily"));
        Button_ChangeTemporarilyAgain->Hide();
    } else {
        ShowDirectoryLabel->SetForegroundColour(wxColor(255, 200, 0));
        wxFont font = ShowDirectoryLabel->GetFont();
        font.SetWeight(wxFONTWEIGHT_BOLD);
        ShowDirectoryLabel->SetFont(font);
        Button_CheckShowFolderTemporarily->SetLabelText(_("Restore to Permanent"));
        Button_ChangeTemporarilyAgain->Show();
    }

    Layout();
}

void ShowDirectoriesDialog::OnButtonChangeShowDirPermanently(wxCommandEvent& event)
{
    _xLights->PromptForShowDirectory(true);
    UpdateControlsState();
}

void ShowDirectoriesDialog::OnButtonCheckShowFolderTemporarily(wxCommandEvent& event)
{
    wxString currentDir = _xLights->CurrentDir;
    bool permanent = (_xLights->_permanentShowFolder.empty() || _xLights->_permanentShowFolder == currentDir.ToStdString());
    if (permanent) {
        _xLights->PromptForShowDirectory(false);
    } else {
        _xLights->GetDisplayElementsPanel()->SetSequenceElementsModelsViews(nullptr, nullptr, nullptr);
        _xLights->GetLayoutPanel()->ClearUndo();
        wxASSERT(!_xLights->_permanentShowFolder.empty());
        _xLights->SetDir(_xLights->_permanentShowFolder, true);
    }
    UpdateControlsState();
}

void ShowDirectoriesDialog::OnButtonChangeTemporarilyAgain(wxCommandEvent& event)
{
    _xLights->PromptForShowDirectory(false);
    UpdateControlsState();
}

void ShowDirectoriesDialog::OnButtonChangeBaseShowDir(wxCommandEvent& event)
{
    wxDirDialog DirDialog1(this, _("Select Base Show Directory"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));
    if (DirDialog1.ShowModal() == wxID_OK) {
        _xLights->SetBaseShowDir(DirDialog1.GetPath());
        _xLights->ValidateWindow();
    }
    UpdateControlsState();
}

void ShowDirectoriesDialog::OnButtonOpenBaseShowDir(wxCommandEvent& event)
{
    _xLights->GetDisplayElementsPanel()->SetSequenceElementsModelsViews(nullptr, nullptr, nullptr);
    _xLights->GetLayoutPanel()->ClearUndo();
    _xLights->SetDir(_xLights->_outputManager.GetBaseShowDir(), false);
    UpdateControlsState();
}

void ShowDirectoriesDialog::OnButtonClearBaseShowDir(wxCommandEvent& event)
{
    _xLights->SetBaseShowDir("");
    _xLights->ValidateWindow();
    UpdateControlsState();
}

void ShowDirectoriesDialog::OnCheckBoxAutoUpdateBase(wxCommandEvent& event)
{
    _xLights->_outputManager.SetAutoUpdateFromBaseShowDir(CheckBox_AutoUpdateBase->IsChecked());
    _xLights->_outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnCheckBox_AutoUpdateBaseClick");
    UpdateControlsState();
}

void ShowDirectoriesDialog::OnButtonUpdateBase(wxCommandEvent& event)
{
    SetCursor(wxCURSOR_WAIT);
    _xLights->waitForPingsToComplete();
    _xLights->UpdateFromBaseShowFolder(true);
    SetCursor(wxCURSOR_ARROW);
    UpdateControlsState();
}
