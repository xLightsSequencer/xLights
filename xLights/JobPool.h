//
//  JobPool.h
//  xLights
//


#ifndef __xLights__JobPool__
#define __xLights__JobPool__


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
    virtual std::string GetStatus() { return EMPTY_STRING; }
    virtual bool DeleteWhenComplete() { return false; }
    virtual bool SetThreadName() { return true; }
    virtual const std::string GetName() const { return EMPTY_STRING; }
    
    const static std::string EMPTY_STRING;
};


class JobPoolWorker;
class JobPool
{
    std::mutex threadLock;
    std::mutex queueLock;
    std::condition_variable signal;
    std::vector<JobPoolWorker*> threads;
    std::deque<Job*> queue;
    std::atomic_int numThreads;
    std::atomic_int maxNumThreads;
    std::atomic_int idleThreads;
    std::atomic_int inFlight;
    std::string threadNameBase;
    
public:
    JobPool(const std::string &threadNameBase);
    virtual ~JobPool();
    
    virtual void PushJob(Job *job);
    int size() const { return (int)threads.size(); }
    int maxSize() const { return maxNumThreads; }
    virtual void Start(size_t poolSize = 1);
    virtual void Stop();
    
    virtual std::string GetThreadStatus();
    
private:
    friend class JobPoolWorker;
    void RemoveWorker(JobPoolWorker*);
    void LockThreads();
    void UnlockThreads();
    Job *GetNextJob();
};


#endif /* defined(__xLights__JobPool__) */
