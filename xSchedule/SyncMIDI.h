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

#include "SyncManager.h"
#include "ScheduleOptions.h"
#include <log4cpp/Category.hh>

#include <mutex>

#define USE_CLOCK_THREAD

class ListenerManager;
class wxMidiOutDevice;
class MIDITimecodeThread;
#ifdef USE_CLOCK_THREAD
class MIDIClockThread;
#endif

class SyncMIDI : public SyncBase
{
    wxMidiOutDevice* _midi = nullptr;
    size_t _timeCodeOffset = 0;
    std::string _timeCodeDevice = "";
    TIMECODEFORMAT _timeCodeFormat = TIMECODEFORMAT::F24;
    MIDITimecodeThread* _threadTimecode = nullptr;
    #ifdef USE_CLOCK_THREAD
    MIDIClockThread* _threadClock = nullptr;
    mutable std::mutex _mutex;
    #endif

    uint8_t GetTimeCodeBits() const;

    public:

        SyncMIDI(SYNCMODE sm, REMOTEMODE rm, const ScheduleOptions& options, ListenerManager* listenerManager);
        SyncMIDI(SyncMIDI&& from);
        virtual ~SyncMIDI();
        virtual void SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const override;
        virtual std::string GetType() const override { return "MIDI"; }
        virtual void SendStop() const override;
        double GetInterval();
        virtual bool IsReactive() const override { return false; }
        void SendClock() const;
};
