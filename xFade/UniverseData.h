#pragma once

#include <mutex>

#include "PacketData.h"

class UniverseData
{
    int _universe = 0;
    int _targetProtocol = 0;
    std::mutex _lock;
    PacketData _left;
    PacketData _right;
    std::string _targetIP;
    std::list<int> _excludedChannels;

    void PrepareData(PacketData* target, PacketData* source, int protocol);
    void Blend(uint8_t* buffer, uint8_t* blendBuffer, size_t channels, float pos, std::list<int> excludeChannels);

public:

    static std::string __leftTag;
    static std::string __rightTag;

    static void ClearTags() { __leftTag = ""; __rightTag = ""; }
    static void SetLeftTag(const std::string& left) { __leftTag = left; }
    static void SetRightTag(const std::string& right) { __rightTag = right; }
    int GetUniverse() const { return _universe; }
    std::mutex& GetLock() { return _lock; }
    std::string GetTargetIP() const { return _targetIP; }
    bool UpdateLeft(int type, uint8_t* buffer, int size);
    bool UpdateRight(int type, uint8_t* buffer, int size);
    int GetLeftSequenceNum();
    int GetRightSequenceNum();
    int GetOutputFormat() const { return _targetProtocol; }
    UniverseData(int universe, const std::string& targetIP, const std::string& targetProtocol, std::list<int> excludedChannels);
    virtual ~UniverseData() {}
    PacketData* GetOutput(PacketData* output, int leftBrightness, int rightBrightness, float pos);
};
