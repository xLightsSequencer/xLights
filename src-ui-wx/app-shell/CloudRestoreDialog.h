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
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "CloudStorageSettings.h"
#include "utils/CloudBackupStore.h"
#include "utils/ShowArchiver.h"

class wxButton;
class wxGauge;
class wxListBox;
class wxListCtrl;
class wxListEvent;
class wxStaticBitmap;
class wxStaticText;
class wxTextCtrl;

struct CloudRestoreHooks {
    std::string currentShowDir;
    std::function<void()> openSettings;
    // Switches xLights to the restored show folder.
    std::function<void(const std::string&)> openShow;
};

// Modeless window that lists the backups in the configured bucket and
// restores one into a new folder. Network and disk work runs on worker
// threads; results come back to the UI thread through CallAfter.
class CloudRestoreDialog : public wxDialog {
public:
    CloudRestoreDialog(wxWindow* parent, CloudRestoreHooks hooks);
    virtual ~CloudRestoreDialog();

    bool IsRunning() const { return _restoring; }
    // Blocks until the restore has stopped; used when xLights quits.
    void CancelAndWait();

private:
    using Backup = CloudBackup;
    enum Stage { DOWNLOADING, VERIFYING, EXTRACTING };

    void RunTask(std::function<void()> task);
    void Reload();
    void LoadShows();
    // doneStatus is shown once the list is loaded.
    void LoadBackups(const std::string& show, const wxString& doneStatus = wxString(), bool doneIsError = false);
    void LoadDetails(const Backup& b);
    void StartRestore();
    void RestoreWorker(S3Config cfg, Backup b, ShowArchiver::BackupSummary summary, std::string dest);
    void RestoreFinished(bool ok, bool cancelled, const std::string& dest, const std::string& message,
                         const std::vector<std::string>& warnings);

    std::string ShowFolderKey(const std::string& show) const;
    std::string DefaultDestination(const std::string& show, const std::string& stamp) const;
    void SetStatus(const wxString& text, bool error = false);
    void UpdateButtons();

    void OnShowSelected(wxCommandEvent& event);
    void OnBackupSelected(wxListEvent& event);
    void OnBrowse(wxCommandEvent& event);
    void OnRestore(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    void OnTimer(wxTimerEvent& event);
    bool ConfirmCancel();

    CloudRestoreHooks _hooks;
    CloudStorageSettings _settings;

    std::vector<std::thread> _threads;
    std::atomic<bool> _closing{ false };
    std::atomic<int> _generation{ 0 }; // bumped on every new listing so stale results are dropped

    std::vector<std::string> _showNames;
    std::vector<Backup> _backups;
    int _selectedBackup = -1;
    ShowArchiver::BackupSummary _summary;
    bool _destEdited = false;
    bool _deleting = false;

    std::atomic<bool> _restoring{ false };
    std::atomic<bool> _cancel{ false };
    std::atomic<int> _stage{ DOWNLOADING };
    std::atomic<uint64_t> _done{ 0 };
    std::atomic<uint64_t> _total{ 0 };
    std::chrono::steady_clock::time_point _stageStart;
    int _shownStage = -1;
    wxTimer _timer;

    wxStaticText* _storageLabel = nullptr;
    wxListBox* _shows = nullptr;
    wxListCtrl* _backupList = nullptr;
    wxStaticBitmap* _thumbnail = nullptr;
    wxStaticText* _details = nullptr;
    wxTextCtrl* _dest = nullptr;
    wxButton* _browse = nullptr;
    wxStaticText* _status = nullptr;
    wxGauge* _gauge = nullptr;
    wxTextCtrl* _log = nullptr;
    wxButton* _settingsButton = nullptr;
    wxButton* _refreshButton = nullptr;
    wxButton* _deleteButton = nullptr;
    wxButton* _restoreButton = nullptr;
    wxButton* _closeButton = nullptr;
};
