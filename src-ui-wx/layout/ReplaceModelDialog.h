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

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include <wx/artprov.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/font.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/string.h>
#include <wx/textctrl.h>

// Multi-select dialog with a live filter, used by LayoutPanel::ReplaceModel to
// pick the target model(s) to replace. Check state is kept persistent across
// filter changes so the user can type to narrow the list, check things, change
// the filter, and retain earlier selections.
class ReplaceModelDialog : public wxDialog {
public:
    ReplaceModelDialog(wxWindow* parent,
                       const wxString& sourceName,
                       const std::vector<std::string>& candidateNames,
                       const std::set<std::string>& baseLinkedNames = {})
        : wxDialog(parent, wxID_ANY,
                   wxString::Format("Replace Model(s) With: %s", sourceName),
                   wxDefaultPosition, wxSize(460, 560),
                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
          _allCandidates(candidateNames),
          _baseLinked(baseLinkedNames)
    {
        auto* top = new wxBoxSizer(wxVERTICAL);

        // Intro: kept short so it fits one line at default dialog width.
        // Wrap() is still called as a belt-and-suspenders for narrower widths
        // (user resize, platforms where default font is wider, etc).
        auto* intro = new wxStaticText(this, wxID_ANY,
            "Each selected model will be replaced with a copy of the current model.");
        intro->Wrap(420);
        top->Add(intro, 0, wxALL, 8);

        auto* filterRow = new wxBoxSizer(wxHORIZONTAL);
        filterRow->Add(new wxStaticText(this, wxID_ANY, "Filter:"),
                       0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
        _filterCtrl = new wxTextCtrl(this, wxID_ANY);
        _filterCtrl->SetHint("type to filter by name...");
        filterRow->Add(_filterCtrl, 1, wxEXPAND | wxRIGHT, 8);
        top->Add(filterRow, 0, wxEXPAND | wxBOTTOM, 4);

        // Build the list ourselves instead of using a native list widget. This
        // gives us:
        //   - real native wxCheckBox rendering on Windows / macOS / Linux
        //   - perfect vertical alignment, because the header row and each item
        //     row share the same 2-column (checkbox + name) sizer layout
        //   - the "select / clear all visible" master checkbox literally in
        //     the header cell next to "Model Name" - requested in PR review
        //
        // The outer frame is a wxStaticBox for a clean visual border around
        // the header + rows. The rows go in a scrollable inner wxPanel so the
        // dialog stays usable with 100+ models.
        auto* listBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
        auto* listBoxSizer = new wxStaticBoxSizer(listBox, wxVERTICAL);

        // Header row: [master checkbox] [bold "Model Name" label]
        auto* headerPanel = new wxPanel(listBox, wxID_ANY);
        auto* headerSizer = new wxBoxSizer(wxHORIZONTAL);
        _masterCB = new wxCheckBox(headerPanel, wxID_ANY, wxEmptyString,
                                   wxDefaultPosition, wxDefaultSize, wxCHK_3STATE);
        _masterCB->SetToolTip("Select / clear all currently visible models");
        auto* headerLabel = new wxStaticText(headerPanel, wxID_ANY, "Model Name");
        wxFont headerFont = headerLabel->GetFont();
        headerFont.SetWeight(wxFONTWEIGHT_BOLD);
        headerLabel->SetFont(headerFont);
        headerSizer->Add(_masterCB, 0, wxALIGN_CENTER_VERTICAL | wxALL, 4);
        headerSizer->Add(headerLabel, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
        headerPanel->SetSizer(headerSizer);
        listBoxSizer->Add(headerPanel, 0, wxEXPAND | wxBOTTOM, 2);

        // Thin separator so the header reads as a proper column header.
        auto* separator = new wxStaticLine(listBox, wxID_ANY,
                                           wxDefaultPosition, wxDefaultSize,
                                           wxLI_HORIZONTAL);
        listBoxSizer->Add(separator, 0, wxEXPAND | wxBOTTOM, 2);

        // Scrollable rows. Each row is its own wxCheckBox (native look) sized
        // to match the header's checkbox column, followed by the name text.
        _rowsWindow = new wxScrolledWindow(listBox, wxID_ANY,
                                           wxDefaultPosition, wxDefaultSize,
                                           wxVSCROLL | wxBORDER_NONE);
        _rowsWindow->SetScrollRate(0, 16);
        _rowsSizer = new wxBoxSizer(wxVERTICAL);
        _rowsWindow->SetSizer(_rowsSizer);
        listBoxSizer->Add(_rowsWindow, 1, wxEXPAND);

        top->Add(listBoxSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);

        top->Add(new wxStaticText(this, wxID_ANY, "For each replaced model, also:"),
                 0, wxLEFT | wxRIGHT | wxTOP, 8);
        _copyStartChannelCB = new wxCheckBox(this, wxID_ANY,
            "Keep each target's start channel and controller settings");
        _mergeSubmodelsCB = new wxCheckBox(this, wxID_ANY,
            "Keep each target's submodels");
        _copySizePosCB = new wxCheckBox(this, wxID_ANY,
            "Keep each target's position and size");
        _copyStartChannelCB->SetValue(true);
        _copySizePosCB->SetValue(true);
        const int cbFlags = wxLEFT | wxRIGHT | wxTOP;
        const int cbPad = 4;
        top->Add(_copyStartChannelCB, 0, cbFlags, cbPad);
        top->Add(_mergeSubmodelsCB,   0, cbFlags, cbPad);
        top->Add(_copySizePosCB,      0, cbFlags, cbPad);

        auto* btnRow = CreateButtonSizer(wxOK | wxCANCEL);
        if (btnRow != nullptr) {
            top->Add(btnRow, 0, wxEXPAND | wxALL, 8);
        }
        SetSizer(top);

        // Disable OK until the user has actually picked at least one target —
        // OK with an empty selection is a confusing no-op for a destructive
        // batch operation.
        _okBtn = static_cast<wxButton*>(FindWindow(wxID_OK));
        if (_okBtn != nullptr) _okBtn->Disable();

        RepopulateRows();
        UpdateMasterState();

        _filterCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent&) {
            RepopulateRows();
            UpdateMasterState();
        });
        _masterCB->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
            // 3-state checkbox: use Get3StateValue() — IsChecked() asserts
            // on a 3-state in wxWidgets debug builds. Treat anything that
            // isn't explicitly "checked" as an uncheck-all.
            const bool shouldCheck = _masterCB->Get3StateValue() == wxCHK_CHECKED;
            for (const auto& row : _rows) {
                row.cb->SetValue(shouldCheck);
                if (shouldCheck) {
                    _checkedSet.insert(row.name);
                } else {
                    _checkedSet.erase(row.name);
                }
            }
            UpdateMasterState();
        });
    }

    std::vector<std::string> GetSelectedNames() const {
        std::vector<std::string> out(_checkedSet.begin(), _checkedSet.end());
        std::sort(out.begin(), out.end());
        return out;
    }

    bool CopyStartChannel() const { return _copyStartChannelCB->IsChecked(); }
    bool MergeSubmodels()   const { return _mergeSubmodelsCB->IsChecked(); }
    bool CopySizePos()      const { return _copySizePosCB->IsChecked(); }

private:
    // Filter is case-insensitive substring match. wxString::Lower() handles
    // Unicode correctly via the platform's locale; std::tolower / std::string
    // would byte-fold and mishandle non-ASCII model names.
    static bool PassesFilter(const std::string& name, const wxString& filterLower) {
        if (filterLower.IsEmpty()) return true;
        return wxString::FromUTF8(name).Lower().Contains(filterLower);
    }

    void RepopulateRows() {
        const wxString filterLower = _filterCtrl->GetValue().Lower();

        _rowsWindow->Freeze();
        _rowsSizer->Clear(true); // deletes existing row widgets
        _rows.clear();

        for (const auto& name : _allCandidates) {
            if (!PassesFilter(name, filterLower)) continue;

            // Base-folder models can't be replaced - doing so would delete a
            // model the base show folder owns. Show them disabled with the
            // link icon so the user sees why they're off-limits, and keep them
            // out of _rows / _checkedSet so master select-all and the result
            // never include them.
            if (_baseLinked.count(name) > 0) {
                auto* row = new wxBoxSizer(wxHORIZONTAL);
                auto* cb = new wxCheckBox(_rowsWindow, wxID_ANY, name);
                cb->SetValue(false);
                cb->Disable();
                cb->SetToolTip("Linked from the base show folder - cannot be replaced");
                auto* icon = new wxStaticBitmap(_rowsWindow, wxID_ANY,
                    wxArtProvider::GetBitmapBundle("xlART_LINK", wxART_OTHER, FromDIP(wxSize(16, 16))));
                icon->SetToolTip("Linked from the base show folder");
                row->Add(cb, 0, wxALIGN_CENTER_VERTICAL);
                row->Add(icon, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
                _rowsSizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 2);
                continue;
            }

            auto* cb = new wxCheckBox(_rowsWindow, wxID_ANY, name);
            cb->SetValue(_checkedSet.count(name) > 0);
            _rowsSizer->Add(cb, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 2);
            _rows.push_back({ name, cb });

            // When an individual row checkbox is toggled, mirror the change
            // into _checkedSet and re-evaluate the master tri-state.
            const std::string capturedName = name;
            cb->Bind(wxEVT_CHECKBOX, [this, capturedName, cb](wxCommandEvent&) {
                if (cb->GetValue()) {
                    _checkedSet.insert(capturedName);
                } else {
                    _checkedSet.erase(capturedName);
                }
                UpdateMasterState();
            });
        }

        _rowsWindow->FitInside();
        _rowsWindow->Layout();
        _rowsWindow->Thaw();
    }

    void UpdateMasterState() {
        // Aggregate state of currently visible rows -> master checkbox tri-state.
        if (_rows.empty()) {
            _masterCB->Set3StateValue(wxCHK_UNCHECKED);
            _masterCB->Enable(false);
            if (_okBtn != nullptr) _okBtn->Enable(!_checkedSet.empty());
            return;
        }
        _masterCB->Enable(true);
        size_t checked = 0;
        for (const auto& r : _rows) {
            if (r.cb->GetValue()) ++checked;
        }
        if (checked == 0) {
            _masterCB->Set3StateValue(wxCHK_UNCHECKED);
        } else if (checked == _rows.size()) {
            _masterCB->Set3StateValue(wxCHK_CHECKED);
        } else {
            _masterCB->Set3StateValue(wxCHK_UNDETERMINED);
        }
        // OK is enabled iff at least one target is checked anywhere in the
        // full set (not just visible ones — a user could filter, check
        // something, then change the filter so it's hidden but still selected).
        if (_okBtn != nullptr) _okBtn->Enable(!_checkedSet.empty());
    }

    struct RowEntry {
        std::string name;
        wxCheckBox* cb;
    };

    wxTextCtrl* _filterCtrl = nullptr;
    wxCheckBox* _masterCB = nullptr;
    wxScrolledWindow* _rowsWindow = nullptr;
    wxBoxSizer* _rowsSizer = nullptr;
    wxCheckBox* _copyStartChannelCB = nullptr;
    wxCheckBox* _mergeSubmodelsCB = nullptr;
    wxCheckBox* _copySizePosCB = nullptr;
    wxButton*   _okBtn = nullptr;

    std::vector<std::string> _allCandidates;
    std::set<std::string> _baseLinked;
    std::vector<RowEntry> _rows;
    std::set<std::string> _checkedSet;
};
