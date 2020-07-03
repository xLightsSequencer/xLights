/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ListenerSMPTE.h"
#include "ListenerManager.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../ScheduleOptions.h"
#include "../../xLights/AudioManager.h"
#include <log4cpp/Category.hh>

ListenerSMPTE::ListenerSMPTE(int mode, ListenerManager* listenerManager) : ListenerBase(listenerManager)
{
    _mode = mode;
    _frameMS = 50;
}

void ListenerSMPTE::Start()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SMPTE listener starting.");
    _thread = new ListenerThread(this);
}

void ListenerSMPTE::Stop()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_stop)
    {
        logger_base.debug("SMPTE listener stopping.");
        if (_thread != nullptr)
        {
            _stop = true;
            _thread->Stop();
        }
    }
}

void ListenerSMPTE::StartProcess()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _total = 0;
    int apv = 1920;
    _decoder = ltc_decoder_create(apv, 32);

    if (_decoder != nullptr)
    {
        AudioManager::GetSDL()->StartListening();
        AudioManager::GetSDL()->PurgeInput();
        _isOk = true;
    }
}

void ListenerSMPTE::StopProcess()
{
     static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

     AudioManager::GetSDL()->StopListening();

     if (_decoder != nullptr)
     {
         ltc_decoder_free(_decoder);
         _decoder = nullptr;
     }

     _isOk = false;
}

void ListenerSMPTE::Poll()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_decoder == nullptr || _stop) return;

    int read = AudioManager::GetSDL()->GetInputAudio((uint8_t*)_buffer, sizeof(_buffer));

    if (read > 0)
    {
        ltc_decoder_write_u16(_decoder, _buffer, read / 2 /* because it is in bytes */, _total);
        while (ltc_decoder_read(_decoder, &_frame)) {
            SMPTETimecode stime;
            ltc_frame_to_time(&stime, &_frame.ltc, 1);
            DoSync(_mode, stime.hours, stime.mins, stime.secs, stime.frame);
        }
        _total += read;
    }
    wxMilliSleep(10);
}

void ListenerSMPTE::DoSync(int mode, int hours, int mins, int secs, int frames)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static long lastms = -99999;

    long ms = ((hours * 60 + mins) * 60 + secs) * 1000;
    double fps = 0;

    switch (mode)
    {
    default:
    case 0:
        //24 fps
        ms += frames * 1000 / 24;
        fps = 24;
        break;
    case 1:
        //25 fps
        ms += frames * 1000 / 25;
        fps = 25;
        break;
    case 2:
        //29.97 fps
        ms += frames * 100000 / 2997;
        fps = 29.97;
        break;
    case 3:
        //30 fps
        ms += frames * 1000 / 30;
        fps = 30;
        break;
    }

    if (ms - lastms > 10000 || ms < lastms)
    {
        logger_base.debug("SMPTE DoSync MS: %ld, hours: %d, Mins: %d, Sec: %d, Frames: %d. (FPS %.2g)", ms, hours, mins, secs, frames, fps);
        lastms = ms;
    }

    _listenerManager->Sync("", ms, GetType());
}
