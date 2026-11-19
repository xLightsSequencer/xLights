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

// The desktop side of live collaboration: the ICollabApp hooks over
// xLightsFrame, the CollabController it drives, the File > Collaborate menu,
// the status bar indicator and notices, the session and join windows, and
// presence for the sequencer to draw (CollabOverlay).

#include <functional>
#include <memory>
#include <string>

#include <wx/event.h>
#include <wx/string.h>
#include <wx/timer.h>

#include "collab/CollabController.h"
#include "sequencer/CollabOverlay.h"

class xLightsFrame;
class wxMenu;
class wxMenuItem;
class wxButton;
class wxPanel;
class wxStaticText;
class CollabSessionWindow;
class CollabJoinDialog;

class DesktopCollab : public Collab::ICollabApp, public wxEvtHandler {
public:
    explicit DesktopCollab(xLightsFrame* frame);
    ~DesktopCollab() override;

    // Adds the menu items and status bar widgets (after the frame's own UI exists).
    void CreateUI(wxMenu* fileMenu, wxMenuItem* insertAfter);

    // "Live collaboration (beta)" preference.
    static bool PreferenceEnabled();
    void SetPreferenceEnabled(bool enabled);
    bool Available() const;

    // The frame asks before closing the sequence: in a session that ends it
    // (host) or leaves it (client), after asking. False keeps it open.
    bool AllowCloseSequence();
    // The open sequence is a client's session copy: plain Save becomes Save As.
    bool IsClientDocument() const;
    // App shutting down.
    void Shutdown();
    // The frame closed its sequence. Once a client's session is over this
    // removes the temporary copy of the session sequence and goes back to
    // the user's own show folder if the session had switched to a copy of
    // the host's.
    void AfterSequenceClosed();

    Collab::CollabController& Controller() { return *_ctl; }

    // ICollabApp
    SequenceElements& Sequence() override;
    bool HasSequence() override;
    bool SaveXsq(std::string& out) override;
    bool LoadXsq(const std::string& xsq) override;
    std::string ShowFolder() override;
    std::vector<std::string> AudioFiles() override;
    bool UseShowFolder(const std::string& folder) override;
    void AddMediaFolder(const std::string& folder) override;
    void SetCollaborationLock(bool locked, const std::string& reason) override;
    void OnSessionStateChanged() override;
    void OnParticipantsChanged() override;
    void OnPresenceChanged(Collab::ParticipantId who) override;
    void OnSequenceChanged(const Collab::SequenceChange& change) override;
    void MarkSequenceModified() override;
    void OnRejected(const Collab::RejectNotice& notice) override;
    void OnUndoStateChanged() override;
    void OnJoinReport(const Collab::JoinReport& report) override;
    void OnSessionsFound(const std::vector<Collab::SessionAdvert>& sessions) override;
    void OnBrowseError(const std::string& reason) override;
    void OnLog(const std::string& line) override;

    // Used by the windows.
    void HostSession();
    void JoinSession();
    void EndOrLeave();
    void ShowSessionWindow();
    void SaveCopy();
    std::string JoinReportText() const;

private:
    void UpdateVisibility();
    void UpdateStatus();
    void StartTimer();
    void OnTimer(wxTimerEvent& event);
    void GatherPresence();
    void RebuildOverlay();
    void RefreshSequencer();
    void ShowNotice(const wxString& text, const wxString& action = wxString(), std::function<void()> onAction = nullptr, int seconds = 10);
    void HideNotice();
    std::string DocFolder();
    static std::string Initials(const std::string& name);

    xLightsFrame* _frame;
    std::unique_ptr<Collab::CollabController> _ctl;
    std::shared_ptr<bool> _alive = std::make_shared<bool>(true);
    wxTimer _timer;
    int64_t _lastPresence = 0;
    int64_t _endedAt = 0;

    wxMenu* _fileMenu = nullptr;
    wxMenu* _menu = nullptr;
    wxMenuItem* _menuItem = nullptr; // File > Collaborate
    wxMenuItem* _hostItem = nullptr;
    wxMenuItem* _joinItem = nullptr;
    wxMenuItem* _endItem = nullptr;
    wxMenuItem* _windowItem = nullptr;
    size_t _menuPos = 0;
    bool _menuShown = false;

    wxButton* _statusButton = nullptr;
    wxPanel* _notice = nullptr;
    wxStaticText* _noticeText = nullptr;
    wxButton* _noticeAction = nullptr;
    std::function<void()> _noticeCallback;
    int64_t _noticeHideAt = 0;

    CollabSessionWindow* _window = nullptr;
    CollabJoinDialog* _joinDialog = nullptr;

    std::string _sequenceName; // client: the session's sequence file name
    std::string _docFolder;    // client: where its copy of the sequence lives
    bool _loading = false;     // LoadXsq is replacing the sequence
    bool _closing = false;     // ending / leaving because the sequence closes
    CollabOverlay _overlay;
    bool _overlayActive = false;
    std::string _lastBanner;
    std::string _codeShown; // host: the pairing code
    std::string _originalShowDir;              // client: show folder before UseShowFolder
    std::vector<std::string> _addedMediaFolders; // client: AddMediaFolder, removed at the end
    void RestoreShowFolder();
    void RemoveDocFolder();
};
