/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xLightsTimer.h"
#include <wx/thread.h>
#include "./utils/spdlog_macros.h"
#include <mutex>

#ifndef __WXOSX__
#define USE_THREADED_TIMER
#endif

#ifdef USE_THREADED_TIMER

#define MIN_SLEEP_BEFORE_LOG 5

class xlTimerThread : public wxThread
{
public:
    xlTimerThread(const std::string& name, int interval, bool oneshot, xLightsTimer* timer, bool log);
    virtual ~xlTimerThread() {
    };
    void Reset(int interval, bool oneshot, const std::string& name);
    void Stop();
    void Suspend();
    void SetFudgeFactor(int ff);
    int GetInterval() const { return _interval; }
    void SetName(const std::string& name) {
        _name = name;
    }
private:
    std::atomic<bool> _stop;
    std::atomic<bool> _suspend;
    std::atomic<bool> _oneshot;
    std::atomic<int> _interval;
    std::atomic<int> _suspendCount;
    int _fudgefactor;
    bool _log;
    xLightsTimer* _timer;
    std::string _name;

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
xLightsTimer::xLightsTimer() :
    wxTimer()
{
    _log = false;
    _suspend = false;
    _timerCallback = nullptr;
    _t = nullptr;
    _pending = false;
    _name = "";
    _fired = 0;
}

xLightsTimer::~xLightsTimer()
{
    if (_t != nullptr)
    {
        _t->Stop();
        _t->Delete();
        delete _t;
        _t = nullptr;
    }
}

void xLightsTimer::Stop()
{
    if (_t != nullptr)
    {
        _t->Suspend();
    }
}

void xLightsTimer::SetName(const std::string& name)
{
    _name = name; 
    if (_t != nullptr) _t->SetName(name);
}

bool xLightsTimer::Start(int time/* = -1*/, bool oneShot/* = wxTIMER_CONTINUOUS*/, const std::string& name)
{
    
    wxStopWatch sw;

    // While there is support for one shot timers here ... it is not the most robust code and should
    // be avoided if possible
    wxASSERT(oneShot == wxTIMER_CONTINUOUS);

    _fired = 0;
    _startTime = std::chrono::system_clock::now();

    if (name != "") _name = name;

    if (_t == nullptr)
    {
        LOG_DEBUG("Timer thread created for %s", (const char*)_name.c_str());
        _t = new xlTimerThread(name, time, oneShot, this, _log);
        if (_t == nullptr) return false;
        _t->Create();
        _t->SetPriority(WXTHREAD_DEFAULT_PRIORITY + 1); // run it with slightly higher priority to ensure events are generated in a timely manner
        _t->Run();
    }
    else
    {
        LOG_INFO("Resetting timer %s as thread already exists.", (const char*)_name.c_str());
        Stop();
        _t->Reset(time, oneShot, _name);
    }

    LOG_DEBUG("Timer %s started in %ldms", (const char*)_name.c_str(), sw.Time());

    return true;
}

void xLightsTimer::DoSendTimer() {
    if (!_pending) {
        return;
    }
    wxTimer::Notify();
    //reset pending to false AFTER sending the event so if sending takes to long, it results in a skipped frame instead of
    //infinite number of CallAfters consuming the CPU
    _pending = false;
}

void xLightsTimer::Notify() {

    ++_fired;

    // don't notify if there is still an event processing or we are suspended
    if (_suspend || _pending)
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
        _pending = true;
        wxTimer::CallAfter(&xLightsTimer::DoSendTimer);
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

std::chrono::time_point<std::chrono::system_clock> xLightsTimer::GetNextEventTime()
{
    std::chrono::time_point<std::chrono::system_clock> next = _startTime + std::chrono::milliseconds((_fired + 1) * GetInterval());
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    if (now >= next) {
        LOG_DEBUG("THREAD %ld: Timer missed %ldms worth of frames.", wxThread::GetCurrentId(), (long)std::chrono::duration_cast<std::chrono::milliseconds>(now - next).count());
        _fired = std::chrono::duration_cast<std::chrono::milliseconds>(now - _startTime).count() / GetInterval();
        next = _startTime + std::chrono::milliseconds((_fired + 1) * GetInterval());
        while (std::chrono::duration_cast<std::chrono::milliseconds>(next - now).count() <= 0) {
            ++_fired;
            next = _startTime + std::chrono::milliseconds((_fired + 1) * GetInterval());
        }
        LOG_DEBUG("     Next frame is now %ldms in future. Interval: %d", (long)std::chrono::duration_cast<std::chrono::milliseconds>(next - now).count(), GetInterval());
    }
    //wxASSERT(next >= now);
    return next;
}

inline void xLightsTimer::Suspend(bool suspend)
{
    if (!suspend) {
        _fired = 0;
        _startTime = std::chrono::system_clock::now();
    }

    _suspend = suspend;
}

xlTimerThread::xlTimerThread(const std::string& name, int interval, bool oneshot, xLightsTimer* timer, bool log) : wxThread(wxTHREAD_JOINABLE)
{
    

    // shouldnt be creating the timer thread with an interval less than zero
    wxASSERT(interval >= 0);

    _name = name;
    _log = log;
    _stop = false;
    _suspend = false;
    _fudgefactor = 0;
    _interval = interval;
    _timer = timer;
    _oneshot = (oneshot == wxTIMER_ONE_SHOT);
    _suspendCount = 0;

    // grab the wait lock so the timer thread has its timing behaviour
    LOG_DEBUG("About to grab the waiter");
    _waiter.lock();
    LOG_DEBUG("    got it");
}

void xlTimerThread::Reset(int interval, bool oneshot, const std::string& name)
{
    

    if (name != "") _name = name;

    LOG_DEBUG("Timer %s reset from interval %d to interval %d %s", (const char*)_name.c_str(), (int)_interval, interval, oneshot ? "ONESHOT" : "");

    wxASSERT(_suspend == true);
    wxASSERT(_stop == false);

    int oldInterval = _interval;

    if (oldInterval == -99)
    {
        LOG_DEBUG("Timer being reset was a one shot");
    }

    // only set the interval to a new value if it is greater than or equal to zero
    if (interval >= 0)
    {
        _interval = interval;
    }
    _oneshot = oneshot;

    wxStopWatch sw;

    // if this was not a one shot suspend
    if (oldInterval != -99)
    {
        // grab the wait lock so the timer thread goes back to its timing behaviour
        LOG_DEBUG("About to grab the waiter");
        _waiter.lock();
        LOG_DEBUG("    got it");
    }

    _suspend = false;

    // if this was not a one shot suspend
    if (oldInterval != -99 && _suspendCount > 0)
    {
        // now release the suspend
        LOG_DEBUG("About to release the suspendLock");
        _suspendLock.unlock();
        LOG_DEBUG("    released");
    }

    LOG_DEBUG("    Reset took %ldms", sw.Time());
}

void xlTimerThread::Suspend()
{
    

    if (_suspend) return;

    LOG_DEBUG("Timer %s suspend", (const char*)_name.c_str());
    wxStopWatch sw;

    _suspend = true;

    // lock the suspend lock on the main thread so the timer thread will block until it is released
    LOG_DEBUG("About to grab the suspendLock");
    _suspendLock.lock();
    LOG_DEBUG("    got it");

    int sc = _suspendCount;

    // release this lock on the main thread which immediately stops the timer wait
    LOG_DEBUG("About to release the waiter");
    _waiter.unlock();
    LOG_DEBUG("    released");

    // this ensures the other thread saw suspended before we continue ...
    // but also even worst case we never wait more than 3ms
    int i = 0;
    while (sc == _suspendCount && i < 3)
    {
        // give the timer thread a chance to use the unlocked waiter
        wxMilliSleep(1);
        i++;
    }
    if (i == 3)
    {
        LOG_WARN("    Waited 3 seconds for thread to lock and didnt see it ... maybe because it grabbed it early.");
    }

    LOG_DEBUG("    Suspend took %ldms", sw.Time());
}

void xlTimerThread::Stop()
{
    

    if (_stop) return;

    LOG_DEBUG("Timer %s stop", (const char*)_name.c_str());
    wxStopWatch sw;

    int oldInterval = _interval;
    _stop = true;
    _suspend = false;

    // release this lock on the main thread which immediately stops the timer wait
    _waiter.unlock();

    // if this was not a one shot suspend
    if (oldInterval != -99 && _suspendCount > 0)
    {
        // also release any suspended state so the thread will exit
        _suspendLock.unlock();
    }

    // give the timer thread a chance to use the unlocked waiter
    wxMilliSleep(1);

    LOG_DEBUG("    Stop took %ldms", sw.Time());
}

void xlTimerThread::DoSleep(int millis)
{
    
    if (millis > MIN_SLEEP_BEFORE_LOG)
    {
        LOG_DEBUG("THREAD %ld: DoSleep(%d)", wxThread::GetCurrentId(), millis);
    }

    // try to grab the lock but time out after the desired number of milliseconds
    if (_waiter.try_lock_for(std::chrono::milliseconds(millis)))
    {
        if (millis > MIN_SLEEP_BEFORE_LOG)
        {
            LOG_DEBUG("THREAD %ld: DoSleep(%d) ... timer was aborted", wxThread::GetCurrentId(), millis);
        }
        wxASSERT(_suspend == true || _stop == true);

        // we got the lock so release it immediately
        _waiter.unlock();
    }
    else
    {
        if (millis > MIN_SLEEP_BEFORE_LOG)
        {
            LOG_DEBUG("THREAD %ld: DoSleep(%d) ... %s timer timed out", wxThread::GetCurrentId(), millis, (const char*)_name.c_str());
        }
    }
}

wxThread::ExitCode xlTimerThread::Entry()
{
    

    bool log = _log;
    bool oneshot = _oneshot;
    int interval = _interval;
    int fudgefactor = _fudgefactor;

    while (!_stop)
    {
        if (_suspend)
        {
            LOG_DEBUG("THREAD %ld: Timer %s thread suspended. Interval %d", wxThread::GetCurrentId(), (const char*)_name.c_str(), interval);

            // If we were one shot we cant use the fancy locks because we were in this thread
            // when we suspended ... and we cant change threads as the delay will cause an issue
            if (_interval == -99)
            {
                // this was one shot so we cant use the locks ... do it oldschool
                ++_suspendCount;
                while (_suspend)
                {
                    wxMilliSleep(1);
                }
            }
            else
            {
                ++_suspendCount;

                // we look like we are in suspend mode so try to grab the suspend lock and block until we get it
                LOG_DEBUG("THREAD %ld: About to grab the suspend lock", wxThread::GetCurrentId());
                _suspendLock.lock();
                LOG_DEBUG("THREAD %ld:     got it", wxThread::GetCurrentId());

                // now we got it ... release it
                _suspendLock.unlock();
                LOG_DEBUG("THREAD %ld:     released it", wxThread::GetCurrentId());
            }

            LOG_DEBUG("THREAD %ld: Timer %s thread unsuspended.", wxThread::GetCurrentId(), (const char*)_name.c_str());
        }

        oneshot = _oneshot;
        interval = _interval;

        if (!_stop)
        {
            long long toSleep = 0;
            auto now = std::chrono::system_clock::now();
            auto nextTime = _timer->GetNextEventTime();
            if (nextTime - std::chrono::milliseconds(_fudgefactor) > now) {
                toSleep = std::chrono::duration_cast<std::chrono::milliseconds>(nextTime - now - std::chrono::milliseconds(fudgefactor)).count();
                LOG_DEBUG("THREAD %ld: Timer %s sleeping for %ldms.", wxThread::GetCurrentId(), (const char*)_name.c_str(), (long)toSleep);
            } else {
                LOG_DEBUG("THREAD %ld: Timer %s did not need to sleep.", wxThread::GetCurrentId(), (const char*)_name.c_str());
            }
            
            //if (log)
            //{
            //    LOG_DEBUG("Timer sleeping for %ldms", (std::max)(1, (int)toSleep));
            //}
            if (toSleep > 0) {
                DoSleep((std::max)(1, (int)toSleep));
            }

            bool suspend = _suspend;
            fudgefactor = _fudgefactor;
            if (!_stop && !suspend)
            {
                LOG_DEBUG("THREAD %ld: Timer %s fired %lu.", wxThread::GetCurrentId(), (const char*)_name.c_str(), _timer->GetFired());
                _timer->Notify();
            }
            if (oneshot)
            {
                LOG_DEBUG("THREAD %ld: %s ONESHOT SO AUTOMATICALLY SUSPENDING.", wxThread::GetCurrentId(), (const char*)_name.c_str());
                _suspend = true;
                _interval = -99;
                interval = -99;
            }
        }
    }

    LOG_DEBUG("Timer %s thread %ld exiting.", (const char*)_name.c_str(), wxThread::GetCurrentId());

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
bool xLightsTimer::Start(int time, bool oneShot, const std::string& name) {return wxTimer::Start(time, oneShot);};
void xLightsTimer::Notify() { RunInAutoReleasePool([this]() {wxTimer::Notify();});}
int xLightsTimer::GetInterval() const { return wxTimer::GetInterval(); }
void xLightsTimer::DoSendTimer() {};
void xLightsTimer::SetName(const std::string& name) {_name = name;}

#endif
