/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "HeadlessRenderContext.h"

#include "render/RenderEngine.h"
#include "render/RenderProgressInfo.h"
#include "render/IRenderProgressSink.h"
#include "render/FSEQFileIO.h"
#include "render/ValueCurve.h"
#include "models/ModelManager.h"
#include "models/ViewObjectManager.h"
#include "outputs/OutputManager.h"
#include "utils/FileUtils.h"
#include "utils/ExternalHooks.h"
#include "utils/UtilFunctions.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <system_error>
#include <thread>

HeadlessRenderContext::~HeadlessRenderContext() {
    // Drain any in-flight render while this (fully-constructed) object's vtable
    // is still valid — the base dtor destroys _renderEngine afterwards.
    if (_renderEngine) {
        AbortRender(3000);
    }
    CloseSequence();
}

bool HeadlessRenderContext::LoadShowFolder(const std::string& showDir,
                                           const std::list<std::string>& mediaFolders) {
    showDirectory = showDir;
    fseqDirectory = showDir;
    mediaDirectories = mediaFolders;

    // Re-resolve saved (possibly cross-machine) asset paths against this show.
    FileUtils::SetFixFileShowDir(showDir);
    FileUtils::SetFixFileDirectories(mediaDirectories);
    FileUtils::ClearNonExistentFiles();

    if (!_outputManager.Load(showDir)) {
        spdlog::warn("HeadlessRenderContext: failed to load xlights_networks.xml from {}", showDir);
    }

    _sequenceViewManager.SetModelManager(&AllModels);

    const std::string rgbPath = showDir + "/xlights_rgbeffects.xml";
    if (!FileExists(rgbPath)) {
        spdlog::error("HeadlessRenderContext: {} not found", rgbPath);
        return false;
    }

    pugi::xml_document doc;
    if (!doc.load_file(rgbPath.c_str())) {
        spdlog::error("HeadlessRenderContext: failed to parse {}", rgbPath);
        return false;
    }

    auto root = doc.child("xrgb");
    if (!root) root = doc.child("xlights");
    if (!root) {
        spdlog::error("HeadlessRenderContext: no <xrgb>/<xlights> root in {}", rgbPath);
        return false;
    }

    if (auto settings = root.child("settings")) {
        for (auto s = settings.first_child(); s; s = s.next_sibling()) {
            const std::string name = s.name();
            const char* v = s.attribute("value").as_string();
            if (name == "previewWidth") {
                int w = (int)std::strtol(v, nullptr, 10);
                if (w > 0) _previewWidth = w;
            } else if (name == "previewHeight") {
                int h = (int)std::strtol(v, nullptr, 10);
                if (h > 0) _previewHeight = h;
            } else if (name == "fseqDir") {
                // Match the desktop: render into the show's configured fseq folder
                // (TabSequence.cpp reads GetXmlSetting("fseqDir")). Fall back to the
                // show dir if the stored (possibly cross-machine) path is gone.
                if (v && *v) {
                    std::error_code ec;
                    if (std::filesystem::exists(v, ec)) {
                        fseqDirectory = v;
                    } else {
                        spdlog::warn("HeadlessRenderContext: configured fseqDir '{}' not found; writing to show dir", v);
                    }
                }
            }
        }
    }

    auto modelsNode = root.child("models");
    if (!modelsNode) {
        spdlog::error("HeadlessRenderContext: no <models> element in {}", rgbPath);
        return false;
    }
    AllModels.LoadModels(modelsNode, _previewWidth, _previewHeight);

    if (auto groupsNode = root.child("modelGroups")) {
        AllModels.LoadGroups(groupsNode, _previewWidth, _previewHeight);
    }
    if (auto viewObjectsNode = root.child("view_objects")) {
        AllObjects.LoadViewObjects(viewObjectsNode);
    }
    if (auto viewsNode = root.child("views")) {
        _sequenceViewManager.Load(viewsNode, 0);
    }
    // 3D viewpoints/cameras — so "Per Preview" effects render in 3D via
    // GetNamedCamera3D, matching desktop.
    if (auto viewpointsNode = root.child("Viewpoints")) {
        viewpoint_mgr.Load(viewpointsNode);
    }

    AllModels.RecalcStartChannels();

    spdlog::info("HeadlessRenderContext: loaded {} models ({}x{}) from {}",
                 AllModels.GetModels().size(), _previewWidth, _previewHeight, showDir);
    return true;
}

bool HeadlessRenderContext::OpenSequence(const std::string& path) {
    CloseSequence();

    auto openStart = std::chrono::steady_clock::now();
    _sequenceFile = std::make_unique<SequenceFile>(path);
    _sequenceDoc = _sequenceFile->Open(showDirectory, false, path);
    if (!_sequenceDoc) {
        spdlog::warn("HeadlessRenderContext: failed to open sequence {}", path);
        _sequenceFile.reset();
        return false;
    }

    // Shared load steps (frequency, views manager, LoadSequencerFile, settings
    // migration, CheckForValidModels [base logs missing models], view prep). See
    // xLightsShowContext::LoadSequenceElements.
    if (!LoadSequenceElements(*_sequenceFile, *_sequenceDoc)) {
        spdlog::warn("HeadlessRenderContext: LoadSequencerFile failed for {}", path);
        _sequenceFile.reset();
        _sequenceDoc.reset();
        return false;
    }

    EnsureSequenceDataSized();

    auto openMS = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - openStart).count();
    spdlog::info("HeadlessRenderContext: opened {} ({} elements, {} ms long) in {} ms",
                 path, _sequenceElements.GetElementCount(),
                 _sequenceFile->GetSequenceDurationMS(), (long long)openMS);
    return true;
}

void HeadlessRenderContext::EnsureRenderEngine() {
    if (!_renderEngine) {
        jobPool.Start(RenderEngine::RecommendedPoolSize());
        _renderEngine = std::make_unique<RenderEngine>(*this, jobPool, _renderCache);
    }
}

bool HeadlessRenderContext::RenderAndWait(int timeoutMs) {
    if (!IsSequenceLoaded()) {
        spdlog::error("HeadlessRenderContext: RenderAndWait with no sequence loaded");
        return false;
    }
    EnsureSequenceDataSized();
    const unsigned int numFrames = _seqData.NumFrames();
    if (numFrames == 0) {
        spdlog::error("HeadlessRenderContext: sequence has zero frames");
        return false;
    }

    EnsureRenderEngine();
    _renderEngine->BuildRenderTree(_sequenceElements, modelsChangeCount);

    auto models = _renderEngine->GetRenderTree().GetModels();
    std::list<Model*> empty;
    _renderEngine->Render(_sequenceElements, _seqData, models, empty,
                          0, (int)numFrames - 1, nullptr, true, [](bool) {});

    spdlog::info("HeadlessRenderContext: rendering {} frames x {} channels...",
                 numFrames, _seqData.NumChannels());

    const auto start = std::chrono::steady_clock::now();
    while (!IsRenderDone()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        if (timeoutMs > 0
            && std::chrono::steady_clock::now() - start > std::chrono::milliseconds(timeoutMs)) {
            spdlog::warn("HeadlessRenderContext: render timed out after {} ms", timeoutMs);
            return false;
        }
    }
    return true;
}

void HeadlessRenderContext::RenderEffectForModel(const std::string& model, int startms, int endms, bool clear) {
    if (_renderEngine) {
        _renderEngine->RenderEffectForModel(model, startms, endms,
                                            _sequenceElements, _seqData,
                                            false /*suspendRender*/, modelsChangeCount, clear);
    }
}

bool HeadlessRenderContext::WriteFseq(const std::string& path) {
    if (!IsSequenceLoaded()) return false;
    FSEQFileIO::WriteOptions opts;
    opts.source = "xLights Headless";
    if (_sequenceFile) opts.mediaFile = _sequenceFile->GetMediaFile();

    // Embed the current on-disk show config + sequence so the .fseq is
    // self-describing, matching desktop's XR/XN/XS headers. Headless has no
    // unsaved edits, so the disk files are authoritative.
    const std::string sep = GetPathSeparator();
    opts.embedded.push_back({{'X', 'R'}, showDirectory + sep + "xlights_rgbeffects.xml", ""});
    opts.embedded.push_back({{'X', 'N'}, showDirectory + sep + "xlights_networks.xml", ""});
    if (_sequenceFile) {
        opts.embedded.push_back({{'X', 'S'}, _sequenceFile->GetFullPath(), ""});
    }

    return FSEQFileIO::Write(path, _seqData, &_sequenceElements, this, opts);
}

const std::string& HeadlessRenderContext::GetHeaderInfo(HEADER_INFO_TYPES type) const {
    static const std::string empty;
    return _sequenceFile ? _sequenceFile->GetHeaderInfo(type) : empty;
}
