#include "SyncMIDI.h"
#include "ScheduleOptions.h"
#include "events/ListenerManager.h"
#include "wxMIDI/src/wxMidi.h"

#include <log4cpp/Category.hh>
#include "../xLights/UtilFunctions.h"

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
    _midi = from._midi;
    from._midi = nullptr; // this is a transfer of ownership
    _timeCodeDevice = from._timeCodeDevice;
    _timeCodeFormat = from._timeCodeFormat;
    _timeCodeOffset = from._timeCodeOffset;
}

SyncMIDI::~SyncMIDI()
{
    if (_midi != nullptr) {
        _midi->Close();
        delete _midi;
        _midi = nullptr;
    }
}

void SyncMIDI::SendSync(uint32_t frameMS, uint32_t stepLengthMS, uint32_t stepMS, uint32_t playlistMS, const std::string& fseq, const std::string& media, const std::string& step, const std::string& timeItem) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static size_t lastmsec = 999999999;

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
    if (playlistMS - lastmsec < 0 || playlistMS - lastmsec > 5000)
    {
        sendresync = true;
    }

    if (lastmsec == 999999999)
    {
        wxMidiShortMessage msg(0xFA, 0, 0);
        msg.SetTimestamp(wxMidiSystem::GetInstance()->GetTime());
        logger_base.debug("MIDI Short Message 0x%02x Data 0x%02x 0x%02x Timestamp 0x%04x", msg.GetStatus(), msg.GetData1(), msg.GetData2(), (int)msg.GetTimestamp());
        _midi->Write(&msg);
    }
    else if (playlistMS == 0xFFFFFFFF)
    {
        lastmsec = 999999999;
        wxMidiShortMessage msg(0xFC, 0, 0);
        msg.SetTimestamp(wxMidiSystem::GetInstance()->GetTime());
        logger_base.debug("MIDI Short Message 0x%02x Data 0x%02x 0x%02x Timestamp 0x%04x", msg.GetStatus(), msg.GetData1(), msg.GetData2(), (int)msg.GetTimestamp());
        _midi->Write(&msg);
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

        buffer[5] = (static_cast<int>(_timeCodeFormat) << 5) + ms / (3600000);
        ms = ms % 360000;

        buffer[6] = ms / 60000;
        ms = ms % 60000;

        buffer[7] = ms / 1000;
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
        _midi->Write(buffer);
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

        for (int i = 0; i < 8; i++)
        {
            int data = 0;
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
                data = (hours & 0xF0) >> 4;
                break;
            default:
                break;
            }
            wxMidiShortMessage msg(0xF1, (i << 4) + data, 0);
            msg.SetTimestamp(wxMidiSystem::GetInstance()->GetTime());
            logger_base.debug("MIDI Short Message 0x%02x Data 0x%02x 0x%02x Timestamp 0x%04x", msg.GetStatus(), msg.GetData1(), msg.GetData2(), (int)msg.GetTimestamp());
            _midi->Write(&msg);
        }
        wxMidiShortMessage msg(0xF8, 0, 0);
        msg.SetTimestamp(wxMidiSystem::GetInstance()->GetTime());
        logger_base.debug("MIDI Short Message 0x%02x Data 0x%02x 0x%02x Timestamp 0x%04x", msg.GetStatus(), msg.GetData1(), msg.GetData2(), (int)msg.GetTimestamp());
        _midi->Write(&msg);
    }
}

void SyncMIDI::SendStop() const
{
    SendSync(50, 0, 0, 0xFFFFFFFF, "", "", "", "");
}
