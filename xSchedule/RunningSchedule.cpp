#include "RunningSchedule.h"
#include "Schedule.h"
#include "PlayList/PlayList.h"

RunningSchedule::RunningSchedule(PlayList* playlist, Schedule* schedule)
{
    _playlist = new PlayList(*playlist);
    _schedule = new Schedule(*schedule);
}

RunningSchedule::~RunningSchedule()
{
    _playlist->Stop();

    delete _playlist;
    delete _schedule;
}

bool RunningSchedule::operator<(const RunningSchedule& rs) const
{
    return *_schedule < *rs._schedule;
}
