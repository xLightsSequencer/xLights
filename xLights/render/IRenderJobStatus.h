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

// Minimum interface for querying / controlling an in-flight render job.
// RenderJob implements this so that RenderProgressInfo (and any UI code that
// iterates the job list) does not need the full RenderJob class definition.
class IRenderJobStatus {
public:
    virtual ~IRenderJobStatus() = default;

    // Control
    virtual void AbortRender() = 0;

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
