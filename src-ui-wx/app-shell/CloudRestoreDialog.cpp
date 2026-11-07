/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CloudRestoreDialog.h"

#include "CloudBackupDialog.h"
#include "utils/ExternalHooks.h"
#include "utils/S3Client.h"
#include "utils/string_utils.h"

#include <wx/button.h>
#include <wx/datetime.h>
#include <wx/dirdlg.h>
#include <wx/gauge.h>
#include <wx/image.h>
#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/mstream.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>
#include <wx/textctrl.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <filesystem>

namespace fs = std::filesystem;

namespace {

const wxSize THUMB_SIZE(320, 180);

fs::path FromUtf8(const std::string& s) {
    return fs::path(std::u8string(s.begin(), s.end()));
}

std::string ToUtf8(const fs::path& p) {
    auto u = p.u8string();
    return std::string(u.begin(), u.end());
}

std::string TrimSlashes(std::string s) {
    while (!s.empty() && (s.back() == '/' || s.back() == '\\')) {
        s.pop_back();
    }
    while (!s.empty() && (s.front() == '/' || s.front() == '\\')) {
        s.erase(0, 1);
    }
    return s;
}

// "2026-11-06-143000" (UTC) to a time_t; 0 when it does not parse.
time_t ParseStamp(const std::string& stamp) {
    int y, mo, d, h, mi, se;
    if (sscanf(stamp.c_str(), "%4d-%2d-%2d-%2d%2d%2d", &y, &mo, &d, &h, &mi, &se) != 6) {
        return 0;
    }
    struct tm t {};
    t.tm_year = y - 1900;
    t.tm_mon = mo - 1;
    t.tm_mday = d;
    t.tm_hour = h;
    t.tm_min = mi;
    t.tm_sec = se;
#ifdef _WIN32
    return _mkgmtime(&t);
#else
    return timegm(&t);
#endif
}

wxString DisplayStamp(const std::string& stamp, const char* fmt = "%Y-%m-%d %H:%M") {
    time_t t = ParseStamp(stamp);
    return t == 0 ? wxString::FromUTF8(stamp) : wxDateTime(t).Format(fmt);
}

bool IsSameOrInside(const fs::path& child, const fs::path& parent) {
    std::error_code ec;
    fs::path c = fs::weakly_canonical(child, ec);
    fs::path p = fs::weakly_canonical(parent, ec);
    auto ci = c.begin();
    for (auto pi = p.begin(); pi != p.end(); ++pi, ++ci) {
        if (pi->empty()) {
            continue;
        }
        if (ci == c.end()) {
            return false;
        }
#ifdef _WIN32
        if (_wcsicmp(pi->c_str(), ci->c_str()) != 0) {
#else
        if (*pi != *ci) {
#endif
            return false;
        }
    }
    return true;
}

} // namespace

CloudRestoreDialog::CloudRestoreDialog(wxWindow* parent, CloudRestoreHooks hooks) :
    wxDialog(parent, wxID_ANY, _("Restore from Cloud"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    _hooks(std::move(hooks)),
    _timer(this) {
    auto* top = new wxBoxSizer(wxVERTICAL);

    auto* storageRow = new wxBoxSizer(wxHORIZONTAL);
    _storageLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
    storageRow->Add(_storageLabel, 1, wxALIGN_CENTER_VERTICAL);
    _refreshButton = new wxButton(this, wxID_ANY, _("Refresh"));
    storageRow->Add(_refreshButton, 0, wxLEFT, 6);
    _settingsButton = new wxButton(this, wxID_ANY, _("Settings..."));
    storageRow->Add(_settingsButton, 0, wxLEFT, 6);
    top->Add(storageRow, 0, wxEXPAND | wxALL, 10);

    auto* lists = new wxBoxSizer(wxHORIZONTAL);
    auto* showCol = new wxBoxSizer(wxVERTICAL);
    showCol->Add(new wxStaticText(this, wxID_ANY, _("Shows")), 0, wxBOTTOM, 4);
    _shows = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(200, 220));
    showCol->Add(_shows, 1, wxEXPAND);
    lists->Add(showCol, 0, wxEXPAND | wxRIGHT, 10);

    auto* backupCol = new wxBoxSizer(wxVERTICAL);
    backupCol->Add(new wxStaticText(this, wxID_ANY, _("Backups")), 0, wxBOTTOM, 4);
    _backupList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(240, 220), wxLC_REPORT | wxLC_SINGLE_SEL);
    _backupList->AppendColumn(_("Date"), wxLIST_FORMAT_LEFT, 140);
    _backupList->AppendColumn(_("Size"), wxLIST_FORMAT_RIGHT, 80);
    backupCol->Add(_backupList, 1, wxEXPAND);
    lists->Add(backupCol, 0, wxEXPAND | wxRIGHT, 10);

    auto* detailCol = new wxBoxSizer(wxVERTICAL);
    _thumbnail = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap, wxDefaultPosition, THUMB_SIZE);
    _thumbnail->SetMinSize(THUMB_SIZE);
    detailCol->Add(_thumbnail, 0, wxBOTTOM, 6);
    _details = new wxStaticText(this, wxID_ANY, wxEmptyString);
    detailCol->Add(_details, 1, wxEXPAND);
    lists->Add(detailCol, 1, wxEXPAND);
    top->Add(lists, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

    auto* destRow = new wxBoxSizer(wxHORIZONTAL);
    destRow->Add(new wxStaticText(this, wxID_ANY, _("Restore to")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    _dest = new wxTextCtrl(this, wxID_ANY);
    _dest->SetToolTip(_("A new or empty folder. The current show is never overwritten."));
    destRow->Add(_dest, 1, wxALIGN_CENTER_VERTICAL);
    _browse = new wxButton(this, wxID_ANY, _("Browse..."));
    destRow->Add(_browse, 0, wxLEFT, 6);
    top->Add(destRow, 0, wxEXPAND | wxALL, 10);

    _status = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
    top->Add(_status, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    _gauge = new wxGauge(this, wxID_ANY, 1000);
    top->Add(_gauge, 0, wxEXPAND | wxALL, 10);
    _log = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE | wxTE_READONLY);
    _log->Hide();
    top->Add(_log, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    auto* buttons = new wxBoxSizer(wxHORIZONTAL);
    _deleteButton = new wxButton(this, wxID_ANY, _("Delete Backup..."));
    _deleteButton->SetToolTip(_("Removes the selected backup from the bucket to free space."));
    buttons->Add(_deleteButton, 0);
    buttons->AddStretchSpacer();
    _restoreButton = new wxButton(this, wxID_ANY, _("Restore"));
    buttons->Add(_restoreButton, 0, wxRIGHT, 6);
    _closeButton = new wxButton(this, wxID_CANCEL, _("Close"));
    buttons->Add(_closeButton, 0);
    top->Add(buttons, 0, wxEXPAND | wxALL, 10);

    SetSizerAndFit(top);
    SetEscapeId(wxID_NONE);

    _shows->Bind(wxEVT_LISTBOX, &CloudRestoreDialog::OnShowSelected, this);
    _backupList->Bind(wxEVT_LIST_ITEM_SELECTED, &CloudRestoreDialog::OnBackupSelected, this);
    _browse->Bind(wxEVT_BUTTON, &CloudRestoreDialog::OnBrowse, this);
    _restoreButton->Bind(wxEVT_BUTTON, &CloudRestoreDialog::OnRestore, this);
    _deleteButton->Bind(wxEVT_BUTTON, &CloudRestoreDialog::OnDelete, this);
    _closeButton->Bind(wxEVT_BUTTON, &CloudRestoreDialog::OnClose, this);
    _refreshButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { Reload(); });
    _settingsButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        if (_hooks.openSettings) {
            _hooks.openSettings();
        }
        Reload();
    });
    _dest->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { _destEdited = true; });
    Bind(wxEVT_TIMER, &CloudRestoreDialog::OnTimer, this);
    Bind(wxEVT_CLOSE_WINDOW, &CloudRestoreDialog::OnCloseWindow, this);

    Reload();
}

CloudRestoreDialog::~CloudRestoreDialog() {
    _timer.Stop();
    _closing = true;
    _cancel = true;
    for (auto& t : _threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void CloudRestoreDialog::CancelAndWait() {
    _cancel = true;
    for (auto& t : _threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    _restoring = false;
}

void CloudRestoreDialog::RunTask(std::function<void()> task) {
    _threads.emplace_back(std::move(task));
}

std::string CloudRestoreDialog::ShowFolderKey(const std::string& show) const {
    return CloudBackupStore::ShowFolderKey(_settings.prefix, show);
}

std::string CloudRestoreDialog::DefaultDestination(const std::string& show, const std::string& stamp) const {
    fs::path base;
    if (!_hooks.currentShowDir.empty()) {
        base = FromUtf8(_hooks.currentShowDir).lexically_normal().parent_path();
    }
    if (base.empty()) {
        base = FromUtf8(wxStandardPaths::Get().GetDocumentsDir().utf8_string());
    }
    std::string name = show + " (restored " + DisplayStamp(stamp, "%Y-%m-%d").utf8_string() + ")";
    fs::path p = base / FromUtf8(name);
    std::error_code ec;
    for (int i = 2; fs::exists(p, ec); ++i) {
        p = base / FromUtf8(name + " " + std::to_string(i));
    }
    return ToUtf8(p);
}

void CloudRestoreDialog::SetStatus(const wxString& text, bool error) {
    _status->SetForegroundColour(error ? *wxRED : GetForegroundColour());
    _status->SetLabel(text);
    _status->SetToolTip(text);
}

void CloudRestoreDialog::UpdateButtons() {
    bool idle = !_restoring && !_deleting;
    _restoreButton->Enable(idle && _selectedBackup >= 0 && _summary.ok);
    _deleteButton->Enable(idle && _selectedBackup >= 0);
    _shows->Enable(idle);
    _backupList->Enable(idle);
    _dest->Enable(idle);
    _browse->Enable(idle);
    _refreshButton->Enable(idle);
    _settingsButton->Enable(idle);
    _closeButton->SetLabel(_restoring ? _("Cancel") : _("Close"));
}

void CloudRestoreDialog::Reload() {
    _settings = CloudStorageSettings::Load();
    _shows->Clear();
    _showNames.clear();
    _backupList->DeleteAllItems();
    _backups.clear();
    _selectedBackup = -1;
    _summary = {};
    _thumbnail->SetBitmap(wxNullBitmap);
    _details->SetLabel(wxEmptyString);
    _log->Hide();
    _gauge->SetValue(0);

    std::string problem = _settings.Validate();
    if (!problem.empty()) {
        _storageLabel->SetLabel(_("Cloud storage is not set up."));
        SetStatus(wxString::FromUTF8(problem) + _(". Click Settings... to enter the storage details."), true);
        UpdateButtons();
        Layout();
        return;
    }
    _storageLabel->SetLabel(wxString::Format(_("Bucket \"%s\" on %s"), wxString::FromUTF8(_settings.bucket),
                                             _settings.IsR2() ? _("Cloudflare R2") : wxString::FromUTF8(_settings.EffectiveEndpoint())));
    UpdateButtons();
    LoadShows();
}

void CloudRestoreDialog::LoadShows() {
    int gen = ++_generation;
    SetStatus(_("Looking for backups..."));
    _gauge->Pulse();
    S3Config cfg = _settings.ToS3Config();
    std::string prefix = TrimSlashes(_settings.prefix);
    std::string root = prefix.empty() ? "" : prefix + "/";
    RunTask([this, gen, cfg, root]() {
        S3Client client(cfg);
        std::vector<S3Object> objects;
        std::vector<std::string> folders;
        std::string error;
        bool ok = client.ListObjects(root, "/", objects, folders, error);
        std::vector<std::string> names;
        for (const auto& f : folders) {
            std::string n = f.substr(root.size());
            if (!n.empty() && n.back() == '/') {
                n.pop_back();
            }
            if (!n.empty()) {
                names.push_back(n);
            }
        }
        std::sort(names.begin(), names.end());
        if (_closing) {
            return;
        }
        CallAfter([this, gen, ok, error, names]() {
            if (gen != _generation) {
                return;
            }
            _gauge->SetValue(0);
            if (!ok) {
                SetStatus(_("Unable to list the bucket: ") + wxString::FromUTF8(error), true);
                return;
            }
            _showNames = names;
            for (const auto& n : names) {
                _shows->Append(wxString::FromUTF8(n));
            }
            if (names.empty()) {
                SetStatus(_("No backups found in this bucket."));
                return;
            }
            SetStatus(wxString::Format(_("%d show(s) found. Select one, then a backup."), (int)names.size()));
            _shows->SetSelection(0);
            LoadBackups(names[0]);
        });
    });
}

void CloudRestoreDialog::OnShowSelected(wxCommandEvent& event) {
    int sel = _shows->GetSelection();
    if (sel >= 0 && sel < (int)_showNames.size()) {
        LoadBackups(_showNames[sel]);
    }
}

void CloudRestoreDialog::LoadBackups(const std::string& show, const wxString& doneStatus, bool doneIsError) {
    int gen = ++_generation;
    _backupList->DeleteAllItems();
    _backups.clear();
    _selectedBackup = -1;
    _summary = {};
    _thumbnail->SetBitmap(wxNullBitmap);
    _details->SetLabel(wxEmptyString);
    UpdateButtons();
    SetStatus(_("Listing backups..."));
    S3Config cfg = _settings.ToS3Config();
    std::string folder = ShowFolderKey(show);
    RunTask([this, gen, cfg, folder, doneStatus, doneIsError]() {
        S3Client client(cfg);
        std::vector<S3Object> objects;
        std::vector<std::string> unused;
        std::string error;
        bool ok = client.ListObjects(folder, "", objects, unused, error);
        std::vector<Backup> backups = CloudBackupStore::FromListing(objects, folder);
        if (_closing) {
            return;
        }
        CallAfter([this, gen, ok, error, backups, doneStatus, doneIsError]() {
            if (gen != _generation) {
                return;
            }
            if (!ok) {
                SetStatus(_("Unable to list the backups: ") + wxString::FromUTF8(error), true);
                return;
            }
            _backups = backups;
            for (size_t i = 0; i < backups.size(); ++i) {
                long row = _backupList->InsertItem((long)i, DisplayStamp(backups[i].stamp));
                _backupList->SetItem(row, 1, wxString::FromUTF8(CloudFormatBytes(backups[i].zipSize)));
            }
            if (backups.empty()) {
                SetStatus(doneStatus.empty() ? _("This show has no complete backup.") : doneStatus, doneIsError);
                return;
            }
            SetStatus(doneStatus, doneIsError);
            _backupList->SetItemState(0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
        });
    });
}

void CloudRestoreDialog::OnBackupSelected(wxListEvent& event) {
    long idx = event.GetIndex();
    if (idx < 0 || idx >= (long)_backups.size()) {
        return;
    }
    _selectedBackup = (int)idx;
    _summary = {};
    UpdateButtons();
    int sel = _shows->GetSelection();
    if (!_destEdited && sel >= 0 && sel < (int)_showNames.size()) {
        _dest->ChangeValue(wxString::FromUTF8(DefaultDestination(_showNames[sel], _backups[idx].stamp)));
    }
    LoadDetails(_backups[idx]);
}

void CloudRestoreDialog::LoadDetails(const Backup& b) {
    int gen = ++_generation;
    _thumbnail->SetBitmap(wxNullBitmap);
    _details->SetLabel(_("Loading..."));
    S3Config cfg = _settings.ToS3Config();
    RunTask([this, gen, cfg, b]() {
        S3Client client(cfg);
        std::string json, png, error, thumbError;
        bool ok = client.ReadObject(b.manifestKey, json, error);
        if (ok && !b.thumbnailKey.empty()) {
            client.ReadObject(b.thumbnailKey, png, thumbError);
        }
        ShowArchiver::BackupSummary summary;
        if (ok) {
            summary = ShowArchiver::ParseManifest(json);
            if (!summary.ok) {
                ok = false;
                error = "The manifest of this backup is not readable";
            }
        }
        if (_closing) {
            return;
        }
        CallAfter([this, gen, ok, error, summary, png]() {
            if (gen != _generation) {
                return;
            }
            if (!ok) {
                _details->SetLabel(wxEmptyString);
                SetStatus(wxString::FromUTF8(error), true);
                return;
            }
            _summary = summary;
            if (!png.empty()) {
                wxMemoryInputStream in(png.data(), png.size());
                wxImage img(in, wxBITMAP_TYPE_PNG);
                if (img.IsOk() && img.GetWidth() > 0 && img.GetHeight() > 0) {
                    double scale = std::min((double)THUMB_SIZE.GetWidth() / img.GetWidth(), (double)THUMB_SIZE.GetHeight() / img.GetHeight());
                    img.Rescale(std::max(1, (int)(img.GetWidth() * scale)), std::max(1, (int)(img.GetHeight() * scale)), wxIMAGE_QUALITY_HIGH);
                    _thumbnail->SetBitmap(wxBitmap(img));
                }
            }
            wxString text = wxString::Format(_("%d sequences, %d files, %s uncompressed\n.fseq files: %s\nMade with xLights %s"),
                                             summary.sequenceCount, summary.fileCount,
                                             wxString::FromUTF8(CloudFormatBytes(summary.totalBytes)),
                                             summary.fseqIncluded ? _("included") : _("not included"),
                                             wxString::FromUTF8(summary.xlightsVersion));
            _details->SetLabel(text);
            SetStatus(wxEmptyString);
            UpdateButtons();
            Layout();
        });
    });
}

void CloudRestoreDialog::OnBrowse(wxCommandEvent& event) {
    wxString start = _dest->GetValue();
    fs::path p = FromUtf8(start.utf8_string());
    std::error_code ec;
    while (!p.empty() && !fs::is_directory(p, ec) && p.has_parent_path() && p.parent_path() != p) {
        p = p.parent_path();
    }
    wxDirDialog dlg(this, _("Folder to restore the show into (new or empty)"), wxString::FromUTF8(ToUtf8(p)));
    if (dlg.ShowModal() == wxID_OK) {
        _dest->ChangeValue(dlg.GetPath());
        _destEdited = true;
    }
}

void CloudRestoreDialog::OnDelete(wxCommandEvent& event) {
    int showSel = _shows->GetSelection();
    if (_restoring || _deleting || _selectedBackup < 0 || _selectedBackup >= (int)_backups.size() ||
        showSel < 0 || showSel >= (int)_showNames.size()) {
        return;
    }
    const std::string show = _showNames[showSel];
    const Backup b = _backups[_selectedBackup];
    wxString msg = wxString::Format(_("Delete the backup of \"%s\" made on %s (%s) from the bucket?\n\nThis cannot be undone."),
                                    wxString::FromUTF8(show), DisplayStamp(b.stamp), wxString::FromUTF8(CloudFormatBytes(b.zipSize)));
    if (_backups.size() == 1) {
        msg += _("\n\nIt is the only backup of this show.");
    }
    if (wxMessageBox(msg, _("Delete Backup"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, this) != wxYES) {
        return;
    }

    _deleting = true;
    UpdateButtons();
    SetStatus(_("Deleting the backup..."));
    _gauge->Pulse();
    S3Config cfg = _settings.ToS3Config();
    std::string folder = ShowFolderKey(show);
    std::vector<Backup> all = _backups;
    RunTask([this, cfg, folder, all, b, show]() {
        S3Client client(cfg);
        std::string error;
        bool ok = CloudBackupStore::Delete(CloudObjectOps::For(client), folder, all, { b }, error);
        if (ok) {
            spdlog::info("Cloud restore: deleted backup {}", b.zipKey);
        } else {
            spdlog::warn("Cloud restore: deleting backup {} failed: {}", b.zipKey, error);
        }
        if (_closing) {
            return;
        }
        CallAfter([this, ok, error, show, lastOne = all.size() == 1]() {
            _deleting = false;
            _gauge->SetValue(0);
            if (ok && lastOne) {
                Reload();
                return;
            }
            // Reload either way: a failed delete may have removed part of the backup.
            LoadBackups(show, ok ? _("Backup deleted.") : _("Unable to delete the backup: ") + wxString::FromUTF8(error), !ok);
        });
    });
}

void CloudRestoreDialog::OnRestore(wxCommandEvent& event) {
    if (_restoring || _deleting || _selectedBackup < 0 || _selectedBackup >= (int)_backups.size() || !_summary.ok) {
        return;
    }
    std::string dest = _dest->GetValue().utf8_string();
    if (dest.empty()) {
        SetStatus(_("Choose the folder to restore into."), true);
        return;
    }
    fs::path d = FromUtf8(dest);
    if (!d.is_absolute()) {
        SetStatus(_("The destination must be a full folder path."), true);
        return;
    }
    std::error_code ec;
    if (fs::exists(d, ec)) {
        if (!fs::is_directory(d, ec)) {
            SetStatus(_("The destination is a file, not a folder."), true);
            return;
        }
        if (fs::directory_iterator(d, ec) != fs::directory_iterator()) {
            SetStatus(_("The destination folder is not empty. Choose a new or empty folder."), true);
            return;
        }
    }
    if (!_hooks.currentShowDir.empty() && IsSameOrInside(d, FromUtf8(_hooks.currentShowDir))) {
        SetStatus(_("The destination cannot be the current show folder or a folder inside it."), true);
        return;
    }
    fs::path existing = d;
    while (!existing.empty() && !fs::exists(existing, ec) && existing.parent_path() != existing) {
        existing = existing.parent_path();
    }
    ObtainAccessToURL(ToUtf8(existing), true);

    _restoring = true;
    _cancel = false;
    _done = 0;
    _total = _backups[_selectedBackup].zipSize;
    _stage = DOWNLOADING;
    _shownStage = -1;
    _log->Hide();
    UpdateButtons();
    _timer.Start(250);
    Backup b = _backups[_selectedBackup];
    ShowArchiver::BackupSummary summary = _summary;
    S3Config cfg = _settings.ToS3Config();
    RunTask([this, cfg, b, summary, dest]() { RestoreWorker(cfg, b, summary, dest); });
}

void CloudRestoreDialog::RestoreWorker(S3Config cfg, Backup b, ShowArchiver::BackupSummary summary, std::string dest) {
    std::error_code ec;
    fs::path tmpDir = fs::temp_directory_path(ec);
    fs::path zip = tmpDir / ("xlights-cloud-restore-" + b.stamp + ".zip");
    auto space = fs::space(tmpDir, ec);
    if (!ec && space.available < b.zipSize + 64ull * 1024 * 1024) {
        RestoreFinished(false, false, dest,
                        "Not enough free disk space in " + ToUtf8(tmpDir) + " to download the backup: " +
                            CloudFormatBytes(b.zipSize) + " needed, " + CloudFormatBytes(space.available) + " available.",
                        {});
        return;
    }

    _stage = DOWNLOADING;
    _total = b.zipSize;
    S3Client client(cfg);
    std::string error, sha;
    bool cancelled = false;
    std::string key = summary.archiveKey.empty() ? b.zipKey : summary.archiveKey;
    bool ok = client.DownloadFile(key, zip, [this](uint64_t done, uint64_t total) {
        _done = done;
        if (total) {
            _total = total;
        }
        return !_cancel;
    }, error, &sha, &cancelled);
    if (!ok) {
        RestoreFinished(false, cancelled, dest, cancelled ? "Restore cancelled." : error, {});
        return;
    }

    _stage = VERIFYING;
    if (!summary.archiveSha256.empty() && sha != summary.archiveSha256) {
        fs::remove(zip, ec);
        RestoreFinished(false, false, dest, "The downloaded archive does not match its manifest (SHA-256 mismatch). Nothing was restored.", {});
        return;
    }

    _stage = EXTRACTING;
    _done = 0;
    _total = summary.totalBytes;
    ShowArchiver::ShowRestoreResult res = ShowArchiver::Extract(zip, FromUtf8(dest), [this](uint64_t done, uint64_t total, const std::string&) {
        _done = done;
        _total = total;
        return !_cancel;
    });
    fs::remove(zip, ec);
    if (!res.ok) {
        RestoreFinished(false, res.cancelled, dest, res.cancelled ? "Restore cancelled." : res.error, res.warnings);
        return;
    }
    RestoreFinished(true, false, dest, "Restored " + std::to_string(res.filesWritten) + " files to " + dest + ".", res.warnings);
}

void CloudRestoreDialog::RestoreFinished(bool ok, bool cancelled, const std::string& dest, const std::string& message,
                                         const std::vector<std::string>& warnings) {
    if (!ok && !cancelled) {
        spdlog::error("Cloud restore failed: {}", message);
    } else if (ok) {
        spdlog::info("Cloud restore: {}", message);
    }
    if (_closing) {
        return;
    }
    CallAfter([this, ok, cancelled, dest, message, warnings]() {
        _restoring = false;
        _timer.Stop();
        _gauge->SetValue(ok ? _gauge->GetRange() : 0);
        SetStatus(wxString::FromUTF8(message), !ok && !cancelled);
        if (!warnings.empty()) {
            wxString text;
            for (const auto& w : warnings) {
                text += wxString::FromUTF8(w) + "\n";
            }
            _log->SetValue(text);
            _log->Show();
        }
        UpdateButtons();
        Layout();
        Fit();
        if (!ok) {
            return;
        }
        _destEdited = false;
        RequestUserAttention();
        if (_hooks.openShow &&
            wxMessageBox(_("The show was restored. Open it now as the current show folder?"), _("Restore from Cloud"),
                         wxYES_NO | wxICON_QUESTION, this) == wxYES) {
            auto open = _hooks.openShow;
            Destroy();
            open(dest);
        }
    });
}

void CloudRestoreDialog::OnTimer(wxTimerEvent& event) {
    int stage = _stage;
    if (stage != _shownStage) {
        _shownStage = stage;
        _stageStart = std::chrono::steady_clock::now();
    }
    uint64_t done = _done;
    uint64_t total = _total;
    wxString label;
    switch (stage) {
    case DOWNLOADING:
        label = _("Step 1/2: downloading");
        break;
    case VERIFYING:
        label = _("Checking the download...");
        break;
    default:
        label = _("Step 2/2: extracting");
        break;
    }
    if (_cancel) {
        label = _("Cancelling...");
    }
    if (stage == VERIFYING || total == 0) {
        _gauge->Pulse();
        SetStatus(label);
        return;
    }
    _gauge->SetValue((int)std::min<uint64_t>(1000, done * 1000 / total));
    std::string text = CloudFormatBytes(done) + " / " + CloudFormatBytes(total);
    double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - _stageStart).count();
    if (stage == DOWNLOADING && secs > 2.0 && done > 0) {
        text += "  -  " + CloudFormatBytes((uint64_t)(done / secs)) + "/s";
    }
    SetStatus(label + "  -  " + wxString::FromUTF8(text));
}

bool CloudRestoreDialog::ConfirmCancel() {
    if (!_restoring || _cancel) {
        return true;
    }
    if (wxMessageBox(_("Cancel the restore in progress? The files restored so far are removed."), _("Restore from Cloud"),
                     wxYES_NO | wxICON_QUESTION, this) == wxYES) {
        _cancel = true;
    }
    return false;
}

void CloudRestoreDialog::OnClose(wxCommandEvent& event) {
    if (_restoring) {
        ConfirmCancel();
        return;
    }
    Destroy();
}

void CloudRestoreDialog::OnCloseWindow(wxCloseEvent& event) {
    if (_restoring && event.CanVeto()) {
        ConfirmCancel();
        event.Veto();
        return;
    }
    Destroy();
}
