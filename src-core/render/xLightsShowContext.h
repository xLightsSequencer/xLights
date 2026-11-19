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
#include "render/RemoteRenderScheduler.h"
#include "render/SequenceElements.h"
#include "render/SequenceViewManager.h"
#include "render/SequenceData.h"
#include "render/SequenceFile.h"
#include "render/ViewpointMgr.h"
#include "utils/LayoutLock.h"
#include "utils/LayoutRenameHints.h"
#include "effects/EffectManager.h"
#include "effects/EffectPresetManager.h"

#include <functional>
#include <memory>
#include <mutex>
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
        _sequenceElements(static_cast<RenderContext*>(this)) {
        _outputManager.SetLayoutLock(&_layoutLock);
        AllModels.SetLayoutLock(&_layoutLock);
        AllObjects.SetLayoutLock(&_layoutLock);
        ApplyLayoutLockFromEnvironment();
    }
    ~xLightsShowContext() override;

    // The render engine (and other managers) hold a back-reference to this
    // context, so it must never be copied or moved.
    xLightsShowContext(const xLightsShowContext&) = delete;
    xLightsShowContext& operator=(const xLightsShowContext&) = delete;
    xLightsShowContext(xLightsShowContext&&) = delete;
    xLightsShowContext& operator=(xLightsShowContext&&) = delete;

    // ---- managers (pulled up out of xLightsFrame / iPadRenderContext) ----
    // Declaration order matters: AllModels/AllObjects take &_outputManager and
    // the RenderContext* (this) in their constructors.
private:
    // Before the managers: they hold a pointer to it until they are destroyed.
    LayoutLock _layoutLock;

public:
    OutputManager _outputManager;
    OutputModelManager _outputModelManager;
    ModelManager AllModels;
    ViewObjectManager AllObjects;

    // Render engine and its inputs. The engine holds a RenderContext& back to
    // this object; the owning host constructs it (and wires any UI status
    // callbacks) once fully built, so it starts null here.
    JobPool jobPool;
    JobPool* GetJobPool() override { return &jobPool; }
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

    // The rename hints kept in the layout file's <renames> section (see
    // LayoutRenameHints). Loaded with the layout; every host writes them back.
    LayoutRenameHints _renameHints;
    // Moves the renames the model, view object and controller managers queued
    // since the last call into _renameHints, then prunes it. Run when building
    // the layout file to write.
    void CollectRenameHints();

    // ---- Layout lock (collaboration session) ----
    // While set, every peer treats the layout (models, groups, submodels,
    // faces, states, view objects, views), controller setup and sequence
    // settings as read-only; sequence editing is untouched. The model, view
    // object and output managers refuse their deletes, renames and the
    // controller save themselves; hosts disable their editors, show
    // GetLayoutLockReason() and check RefusesLayoutWrite() before writing
    // xlights_rgbeffects.xml. Session start/end turns it on/off; for testing,
    // XL_COLLAB_LAYOUT_LOCK=1 in the environment sets it at construction.
    void SetCollaborationLock(bool locked, const std::string& reason = std::string());
    bool IsLayoutLocked() const override { return _layoutLock.IsLocked(); }
    // Empty when not locked.
    std::string GetLayoutLockReason() const { return _layoutLock.GetReason(); }
    // For a host's layout-file save: true (and logged) while locked. Autosave
    // copies and backups are not the layout file and need not ask.
    bool RefusesLayoutWrite(const char* what) const { return LayoutLock::Refuses(&_layoutLock, what); }
    // Runs on the thread that called SetCollaborationLock, after the change,
    // so the host can refresh its editors. Not called for the environment
    // switch (a host reads IsLayoutLocked() when it builds its UI).
    virtual void OnLayoutLockChanged() {}

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

    // Renames the rows of models the sequence refers to by a name the layout
    // no longer has, when the rename hints say (unambiguously) which model or
    // group that name became and the sequence has no row for it yet. Part of
    // opening the sequence: no undo step. Runs first in CheckForValidModels,
    // so only what it could not map reaches the missing-model prompt. Returns
    // (old name, new name) for each row it renamed; the last open's list
    // stays in _autoMappedModels for the host's notice.
    std::vector<std::pair<std::string, std::string>> MapRenamedModels();
    std::vector<std::pair<std::string, std::string>> _autoMappedModels;

    // For a host that saves xlights_rgbeffects.xml by patching the file on disk
    // (the iPad) rather than rebuilding it from memory (the desktop): writes the
    // in-memory layout id of every model, submodel, group, view object and view
    // onto its record in `root` (matched by name), and of every layout group
    // `layoutGroupIdForName` knows. Then any record still without an id - or
    // repeating one - gets a fresh one, which covers records the host does not
    // hold in memory at all. Run on every write so ids assigned at load reach
    // the file even for records the save did not otherwise touch.
    void StampLayoutObjectIds(pugi::xml_node root,
                              std::function<std::string(const std::string&)> const& layoutGroupIdForName = {}) const;

    // The shared "load the sequence's elements from an open SequenceFile" steps,
    // in the one correct order. Every host (desktop frame, iPad, headless) ran
    // these open-coded and each ordering/migration bug (frequency rounding,
    // AdjustEffectSettingsForVersion, view prep) had to be fixed in all three.
    // CheckForValidModels() runs at the migration seam (it may add/rename
    // elements, so it must precede PrepareViews). Returns false if
    // LoadSequencerFile fails. A host adds its own UI via the two virtuals.
    bool LoadSequenceElements(SequenceFile& file, pugi::xml_document& doc);

    // Resolve models the sequence references that aren't in the layout. The base
    // (headless, iPad) maps the ones the rename hints know, then logs the rest; the desktop overrides with the interactive
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
    // desktop override with real copies. Hands back the original path, as the
    // overrides do on failure: callers store the result without checking it.
    std::string MoveToShowFolder(const std::string& file,
                                 const std::string& /*subdirectory*/,
                                 bool /*reuse*/) override { return file; }

    bool IsSequenceLoaded() const override { return _sequenceFile && _sequenceFile->IsOpen(); }
    AudioManager* GetCurrentMediaManager() const override {
        return _sequenceFile ? _sequenceFile->GetMedia() : nullptr;
    }

    // Signal every in-flight render job to bail and block until they finish (or
    // maxTimeMs elapses). Once this returns, mutating _sequenceElements / _seqData
    // is safe. The desktop frame overrides with its own render-pool teardown.
    bool AbortRender(int maxTimeMs = 60000) override;

    // Tear down the open sequence: drain the render, clear elements + seq data,
    // drop the file/doc. Returns false when the render could not be drained, in
    // which case NOTHING was released - live workers still hold raw pointers
    // into the elements and seq data. A caller that goes on to open/replace the
    // sequence anyway corrupts memory under those workers, so the result must
    // be handled rather than assumed.
    [[nodiscard]] bool CloseSequence();

    // "Done" == no in-flight render jobs (must NOT key on _seqData validity —
    // CloseSequence clears it before the next open). Drains finished progress
    // entries; callable from more than one thread (the iPad polls it from the
    // UI while AbortRender drains it from the render thread).
    bool IsRenderDone() override;

    // (Re)allocate _seqData when the sequence shape (frames/channels/frameTime)
    // changes, aborting any in-flight render first to avoid a use-after-free.
    void EnsureSequenceDataSized();

    // The only way to (re)allocate _seqData. init() frees the frame blocks a live
    // render job holds raw pointers into, so this drains the render first; when
    // the abort times out it leaves _seqData untouched and returns false.
    bool ReinitSeqData(unsigned int numChannels, unsigned int numFrames, unsigned int frameTime);

    // The only safe way to read the render progress list from a thread that is
    // not the one driving the render. IsRenderDone() erases completed entries
    // and deletes them (and their RenderJobs) under _renderProgressDrainLock
    // from whichever thread requested the abort, so an unlocked walk on the UI
    // thread reads freed RenderProgressInfo/RenderJob memory. `fn` runs with
    // the lock held: it must not call back into IsRenderDone()/AbortRender().
    void ForEachRenderProgress(const std::function<void(RenderProgressInfo*)>& fn) const;

    // ---- Edits from other participants in a shared session ----
    // Re-render rows another participant changed (SequenceChange::rows /
    // startMS / endMS; -1 / -1 for the whole sequence). Rows in or near the
    // visible range, or in the range being played, render now; the rest wait
    // for one of the calls below (see RemoteRenderScheduler). Main thread.
    void MarkRemoteChange(const std::vector<SeqObjectId>& rows, int startMS, int endMS);
    // The sequencer's visible time range changed. Until a host calls this,
    // nothing waits.
    void SetVisibleTimeRange(int startMS, int endMS);
    // Before the first frame of playback (startMS / endMS < 0: everything),
    // and when it stops.
    void RemoteRendersPlaybackStarted(int startMS = -1, int endMS = -1);
    void RemoteRendersPlaybackStopped();
    // Call periodically (any interval up to a second or so) with a monotonic
    // time; renders waiting rows a few at a time once the app has been quiet
    // and the renderer is idle. True while rows are still waiting.
    bool RemoteRendersIdle(uint64_t nowMs);
    // Start rendering everything still waiting (before writing the fseq).
    void FlushRemoteRenders();
    RemoteRenderScheduler& GetRemoteRenderScheduler() { return _remoteRenders; }

private:
    void RenderRemoteRow(SeqObjectId row, int startMS, int endMS, RemoteRenderScheduler::Reason why);
    RemoteRenderScheduler _remoteRenders{ [this](RemoteRenderScheduler::Row row, int s, int e, RemoteRenderScheduler::Reason why) {
        RenderRemoteRow(row, s, e, why);
    } };

    void ApplyLayoutLockFromEnvironment();

    // Serializes the drain in IsRenderDone(). Two threads reach it: the host's
    // completion poll and AbortRender() on whichever thread asked for the
    // abort. Unsynchronized, both could pull the same RenderProgressInfo off
    // the list and delete it (and its RenderJobs) twice.
    mutable std::mutex _renderProgressDrainLock;

    // Entries pulled off the list but not yet cleaned up. Cleanup runs outside
    // the lock (callbacks can re-enter), so "done" has to account for them or
    // the other thread sees an empty list while jobs are still being torn down.
    int _renderProgressDraining = 0;
};
