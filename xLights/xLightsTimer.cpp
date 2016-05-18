

#include "xLightsTimer.h"
#include <wx/thread.h>

#ifndef __WXOSX__
#define USE_THREADED_TIMER
#endif


#ifdef USE_THREADED_TIMER

class xlTimerThread : public wxThread
{
public:
    xlTimerThread(int interval, bool oneshot, wxTimer* timer);
    void Stop();
    void SetFudgeFactor(int ff);
private:
    bool _oneshot;
    bool _stop;
    int _interval;
    int _fudgefactor;
    wxTimer* _timer;
    virtual ExitCode Entry();
};

#pragma region xlTimerTimer
xLightsTimer::xLightsTimer()
{
    _t = NULL;
    pending = false;
}
xLightsTimer::~xLightsTimer()
{
    if (_t != NULL)
    {
        _t->Stop();
        _t = NULL; // while it may not exit immediately it will exit and kill itself
    }
}
void xLightsTimer::Stop()
{
    if (_t != NULL)
    {
        _t->Stop();
        _t = NULL; // while it may not exit immediately it will exit and kill itself
    }
}
bool xLightsTimer::Start(int time/* = -1*/, bool oneShot/* = wxTIMER_CONTINUOUS*/)
{
    Stop();
    _t = new xlTimerThread(time, oneShot, this);
    if (_t == NULL) return false;
    _t->Create();
    _t->SetPriority(WXTHREAD_DEFAULT_PRIORITY + 1); // run it with slightly higher priority to ensure events are generated in a timely manner
    _t->Run();
    return true;
}

void xLightsTimer::DoSendTimer() {
    if (!pending) {
        return;
    }
    wxTimer::Notify();
    //reset pending to false AFTER sending the event so if sending takes to long, it results in a skipped frame instead of
    //infinite number of CallAfters consuming the CPU
    pending = false;
}
void xLightsTimer::Notify() {
    pending = true;
    CallAfter(&xLightsTimer::DoSendTimer);
}





static wxCriticalSection critsect;

xlTimerThread::xlTimerThread(int interval, bool oneshot, wxTimer* timer)
{
    _stop = false;
    _fudgefactor = 0;
    _interval = interval;
    _timer = timer;
    _oneshot = (oneshot == wxTIMER_ONE_SHOT);
}
void xlTimerThread::Stop()
{
    critsect.Enter();
    _stop = true;
    critsect.Leave();
}
wxThread::ExitCode xlTimerThread::Entry()
{
    critsect.Enter();
    bool stop = _stop;
    int fudgefactor = _fudgefactor;
    bool oneshot = _oneshot;
    critsect.Leave();
    while (!stop)
    {
        wxMilliSleep(std::max(1, _interval + fudgefactor));
        critsect.Enter();
        stop = _stop;
        fudgefactor = _fudgefactor;
        if (!stop)
        {
            _timer->Notify();
        }
        if (oneshot)
        {
            stop = true;
        }
        critsect.Leave();
    }
    return 0;
}

void xlTimerThread::SetFudgeFactor(int ff)
{
    critsect.Enter();
    _fudgefactor = ff;
    critsect.Leave();
}
#else 
xLightsTimer::xLightsTimer() {}
xLightsTimer::~xLightsTimer() {}
void xLightsTimer::Stop() {wxTimer::Stop();}
bool xLightsTimer::Start(int time, bool oneShot) {return wxTimer::Start(time, oneShot);};
void xLightsTimer::Notify() {wxTimer::Notify();}
void xLightsTimer::DoSendTimer() {};
#endif
