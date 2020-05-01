#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <functional>
#include <list>
#include <mutex>
#include <thread>

#include "JobPool.h"


class ParallelJobPool : public JobPool {
public:
    ParallelJobPool();
    
    static ParallelJobPool POOL;
    
    int calcSteps(int minStep, int size);
};


/**
 * Traditional for loop:
 * for(int x = start, x < max; ++x) {  ... use x ...}
 *
 * would convert to:
 * parallel_for(start, max, [&] (int x) {} );
 */
void parallel_for(int start, int max, std::function<void(int)>&& f, int minStep = 1);


/**
 * Traditional for loop:
 * std::list<T> list;
 * int idx = 0;
 * for(T &t : list) { ++idx; ... use t ...}
 *
 * would convert to:
 * std::list<T> list;
 * std::function<void(T&, int)> f = [&](T &t, int idx) { ... use t and idx...}
 * parallel_for(list, f);
 */
template <typename T>
void parallel_for(std::list<T> &list, std::function<void(T&, int)>& f, int minStep = 1) {
    class ParallelListJob : public Job {
        std::atomic_int &doneCount;
        std::function<void(T&, int)>& func;
        std::mutex &lock;
        std::atomic_int &index;
        typename std::list<T>::iterator &iterator;
        const int max;
    public:
        ParallelListJob(std::atomic_int &dc,
                        std::function<void(T&, int)>& f,
                        typename std::list<T>::iterator &it,
                        std::mutex &l,
                        std::atomic_int &idx,
                        int m)
            : Job(), doneCount(dc), func(f), iterator(it), lock(l), index(idx), max(m) {}
        void Process() {
            try {
                while (true) {
                    lock.lock();
                    int idx = index.fetch_add(1);
                    if (idx < max) {
                        T &t = *iterator;
                        ++iterator;
                        lock.unlock();
                        func(t, idx);
                    } else {
                        lock.unlock();
                        doneCount++;
                        return;
                    }
                }
            } catch (...) {
                //nothing
                doneCount++;
            }
        }
    };
    
    int size = list.size();
    int calcSteps = ParallelJobPool::POOL.calcSteps(minStep, size);
    if (calcSteps == 1) {
        int idx = 0;
        for (auto &a : list) {
            f(a, idx);
            idx++;
        }
    } else {
        std::atomic_int doneCount(0);
        std::mutex lock;
        std::atomic_int idx(0);
        typename std::list<T>::iterator it = list.begin();
        
        for (int x = 0; x < calcSteps-1; x++) {
            ParallelJobPool::POOL.PushJob(new ParallelListJob(doneCount, f, it, lock, idx, size));
        }
        ParallelListJob(doneCount, f, it, lock, idx, size).Process();
        while (doneCount < calcSteps) {
            std::this_thread::yield();
        }
    }
}

