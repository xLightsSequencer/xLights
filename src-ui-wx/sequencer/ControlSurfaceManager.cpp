/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControlSurfaceManager.h"

#include "../../dependencies/rtmidi/RtMidi.h"

#include "settings/XLightsConfigAdapter.h"
#include "xLightsMain.h"

#include <spdlog/spdlog.h>

const std::string ControlSurfaceManager::SAME_AS_INPUT = "";
const std::string ControlSurfaceManager::NO_OUTPUT = "<none>";

namespace {
constexpr int POLL_MS = 20;
constexpr int RECONNECT_TICKS = 3000 / POLL_MS;

void OnMidiError(RtMidiError::Type type, const std::string& errorText, void*) {
    if (type == RtMidiError::WARNING || type == RtMidiError::DEBUG_WARNING) {
        spdlog::debug("Control surface MIDI: {}", errorText);
    } else {
        spdlog::warn("Control surface MIDI: {}", errorText);
    }
}

// WinMM port names carry a trailing " <index>" that changes when devices are
// replugged, so names are also compared with it removed.
std::string StripPortIndex(const std::string& name) {
    auto pos = name.find_last_of(' ');
    if (pos == std::string::npos || pos + 1 == name.size()) {
        return name;
    }
    for (size_t i = pos + 1; i < name.size(); ++i) {
        if (name[i] < '0' || name[i] > '9') {
            return name;
        }
    }
    return name.substr(0, pos);
}

int FindPort(RtMidi& midi, const std::string& name) {
    unsigned int count = midi.getPortCount();
    for (unsigned int i = 0; i < count; ++i) {
        if (midi.getPortName(i) == name) {
            return (int)i;
        }
    }
    std::string base = StripPortIndex(name);
    for (unsigned int i = 0; i < count; ++i) {
        if (StripPortIndex(midi.getPortName(i)) == base) {
            return (int)i;
        }
    }
    return -1;
}

template<typename T>
std::vector<std::string> ListPorts() {
    std::vector<std::string> res;
    try {
        T midi;
        midi.setErrorCallback(&OnMidiError, nullptr);
        unsigned int count = midi.getPortCount();
        for (unsigned int i = 0; i < count; ++i) {
            std::string name = midi.getPortName(i);
            if (!name.empty()) {
                res.push_back(name);
            }
        }
    } catch (RtMidiError& e) {
        spdlog::warn("Control surface MIDI: unable to list ports: {}", e.getMessage());
    }
    return res;
}
} // namespace

ControlSurfaceManager::ControlSurfaceManager(xLightsFrame* frame) :
    _frame(frame), _timer(this) {
    Bind(wxEVT_TIMER, &ControlSurfaceManager::OnTimer, this);
}

ControlSurfaceManager::~ControlSurfaceManager() {
    _timer.Stop();
    Disconnect();
}

std::vector<std::string> ControlSurfaceManager::ListInputPorts() {
    return ListPorts<RtMidiIn>();
}

std::vector<std::string> ControlSurfaceManager::ListOutputPorts() {
    return ListPorts<RtMidiOut>();
}

void ControlSurfaceManager::LoadSettings() {
    auto* config = GetXLightsConfig();
    bool enabled = false;
    std::string inPort;
    std::string outPort = SAME_AS_INPUT;
    if (config != nullptr) {
        config->Read("xLightsControlSurfaceEnabled", &enabled, false);
        config->Read("xLightsControlSurfaceInPort", &inPort, std::string());
        config->Read("xLightsControlSurfaceOutPort", &outPort, SAME_AS_INPUT);
    }
    _enabled = enabled;
    _inPort = inPort;
    _outPort = outPort;
    Disconnect();
    if (_enabled) {
        Connect();
        _timer.Start(POLL_MS);
    }
}

void ControlSurfaceManager::Configure(bool enabled, const std::string& inPort, const std::string& outPort) {
    if (enabled == _enabled && inPort == _inPort && outPort == _outPort && (IsConnected() || !enabled)) {
        return;
    }
    auto* config = GetXLightsConfig();
    if (config != nullptr) {
        config->Write("xLightsControlSurfaceEnabled", enabled);
        config->Write("xLightsControlSurfaceInPort", inPort);
        config->Write("xLightsControlSurfaceOutPort", outPort);
    }
    _enabled = enabled;
    _inPort = inPort;
    _outPort = outPort;
    Disconnect();
    if (_enabled) {
        Connect();
        _timer.Start(POLL_MS);
    } else {
        _timer.Stop();
    }
}

void ControlSurfaceManager::Connect() {
    _reconnectTicks = RECONNECT_TICKS;
    if (_inPort.empty()) {
        return;
    }
    try {
        auto in = std::make_unique<RtMidiIn>();
        in->setErrorCallback(&OnMidiError, nullptr);
        int port = FindPort(*in, _inPort);
        if (port < 0) {
            return;
        }
        in->setCallback(&ControlSurfaceManager::OnMidiMessage, this);
        in->ignoreTypes(true, true, true);
        in->openPort(port, "xLights Control Surface");
        if (!in->isPortOpen()) {
            return;
        }
        spdlog::info("Control surface connected on MIDI input '{}'.", in->getPortName(port));
        _in = std::move(in);

        if (_outPort != NO_OUTPUT) {
            auto out = std::make_unique<RtMidiOut>();
            out->setErrorCallback(&OnMidiError, nullptr);
            int outIdx = FindPort(*out, _outPort == SAME_AS_INPUT ? _inPort : _outPort);
            if (outIdx >= 0) {
                out->openPort(outIdx, "xLights Control Surface");
                if (out->isPortOpen()) {
                    spdlog::info("Control surface feedback on MIDI output '{}'.", out->getPortName(outIdx));
                    _out = std::move(out);
                    _surface.Reset();
                }
            }
        }
    } catch (RtMidiError& e) {
        spdlog::warn("Control surface MIDI: unable to open ports: {}", e.getMessage());
        Disconnect();
    }
}

void ControlSurfaceManager::Disconnect() {
    try {
        if (_out != nullptr) {
            _surface.Clear();
            for (const auto& msg : _surface.TakeOutgoing()) {
                Send(msg);
            }
            _out->closePort();
        }
        if (_in != nullptr) {
            _in->cancelCallback();
            _in->closePort();
        }
    } catch (RtMidiError& e) {
        spdlog::warn("Control surface MIDI: error closing ports: {}", e.getMessage());
    }
    _out.reset();
    _in.reset();
    std::lock_guard<std::mutex> lock(_incomingLock);
    _incoming.clear();
}

void ControlSurfaceManager::OnMidiMessage(double, std::vector<unsigned char>* message, void* userData) {
    if (message == nullptr || message->empty()) {
        return;
    }
    auto* self = static_cast<ControlSurfaceManager*>(userData);
    std::lock_guard<std::mutex> lock(self->_incomingLock);
    self->_incoming.emplace_back(message->begin(), message->end());
}

void ControlSurfaceManager::Send(const std::vector<uint8_t>& msg) {
    if (_out == nullptr) {
        return;
    }
    try {
        _out->sendMessage(msg.data(), msg.size());
    } catch (RtMidiError& e) {
        spdlog::warn("Control surface MIDI: send failed: {}", e.getMessage());
    }
}

void ControlSurfaceManager::OnTimer(wxTimerEvent&) {
    if (!_enabled) {
        return;
    }
    if (_in == nullptr) {
        // The surface may be plugged in after xLights starts.
        if (--_reconnectTicks <= 0) {
            Connect();
        }
        return;
    }

    std::vector<std::vector<uint8_t>> incoming;
    {
        std::lock_guard<std::mutex> lock(_incomingLock);
        incoming.swap(_incoming);
    }
    // Jog ticks are summed so a fast spin moves once per poll from a single
    // known position rather than many times from a stale one.
    mcu::Action jog{ mcu::Command::Jog };
    for (const auto& msg : incoming) {
        for (const auto& action : _surface.ProcessMessage(msg.data(), msg.size())) {
            if (action.command == mcu::Command::Jog) {
                jog.ticks += action.ticks;
                jog.scrub = action.scrub;
            } else {
                _frame->ExecuteControlSurfaceAction(action);
            }
        }
    }
    if (jog.ticks != 0) {
        _frame->ExecuteControlSurfaceAction(jog);
    }

    if (_out != nullptr) {
        _surface.Update(_frame->GetControlSurfaceStatus());
    }
    for (const auto& msg : _surface.TakeOutgoing()) {
        Send(msg);
    }
}
