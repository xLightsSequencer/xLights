/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/
//  Created by Daniel Kulp on 8/2/18.
//  Copyright © 2018 Daniel Kulp. All rights reserved.

#include "Parallel.h"
#include <thread>
#include <algorithm>

#include "JobPool.h"

ParallelJobPool::ParallelJobPool(const std::string &name) : JobPool(name) {
    unsigned c = std::thread::hardware_concurrency() - 1; //1 thread is the calling thread
    if (c < 4) {
        c = 4;
    }
    Start(c, c);
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


class ParallelJob : public Job {
    int max;
    std::function<void(int)>& func;
    std::atomic_int &doneCount;
    std::atomic_int &iteration;
    const int calcSteps;
    const int blockSize;
public:
    ParallelJob(int m, std::function<void(int)>& f,
                std::atomic_int &dc,
                std::atomic_int &it,
                int cs,
                int bs)
        : max(m), func(f), iteration(it), doneCount(dc), calcSteps(cs), blockSize(bs) {}
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
            ParallelJobPool::POOL.poolSignal.notify_all();
        }
    };
    virtual bool DeleteWhenComplete() override { return true; };
    virtual bool SetThreadName() override { return false; }
};

void parallel_for(int min, int max, std::function<void(int)>&& func, int minStep, ParallelJobPool *pool) {
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
        for (int x = 0; x < calcSteps-1; x++) {
            pool->PushJob(new ParallelJob(max, f, doneCount, iteration, calcSteps, blockSize));
        }
        ParallelJob(max, f, doneCount, iteration, calcSteps, blockSize).Process();
        std::unique_lock<std::mutex> lock(pool->poolLock);
        while (doneCount < calcSteps) {
            pool->poolSignal.wait_for(lock, std::chrono::nanoseconds(1000000));
        }
    }
}

