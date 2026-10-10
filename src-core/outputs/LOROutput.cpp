
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LOROutput.h"

#include <algorithm>
#include <cstring>

#include "serial.h"

#include <log.h>

namespace {
constexpr size_t LOR_INTENSITY_SIZE = 6;
// Marks a channel whose state on the unit is not known: never sent since the
// output opened, or its command was cut off by a short write.
constexpr uint16_t LOR_UNKNOWN = 0x100;
// A pass's commands go out in one write, capped below what the tty can queue
// so a non-blocking write never drops the tail of a batch.
constexpr size_t LOR_MAX_BATCH_SIZE = 4096;
constexpr int LOR_MAX_BATCH_CMDS = LOR_MAX_BATCH_SIZE / LOR_INTENSITY_SIZE;
// Units go inactive after 2 seconds without a heartbeat and can need several
// before they act on commands again, so the heartbeat runs for as long as the
// port is open, not only while frames are being output.
constexpr auto LOR_HEARTBEAT_INTERVAL = std::chrono::milliseconds(300);
// FPP sends a frame on every frame tick even when nothing is playing; xLights
// only outputs frames while playing, so the pump also runs on this interval
// to keep the refresh and the start-up state going between frames.
constexpr auto LOR_PUMP_INTERVAL = std::chrono::milliseconds(50);
// Pass interval assumed for the first pass, and the range a measured interval
// is clamped to when sizing a pass's byte budget.
constexpr long long LOR_DEFAULT_FRAME_TIME = 50000;
constexpr long long LOR_MIN_FRAME_TIME = 10000;
constexpr long long LOR_MAX_FRAME_TIME = 100000;
// Shortest time the background refresh takes to resend every channel.
constexpr long long LOR_REFRESH_PERIOD = 1000000;
}

#pragma region Constructors and Destructors
LOROutput::LOROutput(const LOROutput& from) :
    SerialOutput(from)
{
    InitState();
}

LOROutput::LOROutput(pugi::xml_node node) : SerialOutput(node) {
    InitState();
}

LOROutput::LOROutput() : SerialOutput() {
    InitState();
}

LOROutput::~LOROutput() {
    StopPump();
}

void LOROutput::InitState() {
    for (size_t i = 0; i < LOR_PACKET_LEN; i++) {
        // LOR intensity is a whole percent (101 levels): 0xF0 is off, 0x01 is
        // full, and 1-99% are 228 - 2 * percent.  The Pixie manual's "LOR
        // %intensity to DMX Intensities" table (page 46) maps each percent to
        // output as floor(percent * 2.55):
        // https://www1.lightorama.com/PDF/Pixie_Man_Web.pdf
        // A linear map over 0xF0..0x01 lands low values past the off end (the
        // bottom of a fade goes dark) and reads 50% as about 54%.  FPP's LOR
        // output uses this same table, so both put the same bytes on the wire.
        int percent = (int)(i * 100 + 127) / 255;
        if (percent == 0) {
            _data[i] = 0xF0;
        } else if (percent >= 100) {
            _data[i] = 0x01;
        } else {
            _data[i] = 228 - percent * 2;
        }
    }
    _staged.assign(LOR_MAX_UNIT_CHANNELS, 0);
    _desired.assign(LOR_MAX_UNIT_CHANNELS, 0);
    _lastValue.assign(LOR_MAX_UNIT_CHANNELS, LOR_UNKNOWN);
    _queuedPass.assign(LOR_MAX_UNIT_CHANNELS, 0);
    _batch.assign(LOR_MAX_BATCH_SIZE, 0);
    _batchChannel.assign(LOR_MAX_BATCH_CMDS, 0);
}
#pragma endregion 

#pragma region Start and Stop
bool LOROutput::Open() {

    if (!_enabled) return true;

    StopPump();
    _ok = SerialOutput::Open();

    // Every channel starts unknown, so the first passes establish the state
    // of every unit (all off until frames arrive) within the line budget.
    {
        std::lock_guard<std::mutex> lk(_lock);
        std::fill(_staged.begin(), _staged.end(), 0);
        std::fill(_desired.begin(), _desired.end(), 0);
        std::fill(_lastValue.begin(), _lastValue.end(), LOR_UNKNOWN);
        _scanStart = 0;
        _refreshAt = 0;
    }
    if (SendsIntensityCommands() && _channels > (int32_t)LOR_MAX_UNIT_CHANNELS) {
        spdlog::warn("LOR: {} channels on {} run past unit ID {}; channels beyond {} are not sent.",
                     _channels, _commPort, LOR_MAX_UNIT_ID, LOR_MAX_UNIT_CHANNELS);
    }
    if (_ok) {
        StartPump();
    }
    return _ok;
}

void LOROutput::Close() {
    StopPump();
    SerialOutput::Close();
}

void LOROutput::StartPump() {
    if (_pumpThread.joinable() || _serial == nullptr) {
        return;
    }
    _sendIntensity = SendsIntensityCommands();
    _stopPump = false;
    _frameReady = false;
    _lastPump = {};
    _pumpThread = std::thread(&LOROutput::PumpLoop, this);
}

void LOROutput::StopPump() {
    if (_pumpThread.joinable()) {
        {
            std::lock_guard<std::mutex> lk(_lock);
            _stopPump = true;
        }
        _pumpCond.notify_all();
        _pumpThread.join();
    }
}

void LOROutput::PumpLoop() {
    std::unique_lock<std::mutex> lk(_lock);
    while (!_stopPump) {
        _pumpCond.wait_for(lk, LOR_PUMP_INTERVAL, [this] { return _stopPump || _frameReady; });
        if (_stopPump) {
            break;
        }
        _frameReady = false;
        auto now = std::chrono::steady_clock::now();
        if (now - _lastHeartbeat >= LOR_HEARTBEAT_INTERVAL) {
            WriteHeartbeatLocked();
        }
        if (_sendIntensity) {
            PumpChannels(now);
        }
    }
}
#pragma endregion 

#pragma region Frame Handling
void LOROutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend) return;

    // SerialOutput::StartFrame reopens a port that failed to open without
    // coming back through Open.
    if (_ok && !_pumpThread.joinable()) {
        StartPump();
    }
    if (_sendIntensity) {
        std::lock_guard<std::mutex> lk(_lock);
        _desired = _staged;
        _frameReady = true;
    }
    _pumpCond.notify_one();
}

void LOROutput::SendHeartbeat() const {
    if (!_enabled || _serial == nullptr || !_ok) return;

    std::lock_guard<std::mutex> lk(_lock);
    WriteHeartbeatLocked();
}

void LOROutput::WriteHeartbeatLocked() const {
    static const uint8_t heartbeat[5] = { 0x00, 0xFF, 0x81, 0x56, 0x00 };
    if (_serial != nullptr) {
        _serial->Write((char*)heartbeat, sizeof(heartbeat));
    }
    _lastHeartbeat = std::chrono::steady_clock::now();
}

void LOROutput::WriteSerial(const uint8_t* d, size_t len) {
    std::lock_guard<std::mutex> lk(_lock);
    if (_serial != nullptr) {
        _serial->Write((char*)d, len);
    }
}

// Mirrors FPP's LOROutput::SendData.
void LOROutput::PumpChannels(std::chrono::steady_clock::time_point now) {
    if (_serial == nullptr) {
        return;
    }
    const int count = std::min((int)_channels, (int)LOR_MAX_UNIT_CHANNELS);
    if (count <= 0) {
        return;
    }

    // Budget the pass to what the wire can carry before the next one, less
    // whatever is still queued in the port.  Writing more only builds a
    // backlog that delays the lights behind the audio.  Changes that do not
    // fit stay pending (_lastValue is untouched) and go out on a later pass.
    long long frameTime = _lastPump.time_since_epoch().count() != 0
                              ? std::chrono::duration_cast<std::chrono::microseconds>(now - _lastPump).count()
                              : LOR_DEFAULT_FRAME_TIME;
    _lastPump = now;
    frameTime = std::clamp(frameTime, LOR_MIN_FRAME_TIME, LOR_MAX_FRAME_TIME);
    long long budget = (long long)GetBaudRate() / 10 * frameTime / 1000000;
    const int queued = _serial->WaitingToWrite();
    if (queued > 0) {
        budget -= queued;
    }
    const int maxCmds = (int)std::clamp(budget / (long long)LOR_INTENSITY_SIZE, 0LL, (long long)LOR_MAX_BATCH_CMDS);

    _pass++;
    _batchCount = 0;
    int i = std::min(_scanStart, count - 1);
    for (int checked = 0; checked < count && _batchCount < maxCmds; checked++) {
        if (_lastValue[i] != _desired[i]) {
            QueueChannel(i, _desired[i]);
        }
        if (++i == count) {
            i = 0;
        }
    }
    _scanStart = i;

    // With room left over, resend a few channels that have not changed.  A
    // unit that missed a command (line noise, waking up, power cycled) would
    // otherwise hold the wrong value until the channel next changed, which
    // for a static channel is never.  Capped at a quarter of the pass so
    // refresh never crowds out real changes, and to one pass over the
    // channels per LOR_REFRESH_PERIOD so a small setup is not resent
    // constantly.
    int refreshCmds = std::min(maxCmds - _batchCount, std::max(1, maxCmds / 4));
    refreshCmds = std::min(refreshCmds, (int)((count * frameTime + LOR_REFRESH_PERIOD - 1) / LOR_REFRESH_PERIOD));
    for (int checked = 0; checked < count && refreshCmds > 0; checked++) {
        int ch = _refreshAt;
        if (++_refreshAt >= count) {
            _refreshAt = 0;
        }
        if (_queuedPass[ch] != _pass && _lastValue[ch] != LOR_UNKNOWN) {
            QueueChannel(ch, _desired[ch]);
            refreshCmds--;
        }
    }
    WriteBatch();
}

void LOROutput::QueueChannel(int channel, uint8_t value) {
    uint8_t* cmd = &_batch[_batchCount * LOR_INTENSITY_SIZE];
    cmd[0] = 0x00;
    cmd[1] = (uint8_t)((channel >> 4) + 1);
    cmd[2] = 0x03;
    cmd[3] = _data[value];
    cmd[4] = 0x80 | (channel & 0x0F);
    cmd[5] = 0x00;
    _batchChannel[_batchCount++] = channel;
    _lastValue[channel] = value;
    _queuedPass[channel] = _pass;
}

void LOROutput::WriteBatch() {
    if (_batchCount == 0 || _serial == nullptr) {
        _batchCount = 0;
        return;
    }
    const int len = _batchCount * (int)LOR_INTENSITY_SIZE;
    int written = _serial->Write((char*)_batch.data(), len);
#ifdef _WIN32
    // An overlapped WriteFile that is still pending reports 0 bytes: the data
    // is queued, not dropped.
    if (written >= 0) {
        written = len;
    }
#endif
    if (written < len) {
        // Every command starts with 0x00, which flushes the unit's buffer, so
        // a cut-off command is discarded.  Its channel and every one after it
        // has to be resent.
        for (int c = std::max(written, 0) / (int)LOR_INTENSITY_SIZE; c < _batchCount; c++) {
            _lastValue[_batchChannel[c]] = LOR_UNKNOWN;
        }
    }
    _batchCount = 0;
}
#pragma endregion 

#pragma region Data Setting
void LOROutput::SetOneChannel(int32_t channel, unsigned char data) {
    if (channel >= 0 && (size_t)channel < _staged.size()) {
        _staged[channel] = data;
    }
}

// Sent immediately rather than through the budgeted pump: output is usually
// stopped straight after.
void LOROutput::AllOff() {

    if (!_enabled) return;

    std::fill(_staged.begin(), _staged.end(), 0);
    std::lock_guard<std::mutex> lk(_lock);
    std::fill(_desired.begin(), _desired.end(), 0);
    if (_serial == nullptr || !_ok) return;

    const int count = std::min((int)_channels, (int)LOR_MAX_UNIT_CHANNELS);
    _pass++;
    _batchCount = 0;
    for (int ch = 0; ch < count; ch++) {
        if (_lastValue[ch] != 0) {
            QueueChannel(ch, 0);
            if (_batchCount == LOR_MAX_BATCH_CMDS) {
                WriteBatch();
            }
        }
    }
    WriteBatch();
    WriteHeartbeatLocked();
}
#pragma endregion 
