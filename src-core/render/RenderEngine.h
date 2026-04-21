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

#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

class Effect;
class IRenderProgressSink;
class PixelBufferClass;
class JobPool;
class Model;
class RenderCache;
class RenderContext;
class RenderEvent;
class RenderProgressInfo;
class RenderTreeData;
class SequenceData;
class SequenceElements;
class SettingsMap;

// Platform-neutral render orchestration engine.
// Owns the render tree, job tracking, and main-thread effect queue.
// xLightsFrame creates one of these and delegates all render work to it.
class RenderEngine {
public:
    RenderEngine(RenderContext& ctx, JobPool& pool, RenderCache& cache);
    ~RenderEngine();

    // ---- render tree ----
    void BuildRenderTree(SequenceElements& elements, unsigned int modelsChangeCount);

    // ---- rendering ----
    void Render(SequenceElements& seqElements, SequenceData& seqData,
                const std::list<Model*> models,
                const std::list<Model*>& restrictToModels,
                int startFrame, int endFrame,
                std::unique_ptr<IRenderProgressSink> sink, bool clear,
                std::function<void(bool)>&& callback);

    void RenderDirtyModels(SequenceElements& elements, SequenceData& seqData,
                           bool suspendRender, unsigned int modelsChangeCount);

    void RenderEffectForModel(const std::string& model, int startms, int endms,
                              SequenceElements& elements, SequenceData& seqData,
                              bool suspendRender, unsigned int modelsChangeCount,
                              bool clear = false);

    // Extract a single model's channel data from rendered sequence data into a
    // new SequenceData buffer.  Returns {exportData, chansPerNode} or {nullptr,0} on failure.
    struct ExportedModelData {
        std::unique_ptr<SequenceData> data;
        int chansPerNode = 0;
    };
    ExportedModelData ExportModelData(const std::string& modelName, SequenceData& sourceData);

    void SignalAbort();
    bool IsRenderDone() const { return _renderProgressInfo.empty(); }

    void RenderMainThreadEffects();

    // ---- render job support ----
    bool RenderEffectFromMap(bool suppress, Effect* effect, int layer,
                             int period, SettingsMap& settings, PixelBufferClass& buffer,
                             bool& resetEffectState, bool bgThread, RenderEvent* event);
    void OnRenderJobComplete(const std::string& modelName);
    void OnAllRenderJobsComplete();

    // Called by each RenderJob's RAII guard as it exits Process() (any path:
    // normal, aborted, early-bail). The thread that decrements the last job
    // fires the render-batch completion callback.
    void NotifyJobFinished(RenderProgressInfo* rpi);

    // ---- state access (for UI layer) ----
    std::list<RenderProgressInfo*>& GetRenderProgressInfo() { return _renderProgressInfo; }
    int GetAbortedRenderJobs() const { return _abortedRenderJobs; }
    void ResetAbortedRenderJobs() { _abortedRenderJobs = 0; }

    // ---- render tree (public for UI layer to query model list) ----
    struct RenderTree {
        RenderTree() = default;
        ~RenderTree() { Clear(); }
        void Clear();
        void Add(Model* el);
        void Print();
        std::list<Model*> GetModels() const;

        unsigned int renderTreeChangeCount = 0;
        std::list<RenderTreeData*> data;
    };
    RenderTree& GetRenderTree() { return _renderTree; }

    // ---- callbacks set by UI layer ----
    void SetOnRenderStatusTimerStart(std::function<void()> fn) { _onRenderStatusTimerStart = std::move(fn); }
    void SetOnCallAfterRenderMainThread(std::function<void()> fn) { _onCallAfterRenderMainThread = std::move(fn); }
    void SetOnRenderJobComplete(std::function<void(const std::string&)> fn) { _onRenderJobComplete = std::move(fn); }
    void SetOnAllRenderJobsComplete(std::function<void()> fn) { _onAllRenderJobsComplete = std::move(fn); }

private:
    void RenderEffectOnMainThread(RenderEvent* ev);

    RenderContext& _ctx;
    JobPool& _jobPool;
    RenderCache& _renderCache;

    RenderTree _renderTree;
    std::list<RenderProgressInfo*> _renderProgressInfo;
    std::queue<RenderEvent*> _mainThreadRenderEvents;
    std::mutex _renderEventLock;
    int _abortedRenderJobs = 0;

    std::function<void()> _onRenderStatusTimerStart;
    std::function<void()> _onCallAfterRenderMainThread;
    std::function<void(const std::string&)> _onRenderJobComplete;
    std::function<void()> _onAllRenderJobsComplete;
};
