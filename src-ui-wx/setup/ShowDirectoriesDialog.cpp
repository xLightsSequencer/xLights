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

#include <wx/textdlg.h>
#include "xLightsMain.h"
#include "layout/LayoutPanel.h"
#include "layout/ViewsModelsPanel.h"
#include "settings/XLightsConfigAdapter.h"
#include "ExternalHooks.h"
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/gbsizer.h>
#include <wx/dirdlg.h>
#include <wx/settings.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>

#include <algorithm>

namespace {
    constexpr int MAX_SHOW_FAVORITES = 30;
    const std::string FAV_KEY_PREFIX = "ShowFolderFavorite";      // legacy: path only
    const std::string FAV_NAME_PREFIX = "ShowFolderFavoriteName";
}

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
const wxWindowID ShowDirectoriesDialog::ID_LISTBOX_FAVORITES = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_BUTTON_RENAME_FAV = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_BUTTON_ADD_FAV = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_BUTTON_REMOVE_FAV = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_BUTTON_GO_FAV_PERM = wxNewId();
const wxWindowID ShowDirectoriesDialog::ID_BUTTON_GO_FAV_TEMP = wxNewId();

wxBEGIN_EVENT_TABLE(ShowDirectoriesDialog, wxDialog)
    EVT_BUTTON(ID_BUTTON_CHANGE_PERMANENT, ShowDirectoriesDialog::OnButtonChangeShowDirPermanently)
    EVT_BUTTON(ID_BUTTON_CHANGE_TEMPORARY, ShowDirectoriesDialog::OnButtonCheckShowFolderTemporarily)
    EVT_BUTTON(ID_BUTTON_CHANGE_TEMP_AGAIN, ShowDirectoriesDialog::OnButtonChangeTemporarilyAgain)
    EVT_BUTTON(ID_BUTTON_CHANGE_BASE, ShowDirectoriesDialog::OnButtonChangeBaseShowDir)
    EVT_BUTTON(ID_BUTTON_OPEN_BASE, ShowDirectoriesDialog::OnButtonOpenBaseShowDir)
    EVT_BUTTON(ID_BUTTON_CLEAR_BASE, ShowDirectoriesDialog::OnButtonClearBaseShowDir)
    EVT_CHECKBOX(ID_CHECKBOX_AUTO_UPDATE, ShowDirectoriesDialog::OnCheckBoxAutoUpdateBase)
    EVT_BUTTON(ID_BUTTON_UPDATE_BASE, ShowDirectoriesDialog::OnButtonUpdateBase)
    EVT_BUTTON(ID_BUTTON_ADD_FAV, ShowDirectoriesDialog::OnAddFavorite)
    EVT_BUTTON(ID_BUTTON_REMOVE_FAV, ShowDirectoriesDialog::OnRemoveFavorite)
    EVT_BUTTON(ID_BUTTON_GO_FAV_PERM, ShowDirectoriesDialog::OnGoFavoritePermanent)
    EVT_BUTTON(ID_BUTTON_GO_FAV_TEMP, ShowDirectoriesDialog::OnGoFavoriteTemporary)
    EVT_LIST_ITEM_SELECTED(ID_LISTBOX_FAVORITES, ShowDirectoriesDialog::OnFavoriteSelectionChanged)
    EVT_LIST_ITEM_DESELECTED(ID_LISTBOX_FAVORITES, ShowDirectoriesDialog::OnFavoriteSelectionChanged)
    EVT_LIST_ITEM_ACTIVATED(ID_LISTBOX_FAVORITES, ShowDirectoriesDialog::OnFavoriteDoubleClick)
    EVT_BUTTON(ID_BUTTON_RENAME_FAV, ShowDirectoriesDialog::OnRenameFavorite)
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
    ShowDirectoryLabel->SetToolTip(_("Double-click to open in file manager"));
    ShowDirectoryLabel->Bind(wxEVT_LEFT_DCLICK, [this](wxMouseEvent&) {
        if (!_xLights->CurrentDir.IsEmpty()) {
            wxLaunchDefaultApplication(_xLights->CurrentDir);
        }
    });
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
    StaticText_BaseShowDir->SetToolTip(_("Double-click to open in file manager"));
    StaticText_BaseShowDir->Bind(wxEVT_LEFT_DCLICK, [this](wxMouseEvent&) {
        const std::string baseDir = _xLights->_outputManager.GetBaseShowDir();
        if (!baseDir.empty()) {
            wxLaunchDefaultApplication(baseDir);
        }
    });

    // Row 3: Base Show Directory Buttons (aligned under base path). Only built
    // when the base show folder feature is enabled, so there are no orphan
    // children left parked at the dialog origin when it is off.
    if (_xLights->IsShowBaseShowFolder()) {
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

        gridBagSizer->Add(StaticText_BaseShowDirLabel, wxGBPosition(2, 0), wxDefaultSpan, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
        gridBagSizer->Add(StaticText_BaseShowDir, wxGBPosition(2, 1), wxDefaultSpan, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
        gridBagSizer->Add(baseButtonsSizer, wxGBPosition(3, 1), wxDefaultSpan, wxTOP | wxBOTTOM | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    } else {
        StaticText_BaseShowDirLabel->Hide();
        StaticText_BaseShowDir->Hide();
    }

    gridBagSizer->AddGrowableCol(1);
    staticBoxSizer->Add(gridBagSizer, 1, wxALL | wxEXPAND, 5);
    mainSizer->Add(staticBoxSizer, 0, wxALL | wxEXPAND, 10);

    // Favorites: pinned quick-switch slots for show folders you use often.
    wxStaticBoxSizer* favBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Favorite Show Folders"));
    FavoritesList = new wxListCtrl(this, ID_LISTBOX_FAVORITES, wxDefaultPosition, wxSize(-1, FromDIP(120)),
                                   wxLC_REPORT | wxLC_SINGLE_SEL);
    FavoritesList->AppendColumn(_("Name"), wxLIST_FORMAT_LEFT, FromDIP(160));
    FavoritesList->AppendColumn(_("Folder"), wxLIST_FORMAT_LEFT, FromDIP(420));
    favBox->Add(FavoritesList, 1, wxALL | wxEXPAND, 5);

    wxBoxSizer* favButtons = new wxBoxSizer(wxHORIZONTAL);
    Button_AddFavorite = new wxButton(this, ID_BUTTON_ADD_FAV, _("Add Current"));
    Button_AddFavorite->SetToolTip(_("Pin the current show folder as a favorite"));
    favButtons->Add(Button_AddFavorite, 0, wxRIGHT, 8);
    Button_RenameFavorite = new wxButton(this, ID_BUTTON_RENAME_FAV, _("Rename"));
    Button_RenameFavorite->SetToolTip(_("Change the name shown in the File menu"));
    favButtons->Add(Button_RenameFavorite, 0, wxRIGHT, 8);
    Button_RemoveFavorite = new wxButton(this, ID_BUTTON_REMOVE_FAV, _("Remove"));
    favButtons->Add(Button_RemoveFavorite, 0, wxRIGHT, 8);
    favButtons->AddStretchSpacer(1);
    Button_GoFavPermanent = new wxButton(this, ID_BUTTON_GO_FAV_PERM, _("Switch Permanently"));
    favButtons->Add(Button_GoFavPermanent, 0, wxRIGHT, 8);
    Button_GoFavTemporary = new wxButton(this, ID_BUTTON_GO_FAV_TEMP, _("Switch Temporarily"));
    favButtons->Add(Button_GoFavTemporary, 0, 0, 0);
    favBox->Add(favButtons, 0, wxALL | wxEXPAND, 5);

    mainSizer->Add(favBox, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 10);

    // Dialog buttons (Close)
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* closeButton = new wxButton(this, wxID_CANCEL, _("Close"));
    buttonSizer->Add(closeButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_RIGHT | wxALL, 10);

    // Populate the real path labels before Fit() so the sizer measures their
    // actual (possibly long) text instead of the short placeholder strings the
    // labels were constructed with.
    LoadFavorites();
    RefreshFavoritesList();
    UpdateControlsState();

    SetSizerAndFit(mainSizer);

    // A long directory path can make the natural fit wider than the display;
    // clamp to that, but otherwise let the sizer's own fit width stand rather
    // than forcing every dialog up to some fixed minimum.
    int w, h;
    GetSize(&w, &h);
    int maxW = wxGetDisplaySize().GetWidth();
    if (w > maxW) {
        SetSize(maxW, h);
        w = maxW;
    }
    SetMinSize(wxSize(w, h));
    CenterOnParent();
    Layout();
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

    // The base-show-folder controls only exist when that feature is enabled.
    if (CheckBox_AutoUpdateBase != nullptr) {
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
        Button_ChangeBaseShowDir->Enable(!readOnly);
    }

    Button_ChangeShowDirPermanently->Enable(!readOnly);
    Button_CheckShowFolderTemporarily->Enable(!readOnly);
    Button_ChangeTemporarilyAgain->Enable(!readOnly);

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

    UpdateFavoriteButtons();
    Layout();
}

void ShowDirectoriesDialog::OnButtonChangeShowDirPermanently(wxCommandEvent& event)
{
    if (_xLights->PromptForShowDirectory(true)) {
        EndModal(wxID_OK);
        return;
    }
    UpdateControlsState();
}

void ShowDirectoriesDialog::OnButtonCheckShowFolderTemporarily(wxCommandEvent& event)
{
    wxString currentDir = _xLights->CurrentDir;
    bool permanent = (_xLights->_permanentShowFolder.empty() || _xLights->_permanentShowFolder == currentDir.ToStdString());
    bool changed;
    if (permanent) {
        changed = _xLights->PromptForShowDirectory(false);
    } else {
        _xLights->GetDisplayElementsPanel()->SetSequenceElementsModelsViews(nullptr, nullptr, nullptr);
        _xLights->GetLayoutPanel()->ClearUndo();
        wxASSERT(!_xLights->_permanentShowFolder.empty());
        changed = _xLights->SetDir(_xLights->_permanentShowFolder, true);
    }
    if (changed) {
        EndModal(wxID_OK);
        return;
    }
    UpdateControlsState();
}

void ShowDirectoriesDialog::OnButtonChangeTemporarilyAgain(wxCommandEvent& event)
{
    if (_xLights->PromptForShowDirectory(false)) {
        EndModal(wxID_OK);
        return;
    }
    UpdateControlsState();
}

void ShowDirectoriesDialog::OnButtonChangeBaseShowDir(wxCommandEvent& event)
{
    wxDirDialog DirDialog1(this, _("Select Base Show Directory"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));
    if (DirDialog1.ShowModal() == wxID_OK) {
        _xLights->SetBaseShowDir(DirDialog1.GetPath());
    }
    UpdateControlsState();
}

void ShowDirectoriesDialog::OnButtonOpenBaseShowDir(wxCommandEvent& event)
{
    _xLights->GetDisplayElementsPanel()->SetSequenceElementsModelsViews(nullptr, nullptr, nullptr);
    _xLights->GetLayoutPanel()->ClearUndo();
    if (_xLights->SetDir(_xLights->_outputManager.GetBaseShowDir(), false)) {
        EndModal(wxID_OK);
        return;
    }
    UpdateControlsState();
}

void ShowDirectoriesDialog::OnButtonClearBaseShowDir(wxCommandEvent& event)
{
    _xLights->SetBaseShowDir("");
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

// ---- Favorite (pinned) show folders ----

std::vector<ShowDirectoriesDialog::ShowFavorite> ShowDirectoriesDialog::ReadFavoritesFromConfig()
{
    std::vector<ShowFavorite> out;
    auto* config = GetXLightsConfig();
    for (int i = 0; i < MAX_SHOW_FAVORITES; ++i) {
        wxString path;
        if (!config->Read(FAV_KEY_PREFIX + std::to_string(i), &path) || path.IsEmpty()) {
            continue;
        }
        wxString name;
        config->Read(FAV_NAME_PREFIX + std::to_string(i), &name);
        if (name.IsEmpty()) {
            // Favorites saved before names existed - fall back to the folder name.
            const wxFileName fn = wxFileName::DirName(path);
            name = fn.GetDirs().IsEmpty() ? path : fn.GetDirs().Last();
        }
        out.push_back({ name, path });
    }
    return out;
}

void ShowDirectoriesDialog::LoadFavorites()
{
    _favorites = ReadFavoritesFromConfig();
}

void ShowDirectoriesDialog::SaveFavorites() const
{
    auto* config = GetXLightsConfig();
    // Write current entries and blank out any trailing slots left from a longer
    // previous list so removed favorites don't reappear on reload.
    for (int i = 0; i < MAX_SHOW_FAVORITES; ++i) {
        const bool live = i < (int)_favorites.size();
        config->Write(FAV_KEY_PREFIX + std::to_string(i), live ? _favorites[i].path : wxString());
        config->Write(FAV_NAME_PREFIX + std::to_string(i), live ? _favorites[i].name : wxString());
    }
    config->Flush();
}

void ShowDirectoriesDialog::RefreshFavoritesList()
{
    if (FavoritesList == nullptr) return;
    FavoritesList->DeleteAllItems();
    for (const auto& f : _favorites) {
        const long row = FavoritesList->InsertItem(FavoritesList->GetItemCount(), f.name);
        FavoritesList->SetItem(row, 1, f.path);
    }
    UpdateFavoriteButtons();
}

long ShowDirectoriesDialog::SelectedFavorite() const
{
    if (FavoritesList == nullptr) return wxNOT_FOUND;
    return FavoritesList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}

void ShowDirectoriesDialog::OnRenameFavorite(wxCommandEvent& event)
{
    const long sel = SelectedFavorite();
    if (sel == wxNOT_FOUND || sel >= (long)_favorites.size()) return;
    const wxString name = PromptFavoriteName(_favorites[sel].name, _("Rename Favorite"));
    if (name.IsEmpty()) return;
    _favorites[sel].name = name;
    SaveFavorites();
    RefreshFavoritesList();
    FavoritesList->SetItemState(sel, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

wxString ShowDirectoriesDialog::PromptFavoriteName(const wxString& suggested, const wxString& title)
{
    wxTextEntryDialog dlg(this, _("Name for this show folder:"), title, suggested);
    if (dlg.ShowModal() != wxID_OK) return wxEmptyString;
    return dlg.GetValue().Trim(true).Trim(false);
}

void ShowDirectoriesDialog::UpdateFavoriteButtons()
{
    if (FavoritesList == nullptr) return;
    const bool hasSel = SelectedFavorite() != wxNOT_FOUND;
    Button_RemoveFavorite->Enable(hasSel);
    Button_RenameFavorite->Enable(hasSel);
    Button_GoFavPermanent->Enable(hasSel);
    Button_GoFavTemporary->Enable(hasSel);

    const wxString cur = _xLights->CurrentDir;
    const bool already = std::any_of(_favorites.begin(), _favorites.end(),
                                     [&cur](const ShowFavorite& f) { return f.path == cur; });
    Button_AddFavorite->Enable(!cur.IsEmpty() && !already && (int)_favorites.size() < MAX_SHOW_FAVORITES);
}

void ShowDirectoriesDialog::SwitchToFolder(const wxString& dir, bool permanent)
{
    if (dir.IsEmpty() || dir == _xLights->CurrentDir) return;
    if (!wxFileName::DirExists(dir)) {
        wxMessageBox(wxString::Format(_("This show folder no longer exists:\n%s"), dir),
                     _("Favorite Show Folder"), wxOK | wxICON_WARNING, this);
        return;
    }
    if (!ObtainAccessToURL(dir.ToStdString(), true)) {
        wxMessageBox(wxString::Format(_("xLights could not get access to:\n%s"), dir),
                     _("Favorite Show Folder"), wxOK | wxICON_WARNING, this);
        return;
    }
    _xLights->GetDisplayElementsPanel()->SetSequenceElementsModelsViews(nullptr, nullptr, nullptr);
    _xLights->GetLayoutPanel()->ClearUndo();
    _xLights->SetDir(dir, permanent);
    UpdateControlsState();
    UpdateFavoriteButtons();
}

void ShowDirectoriesDialog::OnAddFavorite(wxCommandEvent& event)
{
    const wxString cur = _xLights->CurrentDir;
    if (cur.IsEmpty()) return;
    if (std::any_of(_favorites.begin(), _favorites.end(),
                    [&cur](const ShowFavorite& f) { return f.path == cur; })) {
        return;
    }
    if ((int)_favorites.size() >= MAX_SHOW_FAVORITES) return;

    const wxFileName fn = wxFileName::DirName(cur);
    const wxString suggested = fn.GetDirs().IsEmpty() ? cur : fn.GetDirs().Last();
    const wxString name = PromptFavoriteName(suggested, _("Add Favorite"));
    if (name.IsEmpty()) return; // cancelled, or blanked out

    _favorites.push_back({ name, cur });
    SaveFavorites();
    RefreshFavoritesList();
    const long row = (long)_favorites.size() - 1;
    FavoritesList->SetItemState(row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    UpdateFavoriteButtons();
}

void ShowDirectoriesDialog::OnRemoveFavorite(wxCommandEvent& event)
{
    const long sel = SelectedFavorite();
    if (sel == wxNOT_FOUND || sel >= (long)_favorites.size()) return;
    _favorites.erase(_favorites.begin() + sel);
    SaveFavorites();
    RefreshFavoritesList();
}

void ShowDirectoriesDialog::OnGoFavoritePermanent(wxCommandEvent& event)
{
    const long sel = SelectedFavorite();
    if (sel != wxNOT_FOUND && sel < (long)_favorites.size()) SwitchToFolder(_favorites[sel].path, true);
}

void ShowDirectoriesDialog::OnGoFavoriteTemporary(wxCommandEvent& event)
{
    const long sel = SelectedFavorite();
    if (sel != wxNOT_FOUND && sel < (long)_favorites.size()) SwitchToFolder(_favorites[sel].path, false);
}

void ShowDirectoriesDialog::OnFavoriteSelectionChanged(wxListEvent& event)
{
    UpdateFavoriteButtons();
}

void ShowDirectoriesDialog::OnFavoriteDoubleClick(wxListEvent& event)
{
    const long sel = SelectedFavorite();
    if (sel != wxNOT_FOUND && sel < (long)_favorites.size()) SwitchToFolder(_favorites[sel].path, true);
}
