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

#include "SerialOutput.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

// Plain LOR (and D-Light): one 6-byte set-intensity command per changed
// channel, 16 channels per unit.  Kept in step with FPP's LOR output so a
// sequence tested from xLights puts the same commands on the wire as FPP at
// show time: the same intensity table, unit limit, line-rate budget,
// background refresh and heartbeat.
class LOROutput : public SerialOutput
{
protected:
    #pragma region LOR Constants
    static const unsigned int LOR_PACKET_LEN = 256;
    // LOR Optimised's own buffer size; it addresses Pixie ports differently.
    static const unsigned int LOR_MAX_CHANNELS = 20480;
    // Unit IDs run 1-0xF0 (higher IDs are reserved, 0xFF is broadcast), 16
    // channels each, so this is all a plain LOR network can address.
    static constexpr int LOR_MAX_UNIT_ID = 0xF0;
    static constexpr unsigned int LOR_MAX_UNIT_CHANNELS = LOR_MAX_UNIT_ID * 16;
    #pragma endregion 

    #pragma region Member Variables
    uint8_t _data[LOR_PACKET_LEN];
    #pragma endregion 

    // Every write goes through here so the heartbeat never lands inside a
    // command.
    void WriteSerial(const uint8_t* d, size_t len);
    // False for a subclass with its own wire format: the background thread
    // then only sends heartbeats.
    virtual bool SendsIntensityCommands() const { return true; }

private:
    void InitState();
    void StartPump();
    void StopPump();
    void PumpLoop();
    void PumpChannels(std::chrono::steady_clock::time_point now);
    void QueueChannel(int channel, uint8_t value);
    void WriteBatch();
    void WriteHeartbeatLocked() const;

    // Frame thread only: the values set since the last EndFrame.
    std::vector<uint8_t> _staged;
    // The rest is guarded by _lock.
    std::vector<uint8_t> _desired;
    // What each unit was last sent, or LOR_UNKNOWN.
    std::vector<uint16_t> _lastValue;
    // Pump pass a channel was last queued in, so the refresh does not send a
    // channel twice in one pass.
    std::vector<uint32_t> _queuedPass;
    std::vector<uint8_t> _batch;
    std::vector<int> _batchChannel;
    int _batchCount = 0;
    uint32_t _pass = 0;
    // Where the next pass starts looking for changes, so a pass with more
    // changes than fit the budget does not starve the higher channels.
    int _scanStart = 0;
    int _refreshAt = 0;
    std::chrono::steady_clock::time_point _lastPump{};
    mutable std::chrono::steady_clock::time_point _lastHeartbeat{};
    mutable std::mutex _lock;
    std::condition_variable _pumpCond;
    std::thread _pumpThread;
    bool _stopPump = false;
    bool _frameReady = false;
    // SendsIntensityCommands(), captured on the calling thread so the pump
    // never makes a virtual call (it may still be running while a subclass
    // destructor executes).
    bool _sendIntensity = true;

public:

    #pragma region Constructors and Destructors
    LOROutput(pugi::xml_node node);
    LOROutput(const LOROutput& from);
    LOROutput();
    virtual ~LOROutput() override;
    virtual Output* Copy() override
    {
        return new LOROutput(*this);
    }
#pragma endregion 

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_LOR; }

    virtual int32_t GetMaxChannels() const override { return LOR_MAX_UNIT_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= (int32_t)LOR_MAX_UNIT_CHANNELS; }
    #pragma endregion 

    #pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
    #pragma endregion Start and Stop

    #pragma region Frame Handling
    virtual void EndFrame(int suppressFrames) override;
    virtual void SendHeartbeat() const override;
    #pragma endregion 

    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void AllOff() override;
    #pragma endregion 
};
