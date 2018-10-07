#ifndef RUNNINGSCHEDULE_H
#define RUNNINGSCHEDULE_H

class Schedule;
class PlayList;

class RunningSchedule
{
    PlayList* _playlist;
    Schedule* _schedule;
    bool _stop;

public:
    RunningSchedule(PlayList* playlist, Schedule* schedule);
    virtual ~RunningSchedule();
    bool operator<(const RunningSchedule& rs) const;
    PlayList* GetPlayList() const { return _playlist; }
    Schedule* GetSchedule() const { return _schedule; }
    void Reset();
    void Stop();
    bool IsStopped() const { return _stop; }
    bool ShouldBeRunning() const;
};
#endif
