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

#include <functional>

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

