#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

class Schedule;
class PlayList;

class RunningSchedule
{
    PlayList* _playlist = nullptr;
    Schedule* _schedule = nullptr;
    bool _stop = false;

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
