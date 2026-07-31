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

#include <cstdint>
#include <string>
#include <vector>

class OutputManager;
class ChannelTracker;

namespace xltest {

// Which family of tests is running. Mirrors the four tabs of the desktop
// test dialog; the pattern maths differs per family (Standard drives one
// intensity per channel, RGB drives channel triples, etc.).
enum class TestMode {
    Standard = 0,
    RGB = 1,
    RGBCycle = 2,
    Controller = 3
};

enum class TestFunction {
    OFF,
    CHASE,
    DIM,
    TWINKLE,
    SHIMMER,
    RGBW,
    PortCycle,
    ColorBlocks
};

// One controller output port, for the Controller-family tests. Channels are
// absolute and 1-based, matching the rest of the test surface.
struct TestPort {
    uint16_t port { 0 };
    long firstChannel { 0 };
    long lastChannel { 0 };
};

struct TestParameters {
    TestMode mode { TestMode::Standard };
    TestFunction function { TestFunction::OFF };

    int speed { 50 }; // 0-100

    // Number of channels (Standard) or pixels (RGB) in one chase group.
    // Ignored unless chaseWholeSelection is set, in which case the group
    // spans everything currently selected.
    int chaseGrouping { 1 };
    bool chaseWholeSelection { false };

    // 1-in-N channels are lit at any moment during a twinkle.
    int twinkleRatio { 10 };

    // Standard family: single intensity per channel.
    int backgroundIntensity { 0 };
    int highlightIntensity { 255 };

    // RGB family: a colour triple per pixel.
    int backgroundColor[3] { 0, 0, 0 };
    int highlightColor[3] { 255, 255, 255 };

    // Light every 50th pixel at half white so long strings can be counted.
    bool tag50th { false };

    // Milliseconds between Frame() calls. Twinkle converts durations to
    // frame counts with it, so it must match the caller's real timer period.
    int frameTimeMS { 50 };

    // Controller family only.
    std::vector<TestPort> ports;
};

// Drives one light test. The caller owns the timer and the channel
// selection; this holds only the animation state that has to persist
// between frames.
//
// Split out of the desktop PixelTestDialog so the iPad shares the pattern
// maths. Previously this state lived in function-local statics, which meant
// it leaked across dialog open/close and could not be instantiated twice.
class TestPatternEngine
{
public:
    TestPatternEngine() = default;

    // Absolute, 1-based channels, ascending. The per-colour lists are only
    // consulted by the RGBW cycle; pass empty vectors otherwise.
    void SetChannels(std::vector<uint32_t> all,
                     std::vector<uint32_t> red = {},
                     std::vector<uint32_t> green = {},
                     std::vector<uint32_t> blue = {},
                     std::vector<uint32_t> white = {});

    // Drop all animation state so the next Frame() starts a test cleanly.
    void Reset();

    // Emit one frame. Does not call StartFrame/EndFrame — the caller
    // brackets this so it can batch other output into the same frame.
    void Frame(OutputManager* outputManager, const TestParameters& params, long curtimeMS);

    // Human-readable summary of what the last Frame() did.
    const std::string& GetStatus() const { return _status; }

    size_t GetChannelCount() const { return _channels.size(); }

    // Suspend every output that has no selected channel, so unrelated
    // controllers stop receiving frames while a test runs.
    static void ApplySuspend(OutputManager* outputManager, const ChannelTracker& tracker, bool suspend);

private:
    void FrameStandard(OutputManager* om, const TestParameters& p, long curtime);
    void FrameRGB(OutputManager* om, const TestParameters& p, long curtime);
    void FrameRGBCycle(OutputManager* om, const TestParameters& p, long curtime);
    void FrameController(OutputManager* om, const TestParameters& p, long curtime);

    std::vector<uint32_t> _channels;
    std::vector<uint32_t> _channelsR;
    std::vector<uint32_t> _channelsG;
    std::vector<uint32_t> _channelsB;
    std::vector<uint32_t> _channelsW;

    bool _channelsDirty { true };
    TestMode _lastMode { TestMode::Standard };
    TestFunction _lastFunction { TestFunction::OFF };
    bool _haveLast { false };

    int _chaseGrouping { 1 };

    int _lastBgIntensity { -1 };
    int _lastFgIntensity { -1 };
    int _lastBgColor[3] { -1, -1, -1 };
    int _lastFgColor[3] { -1, -1, -1 };
    int _lastSpeed { -1 };
    int _lastTwinkleRatio { -1 };

    long _nextSequenceStart { -1 };
    unsigned int _interval { 0 };
    unsigned int _rgbCycle { 0 };
    unsigned int _seqIdx { 0 };
    float _frequency { 0.0f };

    // Shimmer alternates the whole selection between highlight and
    // background; these track which half of that toggle we are on.
    int _shimIntensity { -1 };
    bool _shimHighlight { false };

    std::vector<int> _twinkleState;

    std::string _status;
};

} // namespace xltest
