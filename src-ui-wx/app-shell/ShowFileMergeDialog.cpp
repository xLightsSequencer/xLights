/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowFileMergeDialog.h"

#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "diagnostics/SeqElementMismatchDialog.h"

using ShowFileMerge::Resolution;

namespace {

bool IsDeletion(const ShowFileMerge::Conflict& c) {
    return c.kind == ShowFileMerge::ConflictKind::DeletedWithEffects;
}

wxString ChoiceText(const ShowFileMerge::Conflict& c, const ShowFileMergeDialog::Labels& labels) {
    switch (c.resolution) {
    case Resolution::Mine: return IsDeletion(c) ? wxString("Keep model") : wxString::FromUTF8(labels.mine);
    case Resolution::Theirs: return IsDeletion(c) ? wxString("Delete") : wxString::FromUTF8(labels.theirs);
    case Resolution::KeepBoth: return "Keep both";
    case Resolution::Map: return wxString::FromUTF8("Map to " + c.mapTarget);
    case Resolution::Unresolved: break;
    }
    return "?";
}

wxTextCtrl* ValueBox(wxWindow* parent) {
    return new wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(220, 90),
                          wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
}

} // namespace

ShowFileMergeDialog::ShowFileMergeDialog(wxWindow* parent, ShowSaveMerge::Plan& plan, const Labels& labels) :
    wxDialog(parent, wxID_ANY, "Merge Changes", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    _plan(plan),
    _labels(labels) {
    auto* top = new wxBoxSizer(wxVERTICAL);
    top->Add(new wxStaticText(this, wxID_ANY, wxString::FromUTF8(_labels.intro)), 0, wxALL | wxEXPAND, 8);

    _list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(720, 220), wxLC_REPORT | wxLC_SINGLE_SEL);
    _list->AppendColumn("File", wxLIST_FORMAT_LEFT, 150);
    _list->AppendColumn("Item", wxLIST_FORMAT_LEFT, 170);
    _list->AppendColumn("What changed", wxLIST_FORMAT_LEFT, 310);
    _list->AppendColumn("Keep", wxLIST_FORMAT_LEFT, 80);
    top->Add(_list, 1, wxLEFT | wxRIGHT | wxEXPAND, 8);

    auto* values = new wxFlexGridSizer(2, 3, 4, 8);
    values->AddGrowableCol(0);
    values->AddGrowableCol(1);
    values->AddGrowableCol(2);
    values->AddGrowableRow(1);
    values->Add(new wxStaticText(this, wxID_ANY, "Before either change"));
    values->Add(new wxStaticText(this, wxID_ANY, wxString::FromUTF8(_labels.mineColumn)));
    values->Add(new wxStaticText(this, wxID_ANY, wxString::FromUTF8(_labels.theirsColumn)));
    _base = ValueBox(this);
    _mine = ValueBox(this);
    _theirs = ValueBox(this);
    values->Add(_base, 1, wxEXPAND);
    values->Add(_mine, 1, wxEXPAND);
    values->Add(_theirs, 1, wxEXPAND);
    top->Add(values, 0, wxALL | wxEXPAND, 8);

    auto* pick = new wxBoxSizer(wxHORIZONTAL);
    _useMine = new wxButton(this, wxID_ANY, wxString::FromUTF8("Keep " + _labels.mine));
    _useTheirs = new wxButton(this, wxID_ANY, wxString::FromUTF8("Keep " + _labels.theirs));
    _keepBoth = new wxButton(this, wxID_ANY, "Keep Both");
    auto* allMine = new wxButton(this, wxID_ANY, wxString::FromUTF8("All " + _labels.mine));
    auto* allTheirs = new wxButton(this, wxID_ANY, wxString::FromUTF8("All " + _labels.theirs));
    pick->Add(_useMine, 0, wxRIGHT, 4);
    pick->Add(_useTheirs, 0, wxRIGHT, 4);
    pick->Add(_keepBoth, 0, wxRIGHT, 16);
    pick->AddStretchSpacer();
    pick->Add(allMine, 0, wxRIGHT, 4);
    pick->Add(allTheirs, 0);
    top->Add(pick, 0, wxLEFT | wxRIGHT | wxEXPAND, 8);

    auto* bottom = new wxBoxSizer(wxHORIZONTAL);
    _status = new wxStaticText(this, wxID_ANY, wxEmptyString);
    bottom->Add(_status, 1, wxALIGN_CENTER_VERTICAL);
    _save = new wxButton(this, wxID_OK, wxString::FromUTF8(_labels.ok));
    bottom->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxRIGHT, 4);
    bottom->Add(_save, 0);
    top->Add(bottom, 0, wxALL | wxEXPAND, 8);

    SetSizerAndFit(top);
    CentreOnParent();

    _list->Bind(wxEVT_LIST_ITEM_SELECTED, &ShowFileMergeDialog::OnSelect, this);
    _useMine->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { Choose(Resolution::Mine); });
    _useTheirs->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { Choose(Resolution::Theirs); });
    _keepBoth->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        long sel = _list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        auto refs = _plan.Conflicts();
        if (sel >= 0 && (size_t)sel < refs.size() && IsDeletion(_plan.GetConflict(refs[sel]))) {
            ChooseMapTarget();
        } else {
            Choose(Resolution::KeepBoth);
        }
    });
    allMine->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        _plan.ResolveAll(Resolution::Mine);
        Populate();
    });
    allTheirs->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        _plan.ResolveAll(Resolution::Theirs);
        Populate();
    });

    Populate();
    if (_list->GetItemCount() > 0) {
        _list->SetItemState(0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    }
}

void ShowFileMergeDialog::Populate() {
    long selected = _list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    // A decision on a controller can change the layout's conflicts, so the
    // list is rebuilt from the plan each time.
    _list->Freeze();
    _list->DeleteAllItems();
    long row = 0;
    for (const auto& ref : _plan.Conflicts()) {
        auto t = _plan.Describe(ref);
        _list->InsertItem(row, wxString::FromUTF8(t.fileName));
        _list->SetItem(row, 1, wxString::FromUTF8(t.record));
        _list->SetItem(row, 2, wxString::FromUTF8(t.what));
        _list->SetItem(row, 3, ChoiceText(_plan.GetConflict(ref), _labels));
        ++row;
    }
    _list->Thaw();
    if (selected >= 0 && selected < _list->GetItemCount()) {
        _list->SetItemState(selected, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    }
    ShowDetail();
    UpdateButtons();
}

void ShowFileMergeDialog::ShowDetail() {
    long sel = _list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    auto refs = _plan.Conflicts();
    if (sel < 0 || (size_t)sel >= refs.size()) {
        _base->Clear();
        _mine->Clear();
        _theirs->Clear();
        return;
    }
    auto t = _plan.Describe(refs[sel]);
    _base->ChangeValue(wxString::FromUTF8(t.base));
    _mine->ChangeValue(wxString::FromUTF8(t.mine));
    _theirs->ChangeValue(wxString::FromUTF8(t.theirs));
}

void ShowFileMergeDialog::Choose(Resolution resolution) {
    long sel = _list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    auto refs = _plan.Conflicts();
    if (sel < 0 || (size_t)sel >= refs.size()) {
        return;
    }
    _plan.Resolve(refs[sel], resolution);
    Populate();
    // move on to the next undecided one
    refs = _plan.Conflicts();
    for (size_t i = 0; i < refs.size(); ++i) {
        size_t idx = ((size_t)sel + 1 + i) % refs.size();
        if (_plan.GetConflict(refs[idx]).resolution == Resolution::Unresolved) {
            _list->SetItemState((long)idx, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
            _list->EnsureVisible((long)idx);
            break;
        }
    }
}

// Someone else deleted a model the open sequence has effects on: the same
// choices as opening a sequence whose model is missing, through the same
// dialog, with Keep first.
void ShowFileMergeDialog::ChooseMapTarget() {
    long sel = _list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    auto refs = _plan.Conflicts();
    if (sel < 0 || (size_t)sel >= refs.size()) {
        return;
    }
    const auto& c = _plan.GetConflict(refs[sel]);
    auto targets = _plan.MapTargets();
    if (targets.empty()) {
        wxMessageBox("There is no model or group without a row in the open sequence to move the effects to.", "Map Effects", wxOK | wxICON_INFORMATION, this);
        return;
    }
    SeqElementMismatchDialog dialog(this);
    dialog.SetTitle("Map Effects");
    dialog.StaticTextMessage->SetLabel(wxString::FromUTF8("Someone else deleted '" + c.mineName + "'.\nIts effects in the open sequence can move to:"));
    wxArrayString names;
    for (const auto& t : targets) {
        names.push_back(wxString::FromUTF8(t));
    }
    dialog.ChoiceModels->Set(names);
    dialog.ChoiceModels->SetSelection(0);
    if (!c.mapTarget.empty()) {
        dialog.ChoiceModels->SetStringSelection(wxString::FromUTF8(c.mapTarget));
    }
    dialog.RadioButtonRename->SetValue(true);
    dialog.RadioButtonDelete->Hide();
    dialog.RadioButtonMap->Hide();
    dialog.CheckBoxAddAlias->Hide();
    dialog.Layout();
    dialog.Fit();
    if (dialog.ShowModal() != wxID_OK) {
        return;
    }
    const std::string target = dialog.ChoiceModels->GetStringSelection().ToStdString(wxConvUTF8);
    if (!_plan.Resolve(refs[sel], Resolution::Map, target)) {
        return;
    }
    Populate();
}

void ShowFileMergeDialog::UpdateButtons() {
    long sel = _list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    auto refs = _plan.Conflicts();
    const bool has = sel >= 0 && (size_t)sel < refs.size();
    const bool deletion = has && IsDeletion(_plan.GetConflict(refs[sel]));
    _useMine->SetLabel(deletion ? wxString("Keep Model") : wxString::FromUTF8("Keep " + _labels.mine));
    _useTheirs->SetLabel(deletion ? wxString("Delete Model && Effects") : wxString::FromUTF8("Keep " + _labels.theirs));
    _keepBoth->SetLabel(deletion ? "Map Effects To..." : "Keep Both");
    Layout();
    _useMine->Enable(has);
    _useTheirs->Enable(has);
    _keepBoth->Enable(has && (_plan.GetConflict(refs[sel]).keepBothAllowed || deletion));
    size_t open = 0;
    for (const auto& r : refs) {
        if (_plan.GetConflict(r).resolution == Resolution::Unresolved) ++open;
    }
    _save->Enable(open == 0);
    _status->SetLabel(open == 0 ? wxString("Every conflict has a choice.")
                                : wxString::Format("%d of %d still to choose.", (int)open, (int)refs.size()));
}

void ShowFileMergeDialog::OnSelect(wxListEvent&) {
    ShowDetail();
    UpdateButtons();
}

void ShowShowFileSaveReport(wxWindow* parent, const std::string& text) {
    wxDialog dlg(parent, wxID_ANY, "Saved with Someone Else's Changes", wxDefaultPosition, wxDefaultSize,
                 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    auto* body = new wxTextCtrl(&dlg, wxID_ANY, wxString::FromUTF8(text), wxDefaultPosition, wxSize(620, 300),
                                wxTE_MULTILINE | wxTE_READONLY);
    sizer->Add(body, 1, wxALL | wxEXPAND, 8);
    sizer->Add(dlg.CreateStdDialogButtonSizer(wxOK), 0, wxALL | wxEXPAND, 8);
    dlg.SetSizerAndFit(sizer);
    dlg.CentreOnParent();
    dlg.ShowModal();
}
