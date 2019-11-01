#pragma once

#include <string>
#include <mutex>
#include <map>
#include <atomic>

#include "PacketData.h"

class ArtNETReceiverThread;
class UniverseData;

class ArtNETReceiver
{
    std::atomic<uint32_t> _leftReceived; // = 0;
    std::atomic<uint32_t> _rightReceived; // = 0;
    ArtNETReceiverThread* _receiverThread = nullptr;
    std::map<int, UniverseData*> _universes;
    bool _stop = false;
    std::atomic<bool> _suspend; // = false;
    wxWindow* _frame = nullptr;

    UniverseData* GetUniverseData(int universe);
    bool IsLeft(uint8_t* packet);
    bool IsRight(uint8_t* packet);

    public:

    ArtNETReceiver(wxWindow* frame, const std::map<int, UniverseData*>& universes, const std::string& localInputIP);
    virtual ~ArtNETReceiver();
    void Stop();
    bool IsSuspended() { return _suspend; }
    void Suspend(bool suspend) { _suspend = suspend; }
    uint32_t GetLeftReceived() const { return _leftReceived; }
    uint32_t GetRightReceived() const { return _rightReceived; }
    void IncrementLeftReceived() { _leftReceived++; }
    void IncrementRightReceived() { _rightReceived++; }
    void ZeroReceived() { _leftReceived = 0; _rightReceived = 0; }
    void StashPacket(uint8_t* buffer, int size);
};