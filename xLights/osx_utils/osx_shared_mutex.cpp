//===---------------------- shared_mutex.cpp ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "__config"
#ifndef _LIBCPP_HAS_NO_THREADS

#define _LIBCPP_BUILDING_SHARED_MUTEX
#include "shared_mutex"


_LIBCPP_BEGIN_NAMESPACE_STD



#if _LIBCPP_VERSION < 3700
#define MUTEX_BASE_CLASS shared_timed_mutex
#else
#define MUTEX_BASE_CLASS __shared_mutex_base
#endif

// Shared Mutex Base
MUTEX_BASE_CLASS::MUTEX_BASE_CLASS()
: __state_(0)
{
}

// Exclusive ownership

void
MUTEX_BASE_CLASS::lock()
{
    unique_lock<mutex> lk(__mut_);
    while (__state_ & __write_entered_)
        __gate1_.wait(lk);
    __state_ |= __write_entered_;
    while (__state_ & __n_readers_)
        __gate2_.wait(lk);
}

bool
MUTEX_BASE_CLASS::try_lock()
{
    unique_lock<mutex> lk(__mut_);
    if (__state_ == 0)
    {
        __state_ = __write_entered_;
        return true;
    }
    return false;
}

void
MUTEX_BASE_CLASS::unlock()
{
    lock_guard<mutex> _(__mut_);
    __state_ = 0;
    __gate1_.notify_all();
}

// Shared ownership

void
MUTEX_BASE_CLASS::lock_shared()
{
    unique_lock<mutex> lk(__mut_);
    while ((__state_ & __write_entered_) || (__state_ & __n_readers_) == __n_readers_)
        __gate1_.wait(lk);
    unsigned num_readers = (__state_ & __n_readers_) + 1;
    __state_ &= ~__n_readers_;
    __state_ |= num_readers;
}

bool
MUTEX_BASE_CLASS::try_lock_shared()
{
    unique_lock<mutex> lk(__mut_);
    unsigned num_readers = __state_ & __n_readers_;
    if (!(__state_ & __write_entered_) && num_readers != __n_readers_)
    {
        ++num_readers;
        __state_ &= ~__n_readers_;
        __state_ |= num_readers;
        return true;
    }
    return false;
}

void
MUTEX_BASE_CLASS::unlock_shared()
{
    lock_guard<mutex> _(__mut_);
    unsigned num_readers = (__state_ & __n_readers_) - 1;
    __state_ &= ~__n_readers_;
    __state_ |= num_readers;
    if (__state_ & __write_entered_)
    {
        if (num_readers == 0)
            __gate2_.notify_one();
    }
    else
    {
        if (num_readers == __n_readers_ - 1)
            __gate1_.notify_one();
    }
}

#if _LIBCPP_VERSION >= 3700

shared_timed_mutex::shared_timed_mutex() : __base() {}
void shared_timed_mutex::lock()     { return __base.lock(); }
bool shared_timed_mutex::try_lock() { return __base.try_lock(); }
void shared_timed_mutex::unlock()   { return __base.unlock(); }
void shared_timed_mutex::lock_shared() { return __base.lock_shared(); }
bool shared_timed_mutex::try_lock_shared() { return __base.try_lock_shared(); }
void shared_timed_mutex::unlock_shared() { return __base.unlock_shared(); }

#endif

_LIBCPP_END_NAMESPACE_STD

#endif // !_LIBCPP_HAS_NO_THREADS