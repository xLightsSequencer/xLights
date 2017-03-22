#ifndef XLIGHTSTIMER_H
#define XLIGHTSTIMER_H

#include <wx/timer.h>

#include <atomic>

class xlTimerThread;

class xLightsTimer :
    public wxTimer
{
    xlTimerThread* _t;
    std::atomic<bool> pending;
public:
    xLightsTimer();
    virtual ~xLightsTimer();
    virtual void Stop();
    virtual bool Start(int time = -1, bool oneShot = wxTIMER_CONTINUOUS);
    virtual void Notify();
    virtual void DoSendTimer();
    int GetInterval() const;
};

#endif