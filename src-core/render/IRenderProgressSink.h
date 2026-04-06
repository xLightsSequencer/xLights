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

#include "IRenderJobStatus.h"

// Abstract progress sink used by Render() to report per-job and overall
// render progress without depending on wx widgets.
// WxRenderProgressSink in RenderUI.cpp provides the wx/desktop implementation.
// A headless no-op implementation can be used for CLI/iPad rendering.
class IRenderProgressSink {
public:
    virtual ~IRenderProgressSink() = default;

    // Called once per job with effects, right after job construction.
    // Implementations call job->SetProgressCallback(...) to hook up UI gauges.
    virtual void SetupJobProgress(IRenderJobStatus* /*job*/) {}

    // Called after all jobs are submitted to the pool.
    // Desktop implementation uses this to finalize dialog layout.
    virtual void OnRenderSetupComplete() {}

    // Show the underlying dialog (e.g. from OnProgressBarDoubleClick).
    virtual void Show() {}
    virtual bool IsShown() const { return false; }
};
