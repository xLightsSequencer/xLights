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

#include <deque>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include <condition_variable>

class Job {
public:
    Job() {}
    virtual ~Job() {};
    virtual void Process() = 0;
    virtual bool DeleteWhenComplete() { return false; }
    virtual bool SetThreadName() { return true; }

    virtual std::string GetStatus();
    virtual const std::string GetName() const;
};


class JobPoolWorker;
class JobPool
{
    const int MIN_JOBPOOLTHREADS = 4;
    std::mutex threadLock;
    std::mutex queueLock;
    std::condition_variable signal;
    std::vector<JobPoolWorker*> threads;
    std::deque<Job*> queue;
    std::atomic_int numThreads;
    std::atomic_int idleThreads;
    std::string threadNameBase;

protected:
    std::atomic_int inFlight;
    int maxNumThreads;
    int minNumThreads;
    
public:
    JobPool(const std::string &threadNameBase);
    virtual ~JobPool();
    
    virtual void PushJob(Job *job);
    int size() const { return (int)threads.size(); }
    int maxSize() const { return maxNumThreads; }
    virtual void Start(size_t poolSize = 1, size_t minPoolSize = 0);
    virtual void Stop();
    void SetMaxThreadCount(int maxThreads);

    virtual std::string GetThreadStatus();
    
private:
    friend class JobPoolWorker;
    void RemoveWorker(JobPoolWorker*);
    void LockThreads();
    void UnlockThreads();
    Job *GetNextJob();
};
