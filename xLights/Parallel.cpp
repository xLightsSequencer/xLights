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

#include "JobPool.h"

ParallelJobPool::ParallelJobPool() : JobPool("parallel_tasks") {
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
        } else if (calcSteps < 1) {
            calcSteps = 1;
        }
        return calcSteps;
    }
    return 1;
}

ParallelJobPool ParallelJobPool::POOL;


class ParallelJob : public Job {
    int max;
    std::function<void(int)>& func;
    int &doneCount;
    std::atomic_int &iteration;
    std::condition_variable &signal;
    std::mutex &mutex;
    const int calcSteps;
public:
    ParallelJob(int m, std::function<void(int)>& f,
                int &dc,
                std::atomic_int &it,
                std::condition_variable &sig,
                std::mutex &mut,
                int cs)
        : max(m), func(f), iteration(it), doneCount(dc), signal(sig), mutex(mut), calcSteps(cs) {}
    virtual ~ParallelJob() {};
    virtual void Process() override {
        try {
            int x;
            while ((x = iteration.fetch_add(1)) < max) {
                func(x);
            }
        } catch (...) {
            //nothing
        }
        std::unique_lock<std::mutex> lock(mutex);
        doneCount++;
        if (doneCount >= calcSteps) {
            signal.notify_all();
        }
    };
    virtual bool DeleteWhenComplete() override { return true; };
    virtual bool SetThreadName() override { return false; }
};

void parallel_for(int min, int max, std::function<void(int)>&& func, int minStep) {
    int calcSteps = ParallelJobPool::POOL.calcSteps(minStep, max - min);
    if (calcSteps == 1) {
        for (int x = min; x < max; x++) {
            func(x);
        }
    } else {
        std::function<void(int)> f(func);
        int doneCount = 0;
        std::atomic_int iteration(min);
        std::condition_variable signal;
        std::mutex mut;
        for (int x = 0; x < calcSteps-1; x++) {
            ParallelJobPool::POOL.PushJob(new ParallelJob(max, f, doneCount, iteration, signal, mut, calcSteps));
        }
        ParallelJob(max, f, doneCount, iteration, signal, mut, calcSteps).Process();
        std::unique_lock<std::mutex> lock(mut);
        while (doneCount < calcSteps) {
            signal.wait_for(lock, std::chrono::nanoseconds(1000000));
        }
    }
}

