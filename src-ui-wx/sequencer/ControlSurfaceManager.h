#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/timer.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "utils/MCUControlSurface.h"

class xLightsFrame;
class RtMidiIn;
class RtMidiOut;

// Connects a Mackie Control (MCU) surface over MIDI to the sequencer.
// Incoming MIDI arrives on RtMidi's thread and is queued; a UI-thread timer
// drains it and pushes transport/timecode feedback back to the surface.
class ControlSurfaceManager : public wxEvtHandler {
public:
    // Output port setting meaning "use the port with the input's name".
    static const std::string SAME_AS_INPUT;
    static const std::string NO_OUTPUT;

    explicit ControlSurfaceManager(xLightsFrame* frame);
    ~ControlSurfaceManager() override;

    void LoadSettings();
    void Configure(bool enabled, const std::string& inPort, const std::string& outPort);

    bool IsEnabled() const { return _enabled; }
    const std::string& GetInputPort() const { return _inPort; }
    const std::string& GetOutputPort() const { return _outPort; }
    bool IsConnected() const { return _in != nullptr; }

    static std::vector<std::string> ListInputPorts();
    static std::vector<std::string> ListOutputPorts();

private:
    void Connect();
    void Disconnect();
    void OnTimer(wxTimerEvent& event);
    void Send(const std::vector<uint8_t>& msg);
    static void OnMidiMessage(double timeStamp, std::vector<unsigned char>* message, void* userData);

    xLightsFrame* _frame;
    wxTimer _timer;
    std::unique_ptr<RtMidiIn> _in;
    std::unique_ptr<RtMidiOut> _out;
    std::mutex _incomingLock;
    std::vector<std::vector<uint8_t>> _incoming;
    mcu::ControlSurface _surface;

    bool _enabled = false;
    std::string _inPort;
    std::string _outPort = SAME_AS_INPUT;
    int _reconnectTicks = 0;
};
