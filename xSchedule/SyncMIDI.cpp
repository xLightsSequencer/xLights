/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SyncMIDI.h"
#include "ScheduleOptions.h"
#include "events/ListenerManager.h"
#include "wxMIDI/src/wxMidi.h"
#include "ScheduleManager.h"
#include "PlayList/PlayList.h"

#include <log4cpp/Category.hh>
#include "../xLights/UtilFunctions.h"

class MIDITimecodeThread : public wxThread
{
    std::atomic<bool> _stop;
    SyncMIDI* _syncMidi = nullptr;
    std::atomic<bool> _running;
    std::atomic<bool> _suspend;
    ScheduleManager* _scheduleManager = nullptr;
    bool _toSendStop = false; // prevents us sending multiple stops

public:
    MIDITimecodeThread(SyncMIDI* syncMIDI, ScheduleManager* scheduleManager)
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        _suspend = false;
        _running = false;
        _stop = false;
        _syncMidi = syncMIDI;
        _scheduleManager = scheduleManager;

        if (Run() != wxTHREAD_NO_ERROR)
        {
            logger_base.error("Failed to start MIDI Timecode thread");
        }
        else
        {
            logger_base.info("MIDI Timecode thread created.");
        }
    }
    virtual ~MIDITimecodeThread()
    {
        Stop();
    }

    void Stop()
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("MIDI Timecode thread stopping.");
        _stop = true;
    }

    void UpdateSyncMIDI(SyncMIDI* syncMIDI)
    {
        _suspend = true;
        wxMilliSleep(100); // ensure it is suspended ... this is lazy ... i really should use sync objects
        _syncMidi = syncMIDI;
        _suspend = false;
    }

    void* Entry()
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        wxLongLong last = 0;
        double interval = _syncMidi->GetInterval() * 1000.0; // quarter frame messages
        _running = true;
        while (!_stop)
        {
            if (!_suspend)
            {
                long long sleepfor = 0;

                if (wxGetUTCTimeUSec() - last > interval)
                {
                    // get our absolute position
                    PlayList* pl = _scheduleManager->GetRunningPlayList();
                    if (pl != nullptr)
                    {
                        // sent a sync
                        auto ms = pl->GetPosition();
                        _syncMidi->SendSync(0, 0, 0, ms, "", "", "", "");
                        _toSendStop = true;
                    }
                    else
                    {
                        if (_toSendStop)
                        {
                            _syncMidi->SendStop();
                            _toSendStop = false;
                        }
                    }

                    sleepfor = (int64_t)((last.GetValue()) + interval - wxGetUTCTimeUSec().GetValue());
                    last = wxGetUTCTimeUSec().GetValue();
                }

                if (sleepfor > 0)
                {
                    wxMicroSleep(sleepfor);
                }
            }
        }
        _running = false;
        logger_base.info("MIDI Timecode thread stopped.");
        return nullptr;
    }
};

#ifdef USE_CLOCK_THREAD
class MIDIClockThread : public wxThread
{
    std::atomic<bool> _stop;
    SyncMIDI* _syncMidi = nullptr;
    std::atomic<bool> _running;
    std::atomic<bool> _suspend;
    ScheduleManager* _scheduleManager = nullptr;

public:
    MIDIClockThread(SyncMIDI* syncMIDI, ScheduleManager* scheduleManager)
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        _suspend = false;
        _running = false;
        _stop = false;
        _syncMidi = syncMIDI;
        _scheduleManager = scheduleManager;

        if (Run() != wxTHREAD_NO_ERROR)
        {
            logger_base.error("Failed to start MIDI Clock thread");
        }
        else
        {
            logger_base.info("MIDI Clock thread created.");
        }
    }
    virtual ~MIDIClockThread()
    {
        Stop();
    }

    void Stop()
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("MIDI Clock thread stopping.");
        _stop = true;
    }

    void UpdateSyncMIDI(SyncMIDI* syncMIDI)
    {
        _suspend = true;
        wxMilliSleep(100); // ensure it is suspended ... this is lazy ... i really should use sync objects
        _syncMidi = syncMIDI;
        _suspend = false;
    }

    void* Entry()
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        wxLongLong last = 0;
        double interval = 1000000.0 / 24.0; // 88 BPM, 24 frames per second
        logger_base.debug("Clock thread interval %gms", interval / 1000);
        _running = true;
        while (!_stop)
        {
            if (!_suspend)
            {
                long long sleepfor = 0;

                if (wxGetUTCTimeUSec() - last > interval)
                {
                    // get our absolute position
                    PlayList* pl = _scheduleManager->GetRunningPlayList();
                    if (pl != nullptr)
                    {
                        // sent a sync
                        auto ms = pl->GetPosition();
                        _syncMidi->SendClock();
                    }

                    sleepfor = (int64_t)((last.GetValue()) + interval - wxGetUTCTimeUSec().GetValue());
                    last = wxGetUTCTimeUSec().GetValue();
                }

                if (sleepfor > 0)
                {
                    wxMicroSleep(sleepfor);
                }
            }
        }
        _running = false;
        logger_base.info("MIDI Clock thread stopped.");
        return nullptr;
    }
};
#endif

SyncMIDI::SyncMIDI(SYNCMODE sm, REMOTEMODE rm, const ScheduleOptions& options, ListenerManager* listenerManager) : SyncBase(sm, rm)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _timeCodeDevice = options.GetMIDITimecodeDevice();
    _timeCodeFormat = options.GetMIDITimecodeFormat();
    _timeCodeOffset = options.GetMIDITimecodeOffset();

    if (sm == SYNCMODE::MIDIMASTER)
    {
        if (_timeCodeDevice != "")
        {
            _midi = new wxMidiOutDevice(wxAtoi(wxString(_timeCodeDevice).AfterLast(' ')));
            if (_midi->IsOutputPort())
            {
                wxMidiError err = _midi->Open(0);
                if (err != wxMIDI_NO_ERROR)
                {
                    delete _midi;
                    _midi = nullptr;
                    logger_base.error("MIDI failed to open as a timecode master: %d", err);
                }
                else
                {
                    logger_base.debug("MIDI opened as a timecode master");
                    // Start the sending thread
                    _threadTimecode = new MIDITimecodeThread(this, listenerManager->GetScheduleManager());
#ifdef USE_CLOCK_THREAD
                    _threadClock = new MIDIClockThread(this, listenerManager->GetScheduleManager());
#endif
                }
            }
            else
            {
                delete _midi;
                _midi = nullptr;
                logger_base.debug("Attempt to use input MIDI device as a timecode master. Device must be an output device.");
                wxMessageBox("Invalid MIDI device type for master mode.");
            }
        }
    }

    if (rm == REMOTEMODE::MIDISLAVE)
    {
        listenerManager->SetRemoteMIDI();
    }
}

SyncMIDI::SyncMIDI(SyncMIDI&& from) : SyncBase(from)
{
    _threadTimecode = from._threadTimecode;
    from._threadTimecode = nullptr; // this is a transfer of ownership
    if (_threadTimecode != nullptr) {
        _threadTimecode->UpdateSyncMIDI(this);
    }

#ifdef USE_CLOCK_THREAD
    _threadClock = from._threadClock;
    from._threadClock = nullptr; // this is a transfer of ownership
    if (_threadClock != nullptr) {
        _threadClock->UpdateSyncMIDI(this);
    }
#endif
    _midi = from._midi;
    from._midi = nullptr; // this is a transfer of ownership
    _timeCodeDevice = from._timeCodeDevice;
    _timeCodeFormat = from._timeCodeFormat;
    _timeCodeOffset = from._timeCodeOffset;
}

SyncMIDI::~SyncMIDI()
{
    // close the sending thread
    if (_threadTimecode != nullptr)
    {
        //logger_base.debug("MIDI Timecode stopping.");
        _threadTimecode->Stop();
        _threadTimecode->Delete();
        _threadTimecode = nullptr;
    }

#ifdef USE_CLOCK_THREAD
    // close the sending thread
    if (_threadClock != nullptr)
    {
        //logger_base.debug("MIDI Clock stopping.");
        _threadClock->Stop();
        _threadClock->Delete();
        _threadClock = nullptr;
    }
#endif

    // give evrything a chance to stop
    wxMilliSleep(50);

    if (_midi != nullptr) {
        _midi->Close();
        delete _midi;
        _midi = nullptr;
    }
}

double SyncMIDI::GetInterval()
{
    switch (_timeCodeFormat)
    {
    default:
    case TIMECODEFORMAT::F24: // 24 fps
        return 1000.0 / 24.0;
    case TIMECODEFORMAT::F25: // 25 fps
        return 1000.0 / 25.0;
    case TIMECODEFORMAT::F2997: // 29.97 fps
        return 1000.0 / 29.97;
    case TIMECODEFORMAT::F30: // 30 fps
        return 1000.0 / 30.0;
        break;
    }
    return 1000.0 / 25.0;
}

void SyncMIDI::SendClock() const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_midi == nullptr) return;

    wxMidiShortMessage msg(0xF8, 0, 0);
    msg.SetTimestamp(wxMidiSystem::GetInstance()->GetTime());
    logger_base.debug("MIDI Short Message 0x%02x Data 0x%02x 0x%02x Timestamp 0x%04x", msg.GetStatus(), msg.GetData1(), msg.GetData2(), (int)msg.GetTimestamp());
    {
#ifdef USE_CLOCK_THREAD
        std::unique_lock<std::mutex> lock(_mutex);
#endif
        _midi->Write(&msg);
    }
}

void SyncMIDI::SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static size_t lastmsec = 999999999;
    static bool firstquarterframe = true;

    if (_midi == nullptr) return;

    bool dosend = false;
    if (playlistMS == 0) dosend = true;

    if (!dosend)
    {
        //if (msec - lastmsec > 1000)
        {
            dosend = true;
        }
    }

    if (!dosend) return;

    bool sendresync = false;
    if (playlistMS - lastmsec < 0 || playlistMS - lastmsec > 1000)
    {
        sendresync = true;
    }

    if (lastmsec == 999999999)
    {
        wxMidiShortMessage msg(0xFA, 0, 0);
        msg.SetTimestamp(wxMidiSystem::GetInstance()->GetTime());
        logger_base.debug("MIDI Short Message 0x%02x Data 0x%02x 0x%02x Timestamp 0x%04x", msg.GetStatus(), msg.GetData1(), msg.GetData2(), (int)msg.GetTimestamp());

        {
#ifdef USE_CLOCK_THREAD
            std::unique_lock<std::mutex> lock(_mutex);
#endif
            _midi->Write(&msg);
        }
    }
    else if (playlistMS == 0xFFFFFFFF)
    {
        lastmsec = 999999999;
        wxMidiShortMessage msg(0xFC, 0, 0);
        msg.SetTimestamp(wxMidiSystem::GetInstance()->GetTime());
        logger_base.debug("MIDI Short Message 0x%02x Data 0x%02x 0x%02x Timestamp 0x%04x", msg.GetStatus(), msg.GetData1(), msg.GetData2(), (int)msg.GetTimestamp());
        {
#ifdef USE_CLOCK_THREAD
            std::unique_lock<std::mutex> lock(_mutex);
#endif
            _midi->Write(&msg);
        }
        return;
    }

    lastmsec = playlistMS;

    // send the packet

    if (sendresync)
    {
        uint8_t buffer[10];
        size_t ms = playlistMS;

        ms += _timeCodeOffset;

        buffer[0] = 0xF0;
        buffer[1] = 0x7F;
        buffer[2] = 0x7F;
        buffer[3] = 0x01;
        buffer[4] = 0x01;

        buffer[5] = (static_cast<int>(_timeCodeFormat) << 5) + ms / (3600000); // hour
        ms = ms % 360000;

        buffer[6] = ms / 60000; // minute
        ms = ms % 60000;

        buffer[7] = ms / 1000; // seconds
        ms = ms % 1000;

        switch (_timeCodeFormat)
        {
        default:
        case TIMECODEFORMAT::F24: // 24 fps
            buffer[8] = ms * 24 / 1000;
            break;
        case TIMECODEFORMAT::F25: // 25 fps
            buffer[8] = ms * 25 / 1000;
            break;
        case TIMECODEFORMAT::F2997: // 29.97 fps
            buffer[8] = ms * 2997 / 100000;
            break;
        case TIMECODEFORMAT::F30: // 30 fps
            buffer[8] = ms * 30 / 1000;
            break;
        }
        buffer[9] = 0xF7;
        {
#ifdef USE_CLOCK_THREAD
            std::unique_lock<std::mutex> lock(_mutex);
#endif
            _midi->Write(buffer, wxMidiSystem::GetInstance()->GetTime());
        }
        firstquarterframe = true;
    }
    else
    {
        size_t ms = playlistMS;
        ms += _timeCodeOffset;

        int hours = (static_cast<int>(_timeCodeFormat) << 5) + ms / (3600000);
        ms = ms % 360000;

        int mins = ms / 60000;
        ms = ms % 60000;

        int secs = ms / 1000;
        ms = ms % 1000;

        int frames;
        switch (_timeCodeFormat)
        {
        default:
        case TIMECODEFORMAT::F24: // 24 fps
            frames = ms * 24 / 1000;
            break;
        case TIMECODEFORMAT::F25: // 25 fps
            frames = ms * 25 / 1000;
            break;
        case TIMECODEFORMAT::F2997: // 29.97 fps
            frames = ms * 2997 / 100000;
            break;
        case TIMECODEFORMAT::F30: // 30 fps
            frames = ms * 30 / 1000;
            break;
        }

        logger_base.debug("%d:%02d:%02d.%02d", hours, mins, secs, frames);

        uint8_t data = 0;
        if (firstquarterframe)
        {
            for (int i = 0; i < 4; i++)
            {
                switch (i)
                {
                case 0:
                    data = frames & 0x0F;
                    break;
                case 1:
                    data = (frames & 0xF0) >> 4;
                    break;
                case 2:
                    data = secs & 0x0F;
                    break;
                case 3:
                    data = (secs & 0xF0) >> 4;
                    break;
                }
                data += (i << 4);
                wxMidiShortMessage msg(0xF1, data, 0);
                msg.SetTimestamp(wxMidiSystem::GetInstance()->GetTime());
                logger_base.debug("MIDI Short Message 0x%02x Data 0x%02x 0x%02x Timestamp 0x%04x", msg.GetStatus(), msg.GetData1(), msg.GetData2(), (int)msg.GetTimestamp());
                {
#ifdef USE_CLOCK_THREAD
                    std::unique_lock<std::mutex> lock(_mutex);
#endif
                    _midi->Write(&msg);
                }
            }
        }
        else
        {
            for (int i = 4; i < 8; i++)
            {
                switch (i)
                {
                case 4:
                    data = mins & 0x0F;
                    break;
                case 5:
                    data = (mins & 0xF0) >> 4;
                    break;
                case 6:
                    data = hours & 0x0F;
                    break;
                case 7:
                    data = ((hours & 0x10) >> 4) + GetTimeCodeBits();
                    break;
                }
                data += (i << 4);
                wxMidiShortMessage msg(0xF1, data, 0);
                msg.SetTimestamp(wxMidiSystem::GetInstance()->GetTime());
                logger_base.debug("MIDI Short Message 0x%02x Data 0x%02x 0x%02x Timestamp 0x%04x", msg.GetStatus(), msg.GetData1(), msg.GetData2(), (int)msg.GetTimestamp());
                {
#ifdef USE_CLOCK_THREAD
                    std::unique_lock<std::mutex> lock(_mutex);
#endif
                    _midi->Write(&msg);
                }
            }
        }
        firstquarterframe = !firstquarterframe;
#ifndef USE_CLOCK_THREAD
        SendClock();
#endif
    }
}

uint8_t SyncMIDI::GetTimeCodeBits() const
{
    switch (_timeCodeFormat)
    {
    default:
    case TIMECODEFORMAT::F24: // 24 fps
        return 0;
    case TIMECODEFORMAT::F25: // 25 fps
        return 2;
    case TIMECODEFORMAT::F2997: // 29.97 fps
        return 4;
    case TIMECODEFORMAT::F30: // 30 fps
        return 6;
        break;
    }
    return 2;
}

void SyncMIDI::SendStop() const
{
    SendSync(50, 0, 0, 0xFFFFFFFF, "", "", "", "");
}
