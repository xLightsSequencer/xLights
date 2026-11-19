/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Bringing changes someone else saved to the shared show files into the open
// show while it is open. Desktop: src-ui-wx/app-shell/ShowFileWatch.cpp.

#include "iPadRenderContext.h"

#include <algorithm>

#include "render/ShowFileLiveMerge.h"
#include "render/ShowMergeApply.h"
#include "utils/ShowFileGuard.h"
#include "utils/ShowFileWatcher.h"
#include "utils/ShowPresence.h"

#include <globals.h>
#include <log.h>

#include <filesystem>
#include <sstream>

namespace {

ShowFileLiveMerge::Reader CoordinatedReader() {
    return [](const std::string& p) { return ShowFileWatcher::CoordinatedRead(p); };
}

std::string FileNames(const std::vector<std::string>& paths) {
    std::string out;
    for (const auto& p : paths) {
        if (!out.empty()) out += ", ";
        out += std::filesystem::path(p).filename().string();
    }
    return out;
}

std::string Who(const std::string& showDir, const std::vector<ShowFileLiveMerge::FileKind>& kinds) {
    auto& presence = ShowPresencePublisher::Instance();
    if (!presence.IsEnabled()) return {};
    return ShowFileLiveMerge::WhoChanged(ShowPresence::FreshOthers(showDir, presence.InstallId(), ShowPresence::Now()), kinds);
}

} // namespace

std::vector<ShowFileLiveMerge::LocalFile> iPadRenderContext::LiveShowFiles() {
    using ShowFileLiveMerge::FileKind;
    std::vector<ShowFileLiveMerge::LocalFile> files;
    if (showDirectory.empty()) return files;
    const std::string rgbPath = showDirectory + "/" + XLIGHTS_RGBEFFECTS_FILE;
    // This session's version of the layout is the loaded file with its edits
    // patched in (as a save writes it), so only the records edited here are mine.
    ShowFileLiveMerge::LocalFile rgb{ FileKind::RgbEffects, rgbPath, false, std::string() };
    if (HasLayoutDirt()) {
        if (auto mine = MemoryLayoutText()) {
            rgb.edited = true;
            rgb.mine = std::move(*mine);
        }
    }
    files.push_back(std::move(rgb));
    const std::string& networksPath = _outputManager.GetNetworksFilePath();
    if (!networksPath.empty()) {
        files.push_back({ FileKind::Networks, networksPath, _layoutDirt.Controllers(),
                          _layoutDirt.Controllers() ? _outputManager.SaveToString() : std::string() });
    }
    // Preset edits are saved as they are made.
    files.push_back({ FileKind::Presets, GetEffectPresetsPath(), false, std::string() });
    return files;
}

void iPadRenderContext::KeepLiveSessionBase(const ShowFileLiveMerge::Applied& applied) {
    // Memory was patched from a layout a live session brought in; it is now
    // the merged result, which the disk does not have either.
    if (applied.layoutApplied && _layoutDirt.Base() && !applied.layoutResult.empty()) {
        _layoutDirt.AdoptBase(applied.layoutResult);
    }
}

void iPadRenderContext::StopWatchingShowFiles() {
    if (_showFileWatcher) _showFileWatcher->Stop();
    _pendingIncoming.reset();
    _pendingMergeLive = false;
}

iPadRenderContext::IncomingShowChanges iPadRenderContext::CheckShowFilesOnDisk(std::string* banner) {
    if (showDirectory.empty()) {
        StopWatchingShowFiles();
        return IncomingShowChanges::None;
    }
    if (!_showFileWatcher) {
        _showFileWatcher = std::make_unique<ShowFileWatcher>(ShowFileGuard::Instance());
    }
    std::vector<std::string> paths{ showDirectory + "/" + XLIGHTS_RGBEFFECTS_FILE };
    if (!_outputManager.GetNetworksFilePath().empty()) paths.push_back(_outputManager.GetNetworksFilePath());
    paths.push_back(GetEffectPresetsPath());
    _showFileWatcher->Watch(paths);

    if (_pendingMergeLive) {
        // The conflict sheet is open on them.
        return IncomingShowChanges::None;
    }
    if (std::chrono::steady_clock::now() < _incomingRetryAt) {
        // Backing off after the models or the render were busy.
        return IncomingShowChanges::None;
    }
    // Called from a main-actor timer: the file I/O runs on a worker (the show
    // folder may be in iCloud or on a slow share). Each call collects the last
    // poll's result and starts the next one.
    auto changed = _showFileWatcher->TakeBackgroundPoll();
    if (!_showFileWatcher->BackgroundPollRunning()) {
        _showFileWatcher->StartBackgroundPoll();
    }
    if (!changed) {
        return IncomingShowChanges::None;
    }
    if (_pendingIncoming) {
        // A save may have merged them meanwhile (judged from the poll's reads).
        bool outstanding = false;
        for (const auto& p : _pendingIncoming->ChangedPaths()) {
            outstanding |= !_showFileWatcher->MatchedRecordedAtLastRead(p).value_or(false);
        }
        if (!outstanding) {
            _pendingIncoming.reset();
            return IncomingShowChanges::Cleared;
        }
    }
    if (changed->empty()) {
        return IncomingShowChanges::None;
    }
    return HandleIncomingShowChanges(banner);
}

iPadRenderContext::IncomingShowChanges iPadRenderContext::HandleIncomingShowChanges(std::string* banner, bool userInitiated) {
    if (GetSequenceElements().IsEditInProgress()) {
        return IncomingShowChanges::None;
    }
    if (HoldsIncomingShowChanges()) {
        // The host brings them in, and they reach this device as its edit.
        return IncomingShowChanges::None;
    }
    using namespace ShowFileLiveMerge;
    auto& guard = ShowFileGuard::Instance();
    ShowFileMerge::Options options;
    options.modelRenames = &AllModels.GetRenameLog();
    options.viewObjectRenames = &AllObjects.GetRenameLog();
    options.controllerRenames = &_outputManager.GetRenameLog();
    // Prepared from what the background poll read: no coordinated read (which
    // waits for any other writer) on the main actor. A file changed since is
    // reported again by the next poll.
    Reader polled = [this](const std::string& p) -> std::optional<std::pair<std::string, ShowFileFingerprint>> {
        return _showFileWatcher ? _showFileWatcher->LastReadContents(p) : std::nullopt;
    };
    auto in = std::make_unique<Incoming>(Incoming::Prepare(LiveShowFiles(), options, guard, ShowMergeApply::SequenceUseOf(*this), polled));
    if (_pendingIncoming) in->AdoptResolutions(*_pendingIncoming);
    if (!in->HasChanges()) {
        const bool had = _pendingIncoming != nullptr;
        _pendingIncoming.reset();
        return had ? IncomingShowChanges::Cleared : IncomingShowChanges::None;
    }
    const auto kinds = in->ChangedKinds();
    const std::string who = Who(showDirectory, kinds);
    const std::string names = FileNames(in->ChangedPaths());
    if (!in->CanMerge()) {
        spdlog::warn("iPadRenderContext: {} changed on disk but cannot be merged now; it is handled when saving", names);
        _pendingIncoming.reset();
        if (banner) *banner = BannerText(who, kinds, 0, 0) + ". It could not be merged; saving will ask what to do.";
        return IncomingShowChanges::CannotMerge;
    }
    if (in->NeedsDecisions()) {
        spdlog::info("iPadRenderContext: {} changed on disk; {} item(s) need a decision", names, in->Decisions());
        if (banner) *banner = BannerText(who, kinds, in->ChangedItems(), in->Decisions());
        _pendingIncoming = std::move(in);
        return IncomingShowChanges::NeedsDecisions;
    }
    // On the main actor: the timer must not sit waiting for a busy gate or a
    // render for seconds, again on every tick.
    ModelMutationScope mutate(*this, userInitiated ? 5000 : 50);
    if (!mutate.ok()) {
        const int delaySec = std::min(60, 2 << std::min(_incomingRetries, 5));
        ++_incomingRetries;
        spdlog::info("iPadRenderContext: {} changed on disk; the models or the render are busy, trying again in {}s", names, delaySec);
        _incomingRetryAt = std::chrono::steady_clock::now() + std::chrono::seconds(delaySec);
        _showFileWatcher->RearmAll();
        return IncomingShowChanges::None;
    }
    _incomingRetries = 0;
    _incomingRetryAt = {};
    const bool controllersEdited = _layoutDirt.Controllers();
    Applied applied = Apply(*this, *in, guard);
    if (!controllersEdited) _outputManager.MarkSaved();
    KeepLiveSessionBase(applied);
    std::string lines;
    for (const auto& l : applied.Describe()) lines += "\n    " + l;
    spdlog::info("iPadRenderContext: brought in the changes someone else saved to {}:{}", names, lines);
    if (banner) *banner = BannerText(who, kinds, in->ChangedItems(), 0);
    _pendingIncoming.reset();
    return IncomingShowChanges::Applied;
}

iPadRenderContext::IncomingShowChanges iPadRenderContext::ReviewIncomingShowChanges(std::string* banner) {
    if (!_pendingIncoming) return IncomingShowChanges::Cleared;
    // Merge again from what memory and the disk hold now, keeping any
    // decisions made earlier.
    auto status = HandleIncomingShowChanges(banner, true);
    _pendingMergeLive = status == IncomingShowChanges::NeedsDecisions && _pendingIncoming != nullptr;
    return status;
}

bool iPadRenderContext::CommitIncomingShowChanges(std::string* report, std::string* error, bool* needsDecisions) {
    using namespace ShowFileLiveMerge;
    auto fail = [&](const std::string& msg) {
        if (error) *error = msg;
        return false;
    };
    if (!_pendingIncoming) {
        _pendingMergeLive = false;
        return fail("Nothing to apply.");
    }
    if (!_pendingIncoming->CanApply()) {
        if (needsDecisions) *needsDecisions = true;
        return fail("Choose a version for every conflict first.");
    }
    if (!_pendingIncoming->StillOnDisk(CoordinatedReader())) {
        ShowFileMerge::Options options;
        options.modelRenames = &AllModels.GetRenameLog();
        options.viewObjectRenames = &AllObjects.GetRenameLog();
        options.controllerRenames = &_outputManager.GetRenameLog();
        auto again = std::make_unique<Incoming>(Incoming::Prepare(LiveShowFiles(), options, ShowFileGuard::Instance(),
                                                                  ShowMergeApply::SequenceUseOf(*this), CoordinatedReader()));
        again->AdoptResolutions(*_pendingIncoming);
        _pendingIncoming = std::move(again);
        if (!_pendingIncoming->CanMerge()) {
            _pendingMergeLive = false;
            _pendingIncoming.reset();
            return fail("The files changed again and can no longer be merged. Saving will ask what to do.");
        }
        if (!_pendingIncoming->CanApply()) {
            if (needsDecisions) *needsDecisions = true;
            return fail("The files changed again. Review the choices below.");
        }
    }
    ModelMutationScope mutate(*this);
    if (!mutate.ok()) {
        return fail("A render is still running. Try again in a moment.");
    }
    const bool controllersEdited = _layoutDirt.Controllers();
    Applied applied = Apply(*this, *_pendingIncoming, ShowFileGuard::Instance());
    if (!controllersEdited) _outputManager.MarkSaved();
    KeepLiveSessionBase(applied);
    std::string text;
    for (const auto& l : applied.layout.Warnings()) {
        text += (text.empty() ? "" : "\n") + l;
    }
    if (applied.droppedEffects > 0) {
        text += (text.empty() ? "" : "\n") + std::to_string(applied.droppedEffects) +
                " effects on submodel or strand rows the new model does not have were dropped.";
    }
    if (report) *report = text;
    spdlog::info("iPadRenderContext: brought in the decided changes to {}", FileNames(_pendingIncoming->ChangedPaths()));
    _pendingIncoming.reset();
    _pendingMergeLive = false;
    return true;
}
