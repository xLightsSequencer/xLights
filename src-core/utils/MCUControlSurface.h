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

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

// Mackie Control Universal (MCU) protocol: turns raw MIDI from a control
// surface into sequencer actions, and sequencer state into the LED and
// timecode-display messages sent back. Transport-agnostic - the host owns the
// MIDI ports and feeds bytes in / sends the queued bytes out.
namespace mcu {

// Note numbers of the MCU buttons handled here (MIDI channel 1).
enum class Button : uint8_t {
    Marker = 0x54,
    Cycle = 0x56,
    Rewind = 0x5B,
    FastForward = 0x5C,
    Stop = 0x5D,
    Play = 0x5E,
    Up = 0x60,
    Down = 0x61,
    Left = 0x62,
    Right = 0x63,
    Zoom = 0x64,
    Scrub = 0x65,
    SMPTE = 0x71
};

enum class Command {
    Play,
    Stop,
    Rewind,
    FastForward,
    Loop,
    AddTimingMark,
    ZoomIn,
    ZoomOut,
    Up,
    Down,
    Left,
    Right,
    Jog
};

struct Action {
    Command command;
    int ticks = 0;      // Jog only: positive = clockwise
    bool scrub = false; // Jog only: Scrub mode is on
};

enum class TransportState {
    Stopped,
    Playing,
    Paused
};

struct Status {
    bool sequenceOpen = false;
    TransportState transport = TransportState::Stopped;
    bool looping = false;
    int timeMS = 0;
};

// 10 characters for the MCU timecode display, left to right: hours (3,
// blank when zero), minutes (2), seconds (2), milliseconds (3).
std::string FormatTimecode(int ms);

class ControlSurface {
public:
    std::vector<Action> ProcessMessage(const uint8_t* data, size_t len);

    // Queues LED / display messages for whatever differs from what the
    // surface was last sent.
    void Update(const Status& status);

    // Forget what the surface is showing so the next Update resends it all,
    // e.g. after the output port is (re)opened.
    void Reset();

    // Queues messages that turn off every LED and blank the display.
    void Clear();

    std::vector<std::vector<uint8_t>> TakeOutgoing();

    bool IsZoomMode() const { return _zoomMode; }
    bool IsScrubMode() const { return _scrubMode; }

private:
    void SetLed(Button b, uint8_t state);
    void SetDigit(int index, uint8_t value);

    bool _zoomMode = false;
    bool _scrubMode = false;
    std::map<uint8_t, uint8_t> _leds;
    std::array<int, 10> _digits{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
    std::vector<std::vector<uint8_t>> _outgoing;
};

} // namespace mcu
