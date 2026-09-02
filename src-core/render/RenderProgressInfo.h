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

#include <atomic>
#include <chrono>
#include <climits>
#include <functional>
#include <list>

#include "IRenderJobStatus.h"

class AggregatorRenderer;
class IRenderProgressSink;
class Model;

// Sentinel frame value used to indicate "rendering complete".
#define END_OF_RENDER_FRAME INT_MAX

// Tracks one in-flight Render() call: its jobs, aggregators, progress sink,
// and completion callback.  Owned by xLightsFrame::renderProgressInfo.
class RenderProgressInfo {
public:
    explicit RenderProgressInfo(std::function<void(bool)>&& cb)
        : callback(std::move(cb)),
          numRows(0), startFrame(0), endFrame(0),
          jobs(nullptr), aggregators(nullptr), progressSink(nullptr),
          jobsRemaining(0), completed(false)
    {}

    // Deletes all jobs, aggregators, and the progress sink.
    // Defined in Render.cpp where AggregatorRenderer is fully declared.
    void CleanupJobs();

    std::function<void(bool)> callback;
    // Atomic and written LAST when the batch is populated. Every walker of the
    // progress list bounds its loop by this, so publishing it after jobs and
    // aggregators are in place is what lets a batch be registered before it is
    // built: a reader sees either no rows, and skips it, or a batch that is
    // wholly there. Setting it earlier would expose a half-built jobs array.
    std::atomic<int> numRows;
    int startFrame;
    int endFrame;
    IRenderJobStatus** jobs;     // owned array; each entry deleted by UpdateRenderStatus
    AggregatorRenderer** aggregators; // owned array
    IRenderProgressSink* progressSink; // owned; deleted when render group completes
    std::list<Model*> restriction;

    // Completion tracking. jobsRemaining is decremented by each RenderJob as it
    // reaches its Done state (normal, aborted, or early-bail paths).  When it
    // hits zero, the last thread CAS-flips completed and fires callback.
    // Platforms without a UI drain loop (iPad) poll `completed` to know when the
    // render batch has finished.
    std::atomic<int> jobsRemaining;
    std::atomic<bool> completed;

    // Set by SignalAbort on every registered batch. A batch whose setup has not
    // run yet owns no jobs, so there is nothing there for SignalAbort to reach;
    // this is how the abort gets to it. The setup checks it and completes the
    // batch instead of building jobs - otherwise AbortRender would sit waiting
    // for a render it had already asked to cancel to be built and run in full.
    std::atomic<bool> abortRequested{ false };

    // Stall watchdog state (see RenderEngine::CheckForStalledRender).
    long long lastProgressSum = -1;
    std::chrono::steady_clock::time_point lastProgressTime = std::chrono::steady_clock::now();

    // Frame progress alone, independent of whether the jobs look idle.  The
    // rescue above only fires when every unfinished job is idle, so a batch
    // wedged with a job still holding a thread resets that timer forever and is
    // invisible.  This pair is what notices it and reports the breakdown.
    long long lastFrameSum = -1;
    std::chrono::steady_clock::time_point lastFrameTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastStallReport = std::chrono::steady_clock::now();

    // Scheduler telemetry, logged to the render log when the batch completes.
    int totalJobs = 0;
    std::atomic<int> suspendCount{0};
    std::atomic<int> parkCount{0};
    std::atomic<long long> suspendedNs{0}; // Σ time jobs sat suspended on upstream
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
};
