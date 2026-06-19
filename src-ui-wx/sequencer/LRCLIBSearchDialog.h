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
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/sizer.h>

#include "lyrics/LRCParser.h"

#include <string>
#include <vector>
#include <utility>

struct LRCLIBResult {
    int id = 0;
    std::string trackName;
    std::string artistName;
    std::string albumName;
    double duration = 0.0;
    bool instrumental = false;
    std::string plainLyrics;
    std::string syncedLyrics;
};

// LRC format → (millisecond, text) pairs. Wraps the wx-free core
// helper so existing call sites continue to compile.
inline std::vector<std::pair<int, std::string>> ParseLRC(const std::string& syncedLyrics) {
    return lrc::ParseLRC(syncedLyrics);
}

class LRCLIBSearchDialog : public wxDialog
{
public:
    LRCLIBSearchDialog(wxWindow* parent,
                       const wxString& songTitle = "",
                       const wxString& artistName = "",
                       bool autoSearch = false,
                       wxWindowID id = wxID_ANY);
    virtual ~LRCLIBSearchDialog();

    std::string GetSelectedSyncedLyrics() const;
    std::string GetSelectedPlainLyrics() const;
    bool HasSyncedLyrics() const;

private:
    void OnSearchClick(wxCommandEvent& event);
    void OnSearchTextEnter(wxCommandEvent& event);
    void OnResultSelected(wxListEvent& event);
    void OnResultActivated(wxListEvent& event);
    void OnOKClick(wxCommandEvent& event);
    void OnCancelClick(wxCommandEvent& event);

    void DoSearch();
    void UpdatePreview();
    void ValidateWindow();

    wxTextCtrl* m_searchText = nullptr;
    wxButton* m_searchButton = nullptr;
    wxListCtrl* m_resultsList = nullptr;
    wxTextCtrl* m_previewText = nullptr;
    wxStaticText* m_statusText = nullptr;
    wxButton* m_okButton = nullptr;
    wxButton* m_cancelButton = nullptr;

    std::vector<LRCLIBResult> m_results;
    int m_selectedIndex = -1;

    static const long ID_SEARCH_TEXT;
    static const long ID_SEARCH_BUTTON;
    static const long ID_RESULTS_LIST;
    static const long ID_OK_BUTTON;
    static const long ID_CANCEL_BUTTON;

    DECLARE_EVENT_TABLE()
};
