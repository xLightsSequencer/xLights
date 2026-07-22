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
#include <chrono>

#include "JobPool.h"

ParallelJobPool::ParallelJobPool(const std::string &name) : JobPool(name) {
    unsigned c = std::thread::hardware_concurrency() - 1; //1 thread is the calling thread
    if (c < 4) {
        c = 4;
    }
    Start(c, c);
}
ParallelJobPool::ParallelJobPool(const std::string &name, int maxThreads) : JobPool(name) {
    int c = std::thread::hardware_concurrency() - 1; //1 thread is the calling thread
    if (c < 4) {
        c = 4;
    }
    Start(c, std::max(c, maxThreads));
}
int ParallelJobPool::calcSteps(int minStep, int total) {
    if (minStep > 0) {
        int calcSteps = total / minStep;
        if (calcSteps > ParallelJobPool::maxSize()) {
            calcSteps = ParallelJobPool::maxSize();
            int i = inFlight;
            // LOTS of things using the parallel pool to a point where
            // the queue is long and we're wasting time/locks managing the queue.
            // In that case, we split into larger blocks.  Still in parallel,
            // but fewer so the existing queue can be reduced
            if (i > (maxNumThreads * 4)) {
                calcSteps = 2;
            } else if (i > (maxNumThreads * 2)) {
                calcSteps /= 2;
            }
        }
        if (calcSteps < 1) {
            calcSteps = 1;
        }
        return calcSteps;
    }
    return 1;
}

ParallelJobPool ParallelJobPool::POOL("parallel_tasks");

void ParallelJobPool::signalDone() {
    // Briefly take poolLock so the increment-of-doneCount that preceded this
    // call happens-before any waiter's predicate re-check. Without the empty
    // critical section, notify_all could fire in the window between a waiter's
    // "doneCount < calcSteps" test and its entry into wait_for, and the wakeup
    // would be lost (the bug the old 1ms poll papered over).
    { std::lock_guard<std::mutex> g(poolLock); }
    poolSignal.notify_all();
}

void ParallelJobPool::waitForDone(std::atomic_int &doneCount, int calcSteps) {
    std::unique_lock<std::mutex> lock(poolLock);
    // The 100ms timeout is belt-and-suspenders only: signalDone() guarantees the
    // wakeup, so the loop normally blocks until notified. The timeout bounds any
    // future lost-wakeup regression to a slow render rather than a hung one.
    while (doneCount.load() < calcSteps) {
        poolSignal.wait_for(lock, std::chrono::milliseconds(100));
    }
}


class ParallelJob : public Job {
    int max;
    std::function<void(int)>& func;
    std::atomic_int &doneCount;
    std::atomic_int &iteration;
    const int calcSteps;
    const int blockSize;
    ParallelJobPool& _pool;
    const std::string threadName;
public:
    ParallelJob(int m, std::function<void(int)>& f,
                std::atomic_int &dc,
                std::atomic_int &it,
                int cs,
                int bs,
                ParallelJobPool& pool = ParallelJobPool::POOL,
                const std::string &tn = "")
        : max(m), func(f), doneCount(dc), iteration(it), calcSteps(cs), blockSize(bs), _pool(pool), threadName(tn) {}
    virtual ~ParallelJob() {};
    virtual void Process() override {
        try {
            int x;
            if (blockSize > 1) {
                while ((x = iteration.fetch_add(blockSize, std::memory_order_relaxed)) < max) {
                    int newM = std::min(x + blockSize, max);
                    while (x < newM) {
                        func(x);
                        x++;
                    }
                }
            } else {
                while ((x = iteration.fetch_add(1, std::memory_order_relaxed)) < max) {
                    func(x);
                }
            }
        } catch (...) {
            //nothing
        }
        int newDoneCount = ++doneCount;
        if (newDoneCount >= calcSteps) {
            _pool.signalDone();
        }
    };
    virtual bool DeleteWhenComplete() override { return true; };
    virtual bool SetThreadName() override { return !threadName.empty(); }
    virtual const std::string GetName() const override {
        return threadName;
    }
    
};

void parallel_for(int min, int max, std::function<void(int)>&& func, int minStep, ParallelJobPool *pool, const std::string &tn) {
    // XL_SERIAL=1 forces every parallel_for to run serially on the calling
    // thread — a determinism diagnostic to separate parallel_for-order bugs
    // from other non-determinism sources (GPU, scheduling).
    static const bool forceSerial = (getenv("XL_SERIAL") != nullptr);
    if (forceSerial) {
        for (int x = min; x < max; x++) {
            func(x);
        }
        return;
    }
    int calcSteps = pool->calcSteps(minStep, max - min);
    if (calcSteps <= 1) {
        for (int x = min; x < max; x++) {
            func(x);
        }
    } else {
        std::function<void(int)> f(func);
        std::atomic_int doneCount = 0;
        std::atomic_int iteration(min);

        // do about 5% at a time, reduces contention on the atomic_int yet keeps unit of
        // work small enough to allow work stealing for faster cores/threads
        int blockSize = (max - min) / (calcSteps * 20);
        if (blockSize < 1) blockSize = 1;
        std::list<Job*> jobs;
        for (int x = 0; x < calcSteps-1; x++) {
            jobs.push_back(new ParallelJob(max, f, doneCount, iteration, calcSteps, blockSize, *pool, tn));
        }
        pool->PushJobs(jobs);
        ParallelJob(max, f, doneCount, iteration, calcSteps, blockSize, *pool).Process();
        pool->waitForDone(doneCount, calcSteps);
    }
}

