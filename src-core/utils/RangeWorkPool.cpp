/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RangeWorkPool.h"

#include "AutoReleasePool.h"

#include <algorithm>
#include <sstream>
#include <thread>

#ifndef _WIN32
#include <pthread.h>
#endif

namespace {
void NameThisThread(const std::string& name) {
#ifdef __APPLE__
    pthread_setname_np(name.c_str());
#elif !defined(_WIN32)
    pthread_setname_np(pthread_self(), name.c_str());
#endif
}
} // namespace

RangeWorkPool::RangeWorkPool(const std::string& name, int workers) :
    poolName(name), numWorkers(std::max(workers, 1)) {
    for (int x = 0; x < numWorkers; ++x) {
        // Detached: the pool outlives any single render and, like JobPool's
        // workers, must not become a join dependency of static destruction.
        std::thread(&RangeWorkPool::WorkerLoop, this).detach();
    }
}

RangeWorkPool::~RangeWorkPool() {
    {
        std::lock_guard<std::mutex> lg(mtx);
        stopping = true;
    }
    workSignal.notify_all();
}

std::unique_ptr<RangeWorkPool::Item> RangeWorkPool::Register(std::function<void(int)>&& fn, int first, int max, int concurrency) {
    std::unique_ptr<Item> item(new Item(this, std::move(fn), first, max, concurrency));
    int wake;
    bool all;
    {
        std::lock_guard<std::mutex> lg(mtx);
        items.push_back(item.get());
        liveItems.store((int)items.size(), std::memory_order_relaxed);
        wake = WakeCountLocked(std::min(max - first, concurrency));
        all = wake >= idleWorkers;
    }
    // Notified with the lock dropped: a woken worker's first act is to take mtx,
    // so signalling under it just makes it block again on wakeup.
    WakeWorkers(wake, all);
    return item;
}

// One notify_all is a single syscall that the kernel expands to every waiter;
// N notify_one calls are N syscalls.  Waking each worker individually is only
// worth it when the new work would not fill the pool anyway - which is the frame
// case (a window registers a couple of indices) but not the parallel_for case (a
// loop registers hundreds of blocks, thousands of times a render, and genuinely
// wants every idle worker).  Doing it one-at-a-time there put ~15 syscalls on
// every loop.
void RangeWorkPool::WakeWorkers(int wake, bool all) {
    if (wake <= 0) {
        return;
    }
    if (all) {
        workSignal.notify_all();
    } else {
        for (int x = 0; x < wake; ++x) {
            workSignal.notify_one();
        }
    }
}

// Wake no more workers than there is new work to claim, and no more than are
// actually asleep.  A notify_all per registration wakes every worker for a
// two-index range and the ones that find nothing pay a context switch to go
// straight back to sleep.  Reading idleWorkers under mtx is what makes the
// wakeup impossible to lose: a worker that has not yet been counted idle has not
// yet released mtx, so its own ClaimLocked still runs after this registration is
// visible and it finds the work instead of parking.
int RangeWorkPool::WakeCountLocked(int newIndices) const {
    return std::min({ newIndices, numWorkers, idleWorkers });
}

std::string RangeWorkPool::GetStatus() {
    std::stringstream ret;
    std::lock_guard<std::mutex> lg(mtx);
    ret << poolName << ": " << numWorkers << " workers, " << idleWorkers << " idle, "
        << items.size() << " live ranges\n";
    for (const Item* it : items) {
        ret << "  [" << it->next << "," << it->maxIdx << ") inFlight=" << it->inFlight
            << "/" << it->maxConcurrent << (it->open ? "" : " closed") << "\n";
    }
    return ret.str();
}

// Caller holds mtx.  Scans from the rotor so consecutive claims spread across
// the live items instead of draining the first one.
bool RangeWorkPool::ClaimLocked(Item*& outItem, int& outIdx) {
    const size_t n = items.size();
    for (size_t k = 0; k < n; ++k) {
        size_t pos = (rotor + k) % n;
        Item* it = items[pos];
        if (it->open && it->next < it->maxIdx && it->inFlight < it->maxConcurrent) {
            outIdx = it->next++;
            ++it->inFlight;
            rotor = pos + 1;
            outItem = it;
            return true;
        }
    }
    return false;
}

// Runs one claimed index with the lock DROPPED, then re-takes it to retire the
// claim.  `it` cannot be destroyed across the gap: ~Item waits for inFlight to
// reach 0 while holding mtx, and this claim keeps it above 0.
void RangeWorkPool::RunOne(Item* it, int idx, std::unique_lock<std::mutex>& lk) {
    lk.unlock();
    try {
        // One pool per index, matching JobPool's one-per-job contract. These
        // workers are permanent and detached, so anything autoreleased by fn
        // with no pool in place is held for the life of the PROCESS, not the
        // life of the work: a render frame that touches Metal or the video
        // decoder autoreleases megabytes, and the frame-parallel path runs
        // whole frames through here. Push/pop are both on this thread inside
        // this scope, so the runtime's LIFO + affinity rules hold.
        AutoReleasePool pool;
        it->fn(idx);
    } catch (...) {
        // Matches parallel_for: one bad index must not take a worker down.
        // The owner sees the gap through whatever completion state fn keeps.
    }
    lk.lock();
    --it->inFlight;
    // Capacity just freed under this item's concurrency cap, so exactly one more
    // index of it became claimable - and an owner may be blocked on that cap or
    // on the drain in ~Item.  Both notifies are skipped when nobody is parked:
    // this runs once per index, and for parallel_for an index is one block of a
    // loop, so two unconditional syscalls here would dominate the claim.
    if (idleWorkers > 0) {
        workSignal.notify_one();
    }
    if (it->capWaiters > 0 || (it->drainWaiters > 0 && it->inFlight == 0)) {
        it->doneCv.notify_all();
    }
}

void RangeWorkPool::WorkerLoop() {
    NameThisThread(poolName);
    std::unique_lock<std::mutex> lk(mtx);
    while (!stopping) {
        Item* it = nullptr;
        int idx = 0;
        if (ClaimLocked(it, idx)) {
            RunOne(it, idx, lk);
        } else {
            ++idleWorkers;
            workSignal.wait(lk);
            --idleWorkers;
        }
    }
}

RangeWorkPool::Item::~Item() {
    std::unique_lock<std::mutex> lk(pool->mtx);
    open = false;
    while (inFlight > 0) {
        ++drainWaiters;
        doneCv.wait(lk);
        --drainWaiters;
    }
    auto& v = pool->items;
    v.erase(std::remove(v.begin(), v.end(), this), v.end());
    pool->liveItems.store((int)v.size(), std::memory_order_relaxed);
    if (pool->rotor > v.size()) {
        pool->rotor = 0;
    }
}

void RangeWorkPool::Item::Extend(int newMax) {
    int wake = 0;
    bool all = false;
    {
        std::lock_guard<std::mutex> lg(pool->mtx);
        if (newMax <= maxIdx) {
            return;
        }
        wake = pool->WakeCountLocked(newMax - maxIdx);
        all = wake >= pool->idleWorkers;
        maxIdx = newMax;
    }
    pool->WakeWorkers(wake, all);
}

void RangeWorkPool::Item::Close() {
    std::lock_guard<std::mutex> lg(pool->mtx);
    open = false;
}

bool RangeWorkPool::Item::RunOwnerOne() {
    std::unique_lock<std::mutex> lk(pool->mtx);
    if (!open || next >= maxIdx || inFlight >= maxConcurrent) {
        return false;
    }
    int idx = next++;
    ++inFlight;
    pool->RunOne(this, idx, lk);
    return true;
}

void RangeWorkPool::Item::RunOwnerShare() {
    std::unique_lock<std::mutex> lk(pool->mtx);
    while (open && next < maxIdx) {
        if (inFlight >= maxConcurrent) {
            // Workers are running the cap's worth already.  Wait rather than
            // return: the caller treats a return as "range exhausted" and would
            // close the item with indices still unclaimed.
            ++capWaiters;
            doneCv.wait(lk);
            --capWaiters;
            continue;
        }
        int idx = next++;
        ++inFlight;
        pool->RunOne(this, idx, lk);
    }
}
