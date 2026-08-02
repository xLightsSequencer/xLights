/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "TestPatternEngine.h"

#include "ChannelTracker.h"
#include "Output.h"
#include "OutputManager.h"
#include "UtilFunctions.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <spdlog/fmt/fmt.h>

namespace xltest {

void TestPatternEngine::SetChannels(std::vector<uint32_t> all,
                                    std::vector<uint32_t> red,
                                    std::vector<uint32_t> green,
                                    std::vector<uint32_t> blue,
                                    std::vector<uint32_t> white)
{
    _channels = std::move(all);
    _channelsR = std::move(red);
    _channelsG = std::move(green);
    _channelsB = std::move(blue);
    _channelsW = std::move(white);
    _channelsDirty = true;
}

void TestPatternEngine::Reset()
{
    _channelsDirty = true;
    _haveLast = false;
    _nextSequenceStart = -1;
    _rgbCycle = 0;
    _seqIdx = 0;
    _twinkleState.clear();
    _shimIntensity = -1;
    _shimHighlight = false;
    _status.clear();
}

void TestPatternEngine::ApplySuspend(OutputManager* outputManager, const ChannelTracker& tracker, bool suspend)
{
    if (outputManager == nullptr) return;

    if (!suspend) {
        outputManager->SuspendAll(false);
        return;
    }

    for (const auto& it : outputManager->GetOutputs()) {
        it->Suspend(!tracker.AreAnyIncluded(it->GetStartChannel(), it->GetEndChannel()));
    }
}

void TestPatternEngine::Frame(OutputManager* om, const TestParameters& p, long curtimeMS)
{
    if (om == nullptr) return;

    if (!_haveLast || p.mode != _lastMode) {
        _lastMode = p.mode;
        _haveLast = true;
        _channelsDirty = true;
        _seqIdx = 0;
    }

    if (p.function != _lastFunction) {
        _lastFunction = p.function;
        _rgbCycle = 0;
        _channelsDirty = true;
        _nextSequenceStart = -1;
    }

    if (_channelsDirty) {
        _nextSequenceStart = -1;
        om->AllOff();

        _chaseGrouping = p.chaseGrouping;
        if (p.chaseWholeSelection) {
            _chaseGrouping = _channels.empty() ? std::numeric_limits<int>::max()
                                               : static_cast<int>(_channels.size());
        }
        if (_chaseGrouping <= 0) _chaseGrouping = 1;

        _lastSpeed = -1;
        _lastBgIntensity = -1;
        _lastFgIntensity = -1;
        _lastTwinkleRatio = -1;
        for (int i = 0; i < 3; i++) {
            _lastBgColor[i] = -1;
            _lastFgColor[i] = -1;
        }

        if (p.function == TestFunction::OFF) {
            _status = "Testing off";
        } else {
            _status = fmt::format("Testing {} channels", _channels.size());
        }
        _channelsDirty = false;
    }

    if (p.function == TestFunction::OFF) return;

    // The controller tests walk port ranges rather than the channel list, but
    // an empty selection still means nothing is under test.
    if (_channels.empty()) return;

    switch (p.mode) {
    case TestMode::Standard:
        FrameStandard(om, p, curtimeMS);
        break;
    case TestMode::RGB:
        FrameRGB(om, p, curtimeMS);
        break;
    case TestMode::RGBCycle:
        FrameRGBCycle(om, p, curtimeMS);
        break;
    case TestMode::Controller:
        FrameController(om, p, curtimeMS);
        break;
    }
}

void TestPatternEngine::FrameStandard(OutputManager* om, const TestParameters& p, long curtime)
{
    const size_t count = _channels.size();
    const int bgIntensity = p.backgroundIntensity;
    const int fgIntensity = p.highlightIntensity;

    const bool colorChange = bgIntensity != _lastBgIntensity || fgIntensity != _lastFgIntensity;
    _lastBgIntensity = bgIntensity;
    _lastFgIntensity = fgIntensity;
    _interval = 1600 - p.speed * 15;

    switch (p.function) {
    case TestFunction::DIM:
        if (colorChange) {
            for (size_t i = 0; i < count; i++) {
                om->SetOneChannel(_channels[i] - 1, bgIntensity);
            }
        }
        break;

    case TestFunction::TWINKLE: {
        double frameTime = p.frameTimeMS > 0 ? p.frameTimeMS : 50.0;
        // NOTE: _lastTwinkleRatio is never assigned p.twinkleRatio, so this
        // rebuild fires every frame and the hold-period decay below is dead.
        // Preserved from the desktop dialog: the test has always re-rolled at
        // the timer rate, and users know that look. Assigning it here would
        // change the visual character of a long-shipped test.
        if (_lastSpeed < 0 || p.twinkleRatio != _lastTwinkleRatio) {
            _lastSpeed = 0;
            _twinkleState.clear();
            for (size_t i = 0; i < count; i++) {
                _twinkleState.push_back(static_cast<int>(rand01() * (double)p.twinkleRatio) == 0 ? -1 : 1);
            }
        }
        for (size_t i = 0; i < _twinkleState.size() && i < count; i++) {
            if (_twinkleState[i] < -1) {
                _twinkleState[i]++;
            } else if (_twinkleState[i] > 1) {
                _twinkleState[i]--;
            } else if (_twinkleState[i] == -1) {
                // was background, now highlight for a random period
                _twinkleState[i] = static_cast<int>((rand01() * (double)_interval + 100.0) / frameTime);
                om->SetOneChannel(_channels[i] - 1, fgIntensity);
            } else {
                // was on, now go to background for a random period
                double ratio = p.twinkleRatio == 1 ? 0.9 : (double)p.twinkleRatio;
                _twinkleState[i] = -static_cast<int>((rand01() * (double)_interval + 100.0) / frameTime * (ratio - 1.0));
                om->SetOneChannel(_channels[i] - 1, bgIntensity);
            }
        }
    } break;

    case TestFunction::SHIMMER:
        if (colorChange || curtime >= _nextSequenceStart) {
            _shimIntensity = (_shimIntensity == fgIntensity) ? bgIntensity : fgIntensity;
            for (size_t i = 0; i < count; i++) {
                om->SetOneChannel(_channels[i] - 1, _shimIntensity);
            }
        }
        if (curtime >= _nextSequenceStart) {
            _nextSequenceStart = curtime + _interval / 2;
        }
        break;

    case TestFunction::CHASE:
        if (colorChange || curtime >= _nextSequenceStart) {
            for (size_t i = 0; i < count; i++) {
                int v = (static_cast<int>(i % _chaseGrouping) == static_cast<int>(_seqIdx)) ? fgIntensity : bgIntensity;
                om->SetOneChannel(_channels[i] - 1, v);
            }
        }
        if (curtime >= _nextSequenceStart) {
            _nextSequenceStart = curtime + _interval;
            _seqIdx = (_seqIdx + 1) % _chaseGrouping;
            if (_seqIdx >= count) _seqIdx = 0;
        }
        _status = fmt::format("Testing {} channels; chase now at ch# {}", count, _seqIdx);
        break;

    default:
        break;
    }
}

void TestPatternEngine::FrameRGB(OutputManager* om, const TestParameters& p, long curtime)
{
    const size_t count = _channels.size();

    bool colorChange = false;
    for (int i = 0; i < 3; i++) {
        colorChange |= (p.backgroundColor[i] != _lastBgColor[i]);
        colorChange |= (p.highlightColor[i] != _lastFgColor[i]);
        _lastBgColor[i] = p.backgroundColor[i];
        _lastFgColor[i] = p.highlightColor[i];
    }
    _interval = 1600 - p.speed * 15;

    switch (p.function) {
    case TestFunction::DIM:
        if (colorChange) {
            for (size_t i = 0; i < count; i++) {
                om->SetOneChannel(_channels[i] - 1, p.backgroundColor[i % 3]);
            }
        }
        break;

    case TestFunction::TWINKLE: {
        double frameTime = p.frameTimeMS > 0 ? p.frameTimeMS : 50.0;
        // See the note in FrameStandard: this rebuild fires every frame.
        if (_lastSpeed < 0 || _lastTwinkleRatio != p.twinkleRatio) {
            _lastSpeed = 0;
            _twinkleState.clear();
            // Guarded subtraction: the desktop wrote `i < count - 2` on an
            // unsigned count, so a single selected channel wrapped to SIZE_MAX
            // and spun until it exhausted memory.
            for (size_t i = 0; i + 2 < count; i += 3) {
                _twinkleState.push_back(static_cast<int>(rand01() * (double)p.twinkleRatio) == 0 ? -1 : 1);
            }
        }
        for (size_t i = 0; i < _twinkleState.size(); i++) {
            const size_t base = i * 3;
            if (base + 2 >= count) break;

            if (_twinkleState[i] < -1) {
                _twinkleState[i]++;
            } else if (_twinkleState[i] > 1) {
                _twinkleState[i]--;
            } else if (_twinkleState[i] == -1) {
                _twinkleState[i] = static_cast<int>((rand01() * (double)_interval + 100.0) / frameTime);
                om->SetOneChannel(_channels[base] - 1, p.highlightColor[0]);
                om->SetOneChannel(_channels[base + 1] - 1, p.highlightColor[1]);
                om->SetOneChannel(_channels[base + 2] - 1, p.highlightColor[2]);
            } else {
                double ratio = p.twinkleRatio == 1 ? 0.9 : (double)p.twinkleRatio;
                _twinkleState[i] = -static_cast<int>((rand01() * (double)_interval + 100.0) / frameTime * (ratio - 1.0));
                om->SetOneChannel(_channels[base] - 1, p.backgroundColor[0]);
                om->SetOneChannel(_channels[base + 1] - 1, p.backgroundColor[1]);
                om->SetOneChannel(_channels[base + 2] - 1, p.backgroundColor[2]);
            }
        }
    } break;

    case TestFunction::SHIMMER:
        if (colorChange || curtime >= _nextSequenceStart) {
            _shimHighlight = !_shimHighlight;
            const int* shim = _shimHighlight ? p.highlightColor : p.backgroundColor;
            for (size_t i = 0; i < count; i++) {
                om->SetOneChannel(_channels[i] - 1, shim[i % 3]);
            }
        }
        if (curtime >= _nextSequenceStart) {
            _nextSequenceStart = curtime + _interval / 2;
        }
        break;

    case TestFunction::CHASE:
        if (colorChange || curtime >= _nextSequenceStart) {
            for (size_t i = 0; i < count; i++) {
                int v = (static_cast<int>((i / 3) % _chaseGrouping) == static_cast<int>(_seqIdx))
                            ? p.highlightColor[i % 3]
                            : p.backgroundColor[i % 3];
                om->SetOneChannel(_channels[i] - 1, v);
            }
        }
        if (curtime >= _nextSequenceStart) {
            _nextSequenceStart = curtime + _interval;
            _seqIdx = (_seqIdx + 1) % _chaseGrouping;
            if (_seqIdx >= (count + 2) / 3) _seqIdx = 0;
        }
        _status = fmt::format("Testing {} channels; chase now at ch# {}", count, _seqIdx);
        break;

    default:
        break;
    }
}

void TestPatternEngine::FrameRGBCycle(OutputManager* om, const TestParameters& p, long curtime)
{
    const size_t count = _channels.size();
    const int v = p.speed;

    if (p.function == TestFunction::DIM) {
        // colour mixing
        if (v != _lastSpeed) {
            _frequency = v / 1000.0f + 0.05f;
            _lastSpeed = v;
        }
        int color[3];
        color[0] = static_cast<int>(sin(_frequency * _seqIdx + 0.0) * 127 + 128);
        color[1] = static_cast<int>(sin(_frequency * _seqIdx + 2.0) * 127 + 128);
        color[2] = static_cast<int>(sin(_frequency * _seqIdx + 4.0) * 127 + 128);
        _seqIdx++;
        for (size_t i = 0; i < count; i++) {
            om->SetOneChannel(_channels[i] - 1, color[i % 3]);
        }
    } else if (p.function == TestFunction::RGBW) {
        if (v != _lastSpeed) {
            _interval = (101 - v) * 50;
            _nextSequenceStart = curtime + _interval;
            _lastSpeed = v;
        }
        if (curtime >= _nextSequenceStart) {
            for (size_t i = 0; i < count; i++) {
                om->SetOneChannel(_channels[i] - 1, 0);
            }
            const std::vector<uint32_t>* lit = nullptr;
            switch (_rgbCycle) {
            case 0: lit = &_channelsR; break;
            case 1: lit = &_channelsG; break;
            case 2: lit = &_channelsB; break;
            case 3: lit = &_channelsW; break;
            default: break;
            }
            if (lit != nullptr) {
                for (const auto ch : *lit) {
                    om->SetOneChannel(ch - 1, 255);
                }
            }
            _rgbCycle = (_rgbCycle + 1) % 4;
            _nextSequenceStart += _interval;
        }
    } else {
        // RGB cycle
        if (v != _lastSpeed) {
            _interval = (101 - v) * 50;
            _nextSequenceStart = curtime + _interval;
            _lastSpeed = v;
        }
        if (curtime >= _nextSequenceStart) {
            for (size_t i = 0; i < count; i++) {
                int val = (_rgbCycle == 3) ? 255 : ((i % 3) == _rgbCycle ? 255 : 0);
                om->SetOneChannel(_channels[i] - 1, val);
            }
            _rgbCycle = (_rgbCycle + 1) % _chaseGrouping;
            _nextSequenceStart += _interval;
        }
    }

    // Light every 50th pixel at half white so long strings can be counted.
    if (p.tag50th) {
        // Guarded upper bound: the desktop indexed i+1/i+2 without checking,
        // reading past the end whenever the selection did not land on a
        // whole 50-pixel boundary.
        for (size_t i = 3 * (50 - 1); i + 2 < count; i += 150) {
            for (size_t j = 0; j < 3; j++) {
                om->SetOneChannel(_channels[i + j] - 1, 128);
            }
        }
    }
}

void TestPatternEngine::FrameController(OutputManager* om, const TestParameters& p, long curtime)
{
    _interval = p.speed * 10;

    if (p.function == TestFunction::PortCycle) {
        uint16_t ports { 0 };
        for (const auto& port : p.ports) {
            ports = std::max(ports, port.port);

            // Port 1 lights the red element, every other port the blue one,
            // so a miswired port is obvious at a glance.
            const int offset = (port.port == 1) ? 0 : 2;

            for (long j = port.firstChannel; j < port.lastChannel; ++j) {
                om->SetOneChannel(j - 1, 0);
            }
            if (port.port == _rgbCycle + 1) {
                for (long j = port.firstChannel; j < port.lastChannel; j += 3) {
                    om->SetOneChannel((j - 1) + offset, 255);
                }
                _status = fmt::format("Testing {} channels Port {}", _channels.size(), port.port);
            }
        }
        if (curtime >= _nextSequenceStart) {
            _rgbCycle++;
            _rgbCycle = (ports > 0) ? (_rgbCycle % ports) : 0;
            _nextSequenceStart = curtime + (_interval * 2);
        }
    } else if (p.function == TestFunction::ColorBlocks) {
        for (const auto& port : p.ports) {
            // Assumes 3-channel pixels: paint 10-pixel blocks of red, green
            // and blue so pixel counts can be read off the string.
            for (long j = port.firstChannel; j < port.lastChannel; j += 3) {
                long pix = j - port.firstChannel;
                pix = (pix % 90) / 30;
                om->SetOneChannel((j - 1) + pix, 255);
            }
        }
    }
}

} // namespace xltest
