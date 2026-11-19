#include <catch2/catch_test_macros.hpp>

#include "render/RemoteRenderScheduler.h"

#include "test_collab_common.h"

#include <map>
#include <random>
#include <string>
#include <tuple>
#include <vector>

// Re-rendering other participants' edits: what renders at once, what waits,
// and what releases waiting work.

namespace {

using R = RemoteRenderScheduler;

struct Call {
    R::Row row;
    int start;
    int end;
    R::Reason why;
    bool operator==(const Call&) const = default;
};

struct Rig {
    std::vector<Call> calls;
    R sched;
    explicit Rig(R::Options o = {}) :
        sched([this](R::Row row, int s, int e, R::Reason why) { calls.push_back({ row, s, e, why }); }, o) {}
    std::vector<Call> Take() {
        auto c = std::move(calls);
        calls.clear();
        return c;
    }
};

} // namespace

TEST_CASE("Remote renders run at once until a visible range is known", "[render][remote_render]") {
    Rig r;
    CHECK_FALSE(r.sched.Mark({ 1, 2 }, 100000, 101000));
    CHECK(r.Take() == std::vector<Call>{ { 1, 100000, 101000, R::Reason::Visible }, { 2, 100000, 101000, R::Reason::Visible } });
    // Whole-sequence changes cover any visible range.
    r.sched.SetVisible(0, 10000);
    CHECK_FALSE(r.sched.Mark({ 3 }, -1, -1));
    CHECK(r.Take() == std::vector<Call>{ { 3, 0, R::kWholeEnd, R::Reason::Visible } });
}

TEST_CASE("Remote renders off screen wait; the margin counts as on screen", "[render][remote_render]") {
    SECTION("default margin is one visible width") {
        Rig r;
        r.sched.SetVisible(10000, 20000);
        r.sched.Mark({ 1 }, 12000, 13000); // inside
        r.sched.Mark({ 2 }, 25000, 26000); // within one width after
        r.sched.Mark({ 3 }, 1000, 1500);   // within one width before
        CHECK(r.sched.Mark({ 4 }, 30001, 31000)); // just past the margin
        CHECK(r.sched.Mark({ 5 }, 0, 0));          // exactly at the margin's edge
        auto c = r.Take();
        REQUIRE(c.size() == 4);
        CHECK(c[0].row == 1);
        CHECK(c[1].row == 2);
        CHECK(c[2].row == 3);
        CHECK(c[3].row == 5);
        CHECK(r.sched.DeferredCount() == 1);
        CHECK(r.sched.Deferred(4) == std::make_pair(30001, 31000));
    }
    SECTION("explicit margin") {
        Rig r({ 0, 1000, 1 });
        r.sched.SetVisible(10000, 20000);
        CHECK(r.sched.Mark({ 1 }, 20001, 20500));
        CHECK(r.calls.empty());
        r.sched.Mark({ 2 }, 20000, 20500);
        CHECK(r.Take() == std::vector<Call>{ { 2, 20000, 20500, R::Reason::Visible } });
        CHECK(r.sched.Deferred(1) == std::make_pair(20001, 20500));
    }
}

TEST_CASE("Remote renders coalesce per row and release the whole row", "[render][remote_render]") {
    Rig r({ 0, 1000, 1 });
    r.sched.SetVisible(0, 10000);
    r.sched.Mark({ 7 }, 50000, 51000);
    r.sched.Mark({ 7 }, 80000, 82000);
    r.sched.Mark({ 8 }, 90000, 91000);
    CHECK(r.calls.empty());
    CHECK(r.sched.Deferred(7) == std::make_pair(50000, 82000));
    // Scrolling onto part of row 7's range renders all of it, once.
    r.sched.SetVisible(79000, 80500);
    CHECK(r.Take() == std::vector<Call>{ { 7, 50000, 82000, R::Reason::Visible } });
    CHECK(r.sched.Deferred(8) == std::make_pair(90000, 91000));
    // A change on a row already waiting, now in view, takes the waiting part along.
    r.sched.SetVisible(0, 10000);
    r.sched.Mark({ 8 }, 5000, 6000);
    CHECK(r.Take() == std::vector<Call>{ { 8, 5000, 91000, R::Reason::Visible } });
    CHECK_FALSE(r.sched.HasDeferred());
}

TEST_CASE("Remote renders scrolled into view go nearest first", "[render][remote_render]") {
    Rig r({ 1000, 1000, 1 });
    r.sched.SetVisible(0, 1000);
    r.sched.Mark({ 2 }, 5000, 5100);
    r.sched.Mark({ 3 }, 7000, 7100);
    r.sched.Mark({ 1 }, 9000, 9100);
    r.sched.Mark({ 4 }, 100000, 100100);
    r.sched.SetVisible(6000, 10000);
    auto c = r.Take();
    REQUIRE(c.size() == 3);
    CHECK(c[0].row == 1); // inside
    CHECK(c[1].row == 3); // inside
    CHECK(c[2].row == 2); // 900 ms before the range, inside the margin
    CHECK(r.sched.DeferredCount() == 1);
    // Setting the same range again does nothing.
    r.sched.SetVisible(6000, 10000);
    CHECK(r.calls.empty());
    // An empty range (a view not laid out yet) is no visible range: the rest renders.
    r.sched.SetVisible(5000, 5000);
    CHECK_FALSE(r.sched.HasVisible());
    CHECK(r.Take() == std::vector<Call>{ { 4, 100000, 100100, R::Reason::Visible } });
    r.sched.SetVisible(0, 1000);
    r.sched.Mark({ 5 }, 100000, 100100);
    CHECK(r.calls.empty());
    r.sched.ClearVisible();
    CHECK(r.Take() == std::vector<Call>{ { 5, 100000, 100100, R::Reason::Visible } });
}

TEST_CASE("Remote renders in the played range start before playback", "[render][remote_render]") {
    Rig r({ 0, 1000, 1 });
    r.sched.SetVisible(0, 1000);
    r.sched.Mark({ 1 }, 20000, 21000);
    r.sched.Mark({ 2 }, 50000, 51000);
    r.sched.Mark({ 3 }, 200000, 201000);
    r.sched.PlaybackStarted(15000, 60000);
    auto c = r.Take();
    REQUIRE(c.size() == 2);
    CHECK(c[0] == Call{ 1, 20000, 21000, R::Reason::Playback });
    CHECK(c[1] == Call{ 2, 50000, 51000, R::Reason::Playback });
    // While playing, changes in the played range render at once even off screen.
    CHECK(r.sched.Mark({ 4 }, 40000, 40500));
    CHECK(r.Take() == std::vector<Call>{ { 4, 40000, 40500, R::Reason::Playback } });
    // Outside it they still wait.
    r.sched.Mark({ 5 }, 100000, 100500);
    CHECK(r.calls.empty());
    r.sched.PlaybackStopped();
    r.sched.Mark({ 6 }, 40000, 40500);
    CHECK(r.calls.empty());
    // Playing everything takes everything.
    r.sched.PlaybackStarted();
    c = r.Take();
    CHECK(c.size() == 3);
    for (auto& k : c) {
        CHECK(k.why == R::Reason::Playback);
    }
    CHECK_FALSE(r.sched.HasDeferred());
}

TEST_CASE("Remote renders wait for a quiet moment and an idle renderer", "[render][remote_render]") {
    Rig r({ 0, 500, 1 });
    r.sched.SetVisible(0, 1000);
    r.sched.Mark({ 1 }, 9000, 9100);
    r.sched.Mark({ 2 }, 3000, 3100);
    r.sched.Mark({ 3 }, 50000, 50100);
    CHECK(r.sched.Idle(1000, true)); // the quiet period starts now
    CHECK(r.sched.Idle(1400, true));
    CHECK(r.calls.empty());
    CHECK(r.sched.Idle(1600, false)); // renderer busy
    CHECK(r.calls.empty());
    CHECK(r.sched.Idle(1600, true));
    CHECK(r.Take() == std::vector<Call>{ { 2, 3000, 3100, R::Reason::Idle } });
    // A new change restarts the quiet period.
    r.sched.Mark({ 4 }, 70000, 70100);
    CHECK(r.sched.Idle(1700, true));
    CHECK(r.calls.empty());
    CHECK(r.sched.Idle(2200, true));
    CHECK(r.Take() == std::vector<Call>{ { 1, 9000, 9100, R::Reason::Idle } });
    CHECK(r.sched.Idle(2300, true));
    CHECK(r.sched.Idle(2400, true) == false);
    auto c = r.Take();
    REQUIRE(c.size() == 2);
    CHECK(c[0].row == 3);
    CHECK(c[1].row == 4);
    CHECK_FALSE(r.sched.Idle(9000, true));
}

TEST_CASE("Remote renders still go idle after the clock jumps backwards", "[render][remote_render]") {
    Rig r({ 0, 500, 1 });
    r.sched.SetVisible(0, 1000);
    r.sched.Mark({ 1 }, 9000, 9100);
    // The quiet period starts late (say, three minutes into playback)...
    CHECK(r.sched.Idle(180000, true));
    CHECK(r.calls.empty());
    // ...then the clock restarts near zero. Half a second of quiet on the
    // new clock is enough; it doesn't wait for the old time to come back.
    CHECK(r.sched.Idle(10, true));
    CHECK(r.calls.empty());
    CHECK(r.sched.Idle(400, true));
    CHECK(r.calls.empty());
    CHECK_FALSE(r.sched.Idle(510, true));
    CHECK(r.Take() == std::vector<Call>{ { 1, 9000, 9100, R::Reason::Idle } });
}

TEST_CASE("Remote render flush and reset", "[render][remote_render]") {
    Rig r({ 0, 1000, 1 });
    r.sched.SetVisible(0, 1000);
    r.sched.Mark({ 1, 2 }, 9000, 9100);
    r.sched.Flush();
    auto c = r.Take();
    REQUIRE(c.size() == 2);
    CHECK(c[0].why == R::Reason::Flush);
    r.sched.Mark({ 3 }, 9000, 9100);
    r.sched.PlaybackStarted(50000, 60000);
    r.sched.Reset();
    CHECK_FALSE(r.sched.HasDeferred());
    CHECK_FALSE(r.sched.HasVisible());
    CHECK_FALSE(r.sched.IsPlaying());
    CHECK(r.calls.empty());
    r.sched.Mark({ 4 }, 9000, 9100);
    CHECK(r.Take().size() == 1);
}

TEST_CASE("Remote render callbacks may mark more changes", "[render][remote_render]") {
    std::vector<Call> calls;
    R* self = nullptr;
    R sched([&](R::Row row, int s, int e, R::Reason why) {
        calls.push_back({ row, s, e, why });
        if (row == 1) {
            self->Mark({ 2 }, 9000, 9100);
            self->Mark({ 3 }, 100, 200);
        }
    });
    self = &sched;
    sched.SetVisible(0, 1000);
    sched.Mark({ 1 }, 9000, 9100);
    sched.SetVisible(8000, 10000);
    REQUIRE(calls.size() == 2);
    CHECK(calls[0].row == 1);
    CHECK(calls[1].row == 2); // marked while row 1 was released, and in view
    CHECK(sched.Deferred(3) == std::make_pair(100, 200));
}

// Random runs: nothing in the played range is ever left waiting while
// playing, and every change is covered by the render that releases its row.
TEST_CASE("Remote renders never leave the played range stale", "[render][remote_render]") {
    for (uint32_t seed = 1; seed <= 200; ++seed) {
        INFO("seed " << seed);
        std::mt19937 rng(seed);
        std::map<R::Row, std::vector<std::pair<int, int>>> marked; // since the row's last render
        bool lost = false;
        std::string lostWhat;
        R sched([&](R::Row row, int s, int e, R::Reason) {
            for (auto [ms, me] : marked[row]) {
                if (ms < s || me > e) {
                    lost = true;
                    lostWhat = "row " + std::to_string(row) + " change " + std::to_string(ms) + "-" + std::to_string(me) + " not in " +
                               std::to_string(s) + "-" + std::to_string(e);
                }
            }
            marked[row].clear();
        }, { (int)(rng() % 3) * 1000 - 1, 200, 1 + rng() % 3 });
        uint64_t now = 0;
        int playStart = 0, playEnd = 0;
        bool playing = false;
        auto waitingInPlay = [&]() {
            for (auto& [row, ranges] : marked) {
                auto d = sched.Deferred(row);
                if (d && playing && d->first <= playEnd && d->second >= playStart) {
                    return true;
                }
            }
            return false;
        };
        for (int step = 0; step < 300; ++step) {
            int op = (int)(rng() % 100);
            if (op < 50) {
                std::vector<R::Row> rows;
                for (int i = 0; i < 1 + (int)(rng() % 3); ++i) {
                    rows.push_back(1 + rng() % 12);
                }
                int s = (int)(rng() % 300000);
                int e = s + (int)(rng() % 20000);
                if (rng() % 20 == 0) {
                    s = e = -1;
                }
                for (auto row : rows) {
                    marked[row].push_back(s < 0 ? std::make_pair(0, R::kWholeEnd) : std::make_pair(s, e));
                }
                sched.Mark(rows, s, e);
            } else if (op < 70) {
                int s = (int)(rng() % 300000);
                sched.SetVisible(s, s + 2000 + (int)(rng() % 30000));
            } else if (op < 78) {
                playStart = (int)(rng() % 300000);
                playEnd = rng() % 4 == 0 ? R::kWholeEnd : playStart + (int)(rng() % 60000);
                playing = true;
                sched.PlaybackStarted(playStart, playEnd == R::kWholeEnd ? -1 : playEnd);
                if (playEnd == R::kWholeEnd) {
                    playStart = 0;
                }
            } else if (op < 84) {
                playing = false;
                sched.PlaybackStopped();
            } else if (op < 98) {
                now += rng() % 300;
                sched.Idle(now, rng() % 3 != 0);
            } else {
                sched.ClearVisible();
            }
            REQUIRE_FALSE(waitingInPlay());
            INFO(lostWhat);
            REQUIRE_FALSE(lost);
        }
        sched.Flush();
        INFO(lostWhat);
        CHECK_FALSE(lost);
        for (auto& [row, ranges] : marked) {
            REQUIRE(ranges.empty());
        }
    }
}

// ---------------------------------------------------------------------------
// The show context's side: row ids to model renders.
// ---------------------------------------------------------------------------

namespace {

class RecordingContext : public HeadlessRenderContext {
public:
    struct Req {
        std::string model;
        int start;
        int end;
        bool direct; // RenderEffectForModel (vs RequestRenderForModel)
        bool operator==(const Req&) const = default;
    };
    std::vector<Req> reqs;
    int timerStarts = 0;
    void RenderEffectForModel(const std::string& model, int s, int e, bool) override { reqs.push_back({ model, s, e, true }); }
    void RequestRenderForModel(const std::string& model, int s, int e) override { reqs.push_back({ model, s, e, false }); }
    void StartOutputTimer() override { ++timerStarts; }
};

} // namespace

TEST_CASE("Remote changes render the changed models through the show context", "[render][remote_render]") {
    CollabTest::Show show;
    RecordingContext ctx;
    REQUIRE(ctx.LoadShowFolder(show.path));
    SequenceFile sf(show.start);
    auto doc = sf.Open(show.path, false, show.start);
    REQUIRE(doc);
    REQUIRE(ctx.LoadSequenceElements(sf, *doc));
    SequenceElements& se = ctx.GetSequenceElements();
    Element* grid = se.GetElement("Grid");
    Element* line = se.GetElement("Line");
    Element* beats = se.GetElement("Beats");
    REQUIRE(grid != nullptr);
    REQUIRE(line != nullptr);
    REQUIRE(beats != nullptr);
    using Req = RecordingContext::Req;

    // No visible range yet: at once.
    ctx.MarkRemoteChange({ grid->GetUid() }, 1000, 2000);
    CHECK(ctx.reqs == std::vector<Req>{ { "Grid", 1000, 2000, false } });
    ctx.reqs.clear();

    ctx.SetVisibleTimeRange(0, 5000);
    ctx.MarkRemoteChange({ grid->GetUid(), line->GetUid() }, 60000, 61000);
    CHECK(ctx.reqs.empty());
    CHECK(ctx.timerStarts == 1);
    // Timing tracks and rows that no longer exist render nothing.
    ctx.MarkRemoteChange({ beats->GetUid(), (SeqObjectId)0x7fffffff1234ull }, 100, 200);
    CHECK(ctx.reqs.empty());
    // Playback starts their renders directly, before the first frame.
    ctx.RemoteRendersPlaybackStarted(55000, 70000);
    CHECK(ctx.reqs == std::vector<Req>{ { "Grid", 60000, 61000, true }, { "Line", 60000, 61000, true } });
    ctx.reqs.clear();
    ctx.RemoteRendersPlaybackStopped();

    // Idle releases them once things are quiet.
    ctx.MarkRemoteChange({ line->GetUid() }, 90000, 91000);
    CHECK(ctx.RemoteRendersIdle(10000));
    CHECK(ctx.reqs.empty());
    CHECK_FALSE(ctx.RemoteRendersIdle(20000));
    CHECK(ctx.reqs == std::vector<Req>{ { "Line", 90000, 91000, false } });
    ctx.reqs.clear();

    // Flush, and a newly loaded sequence forgets what was waiting.
    ctx.MarkRemoteChange({ grid->GetUid() }, 90000, 91000);
    ctx.FlushRemoteRenders();
    CHECK(ctx.reqs == std::vector<Req>{ { "Grid", 90000, 91000, true } });
    ctx.reqs.clear();
    ctx.MarkRemoteChange({ grid->GetUid() }, 90000, 91000);
    CHECK(ctx.GetRemoteRenderScheduler().HasDeferred());
    SequenceFile sf2(show.start);
    auto doc2 = sf2.Open(show.path, false, show.start);
    REQUIRE(ctx.LoadSequenceElements(sf2, *doc2));
    CHECK_FALSE(ctx.GetRemoteRenderScheduler().HasDeferred());
    CHECK_FALSE(ctx.GetRemoteRenderScheduler().HasVisible());
    CHECK(ctx.reqs.empty());
}
