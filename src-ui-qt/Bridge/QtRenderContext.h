#pragma once
#include "../../src-core/render/RenderContext.h"
#include "../../src-core/effects/EffectManager.h"
#include "../../src-core/outputs/OutputManager.h"
#include "../../src-core/models/OutputModelManager.h"
#include "../../src-core/render/SequenceElements.h"
#include <memory>

#include <list>
#include <string>

// Minimal RenderContext for the Qt UI.  Owns a real EffectManager (with all
// 55 effects registered) so RenderableEffect::Render() works correctly.
// Everything else is stubbed — no wx, no ModelManager, no OutputManager.
class QtRenderContext : public RenderContext {
public:
    explicit QtRenderContext(const std::string& metadataDir = {});
    ~QtRenderContext() override = default;

    EffectManager& GetEffectManager() override { return _effectManager; }

    // ── Pure-virtual stubs ────────────────────────────────────────────────
    const std::string& GetShowDirectory()   const override { return _empty; }
    const std::string& GetFseqDirectory()   const override { return _empty; }
    const std::list<std::string>& GetMediaFolders() const override { return _mediaFolders; }
    bool IsInShowFolder(const std::string&)          const override { return false; }
    bool IsInShowOrMediaFolder(const std::string&)   const override { return false; }
    std::string MoveToShowFolder(const std::string& f, const std::string&, bool = false) override { return f; }
    std::string MakeRelativePath(const std::string& f) const override { return f; }
    SequenceElements& GetSequenceElements() override { return *_seqElements; }
    AudioManager* GetCurrentMediaManager() const override { return nullptr; }
    const std::string& GetHeaderInfo(HEADER_INFO_TYPES) const override { return _empty; }
    Model* GetModel(const std::string&)     const override { return nullptr; }
    OutputModelManager* GetOutputModelManager() override { return &_outputModelManager; }
    bool AbortRender(int = 60000)           override { return false; }
    void RenderEffectForModel(const std::string&, int, int, bool = false) override {}
    TimingElement* AddTimingElement(const std::string&, const std::string& = "") override { return nullptr; }
    void SuspendAutoSave(bool)              override {}

    OutputManager& outputManager() { return _outputManager; }

private:
    OutputManager                       _outputManager;      // empty stub — no real hardware
    OutputModelManager                  _outputModelManager; // no-op: _disableASAPWork=true by default
    EffectManager                       _effectManager;
    std::unique_ptr<SequenceElements>   _seqElements;
    std::list<std::string>              _mediaFolders;
    mutable std::string                 _empty;
};
