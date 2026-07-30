/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
//  Created by Daniel Kulp on 8/2/18.
//  Copyright © 2018 Daniel Kulp. All rights reserved.

#include "Parallel.h"
#include <thread>
#include <algorithm>

#include "RangeWorkPool.h"

RangeWorkPool& ParallelForPool() {
    // Deliberately leaked: the workers are detached and must not become a
    // static-destruction ordering dependency (same reasoning as JobPool.cpp's
    // thread-name map and the frame pool in RenderEngine).
    static RangeWorkPool* pool = []() {
        int c = (int)std::thread::hardware_concurrency() - 1; // 1 thread is the caller
        // Sizing override, mirroring XL_PARALLEL_FRAME_WORKERS.  Deliberately
        // allows very small pools: nested loops must still complete when the
        // nesting is deeper than the worker count, which is the property worth
        // being able to test.
        const char* e = getenv("XL_PARALLEL_WORKERS");
        if (e != nullptr) {
            c = (int)strtol(e, nullptr, 10);
        }
        return new RangeWorkPool("parallel_tasks", std::max(c, 1));
    }();
    return *pool;
}

// How many of the loop's blocks to spread across the pool.  The oversubscription
// taper is NOT a leftover of the old job queue - it is load-bearing.  Under the
// render's nesting (frame rows -> per-model buffers -> an effect's own loop)
// hundreds of loops are live at once, and every one of them asking for full
// width piles far more threads onto the pool than there are cores; the work does
// not grow but the contention does, and it shows up as system time rather than
// user time.  So a loop that arrives while the pool is already saturated asks
// for less, exactly as the queue-depth version did.
static int ParallelSteps(int minStep, int total, const RangeWorkPool* pool) {
    if (minStep <= 0) {
        return 1;
    }
    const int workers = pool->Workers();
    int steps = std::min(total / minStep, workers + 1);
    static const bool noTaper = (getenv("XL_PARFOR_NO_TAPER") != nullptr);
    if (!noTaper) {
        const int live = pool->LiveItems();
        if (live > workers * 4) {
            steps = 2;
        } else if (live > workers * 2) {
            steps /= 2;
        }
    }
    return std::max(steps, 1);
}

void parallel_for(int min, int max, std::function<void(int)>&& func, int minStep, RangeWorkPool *pool) {
    // XL_SERIAL=1 forces every parallel_for to run serially on the calling
    // thread — a determinism diagnostic to separate parallel_for-order bugs
    // from other non-determinism sources (GPU, scheduling).
    static const bool forceSerial = (getenv("XL_SERIAL") != nullptr);
    const int total = max - min;
    if (pool == nullptr) {
        pool = &ParallelForPool();
    }
    const int steps = forceSerial ? 1 : ParallelSteps(minStep, total, pool);
    if (steps <= 1) {
        for (int x = min; x < max; x++) {
            func(x);
        }
        return;
    }

    // Blocks per participating worker.  This trades two things directly against
    // each other: a claim costs a pool-mutex round trip, while a block is the
    // fairness quantum (a loop registering now waits at most one block of this
    // one, not all of it) and the work-stealing quantum that evens out uneven
    // blocks.  The old job queue could afford 20 because it claimed with a
    // relaxed atomic and re-entered the pool zero times.
    static const int blocksPerStep = []() {
        const char* e = getenv("XL_PARFOR_BLOCKS_PER_STEP");
        return e != nullptr ? std::max((int)strtol(e, nullptr, 10), 1) : 20;
    }();
    int blockSize = total / (steps * blocksPerStep);
    if (blockSize < 1) {
        blockSize = 1;
    }
    const int blocks = (total + blockSize - 1) / blockSize;

    // func outlives the item: it is a reference to the caller's argument and the
    // item is closed and drained by its destructor below, before we return.
    auto item = pool->Register([&func, min, max, blockSize](int b) {
        int x = min + (b * blockSize);
        const int e = std::min(x + blockSize, max);
        while (x < e) {
            func(x);
            ++x;
        }
    // Registered UNCAPPED.  The frame pool caps concurrency because each
    // concurrent frame holds a clone buffer; a loop's block has no such
    // per-index resource, and the pool cannot run more than its worker count
    // anyway.  Capping here only made the owner park on the cap and be woken
    // again for every block its own loop retired - pure futex traffic for a
    // limit that never needed enforcing.  Fairness between loops comes from the
    // rotor, not from the cap.
    }, 0, blocks, blocks);
    item->RunOwnerShare();
    // ~Item closes the range and waits for the in-flight tail, which is the join
    // the old doneCount/waitForDone protocol provided.
}

