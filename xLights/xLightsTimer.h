#ifndef XLIGHTSTIMER_H
#define XLIGHTSTIMER_H

#include <wx/timer.h>

#ifdef __WXMSW__
#define xLightsTimer wxTimer
//#define XLUSETIMER
#else
#define xLightsTimer wxTimer
#endif

#ifdef XLUSETIMER

#include <wx/thread.h>

class xlTimerThread : public wxThread
{
public:
    xlTimerThread(wxEvtHandler* pParent, int interval, bool oneshot, wxTimer* timer);
    void Stop();
    void SetFudgeFactor(int ff);
private:
    bool _oneshot;
    bool _stop;
    int _interval;
    int _fudgefactor;
    wxTimer* _timer;
    virtual ExitCode Entry();
protected:
    wxEvtHandler* m_pParent;
};

class xLightsTimer :
    public wxTimer
{
    xlTimerThread* _t;
    int _time;
    bool _oneshot;
public:
    xLightsTimer();
    virtual ~xLightsTimer() {};
    void Stop();
    bool Start(int time = -1, bool oneShot = wxTIMER_CONTINUOUS);
    int GetInterval();
};

#endif
#endif