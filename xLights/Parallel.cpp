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

const static std::string EMPTY_STRING = "";

ParallelJobPool::ParallelJobPool() : JobPool("parallel_tasks") {
    unsigned c = std::thread::hardware_concurrency();
    if (c <= 4) {
        c = 4;
    }
    Start(c);
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
    std::atomic_int &doneCount;
    std::atomic_int &iteration;
public:
    ParallelJob(int m, std::function<void(int)>& f,
                std::atomic_int &dc, std::atomic_int &it)
        : max(m), func(f), iteration(it), doneCount(dc) {}
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
        doneCount++;
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
        std::atomic_int doneCount(0);
        std::atomic_int iteration(min);
        for (int x = 0; x < calcSteps-1; x++) {
            ParallelJobPool::POOL.PushJob(new ParallelJob(max, f, doneCount, iteration));
        }
        ParallelJob(max, f, doneCount, iteration).Process();
        while (doneCount < calcSteps) {
            std::this_thread::yield();
        }
    }
}

