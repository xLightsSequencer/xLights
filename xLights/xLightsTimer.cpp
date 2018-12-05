

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

    // the main thread holds a lock on this mutex while the timer is running
    // it is the timer thread timing out trying to lock it that actually makes the timer work
    // when stopped or suspended the main thread releases the lock.
    // If the timer thread manages to get the lock it immediately releases it
    std::timed_mutex _waiter;
    
    // when the main thread holds this lock the timer thread will block ... waiting for it to be
    // released. Once the timer thread gets it it immediately releases it.
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

    // grab the wait lock so the timer thread has its timing behaviour
    _waiter.lock();
}

void xlTimerThread::Reset(int interval, bool oneshot)
{
    _interval = interval;
    _oneshot = oneshot;
    _suspend = false;

    // grab the wait lock so the timer thread goes back to its timing behaviour
    _waiter.lock();

    // now release the suspend
    _suspendLock.unlock();
}

void xlTimerThread::Suspend()
{
    _suspend = true;

    // lock the suspend lock on the main thread so the timer thread will block until it is released
    _suspendLock.lock();

    // release this lock on the main thread which immediately stops the timer wait
    _waiter.unlock();

    // give the timer thread a chance to use the unlocked waiter
    wxMilliSleep(1); 
}

void xlTimerThread::Stop()
{
    _stop = true;

    // release this lock on the main thread which immediately stops the timer wait
    _waiter.unlock();

    // also release any suspended state so the thread will exit
    _suspendLock.unlock();
    
    // give the timer thread a chance to use the unlocked waiter
    wxMilliSleep(1);
}

void xlTimerThread::DoSleep(int millis)
{
    // try to grab the lock but time out after the desired number of milliseconds
    if (_waiter.try_lock_for(std::chrono::milliseconds(millis)))
    {
        // we got the lock so release it immediately
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
            if (_interval < 0)
            {
                // this was one shot so we cant use the locks ... do it oldschool
                while (_suspend)
                {
                    wxMilliSleep(1);
                }
            }
            // we look like we are in suspend mode so try to grab the suspend lock and block until we get it
            _suspendLock.lock();

            // now we got it ... release it
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
                // if it is one shot then it immediately suspends
                _suspend = true;

                // this is done so we can use the oldschool suspend without using the mutex
                // this is necessary because the suspendLock has to be grabbed by another thread 
                // and we have no way of making that happen
                _interval = -1;
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
