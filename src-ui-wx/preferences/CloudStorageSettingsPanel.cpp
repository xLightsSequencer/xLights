/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CloudStorageSettingsPanel.h"

#include "app-shell/CloudStorageSettings.h"
#include "utils/S3Client.h"

#include <wx/busyinfo.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/preferences.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/utils.h>

CloudStorageSettingsPanel::CloudStorageSettingsPanel(wxWindow* parent, wxWindowID id) {
    Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

    auto* top = new wxBoxSizer(wxVERTICAL);

    auto* storageBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Storage"));
    auto* grid = new wxFlexGridSizer(0, 2, 4, 8);
    grid->AddGrowableCol(1);

    auto addRow = [&](const wxString& label, wxWindow* ctrl) {
        grid->Add(new wxStaticText(storageBox->GetStaticBox(), wxID_ANY, label), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
        grid->Add(ctrl, 1, wxEXPAND);
    };
    wxWindow* box = storageBox->GetStaticBox();

    wxArrayString providers;
    providers.Add(_("Cloudflare R2"));
    providers.Add(_("Other S3-compatible storage"));
    _provider = new wxChoice(box, wxID_ANY, wxDefaultPosition, wxDefaultSize, providers);
    addRow(_("Provider"), _provider);

    _accountId = new wxTextCtrl(box, wxID_ANY);
    _accountId->SetHint(_("Cloudflare account ID"));
    addRow(_("Account ID"), _accountId);

    _endpoint = new wxTextCtrl(box, wxID_ANY);
    _endpoint->SetHint("https://s3.example.com");
    addRow(_("Endpoint URL"), _endpoint);

    _region = new wxTextCtrl(box, wxID_ANY);
    _region->SetHint("us-east-1");
    addRow(_("Region"), _region);

    _pathStyle = new wxCheckBox(box, wxID_ANY, _("Path-style addressing (endpoint/bucket/key)"));
    grid->AddSpacer(0);
    grid->Add(_pathStyle, 0);

    _bucket = new wxTextCtrl(box, wxID_ANY);
    addRow(_("Bucket"), _bucket);

    _prefix = new wxTextCtrl(box, wxID_ANY);
    _prefix->SetToolTip(_("Folder inside the bucket. Backups go to <prefix>/<show folder name>/."));
    addRow(_("Folder prefix"), _prefix);

    _accessKey = new wxTextCtrl(box, wxID_ANY);
    addRow(_("Access key ID"), _accessKey);

    _secretKey = new wxTextCtrl(box, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    addRow(_("Secret access key"), _secretKey);

    storageBox->Add(grid, 0, wxEXPAND | wxALL, 4);

    auto* testRow = new wxBoxSizer(wxHORIZONTAL);
    _testButton = new wxButton(box, wxID_ANY, _("Test Connection"));
    testRow->Add(_testButton, 0, wxALIGN_CENTER_VERTICAL);
    _testResult = new wxStaticText(box, wxID_ANY, wxEmptyString);
    testRow->Add(_testResult, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
    storageBox->Add(testRow, 0, wxEXPAND | wxALL, 4);
    top->Add(storageBox, 0, wxEXPAND | wxALL, 8);

    auto* backupBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Backup to Cloud"));
    _includeFseq = new wxCheckBox(backupBox->GetStaticBox(), wxID_ANY, _("Include rendered .fseq files"));
    _includeFseq->SetToolTip(_("A restored show can be played right away without rendering every sequence again."));
    backupBox->Add(_includeFseq, 0, wxALL, 4);
    _includeThumbnail = new wxCheckBox(backupBox->GetStaticBox(), wxID_ANY, _("Include a thumbnail of the layout (.png)"));
    _includeThumbnail->SetToolTip(_("Shown when choosing a backup to restore."));
    backupBox->Add(_includeThumbnail, 0, wxALL, 4);
    auto* keepRow = new wxBoxSizer(wxHORIZONTAL);
    keepRow->Add(new wxStaticText(backupBox->GetStaticBox(), wxID_ANY, _("Keep the last")), 0, wxALIGN_CENTER_VERTICAL);
    _keepBackups = new wxSpinCtrl(backupBox->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                  wxSP_ARROW_KEYS, 0, CloudStorageSettings::MAX_KEEP_BACKUPS, 0);
    keepRow->Add(_keepBackups, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 6);
    keepRow->Add(new wxStaticText(backupBox->GetStaticBox(), wxID_ANY, _("backups of each show (0 = keep them all)")), 0, wxALIGN_CENTER_VERTICAL);
    _keepBackups->SetToolTip(_("After each successful backup, older backups of the same show are deleted from the bucket.\n"
                               "Backups can also be deleted one at a time from File > Restore from Cloud."));
    backupBox->Add(keepRow, 0, wxALL, 4);
    top->Add(backupBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    auto* note = new wxStaticText(this, wxID_ANY,
                                  _("Cloudflare R2 is recommended: its free tier includes 10 GB of storage and free downloads.\n"
                                    "Use a token limited to the backup bucket and keep it private: anyone with these keys can\n"
                                    "read the backups, audio files included. Never make the bucket public.\n"
                                    "On storage with versioning or object lock turned on, deleted backups can still use space.\n"
                                    "Settings are stored on this computer; the secret key is kept in the system's secure storage."));
    top->Add(note, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    SetSizer(top);
    top->Fit(this);

    _provider->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        UpdateEnabled();
        ApplyIfImmediate(false);
    });
    for (auto* c : { _pathStyle, _includeFseq, _includeThumbnail }) {
        c->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) { ApplyIfImmediate(false); });
    }
    _keepBackups->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent&) { ApplyIfImmediate(false); });
    _keepBackups->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { ApplyIfImmediate(false); });
    for (auto* t : { _accountId, _endpoint, _region, _bucket, _prefix, _accessKey }) {
        t->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { ApplyIfImmediate(false); });
    }
    // The secure store is written on focus loss rather than per keystroke.
    _secretKey->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& e) {
        ApplyIfImmediate(true);
        e.Skip();
    });
    _testButton->Bind(wxEVT_BUTTON, &CloudStorageSettingsPanel::OnTestConnection, this);

#ifdef _MSC_VER
    MSWDisableComposited();
#endif
}

CloudStorageSettingsPanel::~CloudStorageSettingsPanel() {
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        ApplyIfImmediate(true);
    }
}

CloudStorageSettings CloudStorageSettingsPanel::FromControls() const {
    CloudStorageSettings s;
    s.provider = _provider->GetSelection() == 1 ? CloudStorageSettings::PROVIDER_S3 : CloudStorageSettings::PROVIDER_R2;
    s.r2AccountId = _accountId->GetValue().utf8_string();
    s.endpoint = _endpoint->GetValue().utf8_string();
    s.region = _region->GetValue().utf8_string();
    s.pathStyle = _pathStyle->GetValue();
    s.bucket = _bucket->GetValue().utf8_string();
    s.prefix = _prefix->GetValue().utf8_string();
    s.accessKeyId = _accessKey->GetValue().utf8_string();
    s.secretAccessKey = _secretKey->GetValue().utf8_string();
    s.includeFseq = _includeFseq->GetValue();
    s.includeThumbnail = _includeThumbnail->GetValue();
    s.keepBackups = _keepBackups->GetValue();
    return s;
}

void CloudStorageSettingsPanel::UpdateEnabled() {
    bool r2 = _provider->GetSelection() != 1;
    _accountId->Enable(r2);
    _endpoint->Enable(!r2);
    _region->Enable(!r2);
    _pathStyle->Enable(!r2);
}

void CloudStorageSettingsPanel::ApplyIfImmediate(bool includeSecret) {
    if (_loading || !wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        return;
    }
    CloudStorageSettings s = FromControls();
    if (!includeSecret) {
        s.secretAccessKey = _savedSecret;
    }
    s.Save();
    _savedSecret = s.secretAccessKey;
}

bool CloudStorageSettingsPanel::TransferDataToWindow() {
    _loading = true;
    CloudStorageSettings s = CloudStorageSettings::Load();
    _provider->SetSelection(s.IsR2() ? 0 : 1);
    _accountId->ChangeValue(wxString::FromUTF8(s.r2AccountId));
    _endpoint->ChangeValue(wxString::FromUTF8(s.endpoint));
    _region->ChangeValue(wxString::FromUTF8(s.region));
    _pathStyle->SetValue(s.pathStyle);
    _bucket->ChangeValue(wxString::FromUTF8(s.bucket));
    _prefix->ChangeValue(wxString::FromUTF8(s.prefix));
    _accessKey->ChangeValue(wxString::FromUTF8(s.accessKeyId));
    _secretKey->ChangeValue(wxString::FromUTF8(s.secretAccessKey));
    _includeFseq->SetValue(s.includeFseq);
    _includeThumbnail->SetValue(s.includeThumbnail);
    _keepBackups->SetValue(s.keepBackups);
    _savedSecret = s.secretAccessKey;
    UpdateEnabled();
    _loading = false;
    return true;
}

bool CloudStorageSettingsPanel::TransferDataFromWindow() {
    CloudStorageSettings s = FromControls();
    s.Save();
    _savedSecret = s.secretAccessKey;
    return true;
}

void CloudStorageSettingsPanel::OnTestConnection(wxCommandEvent& event) {
    CloudStorageSettings s = FromControls();
    std::string problem = s.Validate();
    if (!problem.empty()) {
        _testResult->SetForegroundColour(*wxRED);
        _testResult->SetLabel(problem);
        Layout();
        return;
    }
    std::string error;
    bool ok;
    {
        wxBusyInfo busy(_("Testing connection..."), this);
        S3Client client(s.ToS3Config());
        ok = client.TestConnection(s.prefix, error);
    }
    _testResult->SetForegroundColour(ok ? wxColour(0, 128, 0) : *wxRED);
    _testResult->SetLabel(ok ? _("Connection OK: the bucket is writable.") : wxString::FromUTF8(error));
    Layout();
}
