//
//  Parallel.cpp
//  xLights
//
//  Created by Daniel Kulp on 8/2/18.
//  Copyright © 2018 Daniel Kulp. All rights reserved.
//

#include "Parallel.h"
#include <thread>

#include "JobPool.h"

#ifdef NO_PARALLEL
void parallel_for(int min, int max, std::function<void(int)>&& f, int minStep) {
    for (int x = min; x <= max; ++x) {
        f(x);
    }
}
#else

const static std::string EMPTY_STRING = "";

static class ParallelJobPool : public JobPool {
public:
    ParallelJobPool() : JobPool("parallel_tasks") {
        unsigned c = std::thread::hardware_concurrency();
        if (c <= 4) {
            c = 4;
        }
        Start(c);
    }
    
} pool;

class ParallelJob : public Job {
    int min;
    int max;
    std::function<void(int)>& func;
    std::atomic_int &doneCount;
public:
    ParallelJob(int s, int m, std::function<void(int)>& f, std::atomic_int &dc)
        : min(s), max(m), func(f), doneCount(dc) {}
    virtual ~ParallelJob() {};
    virtual void Process() override {
        for (int x = min; x < max; ++x) {
            func(x);
        }
        doneCount++;
    };
    virtual std::string GetStatus() override { return ""; }
    virtual bool DeleteWhenComplete() override { return true; };
    
    virtual bool SetThreadName() override { return false; }
    virtual const std::string GetName() const override { return EMPTY_STRING; };
};

void parallel_for(int min, int max, std::function<void(int)>&& func, int minStep) {
    int total = max - min;
    int calcSteps = total / minStep;
    
    if (minStep <= 0) {
        calcSteps = 1;
    }
    if (calcSteps > pool.maxSize()) {
        calcSteps = pool.maxSize();
    }
    if (calcSteps < 1) {
        calcSteps = 1;
    }
    int stepSize = total / calcSteps;
    if (stepSize < 1) {
        stepSize = 1;
    }
    if (calcSteps == 1) {
        for (int x = min; x < max; x++) {
            func(x);
        }
    } else {
        std::function<void(int)> f(func);
        std::atomic_int doneCount(0);
        std::vector<ParallelJob> jobs;
        for (int x = 0; x < calcSteps-1; x++) {
            pool.PushJob(new ParallelJob(min, min + stepSize, f, doneCount));
            min += stepSize;
        }
        ParallelJob(min, max, f, doneCount).Process();
        while (doneCount < calcSteps) {
            std::this_thread::yield();
        }
    }
}




#endif
