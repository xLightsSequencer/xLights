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

#include <atomic>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

class RangeWorkPool;

// The pool backing the int parallel_for.  Workers round-robin over every live
// loop rather than draining a queue of them, so a loop that starts while another
// is already running gets pool threads within one BLOCK of the running loop
// instead of after all of it.  The old FIFO could not do that: a queued job ran
// until its whole range was exhausted, so it owned its thread for the loop's
// full duration and a later caller was left running single-threaded on its own
// thread.  Nested loops are safe on one pool because a caller drains its own
// registration before it ever blocks, so an inner loop always progresses.
RangeWorkPool& ParallelForPool();

/**
 * Traditional for loop:
 * for(int x = start, x < max; ++x) {  ... use x ...}
 *
 * would convert to:
 * parallel_for(start, max, [&] (int x) {} );
 */
void parallel_for(int start, int max, std::function<void(int)>&& f, int minStep = 1,
                  RangeWorkPool *pool = nullptr);


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
    const int size = (int)list.size();
    // A list has no random access, so the pool index only meters how many pulls
    // happen; the element-to-index pairing comes off a shared cursor exactly as
    // the serial loop would make it.  The pool hands out each index once, so the
    // cursor is stepped exactly `size` times.
    std::mutex lock;
    int cursor = 0;
    typename std::list<T>::iterator it = list.begin();
    parallel_for(0, size, [&](int) {
        lock.lock();
        T &t = *it;
        ++it;
        const int idx = cursor++;
        lock.unlock();
        f(t, idx);
    }, minStep);
}

