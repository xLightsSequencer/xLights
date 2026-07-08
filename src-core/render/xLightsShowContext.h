#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "render/RenderContext.h"
#include "outputs/OutputManager.h"
#include "models/OutputModelManager.h"
#include "models/ModelManager.h"
#include "models/ViewObjectManager.h"
#include "utils/JobPool.h"
#include "render/RenderCache.h"
#include "render/RenderEngine.h"
#include "render/SequenceElements.h"
#include "render/SequenceViewManager.h"
#include "render/SequenceData.h"
#include "render/SequenceFile.h"
#include "render/ViewpointMgr.h"
#include "effects/EffectManager.h"
#include "effects/EffectPresetManager.h"

#include <memory>
#include <optional>

// Concrete, wx-free owner of a loaded xLights "show": the model / output /
// effect managers, sequence elements, and render engine that both the desktop
// frame (xLightsFrame) and non-wx hosts (the iPad app, headless render/check/
// upload tools) need.
//
// xLightsFrame and iPadRenderContext historically each declared their own copy
// of this state.  Members are being pulled up out of those classes into this
// shared base one cluster at a time; every intermediate state must still build
// and behave identically.  It derives from RenderContext so render/, effects/,
// and models/ code talks to this rather than to any UI frame.
//
// This class MUST stay wx-free (it links into src-core and the iPad app) — no
// wx/ headers, no wxConfig access.  UI concerns stay in the derived host via
// the RenderContext virtuals (which default to headless-safe no-ops) and the
// UICallbacks pointer returned by GetUICallbacks().
class xLightsShowContext : public RenderContext {
public:
    xLightsShowContext() :
        AllModels(&_outputManager, static_cast<RenderContext*>(this)),
        AllObjects(static_cast<RenderContext*>(this)),
        jobPool("RenderPool"),
        _sequenceElements(static_cast<RenderContext*>(this)) {}
    ~xLightsShowContext() override = default;

    // The render engine (and other managers) hold a back-reference to this
    // context, so it must never be copied or moved.
    xLightsShowContext(const xLightsShowContext&) = delete;
    xLightsShowContext& operator=(const xLightsShowContext&) = delete;
    xLightsShowContext(xLightsShowContext&&) = delete;
    xLightsShowContext& operator=(xLightsShowContext&&) = delete;

    // ---- managers (pulled up out of xLightsFrame / iPadRenderContext) ----
    // Declaration order matters: AllModels/AllObjects take &_outputManager and
    // the RenderContext* (this) in their constructors.
    OutputManager _outputManager;
    OutputModelManager _outputModelManager;
    ModelManager AllModels;
    ViewObjectManager AllObjects;

    // Render engine and its inputs. The engine holds a RenderContext& back to
    // this object; the owning host constructs it (and wires any UI status
    // callbacks) once fully built, so it starts null here.
    JobPool jobPool;
    RenderCache _renderCache;
    std::unique_ptr<RenderEngine> _renderEngine;

    // Show / media / fseq directory state (relative-path resolution roots).
    std::string _permanentShowFolder;
    std::string mediaFilename;
    std::string showDirectory;
    std::list<std::string> mediaDirectories;
    std::string fseqDirectory;
    std::string renderCacheDirectory;
    std::string _backupDirectory;

    // Bumped on any model add/replace/delete/clear; folded into the render
    // tree's change-count gate.
    unsigned int modelsChangeCount = 0;

    // Sequence elements/views/effect presets. _sequenceElements takes the
    // RenderContext* (this); its ctor only stores the pointer.
    SequenceElements _sequenceElements;
    SequenceViewManager _sequenceViewManager;
    EffectPresetManager _effectPresetManager;

    // Effect registry. Default-constructed with an empty metadata dir, which
    // makes EffectManager self-resolve it from FileUtils::GetResourcesDir() —
    // so every host (desktop, iPad, headless) gets the same effectmetadata.
    EffectManager effectManager;

    // The render engine's output buffer (frames × channels).
    SequenceData _seqData;

    // 3D viewpoints/cameras (loaded from the show's <Viewpoints>), used by
    // "Per Preview" 3D effect rendering via GetNamedCamera3D.
    ViewpointMgr viewpoint_mgr;

    OutputManager* GetOutputManager() { return &_outputManager; }
    PreviewCamera* GetNamedCamera3D(const std::string& name) override { return viewpoint_mgr.GetNamedCamera3D(name); }
    OutputModelManager* GetOutputModelManager() override { return &_outputModelManager; }
    unsigned int GetModelGeneration() const override { return AllModels.GetModelGeneration(); }
    SequenceElements& GetSequenceElements() override { return _sequenceElements; }
    EffectManager& GetEffectManager() override { return effectManager; }
    SequenceData& GetSeqData() { return _seqData; }
    const SequenceData& GetSeqData() const { return _seqData; }

    const std::string& GetShowDirectory() const override { return showDirectory; }
    const std::string& GetFseqDirectory() const override { return fseqDirectory; }
    [[nodiscard]] const std::list<std::string>& GetMediaFolders() const override { return mediaDirectories; }

    // Result of ScanForMissingModels: models a sequence references that are not
    // in the current layout, plus the layout models NOT used by the sequence
    // (the candidate pool a host's remap UI offers).
    struct MissingModelScan {
        std::vector<std::string> missing;         // referenced (with effects), not in layout — annotated "name(effectCount)"
        std::vector<std::string> availableNames;  // layout model + group names not used by the sequence
        std::vector<std::string> availableModels; // as availableNames, excluding model groups
    };

    // Walk the sequence's model elements against AllModels (alias-aware) and
    // report the mismatch. wx-free; hosts drive resolution off the result.
    MissingModelScan ScanForMissingModels() const;

    // The shared "load the sequence's elements from an open SequenceFile" steps,
    // in the one correct order. Every host (desktop frame, iPad, headless) ran
    // these open-coded and each ordering/migration bug (frequency rounding,
    // AdjustEffectSettingsForVersion, view prep) had to be fixed in all three.
    // CheckForValidModels() runs at the migration seam (it may add/rename
    // elements, so it must precede PrepareViews). Returns false if
    // LoadSequencerFile fails. A host adds its own UI via the two virtuals.
    bool LoadSequenceElements(SequenceFile& file, pugi::xml_document& doc);

    // Resolve models the sequence references that aren't in the layout. The base
    // (headless) just logs them; the desktop overrides with the interactive
    // remap dialog, the iPad with its own mapping UI. Called from
    // LoadSequenceElements after migration, before PrepareViews.
    virtual void CheckForValidModels();

    // Host hook after the rows are populated (before the sequence is marked
    // loaded). Default no-op; the iPad uses it for animated-GIF media compat.
    virtual void OnSequenceElementsLoaded(SequenceFile& /*file*/) {}

    // ---- Currently-open sequence + its shared lifecycle (non-wx hosts) ----
    // The desktop frame keeps its own CurrentSeqXmlFile and overrides the
    // accessors below; these back the headless / iPad hosts, which historically
    // carried identical copies of all of this.
    std::unique_ptr<SequenceFile> _sequenceFile;
    std::optional<pugi::xml_document> _sequenceDoc;

    // Path helpers — pure string logic against the show/media dirs.
    bool IsInShowFolder(const std::string& file) const override;
    bool IsInShowOrMediaFolder(const std::string& file) const override;
    std::string MakeRelativePath(const std::string& file) const override;
    // No-op by default (headless can't move assets into the show); the iPad and
    // desktop override with real copies.
    std::string MoveToShowFolder(const std::string& /*file*/,
                                 const std::string& /*subdirectory*/,
                                 bool /*reuse*/) override { return ""; }

    bool IsSequenceLoaded() const override { return _sequenceFile && _sequenceFile->IsOpen(); }
    AudioManager* GetCurrentMediaManager() const override {
        return _sequenceFile ? _sequenceFile->GetMedia() : nullptr;
    }

    // Signal every in-flight render job to bail and block until they finish (or
    // maxTimeMs elapses). Once this returns, mutating _sequenceElements / _seqData
    // is safe. The desktop frame overrides with its own render-pool teardown.
    bool AbortRender(int maxTimeMs = 60000) override;

    // Tear down the open sequence: drain the render, clear elements + seq data,
    // drop the file/doc.
    void CloseSequence();

    // "Done" == no in-flight render jobs (must NOT key on _seqData validity —
    // CloseSequence clears it before the next open). Drains finished progress
    // entries; safe only from the driver/main thread.
    bool IsRenderDone();

    // (Re)allocate _seqData when the sequence shape (frames/channels/frameTime)
    // changes, aborting any in-flight render first to avoid a use-after-free.
    void EnsureSequenceDataSized();
};
