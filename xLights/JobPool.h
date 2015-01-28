//
//  JobPool.h
//  xLights
//


#ifndef __xLights__JobPool__
#define __xLights__JobPool__

#include <wx/thread.h>
#include <deque>
#include <vector>

class Job {
public:
    Job() {}
    virtual ~Job() {};
    virtual void Process() = 0;
};


class JobPoolWorker;
class JobPool
{
    wxMutex lock;
    wxCondition signal;
    std::vector<JobPoolWorker*> threads;
    std::deque<Job*> queue;
    int numThreads;
    int maxNumThreads;
    int threadPriority;
    volatile int idleThreads;
    
public:
    JobPool();
    virtual ~JobPool();
    
    virtual void PushJob(Job *job);
    
    virtual void Start(size_t poolSize = 1, int priority = WXTHREAD_DEFAULT_PRIORITY);
    virtual void Stop();
};


#endif /* defined(__xLights__JobPool__) */
