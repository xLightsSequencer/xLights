#include "MIDIListener.h"
#include <log4cpp/Category.hh>
#include "../xSchedule/wxMIDI/src/wxMidi.h"

wxDEFINE_EVENT(EVT_MIDI, wxCommandEvent);

MIDIListener::MIDIListener(int deviceId, wxWindow* win)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _thread = nullptr;

    if (deviceId >= 0)
    {
        _thread = new ListenerThread(deviceId, win);
        wxMilliSleep(20);
        if (!_thread->IsOk())
        {
            logger_base.error("MIDI listening thread failed.");
            _thread = nullptr;
        }
    }
}

void MIDIListener::Stop()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("MIDI listener stopping.");
    if (_thread != nullptr)
    {
        _thread->Stop();
        //_thread->Wait();
        //delete _thread;
        wxMilliSleep(10);
        _thread = nullptr;
    }
}

void MIDIListener::ClearTempWindow()
{
    if (_thread != nullptr) _thread->ClearTempWindow();
}

void MIDIListener::SetTempWindow(wxWindow* win)
{
    if (_thread != nullptr) _thread->SetTempWindow(win); 
}

bool MIDIListener::IsOk() const
{
    return _thread != nullptr && _thread->IsOk();
}

void MIDIListener::SetDeviceId(int id, wxWindow* win)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (id < 0)
    {
        Stop();
    }
    else
    {
        if (_thread != nullptr && id != _thread->GetDeviceId())
        {
            Stop();
        }
        _thread = new ListenerThread(id, win);
        wxMilliSleep(20);
        if (!_thread->IsOk())
        {
            logger_base.error("MIDI listening thread failed.");
            _thread = nullptr;
        }
    }
}

ListenerThread::ListenerThread(int deviceId, wxWindow* win)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	_target = win;
    _stop = false;
    _running = false;
	_deviceId = deviceId;
	_isOk = false;
	
    if (Run() != wxTHREAD_NO_ERROR)
    {
        logger_base.error("Failed to start MIDI listener thread");
    }
    else
    {
        logger_base.info("MIDI Listener thread created.");
        wxMilliSleep(10);
        _isOk = _running;
        if (!_isOk)
        {
            logger_base.error("    But it seems to have immediately exited.");
        }
    }
}

void* ListenerThread::Entry()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _running = true;
    logger_base.info("MIDI Listener thread running.");

	_midiIn = nullptr;

	    // We set timeout to one second so it is responsive when we go to shut it down
	_midiIn = new wxMidiInDevice(_deviceId, 1);
    if (_midiIn != nullptr)
    {
        wxASSERT(_midiIn->IsInputPort());
        if (_midiIn->Open() != wxMIDI_NO_ERROR)
        {
            logger_base.error("    Failed to open MIDI.");
            delete _midiIn;
            _midiIn = nullptr;
            _running = false;
            return nullptr;
        }
        else
        {
            _isOk = true;
        }
    }

    while (!_stop)
    {
        wxMidiError error;
        wxMidiMessage* message = _midiIn->Read(&error);

        if (error == wxMIDI_NO_ERROR && message != nullptr)
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

                if (_target != nullptr)
                {
                    wxCommandEvent event(EVT_MIDI);
                    int value = (((int)msg->GetStatus() & 0xF0) << 24) + 
                                (((int)msg->GetStatus() & 0x0F) << 16) + 
                                ((int)msg->GetData1() << 8) + 
                                (int)msg->GetData2();
                    event.SetInt(value);
                    wxPostEvent(_target, event);
                }
            }
            else
            {
                wxMidiSysExMessage* msg = (wxMidiSysExMessage*)message;
                //logger_base.debug("MIDI SysEx Message 0x%02x", msg->GetStatus());
                switch (msg->GetStatus())
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

    _midiIn->Close();
    delete _midiIn;
    _midiIn = nullptr;

    _running = false;

    logger_base.debug("MIDI listening thread exiting.");

    return nullptr;
}
