#ifndef LISTENERBASE_H
#define LISTENERBASE_H

#include <string>
#include <wx/wx.h>

class ListenerManager;
class ScheduleManager;
class ListenerThread;

class ListenerBase
{
    protected:
	    bool _stop;
        ListenerManager* _listenerManager;
        ListenerThread* _thread;

	public:
        ListenerBase(ListenerManager* listenerManager);
		virtual ~ListenerBase() {}
		virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual std::string GetType() const = 0;
        virtual void StartProcess() = 0;
        virtual void StopProcess() = 0;
        virtual void Poll() {};
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

        // give it 10 ms to end ... this is not perfect but as the thread self deletes I cant wait for it.
        wxMilliSleep(10);
    }

    virtual void* Entry() override;
};
#endif

