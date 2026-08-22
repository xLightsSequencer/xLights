#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/listbox.h>

#include <vector>

class xLightsFrame;

class ShowDirectoriesDialog : public wxDialog
{
public:
    ShowDirectoriesDialog(xLightsFrame* parent);
    virtual ~ShowDirectoriesDialog();

private:
    void UpdateControlsState();

    xLightsFrame* _xLights = nullptr;

    // Show directory controls
    wxButton* Button_ChangeShowDirPermanently = nullptr;
    wxButton* Button_CheckShowFolderTemporarily = nullptr;
    wxButton* Button_ChangeTemporarilyAgain = nullptr;
    wxStaticText* ShowDirectoryLabel = nullptr;

    // Favorite (pinned) show folders — quick-switch slots the user manages.
    wxListBox* FavoritesList = nullptr;
    wxButton* Button_AddFavorite = nullptr;
    wxButton* Button_RemoveFavorite = nullptr;
    wxButton* Button_GoFavPermanent = nullptr;
    wxButton* Button_GoFavTemporary = nullptr;
    std::vector<wxString> _favorites; // full paths, parallel to FavoritesList rows

    void LoadFavorites();
    void SaveFavorites() const;
    void RefreshFavoritesList();
    void UpdateFavoriteButtons();
    void SwitchToFolder(const wxString& dir, bool permanent);

    void OnAddFavorite(wxCommandEvent& event);
    void OnRemoveFavorite(wxCommandEvent& event);
    void OnGoFavoritePermanent(wxCommandEvent& event);
    void OnGoFavoriteTemporary(wxCommandEvent& event);
    void OnFavoriteSelectionChanged(wxCommandEvent& event);
    void OnFavoriteDoubleClick(wxCommandEvent& event);

    // Base directory controls
    wxStaticText* StaticText_BaseShowDirLabel = nullptr;
    wxButton* Button_ChangeBaseShowDir = nullptr;
    wxButton* Button_OpenBaseShowDir = nullptr;
    wxButton* Button_ClearBaseShowDir = nullptr;
    wxStaticText* StaticText_BaseShowDir = nullptr;
    wxCheckBox* CheckBox_AutoUpdateBase = nullptr;
    wxButton* Button_UpdateBase = nullptr;

    // Event handlers
    void OnButtonChangeShowDirPermanently(wxCommandEvent& event);
    void OnButtonCheckShowFolderTemporarily(wxCommandEvent& event);
    void OnButtonChangeTemporarilyAgain(wxCommandEvent& event);
    void OnButtonChangeBaseShowDir(wxCommandEvent& event);
    void OnButtonOpenBaseShowDir(wxCommandEvent& event);
    void OnButtonClearBaseShowDir(wxCommandEvent& event);
    void OnCheckBoxAutoUpdateBase(wxCommandEvent& event);
    void OnButtonUpdateBase(wxCommandEvent& event);

    // Identifiers
    static const wxWindowID ID_BUTTON_CHANGE_PERMANENT;
    static const wxWindowID ID_BUTTON_CHANGE_TEMPORARY;
    static const wxWindowID ID_BUTTON_CHANGE_TEMP_AGAIN;
    static const wxWindowID ID_STATICTEXT_SHOWDIR;
    static const wxWindowID ID_STATICTEXT_BASE_LABEL;
    static const wxWindowID ID_BUTTON_CHANGE_BASE;
    static const wxWindowID ID_BUTTON_OPEN_BASE;
    static const wxWindowID ID_BUTTON_CLEAR_BASE;
    static const wxWindowID ID_STATICTEXT_BASE_PATH;
    static const wxWindowID ID_CHECKBOX_AUTO_UPDATE;
    static const wxWindowID ID_BUTTON_UPDATE_BASE;
    static const wxWindowID ID_LISTBOX_FAVORITES;
    static const wxWindowID ID_BUTTON_ADD_FAV;
    static const wxWindowID ID_BUTTON_REMOVE_FAV;
    static const wxWindowID ID_BUTTON_GO_FAV_PERM;
    static const wxWindowID ID_BUTTON_GO_FAV_TEMP;

    wxDECLARE_EVENT_TABLE();
};
