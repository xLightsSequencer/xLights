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

#include "render/xLightsShowContext.h"
#include "render/SequenceFile.h"
#include "render/ViewpointMgr.h"

#include <memory>
#include <optional>
#include <string>
#include <list>

// Minimal windowless render driver. A concrete xLightsShowContext that loads a
// show folder + sequence and renders to an .fseq with no wxFrame and no window
// shown. It inherits the whole show model from the base (models, outputs,
// effects, sequence elements, render engine/pool/cache, directories, output
// buffer) and only adds the currently-open sequence file — proof that the base
// carries everything a render needs.
//
// This mirrors the iPad's iPadRenderContext, which does the same thing on iOS.
// It is wx-free (links into src-core); the desktop app sets up the GL context
// for shader effects before driving it. See AGENTS.md.
class HeadlessRenderContext : public xLightsShowContext {
public:
    HeadlessRenderContext() = default;
    ~HeadlessRenderContext() override;

    // Load networks + models/groups/view-objects from a show folder. Returns
    // false if the folder has no readable xlights_rgbeffects.xml.
    bool LoadShowFolder(const std::string& showDir,
                        const std::list<std::string>& mediaFolders = {});

    // Open a .xsq into the (inherited) sequence elements and size the output
    // buffer. Returns false on parse/load failure.
    bool OpenSequence(const std::string& xsqPath);

    // Kick off a full render and block until it finishes (or timeoutMs, 0 =
    // wait indefinitely). Returns true when the render completed.
    bool RenderAndWait(int timeoutMs = 0);

    // Write the rendered buffer to a sparse v2/zstd .fseq (matching the desktop
    // channel scope). Returns false if nothing is loaded/rendered or on I/O error.
    bool WriteFseq(const std::string& fseqPath);

    // ---- RenderContext pieces the base does not provide ----
    // (IsInShow*Folder, MakeRelativePath, MoveToShowFolder, IsSequenceLoaded,
    // GetCurrentMediaManager, AbortRender, CloseSequence live on the base.)
    const std::string& GetHeaderInfo(HEADER_INFO_TYPES type) const override;

    Model* GetModel(const std::string& name) const override { return AllModels[name]; }

    // 3D render aspect for "Per Preview" effects; the camera comes from the
    // base's GetNamedCamera3D (viewpoint_mgr, loaded from the show's <Viewpoints>).
    void GetRenderPreviewSize(int& w, int& h) const override { w = _previewWidth; h = _previewHeight; }

    void RenderEffectForModel(const std::string& model, int startms, int endms, bool clear) override;
    TimingElement* AddTimingElement(const std::string& /*name*/,
                                    const std::string& /*subType*/ = "") override { return nullptr; }
    void SuspendAutoSave(bool) override {}

private:
    void EnsureRenderEngine();

    int _previewWidth = 1280;
    int _previewHeight = 720;
};
