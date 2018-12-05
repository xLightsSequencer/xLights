

#include "xLightsTimer.h"
#include <wx/thread.h>
#include <log4cpp/Category.hh>
#include <mutex>

#ifndef __WXOSX__
#define USE_THREADED_TIMER
#endif


#ifdef USE_THREADED_TIMER

class xlTimerThread : public wxThread
{
public:
    xlTimerThread(int interval, bool oneshot, wxTimer* timer, bool log);
    void Reset(int interval, bool oneshot);
    void Stop();
    void Suspend();
    void SetFudgeFactor(int ff);
    int GetInterval() const { return _interval; }
private:
    std::atomic<bool> _stop;
    std::atomic<bool> _suspend;
    std::atomic<bool> _oneshot;
    std::atomic<int> _interval;
    int _fudgefactor;
    bool _log;
    wxTimer* _timer;
    std::timed_mutex _waiter;
    std::mutex _suspendLock;
    void DoSleep(int millis);
    virtual ExitCode Entry() override;
};

#pragma region xlTimerTimer
xLightsTimer::xLightsTimer()
{
    _log = false;
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
    }
}

void xLightsTimer::Stop()
{
    if (_t != nullptr)
    {
        _t->Suspend();
    }
}

bool xLightsTimer::Start(int time/* = -1*/, bool oneShot/* = wxTIMER_CONTINUOUS*/)
{
    Stop();
    if (_t == nullptr)
    {
        _t = new xlTimerThread(time, oneShot, this, _log);
        if (_t == nullptr) return false;
        _t->Create();
        _t->SetPriority(WXTHREAD_DEFAULT_PRIORITY + 1); // run it with slightly higher priority to ensure events are generated in a timely manner
        _t->Run();
    }
    else
    {
        _t->Reset(time, oneShot);
    }
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

xlTimerThread::xlTimerThread(int interval, bool oneshot, wxTimer* timer, bool log) : wxThread(wxTHREAD_JOINABLE)
{
    _log = log;
    _stop = false;
    _suspend = false;
    _fudgefactor = 0;
    _interval = interval;
    _timer = timer;
    _oneshot = (oneshot == wxTIMER_ONE_SHOT);
    _waiter.lock();
}

void xlTimerThread::Reset(int interval, bool oneshot)
{
    _interval = interval;
    _oneshot = oneshot;
    _suspend = false;
    _suspendLock.unlock();
    _waiter.lock();
}

void xlTimerThread::Suspend()
{
    _suspend = true;
    _suspendLock.lock();
    _waiter.unlock();
    wxMilliSleep(1); // give the timer thread a chance to use the unlocked waiter
}

void xlTimerThread::Stop()
{
    _stop = true;
    _waiter.unlock();
    wxMilliSleep(1); // give the timer thread a chance to use the unlocked waiter
}

void xlTimerThread::DoSleep(int millis)
{
    if (_waiter.try_lock_for(std::chrono::milliseconds(millis)))
    {
        _waiter.unlock();
    }
}

wxThread::ExitCode xlTimerThread::Entry()
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool log = _log;
    bool stop = _stop;
    bool oneshot = _oneshot;
    int interval = _interval;
    int fudgefactor = _fudgefactor;
    long long last = wxGetLocalTimeMillis().GetValue();

    while (!stop)
    {
        if (_suspend)
        {
            _suspendLock.lock();
            _suspendLock.unlock();
        }

        oneshot = _oneshot;
        interval = _interval;

        if (!_stop)
        {
            long long now = wxGetLocalTimeMillis().GetValue();
            long long toSleep = last + interval + fudgefactor - now;
            //if (log)
            //{
            //    logger_base.debug("Timer sleeping for %ldms", (std::max)(1, (int)toSleep));
            //}
            DoSleep((std::max)(1, (int)toSleep));
            last = wxGetLocalTimeMillis().GetValue();
            stop = _stop;
            fudgefactor = _fudgefactor;
            if (oneshot || !stop)
            {
                _timer->Notify();
            }
            if (oneshot)
            {
                _suspend = true;
            }
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
