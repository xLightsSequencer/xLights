#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

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
        std::string _localIP;

	public:
        ListenerBase(ListenerManager* listenerManager, const std::string& localIP);
		virtual ~ListenerBase() {}
		virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual std::string GetType() const = 0;
        virtual void StartProcess(const std::string& localIP) = 0;
        virtual void StopProcess() = 0;
        bool IsOk() const { return _isOk; }
        virtual void Poll() {};
        void SetFrameMS(int frameMS) {
            _frameMS = frameMS;
        }
        virtual std::string FormatTime(size_t timems) const {
            size_t totalSeconds = timems / 1000;
            size_t hours = totalSeconds / 3600;
            size_t minutes = (totalSeconds % 3600) / 60;
            size_t seconds = totalSeconds % 60;
            size_t milliseconds = timems % 1000;
            return wxString::Format("%02zu:%02zu:%02zu.%03zu", hours, minutes, seconds, milliseconds).ToStdString();
        }
        virtual std::string FormatTime(int mode, size_t timems) const {
            size_t totalSeconds = timems / 1000;
            size_t hours = totalSeconds / 3600;
            size_t minutes = (totalSeconds % 3600) / 60;
            size_t seconds = totalSeconds % 60;
            size_t milliseconds = timems % 1000;
            switch (mode) {
            case 0: // 24 FPS
                return wxString::Format("%02zu:%02zu:%02zu.%02zu", hours, minutes, seconds, (milliseconds * 24) / 1000).ToStdString();
            case 1: // 25 FPS
                return wxString::Format("%02zu:%02zu:%02zu.%02zu", hours, minutes, seconds, (milliseconds * 25) / 1000).ToStdString();
            case 2: // 29.97 FPS
                return wxString::Format("%02zu:%02zu:%02zu.%02zu", hours, minutes, seconds, (int)(((float)milliseconds * 29.97) / 1000.0)).ToStdString();
            case 3: // 30 FPS
                return wxString::Format("%02zu:%02zu:%02zu.%02zu", hours, minutes, seconds, (milliseconds * 30) / 1000).ToStdString();
            }
            return FormatTime(timems);
        }
};

class ListenerThread : public wxThread
{
    bool _stop = false;
    ListenerBase* _listener = nullptr;
    bool _running = false;
    std::string _localIP;

public:
    ListenerThread(ListenerBase* listener, const std::string& localIP);
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
