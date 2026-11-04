#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <mutex>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

#include "support/TempDir.h"
#include "utils/JobPool.h"
#include "utils/Parallel.h"
#include "utils/RangeWorkPool.h"
#include "utils/ShowGuid.h"
#include "utils/TempFileManager.h"
#include "utils/TraceLog.h"
#include "globals.h"

namespace {

class Latch {
public:
    explicit Latch(int n) : _remaining(n) {}
    void Done() {
        std::lock_guard<std::mutex> l(_m);
        if (--_remaining <= 0) _cv.notify_all();
    }
    bool Wait() {
        std::unique_lock<std::mutex> l(_m);
        return _cv.wait_for(l, std::chrono::seconds(30), [&] { return _remaining <= 0; });
    }

private:
    std::mutex _m;
    std::condition_variable _cv;
    int _remaining;
};

class CountingJob : public Job {
public:
    CountingJob(std::atomic<int>& counter, Latch& latch, std::string name = "job") : _counter(counter), _latch(latch), _name(std::move(name)) {}
    void Process() override {
        ++_counter;
        _latch.Done();
    }
    bool DeleteWhenComplete() override { return true; }
    bool SetThreadName() override { return false; }
    const std::string GetName() const override { return _name; }

private:
    std::atomic<int>& _counter;
    Latch& _latch;
    std::string _name;
};

} // namespace

TEST_CASE("JobPool runs every pushed job exactly once", "[utils]") {
    constexpr int N = 200;
    std::atomic<int> counter{ 0 };
    Latch latch(N);
    {
        JobPool pool("TESTPOOL", 0, 4);
        for (int i = 0; i < N; ++i) {
            pool.PushJob(new CountingJob(counter, latch));
        }
        REQUIRE(latch.Wait());
        CHECK(counter == N);
        pool.Stop();
    }
    CHECK(counter == N);
}

TEST_CASE("JobPool PushJobs batches and respects the thread cap", "[utils]") {
    constexpr int N = 50;
    std::atomic<int> counter{ 0 };
    Latch latch(N);
    JobPool pool("TESTPOOL2", 0, 3);
    std::list<Job*> jobs;
    for (int i = 0; i < N; ++i) {
        jobs.push_back(new CountingJob(counter, latch));
    }
    pool.PushJobs(jobs);
    REQUIRE(latch.Wait());
    CHECK(counter == N);
    CHECK(pool.maxSize() == 3);
    CHECK(pool.size() <= 3);
    pool.Stop();
}

TEST_CASE("JobPool becomes empty after jobs complete", "[utils]") {
    std::atomic<int> counter{ 0 };
    Latch latch(5);
    JobPool pool("TESTPOOL3", 0, 2);
    CHECK(pool.isEmpty());
    for (int i = 0; i < 5; ++i) pool.PushJob(new CountingJob(counter, latch));
    REQUIRE(latch.Wait());
    pool.Stop();
    CHECK(pool.size() == 0);
    CHECK(pool.isEmpty());
}

TEST_CASE("JobPool high priority jobs also run", "[utils]") {
    std::atomic<int> counter{ 0 };
    Latch latch(6);
    JobPool pool("TESTPOOL4", 0, 2);
    for (int i = 0; i < 6; ++i) {
        auto* j = new CountingJob(counter, latch);
        j->SetHighPriority(i % 2 == 0);
        CHECK(j->IsHighPriority() == (i % 2 == 0));
        pool.PushJob(j);
    }
    REQUIRE(latch.Wait());
    CHECK(counter == 6);
    pool.Stop();
}

TEST_CASE("Job defaults", "[utils]") {
    std::atomic<int> c{ 0 };
    Latch l(1);
    CountingJob j(c, l, "named");
    CHECK(j.GetName() == "named");
    CHECK(j.GetStatus().empty());
    CHECK_FALSE(j.IsHighPriority());
}

TEST_CASE("parallel_for visits every index once", "[utils]") {
    constexpr int N = 5000;
    std::vector<std::atomic<int>> hits(N);
    for (auto& h : hits) h = 0;
    parallel_for(0, N, [&](int i) { hits[i]++; });
    for (int i = 0; i < N; ++i) {
        REQUIRE(hits[i] == 1);
    }
}

TEST_CASE("parallel_for honours a non zero start and an empty range", "[utils]") {
    std::vector<int> out(100, 0);
    parallel_for(40, 60, [&](int i) { out[i] = i; });
    for (int i = 0; i < 100; ++i) {
        CHECK(out[i] == ((i >= 40 && i < 60) ? i : 0));
    }

    std::atomic<int> calls{ 0 };
    parallel_for(5, 5, [&](int) { ++calls; });
    parallel_for(7, 3, [&](int) { ++calls; });
    CHECK(calls == 0);
}

TEST_CASE("parallel_for result is deterministic regardless of scheduling", "[utils]") {
    constexpr int N = 2000;
    std::vector<long long> a(N), b(N);
    auto work = [](int i) { return static_cast<long long>(i) * i % 977; };
    parallel_for(0, N, [&](int i) { a[i] = work(i); });
    parallel_for(0, N, [&](int i) { b[i] = work(i); }, 7);
    CHECK(a == b);
    long long expected = 0;
    for (int i = 0; i < N; ++i) expected += work(i);
    CHECK(std::accumulate(a.begin(), a.end(), 0LL) == expected);
}

TEST_CASE("parallel_for with a huge minStep runs serially in order", "[utils]") {
    std::vector<int> order;
    parallel_for(0, 20, [&](int i) { order.push_back(i); }, 1000);
    std::vector<int> expected(20);
    std::iota(expected.begin(), expected.end(), 0);
    CHECK(order == expected);
}

TEST_CASE("parallel_for supports nested loops without deadlock", "[utils]") {
    constexpr int OUTER = 16;
    constexpr int INNER = 64;
    std::atomic<int> total{ 0 };
    parallel_for(0, OUTER, [&](int) {
        parallel_for(0, INNER, [&](int) {
            parallel_for(0, 4, [&](int) { ++total; });
        });
    });
    CHECK(total == OUTER * INNER * 4);
}

TEST_CASE("parallel_for runs on a caller supplied pool", "[utils]") {
    RangeWorkPool pool("test_parfor", 2);
    CHECK(pool.Workers() == 2);
    std::atomic<int> sum{ 0 };
    parallel_for(0, 1000, [&](int i) { sum += i; }, 1, &pool);
    CHECK(sum == 999 * 1000 / 2);
}

TEST_CASE("RangeWorkPool item runs each index once on the owner thread", "[utils]") {
    RangeWorkPool pool("test_range", 3);
    constexpr int N = 300;
    std::vector<std::atomic<int>> hits(N);
    for (auto& h : hits) h = 0;
    {
        auto item = pool.Register([&](int i) { hits[i]++; }, 0, N, 4);
        item->RunOwnerShare();
    }
    for (int i = 0; i < N; ++i) {
        REQUIRE(hits[i] == 1);
    }
    CHECK(pool.LiveItems() == 0);
}

TEST_CASE("RangeWorkPool concurrency cap is respected", "[utils]") {
    RangeWorkPool pool("test_cap", 4);
    std::atomic<int> running{ 0 };
    std::atomic<int> peak{ 0 };
    std::atomic<int> done{ 0 };
    {
        auto item = pool.Register([&](int) {
            int now = ++running;
            int p = peak.load();
            while (now > p && !peak.compare_exchange_weak(p, now)) {
            }
            for (volatile int spin = 0; spin < 2000; spin = spin + 1) {
            }
            --running;
            ++done;
        }, 0, 100, 1);
        item->RunOwnerShare();
    }
    CHECK(done == 100);
    CHECK(peak == 1);
}

TEST_CASE("RangeWorkPool Extend adds work and Close stops new claims", "[utils]") {
    RangeWorkPool pool("test_extend", 2);
    std::atomic<int> count{ 0 };
    {
        auto item = pool.Register([&](int) { ++count; }, 0, 10, 1);
        item->RunOwnerShare();
        item->Extend(25);
        item->RunOwnerShare();
        item->Extend(5);
        item->RunOwnerShare();
    }
    CHECK(count == 25);
}

TEST_CASE("RangeWorkPool RunOwnerOne claims one index at a time", "[utils]") {
    RangeWorkPool pool("test_one", 1);
    std::mutex m;
    std::vector<int> seen;
    auto item = pool.Register([&](int i) {
        std::lock_guard<std::mutex> l(m);
        seen.push_back(i);
    }, 0, 3, 1);
    item->Close();
    CHECK_FALSE(item->RunOwnerOne());
    item.reset();
    CHECK(seen.size() <= 3);
    CHECK(pool.LiveItems() == 0);
}

TEST_CASE("RangeWorkPool reports status lines for live items", "[utils]") {
    RangeWorkPool pool("test_status", 1);
    CHECK(pool.GetStatus().find("test_status") != std::string::npos);
}

TEST_CASE("TempFileManager deletes registered files on cleanup", "[utils]") {
    TempDir d;
    const std::string a = d.Write("a.tmp", "1");
    const std::string b = d.Write("b.tmp", "2");
    const std::string keep = d.Write("keep.tmp", "3");
    TempFileManager& m = TempFileManager::GetTempFileManager();
    m.AddTempFile(a);
    m.AddTempFile(b);
    m.AddTempFile(d.File("never_existed.tmp"));
    m.CleanupTempFiles();
    CHECK_FALSE(std::filesystem::exists(a));
    CHECK_FALSE(std::filesystem::exists(b));
    CHECK(std::filesystem::exists(keep));
    m.CleanupTempFiles();
    CHECK(std::filesystem::exists(keep));
}

TEST_CASE("TempFileManager is a singleton", "[utils]") {
    CHECK(&TempFileManager::GetTempFileManager() == &TempFileManager::GetTempFileManager());
}

TEST_CASE("TraceLog keeps per thread messages and contexts", "[utils]") {
    TraceLog::ClearTraceMessages();
    TraceLog::AddTraceMessage("one");
    TraceLog::AddTraceMessage("two");
    std::list<std::string> msgs;
    TraceLog::GetTraceMessages(msgs);
    REQUIRE(msgs.size() == 2);
    CHECK(msgs.front() == "one");
    CHECK(msgs.back() == "two");

    TraceLog::PushTraceContext();
    TraceLog::AddTraceMessage("inner");
    msgs.clear();
    TraceLog::GetTraceMessages(msgs);
    CHECK(msgs.size() == 4);
    TraceLog::PopTraceContext();
    msgs.clear();
    TraceLog::GetTraceMessages(msgs);
    CHECK(msgs == std::list<std::string>{ "one", "two" });

    TraceLog::ClearTraceMessages();
    msgs.clear();
    TraceLog::GetTraceMessages(msgs);
    CHECK(msgs.empty());
}

TEST_CASE("TraceLog keeps only the most recent messages", "[utils]") {
    TraceLog::ClearTraceMessages();
    for (int i = 0; i < 50; ++i) TraceLog::AddTraceMessage("m" + std::to_string(i));
    std::list<std::string> msgs;
    TraceLog::GetTraceMessages(msgs);
    CHECK(msgs.size() == 20);
    CHECK(msgs.back() == "m49");
    CHECK(msgs.front() == "m30");
    TraceLog::ClearTraceMessages();
}

TEST_CASE("TraceLog messages are separate per thread", "[utils]") {
    TraceLog::ClearTraceMessages();
    TraceLog::AddTraceMessage("main-only");
    std::list<std::string> other;
    std::thread t([&] {
        TraceLog::AddTraceMessage("worker-only");
        TraceLog::GetTraceMessages(other);
    });
    t.join();
    CHECK(other == std::list<std::string>{ "worker-only" });
    std::list<std::string> mine;
    TraceLog::GetTraceMessages(mine);
    CHECK(mine == std::list<std::string>{ "main-only" });
    TraceLog::ClearTraceMessages();
}

TEST_CASE("ShowGuid reads the GUID from rgbeffects", "[utils]") {
    TempDir d;
    CHECK(ShowGuid::ReadFromShowFolder("").empty());
    CHECK(ShowGuid::ReadFromShowFolder(d.path.string()).empty());

    d.Write(XLIGHTS_RGBEFFECTS_FILE, R"(<xrgb><settings><Foo value="1"/><ShowGUID value="abc-123"/></settings></xrgb>)");
    CHECK(ShowGuid::ReadFromShowFolder(d.path.string()) == "abc-123");
}

TEST_CASE("ShowGuid accepts the legacy root and tolerates a missing id", "[utils]") {
    TempDir legacy;
    legacy.Write(XLIGHTS_RGBEFFECTS_FILE, R"(<xlights><settings><ShowGUID value="legacy-1"/></settings></xlights>)");
    CHECK(ShowGuid::ReadFromShowFolder(legacy.path.string()) == "legacy-1");

    TempDir none;
    none.Write(XLIGHTS_RGBEFFECTS_FILE, R"(<xrgb><settings><Other value="1"/></settings></xrgb>)");
    CHECK(ShowGuid::ReadFromShowFolder(none.path.string()).empty());

    TempDir wrong;
    wrong.Write(XLIGHTS_RGBEFFECTS_FILE, R"(<other><settings><ShowGUID value="x"/></settings></other>)");
    CHECK(ShowGuid::ReadFromShowFolder(wrong.path.string()).empty());

    TempDir broken;
    broken.Write(XLIGHTS_RGBEFFECTS_FILE, "<xrgb><settings>");
    CHECK(ShowGuid::ReadFromShowFolder(broken.path.string()).empty());
}
