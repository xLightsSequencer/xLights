/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RemoteRenderScheduler.h"

#include <algorithm>

RemoteRenderScheduler::RemoteRenderScheduler(RenderFn render) :
    _render(std::move(render)) {
}

RemoteRenderScheduler::RemoteRenderScheduler(RenderFn render, Options opts) :
    _render(std::move(render)), _opts(opts) {
}

RemoteRenderScheduler::Range RemoteRenderScheduler::Normalize(int startMS, int endMS) {
    if (startMS < 0 || endMS < 0) {
        return { 0, kWholeEnd };
    }
    if (endMS < startMS) {
        std::swap(startMS, endMS);
    }
    return { startMS, endMS };
}

bool RemoteRenderScheduler::Overlaps(const Range& a, const Range& b) {
    return a.start <= b.end && a.end >= b.start;
}

bool RemoteRenderScheduler::InView(const Range& r) const {
    if (!_haveVisible) {
        return true;
    }
    int64_t margin = _opts.marginMs >= 0 ? _opts.marginMs : (int64_t)_visible.end - _visible.start;
    int64_t vs = (int64_t)_visible.start - margin;
    int64_t ve = (int64_t)_visible.end + margin;
    return r.start <= ve && r.end >= vs;
}

bool RemoteRenderScheduler::InPlay(const Range& r) const {
    return _playing && Overlaps(r, _play);
}

int64_t RemoteRenderScheduler::Distance(const Range& r) const {
    if (!_haveVisible) {
        return 0;
    }
    if (r.end < _visible.start) {
        return (int64_t)_visible.start - r.end;
    }
    if (r.start > _visible.end) {
        return (int64_t)r.start - _visible.end;
    }
    return 0;
}

std::vector<RemoteRenderScheduler::Row> RemoteRenderScheduler::Ordered() const {
    std::vector<std::pair<int64_t, Row>> d;
    d.reserve(_waiting.size());
    for (const auto& [row, r] : _waiting) {
        d.emplace_back(Distance(r), row);
    }
    std::sort(d.begin(), d.end());
    std::vector<Row> out;
    out.reserve(d.size());
    for (const auto& [dist, row] : d) {
        out.push_back(row);
    }
    return out;
}

void RemoteRenderScheduler::Release(Row row, Reason why) {
    auto it = _waiting.find(row);
    if (it == _waiting.end()) {
        return;
    }
    Range r = it->second;
    // Off the list before the callback, which may mark more changes.
    _waiting.erase(it);
    if (_render) {
        _render(row, r.start, r.end, why);
    }
}

void RemoteRenderScheduler::ReleaseWhere(const std::function<bool(const Range&)>& want, Reason why) {
    for (Row row : Ordered()) {
        auto it = _waiting.find(row);
        if (it != _waiting.end() && want(it->second)) {
            Release(row, why);
        }
    }
}

bool RemoteRenderScheduler::Mark(const std::vector<Row>& rows, int startMS, int endMS) {
    _activity = true;
    Range change = Normalize(startMS, endMS);
    for (Row row : rows) {
        Range r = change;
        if (auto it = _waiting.find(row); it != _waiting.end()) {
            r.start = std::min(r.start, it->second.start);
            r.end = std::max(r.end, it->second.end);
        }
        _waiting[row] = r;
        if (InPlay(r)) {
            Release(row, Reason::Playback);
        } else if (InView(r)) {
            Release(row, Reason::Visible);
        }
    }
    return !_waiting.empty();
}

void RemoteRenderScheduler::SetVisible(int startMS, int endMS) {
    if (startMS >= 0 && endMS >= 0 && endMS <= startMS) {
        // A view not laid out yet; deferring against it would defer everything.
        ClearVisible();
        return;
    }
    Range v = Normalize(startMS, endMS);
    if (_haveVisible && v.start == _visible.start && v.end == _visible.end) {
        return;
    }
    _haveVisible = true;
    _visible = v;
    _activity = true;
    ReleaseWhere([this](const Range& r) { return InView(r); }, Reason::Visible);
}

void RemoteRenderScheduler::ClearVisible() {
    _haveVisible = false;
    _activity = true;
    ReleaseWhere([](const Range&) { return true; }, Reason::Visible);
}

void RemoteRenderScheduler::PlaybackStarted(int startMS, int endMS) {
    _playing = true;
    _play = Normalize(startMS, endMS);
    ReleaseWhere([this](const Range& r) { return InPlay(r); }, Reason::Playback);
}

void RemoteRenderScheduler::PlaybackStopped() {
    _playing = false;
}

bool RemoteRenderScheduler::Idle(uint64_t nowMs, bool rendererIdle) {
    // A clock that went backwards would otherwise hold idle work back until
    // it caught up again.
    if (_activity || nowMs < _quietSince) {
        _activity = false;
        _quietSince = nowMs;
    }
    if (_waiting.empty()) {
        return false;
    }
    if (!rendererIdle || nowMs < _quietSince + _opts.idleDelayMs) {
        return true;
    }
    std::vector<Row> order = Ordered();
    size_t n = std::min(order.size(), std::max<size_t>(1, _opts.idleRowsPerStep));
    for (size_t i = 0; i < n; ++i) {
        Release(order[i], Reason::Idle);
    }
    return !_waiting.empty();
}

void RemoteRenderScheduler::Flush() {
    ReleaseWhere([](const Range&) { return true; }, Reason::Flush);
}

void RemoteRenderScheduler::Reset() {
    _waiting.clear();
    _haveVisible = false;
    _playing = false;
    _activity = true;
}

std::optional<std::pair<int, int>> RemoteRenderScheduler::Deferred(Row row) const {
    auto it = _waiting.find(row);
    if (it == _waiting.end()) {
        return std::nullopt;
    }
    return std::make_pair(it->second.start, it->second.end);
}
