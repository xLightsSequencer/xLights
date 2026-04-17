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
    int numRows;
    int startFrame;
    int endFrame;
    IRenderJobStatus** jobs;     // owned array; each entry deleted by UpdateRenderStatus
    AggregatorRenderer** aggregators; // owned array
    IRenderProgressSink* progressSink; // owned; deleted when render group completes
    std::list<Model*> restriction;

    // Completion tracking. jobsRemaining is decremented by each RenderJob as it
    // exits (via RAII guard in Process) — normal, aborted, or early-bail paths.
    // When it hits zero, the last thread CAS-flips completed and fires callback.
    // Platforms without a UI drain loop (iPad) poll `completed` to know when the
    // render batch has finished.
    std::atomic<int> jobsRemaining;
    std::atomic<bool> completed;
};
