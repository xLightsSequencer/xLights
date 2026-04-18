/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "iPadRenderContext.h"

#include "render/Element.h"
#include "render/RenderProgressInfo.h"
#include "models/Model.h"
#include "models/Node.h"
#include "render/ValueCurve.h"
#include "utils/ExternalHooks.h"

#include <pugixml.hpp>
#include "utils/FileUtils.h"
#include <log.h>

#include <algorithm>
#include <thread>

iPadRenderContext::iPadRenderContext()
    : _effectManager(FileUtils::GetResourcesDir() + "/effectmetadata"),
      _sequenceElements(this) {
}

iPadRenderContext::~iPadRenderContext() {
    CloseSequence();
}

bool iPadRenderContext::LoadShowFolder(const std::string& showDir) {
    return LoadShowFolder(showDir, {});
}

bool iPadRenderContext::LoadShowFolder(const std::string& showDir,
                                       const std::list<std::string>& mediaFolders) {
    _showDir = showDir;
    _mediaFolders = mediaFolders;

    ObtainAccessToURL(showDir, false);
    for (const auto& folder : _mediaFolders) {
        ObtainAccessToURL(folder, false);
    }

    // Wire the show dir + media folders into FileUtils::FixFile so that
    // sequence references (audio, videos, images, 3D meshes, shaders, etc.)
    // that were saved with absolute paths from another machine get re-resolved
    // against the iPad's current show/media locations. Without this,
    // _fixFileSearchDirs stays empty and FixFile has no way to relocate
    // assets — the raw saved paths fall straight through and FileExists fails.
    FileUtils::SetFixFileShowDir(showDir);
    FileUtils::SetFixFileDirectories(_mediaFolders);
    FileUtils::ClearNonExistentFiles();

    // Load network/controller configuration
    if (!_outputManager.Load(showDir)) {
        spdlog::warn("iPadRenderContext: Failed to load xlights_networks.xml from {}", showDir);
    }

    // Create ModelManager + ViewObjectManager
    _modelManager = std::make_unique<ModelManager>(&_outputManager, this);
    _viewObjectManager = std::make_unique<ViewObjectManager>(this);
    _viewsManager.SetModelManager(_modelManager.get());

    // Load models from xlights_rgbeffects.xml
    std::string rgbPath = showDir + "/xlights_rgbeffects.xml";
    ObtainAccessToURL(rgbPath, false);

    spdlog::info("iPadRenderContext: Loading rgbeffects from {}", rgbPath);
    if (FileExists(rgbPath)) {
        spdlog::info("iPadRenderContext: File exists: {}", rgbPath);
    } else {
        spdlog::error("iPadRenderContext: File NOT found: {}", rgbPath);
    }

    pugi::xml_document doc;
    auto result = doc.load_file(rgbPath.c_str());
    if (result) {
        auto xlightsNode = doc.child("xrgb");
        if (!xlightsNode) {
            xlightsNode = doc.child("xlights");
        }
        if (!xlightsNode) {
            spdlog::error("iPadRenderContext: No <xrgb> or <xlights> root element in {}", rgbPath);
        } else {
            auto modelsNode = xlightsNode.child("models");
            if (!modelsNode) {
                spdlog::error("iPadRenderContext: No <models> element in {}", rgbPath);
            } else {
                _modelManager->LoadModels(modelsNode, 1920, 1080);
                spdlog::info("iPadRenderContext: Loaded {} models", _modelManager->GetModels().size());

                // Load model groups
                auto groupsNode = xlightsNode.child("modelGroups");
                if (groupsNode) {
                    _modelManager->LoadGroups(groupsNode, 1920, 1080);
                    spdlog::info("iPadRenderContext: Loaded groups, total models now {}",
                                 _modelManager->GetModels().size());
                }

                // Load view objects (house meshes, ground images, gridlines, terrain, rulers)
                auto viewObjectsNode = xlightsNode.child("view_objects");
                if (viewObjectsNode) {
                    _viewObjectManager->LoadViewObjects(viewObjectsNode);
                    spdlog::info("iPadRenderContext: Loaded {} view objects",
                                 _viewObjectManager->size());
                }

                // Load saved views. `SequenceViewManager::GetViews()` always
                // ensures a Master View entry, but the rest (Christmas,
                // Halloween, etc.) come from the <views> node.
                auto viewsNode = xlightsNode.child("views");
                if (viewsNode) {
                    _viewsManager.Load(viewsNode, 0);
                    spdlog::info("iPadRenderContext: Loaded {} views",
                                 _viewsManager.GetViewCount());
                }
            }
        }
    } else {
        spdlog::error("iPadRenderContext: Failed to load {}: {}", rgbPath, result.description());
    }

    return true;
}

bool iPadRenderContext::OpenSequence(const std::string& path) {
    CloseSequence();

    ObtainAccessToURL(path, false);

    _sequenceFile = std::make_unique<SequenceFile>(path);
    _sequenceDoc = _sequenceFile->Open(_showDir, false, path);

    if (!_sequenceDoc) {
        spdlog::warn("iPadRenderContext: Failed to open sequence {}", path);
        _sequenceFile.reset();
        return false;
    }

    // Must set the views manager BEFORE LoadSequencerFile so SequenceElements
    // can resolve the current view while populating rows. Desktop does the
    // same in `tabSequencer.cpp::NewSequence/OpenSequence`.
    _sequenceElements.SetViewsManager(&_viewsManager);

    if (!_sequenceElements.LoadSequencerFile(*_sequenceFile, *_sequenceDoc, _showDir)) {
        spdlog::warn("iPadRenderContext: Failed to load sequence elements from {}", path);
        _sequenceFile.reset();
        _sequenceDoc.reset();
        return false;
    }

    // PrepareViews grows mAllViews so there is one slot per view in the
    // view manager. Without this only the Master slot exists and switching
    // to any other view crashes inside PopulateRowInformation at
    // `mAllViews[mCurrentView]`. Desktop calls this from tabSequencer.cpp.
    _sequenceElements.PrepareViews(*_sequenceFile);

    _sequenceElements.PopulateRowInformation();

    // ValueCurve needs sequence elements for VC expressions referencing timing tracks
    ValueCurve::SetSequenceElements(&_sequenceElements);

    spdlog::info("iPadRenderContext: Row info size: {}, timing rows: {}",
                 _sequenceElements.GetRowInformationSize(),
                 _sequenceElements.GetNumberOfTimingElements());

    spdlog::info("iPadRenderContext: Opened sequence {} ({} elements, {} ms)",
                 path,
                 _sequenceElements.GetElementCount(),
                 _sequenceFile->GetSequenceDurationMS());
    return true;
}

void iPadRenderContext::CloseSequence() {
    _sequenceElements.Clear();
    _sequenceDoc.reset();
    _sequenceFile.reset();
}

bool iPadRenderContext::IsInShowFolder(const std::string& file) const {
    return file.find(_showDir) == 0;
}

bool iPadRenderContext::IsInShowOrMediaFolder(const std::string& file) const {
    if (IsInShowFolder(file)) return true;
    for (const auto& folder : _mediaFolders) {
        if (file.find(folder) == 0) return true;
    }
    return false;
}

std::string iPadRenderContext::MakeRelativePath(const std::string& file) const {
    if (file.find(_showDir) == 0 && file.size() > _showDir.size() + 1) {
        return file.substr(_showDir.size() + 1);
    }
    return file;
}

AudioManager* iPadRenderContext::GetCurrentMediaManager() const {
    return _sequenceFile ? _sequenceFile->GetMedia() : nullptr;
}

const std::string& iPadRenderContext::GetHeaderInfo(HEADER_INFO_TYPES type) const {
    if (_sequenceFile) {
        return _sequenceFile->GetHeaderInfo(type);
    }
    static const std::string empty;
    return empty;
}

Model* iPadRenderContext::GetModel(const std::string& name) const {
    return _modelManager ? _modelManager->GetModel(name) : nullptr;
}

TimingElement* iPadRenderContext::AddTimingElement(const std::string& /*name*/,
                                                    const std::string& /*subType*/) {
    return nullptr;
}

bool iPadRenderContext::AbortRender(int /*maxTimeMs*/) {
    if (_renderEngine) {
        _renderEngine->SignalAbort();
    }
    return true;
}

void iPadRenderContext::RenderEffectForModel(const std::string& model,
                                              int startms, int endms, bool clear) {
    if (_renderEngine && _sequenceData.IsValidData()) {
        _renderEngine->RenderEffectForModel(model, startms, endms,
                                             _sequenceElements, _sequenceData,
                                             false, _modelsChangeCount, clear);
    }
}

void iPadRenderContext::RenderAll() {
    if (!_sequenceFile || !_modelManager) return;

    int numFrames = _sequenceFile->GetSequenceDurationMS() / _sequenceFile->GetFrameMS();
    int numChannels = _outputManager.GetTotalChannels();
    if (numChannels == 0) numChannels = 1;

    _sequenceData.init(numChannels, numFrames, _sequenceFile->GetFrameMS());

    if (!_jobPool) {
        _jobPool = std::make_unique<JobPool>("RenderPool");
        // RenderEngine workers can block waiting on frames from other models;
        // with too few threads a contended sequence deadlocks. Oversubscribe
        // well past core count so a blocked worker never exhausts the pool.
        size_t hw = std::thread::hardware_concurrency();
        size_t poolThreads = std::max<size_t>(24, hw * 2);
        _jobPool->Start(poolThreads);
    }

    if (!_renderEngine) {
        _renderEngine = std::make_unique<RenderEngine>(*this, *_jobPool, _renderCache);
    }

    _renderEngine->BuildRenderTree(_sequenceElements, _modelsChangeCount);

    auto models = _renderEngine->GetRenderTree().GetModels();
    std::list<Model*> empty;

    _renderEngine->Render(_sequenceElements, _sequenceData,
                           models, empty,
                           0, numFrames - 1,
                           nullptr, true,
                           [](bool) {});

    spdlog::info("iPadRenderContext: RenderAll started for {} frames, {} channels",
                 numFrames, numChannels);
}

void iPadRenderContext::HandleMemoryWarning() {
    if (_renderEngine) {
        _renderEngine->SignalAbort();
    }
    _renderCache.Purge(nullptr, false);
    spdlog::warn("iPadRenderContext: memory warning handled -- aborted render, purged cache");
}

void iPadRenderContext::HandleMemoryCritical() {
    HandleMemoryWarning();
    // Phase D note: once Model Preview owns its own vertex / texture buffers,
    // free them here too. iPadModelPreview today rebuilds accumulators per
    // frame, so there's nothing extra to drop.
    spdlog::error("iPadRenderContext: memory critical handled");
}

bool iPadRenderContext::IsRenderDone() {
    // No render ever kicked off (or already torn down) — treat as
    // "done" so abort-and-wait short-circuits cleanly.
    if (!_renderEngine) return true;
    if (!_sequenceData.IsValidData()) return false;
    // Each RenderProgressInfo flips its `completed` atomic when its last
    // RenderJob signals via FinishNotifier (covers normal, aborted, and
    // early-bail exits). We both check completion and lazily drain finished
    // entries here -- called from the main thread, so cleanup + callback run
    // safely off the render workers. Any pending rpi keeps the result false.
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

void iPadRenderContext::SetModelColors(int frameMS) {
    if (!_sequenceData.IsValidData() || !_modelManager) return;

    int frame = frameMS / _sequenceData.FrameTime();
    if (frame < 0 || (unsigned int)frame >= _sequenceData.NumFrames()) return;

    auto& fd = _sequenceData[frame];
    auto models = _modelManager->GetModels();
    for (auto& [name, model] : models) {
        int chansPerNode = model->GetChanCountPerNode();
        for (size_t n = 0; n < model->GetNodeCount(); n++) {
            int32_t startChan = model->NodeStartChannel(n);
            if (startChan >= 0 && (unsigned int)startChan + chansPerNode <= _sequenceData.NumChannels()) {
                model->SetNodeChannelValues(n, &fd[startChan]);
            }
        }
    }
}

std::vector<iPadRenderContext::PixelData> iPadRenderContext::GetModelPixels(
    const std::string& modelName, int frameMS) {

    std::vector<PixelData> pixels;
    Model* model = GetModel(modelName);
    if (!model) return pixels;

    SetModelColors(frameMS);

    for (size_t n = 0; n < model->GetNodeCount(); n++) {
        xlColor color = model->GetNodeColor(n);
        std::vector<std::tuple<float, float, float>> pts;
        model->GetNode3DScreenCoords(n, pts);
        for (const auto& [sx, sy, sz] : pts) {
            pixels.push_back({sx, sy, color.red, color.green, color.blue});
        }
    }
    return pixels;
}

std::vector<iPadRenderContext::PixelData> iPadRenderContext::GetAllModelPixels(int frameMS) {
    std::vector<PixelData> allPixels;
    if (!_modelManager) return allPixels;

    SetModelColors(frameMS);

    static bool loggedOnce = false;
    auto models = _modelManager->GetModels();
    for (auto& [name, model] : models) {
        for (size_t n = 0; n < model->GetNodeCount(); n++) {
            xlColor color = model->GetNodeColor(n);
            std::vector<std::tuple<float, float, float>> pts;
            model->GetNode3DScreenCoords(n, pts);
            for (const auto& [sx, sy, sz] : pts) {
                allPixels.push_back({sx, sy, color.red, color.green, color.blue});
            }
        }
        if (!loggedOnce && model->GetNodeCount() > 0) {
            std::vector<std::tuple<float, float, float>> firstPts;
            model->GetNode3DScreenCoords(0, firstPts);
            if (!firstPts.empty()) {
                auto [fx, fy, fz] = firstPts[0];
                spdlog::info("Preview: model '{}' node0 screen=({},{},{}), color=({},{},{}), nodes={}",
                             name, fx, fy, fz,
                             model->GetNodeColor(0).red,
                             model->GetNodeColor(0).green,
                             model->GetNodeColor(0).blue,
                             model->GetNodeCount());
            }
        }
    }
    if (!loggedOnce && !allPixels.empty()) {
        spdlog::info("Preview: total {} pixels from {} models", allPixels.size(), models.size());
        loggedOnce = true;
    }
    return allPixels;
}
