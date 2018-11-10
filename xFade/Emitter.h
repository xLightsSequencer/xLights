#ifndef EMITTER_H
#define EMITTER_H

#include <string>
#include <mutex>
#include <map>
#include "PacketData.h"

#define PINGINTERVAL 60

class OutputManager;
class EmitterThread;
class ListenerManager;
class Settings;

class Emitter
{
    unsigned long _sent = 0;
    EmitterThread* _emitterThread;
    std::map<int, std::string>* _targetIP;
    std::map<int, std::string>* _protocol;
    std::map<int, PacketData>* _leftData;
    std::map<int, PacketData>* _rightData;
    int _frameMS;
    float _pos;
    bool _stop;
    std::mutex* _lock;
    std::string _localIP;
    int _leftBrightness;
    int _rightBrightness;
    Settings* _settings;

    public:

	Emitter(std::map<int, std::string>* ip, std::map<int, PacketData>* left, std::map<int, PacketData>* right, std::map<int, std::string>* protocol, std::mutex* mutex, std::string localIP, Settings* settings);
	virtual ~Emitter();
    void Stop();
    void Restart();
    int GetFrameMS() const { std::unique_lock<std::mutex> mutLock(*_lock); return _frameMS; }
    float GetPos() const { std::unique_lock<std::mutex> mutLock(*_lock); return _pos; }
    void SetFrameMS(int ms) { std::unique_lock<std::mutex> mutLock(*_lock); _frameMS = ms; }
    void SetPos(float pos) { std::unique_lock<std::mutex> mutLock(*_lock); _pos = pos; }
    std::map<int, std::string> GetIps() const;
    PacketData GetLeft(int u) const;
    PacketData GetRight(int u) const;
    long GetProtocol(int u) const;
    std::string GetLocalIP() const { return _localIP; }
    void SetLocalIP(std::string localIP) { _localIP = localIP; }
    void SetLeftBrightness(int brightness) { _leftBrightness = brightness; }
    void SetRightBrightness(int brightness) { _rightBrightness = brightness; }
    int GetLeftBrightness() const { return _leftBrightness; }
    int GetRightBrightness() const { return _rightBrightness; }
    unsigned long GetSent() const { return _sent; }
    void IncrementSent() { _sent++; }
    void ZeroSent() { _sent = 0; }
    Settings* GetSettings() const { return _settings; }
};

#endif 