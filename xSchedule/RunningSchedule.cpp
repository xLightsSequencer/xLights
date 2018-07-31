#include "RunningSchedule.h"
#include "Schedule.h"
#include "PlayList/PlayList.h"
#include <log4cpp/Category.hh>

RunningSchedule::RunningSchedule(PlayList* playlist, Schedule* schedule)
{
    _stop = false;
    _playlist = new PlayList(*playlist);
    _schedule = new Schedule(*schedule);

    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.info("Running schedule create containing Playlist %s and Schedule %s 0x%lx.", (const char*)GetPlayList()->GetName().c_str(), (const char*)GetSchedule()->GetName().c_str(), this);
}

RunningSchedule::~RunningSchedule()
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.info("Running schedule containing Playlist %s and Schedule %s being destroyed 0x%lx.", (const char*)GetPlayList()->GetName().c_str(), (const char*)GetSchedule()->GetName().c_str(), this);

    _playlist->Stop();

    delete _playlist;
    _playlist = nullptr;
    delete _schedule;
    _schedule = nullptr;
}

bool RunningSchedule::operator<(const RunningSchedule& rs) const
{
    // this is intentional ... I want them in reverse order
    return _schedule->GetPriority() > rs._schedule->GetPriority();
}

void RunningSchedule::Reset()
{
    _stop = false;
    if (!_playlist->IsRunning())
    {
        _playlist->StartSuspended(_schedule->GetLoop(), _schedule->GetRandom(), _schedule->GetLoops());
    }
    else
    {
        _playlist->Stop();
        _playlist->StartSuspended(_schedule->GetLoop(), _schedule->GetRandom(), _schedule->GetLoops());
    }
    if (_playlist->IsPaused())
    {
        _playlist->TogglePause();
    }
}

void RunningSchedule::Stop()
{
    _stop = true;
    _playlist->Stop();
}

bool RunningSchedule::ShouldBeRunning() const
{
    return !_stop && _schedule->CheckActive();
}