#include "FPPUploadProgressDialog.h"

#include <wx/gauge.h>
#include <wx/settings.h>
#include <wx/treectrl.h>

#include "shared/utils/wxUtilities.h"

#include <algorithm>

//(*InternalHeaders(FPPUploadProgressDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(FPPUploadProgressDialog)
const long FPPUploadProgressDialog::ID_STATICTEXT1 = wxNewId();
const long FPPUploadProgressDialog::ID_SCROLLEDWINDOW1 = wxNewId();
const long FPPUploadProgressDialog::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(FPPUploadProgressDialog,wxDialog)
    //(*EventTable(FPPUploadProgressDialog)
    //*)
END_EVENT_TABLE()

FPPUploadProgressDialog::FPPUploadProgressDialog(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(FPPUploadProgressDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;

    Create(parent, id, _("Upload Progress"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
    SetMinSize(wxSize(400,-1));
    FlexGridSizer1 = new wxFlexGridSizer(3, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    ActionLabel = new wxStaticText(this, ID_STATICTEXT1, _("Action"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(ActionLabel, 1, wxALL|wxEXPAND, 5);
    scrolledWindow = new wxScrolledWindow(this, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_SCROLLEDWINDOW1"));
    scrolledWindow->SetMinSize(wxDLG_UNIT(this,wxSize(400,0)));
    scrolledWindowSizer = new wxFlexGridSizer(0, 2, 0, 0);
    scrolledWindowSizer->AddGrowableCol(1);
    scrolledWindow->SetSizer(scrolledWindowSizer);
    FlexGridSizer1->Add(scrolledWindow, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
    CancelButton = new wxButton(this, ID_BUTTON1, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(CancelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    Fit();

    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FPPUploadProgressDialog::OnCancelButtonClick);
    //*)
    SetEscapeId(ID_BUTTON1);
    
    FlexGridSizer1->SetSizeHints(this);
    Bind(wxEVT_CLOSE_WINDOW, &FPPUploadProgressDialog::OnClose, this);
    _elapsedTimer.SetOwner(this);
    Bind(wxEVT_TIMER, [this](wxTimerEvent&) { updateOverall(); }, _elapsedTimer.GetId());
}

FPPUploadProgressDialog::~FPPUploadProgressDialog()
{
    _elapsedTimer.Stop();
    //(*Destroy(FPPUploadProgressDialog)
    //*)
}


void FPPUploadProgressDialog::OnCancelButtonClick(wxCommandEvent& event)
{
    if (_finished) {
        EndModal(_returnCode);
        return;
    }
    RequestCancel();
}

void FPPUploadProgressDialog::OnClose(wxCloseEvent& event)
{
    if (_finished) {
        EndModal(_returnCode);
        return;
    }
    // Closing mid-upload cancels it; the dialog ends once the upload unwinds.
    RequestCancel();
    if (event.CanVeto()) {
        event.Veto();
    }
}

namespace {
wxColour OkColour() {
    return IsDarkMode() ? wxColour(90, 200, 90) : wxColour(0, 140, 0);
}
wxColour WarningColour() {
    return IsDarkMode() ? wxColour(240, 170, 60) : wxColour(200, 120, 0);
}
wxColour FailedColour() {
    return IsDarkMode() ? wxColour(255, 100, 100) : *wxRED;
}
wxColour NotStartedColour() {
    return wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
}

wxString FormatDuration(long ms) {
    long secs = ms / 1000;
    if (secs >= 3600) {
        return wxString::Format("%ld:%02ld:%02ld", secs / 3600, (secs / 60) % 60, secs % 60);
    }
    return wxString::Format("%ld:%02ld", secs / 60, secs % 60);
}
}

void FPPUploadProgressDialog::setSequences(const std::vector<std::string>& names)
{
    auto* sizer = dynamic_cast<wxFlexGridSizer*>(GetSizer());
    // Built even with no sequences: a settings-only upload still reports its
    // steps and controller messages here.
    if (sizer == nullptr || _completedTree != nullptr) return;
    _sequenceNames = names;
    _sequenceDone.assign(names.size(), false);

    _sequenceHeader = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
    _sequenceHeader->SetFont(_sequenceHeader->GetFont().Bold().Larger());
    _completedLabel = new wxStaticText(this, wxID_ANY, wxEmptyString);
    _completedTree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(520, 160)),
                                    wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT | wxTR_SINGLE);
    _completedTree->AddRoot(wxEmptyString);
    _overallLabel = new wxStaticText(this, wxID_ANY, wxEmptyString);

    // Rows: header, action, controller gauges, completed label, completed tree, overall, buttons.
    sizer->SetRows(0);
    sizer->RemoveGrowableRow(1);
    sizer->Insert(0, _sequenceHeader, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 5);
    sizer->Insert(3, _completedLabel, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 5);
    sizer->Insert(4, _completedTree, 1, wxALL | wxEXPAND, 5);
    sizer->Insert(5, _overallLabel, 0, wxLEFT | wxRIGHT | wxEXPAND, 5);
    sizer->AddGrowableRow(2, 1);
    sizer->AddGrowableRow(4, 1);
    const int gaugeRows = std::clamp((int)scrolledWindowSizer->GetItemCount() / 2, 1, 6);
    scrolledWindow->SetMinSize(wxSize(scrolledWindow->GetMinSize().GetWidth(), gaugeRows * FromDIP(28)));

    _totalTime.Start();
    _elapsedTimer.Start(1000);
    if (names.empty()) {
        _sequenceHeader->SetLabel(_("Getting ready to upload"));
    } else {
        _sequenceHeader->SetLabel(wxString::Format(wxPLURAL("Getting ready to upload %d sequence", "Getting ready to upload %d sequences", (int)names.size()), (int)names.size()));
    }
    _completedLabel->SetLabel(_("Completed"));
    updateOverall();
}

void FPPUploadProgressDialog::startSequence(size_t index)
{
    if (_sequenceHeader == nullptr || index >= _sequenceNames.size()) return;
    _sequenceTime.Start();
    if (_firstSequenceStartMs < 0) {
        _firstSequenceStartMs = _totalTime.Time();
    }
    _sequenceHeader->SetLabel(wxString::Format(_("Uploading %d/%d: %s"), (int)index + 1, (int)_sequenceNames.size(),
                                               wxString::FromUTF8(_sequenceNames[index])));
    updateOverall();
}

void FPPUploadProgressDialog::finishSequence(size_t index, SequenceState state, const std::string& detail, const std::vector<ControllerResult>& controllers)
{
    if (_completedTree == nullptr || index >= _sequenceDone.size() || _sequenceDone[index]) return;
    _sequenceDone[index] = true;
    ++_completedCount;

    wxString status;
    wxColour colour;
    switch (state) {
    case SequenceState::Uploaded:
        status = wxString::Format(_("Uploaded (%s)"), FormatDuration(_sequenceTime.Time()));
        colour = OkColour();
        ++_uploadedCount;
        break;
    case SequenceState::UpToDate:
        status = _("Already up to date");
        colour = OkColour();
        ++_upToDateCount;
        break;
    case SequenceState::Failed:
        status = detail.empty() ? _("Failed") : wxString::Format(_("Failed: %s"), wxString::FromUTF8(detail));
        colour = FailedColour();
        ++_failedCount;
        break;
    case SequenceState::Cancelled:
        status = _("Cancelled");
        colour = WarningColour();
        ++_cancelledCount;
        break;
    }
    addCompleted(wxString::FromUTF8(_sequenceNames[index]) + " - " + status, colour, controllers, state == SequenceState::Failed);
    _completedLabel->SetLabel(wxString::Format(_("Completed (%d of %d sequences)"), _completedCount, (int)_sequenceNames.size()));
    updateOverall();
}

void FPPUploadProgressDialog::startStep(const std::string& title)
{
    if (_sequenceHeader != nullptr) {
        _sequenceHeader->SetLabel(wxString::FromUTF8(title));
    }
    ActionLabel->SetLabel(wxEmptyString);
}

void FPPUploadProgressDialog::finishStep(const std::string& title, const std::string& status, bool warning, const std::vector<ControllerResult>& controllers)
{
    if (_completedTree == nullptr) return;
    bool failed = std::any_of(controllers.begin(), controllers.end(), [](auto const& c) { return c.failed; });
    wxColour colour = failed ? FailedColour() : (warning ? WarningColour() : OkColour());
    addCompleted(wxString::FromUTF8(title) + " - " + wxString::FromUTF8(status), colour, controllers, failed || warning);
}

void FPPUploadProgressDialog::addCompleted(const wxString& text, const wxColour& colour, const std::vector<ControllerResult>& controllers, bool expand)
{
    wxTreeItemId item = _completedTree->AppendItem(_completedTree->GetRootItem(), text);
    _completedTree->SetItemTextColour(item, colour);
    for (auto const& c : controllers) {
        wxString label = wxString::FromUTF8(c.name);
        if (!c.status.empty()) {
            label += ": " + wxString::FromUTF8(c.status);
        }
        wxTreeItemId child = _completedTree->AppendItem(item, label);
        if (c.failed) {
            _completedTree->SetItemTextColour(child, FailedColour());
        }
        for (auto const& d : c.details) {
            _completedTree->AppendItem(child, wxString::FromUTF8(d));
        }
        if (expand && !c.details.empty()) {
            _completedTree->Expand(child);
        }
    }
    if (expand && !controllers.empty()) {
        _completedTree->Expand(item);
    }
    _completedTree->EnsureVisible(item);
}

void FPPUploadProgressDialog::updateOverall()
{
    if (_overallLabel == nullptr || _finished) return;
    int total = (int)_sequenceDone.size();
    wxString text = total == 0 ? wxString::Format(_("Elapsed %s"), FormatDuration(_totalTime.Time()))
                               : wxString::Format(_("%d of %d done - elapsed %s"), _completedCount, total, FormatDuration(_totalTime.Time()));
    if (_completedCount > 0 && _completedCount < total && _firstSequenceStartMs >= 0) {
        long avg = (_totalTime.Time() - _firstSequenceStartMs) / _completedCount;
        text += wxString::Format(_(" - about %s left"), FormatDuration(avg * (total - _completedCount)));
    }
    _overallLabel->SetLabel(text);
    _overallLabel->Update();
}

void FPPUploadProgressDialog::finish(bool wasCancelled, int returnCode, bool stayOpen)
{
    _returnCode = returnCode;
    if (!stayOpen) {
        _finished = true;
        _elapsedTimer.Stop();
        EndModal(returnCode);
        return;
    }
    int notStarted = 0;
    if (_completedTree != nullptr) {
        for (size_t i = 0; i < _sequenceDone.size(); ++i) {
            if (!_sequenceDone[i]) {
                ++notStarted;
                wxTreeItemId item = _completedTree->AppendItem(_completedTree->GetRootItem(),
                                                               wxString::FromUTF8(_sequenceNames[i]) + " - " + _("Not started"));
                _completedTree->SetItemTextColour(item, NotStartedColour());
            }
        }
    }
    _finished = true;
    _elapsedTimer.Stop();
    wxString summary;
    if (_sequenceDone.empty()) {
        summary = wxString::Format(_("Total time %s"), FormatDuration(_totalTime.Time()));
    } else {
        summary = wxString::Format(_("%d uploaded, %d already up to date, %d failed"), _uploadedCount, _upToDateCount, _failedCount);
        if (_cancelledCount + notStarted > 0) {
            summary += wxString::Format(_(", %d not uploaded"), _cancelledCount + notStarted);
        }
        summary += wxString::Format(_(" - total time %s"), FormatDuration(_totalTime.Time()));
    }
    if (_overallLabel != nullptr) {
        _overallLabel->SetLabel(summary);
    }
    if (_sequenceHeader != nullptr) {
        _sequenceHeader->SetLabel(wasCancelled ? _("Upload cancelled") : _("Upload complete"));
    }
    ActionLabel->SetLabel(wxEmptyString);
    CancelButton->SetLabelText(_("Close"));
    CancelButton->Enable(true);
    Layout();
}

void FPPUploadProgressDialog::RequestCancel()
{
    CancelButton->SetLabelText("Canceling...");
    CancelButton->Enable(false);
    cancelled = true;
}

void FPPUploadProgressDialog::setActionLabel(const std::string &action) {
    ActionLabel->SetLabel(action);
    ActionLabel->Update();
}

wxGauge *FPPUploadProgressDialog::addGauge(const std::string &name) {
    wxStaticText *label = new wxStaticText(scrolledWindow, wxID_ANY, name);
    scrolledWindowSizer->Add(label,1, wxALL |wxEXPAND,3);
    wxGauge *g = new wxGauge(scrolledWindow, wxID_ANY, 100);
    g->SetValue(0);
    g->SetMinSize(wxSize(200, -1));
    scrolledWindowSizer->Add(g, 1, wxALL |wxEXPAND,3);
    g->SetRange(1000);
    return g;
}
