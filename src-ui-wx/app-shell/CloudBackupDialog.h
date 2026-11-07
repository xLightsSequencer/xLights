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
#include <wx/timer.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "CloudStorageSettings.h"
#include "utils/ShowArchiver.h"

class ModelPreview;
class wxButton;
class wxGauge;
class wxStaticText;
class wxTextCtrl;

struct CloudBackupJob {
    ShowArchiveOptions archive;
    CloudStorageSettings settings;
    std::string thumbnailPng;     // empty: no thumbnail
    std::string thumbnailWarning; // why the thumbnail could not be made
};

// "12.3 MB" style size for progress and result messages.
std::string CloudFormatBytes(uint64_t bytes);

// PNG of the layout preview, at most maxWidth pixels wide. Must run on the UI
// thread. Returns an empty string and sets warning when nothing was captured.
std::string CaptureLayoutThumbnailPng(ModelPreview* preview, int maxWidth, std::string& warning);

// Modeless window that archives the show folder and uploads it on a worker
// thread, so the user can keep working. The worker only touches the atomics
// and the mutex-guarded fields below; a timer copies them to the controls.
class CloudBackupDialog : public wxDialog {
public:
    CloudBackupDialog(wxWindow* parent, CloudBackupJob job);
    virtual ~CloudBackupDialog();

    void Start();
    bool IsRunning() const { return _running; }
    // Blocks until the worker has stopped; used when xLights quits.
    void CancelAndWait();

private:
    enum Stage { SCANNING, COMPRESSING, UPLOADING, FINISHING };

    void Run();
    bool AskContinueWithStaleFseq(const std::vector<std::string>& names);
    void Finish(bool ok, bool cancelled, const std::string& message, const std::vector<std::string>& warnings);
    void SetCurrent(const std::string& s);

    void OnTimer(wxTimerEvent& event);
    void OnButton(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    bool ConfirmCancel();

    CloudBackupJob _job;
    std::thread _thread;
    std::atomic<bool> _running{ false };
    std::atomic<bool> _cancel{ false };
    std::atomic<bool> _waitingForAnswer{ false };
    std::atomic<int> _stage{ SCANNING };
    std::atomic<uint64_t> _done{ 0 };
    std::atomic<uint64_t> _total{ 0 };
    std::mutex _currentLock;
    std::string _current;

    int _shownStage = -1;
    std::chrono::steady_clock::time_point _startTime;
    std::chrono::steady_clock::time_point _stageStart;

    wxTimer _timer;
    wxStaticText* _stageText = nullptr;
    wxGauge* _gauge = nullptr;
    wxStaticText* _detail = nullptr;
    wxTextCtrl* _log = nullptr;
    wxButton* _button = nullptr;
};
