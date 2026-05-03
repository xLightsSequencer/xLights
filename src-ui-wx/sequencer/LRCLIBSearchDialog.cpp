/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LRCLIBSearchDialog.h"
#include "utils/CurlManager.h"
#include "UtilFunctions.h"

#include <wx/uri.h>
#include <wx/font.h>

#include <nlohmann/json.hpp>
#include <log.h>

#include <sstream>
#include <algorithm>

const long LRCLIBSearchDialog::ID_SEARCH_TEXT = wxNewId();
const long LRCLIBSearchDialog::ID_SEARCH_BUTTON = wxNewId();
const long LRCLIBSearchDialog::ID_RESULTS_LIST = wxNewId();
const long LRCLIBSearchDialog::ID_OK_BUTTON = wxNewId();
const long LRCLIBSearchDialog::ID_CANCEL_BUTTON = wxNewId();

BEGIN_EVENT_TABLE(LRCLIBSearchDialog, wxDialog)
    EVT_BUTTON(LRCLIBSearchDialog::ID_SEARCH_BUTTON, LRCLIBSearchDialog::OnSearchClick)
    EVT_TEXT_ENTER(LRCLIBSearchDialog::ID_SEARCH_TEXT, LRCLIBSearchDialog::OnSearchTextEnter)
    EVT_LIST_ITEM_SELECTED(LRCLIBSearchDialog::ID_RESULTS_LIST, LRCLIBSearchDialog::OnResultSelected)
    EVT_LIST_ITEM_ACTIVATED(LRCLIBSearchDialog::ID_RESULTS_LIST, LRCLIBSearchDialog::OnResultActivated)
    EVT_BUTTON(LRCLIBSearchDialog::ID_OK_BUTTON, LRCLIBSearchDialog::OnOKClick)
    EVT_BUTTON(LRCLIBSearchDialog::ID_CANCEL_BUTTON, LRCLIBSearchDialog::OnCancelClick)
END_EVENT_TABLE()

// ParseLRC is implemented in src-core/lyrics/LRCParser.cpp; the header
// of this dialog re-exports it via an inline wrapper for source
// compatibility with the original RowHeading.cpp call sites.

static std::string UrlEncode(const wxString& str)
{
    wxString encoded;
    wxString utf8 = str;
    for (size_t i = 0; i < utf8.length(); i++) {
        wxChar c = utf8[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else if (c == ' ') {
            encoded += '+';
        } else {
            // Encode the UTF-8 bytes
            std::string utf8Bytes = wxString(c).ToStdString();
            for (unsigned char byte : utf8Bytes) {
                encoded += wxString::Format("%%%02X", byte);
            }
        }
    }
    return encoded.ToStdString();
}

LRCLIBSearchDialog::LRCLIBSearchDialog(wxWindow* parent, const wxString& songTitle,
                                       const wxString& artistName, bool autoSearch,
                                       wxWindowID id)
{
    Create(parent, id, "Search for Lyrics Online", wxDefaultPosition, wxSize(700, 550),
           wxCAPTION | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxCLOSE_BOX, "LRCLIBSearchDialog");

    auto* mainSizer = new wxFlexGridSizer(0, 1, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->AddGrowableRow(1);
    mainSizer->AddGrowableRow(2);

    // Search bar
    auto* searchSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* searchLabel = new wxStaticText(this, wxID_ANY, "Search:");
    searchSizer->Add(searchLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    m_searchText = new wxTextCtrl(this, ID_SEARCH_TEXT, wxEmptyString, wxDefaultPosition,
                                  wxDefaultSize, wxTE_PROCESS_ENTER);
    searchSizer->Add(m_searchText, 1, wxALL | wxEXPAND, 5);
    m_searchButton = new wxButton(this, ID_SEARCH_BUTTON, "Search", wxDefaultPosition, wxDefaultSize);
    searchSizer->Add(m_searchButton, 0, wxALL, 5);
    mainSizer->Add(searchSizer, 0, wxEXPAND | wxALL, 5);

    // Results list
    m_resultsList = new wxListCtrl(this, ID_RESULTS_LIST, wxDefaultPosition, wxSize(-1, 180),
                                   wxLC_REPORT | wxLC_SINGLE_SEL);
    m_resultsList->InsertColumn(0, "Track", wxLIST_FORMAT_LEFT, 200);
    m_resultsList->InsertColumn(1, "Artist", wxLIST_FORMAT_LEFT, 160);
    m_resultsList->InsertColumn(2, "Album", wxLIST_FORMAT_LEFT, 150);
    m_resultsList->InsertColumn(3, "Synced", wxLIST_FORMAT_CENTER, 60);
    mainSizer->Add(m_resultsList, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Preview
    auto* previewLabel = new wxStaticText(this, wxID_ANY, "Preview:");
    mainSizer->Add(previewLabel, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    m_previewText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 120),
                                   wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
    wxFont monoFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_previewText->SetFont(monoFont);
    mainSizer->Add(m_previewText, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Status + caution
    m_statusText = new wxStaticText(this, wxID_ANY, "Enter a search query to find lyrics.");
    mainSizer->Add(m_statusText, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    auto* cautionText = new wxStaticText(this, wxID_ANY,
        "Caution: All timings/labels on this track will be replaced.");
    wxFont cautionFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    cautionText->SetFont(cautionFont);
    mainSizer->Add(cautionText, 0, wxLEFT | wxRIGHT, 10);

    // Buttons
    auto* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer();
    m_okButton = new wxButton(this, ID_OK_BUTTON, "OK");
    m_cancelButton = new wxButton(this, ID_CANCEL_BUTTON, "Cancel");
    buttonSizer->Add(m_okButton, 0, wxALL, 5);
    buttonSizer->Add(m_cancelButton, 0, wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);

    SetSizer(mainSizer);
    SetEscapeId(ID_CANCEL_BUTTON);

    // Pre-populate search field
    wxString query;
    if (!artistName.IsEmpty() && !songTitle.IsEmpty()) {
        query = artistName + " - " + songTitle;
    } else if (!songTitle.IsEmpty()) {
        query = songTitle;
    } else if (!artistName.IsEmpty()) {
        query = artistName;
    }
    m_searchText->SetValue(query);

    ValidateWindow();
    m_searchText->SetFocus();

    if (autoSearch && !query.IsEmpty()) {
        DoSearch();
        // Pre-select first result with synced lyrics
        for (size_t i = 0; i < m_results.size(); i++) {
            if (!m_results[i].syncedLyrics.empty()) {
                m_resultsList->SetItemState(i, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                                            wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
                m_selectedIndex = i;
                UpdatePreview();
                ValidateWindow();
                break;
            }
        }
    }
}

LRCLIBSearchDialog::~LRCLIBSearchDialog()
{
}

std::string LRCLIBSearchDialog::GetSelectedSyncedLyrics() const
{
    if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_results.size()) {
        return m_results[m_selectedIndex].syncedLyrics;
    }
    return "";
}

std::string LRCLIBSearchDialog::GetSelectedPlainLyrics() const
{
    if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_results.size()) {
        return m_results[m_selectedIndex].plainLyrics;
    }
    return "";
}

bool LRCLIBSearchDialog::HasSyncedLyrics() const
{
    if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_results.size()) {
        return !m_results[m_selectedIndex].syncedLyrics.empty();
    }
    return false;
}

void LRCLIBSearchDialog::OnSearchClick(wxCommandEvent& event)
{
    DoSearch();
}

void LRCLIBSearchDialog::OnSearchTextEnter(wxCommandEvent& event)
{
    DoSearch();
}

void LRCLIBSearchDialog::OnResultSelected(wxListEvent& event)
{
    m_selectedIndex = event.GetIndex();
    UpdatePreview();
    ValidateWindow();
}

void LRCLIBSearchDialog::OnResultActivated(wxListEvent& event)
{
    m_selectedIndex = event.GetIndex();
    UpdatePreview();
    ValidateWindow();
    if (m_okButton->IsEnabled()) {
        EndDialog(wxID_OK);
    }
}

void LRCLIBSearchDialog::OnOKClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void LRCLIBSearchDialog::OnCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void LRCLIBSearchDialog::DoSearch()
{
    wxString query = m_searchText->GetValue().Trim().Trim(false);
    if (query.IsEmpty()) {
        m_statusText->SetLabel("Please enter a search query.");
        return;
    }

    m_statusText->SetLabel("Searching...");
    m_resultsList->DeleteAllItems();
    m_previewText->Clear();
    m_results.clear();
    m_selectedIndex = -1;
    ValidateWindow();
    wxYieldIfNeeded();

    std::string url = "https://lrclib.net/api/search?q=" + UrlEncode(query);
    spdlog::info("LRCLIB search: {}", url);

    std::vector<std::pair<std::string, std::string>> headers = {
        {"User-Agent", "xLights (https://xlights.org)"}
    };

    int responseCode = 0;
    std::string response = CurlManager::HTTPSGet(url, "", "", 10, headers, &responseCode);

    if (response.empty()) {
        spdlog::warn("LRCLIB search returned empty response (HTTP {})", responseCode);
        m_statusText->SetLabel("No results found or network error.");
        ValidateWindow();
        return;
    }

    try {
        nlohmann::json val = nlohmann::json::parse(response);
        if (!val.is_array()) {
            m_statusText->SetLabel("Unexpected response from LRCLIB.");
            ValidateWindow();
            return;
        }

        int syncedCount = 0;
        for (const auto& item : val) {
            if (!item.is_object())
                continue;

            LRCLIBResult result;
            if (item.contains("id") && item["id"].is_number())
                result.id = item["id"].get<int>();
            if (item.contains("trackName") && item["trackName"].is_string())
                result.trackName = item["trackName"].get<std::string>();
            if (item.contains("artistName") && item["artistName"].is_string())
                result.artistName = item["artistName"].get<std::string>();
            if (item.contains("albumName") && item["albumName"].is_string())
                result.albumName = item["albumName"].get<std::string>();
            if (item.contains("duration") && item["duration"].is_number())
                result.duration = item["duration"].get<double>();
            if (item.contains("instrumental") && item["instrumental"].is_boolean())
                result.instrumental = item["instrumental"].get<bool>();
            if (item.contains("plainLyrics") && item["plainLyrics"].is_string())
                result.plainLyrics = item["plainLyrics"].get<std::string>();
            if (item.contains("syncedLyrics") && item["syncedLyrics"].is_string())
                result.syncedLyrics = item["syncedLyrics"].get<std::string>();

            if (!result.syncedLyrics.empty())
                syncedCount++;

            m_results.push_back(result);
        }

        // Populate list control
        for (size_t i = 0; i < m_results.size(); i++) {
            long idx = m_resultsList->InsertItem(i, wxString(m_results[i].trackName));
            m_resultsList->SetItem(idx, 1, wxString(m_results[i].artistName));
            m_resultsList->SetItem(idx, 2, wxString(m_results[i].albumName));
            m_resultsList->SetItem(idx, 3, m_results[i].syncedLyrics.empty() ? "No" : "Yes");
        }

        m_statusText->SetLabel(wxString::Format("%zu results found (%d with synced lyrics).",
                                                m_results.size(), syncedCount));
    } catch (std::exception& e) {
        spdlog::error("LRCLIB JSON parse error: {}", e.what());
        m_statusText->SetLabel("Error parsing search results.");
    }

    ValidateWindow();
}

void LRCLIBSearchDialog::UpdatePreview()
{
    m_previewText->Clear();
    if (m_selectedIndex < 0 || m_selectedIndex >= (int)m_results.size())
        return;

    const auto& result = m_results[m_selectedIndex];
    if (!result.syncedLyrics.empty()) {
        m_previewText->SetValue(wxString(result.syncedLyrics));
    } else if (!result.plainLyrics.empty()) {
        m_previewText->SetValue(wxString(result.plainLyrics));
    } else if (result.instrumental) {
        m_previewText->SetValue("(Instrumental)");
    }
}

void LRCLIBSearchDialog::ValidateWindow()
{
    bool hasSelection = m_selectedIndex >= 0 && m_selectedIndex < (int)m_results.size();
    bool hasLyrics = false;
    if (hasSelection) {
        const auto& r = m_results[m_selectedIndex];
        hasLyrics = !r.syncedLyrics.empty() || !r.plainLyrics.empty();
    }
    m_okButton->Enable(hasSelection && hasLyrics);
}
