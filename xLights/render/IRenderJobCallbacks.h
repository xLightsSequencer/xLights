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

#include <string>

class Effect;
class PixelBufferClass;
class RenderEvent;
class SettingsMap;

// Abstract interface that decouples RenderJob from xLightsFrame.
// xLightsFrame implements this; a headless / platform-neutral adapter
// can provide its own lightweight implementation (e.g., for an iPad app).
class IRenderJobCallbacks {
public:
    virtual ~IRenderJobCallbacks() = default;

    // Render a single effect. Called from both main and background threads.
    virtual bool RenderEffectFromMap(bool suppress, Effect* effect, int layer,
        int period, SettingsMap& settings, PixelBufferClass& buffer,
        bool& resetEffectState, bool bgThread, RenderEvent* event) = 0;

    // Called from a render thread when this (chained) job has finished rendering.
    // Replaces CallAfter(&xLightsFrame::SetStatusText, ...).
    virtual void OnRenderJobComplete(const std::string& modelName) = 0;

    // Called from a render thread when the final (last) job in a render group
    // finishes. Replaces CallAfter(&xLightsFrame::RenderDone).
    virtual void OnAllRenderJobsComplete() = 0;
};
