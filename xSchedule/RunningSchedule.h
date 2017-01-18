#ifndef RUNNINGSCHEDULE_H
#define RUNNINGSCHEDULE_H
#include <list>
#include <string>
#include <wx/wx.h>

class Schedule;
class PlayList;

class RunningSchedule
{
    PlayList* _playlist;
    Schedule* _schedule;

public:
    RunningSchedule(PlayList* playlist, Schedule* schedule);
    virtual ~RunningSchedule();
    bool operator<(const RunningSchedule& rs) const;
    PlayList* GetPlayList() const { return _playlist; }
    Schedule* GetSchedule() const { return _schedule; }
    void Reset();
};
#endif
