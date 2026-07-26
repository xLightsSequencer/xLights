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

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// A pool whose workers round-robin over every registered index RANGE, rather
// than draining a FIFO of independent jobs.
//
// The frame-parallel render needs a shape a job queue cannot express: several
// model rows each want a long run of frames farmed out, no single row may
// monopolise the pool, and a row wants to GROW its run while that run is
// already executing.  Expressed as queued jobs, the run has to be chopped into
// fixed-size windows with a barrier between them - which costs a straggler wait
// per window and caps how far ahead a row can work.
//
// Here an owner registers one Item (a half-open [next,max) index range plus the
// function to run per index) and workers take the next unclaimed index from a
// ROTATING position in the item list, so a freshly registered item starts being
// serviced after at most one index of other work, however much the other items
// have left.  The owner thread claims and runs indices of its OWN item only: it
// must not disappear into an unrelated item while it holds locks of its own
// (for the render, the row's render lock), and that also guarantees the item
// advances even when every worker is busy elsewhere.
//
// An item also carries its own concurrency cap, so one registration says both
// "here is a long run of work" and "run at most K of it at a time".  The two are
// independent on purpose: the run should be as long as the caller has work for
// (that is what removes the barriers), while K is what the caller sizes
// per-concurrent-index resources on and what bounds the nested parallelism a
// single item can provoke.
//
// The unit of work is expected to be coarse (one whole frame of a model is
// milliseconds), so the registry is guarded by one plain mutex instead of
// lock-free machinery.  The lock is held only for claim bookkeeping - never
// while the work itself runs.
class RangeWorkPool {
public:
    class Item {
    public:
        // Closes the range, waits for every claimed index to finish, and
        // unregisters.  Indices still unclaimed are simply never run.
        ~Item();

        Item(const Item&) = delete;
        Item& operator=(const Item&) = delete;

        // Grow the range.  Safe while the item is executing; the new indices
        // become claimable immediately.  Never shrinks.
        void Extend(int newMax);

        // Stop handing out indices.  In-flight work still completes.
        void Close();

        // Claim and run this item's unclaimed indices on the CALLING thread
        // until none are left, blocking while the concurrency cap is reached
        // (the owner counts against it like any worker).  Returns as soon as the
        // range is exhausted - workers may still be finishing indices they
        // claimed, so the owner can use the gap to Extend().  Never touches
        // another item.
        void RunOwnerShare();

        // Claim and run at most ONE index on the calling thread, returning
        // false without blocking if the range is exhausted or the concurrency
        // cap is reached.  For an owner that has its own serial work to get back
        // to and only wants to fill in when the workers are not keeping up.
        bool RunOwnerOne();

    private:
        friend class RangeWorkPool;
        Item(RangeWorkPool* p, std::function<void(int)>&& f, int first, int max, int concurrency) :
            pool(p), fn(std::move(f)), next(first), maxIdx(max), maxConcurrent(std::max(concurrency, 1)) {}

        RangeWorkPool* const pool;
        std::function<void(int)> fn;
        const int maxConcurrent;
        // All four are guarded by pool->mtx.
        int next;
        int maxIdx;
        int inFlight = 0;
        bool open = true;
    };

    RangeWorkPool(const std::string& name, int workers);
    ~RangeWorkPool();

    RangeWorkPool(const RangeWorkPool&) = delete;
    RangeWorkPool& operator=(const RangeWorkPool&) = delete;

    // Register a range, running at most `concurrency` of its indices at a time
    // (the owner thread counts against that).  The returned Item is live
    // immediately - workers may start calling fn before this returns - so
    // everything fn captures must already be valid.  Destroying the Item closes
    // and drains it.
    std::unique_ptr<Item> Register(std::function<void(int)>&& fn, int first, int max, int concurrency);

private:
    void WorkerLoop();
    bool ClaimLocked(Item*& outItem, int& outIdx);
    void RunOne(Item* it, int idx, std::unique_lock<std::mutex>& lk);
    void NotifyWork(int newIndices);

    const std::string poolName;
    const int numWorkers;

    std::mutex mtx;
    std::condition_variable workSignal; // an item gained claimable indices
    std::condition_variable doneSignal; // an item's in-flight count dropped
    std::vector<Item*> items;
    size_t rotor = 0; // round-robin cursor into items
    bool stopping = false;
};
