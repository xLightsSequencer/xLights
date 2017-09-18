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
#include <condition_variable>

class Job {
public:
    Job() {}
    virtual ~Job() {};
    virtual void Process() = 0;
    virtual std::string GetStatus() = 0;
    
    virtual bool DeleteWhenComplete() = 0;
};


class JobPoolWorker;
class JobPool
{
    std::mutex lock;
    std::condition_variable signal;
    std::vector<JobPoolWorker*> threads;
    std::deque<Job*> queue;
    int numThreads;
    int maxNumThreads;
    volatile int idleThreads;
    volatile unsigned int inFlight;
    
public:
    JobPool();
    virtual ~JobPool();
    
    virtual void PushJob(Job *job);
    int size() const { return threads.size(); }
    virtual void Start(size_t poolSize = 1);
    virtual void Stop();
    
    virtual std::string GetThreadStatus();
};


#endif /* defined(__xLights__JobPool__) */
