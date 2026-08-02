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
#include <list>

// Tracks the set of selected (absolute, 1-based) channels for the light test
// as a sorted, non-overlapping list of ranges. Each range is packed into a
// single int64_t: start in the high 32 bits, end in the low 32.
class ChannelTracker
{
    long _changeCount { 0 };
    std::list<int64_t> _ranges;
    long _lastReturnedChannel { -1 };
    static constexpr int64_t NORANGE = 0x0FFFFFFFFFFFFFFF;
    int64_t _lastReturnedRange { NORANGE };

    static int64_t SetBoth(long start, long end) { return (static_cast<int64_t>(start) << 32) + end; }

    void ClearLast()
    {
        _lastReturnedChannel = -1;
        _lastReturnedRange = NORANGE;
    }

public:
    ChannelTracker() = default;
    virtual ~ChannelTracker() = default;

    static long GetStart(int64_t value) { return static_cast<long>(value >> 32); }
    static long GetEnd(int64_t value) { return static_cast<long>(value & 0xFFFFFFFF); }

    void FixOverlaps();
    void Dump();
    void AddRange(long start, long end);
    void RemoveRange(long start, long end);
    long GetChangeCount() const { return _changeCount; }

    void Clear()
    {
        _changeCount++;
        _ranges.clear();
    }

    bool IsChannelOn(long ch) const
    {
        for (const auto& it : _ranges) {
            if (ch >= GetStart(it) && ch <= GetEnd(it)) return true;
        }
        return false;
    }

    bool AreAnyIncluded(long start, long end) const
    {
        for (const auto& it : _ranges) {
            long s = GetStart(it);
            long e = GetEnd(it);
            if ((s >= start && s <= end) || (e >= start && e <= end)) return true;
        }
        return false;
    }

    long GetFirst()
    {
        if (_ranges.empty()) return -1;
        _lastReturnedChannel = GetStart(_ranges.front());
        return _lastReturnedChannel;
    }

    long GetNext()
    {
        // Assumes ranges are sorted
        if (_lastReturnedChannel == -1) return -1;

        for (const auto& it : _ranges) {
            long s = GetStart(it);
            long e = GetEnd(it);

            _lastReturnedChannel++;

            if (s <= _lastReturnedChannel && e >= _lastReturnedChannel) return _lastReturnedChannel;

            if (s > _lastReturnedChannel) {
                _lastReturnedChannel = s;
                return _lastReturnedChannel;
            }
            _lastReturnedChannel--;
        }

        return -1;
    }

    long GetChannelAfter(long ch) const
    {
        for (const auto& it : _ranges) {
            long s = GetStart(it);
            long e = GetEnd(it);

            ch++;
            if (s <= ch && e >= ch) return ch;
            if (s > ch) return s;
        }

        return -1;
    }

    void GetFirstRange(long& start, long& end)
    {
        if (_ranges.empty()) {
            start = -1;
            end = -1;
            return;
        }

        _lastReturnedRange = _ranges.front();
        start = GetStart(_lastReturnedRange);
        end = GetEnd(_lastReturnedRange);
    }

    void GetNextRange(long& start, long& end)
    {
        if (_lastReturnedRange == NORANGE) {
            start = -1;
            end = -1;
            return;
        }

        for (auto it = _ranges.begin(); it != _ranges.end(); ++it) {
            if (*it == _lastReturnedRange) {
                ++it;
                if (it == _ranges.end()) {
                    _lastReturnedRange = NORANGE;
                    start = -1;
                    end = -1;
                    return;
                }
                _lastReturnedRange = *it;
                start = GetStart(*it);
                end = GetEnd(*it);
                return;
            }
        }

        _lastReturnedRange = NORANGE;
        start = -1;
        end = -1;
    }
};
