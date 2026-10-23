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

#include <wx/arrstr.h>
#include <wx/dialog.h>
#include <wx/stopwatch.h>
#include <wx/timer.h>

#include <functional>
#include <map>
#include <vector>

class wxButton;
class wxGauge;
class wxListCtrl;
class wxStaticText;

// Modeless status window for a batch render: one row per sequence with its
// result, overall progress and a Stop button.
class BatchRenderProgressDialog : public wxDialog
{
public:
    enum class Result { Rendered, Failed, Stopped };

    BatchRenderProgressDialog(wxWindow* parent, const wxArrayString& files, const wxString& showDir,
                              std::function<void()> onStop, std::function<int()> currentProgress,
                              std::function<void()> onClosed);
    ~BatchRenderProgressDialog() override;

    void SetRendering(const wxString& file);
    void SetResult(const wxString& file, Result result, const wxString& detail = wxString());
    // The batch is over: anything not yet started is marked as such.
    void Finished(bool stopped);

private:
    void OnTimer(wxTimerEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnCloseButton(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    void UpdateOverall();
    long RowFor(const wxString& file) const;
    wxString DisplayName(const wxString& file) const;
    static wxString FormatDuration(long ms);

    wxListCtrl* _list = nullptr;
    wxStaticText* _current = nullptr;
    wxStaticText* _overall = nullptr;
    wxGauge* _gauge = nullptr;
    wxButton* _stopButton = nullptr;
    wxButton* _copyButton = nullptr;
    wxButton* _closeButton = nullptr;
    wxTimer _timer;
    wxStopWatch _total;
    wxStopWatch _sequence;

    wxString _showDir;
    std::map<wxString, long> _rows;
    std::vector<bool> _done;
    long _renderingRow = -1;
    long _completedMs = 0;
    int _rendered = 0;
    int _failed = 0;
    bool _finished = false;
    std::function<void()> _onStop;
    std::function<int()> _currentProgress;
    std::function<void()> _onClosed;
};
