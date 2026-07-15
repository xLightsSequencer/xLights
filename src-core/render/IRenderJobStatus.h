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
#include <string>

struct RenderJobProfile;

// Minimum interface for querying / controlling an in-flight render job.
// RenderJob implements this so that RenderProgressInfo (and any UI code that
// iterates the job list) does not need the full RenderJob class definition.
class IRenderJobStatus {
public:
    virtual ~IRenderJobStatus() = default;

    // Control
    virtual void AbortRender() = 0;

    // Stall-watchdog hook: reschedule the job if it is suspended waiting for
    // an upstream frame (a lost wake-up would otherwise hang the batch).
    // No-op for jobs that are queued, running, or done.
    virtual void NudgeIfSuspended() {}

    // Stall-watchdog query: true when the job holds no thread (suspended or
    // parked).  A batch is only considered stalled when every unfinished job
    // is idle - a job actively rendering a slow frame is not a stall.
    virtual bool IsIdle() { return false; }

    // XL_RENDER_PROFILE telemetry.  Valid to read after the job has finished
    // (the batch keeps every job alive until completion is signaled).
    virtual const RenderJobProfile* GetRenderProfile() const { return nullptr; }

    // Progress query (atomic reads, callable from any thread)
    virtual int GetCurrentFrame() const = 0;
    virtual int GetEndFrame() const = 0;
    virtual int GetStartFrame() const = 0;

    // Identity / status strings
    virtual const std::string GetName() const = 0;
    virtual std::string GetStatus() const = 0;
    virtual std::string GetStatusForUser() = 0; // non-const: implementations may traverse mutable effect state
    virtual std::string GetStatusString() const = 0;

    // UI progress callback — set by IRenderProgressSink::SetupJobProgress.
    // Implementations should call cb when progress changes.
    virtual void SetProgressCallback(std::function<void(int, const std::string&)> cb) = 0;
    virtual void UpdateProgress(int value, const std::string& tooltip = {}) = 0;
};
