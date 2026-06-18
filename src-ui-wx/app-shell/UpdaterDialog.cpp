/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
 
//(*InternalHeaders(UpdaterDialog)
#include <wx/string.h>
#include <wx/intl.h>
//*)

#include "settings/XLightsConfigAdapter.h"

#include "UpdaterDialog.h"

#include "utils/CurlManager.h"
#include "utils/ExternalHooks.h"

#include <wx/progdlg.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/filefn.h>
#include <wx/msgdlg.h>
#include <wx/utils.h>
#include <wx/app.h>

#include <atomic>
#include <thread>

#include <log.h>

//(*IdInit(UpdaterDialog)
const long UpdaterDialog::ID_UPDATELABEL = wxNewId();
const long UpdaterDialog::ID_BUTTONUPDOWN = wxNewId();
const long UpdaterDialog::ID_BUTTON2 = wxNewId();
const long UpdaterDialog::ID_BUTTON3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(UpdaterDialog,wxDialog)
	//(*EventTable(UpdaterDialog)
	//*)
END_EVENT_TABLE()

UpdaterDialog::UpdaterDialog(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(UpdaterDialog)
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer1;

	Create(parent, wxID_ANY, _("xLights Update Available"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	StaticTextUpdateLabel = new wxStaticText(this, ID_UPDATELABEL, _("You are running xLights version xxx\nWhereas the most recent release is xxx"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_UPDATELABEL"));
	BoxSizer1->Add(StaticTextUpdateLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	ButtonUpDownload = new wxButton(this, ID_BUTTONUPDOWN, _("Download new release"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONUPDOWN"));
	BoxSizer2->Add(ButtonUpDownload, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonUpdateIgnore = new wxButton(this, ID_BUTTON2, _("Ignore this version"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer2->Add(ButtonUpdateIgnore, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonUpdateSkip = new wxButton(this, ID_BUTTON3, _("Skip this time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	ButtonUpdateSkip->SetDefault();
	BoxSizer2->Add(ButtonUpdateSkip, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	Center();

	Connect(ID_BUTTONUPDOWN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&UpdaterDialog::OnButtonDownloadNewRelease);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&UpdaterDialog::OnButtonUpdateIgnoreClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&UpdaterDialog::OnButtonUpdateSkipClick);
	//*)
}

UpdaterDialog::~UpdaterDialog()
{
	//(*Destroy(UpdaterDialog)
	//*)
}

void UpdaterDialog::OnButtonDownloadNewRelease(wxCommandEvent& event)
{
    spdlog::debug("User has chosen to upgrade to version {}. URL: {}",
        urlVersion.ToStdString(), downloadUrl.ToStdString());

    // Download the release installer to the temp dir, named after the asset.
    wxString fname = downloadUrl.AfterLast('/');
    if (fname.empty()) {
        fname = "xLights-" + urlVersion + (downloadUrl.EndsWith("AppImage") ? ".AppImage" : ".exe");
    }
    const wxString localPath = wxFileName(wxStandardPaths::Get().GetTempDir(), fname).GetFullPath();
    const std::string url = downloadUrl.ToStdString();
    const std::string out = localPath.ToStdString();

    // Download on a worker thread and poll progress on the UI thread so the
    // dialog stays responsive/cancellable (mirrors Waveform.cpp's model
    // download). The CurlManager progress callback reports per-mille (0..1000).
    wxProgressDialog prog("Downloading update",
        "Downloading xLights " + urlVersion + " ...", 1000, this,
        wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);

    std::atomic<bool> dlDone(false), dlOk(false), dlAbort(false);
    std::atomic<int> dlPct(0);
    std::thread worker([&] {
        auto cb = [&dlPct, &dlAbort](int pos) -> bool {
            dlPct.store(pos);
            return !dlAbort.load();
        };
        dlOk = CurlManager::HTTPSGetFile(url, out, {}, {}, 600, cb);
        dlDone.store(true);
    });

    bool cancelled = false;
    while (!dlDone.load()) {
        if (!prog.Update(dlPct.load())) {
            dlAbort.store(true);
            cancelled = true;
            break;
        }
        wxMilliSleep(50);
        wxTheApp->Yield(true);
    }
    worker.join();
    prog.Hide();

    if (cancelled) {
        spdlog::debug("Update download cancelled by the user.");
        if (FileExists(out)) {
            ::wxRemoveFile(localPath);
        }
        return; // leave the updater dialog open
    }

    if (!dlOk.load() || !FileExists(out)) {
        spdlog::warn("Update download failed; opening the download page in a browser instead.");
        wxMessageBox("Download failed. Opening the download page in your browser instead.",
            "Update", wxOK | wxICON_WARNING, this);
        ::wxLaunchDefaultBrowser(downloadUrl);
        GetXLightsConfig()->Write("SkipVersion", "");
        EndDialog(wxID_OK);
        return;
    }

    GetXLightsConfig()->Write("SkipVersion", "");

#ifdef __WXMSW__
    if (wxMessageBox(
            "Download complete.\n\nxLights will now close and the installer will launch to finish the update.",
            "Update ready", wxOK | wxCANCEL | wxICON_INFORMATION, this) == wxOK) {
        // Launch the installer detached, then ask xLights to close so the
        // installer can replace the running files (normal save prompts apply).
        if (::wxExecute("\"" + localPath + "\"", wxEXEC_ASYNC) == 0) {
            wxMessageBox("Could not launch the installer.\nIt was saved to:\n" + localPath,
                "Update", wxOK | wxICON_ERROR, this);
            return;
        }
        EndDialog(wxID_OK);
        if (GetParent() != nullptr) {
            GetParent()->Close();
        }
    }
#else
    // Non-Windows: the asset is not a self-running Windows installer, so just
    // tell the user where it was saved.
    wxMessageBox("Downloaded to:\n" + localPath, "Update downloaded", wxOK | wxICON_INFORMATION, this);
    EndDialog(wxID_OK);
#endif
}

void UpdaterDialog::OnButtonUpdateSkipClick(wxCommandEvent& event)
{
    spdlog::debug("User has chosen to skip upgrade to version {}.", urlVersion.ToStdString());
    auto* config = GetXLightsConfig();
    config->Write("SkipVersion","");
    EndDialog(wxID_CANCEL);
}

void UpdaterDialog::OnButtonUpdateIgnoreClick(wxCommandEvent& event)
{
    spdlog::debug("User has chosen to ignore upgrade to version {}.", urlVersion.ToStdString());
    auto* config = GetXLightsConfig();
    config->Write("SkipVersion", urlVersion);
    EndDialog(wxID_CLOSE);
}
