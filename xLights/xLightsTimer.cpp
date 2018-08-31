

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
    int GetInterval() const { return _interval; }
private:
    std::atomic<bool> _stop;
    bool _oneshot;
    int _interval;
    int _fudgefactor;
    wxTimer* _timer;
    virtual ExitCode Entry();
};

#pragma region xlTimerTimer
xLightsTimer::xLightsTimer()
{
    _suspend = false;
    _timerCallback = nullptr;
    _t = nullptr;
    pending = false;
}
xLightsTimer::~xLightsTimer()
{
    if (_t != nullptr)
    {
        _t->Stop();
        _t = nullptr; // while it may not exit immediately it will exit and kill itself
    }
}
void xLightsTimer::Stop()
{
    if (_t != nullptr)
    {
        _t->Stop();
        _t = nullptr; // while it may not exit immediately it will exit and kill itself
    }
}
bool xLightsTimer::Start(int time/* = -1*/, bool oneShot/* = wxTIMER_CONTINUOUS*/)
{
    Stop();
    _t = new xlTimerThread(time, oneShot, this);
    if (_t == nullptr) return false;
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
    if (_suspend)
    {
        return;
    }

    if (_timerCallback != nullptr)
    {
        wxTimerEvent event(*this);
        _timerCallback->TimerCallback(event);
    }
    else
    {
        pending = true;
        CallAfter(&xLightsTimer::DoSendTimer);
    }
}

int xLightsTimer::GetInterval() const
{
    if (_t != nullptr)
    {
        return _t->GetInterval();
    }
    return -1;
}


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
    _stop = true;
}
wxThread::ExitCode xlTimerThread::Entry()
{
    bool stop = _stop;
    int fudgefactor = _fudgefactor;
    bool oneshot = _oneshot;
    long long last = wxGetLocalTimeMillis().GetValue();

    while (!stop)
    {
        long long now = wxGetLocalTimeMillis().GetValue();
        long long toSleep = last + _interval + fudgefactor - now;
        wxMilliSleep((std::max)(1, (int)toSleep));
        last = wxGetLocalTimeMillis().GetValue();
        stop = _stop;
        fudgefactor = _fudgefactor;
        if (oneshot || !stop)
        {
            _timer->Notify();
        }
        if (oneshot)
        {
            stop = true;
        }
    }

    return wxThread::ExitCode(nullptr);
}

void xlTimerThread::SetFudgeFactor(int ff)
{
    _fudgefactor = ff;
}
#else 
xLightsTimer::xLightsTimer() {}
xLightsTimer::~xLightsTimer() {}
void xLightsTimer::Stop() {wxTimer::Stop();}
bool xLightsTimer::Start(int time, bool oneShot) {return wxTimer::Start(time, oneShot);};
void xLightsTimer::Notify() {wxTimer::Notify();}
int xLightsTimer::GetInterval() const { return wxTimer::GetInterval(); }
void xLightsTimer::DoSendTimer() {};
#endif
