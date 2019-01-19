#ifndef SYNCMIDI_H
#define SYNCMIDI_H

#include "SyncManager.h"
#include "ScheduleOptions.h"
#include <log4cpp/Category.hh>

class ListenerManager;
class wxMidiOutDevice;

class SyncMIDI : public SyncBase
{
    wxMidiOutDevice* _midi = nullptr;
    size_t _timeCodeOffset = 0;
    std::string _timeCodeDevice = "";
    TIMECODEFORMAT _timeCodeFormat = TIMECODEFORMAT::F24;

    public:

        SyncMIDI(SYNCMODE sm, REMOTEMODE rm, const ScheduleOptions& options, ListenerManager* listenerManager);
        SyncMIDI(SyncMIDI& from);
        virtual ~SyncMIDI();
        virtual void SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const override;
        virtual std::string GetType() const override { return "MIDI"; }
        virtual void SendStop() const override;
};
#endif
