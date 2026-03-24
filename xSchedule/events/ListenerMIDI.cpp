/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ListenerMIDI.h"
#include "ListenerManager.h"
#include "../ScheduleManager.h"
#include "../ScheduleOptions.h"
#include "../wxMIDI/src/wxMidi.h"
#include "../xScheduleMain.h"
#include <log.h>

ListenerMIDI::ListenerMIDI(int deviceId, ListenerManager* listenerManager) :
    ListenerBase(listenerManager, "") {
    _deviceId = deviceId;
    _frameMS = 50;
    _midiIn = nullptr;
}

void ListenerMIDI::Start() {
    spdlog::debug("MIDI listener starting.");
    _thread = new ListenerThread(this, _localIP);
}

void ListenerMIDI::Stop() {
    if (!_stop) {
        spdlog::debug("MIDI listener stopping.");
        if (_thread != nullptr) {
            _stop = true;
            _thread->Stop();
            _thread->Delete();
            delete _thread;
            _thread = nullptr;
        }
    }
}

void ListenerMIDI::StartProcess(const std::string& localIP) {
    // We set timeout to one second so it is responsive when we go to shut it down
    _midiIn = new wxMidiInDevice(_deviceId, 1);
    if (_midiIn != nullptr) {
        if (_midiIn->IsInputPort()) {
            if (_midiIn->Open() != wxMIDI_NO_ERROR) {
                spdlog::error("ListenerMIDI Failed to open MIDI port {}.", _deviceId);
                delete _midiIn;
                _midiIn = nullptr;
            } else {
                spdlog::debug("ListenerMIDI MIDI port {} opened.", _deviceId);
                _isOk = true;
            }
        } else {
            spdlog::error("ListenerMIDI Attempt to read from a write MIDI port {}.", _deviceId);
            delete _midiIn;
            _midiIn = nullptr;
        }
    }
}

void ListenerMIDI::StopProcess() {
    if (_midiIn != nullptr) {
        spdlog::debug("ListenerMIDI Closing MIDI port {}.", _deviceId);
        _midiIn->Close();
        delete _midiIn;
        _midiIn = nullptr;
    }
    _isOk = false;
}

void ListenerMIDI::Poll() {
    if (_midiIn == nullptr || _stop)
        return;

    wxMidiError error;
    wxMidiMessage* message = _midiIn->Read(&error);
    while (error == wxMIDI_NO_ERROR && message != nullptr && !_stop) {
        if (message->GetType() == wxMIDI_SHORT_MSG) {
            wxMidiShortMessage* msg = (wxMidiShortMessage*)message;
            int status = msg->GetStatus();
            if (status >= 0x80 && status <= 0x8F) {
                spdlog::debug("MIDI Short Message {:02x} Data {:02x} {:02x} Timestamp {:04x}", msg->GetStatus(), msg->GetData1(), msg->GetData2(), (int)msg->GetTimestamp());
                spdlog::debug("    Note Off");
            } else if (status >= 0x90 && status <= 0x9F) {
                spdlog::debug("MIDI Short Message {:02x} Data {:02x} {:02x} Timestamp {:04x}", msg->GetStatus(), msg->GetData1(), msg->GetData2(), (int)msg->GetTimestamp());
                spdlog::debug("    Note On");
            } else if (status >= 0xA0 && status <= 0xAF) {
                spdlog::debug("    Polyphonic Key Pressure");
            } else if (status >= 0xB0 && status <= 0xBF) {
                spdlog::debug("    Control Change");
            } else if (status >= 0xC0 && status <= 0xCF) {
                spdlog::debug("    Program Change");
            } else if (status >= 0xD0 && status <= 0xDF) {
                spdlog::debug("    Channel Pressure");
            } else if (status >= 0xE0 && status <= 0xEF) {
                spdlog::debug("    Pitch Bend");
            } else if (status == 0xF8) {
                // Real - time Clock	0xF8
            } else if (status == 0xFa) {
                // start
                _listenerManager->Sync("", 0xFFFFFFFD, GetType());
            } else if (status == 0xFc) {
                // stop
                _listenerManager->Sync("", 0xFFFFFFFE, GetType());
            } else if (status == 0xF1) {
                // MIDI Time Code	0xF1
                // quarter frame messages
                static int qfhours = 0;
                static int qfmins = 0;
                static int qfsecs = 0;
                static int qfframes = 0;
                static int qfmode = 0;
                int bit = (msg->GetData1() & 0xF0) >> 4;
                int data = (msg->GetData1() & 0x0F);
                switch (bit) {
                case 0:
                    qfframes = (qfframes & 0xf0) + data;
                    break;
                case 1:
                    qfframes = (qfframes & 0x0f) + (data << 4);
                    break;
                case 2:
                    qfsecs = (qfsecs & 0xf0) + data;
                    break;
                case 3:
                    qfsecs = (qfsecs & 0x0f) + (data << 4);
                    break;
                case 4:
                    qfmins = (qfmins & 0xf0) + data;
                    break;
                case 5:
                    qfmins = (qfmins & 0x0f) + (data << 4);
                    break;
                case 6:
                    qfhours = (qfhours & 0xf0) + data;
                    break;
                case 7:
                    qfhours = (qfhours & 0x0f) + ((msg->GetData1() & 0x01) << 4);
                    qfmode = (msg->GetData1() & 0x06) >> 1;
                    _lastMode = qfmode;
                    DoSync(qfmode, qfhours, qfmins, qfsecs, qfframes);
                    break;
                default:
                    break;
                }
            }
            _listenerManager->ProcessPacket(GetType(), GetDeviceId(), msg->GetStatus() & 0xF0, msg->GetStatus() & 0x0F, msg->GetData1(), msg->GetData2());
        } else {
            wxMidiSysExMessage* msg = (wxMidiSysExMessage*)message;
            spdlog::debug("MIDI SysEx Message {:02x}", msg->GetStatus());
            switch (msg->GetStatus()) {
            case 0xF0: // Begin System Exclusive	0xF0
            {
                uint8_t* buffer = msg->GetMessage();
                if (buffer[3] == 0x01 && buffer[4] == 0x01) {
                    // F0 7F 7F 01 01 hh mm ss ff F7
                    int hours = buffer[5] & 0x1F;
                    int mode = (buffer[5] & 0x60) >> 5;
                    int mins = buffer[6];
                    int secs = buffer[7];
                    int frames = buffer[8];

                    DoSync(mode, hours, mins, secs, frames);
                }
            } break;
            case 0xF1: // MIDI Time Code	0xF1
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
        message = _midiIn->Read(&error);
    }
    wxMilliSleep(10);
}

void ListenerMIDI::DoSync(int mode, int hours, int mins, int secs, int frames) {
    static long lastms = -99999;

    long stepoffset = _listenerManager->GetStepMMSSOfset(hours, _listenerManager->GetScheduleManager()->GetOptions()->GetMIDITimecodeOffset() / 3600000);

    long ms = ((hours * 60 + mins) * 60 + secs) * 1000 + stepoffset;

    switch (mode) {
    default:
    case 0:
        // 24 fps
        ms += frames * 1000 / 24;
        break;
    case 1:
        // 25 fps
        ms += frames * 1000 / 25;
        break;
    case 2:
        // 29.97 fps
        ms += frames * 100000 / 2997;
        break;
    case 3:
        // 30 fps
        ms += frames * 1000 / 30;
        break;
    }

    ms -= _listenerManager->GetScheduleManager()->GetOptions()->GetMIDITimecodeOffset();

    if (ms - lastms > 10000 || ms < lastms) {
        spdlog::debug("MIDI DoSync MS: {}, Mode: {}, hours: {}, Mins: {}, Sec: {}, Frames: {}.", ms, mode, hours, mins, secs, frames);
        lastms = ms;
    }

    _listenerManager->Sync("", ms, GetType());
}
