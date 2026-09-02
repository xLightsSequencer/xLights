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

#ifndef __WXOSX__

// macOS drives the timer from CADisplayLink in xLightsTimer.mm.
//
// On GTK wxTimer is g_timeout_add(), a GLib main loop timeout with millisecond
// resolution, which is accurate enough to use directly.
//
// On Windows wxTimer is SetTimer()/WM_TIMER, whose period is rounded up to the
// system timer tick - 15.6ms, so a 25ms frame interval is really delivered every
// 31.2ms. timeBeginPeriod() does not help: it raises the scheduler/waitable timer
// resolution but WM_TIMER is still serviced off the USER tick. A waitable timer
// created with CREATE_WAITABLE_TIMER_HIGH_RESOLUTION does hit the requested
// interval, so Windows waits on one of those and hands the tick to the main
// thread. The wait runs on a thread pool thread rather than one of ours.

#ifdef __WXMSW__

#include <windows.h>

#include <wx/thread.h>

#include <log.h>

#include <mutex>

#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

namespace {
// QPC ticks elapsed since a reference, in 100ns units. Split the divide so the
// intermediate cannot overflow on a long running timer.
long long ElapsedIn100ns(long long ticks, long long freq) {
    return (ticks / freq) * 10000000LL + ((ticks % freq) * 10000000LL) / freq;
}
long long QpcFrequency() {
    static const long long freq = [] {
        LARGE_INTEGER f;
        ::QueryPerformanceFrequency(&f);
        return (long long)f.QuadPart;
    }();
    return freq;
}
long long QpcNow() {
    LARGE_INTEGER c;
    ::QueryPerformanceCounter(&c);
    return (long long)c.QuadPart;
}
}

class xLightsTimerDataImpl
{
public:
    explicit xLightsTimerDataImpl(xLightsTimer* timer) :
        _timer(timer) {
    }

    ~xLightsTimerDataImpl() {
        Stop();
        if (_handle != nullptr) {
            ::CloseHandle(_handle);
            _handle = nullptr;
        }
    }

    bool Start(int interval) {
        Stop();

        if (interval <= 0) {
            return false;
        }
        _interval = interval;

        if (_handle == nullptr) {
            _handle = ::CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
            if (_handle == nullptr) {
                // Pre-1803 does not know the flag. The coarser timer is still far
                // better than WM_TIMER.
                spdlog::debug("High resolution waitable timer unavailable ({}); falling back.", (unsigned long)::GetLastError());
                _handle = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
            }
            if (_handle == nullptr) {
                spdlog::error("Could not create a waitable timer: {}", (unsigned long)::GetLastError());
                return false;
            }
        }

        if (_wait == nullptr) {
            _wait = ::CreateThreadpoolWait(&xLightsTimerDataImpl::WaitCallback, this, nullptr);
            if (_wait == nullptr) {
                spdlog::error("Could not create a thread pool wait: {}", (unsigned long)::GetLastError());
                return false;
            }
        }

        _epoch = QpcNow();
        _ticks = 0;
        {
            std::lock_guard<std::mutex> lock(_armLock);
            _stopping = false;
            Arm();
            ::SetThreadpoolWait(_wait, _handle, nullptr);
        }
        return true;
    }

    void Stop() {
        if (_wait != nullptr) {
            {
                // The callback re-arms the wait under this lock, so taking it here
                // means no re-arm can slip in after the cancel below and leave a
                // wait outstanding when the object is closed.
                std::lock_guard<std::mutex> lock(_armLock);
                _stopping = true;
                ::SetThreadpoolWait(_wait, nullptr, nullptr);
            }
            // Must not be held under the lock - a callback already running wants it.
            ::WaitForThreadpoolWaitCallbacks(_wait, TRUE);
            ::CloseThreadpoolWait(_wait);
            _wait = nullptr;
        }
        if (_handle != nullptr) {
            ::CancelWaitableTimer(_handle);
        }
    }

    int GetInterval() const {
        return _interval;
    }

private:
    // Schedules the next tick against an absolute schedule rather than "interval
    // from now", so a late tick does not push every later one out with it. If we
    // are already past a tick - a frame took longer than the interval - that tick
    // is skipped rather than fired immediately.
    void Arm() {
        const long long freq = QpcFrequency();
        long long elapsed = ElapsedIn100ns(QpcNow() - _epoch, freq);
        const long long period = (long long)_interval * 10000LL;

        ++_ticks;
        long long due = _ticks * period;
        if (due <= elapsed) {
            _ticks = elapsed / period + 1;
            due = _ticks * period;
        }

        LARGE_INTEGER relative;
        relative.QuadPart = -(due - elapsed); // negative == relative, in 100ns units
        ::SetWaitableTimer(_handle, &relative, 0, nullptr, nullptr, FALSE);
    }

    static void CALLBACK WaitCallback(PTP_CALLBACK_INSTANCE, PVOID context, PTP_WAIT, TP_WAIT_RESULT) {
        auto* self = static_cast<xLightsTimerDataImpl*>(context);
        {
            std::lock_guard<std::mutex> lock(self->_armLock);
            if (self->_stopping) {
                return;
            }
            self->Arm();
            ::SetThreadpoolWait(self->_wait, self->_handle, nullptr);
        }
        self->_timer->Notify();
    }

    xLightsTimer* _timer;
    HANDLE _handle = nullptr;
    PTP_WAIT _wait = nullptr;
    int _interval = 0;
    long long _epoch = 0;
    long long _ticks = 0;
    std::mutex _armLock;
    bool _stopping = false;
};

xLightsTimer::xLightsTimer() {
    data = new xLightsTimerDataImpl(this);
}

xLightsTimer::~xLightsTimer() {
    delete data;
    data = nullptr;
}

void xLightsTimer::Stop() {
    _running = false;
    data->Stop();
    wxTimer::Stop();
}

bool xLightsTimer::Start(int time, bool oneShot, const std::string& name) {
    if (name != "") {
        _name = name;
    }
    if (time < 0) {
        time = data->GetInterval();
    }

    // One shot timers are rare here and do not need the accuracy, so let wx have them.
    if (oneShot == wxTIMER_ONE_SHOT || !data->Start(time)) {
        bool started = wxTimer::Start(time, oneShot);
        _running = started;
        return started;
    }

    _running = true;
    return true;
}

int xLightsTimer::GetInterval() const {
    int interval = data->GetInterval();
    return interval > 0 ? interval : wxTimer::GetInterval();
}

void xLightsTimer::Notify() {
    if (!wxThread::IsMain()) {
        // One undelivered tick may be outstanding at a time, so a slow frame cannot
        // queue up a backlog of CallAfters.
        if (_pending.exchange(true)) {
            return;
        }
        wxTimer::CallAfter(&xLightsTimer::DoSendTimer);
        return;
    }
    wxTimer::Notify();
}

void xLightsTimer::DoSendTimer() {
    // Cleared only once the frame has been delivered. Clearing first lets a tick
    // that lands mid-frame queue another CallAfter, and
    // wxEvtHandler::ProcessPendingEvents loops until its queue is empty - with a
    // frame slower than the interval it never is, so the main loop stops
    // servicing paint and input and the app hangs.
    struct ClearOnExit {
        std::atomic<bool>& flag;
        ~ClearOnExit() {
            flag = false;
        }
    } clearOnExit{ _pending };
    wxTimer::Notify();
}

#else // !__WXMSW__

xLightsTimer::xLightsTimer() {}
xLightsTimer::~xLightsTimer() {}

void xLightsTimer::Stop() {
    _running = false;
    wxTimer::Stop();
}

bool xLightsTimer::Start(int time, bool oneShot, const std::string& name) {
    if (name != "") {
        _name = name;
    }
    bool started = wxTimer::Start(time, oneShot);
    _running = started;
    return started;
}

int xLightsTimer::GetInterval() const {
    return wxTimer::GetInterval();
}

void xLightsTimer::Notify() {
    wxTimer::Notify();
}

void xLightsTimer::DoSendTimer() {}

#endif // __WXMSW__

void xLightsTimer::SetName(const std::string& name) {
    _name = name;
}

#endif // !__WXOSX__
