/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "render/xLightsShowContext.h"

#include "render/ValueCurve.h"
#include "render/Element.h"
#include "render/RenderEngine.h"
#include "render/RenderProgressInfo.h"
#include "render/IRenderProgressSink.h"
#include "render/SequenceFile.h"
#include "models/Model.h"
#include "models/DisplayAsType.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <thread>

xLightsShowContext::MissingModelScan xLightsShowContext::ScanForMissingModels() const {
    MissingModelScan scan;

    for (const auto& it : AllModels) {
        if (it.second != nullptr) {
            scan.availableNames.push_back(it.first);
            if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
                scan.availableModels.push_back(it.first);
            }
        }
    }

    auto removeName = [](std::vector<std::string>& names, const std::string& str) {
        names.erase(std::remove(names.begin(), names.end(), str), names.end());
    };

    for (int x = _sequenceElements.GetElementCount() - 1; x >= 0; x--) {
        Element* element = _sequenceElements.GetElement(x);
        if (element && ElementType::ELEMENT_TYPE_MODEL == element->GetType()) {
            const std::string name = element->GetModelName();
            if (AllModels[name] == nullptr && element->GetEffectCount() > 0) {
                // Not present under its own name — see whether some model claims
                // it as an alias (an old-name alias remaps later; a current-name
                // alias means it's genuinely missing and worth reporting).
                for (const auto& it : AllModels) {
                    if (it.second->IsAlias(name, true)) {
                        // will map to an alias later, skip it
                    } else if (it.second->IsAlias(name, false)) {
                        scan.missing.push_back(name + "(" + std::to_string(element->GetEffectCount()) + ")");
                    }
                }
            }
            // Drop the models the sequence uses so what remains is the pool of
            // layout models a remap UI can offer as targets.
            removeName(scan.availableNames, name);
            removeName(scan.availableModels, name);
        }
    }
    return scan;
}

void xLightsShowContext::CheckForValidModels() {
    // Base (headless) behavior: no interactive remap — just report. The desktop
    // frame overrides this with the SeqElementMismatchDialog flow.
    const MissingModelScan scan = ScanForMissingModels();
    for (const auto& m : scan.missing) {
        spdlog::warn("Sequence references a model that is not in the layout: {}", m);
    }
}

bool xLightsShowContext::LoadSequenceElements(SequenceFile& file, pugi::xml_document& doc) {
    // Frequency must be set BEFORE LoadSequencerFile: that loader rounds every
    // effect start/end to the frame period (RoundToMultipleOfPeriod), and the
    // default 20Hz/50ms grid shifts effects a frame on 40fps sequences.
    _sequenceElements.SetFrequency(file.GetFrequency());

    // Views manager must be set before LoadSequencerFile so rows resolve views.
    _sequenceElements.SetViewsManager(&_sequenceViewManager);

    if (!_sequenceElements.LoadSequencerFile(file, doc, showDirectory)) {
        return false;
    }

    // Migrate legacy effect settings to the current format; without this older
    // sequences render with un-migrated settings.
    file.AdjustEffectSettingsForVersion(_sequenceElements, this);

    // Resolve any models the sequence references that aren't in the layout. This
    // can add/rename elements (desktop remap), so it must run before PrepareViews.
    CheckForValidModels();

    // PrepareViews grows mAllViews to one slot per view; without it switching
    // views crashes in PopulateRowInformation.
    _sequenceElements.PrepareViews(file);
    _sequenceElements.PopulateRowInformation();

    OnSequenceElementsLoaded(file);

    // Route new timing-track additions through the live in-memory path.
    file.SetSequenceLoaded(true);
    // ValueCurve expressions referencing timing tracks need the elements.
    ValueCurve::SetSequenceElements(&_sequenceElements);
    return true;
}

bool xLightsShowContext::IsInShowFolder(const std::string& file) const {
    return file.find(showDirectory) == 0;
}

bool xLightsShowContext::IsInShowOrMediaFolder(const std::string& file) const {
    if (IsInShowFolder(file)) return true;
    for (const auto& folder : mediaDirectories) {
        if (file.find(folder) == 0) return true;
    }
    return false;
}

std::string xLightsShowContext::MakeRelativePath(const std::string& file) const {
    if (file.find(showDirectory) == 0 && file.size() > showDirectory.size() + 1) {
        return file.substr(showDirectory.size() + 1);
    }
    return file;
}

bool xLightsShowContext::IsRenderDone() {
    if (!_renderEngine) return true;
    _renderEngine->CheckForStalledRender();

    // Drain finished progress entries (safe: called from the driver/main thread,
    // not a render worker). Any still-pending entry keeps the result false.
    auto& list = _renderEngine->GetRenderProgressInfo();
    bool allDone = true;
    for (auto it = list.begin(); it != list.end();) {
        RenderProgressInfo* rpi = *it;
        if (rpi->completed.load()) {
            rpi->CleanupJobs();
            if (rpi->callback) rpi->callback(_renderEngine->GetAbortedRenderJobs() > 0);
            delete rpi;
            it = list.erase(it);
        } else {
            allDone = false;
            ++it;
        }
    }
    return allDone;
}

bool xLightsShowContext::AbortRender(int maxTimeMs) {
    if (!_renderEngine) return true;
    if (IsRenderDone()) return true;
    _renderEngine->SignalAbort();
    if (maxTimeMs <= 0) maxTimeMs = 60000;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(maxTimeMs);
    while (!IsRenderDone() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return IsRenderDone();
}

void xLightsShowContext::EnsureSequenceDataSized() {
    if (!_sequenceFile) return;
    const unsigned int numFrames =
        (unsigned int)(_sequenceFile->GetSequenceDurationMS() / _sequenceFile->GetFrameMS());
    unsigned int numChannels = (unsigned int)_outputManager.GetTotalChannels();
    if (numChannels == 0) numChannels = 1;
    const unsigned int frameTime = (unsigned int)_sequenceFile->GetFrameMS();

    if (_seqData.IsValidData()
        && _seqData.NumChannels() == numChannels
        && _seqData.NumFrames() == numFrames
        && _seqData.FrameTime() == frameTime) {
        return;
    }
    // init() Cleanup()s and frees the seqData storage; a render job mid-
    // GetColors/SetColors holds a raw pointer into it, so drain any in-flight
    // render first rather than pull the buffer out from under a live job.
    if (!AbortRender()) {
        spdlog::error("xLightsShowContext: could not abort in-flight render; skipping the seqData resize");
        return;
    }
    _seqData.init(numChannels, numFrames, frameTime);
}

void xLightsShowContext::CloseSequence() {
    // Drain any in-flight render before destroying the elements / seq data the
    // render workers read (else a use-after-free when opening the next sequence
    // while the first is still rendering).
    if (_renderEngine) {
        AbortRender(5000);
    }
    _sequenceElements.Clear();
    _seqData.Cleanup();
    _sequenceDoc.reset();
    _sequenceFile.reset();
}
