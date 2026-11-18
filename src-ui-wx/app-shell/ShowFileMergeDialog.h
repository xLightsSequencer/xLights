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

#include <string>

#include "utils/ShowSaveMerge.h"

class wxButton;
class wxListCtrl;
class wxListEvent;
class wxStaticText;
class wxTextCtrl;

// Lists what both this session and someone else changed differently in the
// shared show files and asks which version to keep for each one. Save stays
// disabled until every conflict has an answer; Cancel saves nothing.
class ShowFileMergeDialog : public wxDialog {
public:
    ShowFileMergeDialog(wxWindow* parent, ShowSaveMerge::Plan& plan);

private:
    void Populate();
    void ShowDetail();
    void Choose(ShowFileMerge::Resolution resolution);
    void ChooseMapTarget();
    void UpdateButtons();
    void OnSelect(wxListEvent& event);

    ShowSaveMerge::Plan& _plan;
    wxListCtrl* _list = nullptr;
    wxTextCtrl* _base = nullptr;
    wxTextCtrl* _mine = nullptr;
    wxTextCtrl* _theirs = nullptr;
    wxStaticText* _status = nullptr;
    wxButton* _useMine = nullptr;
    wxButton* _useTheirs = nullptr;
    wxButton* _keepBoth = nullptr;
    wxButton* _save = nullptr;
};

// The save report: what the merge brought in and what to check.
void ShowShowFileSaveReport(wxWindow* parent, const std::string& text);
