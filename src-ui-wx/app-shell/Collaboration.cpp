/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "Collaboration.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/config.h>
#include <wx/dcmemory.h>
#include <wx/dialog.h>
#include <wx/filename.h>
#include <wx/imaglist.h>
#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/dirdlg.h>
#include <wx/stdpaths.h>
#include <wx/textctrl.h>
#include <wx/textdlg.h>
#include <wx/time.h>
#include <wx/utils.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <pugixml.hpp>

#include "xLightsMain.h"
#include "xLightsApp.h"
#include "layout/LayoutPanel.h"
#include "models/BaseObject.h"
#include "models/Model.h"
#include "models/SubModel.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "collab/CollabCommandInfo.h"
#include "collab/CollabManifest.h"
#include "render/Element.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "sequencer/EffectsGrid.h"
#include "sequencer/EffectsPanel.h"
#include "sequencer/MainSequencer.h"
#include "sequencer/RowHeading.h"
#include "sequencer/TimeLine.h"
#include "shared/utils/wxUtilities.h"
#include "settings/XLightsConfigAdapter.h"
#include "utils/ExternalHooks.h"
#include "utils/FileUtils.h"
#include "xLightsVersion.h"

#include <log.h>

namespace {

// The session's timers (retries, stalls, presence, banners) need a clock that
// a wall-clock change can't move.
int64_t NowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

const char* DeviceKind() {
#if defined(__WXOSX__)
    return "mac";
#elif defined(__WXMSW__)
    return "windows";
#else
    return "linux";
#endif
}

wxString DisplayName(wxWindow* parent) {
    wxConfigBase* config = wxConfigBase::Get();
    wxString name = config != nullptr ? config->Read("CollabDisplayName", wxGetUserName()) : wxGetUserName();
    if (name.empty()) {
        name = wxGetHostName();
    }
    wxString chosen = wxGetTextFromUser("Your name, as the others in the session will see it:", "Live Collaboration", name, parent);
    chosen.Trim().Trim(false);
    if (!chosen.empty() && config != nullptr) {
        config->Write("CollabDisplayName", chosen);
        config->Flush();
    }
    return chosen;
}

// Every effect layer of the sequence's rows, submodels and strands.
template<class F>
void ForEachLayer(SequenceElements& seq, F&& fn) {
    for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
        Element* el = seq.GetElement(i, MASTER_VIEW);
        if (el == nullptr) {
            continue;
        }
        for (size_t l = 0; l < el->GetEffectLayerCount(); ++l) {
            fn(el, el->GetEffectLayer((int)l));
        }
        if (auto* me = dynamic_cast<ModelElement*>(el)) {
            for (int s = 0; s < me->GetSubModelAndStrandCount(); ++s) {
                Element* sub = me->GetSubModel(s);
                for (size_t l = 0; sub != nullptr && l < sub->GetEffectLayerCount(); ++l) {
                    fn(sub, sub->GetEffectLayer((int)l));
                }
            }
        }
    }
}

} // namespace

// ---------------------------------------------------------------------------
// Join dialog
// ---------------------------------------------------------------------------

class CollabJoinDialog : public wxDialog {
public:
    explicit CollabJoinDialog(wxWindow* parent) : wxDialog(parent, wxID_ANY, "Join Collaboration Session", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
        auto* sizer = new wxBoxSizer(wxVERTICAL);
#ifdef __WXOSX__
        sizer->Add(new wxStaticText(this, wxID_ANY, "Sessions on this network or in your SharePlay call:"), 0, wxALL, 8);
#else
        sizer->Add(new wxStaticText(this, wxID_ANY, "Sessions on this network:"), 0, wxALL, 8);
#endif
        _list = new wxListBox(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(420, 160)));
        sizer->Add(_list, 1, wxLEFT | wxRIGHT | wxEXPAND, 8);
        _error = new wxStaticText(this, wxID_ANY, wxEmptyString);
        _error->SetForegroundColour(wxColour(0xC0, 0x30, 0x30));
        sizer->Add(_error, 0, wxLEFT | wxRIGHT | wxTOP, 8);
        auto* row = new wxBoxSizer(wxHORIZONTAL);
        _codeLabel = new wxStaticText(this, wxID_ANY, "Pairing code:");
        row->Add(_codeLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        _code = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(100, -1)));
        _code->SetMaxLength(6);
        row->Add(_code, 0, wxALIGN_CENTER_VERTICAL);
        sizer->Add(row, 0, wxALL, 8);
        if (Collab::LanJoinByAddressAvailable()) {
            // For a host this computer can't find by browsing (no network
            // discovery on one side, or a network that blocks it).
            auto* addrRow = new wxBoxSizer(wxHORIZONTAL);
            addrRow->Add(new wxStaticText(this, wxID_ANY, "Or join by address:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
            _address = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(200, -1)));
            _address->SetHint("192.168.1.20:50000");
            _address->SetToolTip("The address the host's session window shows. Only hosts on Windows or Linux can be joined this way.");
            addrRow->Add(_address, 1, wxALIGN_CENTER_VERTICAL);
            sizer->Add(addrRow, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 8);
            _address->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { UpdateCode(); });
        }
        _fetch = new wxCheckBox(this, wxID_ANY, "Fetch show files that differ from the host's");
        _fetch->SetToolTip("Copies the host's layout, audio and media where they differ from yours. Your show folder is never changed.");
        sizer->Add(_fetch, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
        auto* buttons = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
        if (auto* ok = dynamic_cast<wxButton*>(FindWindow(wxID_OK))) {
            ok->SetLabel("Join");
        }
        sizer->Add(buttons, 0, wxALL | wxEXPAND, 8);
        SetSizerAndFit(sizer);
        Bind(wxEVT_BUTTON, &CollabJoinDialog::OnOK, this, wxID_OK);
        _list->Bind(wxEVT_LISTBOX, [this](wxCommandEvent&) { UpdateCode(); });
        _list->Bind(wxEVT_LISTBOX_DCLICK, [this](wxCommandEvent&) {
            if (SelectedIsGroup()) {
                wxCommandEvent ok(wxEVT_BUTTON, wxID_OK);
                ok.SetEventObject(this);
                ProcessWindowEvent(ok);
            } else {
                _code->SetFocus();
            }
        });
    }

    // `groupId`: the offered SharePlay session among `sessions` (empty: none).
    void SetSessions(const std::vector<Collab::SessionAdvert>& sessions, const std::string& groupId = std::string()) {
        std::string selected = Selected() ? Selected()->id : _preferred;
        _sessions = sessions;
        _groupId = groupId;
        _list->Clear();
        int sel = -1;
        for (size_t i = 0; i < _sessions.size(); ++i) {
            const auto& s = _sessions[i];
            std::string label = s.sequenceName + "  \xE2\x80\x94  " + s.hostName;
            if (!_groupId.empty() && s.id == _groupId) {
                label += "  (SharePlay)";
            }
            _list->Append(wxString::FromUTF8(label));
            if (s.id == selected || (selected.empty() && i == 0)) {
                sel = (int)i;
            }
        }
        if (sel >= 0) {
            _list->SetSelection(sel);
        }
        _error->SetLabel(_sessions.empty() ? "Looking for sessions..." : "");
        UpdateCode();
        Layout();
    }
    // Select this session once it is listed.
    void Prefer(const std::string& id) { _preferred = id; }
    bool SelectedIsGroup() const { return Selected() != nullptr && !_groupId.empty() && Selected()->id == _groupId; }
    void SetError(const wxString& e) {
        _error->SetLabel(e);
        Layout();
    }
    const Collab::SessionAdvert* Selected() const {
        if (!Address().empty()) {
            std::string id;
            if (!Collab::SessionIdForAddress(Address(), id)) {
                return nullptr;
            }
            _byAddress.id = id;
            _byAddress.hostName = Address();
            return &_byAddress;
        }
        int s = _list != nullptr ? _list->GetSelection() : wxNOT_FOUND;
        return s == wxNOT_FOUND || s >= (int)_sessions.size() ? nullptr : &_sessions[s];
    }
    std::string Address() const { return _address != nullptr ? _address->GetValue().Trim().Trim(false).utf8_string() : std::string(); }
    std::string Code() const { return _code->GetValue().Trim().Trim(false).utf8_string(); }
    bool Fetch() const { return _fetch->IsChecked(); }

private:
    void OnOK(wxCommandEvent& e) {
        if (Selected() == nullptr) {
            SetError(Address().empty() ? "Choose a session." : "Enter the address as the host shows it, like 192.168.1.20:50000.");
            return;
        }
        if (!SelectedIsGroup() && Code().size() != 6) {
            SetError("Enter the 6-digit code the host shows.");
            return;
        }
        e.Skip();
    }

    // A SharePlay session is limited to the people in the call: no code.
    void UpdateCode() {
        bool group = SelectedIsGroup();
        _codeLabel->Enable(!group);
        _code->Enable(!group);
    }

    wxListBox* _list = nullptr;
    wxStaticText* _error = nullptr;
    wxStaticText* _codeLabel = nullptr;
    wxTextCtrl* _code = nullptr;
    wxTextCtrl* _address = nullptr;
    wxCheckBox* _fetch = nullptr;
    mutable Collab::SessionAdvert _byAddress;
    std::vector<Collab::SessionAdvert> _sessions;
    std::string _groupId;
    std::string _preferred;
};

// ---------------------------------------------------------------------------
// Session window (modeless)
// ---------------------------------------------------------------------------

class CollabSessionWindow : public wxDialog {
public:
    CollabSessionWindow(wxWindow* parent, DesktopCollab& collab) :
        wxDialog(parent, wxID_ANY, "Live Collaboration", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), _collab(collab) {
        auto* sizer = new wxBoxSizer(wxVERTICAL);
        _status = new wxStaticText(this, wxID_ANY, wxEmptyString);
        wxFont bold = _status->GetFont();
        bold.MakeBold();
        _status->SetFont(bold);
        sizer->Add(_status, 0, wxALL, 8);
        _codeLabel = new wxStaticText(this, wxID_ANY, "Pairing code - give it to the people joining:");
        sizer->Add(_codeLabel, 0, wxLEFT | wxRIGHT, 8);
        _code = new wxStaticText(this, wxID_ANY, wxEmptyString);
        wxFont big = _code->GetFont();
        big.SetPointSize(big.GetPointSize() * 3);
        big.MakeBold();
        _code->SetFont(big);
        sizer->Add(_code, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 8);
        _addresses = new wxStaticText(this, wxID_ANY, wxEmptyString);
        sizer->Add(_addresses, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
        _people = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(520, 140)), wxLC_REPORT | wxLC_SINGLE_SEL);
        _people->AppendColumn("Name", wxLIST_FORMAT_LEFT, FromDIP(200));
        _people->AppendColumn("Device", wxLIST_FORMAT_LEFT, FromDIP(70));
        _people->AppendColumn("Role", wxLIST_FORMAT_LEFT, FromDIP(70));
        _people->AppendColumn(Collab::Labels::AssignedRows, wxLIST_FORMAT_LEFT, FromDIP(170));
        _images = new wxImageList(FromDIP(12), FromDIP(12), false);
        _people->SetImageList(_images, wxIMAGE_LIST_SMALL);
        _people->Bind(wxEVT_LIST_ITEM_SELECTED, [this](wxListEvent&) { UpdateRoleButton(); });
        _people->Bind(wxEVT_LIST_ITEM_DESELECTED, [this](wxListEvent&) { UpdateRoleButton(); });
        sizer->Add(_people, 1, wxLEFT | wxRIGHT | wxEXPAND, 8);
        // The host's role controls.
        auto* roles = new wxBoxSizer(wxHORIZONTAL);
        _roleButton = new wxButton(this, wxID_ANY, Collab::Labels::MakeViewer);
        _roleButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { ToggleRole(); });
        roles->Add(_roleButton, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 8);
        _joinViewers = new wxCheckBox(this, wxID_ANY, Collab::Labels::JoinAsViewers);
        _joinViewers->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& e) {
            _collab.Controller().SetJoinRole(e.IsChecked() ? Collab::ParticipantRole::Viewer : Collab::ParticipantRole::Editor);
        });
        roles->Add(_joinViewers, 0, wxALIGN_CENTER_VERTICAL);
        sizer->Add(roles, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 8);
        _report = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(380, 90)), wxTE_MULTILINE | wxTE_READONLY);
        sizer->Add(_report, 0, wxALL | wxEXPAND, 8);
        auto* buttons = new wxBoxSizer(wxHORIZONTAL);
        _end = new wxButton(this, wxID_ANY, "End Session");
        _end->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { _collab.EndOrLeave(); });
        buttons->Add(_end, 0, wxRIGHT, 6);
        _saveCopy = new wxButton(this, wxID_ANY, "Save a Copy...");
        _saveCopy->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { _collab.SaveCopy(); });
        buttons->Add(_saveCopy, 0, wxRIGHT, 6);
        buttons->AddStretchSpacer();
        auto* close = new wxButton(this, wxID_CLOSE, "Close");
        close->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { Hide(); });
        buttons->Add(close, 0);
        sizer->Add(buttons, 0, wxALL | wxEXPAND, 8);
        SetSizerAndFit(sizer);
        Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent&) { Hide(); });
    }

    void ShowState(const std::string& code) {
        auto& ctl = _collab.Controller();
        using Role = Collab::CollabController::Role;
        using State = Collab::CollabController::State;
        _status->SetLabel(wxString::FromUTF8(ctl.StatusText()));
        bool host = ctl.GetRole() == Role::Host;
        bool live = ctl.GetState() != State::Ended && ctl.GetState() != State::Idle;
        bool group = ctl.SessionNetwork() == Collab::CollabController::Network::Group;
        _codeLabel->SetLabel(group ? "Shared over SharePlay - everyone in the call can join:" : "Pairing code - give it to the people joining:");
        _codeLabel->Show(host && live);
        _code->Show(host && live);
        _code->SetLabel(group ? wxString("SharePlay") : wxString::FromUTF8(code));
        const std::string& addrs = _collab.JoinAddressesText();
        _addresses->SetLabel(wxString::FromUTF8("If the session isn't listed for someone, they can join by address: " + addrs));
        _addresses->Show(host && live && !group && !addrs.empty());
        _end->SetLabel(host ? "End Session" : "Leave Session");
        _end->Show(live);
        _saveCopy->Show((!host || ctl.SaveAsOnly()) && !live && ctl.GetJoinReport().has_value() && ctl.GetJoinReport()->sessionCopy);
        std::string report = _collab.JoinReportText();
        _report->ChangeValue(wxString::FromUTF8(report));
        _report->Show(!host && !report.empty());

        Collab::ParticipantId selected = SelectedParticipant();
        _people->DeleteAllItems();
        _images->RemoveAll();
        _ids.clear();
        int row = 0;
        for (const auto& p : ctl.Participants()) {
            wxBitmap swatch(_images->GetSize().x, _images->GetSize().y);
            {
                wxMemoryDC dc(swatch);
                dc.SetBackground(wxBrush(xlColorToWxColour(p.colour)));
                dc.Clear();
            }
            int img = _images->Add(swatch);
            std::string name = p.name + (p.isLocal ? " (you)" : "") + (p.isHost ? " - host" : "");
            long item = _people->InsertItem(row++, wxString::FromUTF8(name), img);
            _people->SetItem(item, 1, wxString::FromUTF8(p.deviceKind));
            _people->SetItem(item, 2, p.role == Collab::ParticipantRole::Viewer ? Collab::Labels::Viewer : Collab::Labels::Editor);
            std::string rows;
            for (const auto& r : p.assignedRows) {
                rows += (rows.empty() ? "" : ", ") + r;
            }
            _people->SetItem(item, 3, wxString::FromUTF8(rows));
            _ids.push_back(p.id);
            if (p.id == selected) {
                _people->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            }
        }
        bool active = ctl.GetState() == State::Active;
        _roleButton->Show(host && active);
        _joinViewers->Show(host && active);
        _joinViewers->SetValue(ctl.JoinRole() == Collab::ParticipantRole::Viewer);
        UpdateRoleButton();
        Layout();
        Fit();
    }

private:
    Collab::ParticipantId SelectedParticipant() const {
        long item = _people->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        return item >= 0 && item < (long)_ids.size() ? _ids[item] : Collab::kNoParticipant;
    }
    void UpdateRoleButton() {
        Collab::ParticipantId who = SelectedParticipant();
        bool viewer = who != Collab::kNoParticipant && _collab.Controller().RoleOf(who) == Collab::ParticipantRole::Viewer;
        _roleButton->SetLabel(viewer ? Collab::Labels::MakeEditor : Collab::Labels::MakeViewer);
        _roleButton->Enable(who != Collab::kNoParticipant && who != Collab::kHostParticipant);
    }
    void ToggleRole() {
        Collab::ParticipantId who = SelectedParticipant();
        if (who == Collab::kNoParticipant) {
            return;
        }
        auto& ctl = _collab.Controller();
        auto role = ctl.RoleOf(who) == Collab::ParticipantRole::Viewer ? Collab::ParticipantRole::Editor : Collab::ParticipantRole::Viewer;
        std::string error;
        if (!ctl.SetRole(who, role, error)) {
            wxMessageBox(wxString::FromUTF8(error), "Live Collaboration", wxOK | wxICON_INFORMATION, this);
        }
    }

public:

private:
    DesktopCollab& _collab;
    wxStaticText* _status = nullptr;
    wxStaticText* _codeLabel = nullptr;
    wxStaticText* _code = nullptr;
    wxStaticText* _addresses = nullptr;
    wxListCtrl* _people = nullptr;
    wxImageList* _images = nullptr;
    std::vector<Collab::ParticipantId> _ids; // per list row
    wxButton* _roleButton = nullptr;
    wxCheckBox* _joinViewers = nullptr;
    wxTextCtrl* _report = nullptr;
    wxButton* _end = nullptr;
    wxButton* _saveCopy = nullptr;
};

// ---------------------------------------------------------------------------
// DesktopCollab
// ---------------------------------------------------------------------------

DesktopCollab::DesktopCollab(xLightsFrame* frame) :
    _frame(frame) {
    std::shared_ptr<DispatchGate> gate = _gate;
    Collab::Dispatcher dispatch = [gate](std::function<void()> fn) {
        // Posted to the app, which outlives the frame and this object;
        // QueueEvent is safe from any thread.
        std::lock_guard<std::mutex> lock(gate->lock);
        if (!gate->open || wxTheApp == nullptr) {
            return;
        }
        wxTheApp->CallAfter([gate, fn = std::move(fn)]() {
            bool open;
            {
                std::lock_guard<std::mutex> l(gate->lock);
                open = gate->open;
            }
            if (open) {
                fn();
            }
        });
    };
    Collab::ControllerOptions opts;
    opts.appVersion = xlights_version_string;
    opts.deviceKind = DeviceKind();
    _ctl = std::make_unique<Collab::CollabController>(*this, Collab::CollabNetFactory::Lan(dispatch), dispatch, opts);
    _dispatch = dispatch;
    if (PreferenceEnabled()) {
        EnableGroupNetwork();
    }
    _timer.SetOwner(this);
    Bind(wxEVT_TIMER, &DesktopCollab::OnTimer, this);
}

DesktopCollab::~DesktopCollab() {
    {
        std::lock_guard<std::mutex> lock(_gate->lock);
        _gate->open = false;
    }
    _timer.Stop();
    RemoveDocFolder();
    if (!_menuShown && _menuItem != nullptr) {
        delete _menuItem; // never handed to the menu (owns the submenu)
    }
    CollabOverlay::Current() = nullptr;
    CollabLayoutOverlay::Current() = nullptr;
    _ctl.reset();
}

bool DesktopCollab::PreferenceEnabled() {
    wxConfigBase* config = wxConfigBase::Get();
    bool on = false;
    if (config != nullptr) {
        config->Read(Collab::kCollaborationEnabledPref, &on, false);
    }
    return on;
}

void DesktopCollab::SetPreferenceEnabled(bool enabled) {
    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        config->Write(Collab::kCollaborationEnabledPref, enabled);
        config->Flush();
    }
    if (enabled) {
        EnableGroupNetwork();
    }
    UpdateVisibility();
}

// Only once the beta is on: it starts watching for SharePlay activities.
void DesktopCollab::EnableGroupNetwork() {
    if (!_groupNetworkSet) {
        _groupNetworkSet = true;
        _ctl->SetGroupNetwork(Collab::CollabNetFactory::Group(_dispatch));
    }
}

bool DesktopCollab::Available() const {
    return _ctl->Available() || _ctl->GroupAvailable();
}

void DesktopCollab::CreateUI(wxMenu* fileMenu, wxMenuItem* insertAfter) {
    _fileMenu = fileMenu;
    _menu = new wxMenu();
    _hostItem = _menu->Append(wxID_ANY, "Host Session...", "Let others on this network edit the open sequence with you");
#ifdef __WXOSX__
    _hostGroupItem = _menu->Append(wxID_ANY, GroupHostLabel(), "Edit the open sequence with the people in a FaceTime call or Messages conversation");
    _joinItem = _menu->Append(wxID_ANY, "Join Session...", "Edit a sequence someone on this network or in your SharePlay call is hosting");
#else
    // SharePlay is Apple-only; there is never a group network here.
    _joinItem = _menu->Append(wxID_ANY, "Join Session...", "Edit a sequence someone on this network is hosting");
#endif
    _menu->AppendSeparator();
    _windowItem = _menu->Append(wxID_ANY, "Session Window...");
    _endItem = _menu->Append(wxID_ANY, "End Session");
    _frame->Bind(wxEVT_MENU, [this](wxCommandEvent&) { HostSession(); }, _hostItem->GetId());
    if (_hostGroupItem != nullptr) {
        _frame->Bind(wxEVT_MENU, [this](wxCommandEvent&) { HostSession(true); }, _hostGroupItem->GetId());
    }
    // Whether a FaceTime call is active changes without telling us.
    _frame->Bind(wxEVT_MENU_OPEN, [this](wxMenuEvent& e) {
        e.Skip();
        if (e.GetMenu() == _menu || e.GetMenu() == _fileMenu) {
            UpdateStatus();
        }
    });
    _frame->Bind(wxEVT_MENU, [this](wxCommandEvent&) { JoinSession(); }, _joinItem->GetId());
    _frame->Bind(wxEVT_MENU, [this](wxCommandEvent&) { ShowSessionWindow(); }, _windowItem->GetId());
    _frame->Bind(wxEVT_MENU, [this](wxCommandEvent&) { EndOrLeave(); }, _endItem->GetId());
    _menuItem = new wxMenuItem(fileMenu, wxID_ANY, "Collaborate", wxEmptyString, wxITEM_NORMAL, _menu);
    _menuPos = fileMenu->GetMenuItemCount();
    for (size_t i = 0; i < fileMenu->GetMenuItemCount(); ++i) {
        if (fileMenu->FindItemByPosition(i) == insertAfter) {
            _menuPos = i + 1;
        }
    }

    _statusButton = new wxButton(_frame->AUIStatusBar, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    _statusButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { ShowSessionWindow(); });
    _statusButton->Hide();
    _frame->StatusBarSizer->Add(_statusButton, wxGBPosition(0, 6), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    _notice = new wxPanel(_frame->AUIStatusBar, wxID_ANY);
    auto* sizer = new wxBoxSizer(wxHORIZONTAL);
    _noticeText = new wxStaticText(_notice, wxID_ANY, wxEmptyString);
    _noticeText->SetForegroundColour(wxColour(0x20, 0x70, 0xD0));
    sizer->Add(_noticeText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    _noticeAction = new wxButton(_notice, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    _noticeAction->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto cb = _noticeCallback;
        HideNotice();
        if (cb) {
            cb();
        }
    });
    sizer->Add(_noticeAction, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    auto* dismiss = new wxButton(_notice, wxID_ANY, "Dismiss", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    dismiss->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { HideNotice(); });
    sizer->Add(dismiss, 0, wxALIGN_CENTER_VERTICAL);
    _notice->SetSizer(sizer);
    _notice->Hide();
    _frame->StatusBarSizer->Add(_notice, wxGBPosition(0, 7), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    UpdateVisibility();
}

void DesktopCollab::UpdateVisibility() {
    if (_fileMenu == nullptr) {
        return;
    }
    using State = Collab::CollabController::State;
    bool inSession = _ctl->GetState() != State::Idle && _ctl->GetState() != State::Ended;
    bool show = Available() && (PreferenceEnabled() || inSession);
    if (show != _menuShown) {
        if (show) {
            _fileMenu->Insert(std::min(_menuPos, _fileMenu->GetMenuItemCount()), _menuItem);
        } else {
            _fileMenu->Remove(_menuItem);
        }
        _menuShown = show;
    }
    UpdateStatus();
}

void DesktopCollab::UpdateStatus() {
    if (_statusButton == nullptr) {
        return;
    }
    using State = Collab::CollabController::State;
    using Role = Collab::CollabController::Role;
    State st = _ctl->GetState();
    bool live = st != State::Idle && st != State::Ended;
    _hostItem->Enable(!live && _ctl->Available());
    bool group = _ctl->GroupAvailable();
    if (_hostGroupItem != nullptr) {
        _hostGroupItem->Enable(!live && group);
        _hostGroupItem->SetItemLabel(group ? GroupHostLabel() : wxString("Host via SharePlay (not available)"));
    }
    _joinItem->Enable(!live);
    _endItem->Enable(live);
    _endItem->SetItemLabel(_ctl->GetRole() == Role::Client ? "Leave Session" : "End Session");
    _windowItem->Enable(_ctl->GetRole() != Role::None);
    bool show = _menuShown && st != State::Idle;
    if (show) {
        std::string label = _ctl->StatusText();
        if (st == State::Active) {
            label += " (" + std::to_string(_ctl->Participants().size()) + ")";
            if (!_ctl->CanEditSequence()) {
                label = std::string(Collab::Labels::ViewOnly) + " - " + label;
            }
        }
        _statusButton->SetLabel(wxString::FromUTF8(label));
        std::string tip;
        for (const auto& p : _ctl->Participants()) {
            tip += (tip.empty() ? "" : "\n") + p.name + " (" + p.deviceKind + ")";
            if (p.role == Collab::ParticipantRole::Viewer) {
                tip += std::string(" - ") + Collab::Labels::Viewer;
            }
        }
        _statusButton->SetToolTip(wxString::FromUTF8(tip));
    }
    if (show != _statusButton->IsShown()) {
        _statusButton->Show(show);
    }
    _frame->AUIStatusBar->Layout();
}

void DesktopCollab::ShowNotice(const wxString& text, const wxString& action, std::function<void()> onAction, int seconds) {
    if (_notice == nullptr || _frame->IsExiting()) {
        return;
    }
    _noticeText->SetLabel(text);
    _noticeText->SetToolTip(text);
    _noticeAction->SetLabel(action);
    _noticeAction->Show(!action.empty());
    _noticeCallback = std::move(onAction);
    _noticeHideAt = NowMs() + seconds * 1000LL;
    _notice->Show();
    _notice->Layout();
    _frame->AUIStatusBar->Layout();
    StartTimer();
}

void DesktopCollab::HideNotice() {
    _noticeHideAt = 0;
    _noticeCallback = nullptr;
    if (_notice != nullptr && _notice->IsShown()) {
        _notice->Hide();
        _frame->AUIStatusBar->Layout();
    }
}

void DesktopCollab::StartTimer() {
    if (!_timer.IsRunning()) {
        _timer.Start(50);
    }
}

void DesktopCollab::OnTimer(wxTimerEvent&) {
    using State = Collab::CollabController::State;
    int64_t now = NowMs();
    _ctl->Tick((uint64_t)now);
    if (_ctl->InSession() && now - _lastPresence >= 200) {
        _lastPresence = now;
        GatherPresence();
    }
    if (_noticeHideAt != 0 && now >= _noticeHideAt) {
        HideNotice();
    }
    bool flashing = false;
    for (auto it = _overlay.flashUntil.begin(); it != _overlay.flashUntil.end();) {
        if (it->second <= now) {
            it = _overlay.flashUntil.erase(it);
            flashing = true; // one more redraw without it
        } else {
            flashing = true;
            ++it;
        }
    }
    if (flashing) {
        _overlay.now = now;
        RefreshSequencer();
    }
    State st = _ctl->GetState();
    bool done = st == State::Idle || (st == State::Ended && now - _endedAt > 3000);
    if (done && _noticeHideAt == 0 && _overlay.flashUntil.empty()) {
        _timer.Stop();
    }
}

void DesktopCollab::RefreshSequencer() {
    MainSequencer* ms = _frame->mainSequencer;
    if (ms == nullptr) {
        return;
    }
    ms->PanelTimeLine->Refresh(false);
    ms->PanelRowHeadings->Refresh(false);
    ms->PanelEffectGrid->Refresh(false);
}

std::string DesktopCollab::Initials(const std::string& name) {
    std::string out;
    bool start = true;
    for (char c : name) {
        if (c == ' ' || c == '-' || c == '.') {
            start = true;
        } else if (start && out.size() < 2) {
            out += (char)toupper((unsigned char)c);
            start = false;
        } else {
            start = false;
        }
    }
    return out.empty() ? std::string("?") : out;
}

void DesktopCollab::GatherPresence() {
    GatherLayoutSelection();
    if (_frame->CurrentSeqXmlFile == nullptr || _frame->mainSequencer == nullptr) {
        return;
    }
    Collab::PresenceInfo p;
    TimeLine* tl = _frame->mainSequencer->PanelTimeLine;
    p.startMS = (int)tl->GetStartTimeMS();
    p.endMS = tl->GetTimeMSfromPosition(tl->GetSize().GetWidth());
    std::vector<SeqObjectId> rows;
    ForEachLayer(_frame->_sequenceElements, [&](Element* el, EffectLayer* layer) {
        bool any = false;
        for (Effect* e : layer->GetEffects()) {
            if (e->GetSelected() != EFFECT_NOT_SELECTED) {
                p.selectedEffects.push_back(e->GetUid());
                any = true;
            }
        }
        if (any && std::find(rows.begin(), rows.end(), el->GetUid()) == rows.end()) {
            rows.push_back(el->GetUid());
        }
    });
    p.focusedElements = rows;
    Effect* open = _frame->selectedEffect;
    if (open != nullptr) {
        p.softLock = open->GetUid();
    }
    _ctl->SetLocalPresence(p);

    // The banner on the effect settings panel.
    std::string banner;
    if (open != nullptr) {
        Collab::ParticipantId holder = _ctl->SoftLockHolder(open->GetUid());
        if (holder != Collab::kNoParticipant) {
            if (auto who = _ctl->FindParticipant(holder)) {
                banner = who->name + " is editing this effect";
            }
        }
    }
    if (banner != _lastBanner && _frame->EffectsPanel1 != nullptr) {
        _lastBanner = banner;
        _frame->EffectsPanel1->SetCollabNotice(wxString::FromUTF8(banner));
    }
}

void DesktopCollab::RebuildOverlay() {
    auto flash = std::move(_overlay.flashUntil);
    _overlay = CollabOverlay();
    _overlay.flashUntil = std::move(flash);
    _overlay.now = NowMs();
    if (!_ctl->InSession()) {
        CollabOverlay::Current() = nullptr;
        _overlay.flashUntil.clear();
        RefreshSequencer();
        return;
    }
    for (const auto& who : _ctl->Participants()) {
        if (who.isLocal) {
            continue;
        }
        const Collab::PresenceInfo* p = _ctl->PresenceOf(who.id);
        if (p == nullptr) {
            continue;
        }
        if (p->startMS >= 0 && p->endMS > p->startMS) {
            _overlay.bands.push_back({ p->startMS, p->endMS, who.colour });
        }
        for (SeqObjectId row : p->focusedElements) {
            _overlay.rowStripes[row].push_back(who.colour);
        }
        for (SeqObjectId e : p->selectedEffects) {
            _overlay.outlines[e].push_back(who.colour);
        }
        if (p->softLock != kNoSeqObjectId && _ctl->SoftLockHolder(p->softLock) == who.id) {
            _overlay.locks[p->softLock] = { Initials(who.name), who.colour };
        }
    }
    if (!_ctl->Assignments().empty()) {
        Collab::ParticipantId me = _ctl->LocalParticipant();
        auto mark = [&](Element* el) {
            if (auto a = _ctl->AssignmentOf(el->GetUid())) {
                auto who = _ctl->FindParticipant(a->owner);
                _overlay.assigned[el->GetUid()] = { Collab::ParticipantColour(a->owner), who ? who->name : a->name, a->owner == me };
            }
        };
        SequenceElements& seq = _frame->_sequenceElements;
        for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
            Element* el = seq.GetElement(i, MASTER_VIEW);
            if (el == nullptr) {
                continue;
            }
            mark(el);
            if (auto* me2 = dynamic_cast<ModelElement*>(el)) {
                for (int s = 0; s < me2->GetSubModelAndStrandCount(); ++s) {
                    if (Element* sub = me2->GetSubModel(s)) {
                        mark(sub);
                    }
                }
            }
        }
    }
    _overlay.ghostColour = Collab::ParticipantColour(_ctl->LocalParticipant());
    for (const auto& g : _ctl->PendingGhosts()) {
        _overlay.ghosts.push_back({ g.effect, g.startMS, g.endMS, g.kind == Collab::PendingGhost::Kind::Delete });
    }
    CollabOverlay::Current() = &_overlay;
    RefreshSequencer();
}

std::string DesktopCollab::DocFolder() {
    if (_docFolder.empty()) {
        std::error_code ec;
        std::filesystem::path p = std::filesystem::path(wxStandardPaths::Get().GetTempDir().ToStdString()) / ("xLightsCollab-" + std::to_string(wxGetProcessId()));
        std::filesystem::create_directories(p, ec);
        _docFolder = p.string();
    }
    return _docFolder;
}

// ---------------------------------------------------------------------------
// ICollabApp
// ---------------------------------------------------------------------------

SequenceElements& DesktopCollab::Sequence() {
    return _frame->_sequenceElements;
}

bool DesktopCollab::HasSequence() {
    return _frame->CurrentSeqXmlFile != nullptr;
}

bool DesktopCollab::SaveXsq(std::string& out) {
    if (_frame->CurrentSeqXmlFile == nullptr) {
        return false;
    }
    pugi::xml_document doc;
    if (!_frame->CurrentSeqXmlFile->BuildDocument(doc, _frame->_sequenceElements)) {
        return false;
    }
    std::ostringstream s;
    doc.save(s, "  ");
    out = s.str();
    return !out.empty();
}

bool DesktopCollab::LoadXsq(const std::string& xsq) {
    std::string path = (std::filesystem::path(DocFolder()) / Collab::SessionSequenceFileName(_sequenceName)).string();
    {
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        f << xsq;
        if (!f) {
            spdlog::error("Collab: could not write the session sequence to {}", path);
            return false;
        }
    }
    _loading = true;
    // The session replaces the document: changes in the old copy are the
    // session's, never this user's to save.
    _frame->mSavedChangeCount = _frame->_sequenceElements.GetChangeCount();
    if (_frame->CurrentSeqXmlFile != nullptr && !_frame->CloseSequence()) {
        _loading = false;
        return false;
    }
    _frame->OpenSequence(wxString::FromUTF8(path), nullptr);
    _loading = false;
    if (_frame->CurrentSeqXmlFile == nullptr) {
        return false;
    }
    _frame->mSavedChangeCount = _frame->_sequenceElements.GetChangeCount();
    return true;
}

std::string DesktopCollab::ShowFolder() {
    return _frame->showDirectory;
}

std::vector<std::string> DesktopCollab::AudioFiles() {
    std::vector<std::string> out;
    SequenceFile* f = _frame->CurrentSeqXmlFile;
    if (f != nullptr) {
        if (!f->GetMediaFile().empty()) {
            out.push_back(f->GetMediaFile());
        }
        for (int i = 0; i < f->GetAltTrackCount(); ++i) {
            out.push_back(f->GetAltTrack(i).path);
        }
    }
    return out;
}

bool DesktopCollab::UseShowFolder(const std::string& folder) {
    if (_originalShowDir.empty()) {
        _originalShowDir = _frame->showDirectory;
    }
    _loading = true;
    _frame->mSavedChangeCount = _frame->_sequenceElements.GetChangeCount();
    bool ok = _frame->SetDir(wxString::FromUTF8(folder), false);
    _loading = false;
    return ok;
}

void DesktopCollab::AddMediaFolder(const std::string& folder) {
    auto& dirs = _frame->mediaDirectories;
    if (std::find(dirs.begin(), dirs.end(), folder) == dirs.end()) {
        dirs.push_back(folder);
        _addedMediaFolders.push_back(folder);
        FileUtils::SetFixFileDirectories(dirs);
    }
}

void DesktopCollab::RemoveDocFolder() {
    if (_docFolder.empty()) {
        return;
    }
    std::error_code ec;
    std::filesystem::remove_all(_docFolder, ec);
    _docFolder.clear();
}

void DesktopCollab::AfterSequenceClosed() {
    using State = Collab::CollabController::State;
    State st = _ctl->GetState();
    if (_loading || (st != State::Idle && st != State::Ended)) {
        return;
    }
    // Saving the sequence (always Save As for this copy) wrote it elsewhere,
    // so the temporary copy is never the one to keep.
    RemoveDocFolder();
    if (!_keptSessionShow.empty()) {
        // Kept only while the sequence used it ("Save a Copy" made the
        // lasting copy elsewhere).
        std::error_code ec;
        std::filesystem::path show(_keptSessionShow);
        _keptSessionShow.clear();
        std::filesystem::remove_all(show, ec);
        std::filesystem::remove(show.parent_path(), ec); // the session's folder, once empty
    }
    if (_originalShowDir.empty()) {
        return;
    }
    if (_frame->IsExiting()) {
        _originalShowDir.clear(); // the switch was never made permanent
        return;
    }
    RestoreShowFolder();
}

void DesktopCollab::RestoreShowFolder() {
    // A session that opened against a copy of the host's show left the frame
    // on that copy, which goes away with the session: go back before starting
    // another one.
    if (_originalShowDir.empty()) {
        return;
    }
    std::string original = std::move(_originalShowDir);
    _originalShowDir.clear();
    if (_frame->showDirectory != original) {
        _frame->SetDir(wxString::FromUTF8(original), false);
    }
}

void DesktopCollab::SetCollaborationLock(bool locked, const std::string& reason) {
    _frame->SetCollaborationLock(locked, reason);
}

void DesktopCollab::SetSequenceSettingsLock(bool locked, const std::string& reason) {
    _frame->SetSequenceSettingsLock(locked, reason);
}

void DesktopCollab::OnSessionStateChanged() {
    using State = Collab::CollabController::State;
    State st = _ctl->GetState();
    if (st == State::Ended) {
        _endedAt = NowMs();
        if (!_addedMediaFolders.empty()) {
            auto& dirs = _frame->mediaDirectories;
            for (const auto& f : _addedMediaFolders) {
                dirs.remove(f);
            }
            _addedMediaFolders.clear();
            FileUtils::SetFixFileDirectories(dirs);
        }
        HideNotice();
        std::string text = _ctl->StatusText();
        auto report = _ctl->GetJoinReport();
        if ((_ctl->GetRole() == Collab::CollabController::Role::Client || _ctl->SaveAsOnly()) && report && report->sessionCopy) {
            ShowNotice(wxString::FromUTF8(text), "Save a Copy...", [this] { SaveCopy(); }, 60);
        } else {
            ShowNotice(wxString::FromUTF8(text), wxString(), nullptr, 30);
        }
        if (_frame->EffectsPanel1 != nullptr && !_lastBanner.empty()) {
            _lastBanner.clear();
            _frame->EffectsPanel1->SetCollabNotice(wxString());
        }
    }
    if (st != State::Idle) {
        StartTimer();
    }
    UpdateVisibility();
    RebuildOverlay();
    RebuildLayoutOverlay();
    // Host-only actions (saving the show files, uploads, output) follow the role.
    _frame->UpdateLayoutLockState();
    if (_window != nullptr) {
        _window->ShowState(_codeShown);
    }
}

void DesktopCollab::OnParticipantsChanged() {
    UpdateStatus();
    RebuildOverlay();
    if (_window != nullptr) {
        _window->ShowState(_codeShown);
    }
}

void DesktopCollab::OnPresenceChanged(Collab::ParticipantId) {
    RebuildOverlay();
    RebuildLayoutOverlay();
}

void DesktopCollab::OnSequenceChanged(const Collab::SequenceChange& c) {
    using Cause = Collab::SequenceChange::Cause;
    if (_frame->CurrentSeqXmlFile == nullptr || _frame->mainSequencer == nullptr) {
        RebuildOverlay();
        return;
    }
    // The effect open in the settings panel may be one the change removed.
    if (Effect* sel = _frame->selectedEffect) {
        bool touched = c.everything || c.rowsRemoved || std::find(c.effects.begin(), c.effects.end(), sel->GetUid()) != c.effects.end();
        if (touched && FindEffect(_frame->_sequenceElements, LocatorOf(sel)) != sel) {
            _frame->ClearEffectSelectionForDocumentChange();
        }
    }
    if (c.rowsAdded || c.rowsRemoved || c.rowsReordered || c.rowsRenamed || c.everything) {
        wxCommandEvent rows(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(_frame, rows);
    }
    const Collab::RenderPlan plan = Collab::PlanRender(c);
    const int duration = _frame->CurrentSeqXmlFile->GetSequenceDurationMS();
    const int s = plan.startMS < 0 ? 0 : plan.startMS;
    const int e = plan.endMS < 0 ? duration : plan.endMS;
    switch (plan.kind) {
    case Collab::RenderPlan::Kind::Nothing:
        break;
    case Collab::RenderPlan::Kind::Rows:
        // Rows touching what is on screen (or being played) render now, the
        // rest when they scroll into view, playback starts or the app is idle.
        _frame->MarkRemoteChange(plan.rows, plan.startMS, plan.endMS);
        break;
    case Collab::RenderPlan::Kind::Models:
        for (const auto& m : plan.models) {
            _frame->RequestRenderForModel(m, 0, duration);
        }
        break;
    case Collab::RenderPlan::Kind::Everything:
        _frame->mainSequencer->PanelEffectGrid->sendRenderEvent("", s, e);
        break;
    }
    if (c.cause == Cause::RemoteEdit) {
        int64_t until = NowMs() + 1000;
        for (SeqObjectId uid : c.effects) {
            _overlay.flashUntil[uid] = until;
        }
        StartTimer();
    }
    RebuildOverlay();
}

void DesktopCollab::UpdateJoinAddresses() {
    _addressesShown.clear();
    for (const auto& a : _ctl->HostJoinAddresses()) {
        _addressesShown += (_addressesShown.empty() ? "" : ", ") + a;
    }
}

void DesktopCollab::OnHostChanged(const Collab::HostChange& change) {
    if (change.becameHost) {
        // Saving, uploads and output are this device's now; the sequence
        // still lives in the session's temporary folder, so Save is Save As.
        _codeShown = _ctl->PairingCode();
        UpdateJoinAddresses();
    }
    ShowNotice(wxString::FromUTF8(change.text), wxString(), nullptr, change.becameHost ? 30 : 15);
    UpdateStatus();
    RebuildOverlay();
    RebuildLayoutOverlay();
    _frame->UpdateLayoutLockState();
    if (_window != nullptr) {
        _window->ShowState(_codeShown);
    }
}

void DesktopCollab::MarkSequenceModified() {
    _frame->_sequenceElements.IncrementChangeCount(nullptr);
}

void DesktopCollab::OnRejected(const Collab::RejectNotice& n) {
    if (n.action == Collab::RejectAction::PutOnNewLayer) {
        uint64_t id = n.id;
        ShowNotice(wxString::FromUTF8(n.text), "Put on New Layer", [this, id] { _ctl->PutOnNewLayer(id); });
    } else {
        ShowNotice(wxString::FromUTF8(n.text));
    }
    RebuildOverlay();
}

void DesktopCollab::OnUndoStateChanged() {
}

void DesktopCollab::OnJoinReport(const Collab::JoinReport& r) {
    if (!r.refusal.empty()) {
        ShowNotice(wxString::FromUTF8(r.refusal), wxString(), nullptr, 30);
    }
    if (_window != nullptr) {
        _window->ShowState(_codeShown);
    }
}

void DesktopCollab::OnSessionsFound(const std::vector<Collab::SessionAdvert>& sessions) {
    if (_joinDialog != nullptr) {
        _joinDialog->SetSessions(WithGroupOffer(sessions), _ctl->OfferedGroupSession().id);
    }
    CheckGroupOffer();
}

std::vector<Collab::SessionAdvert> DesktopCollab::WithGroupOffer(std::vector<Collab::SessionAdvert> sessions) const {
    Collab::SessionAdvert offer = _ctl->OfferedGroupSession();
    if (!offer.id.empty() && std::none_of(sessions.begin(), sessions.end(), [&](const auto& s) { return s.id == offer.id; })) {
        sessions.insert(sessions.begin(), offer);
    }
    return sessions;
}

// "Sam started a SharePlay session" once per offer, while nothing else is going on.
void DesktopCollab::CheckGroupOffer() {
    Collab::SessionAdvert offer = _ctl->OfferedGroupSession();
    if (offer.id == _offerNoticed) {
        return;
    }
    _offerNoticed = offer.id;
    using State = Collab::CollabController::State;
    State st = _ctl->GetState();
    if (offer.id.empty() || !PreferenceEnabled() || _joinDialog != nullptr || (st != State::Idle && st != State::Ended)) {
        return;
    }
    std::string who = offer.hostName.empty() ? std::string("Someone") : offer.hostName;
    std::string text = who + " started a SharePlay session for '" + offer.sequenceName + "'";
    std::string id = offer.id;
    ShowNotice(wxString::FromUTF8(text), "Join...", [this, id] { JoinSession(id); }, 120);
}

void DesktopCollab::OnBrowseError(const std::string& reason) {
    if (_joinDialog != nullptr) {
        _joinDialog->SetError(wxString::FromUTF8(reason));
    }
}

void DesktopCollab::OnSessionNotice(const std::string& text) {
    ShowNotice(wxString::FromUTF8(text), wxString(), nullptr, 15);
    UpdateStatus();
}

wxMenu* DesktopCollab::AppendRowMenu(wxMenu& menu, Element* element) {
    _rowMenuActions.clear();
    if (element == nullptr || !_ctl->InSession() || !_ctl->CanEditSequence()) {
        return nullptr;
    }
    SeqObjectId row = Collab::TopLevelOf(_frame->_sequenceElements, element->GetUid());
    Collab::ParticipantId me = _ctl->LocalParticipant();
    bool host = _ctl->GetRole() == Collab::CollabController::Role::Host;
    size_t next = 0;
    auto add = [&](wxMenu& m, const wxString& label, std::function<void()> fn) {
        if (next == _rowMenuIds.size()) {
            _rowMenuIds.push_back(wxNewId());
        }
        int id = _rowMenuIds[next++];
        m.Append(id, label);
        _rowMenuActions[id] = std::move(fn);
    };
    auto run = [this](std::function<bool(std::string&)> fn) {
        std::string error;
        if (!fn(error) && !error.empty()) {
            ShowNotice(wxString::FromUTF8(error));
        }
    };
    menu.AppendSeparator();
    if (auto a = _ctl->AssignmentOf(row)) {
        auto who = _ctl->FindParticipant(a->owner);
        menu.Append(wxID_ANY, wxString::FromUTF8(Collab::AssignedToText(who ? who->name : a->name, a->owner == me)))->Enable(false);
        if (host || a->owner == me) {
            SeqObjectId held = a->element;
            add(menu, Collab::Labels::Release, [this, held, run] { run([this, held](std::string& e) { return _ctl->Release(held, e); }); });
        }
        return nullptr;
    }
    if (!host) {
        add(menu, Collab::Labels::AssignToMe, [this, row, me, run] { run([this, row, me](std::string& e) { return _ctl->Assign(row, me, e); }); });
        return nullptr;
    }
    auto* sub = new wxMenu();
    for (const auto& p : _ctl->Participants()) {
        if (p.role == Collab::ParticipantRole::Viewer) {
            continue;
        }
        Collab::ParticipantId id = p.id;
        add(*sub, wxString::FromUTF8(p.isLocal ? p.name + " (you)" : p.name),
            [this, row, id, run] { run([this, row, id](std::string& e) { return _ctl->Assign(row, id, e); }); });
    }
    menu.AppendSubMenu(sub, Collab::Labels::AssignTo);
    return sub;
}

bool DesktopCollab::HandleRowMenu(int id) {
    auto it = _rowMenuActions.find(id);
    if (it == _rowMenuActions.end()) {
        return false;
    }
    auto fn = it->second;
    _rowMenuActions.clear();
    fn();
    RebuildOverlay();
    return true;
}

void DesktopCollab::OnLog(const std::string& line) {
    spdlog::debug("{}", line);
}

std::string DesktopCollab::JoinReportText() const {
    const auto& r = _ctl->GetJoinReport();
    if (!r) {
        return {};
    }
    std::string out;
    auto list = [&](const char* title, const std::vector<std::string>& files) {
        if (files.empty()) {
            return;
        }
        out += std::string(out.empty() ? "" : "\n") + title;
        for (const auto& f : files) {
            out += "\n    " + f;
        }
    };
    if (!r->refusal.empty()) {
        out += r->refusal;
    }
    if (!r->layoutDifference.empty() && r->refusal.find(r->layoutDifference) == std::string::npos) {
        out += std::string(out.empty() ? "" : "\n") + r->layoutDifference;
    }
    if (r->sessionCopy) {
        out += std::string(out.empty() ? "" : "\n") + "Using a copy of the host's show files for this session.";
    }
    list("Fetched from the host:", r->fetched);
    list("Missing here (those effects render blank):", r->missingMedia);
    list("Different here (those effects render differently):", r->differentMedia);
    return out;
}

// ---------------------------------------------------------------------------
// Actions
// ---------------------------------------------------------------------------

wxString DesktopCollab::GroupHostLabel() const {
    return _ctl->GroupInCall() ? "Host via SharePlay - Start in FaceTime Call" : "Host via SharePlay - Invite with Messages...";
}

void DesktopCollab::HostSession(bool group) {
    if (group ? !_ctl->GroupAvailable() : !_ctl->Available()) {
        return;
    }
    if (_frame->CurrentSeqXmlFile == nullptr) {
        wxMessageBox("Open a sequence first: a session shares the open sequence.", "Host Session", wxOK | wxICON_INFORMATION, _frame);
        return;
    }
    if (_frame->mSavedChangeCount != _frame->_sequenceElements.GetChangeCount()) {
        if (wxMessageBox("The sequence has unsaved changes. Save it before hosting?", "Host Session", wxYES_NO | wxICON_QUESTION, _frame) != wxYES ||
            !_frame->SaveSequence()) {
            return;
        }
    }
    wxString name = DisplayName(_frame);
    if (name.empty()) {
        return;
    }
    if (!_originalShowDir.empty()) {
        // Still on the copy of another host's show from an earlier session.
        wxMessageBox("Reopen your own show and sequence first: the last session left a copy of another show open.", "Host Session",
                     wxOK | wxICON_INFORMATION, _frame);
        RestoreShowFolder();
        return;
    }
    std::string code;
    std::string error;
    std::string seq = wxFileName(wxString::FromUTF8(_frame->CurrentSeqXmlFile->GetFullPath())).GetFullName().utf8_string();
    bool ok = group ? _ctl->HostGroup(name.utf8_string(), seq, error) : _ctl->Host(name.utf8_string(), seq, code, error);
    if (!ok) {
        wxMessageBox(wxString::FromUTF8(error), "Host Session", wxOK | wxICON_ERROR, _frame);
        return;
    }
    _codeShown = code;
    UpdateJoinAddresses();
    StartTimer();
    ShowSessionWindow();
}

void DesktopCollab::JoinSession(const std::string& preferred) {
    if (!Available() || _joinDialog != nullptr) {
        return;
    }
    CollabJoinDialog dlg(_frame);
    dlg.Prefer(preferred);
    _joinDialog = &dlg;
    _ctl->StartBrowsing();
    dlg.SetSessions(WithGroupOffer(_ctl->Sessions()), _ctl->OfferedGroupSession().id);
    int rc = dlg.ShowModal();
    _ctl->StopBrowsing();
    _joinDialog = nullptr;
    if (rc != wxID_OK || dlg.Selected() == nullptr) {
        return;
    }
    Collab::SessionAdvert advert = *dlg.Selected();
    bool group = dlg.SelectedIsGroup();
    std::string code = dlg.Code();
    bool fetch = dlg.Fetch();
    wxString name = DisplayName(_frame);
    if (name.empty()) {
        return;
    }
    // The session brings its own copy of the sequence; close this one first
    // (asking about unsaved changes) rather than in the middle of joining.
    if (_frame->CurrentSeqXmlFile != nullptr && !_frame->CloseSequence()) {
        return;
    }
    RestoreShowFolder();
    _sequenceName = advert.sequenceName;
    _codeShown.clear();
    _addressesShown.clear();
    std::string error;
    bool ok = group ? _ctl->JoinGroup(name.utf8_string(), advert.id, fetch, error)
                    : _ctl->Join(name.utf8_string(), advert.id, code, fetch, error);
    if (!ok) {
        wxMessageBox(wxString::FromUTF8(error), "Join Session", wxOK | wxICON_ERROR, _frame);
        return;
    }
    StartTimer();
    ShowSessionWindow();
}

void DesktopCollab::EndOrLeave() {
    using Role = Collab::CollabController::Role;
    using State = Collab::CollabController::State;
    State st = _ctl->GetState();
    if (st == State::Idle || st == State::Ended) {
        return;
    }
    if (_ctl->GetRole() == Role::Host) {
        if (_frame->CurrentSeqXmlFile != nullptr && _frame->mSavedChangeCount != _frame->_sequenceElements.GetChangeCount()) {
            int rc = wxMessageBox("Save the sequence before ending the session?", "End Session", wxYES_NO | wxCANCEL | wxICON_QUESTION, _frame);
            if (rc == wxCANCEL) {
                return;
            }
            if (rc == wxYES && !_frame->SaveSequence()) {
                return;
            }
        }
        _ctl->End();
    } else {
        _ctl->Leave();
    }
}

void DesktopCollab::ShowSessionWindow() {
    if (_window == nullptr) {
        _window = new CollabSessionWindow(_frame, *this);
    }
    _window->ShowState(_codeShown);
    _window->Show();
    _window->Raise();
}

void DesktopCollab::SaveCopy() {
    if (_frame->CurrentSeqXmlFile == nullptr) {
        return;
    }
    if (!OnSessionCopy()) {
        // The sequence used this device's own show folder: Save As into it.
        _frame->SaveAsSequence();
        return;
    }
    // The session ran on a temporary copy of the host's show, which goes
    // with it. Keep a copy where the user chooses, as a show folder of its
    // own with the sequence saved in it.
    const std::vector<std::string> temporary = TemporaryFolders();
    const std::string sessionShow = _frame->showDirectory;
    std::string start = Collab::SessionSaveStartFolder({ wxStandardPaths::Get().GetDocumentsDir().utf8_string(), _originalShowDir }, temporary);
    std::string target;
    for (;;) {
        wxDirDialog dlg(_frame, "Choose an empty folder for a copy of the host's show and this sequence", wxString::FromUTF8(start),
                        wxDD_DEFAULT_STYLE);
        if (dlg.ShowModal() != wxID_OK) {
            return;
        }
        target = dlg.GetPath().utf8_string();
        ObtainAccessToURL(target, true);
        std::string refusal = Collab::ShowCopyFolderRefusal(target, temporary);
        if (refusal.empty()) {
            break;
        }
        DisplayWarning(refusal, _frame);
    }
    std::string error;
    if (!Collab::CopyFolderContents(sessionShow, target, error)) {
        DisplayError("Could not copy the host's show: " + error, _frame);
        return;
    }
    // The sequence open here still reads the session's copy until it closes.
    std::string kept;
    if (_ctl->KeepSessionCopy(kept)) {
        _keptSessionShow = kept;
    }
    const std::string name = wxFileName(wxString::FromUTF8(_frame->CurrentSeqXmlFile->GetFullPath())).GetFullName().utf8_string();
    const std::string seqPath = (std::filesystem::path(target) / Collab::SessionSequenceFileName(name)).string();
    if (_frame->SaveAsSequence(seqPath)) {
        spdlog::info("Collab: kept a copy of the host's show and the sequence in {}", target);
    }
}

bool DesktopCollab::AllowCloseSequence() {
    using Role = Collab::CollabController::Role;
    using State = Collab::CollabController::State;
    State st = _ctl->GetState();
    if (_loading || _closing || st == State::Idle || st == State::Ended) {
        return true;
    }
    bool host = _ctl->GetRole() == Role::Host;
    if (_frame->IsNonInteractiveSave()) {
        // Nobody to ask: a script never ends (or leaves) a session other
        // people are working in.
        spdlog::warn("Collab: not closing the sequence: that would {} the collaboration session; end it in the app first.", host ? "end" : "leave");
        return false;
    }
    wxString msg = host ? "Closing the sequence ends the collaboration session for everyone. Close it?"
                        : "Closing the sequence leaves the collaboration session. Close it?";
    return wxMessageBox(msg, "Live Collaboration", wxYES_NO | wxICON_QUESTION, _frame) == wxYES;
}

void DesktopCollab::EndForClose() {
    using Role = Collab::CollabController::Role;
    using State = Collab::CollabController::State;
    State st = _ctl->GetState();
    if (_loading || _closing || st == State::Idle || st == State::Ended) {
        return;
    }
    _closing = true;
    if (_ctl->GetRole() == Role::Host) {
        _ctl->End();
    } else {
        _ctl->Leave();
    }
    _closing = false;
}

std::vector<std::string> DesktopCollab::TemporaryFolders() const {
    std::vector<std::string> out;
    if (!_docFolder.empty()) {
        out.push_back(_docFolder);
    }
    if (OnSessionCopy() && !_frame->showDirectory.empty()) {
        // The copy sits in the session's own folder (SessionShowFolder).
        out.push_back(std::filesystem::path(_frame->showDirectory).parent_path().string());
    }
    if (Collab::CollabParticipant* s = _ctl->Session()) {
        out.push_back(s->SessionFolder());
    }
    return out;
}

bool DesktopCollab::IsClientDocument() const {
    if (_docFolder.empty() || _frame->CurrentSeqXmlFile == nullptr) {
        return false;
    }
    std::error_code ec;
    auto doc = std::filesystem::weakly_canonical(std::filesystem::path(_frame->CurrentSeqXmlFile->GetFullPath()).parent_path(), ec);
    auto mine = std::filesystem::weakly_canonical(std::filesystem::path(_docFolder), ec);
    return doc == mine;
}

void DesktopCollab::Shutdown() {
    using State = Collab::CollabController::State;
    _timer.Stop();
    if (_window != nullptr) {
        _window->Destroy();
        _window = nullptr;
    }
    State st = _ctl->GetState();
    if (st != State::Idle && st != State::Ended) {
        _closing = true;
        if (_ctl->GetRole() == Collab::CollabController::Role::Host) {
            _ctl->End();
        } else {
            _ctl->Leave();
        }
        _closing = false;
    }
    CollabOverlay::Current() = nullptr;
    CollabLayoutOverlay::Current() = nullptr;
}

// ---------------------------------------------------------------------------
// Live layout
// ---------------------------------------------------------------------------

bool DesktopCollab::SessionLive() const {
    using State = Collab::CollabController::State;
    State st = _ctl->GetState();
    return st != State::Idle && st != State::Ended;
}

bool DesktopCollab::IsSessionClient() const {
    return SessionLive() && _ctl->GetRole() == Collab::CollabController::Role::Client;
}

uint64_t DesktopCollab::LayoutGeneration() {
    return _frame->LayoutGeneration();
}

bool DesktopCollab::LayoutEditInProgress() {
    return _editorLockDepth > 0 || _frame->LayoutEditInProgress();
}

bool DesktopCollab::SerializeLayout(Collab::LayoutFiles& out) {
    if (_loading || _frame->CurrentDir.empty() || _frame->IsExiting() || LayoutPanel::IsFinalizingModel()) {
        return false;
    }
    _frame->SerializeLayoutFiles(&out.rgbEffects, &out.networks);
    return !out.rgbEffects.empty();
}

bool DesktopCollab::ApplyLayout(const Collab::LayoutFiles& current, const Collab::LayoutFiles& target, Collab::LayoutApplied& out) {
    if (_loading || _frame->IsExiting() || LayoutPanel::IsFinalizingModel()) {
        return false;
    }
    if (!_frame->ApplySessionLayout(current, target, out)) {
        return false;
    }
    if (out.layoutChanged || out.controllersChanged) {
        RebuildLayoutOverlay();
    }
    return true;
}

void DesktopCollab::OnLayoutPresenceChanged() {
    RebuildLayoutOverlay();
}

void DesktopCollab::OnLayoutRejected(const std::string& text) {
    ShowNotice(wxString::FromUTF8(text), wxString(), nullptr, 15);
}

void DesktopCollab::GatherLayoutSelection() {
    if (!_ctl->InSession() || !_ctl->LiveLayout() || _frame->layoutPanel == nullptr || LayoutPanel::IsFinalizingModel()) {
        return;
    }
    std::vector<std::string> keys;
    for (const BaseObject* o : _frame->layoutPanel->GetSelectedLayoutObjects()) {
        keys.push_back(CollabLayoutOverlay::KeyOf(o));
    }
    if (keys != _lastLayoutSelection) {
        _lastLayoutSelection = keys;
        _ctl->SetLayoutSelection(keys);
    }
}

void DesktopCollab::RebuildLayoutOverlay() {
    CollabLayoutOverlay next;
    const bool live = _ctl->InSession() && _ctl->LiveLayout();
    if (live) {
        for (const auto& who : _ctl->Participants()) {
            if (who.isLocal) {
                continue;
            }
            const Collab::PresenceInfo* p = _ctl->PresenceOf(who.id);
            if (p == nullptr) {
                continue;
            }
            for (const auto& key : p->layoutSelected) {
                next.selected.emplace(key, who.colour); // the first participant's colour wins
            }
            const auto& lock = p->layoutLock;
            if (!lock.object.empty() && _ctl->LayoutLockHolder(lock.object) == who.id) {
                std::string why;
                if (!_ctl->LayoutObjectReadOnly(lock.object, why) || why.empty()) {
                    why = who.name + " is editing " + lock.name + "'s " + lock.what;
                }
                next.locks[lock.object] = { why, who.colour };
            }
        }
    }
    if (!live) {
        _lastLayoutSelection.clear();
    }
    const bool changed = !(next == _layoutOverlay);
    _layoutOverlay = std::move(next);
    CollabLayoutOverlay::Current() = live ? &_layoutOverlay : nullptr;
    if (changed && _frame->layoutPanel != nullptr && !_frame->IsExiting()) {
        _frame->layoutPanel->UpdateCollabPresence();
    }
}

bool DesktopCollab::BeginEditorLock(const std::vector<const BaseObject*>& objects, const std::string& what, wxWindow* parent) {
    if (!_ctl->InSession() || !_ctl->LiveLayout()) {
        return true;
    }
    // Someone else's editor already holds one of them.
    for (const BaseObject* o : objects) {
        std::string why;
        if (o != nullptr && _ctl->LayoutObjectReadOnly(CollabLayoutOverlay::KeyOf(o), why)) {
            wxMessageBox(wxString::FromUTF8(why.empty() ? std::string("Someone else is editing ") + o->GetName() : why), "Live Collaboration",
                         wxOK | wxICON_INFORMATION, parent != nullptr ? parent : _frame);
            return false;
        }
    }
    if (_editorLockDepth == 0 && objects.size() == 1 && objects.front() != nullptr) {
        const BaseObject* o = objects.front();
        Collab::LayoutLockInfo info;
        info.object = CollabLayoutOverlay::KeyOf(o);
        info.name = o->GetName();
        info.what = what;
        std::string heldBy;
        if (!_ctl->BeginLayoutLock(info, heldBy)) {
            std::string text = (heldBy.empty() ? std::string("Someone else") : heldBy) + " is editing " + o->GetName() + "'s " + what + ".";
            wxMessageBox(wxString::FromUTF8(text), "Live Collaboration", wxOK | wxICON_INFORMATION, parent != nullptr ? parent : _frame);
            return false;
        }
        _editorLockTaken = true;
    }
    ++_editorLockDepth;
    return true;
}

void DesktopCollab::EndEditorLock() {
    if (_editorLockDepth <= 0) {
        return;
    }
    if (--_editorLockDepth == 0) {
        if (_editorLockTaken) {
            _editorLockTaken = false;
            _ctl->EndLayoutLock();
        }
        // What the editor changed can go out now.
        _ctl->FlushLayout();
    }
}

CollabLayoutEditLock::CollabLayoutEditLock(const BaseObject* object, const std::string& what, wxWindow* parent) :
    CollabLayoutEditLock(std::vector<const BaseObject*>{ object }, what, parent) {
}

CollabLayoutEditLock::CollabLayoutEditLock(const std::vector<const BaseObject*>& objects, const std::string& what, wxWindow* parent) {
    xLightsFrame* frame = xLightsApp::GetFrame();
    DesktopCollab* collab = frame != nullptr ? frame->GetCollab() : nullptr;
    if (collab == nullptr) {
        return;
    }
    _granted = collab->BeginEditorLock(objects, what, parent);
    if (_granted) {
        _collab = collab;
    }
}

CollabLayoutEditLock::~CollabLayoutEditLock() {
    if (_collab != nullptr) {
        _collab->EndEditorLock();
    }
}

// ---------------------------------------------------------------------------
// What a session refuses on this device
// ---------------------------------------------------------------------------

std::string xLightsFrame::SequenceSettingsFreezeReason() const {
    return IsSequenceSettingsLocked() ? GetSequenceSettingsLockReason() : std::string();
}

std::string xLightsFrame::CollabHostOnlyReason(const std::string& action) const {
    if (_collab == nullptr || _collab->Controller().CanDriveControllers()) {
        return {};
    }
    return "Only the session host can " + action + ".";
}

bool xLightsFrame::RefuseIfCollabHostOnly(const std::string& action, wxWindow* parent) {
    std::string why = CollabHostOnlyReason(action);
    if (why.empty()) {
        return false;
    }
    if (!IsNonInteractiveSave()) {
        DisplayInfo(why, parent != nullptr ? parent : this);
    } else {
        spdlog::warn("{}", why);
    }
    return true;
}
