#include "xLightsTimer.h"

#ifdef XLUSETIMER

#pragma region xlTimerTimer
xLightsTimer::xLightsTimer()
{
    _t = NULL;
}
void xLightsTimer::Stop()
{
    if (_t != NULL)
    {
        _t->Stop();
        _t = NULL; // while it may not exit immediately it will exit and kill itself
    }
}
int xLightsTimer::GetInterval()
{
    return _time;
}
bool xLightsTimer::Start(int time/* = -1*/, bool oneShot/* = wxTIMER_CONTINUOUS*/)
{
    Stop();
    if (time != -1)
    {
        _time = time;
    }
    _oneshot = oneShot;
    _t = new xlTimerThread(GetOwner(), time, _oneshot, this);
    if (_t == NULL) return false;
    _t->Create();
    _t->SetPriority(WXTHREAD_DEFAULT_PRIORITY + 1); // run it with slightly higher priority to ensure events are generated in a timely manner
    _t->Run();
    return true;
}
#pragma endregion xlTimerTimer

#pragma region xlTimerThread

static wxCriticalSection critsect;

xlTimerThread::xlTimerThread(wxEvtHandler* pParent, int interval, bool oneshot, wxTimer* timer)
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
    wxEvtHandler* parent = m_pParent;
    critsect.Leave();
    while (!stop)
    {
        wxMilliSleep(std::max(1, _interval + fudgefactor));
        critsect.Enter();
        stop = _stop;
        fudgefactor = _fudgefactor;
        if (oneshot)
        {
            stop = true;
        }
        parent = m_pParent;
        critsect.Leave();
        if (!stop && parent != NULL)
        {
             wxTimerEvent *event = new wxTimerEvent(*(wxTimer*)this);
            _timer->GetOwner()->QueueEvent(event);
            //_timer->Notify();
        }
    }
    return 0;
}

void xlTimerThread::SetFudgeFactor(int ff)
{
    critsect.Enter();
    _fudgefactor = ff;
    critsect.Leave();
}
#pragma endregion xlTimerThread

#endif