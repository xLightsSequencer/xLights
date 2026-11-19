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

// Re-renders for edits that arrive from other people in a shared session,
// ordered by what this screen shows. A change that touches the visible time
// range (or the range being played) is rendered at once; one that doesn't
// waits until it scrolls into view, playback starts, or the app has been
// idle for a while, and is then rendered nearest-first.
//
// Rows are coalesced: a row's waiting changes are one range (their union),
// and once any part of it is wanted the whole range is rendered, the way the
// render engine widens a row's render to its whole dirty range anyway.
//
// It only decides when; the owner's callback does the rendering. Main
// thread only.

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <utility>
#include <vector>

class RemoteRenderScheduler {
public:
    using Row = uint64_t;
    enum class Reason {
        Visible,  // in (or near) the visible range, or no visible range is known
        Playback, // in the range being played
        Idle,     // released while nothing else was happening
        Flush     // everything, e.g. before writing the fseq
    };
    using RenderFn = std::function<void(Row row, int startMS, int endMS, Reason why)>;

    // The end used for "to the end of the sequence".
    static constexpr int kWholeEnd = 99999999;

    struct Options {
        // Changes this close to the visible range count as visible, so a
        // short scroll finds them rendered. -1: one visible width.
        int marginMs = -1;
        // Idle work starts after this long without a change or a scroll.
        uint64_t idleDelayMs = 1000;
        // Rows released per idle step (each step needs an idle renderer).
        size_t idleRowsPerStep = 1;
    };

    explicit RemoteRenderScheduler(RenderFn render);
    RemoteRenderScheduler(RenderFn render, Options opts);

    // Rows changed over [startMS, endMS] (either < 0: the whole sequence).
    // Renders what is wanted now; returns true if anything is left waiting.
    bool Mark(const std::vector<Row>& rows, int startMS, int endMS);

    // The time range on screen. Releases what it brings into view. Until the
    // first call (and after ClearVisible, or an empty range) nothing is
    // deferred.
    void SetVisible(int startMS, int endMS);
    void ClearVisible();
    bool HasVisible() const { return _haveVisible; }

    // Releases everything waiting in the played range (startMS / endMS < 0:
    // the whole sequence) before the first frame is shown; while playing,
    // changes in that range render at once.
    void PlaybackStarted(int startMS = -1, int endMS = -1);
    void PlaybackStopped();
    bool IsPlaying() const { return _playing; }

    // Call regularly. Once nothing has been marked or scrolled for
    // idleDelayMs and the renderer is idle, releases the waiting rows
    // nearest the visible range (idleRowsPerStep per call). True while
    // anything is still waiting. nowMs is a steady clock; a value before the
    // last one starts the quiet period again.
    bool Idle(uint64_t nowMs, bool rendererIdle);

    // Releases everything that is waiting.
    void Flush();
    // The sequence was closed or replaced: forget everything, including the
    // visible range and playback.
    void Reset();

    bool HasDeferred() const { return !_waiting.empty(); }
    size_t DeferredCount() const { return _waiting.size(); }
    std::optional<std::pair<int, int>> Deferred(Row row) const;

private:
    struct Range {
        int start = 0;
        int end = 0;
    };
    static Range Normalize(int startMS, int endMS);
    static bool Overlaps(const Range& a, const Range& b);
    bool InView(const Range& r) const;
    bool InPlay(const Range& r) const;
    int64_t Distance(const Range& r) const;
    // Waiting rows, nearest the visible range first.
    std::vector<Row> Ordered() const;
    void Release(Row row, Reason why);
    // Releases every waiting row that `want` accepts, nearest first.
    void ReleaseWhere(const std::function<bool(const Range&)>& want, Reason why);

    RenderFn _render;
    Options _opts;
    std::map<Row, Range> _waiting;
    bool _haveVisible = false;
    Range _visible;
    bool _playing = false;
    Range _play;
    bool _activity = true;
    uint64_t _quietSince = 0;
};
