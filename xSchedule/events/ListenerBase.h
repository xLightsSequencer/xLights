#ifndef LISTENERBASE_H
#define LISTENERBASE_H

#include <string>
#include <wx/wx.h>
#include <atomic>

class ListenerManager;
class ScheduleManager;
class ListenerThread;

class ListenerBase
{
    protected:
	    bool _stop;
        ListenerManager* _listenerManager;
        ListenerThread* _thread;
        bool _isOk;
        std::atomic<int> _frameMS;

	public:
        ListenerBase(ListenerManager* listenerManager);
		virtual ~ListenerBase() {}
		virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual std::string GetType() const = 0;
        virtual void StartProcess() = 0;
        virtual void StopProcess() = 0;
        bool IsOk() const { return _isOk; }
        virtual void Poll() {};
        void SetFrameMS(int frameMS) { _frameMS = frameMS; }
};

class ListenerThread : public wxThread
{
    bool _stop;
    ListenerBase* _listener;
    bool _running;

public:
    ListenerThread(ListenerBase* listener);
    virtual ~ListenerThread()
    {
        Stop();
    }

    void Stop()
    {
        _stop = true;
        Wait();
    }

    virtual void* Entry() override;
};
#endif
