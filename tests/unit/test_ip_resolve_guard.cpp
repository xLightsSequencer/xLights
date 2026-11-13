#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <string>
#include <thread>

#include "utils/ip_utils.h"

using ip_utils::ResolveGuard;

TEST_CASE("ResolveGuard runs the callback while its owner is alive", "[ip]") {
    ResolveGuard guard;
    std::string got;
    auto cb = guard.Wrap([&got](const std::string& r) { got = r; });
    cb("10.0.0.1");
    REQUIRE(got == "10.0.0.1");
}

TEST_CASE("ResolveGuard drops a callback that arrives after Cancel", "[ip]") {
    bool called = false;
    std::function<void(const std::string&)> cb;
    {
        ResolveGuard guard;
        cb = guard.Wrap([&called](const std::string&) { called = true; });
    }
    // the lookup finishing after the owner is gone must not touch it
    cb("10.0.0.1");
    REQUIRE_FALSE(called);
}

TEST_CASE("ResolveGuard copies do not share cancellation", "[ip]") {
    ResolveGuard original;
    bool called = false;
    auto cb = original.Wrap([&called](const std::string&) { called = true; });
    {
        ResolveGuard copy(original);
        copy.Cancel();
    }
    cb("10.0.0.1");
    REQUIRE(called);
}

TEST_CASE("ResolveGuard Cancel waits for a callback already running", "[ip]") {
    ResolveGuard guard;
    std::promise<void> entered;
    std::atomic<bool> release{ false };
    std::atomic<bool> finished{ false };
    auto cb = guard.Wrap([&](const std::string&) {
        entered.set_value();
        while (!release) {
            std::this_thread::yield();
        }
        finished = true;
    });

    std::thread worker([&cb] { cb("10.0.0.1"); });
    entered.get_future().wait();

    auto cancelled = std::async(std::launch::async, [&guard] { guard.Cancel(); });
    REQUIRE(cancelled.wait_for(std::chrono::milliseconds(50)) == std::future_status::timeout);

    release = true;
    cancelled.wait();
    REQUIRE(finished);
    worker.join();
}

TEST_CASE("ResolveGuard Reset drops earlier callbacks and keeps guarding new ones", "[ip]") {
    ResolveGuard guard;
    bool oldCalled = false;
    bool newCalled = false;
    auto oldCb = guard.Wrap([&oldCalled](const std::string&) { oldCalled = true; });

    REQUIRE(guard.Reset());

    auto newCb = guard.Wrap([&newCalled](const std::string&) { newCalled = true; });
    oldCb("10.0.0.1");
    newCb("10.0.0.2");
    REQUIRE_FALSE(oldCalled);
    REQUIRE(newCalled);
}

TEST_CASE("ResolveGuard Reset reports whether a lookup was still pending", "[ip]") {
    ResolveGuard guard;
    REQUIRE_FALSE(guard.Reset());

    auto cb = guard.Wrap([](const std::string&) {});
    cb("10.0.0.1");
    // that lookup already finished, so nothing needs queueing again
    REQUIRE_FALSE(guard.Reset());

    auto pending = guard.Wrap([](const std::string&) {});
    REQUIRE(guard.Reset());
}
