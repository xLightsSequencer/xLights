/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "BatchRenderProgressDialog.h"

#include <wx/button.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/filename.h>
#include <wx/gauge.h>
#include <wx/intl.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/stattext.h>

#include <algorithm>

namespace {
enum Column { COL_SEQUENCE = 0, COL_STATUS, COL_TIME };

bool IsDarkMode()
{
    return wxSystemSettings::GetAppearance().IsDark();
}
wxColour OkColour() { return IsDarkMode() ? wxColour(90, 200, 90) : wxColour(0, 140, 0); }
wxColour WarnColour() { return IsDarkMode() ? wxColour(240, 160, 60) : wxColour(200, 120, 0); }
wxColour FailColour() { return IsDarkMode() ? wxColour(255, 100, 100) : *wxRED; }
}

BatchRenderProgressDialog::BatchRenderProgressDialog(wxWindow* parent, const wxArrayString& files, const wxString& showDir,
                                                     std::function<void()> onStop, std::function<int()> currentProgress,
                                                     std::function<void()> onClosed) :
    wxDialog(parent, wxID_ANY, _("Batch Render"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    _timer(this),
    _showDir(showDir),
    _onStop(std::move(onStop)),
    _currentProgress(std::move(currentProgress)),
    _onClosed(std::move(onClosed))
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    _overall = new wxStaticText(this, wxID_ANY, wxEmptyString);
    sizer->Add(_overall, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, FromDIP(10));

    _current = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
    sizer->Add(_current, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, FromDIP(10));

    _gauge = new wxGauge(this, wxID_ANY, 100);
    sizer->Add(_gauge, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, FromDIP(10));

    _list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(640, 360)), wxLC_REPORT | wxLC_SINGLE_SEL);
    _list->AppendColumn(_("Sequence"), wxLIST_FORMAT_LEFT, FromDIP(360));
    _list->AppendColumn(_("Status"), wxLIST_FORMAT_LEFT, FromDIP(180));
    _list->AppendColumn(_("Time"), wxLIST_FORMAT_RIGHT, FromDIP(80));
    for (size_t i = 0; i < files.size(); ++i) {
        long row = _list->InsertItem((long)i, DisplayName(files[i]));
        _list->SetItem(row, COL_STATUS, _("Waiting"));
        _rows[files[i]] = row;
    }
    _done.assign(files.size(), false);
    sizer->Add(_list, 1, wxEXPAND | wxALL, FromDIP(10));

    auto* buttons = new wxBoxSizer(wxHORIZONTAL);
    _stopButton = new wxButton(this, wxID_ANY, _("Stop"));
    _stopButton->SetToolTip(_("Stop now. The sequence being rendered is abandoned and its fseq is not saved."));
    _copyButton = new wxButton(this, wxID_ANY, _("Copy Results"));
    _closeButton = new wxButton(this, wxID_ANY, _("Close"));
    _closeButton->Enable(false);
    buttons->Add(_stopButton, 0, wxRIGHT, FromDIP(10));
    buttons->AddStretchSpacer();
    buttons->Add(_copyButton, 0, wxRIGHT, FromDIP(10));
    buttons->Add(_closeButton, 0);
    sizer->Add(buttons, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(10));

    SetSizerAndFit(sizer);

    _stopButton->Bind(wxEVT_BUTTON, &BatchRenderProgressDialog::OnStop, this);
    _copyButton->Bind(wxEVT_BUTTON, &BatchRenderProgressDialog::OnCopy, this);
    _closeButton->Bind(wxEVT_BUTTON, &BatchRenderProgressDialog::OnCloseButton, this);
    Bind(wxEVT_CLOSE_WINDOW, &BatchRenderProgressDialog::OnCloseWindow, this);
    Bind(wxEVT_TIMER, &BatchRenderProgressDialog::OnTimer, this, _timer.GetId());

    _total.Start();
    UpdateOverall();
    _timer.Start(500);
}

BatchRenderProgressDialog::~BatchRenderProgressDialog()
{
    _timer.Stop();
}

wxString BatchRenderProgressDialog::DisplayName(const wxString& file) const
{
    wxFileName fn(file);
    if (!_showDir.IsEmpty() && fn.MakeRelativeTo(_showDir) && !fn.GetFullPath().StartsWith("..")) {
        return fn.GetFullPath();
    }
    return file;
}

long BatchRenderProgressDialog::RowFor(const wxString& file) const
{
    auto it = _rows.find(file);
    return it == _rows.end() ? -1 : it->second;
}

wxString BatchRenderProgressDialog::FormatDuration(long ms)
{
    long secs = ms / 1000;
    if (secs >= 3600) {
        return wxString::Format("%ld:%02ld:%02ld", secs / 3600, (secs / 60) % 60, secs % 60);
    }
    return wxString::Format("%ld:%02ld", secs / 60, secs % 60);
}

void BatchRenderProgressDialog::SetRendering(const wxString& file)
{
    long row = RowFor(file);
    // A retry of the same sequence keeps timing from the first attempt.
    if (row != _renderingRow || row < 0) {
        _sequence.Start();
    }
    _renderingRow = row;
    _gauge->SetValue(0);
    _current->SetLabel(wxString::Format(_("Rendering %s"), DisplayName(file)));
    if (_renderingRow >= 0) {
        _list->SetItem(_renderingRow, COL_STATUS, _("Rendering..."));
        _list->EnsureVisible(_renderingRow);
    }
    UpdateOverall();
}

void BatchRenderProgressDialog::SetResult(const wxString& file, Result result, const wxString& detail)
{
    long row = RowFor(file);
    long ms = _sequence.Time();
    if (row >= 0 && !_done[row]) {
        _done[row] = true;
        wxString status;
        wxColour colour;
        switch (result) {
        case Result::Rendered:
            status = detail.IsEmpty() ? _("Rendered") : wxString::Format(_("Rendered (%s)"), detail);
            colour = detail.IsEmpty() ? OkColour() : WarnColour();
            ++_rendered;
            _completedMs += ms;
            _list->SetItem(row, COL_TIME, FormatDuration(ms));
            break;
        case Result::Failed:
            status = detail.IsEmpty() ? _("Failed") : wxString::Format(_("Failed: %s"), detail);
            colour = FailColour();
            ++_failed;
            break;
        case Result::Stopped:
            status = _("Stopped - not rendered");
            colour = WarnColour();
            break;
        }
        _list->SetItem(row, COL_STATUS, status);
        _list->SetItemTextColour(row, colour);
    }
    if (row == _renderingRow) {
        _renderingRow = -1;
    }
    UpdateOverall();
}

void BatchRenderProgressDialog::Finished(bool stopped)
{
    _finished = true;
    _timer.Stop();
    int notStarted = 0;
    for (size_t row = 0; row < _done.size(); ++row) {
        if (!_done[row]) {
            ++notStarted;
            _list->SetItem((long)row, COL_STATUS, _("Not started"));
            _list->SetItemTextColour((long)row, wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
        }
    }
    _gauge->SetValue(0);
    wxString summary = wxString::Format(_("%s: %d rendered, %d failed"), stopped ? _("Stopped") : _("Done"), _rendered, _failed);
    if (notStarted > 0) {
        summary += wxString::Format(_(", %d not started"), notStarted);
    }
    summary += wxString::Format(_(" - total time %s"), FormatDuration(_total.Time()));
    _overall->SetLabel(summary);
    _current->SetLabel(wxEmptyString);
    _stopButton->Enable(false);
    _closeButton->Enable(true);
    Layout();
}

void BatchRenderProgressDialog::UpdateOverall()
{
    if (_finished) return;
    int total = (int)_done.size();
    int complete = 0;
    for (bool d : _done) {
        if (d) ++complete;
    }
    wxString text = wxString::Format(_("Sequence %d of %d - elapsed %s"), std::min(complete + 1, total), total, FormatDuration(_total.Time()));
    if (_rendered > 0 && complete < total) {
        long avg = _completedMs / _rendered;
        long left = avg * (total - complete) - (_renderingRow >= 0 ? std::min(avg, (long)_sequence.Time()) : 0);
        text += wxString::Format(_(" - about %s left"), FormatDuration(std::max(0L, left)));
    }
    _overall->SetLabel(text);
}

void BatchRenderProgressDialog::OnTimer(wxTimerEvent&)
{
    if (_currentProgress && _renderingRow >= 0) {
        _gauge->SetValue(std::clamp(_currentProgress(), 0, 100));
    }
    UpdateOverall();
}

void BatchRenderProgressDialog::OnStop(wxCommandEvent&)
{
    _stopButton->Enable(false);
    _current->SetLabel(_("Stopping..."));
    if (_onStop) {
        _onStop();
    }
}

void BatchRenderProgressDialog::OnCopy(wxCommandEvent&)
{
    wxString text = _overall->GetLabel() + "\n";
    for (long row = 0; row < _list->GetItemCount(); ++row) {
        text += _list->GetItemText(row, COL_SEQUENCE) + "\t" + _list->GetItemText(row, COL_STATUS) + "\t" + _list->GetItemText(row, COL_TIME) + "\n";
    }
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(text));
        wxTheClipboard->Close();
    }
}

void BatchRenderProgressDialog::OnCloseButton(wxCommandEvent&)
{
    Close();
}

void BatchRenderProgressDialog::OnCloseWindow(wxCloseEvent& event)
{
    // While running, closing only hides the window; the batch carries on.
    if (!_finished && event.CanVeto()) {
        Hide();
        event.Veto();
        return;
    }
    _timer.Stop();
    if (_onClosed) {
        _onClosed();
    }
    Destroy();
}
