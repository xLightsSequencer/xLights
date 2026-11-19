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
#include <wx/propgrid/propgrid.h>

#include <filesystem>
#include <map>
#include <optional>
#include <set>

#include <pugixml.hpp>

#include "xLightsMain.h"
#include "app-shell/Collaboration.h"
#include "app-shell/ShowFileMergeDialog.h"
#include "collab/CollabLayout.h"
#include "layout/CollabLayoutOverlay.h"
#include "sequencer/EffectTreeDialog.h"
#include "layout/LayoutPanel.h"
#include "layout/ViewsModelsPanel.h"
#include "render/ShowFileLiveMerge.h"
#include "render/ShowMergeApply.h"
#include "sequencer/MainSequencer.h"
#include "utils/LayoutObjectId.h"
#include "utils/LayoutRecords.h"
#include "utils/ShowFileGuard.h"
#include "utils/ShowFileMerge.h"
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

// The keys (layout id, and name) of the model, group and view object records
// of `from` that `to` changes or removes: the in-memory objects that applying
// `to` replaces or frees.
std::set<std::string> ReplacedRecords(const pugi::xml_document& from, const pugi::xml_document& to) {
    std::set<std::string> out;
    pugi::xml_node rf = from.document_element();
    pugi::xml_node rt = to.document_element();
    for (const auto& [section, tag] : { std::pair{ "models", "model" }, std::pair{ "modelGroups", "modelGroup" }, std::pair{ "view_objects", "view_object" } }) {
        std::map<std::string, pugi::xml_node> byId;
        std::map<std::string, pugi::xml_node> byName;
        for (pugi::xml_node n : rt.child(section).children(tag)) {
            std::string id = LayoutObjectId::Read(n);
            if (!id.empty()) {
                byId.emplace(id, n);
            }
            byName.emplace(LayoutRecords::NameOf(n), n);
        }
        for (pugi::xml_node n : rf.child(section).children(tag)) {
            std::string id = LayoutObjectId::Read(n);
            std::string name = LayoutRecords::NameOf(n);
            pugi::xml_node match;
            if (auto it = byId.find(id); !id.empty() && it != byId.end()) {
                match = it->second;
            } else if (auto it2 = byName.find(name); it2 != byName.end() && (id.empty() || LayoutObjectId::Read(it2->second).empty())) {
                match = it2->second;
            }
            if (!match || !ShowFileMerge::RecordsEqual(ShowFileMerge::FileKind::RgbEffects, n, match)) {
                if (!id.empty()) {
                    out.insert(id);
                }
                out.insert(name);
            }
        }
    }
    return out;
}

bool LayoutGridTypingInProgress() {
    for (wxWindow* w = wxWindow::FindFocus(); w != nullptr; w = w->GetParent()) {
        if (auto* grid = dynamic_cast<wxPropertyGrid*>(w)) {
            return grid->IsEditorsValueModified();
        }
        if (w->IsTopLevel()) {
            break;
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
    const bool netsEdited = !nets.empty() && (UnsavedNetworkChanges || _outputManager.IsDirty());
    std::string rgbText;
    std::string netsText;
    SerializeLayoutFiles(rgbEdited ? &rgbText : nullptr, netsEdited ? &netsText : nullptr);
    files.push_back({ FileKind::RgbEffects, rgb, rgbEdited, std::move(rgbText) });
    if (!nets.empty()) {
        files.push_back({ FileKind::Networks, nets, netsEdited, std::move(netsText) });
    }
    const bool presetsEdited = UnsavedPresetChanges;
    files.push_back({ FileKind::Presets, presets, presetsEdited, presetsEdited ? _effectPresetManager.SaveToText() : std::string() });
    return files;
}

void xLightsFrame::SerializeLayoutFiles(std::string* rgbEffects, std::string* networks)
{
    if (rgbEffects != nullptr) {
        *rgbEffects = BuildEffectsXml();
    }
    if (networks != nullptr) {
        *networks = _outputManager.SaveToString();
    }
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
    // A collaboration session's layout comes from its host; the host brings
    // disk changes in and they reach everyone as the host's edits.
    if (!CollabHostOnlyReason("bring show file changes in").empty()) {
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
    // Nor in the middle of an edit (an import, undo or batch reached from the
    // event loop while it waited for something).
    if (ModalDialogOpen() || wxWindow::GetCapture() != nullptr || playType == PLAY_TYPE_MODEL || playType == PLAY_TYPE_EFFECT ||
        _sequenceElements.IsEditInProgress()) {
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
    if (_inShowFileCheck || _sequenceElements.IsEditInProgress()) {
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
    // The merge is prepared from what the background poll read, so this
    // doesn't wait on the share or a coordinated writer on the UI thread.
    // A file changed since is reported again by the next poll.
    const Reader polled = [this](const std::string& p) -> std::optional<std::pair<std::string, ShowFileFingerprint>> {
        return _showFileWatcher != nullptr ? _showFileWatcher->LastReadContents(p) : std::nullopt;
    };

    for (int attempt = 0; attempt < 3; ++attempt) {
        auto in = std::make_unique<Incoming>(Incoming::Prepare(LiveShowFiles(), options, guard, ShowMergeApply::SequenceUseOf(*this), polled));
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
    if (applied.layoutApplied || applied.controllersReloaded || applied.droppedEffects > 0) {
        ++_layoutChanges;
    }
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

uint64_t xLightsFrame::LayoutGeneration() const
{
    return UnsavedRgbEffectsChanges.Generation() + UnsavedNetworkChanges.Generation() + _layoutChanges + AllModels.GetModelGeneration();
}

bool xLightsFrame::LayoutEditInProgress()
{
    return wxWindow::GetCapture() != nullptr || ModalDialogOpen() || LayoutGridTypingInProgress() ||
           (layoutPanel != nullptr && layoutPanel->IsLayoutEditInProgress()) || _inShowFileCheck;
}

bool xLightsFrame::ApplySessionLayout(const Collab::LayoutFiles& current, const Collab::LayoutFiles& target, Collab::LayoutApplied& out)
{
    out = Collab::LayoutApplied();
    const bool rgbDiffers = current.rgbEffects != target.rgbEffects;
    const bool netsDiffer = current.networks != target.networks;
    if (!rgbDiffers && !netsDiffer) {
        return true;
    }
    if (IsLayoutLocked()) {
        return false;
    }
    // A drag, a model being created and an open dialog (the modal layout
    // editors, the bulk edits) hold pointers to the models they work on, and
    // the models being rebuilt may be among them: wait until they are done.
    if (wxWindow::GetCapture() != nullptr || ModalDialogOpen() || (layoutPanel != nullptr && layoutPanel->IsLayoutEditInProgress()) ||
        _inShowFileCheck) {
        return false;
    }
    pugi::xml_document mineDoc;
    pugi::xml_document resultDoc;
    std::set<std::string> replaced;
    if (rgbDiffers) {
        if (!mineDoc.load_string(current.rgbEffects.c_str()) || !resultDoc.load_string(target.rgbEffects.c_str())) {
            spdlog::error("Collab: a session layout does not parse; not applying it");
            return false;
        }
        replaced = ReplacedRecords(mineDoc, resultDoc);
    }
    auto touched = [&](const std::string& key) { return replaced.count(key) > 0; };
    // Typing not yet committed in the property grid of an object about to be
    // rebuilt would be lost (or land on a freed object).
    if (layoutPanel != nullptr && LayoutGridTypingInProgress() && touched(CollabLayoutOverlay::KeyOf(layoutPanel->GetSelectedBaseObject()))) {
        return false;
    }

    // Stopping a render may run other events while it waits; edits made then
    // would be in neither side.
    const uint64_t generation = LayoutGeneration();
    if (!AbortRender()) {
        return false;
    }
    if (LayoutGeneration() != generation) {
        return false;
    }

    std::optional<std::string> reselect;
    if (layoutPanel != nullptr && !replaced.empty()) {
        reselect = layoutPanel->ReleaseSelectionFor(touched);
    }
    bool removesModels = false;
    if (rgbDiffers) {
        std::set<std::string> after;
        for (const char* section : { "models", "modelGroups" }) {
            for (pugi::xml_node n : resultDoc.document_element().child(section).children()) {
                after.insert(LayoutRecords::NameOf(n));
            }
        }
        for (const char* section : { "models", "modelGroups" }) {
            for (pugi::xml_node n : mineDoc.document_element().child(section).children()) {
                removesModels |= after.count(LayoutRecords::NameOf(n)) == 0;
            }
        }
    }
    if (removesModels) {
        // Rows or effects the selection points at may go with their model.
        ClearEffectSelectionForDocumentChange();
        _sequenceElements.UnSelectAllEffects();
        if (mainSequencer != nullptr) {
            mainSequencer->PanelEffectGrid->ClearSelection();
        }
    }

    ShowFileLiveMerge::Applied applied;
    if (rgbDiffers) {
        applied.layout = ShowMergeApply::ApplyLayout(*this, mineDoc, resultDoc);
        applied.layoutApplied = true;
        applied.layoutResult = target.rgbEffects;
    }
    if (netsDiffer) {
        applied.controllersReloaded = ShowMergeApply::ReloadControllers(*this, target.networks, true);
    }
    RefreshAfterIncomingShowChanges(applied);
    if (applied.controllersReloaded && layoutPanel != nullptr) {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "CollabLayout");
    }

    // The host saves the session's layout, and a client on its own show folder
    // may save it there after the session. A session copy is thrown away, so
    // it is never marked (restoring the show folder would offer to save it),
    // not even on a client that took over hosting.
    if (_collab != nullptr && !_collab->OnSessionCopy()) {
        if (rgbDiffers) {
            UnsavedRgbEffectsChanges = { true, "collaboration session layout change" };
        }
        if (netsDiffer) {
            UnsavedNetworkChanges = { true, "collaboration session controller change" };
        }
        UpdateLayoutSave();
    }

    if (reselect && !reselect->empty() && layoutPanel != nullptr) {
        layoutPanel->ReselectLayoutObject(*reselect);
    }

    const auto& c = applied.layout;
    out.layoutChanged = applied.layoutApplied && c.Any();
    out.controllersChanged = applied.controllersReloaded;
    out.modelsChanged = c.modelsAdded;
    out.modelsChanged.insert(out.modelsChanged.end(), c.modelsChanged.begin(), c.modelsChanged.end());
    out.modelsRemoved = c.modelsRemoved;
    out.modelsRenamed = c.modelsRenamed;
    for (const auto& l : c.Warnings()) {
        spdlog::info("Collab: layout commit: {}", l);
    }
    return true;
}
