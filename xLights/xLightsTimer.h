#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/timer.h>

#include <atomic>

class xlTimerThread;

// CLasses that want to have the timer call it from another thread need to derive from this class
class xLightsTimerCallback
{
public:
    virtual void TimerCallback(wxTimerEvent& event) = 0;
    virtual ~xLightsTimerCallback() {}
    xLightsTimerCallback() {}
};

class xLightsTimer :
    public wxTimer
{
    xlTimerThread* _t;
    std::atomic<bool> _pending;
    xLightsTimerCallback* _timerCallback;
    std::atomic<bool> _suspend;
    std::atomic<bool> _log;
    std::string _name;

public:
    xLightsTimer();
    virtual ~xLightsTimer();
    virtual void Stop() override;
    void SetName(const std::string& name);
    bool Start(int time, bool oneShot, const std::string& name);
    virtual bool Start(int time = -1, bool oneShot = wxTIMER_CONTINUOUS) override { return Start(time, oneShot, ""); }
    virtual void Notify() override;
    virtual void DoSendTimer();
    int GetInterval() const;
    void SetLog(bool log) { _log = true; }

    // If you use this method to receive the timer notification then be sure that you dont do any UI
    // updates in the callback function as it will be called on another thread. Also if you are going
    // to delete objects used in the callback be sure to suspend the time first
    void SetTimerCallback(xLightsTimerCallback* callback) { _timerCallback = callback; }
    void Suspend(bool suspend = true) { _suspend = suspend; }
};
