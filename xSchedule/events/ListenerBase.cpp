#include "ListenerBase.h"
#include <wx/wx.h>
#include "../ScheduleManager.h"
#include <log4cpp/Category.hh>

ListenerBase::ListenerBase(ListenerManager* listenerManager)
{
    _listenerManager = listenerManager;
    _stop = false;
    _thread = nullptr;
    _isOk = false;
}

ListenerThread::ListenerThread(ListenerBase* listener) : wxThread(wxTHREAD_JOINABLE)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _listener = listener;
    _stop = false;
    _running = false;
    if (Run() != wxTHREAD_NO_ERROR)
    {
        logger_base.error("Failed to start listener thread for %s", (const char *)listener->GetType().c_str());
    }
    else
    {
        logger_base.info("Listener thread for %s created.", (const char *)listener->GetType().c_str());
    }
}

void* ListenerThread::Entry()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_listener == nullptr)
    {
        logger_base.info("Listener thread started but listener was null. Exiting.");
        return nullptr;
    }

    _running = true;
    logger_base.info("Listener thread for %s running.", (const char *)_listener->GetType().c_str());

    _listener->StartProcess();

    if (_listener->IsOk())
    {
        while (!_stop)
        {
            _listener->Poll();
        }
    }

    _listener->StopProcess();
    _running = false;

    return nullptr;
}
