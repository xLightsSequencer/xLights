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

#include <wx/timer.h>

#include <atomic>
#include <string>

class xLightsTimerDataImpl;

class xLightsTimer :
    public wxTimer
{
protected:
    std::string _name;
    xLightsTimerDataImpl* data = nullptr;
    std::atomic<bool> _running{ false };
    std::atomic<bool> _pending{ false };

public:
    xLightsTimer();
    virtual ~xLightsTimer();
    virtual void Stop() override;
    void SetName(const std::string& name);
    bool Start(int time, bool oneShot, const std::string& name);
    virtual bool Start(int time = -1, bool oneShot = wxTIMER_CONTINUOUS) override { return Start(time, oneShot, ""); }
    virtual void Notify() override;
    int GetInterval() const;

    // wxTimer::IsRunning() only tracks timers started through wxTimer itself, and
    // the Windows and macOS backends drive their own timer instead.
    bool IsRunning() const { return _running; }

    // Delivers a tick that arrived on a worker thread. Public only because it is
    // the target of a CallAfter.
    void DoSendTimer();

#ifdef __WXOSX__
    double presentTimeForScreen(int i) const;
#endif
};
