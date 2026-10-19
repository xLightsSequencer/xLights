/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MCUControlSurface.h"

#include <cstdio>

namespace mcu {

namespace {
constexpr uint8_t NOTE_ON = 0x90;
constexpr uint8_t CONTROL_CHANGE = 0xB0;
constexpr uint8_t CC_JOG = 0x3C;
// CC 0x40 is the rightmost timecode digit, 0x49 the leftmost.
constexpr uint8_t CC_TIMECODE_FIRST = 0x40;

constexpr uint8_t LED_OFF = 0x00;
constexpr uint8_t LED_FLASH = 0x01;
constexpr uint8_t LED_ON = 0x7F;

constexpr uint8_t DIGIT_DOT = 0x40;

constexpr Button kLedButtons[] = { Button::Play, Button::Stop, Button::Cycle, Button::Zoom, Button::Scrub, Button::SMPTE };
} // namespace

std::string FormatTimecode(int ms) {
    if (ms < 0) {
        ms = 0;
    }
    int hours = ms / 3600000;
    int minutes = (ms / 60000) % 60;
    int seconds = (ms / 1000) % 60;
    int millis = ms % 1000;

    char buf[16];
    if (hours == 0) {
        snprintf(buf, sizeof(buf), "   %02d%02d%03d", minutes, seconds, millis);
    } else {
        snprintf(buf, sizeof(buf), "%3d%02d%02d%03d", hours % 1000, minutes, seconds, millis);
    }
    return std::string(buf, 10);
}

std::vector<Action> ControlSurface::ProcessMessage(const uint8_t* data, size_t len) {
    std::vector<Action> actions;
    if (data == nullptr || len < 3) {
        return actions;
    }
    uint8_t status = data[0] & 0xF0;
    uint8_t d1 = data[1];
    uint8_t d2 = data[2];

    if (status == CONTROL_CHANGE && d1 == CC_JOG) {
        int ticks = (d2 & 0x40) ? -(int)(d2 & 0x3F) : (int)(d2 & 0x3F);
        if (ticks == 0) {
            return actions;
        }
        if (_zoomMode) {
            actions.push_back({ ticks > 0 ? Command::ZoomIn : Command::ZoomOut });
        } else {
            actions.push_back({ Command::Jog, ticks, _scrubMode });
        }
        return actions;
    }

    // Surfaces report a release as either Note Off or Note On with velocity 0;
    // only presses act.
    if (status != NOTE_ON || d2 == 0) {
        return actions;
    }

    switch ((Button)d1) {
    case Button::Play:
        actions.push_back({ Command::Play });
        break;
    case Button::Stop:
        actions.push_back({ Command::Stop });
        break;
    case Button::Rewind:
        actions.push_back({ Command::Rewind });
        break;
    case Button::FastForward:
        actions.push_back({ Command::FastForward });
        break;
    case Button::Cycle:
        actions.push_back({ Command::Loop });
        break;
    case Button::Marker:
        actions.push_back({ Command::AddTimingMark });
        break;
    case Button::Up:
        actions.push_back({ Command::Up });
        break;
    case Button::Down:
        actions.push_back({ Command::Down });
        break;
    case Button::Left:
        actions.push_back({ _zoomMode ? Command::ZoomOut : Command::Left });
        break;
    case Button::Right:
        actions.push_back({ _zoomMode ? Command::ZoomIn : Command::Right });
        break;
    case Button::Zoom:
        _zoomMode = !_zoomMode;
        SetLed(Button::Zoom, _zoomMode ? LED_ON : LED_OFF);
        break;
    case Button::Scrub:
        _scrubMode = !_scrubMode;
        SetLed(Button::Scrub, _scrubMode ? LED_ON : LED_OFF);
        break;
    default:
        break;
    }
    return actions;
}

void ControlSurface::Update(const Status& status) {
    uint8_t play = LED_OFF;
    if (status.transport == TransportState::Playing) {
        play = LED_ON;
    } else if (status.transport == TransportState::Paused) {
        play = LED_FLASH;
    }
    SetLed(Button::Play, play);
    SetLed(Button::Stop, (status.sequenceOpen && status.transport == TransportState::Stopped) ? LED_ON : LED_OFF);
    SetLed(Button::Cycle, (status.looping && status.transport != TransportState::Stopped) ? LED_ON : LED_OFF);
    SetLed(Button::Zoom, _zoomMode ? LED_ON : LED_OFF);
    SetLed(Button::Scrub, _scrubMode ? LED_ON : LED_OFF);
    SetLed(Button::SMPTE, status.sequenceOpen ? LED_ON : LED_OFF);

    if (!status.sequenceOpen) {
        for (int i = 0; i < 10; ++i) {
            SetDigit(i, ' ');
        }
        return;
    }

    std::string tc = FormatTimecode(status.timeMS);
    for (int col = 0; col < 10; ++col) {
        uint8_t value = (uint8_t)tc[col] & 0x3F;
        // Separator dots after the hours (when shown), minutes and seconds.
        if ((col == 2 && tc[col] != ' ') || col == 4 || col == 6) {
            value |= DIGIT_DOT;
        }
        SetDigit(9 - col, value);
    }
}

void ControlSurface::Reset() {
    _leds.clear();
    _digits.fill(-1);
}

void ControlSurface::Clear() {
    for (auto b : kLedButtons) {
        SetLed(b, LED_OFF);
    }
    for (int i = 0; i < 10; ++i) {
        SetDigit(i, ' ');
    }
}

std::vector<std::vector<uint8_t>> ControlSurface::TakeOutgoing() {
    std::vector<std::vector<uint8_t>> out;
    out.swap(_outgoing);
    return out;
}

void ControlSurface::SetLed(Button b, uint8_t state) {
    auto it = _leds.find((uint8_t)b);
    if (it != _leds.end() && it->second == state) {
        return;
    }
    _leds[(uint8_t)b] = state;
    _outgoing.push_back({ NOTE_ON, (uint8_t)b, state });
}

void ControlSurface::SetDigit(int index, uint8_t value) {
    if (_digits[index] == value) {
        return;
    }
    _digits[index] = value;
    _outgoing.push_back({ CONTROL_CHANGE, (uint8_t)(CC_TIMECODE_FIRST + index), value });
}

} // namespace mcu
