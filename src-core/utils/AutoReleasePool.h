#pragma once

// RAII wrapper for an Objective-C autorelease pool.
//
// Construct one on the stack to push a pool; it pops on scope exit. The
// pool follows the same LIFO + thread-affinity contract as
// `@autoreleasepool { ... }` — the runtime asserts if a pool is popped
// out of order or on a different thread than it was pushed. To keep
// callers honest the type is non-copyable, non-movable, and not heap
// allocatable.
//
// On Windows / Linux this is an empty type so the same source compiles
// everywhere; an `AutoReleasePool pool;` line costs nothing off Apple
// platforms.

#include <cstddef>

#if defined(__APPLE__)

extern "C" void *objc_autoreleasePoolPush(void);
extern "C" void  objc_autoreleasePoolPop(void *);

class AutoReleasePool {
public:
    AutoReleasePool() : token(objc_autoreleasePoolPush()) {}
    ~AutoReleasePool() { objc_autoreleasePoolPop(token); }

    AutoReleasePool(const AutoReleasePool &) = delete;
    AutoReleasePool &operator=(const AutoReleasePool &) = delete;
    AutoReleasePool(AutoReleasePool &&) = delete;
    AutoReleasePool &operator=(AutoReleasePool &&) = delete;

    static void *operator new(size_t) = delete;
    static void *operator new[](size_t) = delete;

private:
    void *token;
};

#else

class AutoReleasePool {
public:
    AutoReleasePool() = default;

    AutoReleasePool(const AutoReleasePool &) = delete;
    AutoReleasePool &operator=(const AutoReleasePool &) = delete;
    AutoReleasePool(AutoReleasePool &&) = delete;
    AutoReleasePool &operator=(AutoReleasePool &&) = delete;

    static void *operator new(size_t) = delete;
    static void *operator new[](size_t) = delete;
};

#endif
