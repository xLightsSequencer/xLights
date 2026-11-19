/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Bringing changes someone else saved to the shared show files (layout,
// controllers, effect presets) into the open show while it is open.

#include <wx/config.h>
#include <wx/dialog.h>
#include <wx/filename.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/time.h>

#include <filesystem>

#include <pugixml.hpp>

#include "xLightsMain.h"
#include "app-shell/ShowFileMergeDialog.h"
#include "sequencer/EffectTreeDialog.h"
#include "layout/LayoutPanel.h"
#include "layout/ViewsModelsPanel.h"
#include "render/ShowFileLiveMerge.h"
#include "render/ShowMergeApply.h"
#include "sequencer/MainSequencer.h"
#include "utils/ShowFileGuard.h"
#include "utils/ShowFileWatcher.h"
#include "utils/ShowPresence.h"
#include "globals.h"

#include <log.h>

namespace {

constexpr long long kInfoBannerMs = 30000;

bool ModalDialogOpen() {
    for (wxWindowList::compatibility_iterator node = wxTopLevelWindows.GetFirst(); node; node = node->GetNext()) {
        auto* d = dynamic_cast<wxDialog*>(node->GetData());
        if (d != nullptr && d->IsModal()) {
            return true;
        }
    }
    return false;
}

std::string FileNames(const std::vector<std::string>& paths) {
    std::string out;
    for (const auto& p : paths) {
        if (!out.empty()) out += ", ";
        out += wxFileName(wxString::FromUTF8(p)).GetFullName().ToStdString();
    }
    return out;
}

} // namespace

void xLightsFrame::CreateShowFileBanner()
{
    _showFileBanner = new wxPanel(AUIStatusBar, wxID_ANY);
    auto* sizer = new wxBoxSizer(wxHORIZONTAL);
    _showFileBannerText = new wxStaticText(_showFileBanner, wxID_ANY, wxEmptyString);
    _showFileBannerText->SetForegroundColour(wxColour(0x20, 0x70, 0xD0));
    sizer->Add(_showFileBannerText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    _showFileBannerReview = new wxButton(_showFileBanner, wxID_ANY, "Review...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    _showFileBannerReview->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { ReviewIncomingShowChanges(); });
    sizer->Add(_showFileBannerReview, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    auto* dismiss = new wxButton(_showFileBanner, wxID_ANY, "Dismiss", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    dismiss->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { HideShowFileBanner(); });
    sizer->Add(dismiss, 0, wxALIGN_CENTER_VERTICAL);
    _showFileBanner->SetSizer(sizer);
    _showFileBanner->Hide();
    StatusBarSizer->Add(_showFileBanner, wxGBPosition(0, 5), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL, 2);
}

void xLightsFrame::ShowShowFileBanner(const wxString& text, bool review)
{
    if (_showFileBanner == nullptr || IsExiting()) {
        return;
    }
    _showFileBannerText->SetLabel(text);
    _showFileBannerText->SetToolTip(review ? text + ".\nReview to choose which version to keep. Until then, saving asks the same questions."
                                           : text);
    _showFileBannerReview->Show(review);
    _showFileBannerHideAt = review ? 0 : wxGetLocalTimeMillis().GetValue() + kInfoBannerMs;
    _showFileBanner->Show();
    _showFileBanner->Layout();
    AUIStatusBar->Layout();
}

void xLightsFrame::HideShowFileBanner()
{
    _showFileBannerHideAt = 0;
    if (_showFileBanner != nullptr && _showFileBanner->IsShown()) {
        _showFileBanner->Hide();
        AUIStatusBar->Layout();
    }
}

void xLightsFrame::StartShowFileWatch()
{
    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        config->Read("xLightsAutoReloadShowFiles", &_autoReloadShowFiles, true);
    }
    if (!_autoReloadShowFiles) {
        spdlog::info("Show files changed by someone else are merged only when saving (xLightsAutoReloadShowFiles is off).");
    }
    if (_showFileWatcher == nullptr) {
        _showFileWatcher = std::make_unique<ShowFileWatcher>(ShowFileGuard::Instance());
    }
    _showFileWatchTimer.Bind(wxEVT_TIMER, [this](wxTimerEvent&) { CheckShowFilesOnDisk(); });
    _showFileWatchTimer.Start(1000);
}

void xLightsFrame::StopShowFileWatch()
{
    _showFileWatchTimer.Stop();
    if (_showFileWatcher != nullptr) {
        _showFileWatcher->Stop();
    }
    _pendingIncoming.reset();
}

std::vector<ShowFileLiveMerge::LocalFile> xLightsFrame::LiveShowFiles()
{
    using ShowFileLiveMerge::FileKind;
    std::vector<ShowFileLiveMerge::LocalFile> files;
    const std::string dir = CurrentDir.ToStdString();
    const std::string rgb = (std::filesystem::path(dir) / XLIGHTS_RGBEFFECTS_FILE).string();
    const std::string presets = (std::filesystem::path(dir) / XLIGHTS_PRESETS_FILE).string();
    const std::string nets = _outputManager.GetNetworksFilePath();
    // Serializing is only worth it for a file with unsaved edits; otherwise
    // memory is the recorded base.
    const bool rgbEdited = UnsavedRgbEffectsChanges;
    files.push_back({ FileKind::RgbEffects, rgb, rgbEdited, rgbEdited ? BuildEffectsXml() : std::string() });
    if (!nets.empty()) {
        const bool netsEdited = UnsavedNetworkChanges || _outputManager.IsDirty();
        files.push_back({ FileKind::Networks, nets, netsEdited, netsEdited ? _outputManager.SaveToString() : std::string() });
    }
    const bool presetsEdited = UnsavedPresetChanges;
    files.push_back({ FileKind::Presets, presets, presetsEdited, presetsEdited ? _effectPresetManager.SaveToText() : std::string() });
    return files;
}

void xLightsFrame::CheckShowFilesOnDisk()
{
    if (_inShowFileCheck || IsExiting() || _showFileWatcher == nullptr) {
        return;
    }
    if (_showFileBannerHideAt != 0 && wxGetLocalTimeMillis().GetValue() >= _showFileBannerHideAt) {
        HideShowFileBanner();
    }
    // Never during batch render, command line or automation runs, nor for a
    // packaged sequence opened in a temporary folder nobody shares.
    if (!_autoReloadShowFiles || IsNonInteractiveSave() || readOnlyMode || CurrentDir.empty()) {
        _showFileWatcher->Stop();
        return;
    }
    const std::string dir = CurrentDir.ToStdString();
    std::vector<std::string> paths{ (std::filesystem::path(dir) / XLIGHTS_RGBEFFECTS_FILE).string() };
    if (!_outputManager.GetNetworksFilePath().empty()) {
        paths.push_back(_outputManager.GetNetworksFilePath());
    }
    paths.push_back((std::filesystem::path(dir) / XLIGHTS_PRESETS_FILE).string());
    _showFileWatcher->Watch(paths);

    // Not while a dialog is up, a mouse drag is in progress, playback runs (a
    // controller change restarts output mid-show) or a save runs: the next
    // tick looks again (the watcher keeps a change until it reports it).
    if (ModalDialogOpen() || wxWindow::GetCapture() != nullptr || playType == PLAY_TYPE_MODEL || playType == PLAY_TYPE_EFFECT) {
        return;
    }
    {
        std::unique_lock<std::mutex> lock(saveLock, std::try_to_lock);
        if (!lock.owns_lock()) {
            return;
        }
    }
    // The file I/O runs on a worker: a show folder on a stalled network share
    // or behind an iCloud writer can block for a long time. Each tick collects
    // the last poll's result and starts the next one.
    auto changed = _showFileWatcher->TakeBackgroundPoll();
    if (!_showFileWatcher->BackgroundPollRunning()) {
        _showFileWatcher->StartBackgroundPoll();
    }
    if (!changed) {
        return;
    }
    if (_pendingIncoming != nullptr) {
        // A save may have merged them meanwhile (judged from the poll's reads).
        bool outstanding = false;
        for (const auto& p : _pendingIncoming->ChangedPaths()) {
            outstanding |= !_showFileWatcher->MatchedRecordedAtLastRead(p).value_or(false);
        }
        if (!outstanding) {
            _pendingIncoming.reset();
            HideShowFileBanner();
        }
    }
    if (changed->empty()) {
        return;
    }
    HandleIncomingShowChanges(false);
}

void xLightsFrame::ReviewIncomingShowChanges()
{
    if (_inShowFileCheck) {
        return;
    }
    HandleIncomingShowChanges(true);
}

void xLightsFrame::HandleIncomingShowChanges(bool review)
{
    using namespace ShowFileLiveMerge;
    _inShowFileCheck = true;
    struct Reset {
        bool& flag;
        ~Reset() { flag = false; }
    } reset{ _inShowFileCheck };

    auto& guard = ShowFileGuard::Instance();
    ShowFileMerge::Options options;
    options.modelRenames = &AllModels.GetRenameLog();
    options.viewObjectRenames = &AllObjects.GetRenameLog();
    options.controllerRenames = &_outputManager.GetRenameLog();
    const Reader reader = [](const std::string& p) { return ShowFileWatcher::CoordinatedRead(p); };

    for (int attempt = 0; attempt < 3; ++attempt) {
        auto in = std::make_unique<Incoming>(Incoming::Prepare(LiveShowFiles(), options, guard, ShowMergeApply::SequenceUseOf(*this), reader));
        if (_pendingIncoming != nullptr) {
            in->AdoptResolutions(*_pendingIncoming);
        }
        if (!in->HasChanges()) {
            _pendingIncoming.reset();
            HideShowFileBanner();
            return;
        }
        const auto kinds = in->ChangedKinds();
        auto& presence = ShowPresencePublisher::Instance();
        const std::string who = WhoChanged(ShowPresence::FreshOthers(showDirectory, presence.InstallId(), ShowPresence::Now()), kinds);
        const std::string names = FileNames(in->ChangedPaths());

        if (!in->CanMerge()) {
            spdlog::warn("{} changed on disk but cannot be merged now (unknown base or a file that does not parse); it is handled when saving.", names);
            _pendingIncoming.reset();
            ShowShowFileBanner(wxString::FromUTF8(BannerText(who, kinds, 0, 0) + ". It could not be merged; saving will ask what to do."), false);
            return;
        }
        if (in->NeedsDecisions()) {
            const std::string text = BannerText(who, kinds, in->ChangedItems(), in->Decisions());
            if (!review) {
                spdlog::info("{} changed on disk: {} item(s) need a decision before they can be brought in.", names, in->Decisions());
                ShowShowFileBanner(wxString::FromUTF8(text), true);
                _pendingIncoming = std::move(in);
                return;
            }
            ShowFileMergeDialog::Labels labels;
            labels.ok = "Apply";
            ShowFileMergeDialog dlg(this, in->Plan(), labels);
            const int rc = dlg.ShowModal();
            if (rc != wxID_OK || !in->Plan().AllResolved()) {
                ShowShowFileBanner(wxString::FromUTF8(text), true);
                _pendingIncoming = std::move(in);
                return;
            }
            if (!in->StillOnDisk(reader)) {
                // Changed again while deciding: merge again, keeping the decisions.
                _pendingIncoming = std::move(in);
                continue;
            }
        }

        // Stopping a render can run other events while it waits (it yields),
        // and edits made then are in neither side of the merge prepared above.
        const bool wasRendering = !IsRenderDone();
        if (!AbortRender()) {
            spdlog::info("{} changed on disk; a render is still running, trying again shortly.", names);
            _showFileWatcher->RearmAll();
            if (in->Decisions() > 0) {
                _pendingIncoming = std::move(in);
            }
            return;
        }

        if (wasRendering && attempt < 2) {
            // Render is stopped now, so the next pass doesn't wait or yield.
            _pendingIncoming = std::move(in);
            continue;
        }

        const bool rgbEdited = UnsavedRgbEffectsChanges;
        const bool netsEdited = UnsavedNetworkChanges;
        const bool presetsEdited = UnsavedPresetChanges;
        bool presetsChange = false;
        for (auto k : kinds) {
            presetsChange |= k == FileKind::Presets;
        }
        if (layoutPanel != nullptr) {
            layoutPanel->UnSelectAllModels();
        }
        if (presetsChange && EffectTreeDlg != nullptr) {
            // The tree points into the presets that are about to be replaced.
            EffectTreeDlg->ClearItems();
            _effectPresetsInitialized = false;
        }
        // Rows or effects the selection points at may be removed.
        ClearEffectSelectionForDocumentChange();
        _sequenceElements.UnSelectAllEffects();
        if (mainSequencer != nullptr) {
            mainSequencer->PanelEffectGrid->ClearSelection();
        }
        Applied applied = Apply(*this, *in, guard);
        RefreshAfterIncomingShowChanges(applied);
        // Only this process's own edits make the files unsaved.
        if (!rgbEdited) UnsavedRgbEffectsChanges = false;
        if (!netsEdited) {
            UnsavedNetworkChanges = false;
            _outputManager.MarkSaved();
        }
        if (!presetsEdited) UnsavedPresetChanges = false;
        UpdateLayoutSave();

        std::string lines;
        for (const auto& l : applied.Describe()) lines += "\n    " + l;
        for (const auto& l : applied.layout.Warnings()) lines += "\n    " + l;
        spdlog::info("Brought in the changes someone else saved to {}:{}", names, lines);
        _pendingIncoming.reset();
        ShowShowFileBanner(wxString::FromUTF8(BannerText(who, kinds, in->ChangedItems(), 0)), false);
        if (!lines.empty()) {
            _showFileBannerText->SetToolTip(_showFileBannerText->GetLabel() + ":" + wxString::FromUTF8(lines));
        }
        return;
    }
    // Kept changing while being decided.
    if (_pendingIncoming != nullptr) {
        ShowShowFileBanner(wxString::FromUTF8(ShowFileLiveMerge::BannerText(std::string(), _pendingIncoming->ChangedKinds(),
                                                                             _pendingIncoming->ChangedItems(), _pendingIncoming->Decisions())),
                           true);
    }
}

void xLightsFrame::RefreshAfterIncomingShowChanges(const ShowFileLiveMerge::Applied& applied)
{
    if (applied.droppedEffects > 0 || applied.layout.ModelsChanged()) {
        _sequenceElements.PopulateRowInformation();
        if (mainSequencer != nullptr) {
            mainSequencer->PanelEffectGrid->Refresh();
            mainSequencer->PanelRowHeadings->Refresh();
        }
    }
    if (applied.layoutApplied) {
        pugi::xml_document result;
        if (result.load_string(applied.layoutResult.c_str())) {
            ReloadMergedLayoutSections(result.document_element(), applied.layout.otherSections);
        }
        const auto& changes = applied.layout;
        if (changes.viewsReloaded) {
            displayElementsPanel->SetSequenceElementsModelsViews(&_seqData, &_sequenceElements, &_sequenceViewManager);
        }
        if (changes.viewsReloaded || !changes.modelsRenamed.empty() || !changes.modelsRemoved.empty()) {
            displayElementsPanel->UpdateModelsForSelectedView();
        }
        if (changes.Any()) {
            for (uint32_t work : { OutputModelManager::WORK_RELOAD_MODELLIST, OutputModelManager::WORK_RELOAD_OBJECTLIST,
                                   OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER,
                                   OutputModelManager::WORK_CALCULATE_START_CHANNELS }) {
                _outputModelManager.AddASAPWork(work, "IncomingShowChanges");
            }
        }
    }
    if (applied.controllersReloaded) {
        for (uint32_t work : { OutputModelManager::WORK_UPDATE_NETWORK_LIST, OutputModelManager::WORK_CALCULATE_START_CHANNELS,
                               OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER }) {
            _outputModelManager.AddASAPWork(work, "IncomingShowChanges");
        }
    }
    if (applied.presetsReloaded && EffectTreeDlg != nullptr && m_mgr->GetPane("EffectPresets").IsShown()) {
        EffectTreeDlg->InitItems(_effectPresetManager);
        _effectPresetsInitialized = true;
    }
}
