#include "ListenerMIDI.h"
#include <log4cpp/Category.hh>
#include "ListenerManager.h"
#include "../wxMIDI/src/wxMidi.h"

ListenerMIDI::ListenerMIDI(int deviceId, ListenerManager* listenerManager) : ListenerBase(listenerManager)
{
    _deviceId = deviceId;
    _frameMS = 50;
    _midiIn = nullptr;
}

void ListenerMIDI::Start()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("MIDI listener starting.");
    _thread = new ListenerThread(this);
}

void ListenerMIDI::Stop()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("MIDI listener stopping.");
    if (_thread != nullptr)
    {
        _stop = true;
        _thread->Stop();
    }
}

void ListenerMIDI::StartProcess()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // We set timeout to one second so it is responsive when we go to shut it down
    _midiIn = new wxMidiInDevice(_deviceId, 1);
    if (_midiIn != nullptr)
    {
        wxASSERT(_midiIn->IsInputPort());
        if (_midiIn->Open() != wxMIDI_NO_ERROR)
        {
            delete _midiIn;
            _midiIn = nullptr;
        }
        else
        {
            _isOk = true;
        }
    }
}

void ListenerMIDI::StopProcess()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_midiIn != nullptr)
    {
        _midiIn->Close();
        delete _midiIn;
        _midiIn = nullptr;
    }
    _isOk = false;
}

void ListenerMIDI::Poll()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_midiIn == nullptr || _stop) return;

    wxMidiError error;
    wxMidiMessage* message = _midiIn->Read(&error);

    if (error == wxMIDI_NO_ERROR && message != nullptr && !_stop)
    {
        if (message->GetType() == wxMIDI_SHORT_MSG)
        {
            wxMidiShortMessage* msg = (wxMidiShortMessage*)message;
            logger_base.debug("MIDI Short Message 0x%02x Data 0x%02x 0x%02x", msg->GetStatus(), msg->GetData1(), msg->GetData2());
            int status = msg->GetStatus();
            if (status >= 0x80 && status <= 0x8F)
            {
                logger_base.debug("    Note Off");
            }
            else if (status >= 0x90 && status <= 0x9F)
            {
                logger_base.debug("    Note On");
            }
            else if (status >= 0xA0 && status <= 0xAF)
            {
                logger_base.debug("    Polyphonic Key Pressure");
            }
            else if (status >= 0xB0 && status <= 0xBF)
            {
                logger_base.debug("    Control Change");
            }
            else if (status >= 0xC0 && status <= 0xCF)
            {
                logger_base.debug("    Program Change");
            }
            else if (status >= 0xD0 && status <= 0xDF)
            {
                logger_base.debug("    Channel Pressure");
            }
            else if (status >= 0xE0 && status <= 0xEF)
            {
                logger_base.debug("    Pitch Bend");
            }
            _listenerManager->ProcessPacket(GetType(), msg->GetStatus() & 0xF0, msg->GetStatus() & 0x0F, msg->GetData1(), msg->GetData2());
        }
        else
        {
            wxMidiSysExMessage* msg = (wxMidiSysExMessage*)message;
            logger_base.debug("MIDI SysEx Message 0x%02x", msg->GetStatus());
            switch(msg->GetStatus())
            {
            case 0xF0: // Begin System Exclusive	0xF0
            case 0xF1: // MIDI Time Code	0xF1
                break;
            case 0xF2: // Song Position Pointer	0xF2
            case 0xF3: // Song Select	0xF3
            case 0xF4: // Tune Request	0xF6
            case 0xF7: // End System Exclusive	0xF7
            case 0xF8: // Real - time Clock	0xF8
            case 0xF9: // Undefined	0xF9
            case 0xFA: // Start	0xFA
            case 0xFB: // Continue	0xFB
            case 0xFC: // Stop	0xFC
            case 0xFD: // Undefined	0xFD
            case 0xFE: // Active Sensing	0xFE
            case 0xFF: // System Reset	0xFF
                break;
            default:
                wxASSERT(false);
                break;
            }
        }
    }
}
