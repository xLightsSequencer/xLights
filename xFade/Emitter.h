#pragma once

#include <string>
#include <mutex>
#include <map>
#include <atomic>
#include "PacketData.h"

#define PINGINTERVAL 60

class OutputManager;
class EmitterThread;
class ListenerManager;
class Settings;
class UniverseData;

class Emitter
{
    std::atomic<uint32_t> _sent; // = 0;
    EmitterThread* _emitterThread = nullptr;
    std::map<int, UniverseData*> _universes;
    std::atomic<int> _frameMS; // = 50;
    std::atomic<float> _pos; // = 0.0;
    bool _stop = false;
    std::string _localIP;
    std::atomic<int> _leftBrightness;
    std::atomic<int> _rightBrightness;
    Settings* _settings = nullptr;

    public:

	Emitter(const std::map<int, UniverseData*>& universes, std::string localIP, Settings* settings);
	virtual ~Emitter();
    void Stop();
    int GetFrameMS() const { return _frameMS; }
    float GetPos() const { return _pos; }
    void SetFrameMS(int ms) { _frameMS = ms; }
    void SetPos(float pos) { _pos = pos; }
    std::map<int, UniverseData*> GetUniverses() const { return _universes; }
    std::string GetLocalIP() const { return _localIP; }
    void SetLocalIP(std::string localIP) { _localIP = localIP; }
    void SetLeftBrightness(int brightness) { _leftBrightness = brightness; }
    void SetRightBrightness(int brightness) { _rightBrightness = brightness; }
    int GetLeftBrightness() const { return _leftBrightness; }
    int GetRightBrightness() const { return _rightBrightness; }
    uint32_t GetSent() const { return _sent; }
    void IncrementSent() { _sent++; }
    void ZeroSent() { _sent = 0; }
    Settings* GetSettings() const { return _settings; }
};

