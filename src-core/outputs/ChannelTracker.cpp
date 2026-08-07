/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ChannelTracker.h"

#include <spdlog/spdlog.h>

static bool CompareRange(const int64_t& a, const int64_t& b)
{
    return ChannelTracker::GetStart(a) < ChannelTracker::GetStart(b);
}

void ChannelTracker::FixOverlaps()
{
    _ranges.sort(CompareRange);

    bool ok = true;
    do {
        ok = true;

        auto first = _ranges.begin();
        if (first == _ranges.end()) return;
        auto second = first;
        ++second;

        while (second != _ranges.end()) {
            long fe = GetEnd(*first);
            long fs = GetStart(*first);
            long ss = GetStart(*second);
            long se = GetEnd(*second);

            if (fe >= ss - 1 && fe <= se) {
                // first runs into second ... extend first and delete second
                _ranges.push_front(SetBoth(fs, se));
                _ranges.erase(first);
                _ranges.erase(second);
                ok = false;
                break;
            } else if (fe > se) {
                // second is totally unnecessary ... remove it
                _ranges.erase(second);
                ok = false;
                break;
            } else if (fe < ss - 1) {
                // there is a gap between first and second so move on
                ++first;
                ++second;
            } else {
                break;
            }
        }
        _ranges.sort(CompareRange);
    } while (!ok);
}

void ChannelTracker::Dump()
{
    spdlog::debug("Selected channels dump:");
    for (const auto& it : _ranges) {
        spdlog::debug("   {}-{}", GetStart(it), GetEnd(it));
    }
}

void ChannelTracker::AddRange(long start, long end)
{
    if (start < 0 || end < 0) return;

    ClearLast();
    for (auto it = _ranges.begin(); it != _ranges.end(); ++it) {
        long s = GetStart(*it);
        long e = GetEnd(*it);

        if (end < s - 1 || start > e + 1) {
            // this is not connected to this range
        } else if (end <= e && start < s) {
            // extend this range at the start
            _ranges.push_back(SetBoth(start, GetEnd(*it)));
            _ranges.erase(it);
            _ranges.sort(CompareRange);
            _changeCount++;
            FixOverlaps();
            return;
        } else if (start >= s && end > e) {
            // extend this range at the end
            _ranges.push_back(SetBoth(GetStart(*it), end));
            _ranges.erase(it);
            _ranges.sort(CompareRange);
            _changeCount++;
            FixOverlaps();
            return;
        } else if (start >= s && end <= e) {
            // already contained within this range ... I can ignore this request
            return;
        } else if (start < s && end > e) {
            // this new range fully encapsulates the existing range
            _ranges.push_back(SetBoth(start, end));
            _ranges.erase(it);
            _ranges.sort(CompareRange);
            _changeCount++;
            FixOverlaps();
            return;
        }
    }

    // at this point it must be an entirely new range
    _changeCount++;
    _ranges.push_back(SetBoth(start, end));
    _ranges.sort(CompareRange);
}

void ChannelTracker::RemoveRange(long start, long end)
{
    ClearLast();

    auto it = _ranges.begin();

    while (it != _ranges.end()) {
        long s = GetStart(*it);
        long e = GetEnd(*it);

        if (start <= s && end >= e) {
            // remove the whole item
            auto temp = it;
            ++temp;
            _ranges.remove(*it);
            it = temp;
            _changeCount++;
        } else if (end < s || start > e) {
            // do nothing this one does not overlap
            ++it;
        } else if (start <= s && end < e) {
            _ranges.push_back(SetBoth(end + 1, GetEnd(*it)));
            it = _ranges.erase(it);
            _changeCount++;
        } else if (start <= e && end >= e) {
            _ranges.push_back(SetBoth(GetStart(*it), start - 1));
            it = _ranges.erase(it);
            _changeCount++;
        } else if (start > s && end < e) {
            _ranges.push_back(SetBoth(GetStart(*it), start - 1));
            _ranges.push_back(SetBoth(end + 1, e));
            it = _ranges.erase(it);
            _changeCount++;
        } else {
            ++it;
        }
    }
    _ranges.sort(CompareRange);
}
