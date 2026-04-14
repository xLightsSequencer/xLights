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
#include "render/RenderEngine.h"
#include "render/RenderCache.h"
#include "render/IRenderProgressSink.h"
#include "effects/EffectManager.h"
#include "outputs/OutputManager.h"
#include "models/ModelManager.h"
#include "models/OutputModelManager.h"
#include "models/ViewObjectManager.h"
#include "utils/JobPool.h"

#include <list>
#include <memory>
#include <optional>
#include <string>

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
    std::string MoveToShowFolder(const std::string&, const std::string&, bool) override { return ""; }
    std::string MakeRelativePath(const std::string& file) const override;

    SequenceElements& GetSequenceElements() override { return _sequenceElements; }
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

    // Accessors
    OutputManager& GetOutputManager() { return _outputManager; }
    ModelManager& GetModelManager() { return *_modelManager; }
    ViewObjectManager& GetAllObjects() { return *_viewObjectManager; }
    SequenceFile* GetSequenceFile() { return _sequenceFile.get(); }

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
    std::unique_ptr<SequenceFile> _sequenceFile;
    std::optional<pugi::xml_document> _sequenceDoc;

    // Rendering
    SequenceData _sequenceData;
    std::unique_ptr<JobPool> _jobPool;
    RenderCache _renderCache;
    std::unique_ptr<RenderEngine> _renderEngine;
    unsigned int _modelsChangeCount = 0;
};
