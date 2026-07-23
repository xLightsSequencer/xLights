/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "TreeListColumnVisibility.h"

#include <wx/dataview.h>
#include <wx/menu.h>
#include <wx/treelist.h>

#include <algorithm>

#include "../../settings/XLightsConfigAdapter.h"

static void SaveHiddenColumns(wxTreeListCtrl* tree, const std::string& configKey) {
    wxString hidden;
    for (unsigned i = 1; i < tree->GetColumnCount(); i++) {
        auto* col = tree->GetDataView()->GetColumn(i);
        if (col->IsHidden()) {
            if (!hidden.empty()) hidden += ",";
            hidden += col->GetTitle();
        }
    }
    GetXLightsConfig()->Write(configKey, hidden);
}

static void ApplyHiddenColumns(wxTreeListCtrl* tree, const std::string& configKey,
                                const std::vector<std::string>& defaultHiddenTitles) {
    wxString hidden = GetXLightsConfig()->Read(configKey, "");
    wxArrayString hiddenTitles = wxSplit(hidden, ',');

    const std::string seenKey = configKey + "Seen";
    wxArrayString seenTitles = wxSplit(GetXLightsConfig()->Read(seenKey, ""), ',');
    bool seenChanged = false;

    for (unsigned i = 1; i < tree->GetColumnCount(); i++) {
        auto* col = tree->GetDataView()->GetColumn(i);
        wxString title = col->GetTitle();
        bool alreadySeen = seenTitles.Index(title) != wxNOT_FOUND;
        if (!alreadySeen) {
            seenTitles.push_back(title);
            seenChanged = true;
            bool defaultHidden = std::find(defaultHiddenTitles.begin(), defaultHiddenTitles.end(),
                                            title.ToStdString()) != defaultHiddenTitles.end();
            if (defaultHidden && hiddenTitles.Index(title) == wxNOT_FOUND) {
                hiddenTitles.push_back(title);
            }
        }
        col->SetHidden(hiddenTitles.Index(title) != wxNOT_FOUND);
    }

    if (seenChanged) {
        GetXLightsConfig()->Write(seenKey, wxJoin(seenTitles, ','));
        GetXLightsConfig()->Write(configKey, wxJoin(hiddenTitles, ','));
    }
}

void InstallTreeListColumnVisibility(wxTreeListCtrl* tree, const std::string& configKey,
                                      const std::vector<std::string>& defaultHiddenTitles) {
    ApplyHiddenColumns(tree, configKey, defaultHiddenTitles);

    tree->GetDataView()->Bind(wxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK,
        [tree, configKey](wxDataViewEvent& event) {
            if (tree->GetColumnCount() < 2) {
                event.Skip();
                return;
            }
            wxMenu menu;
            const int baseId = wxID_HIGHEST + 1000;
            // Menu entries are alphabetized for ease of scanning; this is purely
            // a display convenience and does not affect the tree's own column
            // order, which stays exactly as the user laid it out.
            std::vector<unsigned> order(tree->GetColumnCount() - 1);
            for (unsigned i = 1; i < tree->GetColumnCount(); i++) order[i - 1] = i;
            std::sort(order.begin(), order.end(), [tree](unsigned a, unsigned b) {
                return tree->GetDataView()->GetColumn(a)->GetTitle().CmpNoCase(
                           tree->GetDataView()->GetColumn(b)->GetTitle()) < 0;
            });
            for (unsigned i : order) {
                auto* col = tree->GetDataView()->GetColumn(i);
                auto* item = menu.AppendCheckItem(baseId + i, col->GetTitle());
                item->Check(!col->IsHidden());
            }
            menu.Bind(wxEVT_MENU, [tree, configKey, baseId](wxCommandEvent& mev) {
                unsigned idx = mev.GetId() - baseId;
                if (idx >= 1 && idx < tree->GetColumnCount()) {
                    auto* col = tree->GetDataView()->GetColumn(idx);
                    col->SetHidden(!col->IsHidden());
                    SaveHiddenColumns(tree, configKey);
                }
            });
            tree->PopupMenu(&menu);
        });
}
