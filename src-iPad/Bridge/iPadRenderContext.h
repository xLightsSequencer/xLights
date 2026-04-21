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

// iPadRenderContext — RenderContext for loading, rendering, and displaying
// sequences on iPad.  Includes RenderEngine for effect rendering.

#include "render/RenderContext.h"
#include "render/SequenceData.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "render/SequenceViewManager.h"
#include "render/RenderEngine.h"
#include "render/RenderCache.h"
#include "render/IRenderProgressSink.h"
#include "render/ViewpointMgr.h"
#include "effects/EffectManager.h"
#include "outputs/OutputManager.h"
#include "models/ModelManager.h"
#include "models/OutputModelManager.h"
#include "models/ViewObjectManager.h"
#include "utils/JobPool.h"
#include "utils/xlImage.h"

#include <list>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class iPadRenderContext : public RenderContext {
public:
    iPadRenderContext();
    ~iPadRenderContext() override;

    // Show folder management
    bool LoadShowFolder(const std::string& showDir);
    bool LoadShowFolder(const std::string& showDir,
                        const std::list<std::string>& mediaFolders);
    const std::string& GetShowDirectory() const override { return _showDir; }

    // Sequence management
    bool OpenSequence(const std::string& path);
    void CloseSequence();

    // RenderContext implementation
    const std::string& GetFseqDirectory() const override { return _showDir; }
    const std::list<std::string>& GetMediaFolders() const override { return _mediaFolders; }
    bool IsInShowFolder(const std::string& file) const override;
    bool IsInShowOrMediaFolder(const std::string& file) const override;
    // Copy `file` into `<showDir>/<subdirectory>`, returning the final
    // absolute path. Appends `_N` on name collision unless `reuse` and
    // the existing file's contents already match. Empty string on
    // failure (no show folder configured, copy error).
    std::string MoveToShowFolder(const std::string& file,
                                  const std::string& subdirectory,
                                  bool reuse) override;

    // Same as MoveToShowFolder but the destination root is one of the
    // configured media folders (`mediaFolderPath` must appear in
    // `_mediaFolders` or we refuse). Used by the iPad fileImporter
    // "destination: Media Folder X" branch.
    std::string CopyToMediaFolder(const std::string& file,
                                   const std::string& mediaFolderPath,
                                   const std::string& subdirectory);
    std::string MakeRelativePath(const std::string& file) const override;

    SequenceElements& GetSequenceElements() override { return _sequenceElements; }
    SequenceViewManager& GetSequenceViewManager() { return _viewsManager; }
    bool IsSequenceLoaded() const override { return _sequenceFile && _sequenceFile->IsOpen(); }
    AudioManager* GetCurrentMediaManager() const override;
    const std::string& GetHeaderInfo(HEADER_INFO_TYPES type) const override;

    Model* GetModel(const std::string& name) const override;
    EffectManager& GetEffectManager() override { return _effectManager; }
    OutputModelManager* GetOutputModelManager() override { return &_outputModelManager; }

    bool AbortRender(int maxTimeMs = 60000) override;
    void RenderEffectForModel(const std::string& model, int startms, int endms, bool clear) override;
    TimingElement* AddTimingElement(const std::string& name,
                                    const std::string& subType = "") override;
    void SuspendAutoSave(bool) override {}
    bool IsLowDefinitionRender() const override { return true; }

    // Rendering
    void RenderAll();
    void SetModelColors(int frameMS);
    SequenceData& GetSequenceData() { return _sequenceData; }
    bool IsRenderDone();

    // === Preset model / preview rendering =================================
    // Mirrors xLightsFrame's standalone preset-render scaffolding: a 64×64
    // RGB `MatrixModel` owned by its own `ModelManager`, plus a dedicated
    // `SequenceElements` / `SequenceData` pair. Used to render an effect
    // in isolation for media-picker thumbnails (shader previews, future
    // preset GIFs, etc.) without touching the user's real sequence.
    void EnsurePresetModel();
    Model* GetPresetModel() { EnsurePresetModel(); return _presetModel; }
    SequenceElements& GetPresetSequenceElements() { return _presetSequenceElements; }
    SequenceData& GetPresetSequenceData() { return _presetSequenceData; }

    // Render an effect sitting in `seqElements` on `matrixModel` for
    // `numFrames` frames at `frameTimeMs`. Returns the per-frame RGBA
    // rasterisation as `xlImage`s (same layout `ShaderPreviewGenerator`
    // stores via `MediaCacheEntry::SetPreviewFrames`). Synchronously
    // blocks the calling thread until the render completes (iPad path:
    // call from a utility queue, not the main thread). Ports
    // `xLightsFrame::RenderEffectToFrames` from
    // `src-ui-wx/app-shell/TabConvert.cpp:856`.
    std::vector<std::shared_ptr<xlImage>> RenderEffectToFrames(
        Model* matrixModel,
        SequenceData& seqData,
        SequenceElements& seqElements,
        size_t numFrames,
        int frameTimeMs);

    // Fill a `ShaderMediaCacheEntry`'s preview-frame strip by rendering
    // the shader on the preset matrix model at default parameter
    // values. No-op if the entry already has a preview cached. Ports
    // `GenerateShaderPreview` from
    // `src-ui-wx/media/ShaderPreviewGenerator.cpp`. Synchronous; call
    // from a utility-priority thread.
    void GenerateShaderPreview(class ShaderMediaCacheEntry* entry);

    // Memory-pressure response. Called from Swift when the system signals
    // memory warning / critical. Aborts any in-flight render and purges the
    // render cache so we don't hold onto frame buffers we no longer need.
    void HandleMemoryWarning();
    void HandleMemoryCritical();

    // Accessors
    OutputManager& GetOutputManager() { return _outputManager; }
    ModelManager& GetModelManager() { return *_modelManager; }
    ViewObjectManager& GetAllObjects() { return *_viewObjectManager; }
    SequenceFile* GetSequenceFile() { return _sequenceFile.get(); }
    // Virtual preview canvas size from <settings><previewWidth/Height>
    // in xlights_rgbeffects.xml, defaulted to desktop's 1280×720 when
    // absent. Consumed by iPadModelPreview in House Preview mode so the
    // 2D ortho projection maps world coords onto pixel coords the same
    // way desktop does.
    int GetPreviewWidth() const { return _previewWidth; }
    int GetPreviewHeight() const { return _previewHeight; }

    // <settings><Display2DCenter0 value="1"/>. When set, desktop places
    // world X=0 at the horizontal centre of the preview (shows with
    // models laid out around a centered origin, e.g. -600..+600 rather
    // than 0..1200). Ignoring this flag was the cause of the House
    // Preview rendering blank for center-origin shows.
    bool GetDisplay2DCenter0() const { return _display2DCenter0; }

    // <settings><LayoutMode3D value="1"/>. Desktop's last-used 3D vs 2D
    // preference for the House Preview, read at show-folder load.
    // iPad uses this as the initial value for House Preview's is3D
    // toggle (the user can still flip it per-session via the overlay
    // picker; we don't write back since layout editing is desktop-only).
    bool GetLayoutMode3D() const { return _layoutMode3D; }

    // House Preview background image + brightness/alpha/scale — values
    // come from `<settings>` in xlights_rgbeffects.xml and are read-only
    // on iPad (editing lives in the desktop Layout panel, out of iPad
    // scope). Path is FixFile-resolved against the show directory; empty
    // string means "no background".
    //
    // These are the DEFAULT-group values. Use GetActiveBackground*()
    // below to pick between these and the named-group overrides when a
    // non-Default layout group is active.
    const std::string& GetBackgroundImage() const { return _backgroundImage; }
    int GetBackgroundBrightness() const { return _backgroundBrightness; }
    int GetBackgroundAlpha() const { return _backgroundAlpha; }
    bool GetScaleBackgroundImage() const { return _scaleBackgroundImage; }

    // Named layout groups from <layoutGroups> in rgbeffects.xml. Each
    // carries its own background settings and scopes the House Preview
    // to a filtered set of models (those whose `layout_group` matches
    // the group name, plus any marked "All Previews"). The "Default"
    // group is implicit (driven by the top-level <settings> values
    // above) — it is NOT listed in GetNamedLayoutGroups().
    struct NamedLayoutGroup {
        std::string name;
        std::string backgroundImage;
        int backgroundBrightness = 100;
        int backgroundAlpha = 100;
        bool scaleBackgroundImage = false;
    };
    const std::vector<NamedLayoutGroup>& GetNamedLayoutGroups() const { return _namedLayoutGroups; }

    // Active House-Preview layout group. "Default" means the implicit
    // default preview (models with layout_group == "Default" or
    // "All Previews"); other values must match a named group from
    // GetNamedLayoutGroups(). Unknown names fall back to "Default".
    const std::string& GetActiveLayoutGroup() const { return _activeLayoutGroup; }
    void SetActiveLayoutGroup(const std::string& name);

    // Active-group view of the background settings. Picks the correct
    // source (Default vs. a named group) so callers don't need to
    // switch on GetActiveLayoutGroup() themselves.
    const std::string& GetActiveBackgroundImage() const;
    int GetActiveBackgroundBrightness() const;
    int GetActiveBackgroundAlpha() const;
    bool GetActiveScaleBackgroundImage() const;

    // Expanded list of models to render for the active layout group.
    // Filters by layout_group and expands ModelGroup children exactly
    // like desktop UpdateModelsList (TabSequence.cpp:1209), minus
    // duplicate suppression. Call once per frame.
    std::vector<Model*> GetModelsForActivePreview() const;

    // Whether the active group draws view objects. Only "Default"
    // draws them (matching desktop — view objects are hard-coded to
    // layout_group "Default").
    bool ActivePreviewShowsViewObjects() const { return _activeLayoutGroup == "Default"; }

    // Saved camera views (viewpoints). Loaded from the `<Viewpoints>`
    // node in xlights_rgbeffects.xml; each camera is flagged 2D or 3D
    // and named. UI filters by the preview's current mode before
    // showing them.
    ViewpointMgr& GetViewpointMgr() { return _viewpointMgr; }
    const ViewpointMgr& GetViewpointMgr() const { return _viewpointMgr; }

    // Rewrite just the `<Viewpoints>` subtree of the on-disk
    // xlights_rgbeffects.xml so saved-as / delete survive app restart.
    // Preserves every other node (models, layoutGroups, settings, …)
    // by re-reading the file, replacing the Viewpoints child, and
    // writing back. Returns false if the file couldn't be read /
    // serialized. Heavy-ish (one disk round-trip per save), but
    // viewpoint edits are a rare user action.
    bool SaveViewpoints();

    // Model pixel data for a given frame — returns (x, y, r, g, b) tuples
    struct PixelData {
        float x, y;
        uint8_t r, g, b;
    };
    std::vector<PixelData> GetModelPixels(const std::string& modelName, int frameMS);
    std::vector<PixelData> GetAllModelPixels(int frameMS);

private:
    std::string _showDir;
    std::list<std::string> _mediaFolders;

    OutputManager _outputManager;
    OutputModelManager _outputModelManager;
    std::unique_ptr<ModelManager> _modelManager;
    std::unique_ptr<ViewObjectManager> _viewObjectManager;
    EffectManager _effectManager;
    SequenceElements _sequenceElements;
    SequenceViewManager _viewsManager;
    std::unique_ptr<SequenceFile> _sequenceFile;
    std::optional<pugi::xml_document> _sequenceDoc;

    // Rendering
    SequenceData _sequenceData;
    std::unique_ptr<JobPool> _jobPool;
    RenderCache _renderCache;
    std::unique_ptr<RenderEngine> _renderEngine;
    unsigned int _modelsChangeCount = 0;

    // Virtual preview canvas size — desktop defaults.
    int _previewWidth = 1280;
    int _previewHeight = 720;
    bool _display2DCenter0 = false;
    bool _layoutMode3D = true;

    std::string _backgroundImage;
    int _backgroundBrightness = 100;
    int _backgroundAlpha = 100;
    bool _scaleBackgroundImage = false;

    std::vector<NamedLayoutGroup> _namedLayoutGroups;
    std::string _activeLayoutGroup = "Default";

    ViewpointMgr _viewpointMgr;

    // Preset model scaffolding — lazily built on first preview render.
    Model* _presetModel = nullptr;
    std::unique_ptr<ModelManager> _presetModelManager;
    SequenceElements _presetSequenceElements{ this };
    SequenceData _presetSequenceData;

    // Ensures the render engine + its pool are ready before using them
    // from a preview render path (before `RenderAll` has been called).
    void EnsureRenderEngine();
};
