//
//  JobPool.cpp
//  xLights


#include "JobPool.h"


class JobPoolWorker : public wxThread
{
    wxMutex *lock;
    wxCondition *signal;
    volatile int &idleThreads;
    volatile int &numThreads;
    std::deque<Job*> *queue;
    
public:
    JobPoolWorker(wxMutex *l,
                  wxCondition *signal,
                  std::deque<Job*> *queue,
                  volatile int &idleThreadPtr,
                  volatile int &numThreadsPtr);
    virtual ~JobPoolWorker();
    
    void Stop();
    void Start(int priority = WXTHREAD_DEFAULT_PRIORITY);
    virtual void* Entry();
    
    Job *GetJob();
    
    virtual void ProcessJob(Job *job);
};

JobPoolWorker::JobPoolWorker(wxMutex *l, wxCondition *s, std::deque<Job*> *queue,
                             volatile int &idleThreadPtr, volatile int &numThreadsPtr)
: wxThread(wxTHREAD_JOINABLE), lock(l) ,signal(s), queue(queue), idleThreads(idleThreadPtr), numThreads(numThreadsPtr)
{
}

JobPoolWorker::~JobPoolWorker()
{
}

void JobPoolWorker::Stop()
{
    if ( IsAlive() )
        Delete();
    
    while ( IsAlive() ) {
        wxThread::Sleep( 5 );
    }
}

void JobPoolWorker::Start(int priority)
{
    Create();
    SetPriority(priority);
    Run();
}

Job *JobPoolWorker::GetJob()
{
    wxMutexLocker mutLock(*lock);
    Job *req(NULL);
    if (queue->empty()) {
        idleThreads++;
        signal->WaitTimeout(100);
        idleThreads--;
    }
    if ( !queue->empty() ) {
        req = queue->front();
        queue->pop_front();
    }
    return req;
}

void* JobPoolWorker::Entry()
{
    while ( true ) {
        // Did we get a request to terminate?
        if (TestDestroy())
            break;
        
        Job *job = GetJob();
        if (job) {
            // Call user's implementation for processing request
            ProcessJob(job);
            delete job;
            job = NULL;
        } else {
            wxMutexLocker mutLock(*lock);
            if (idleThreads > 5) {
                break;
            }
        }
    }
    wxMutexLocker mutLock(*lock);
    numThreads--;
    return NULL;
}

void JobPoolWorker::ProcessJob(Job *job)
{
    if (job) {
        job->Process();
    }
}

JobPool::JobPool() : lock(), signal(lock), queue()
{
    idleThreads = 0;
    numThreads = 0;
    maxNumThreads = 8;
}

JobPool::~JobPool()
{
    if ( !queue.empty() ) {
        std::deque<Job*>::iterator iter = queue.begin();
        for (; iter != queue.end(); iter++) {
            delete (*iter);
        }
        queue.clear();
    }
    Stop();
}

void JobPool::PushJob(Job *job)
{
    wxMutexLocker locker(lock);
    if (idleThreads == 0 && numThreads < maxNumThreads) {
        numThreads++;
        
        JobPoolWorker *worker = new JobPoolWorker(&lock, &signal, &queue, idleThreads, numThreads);
        worker->Start(threadPriority);
        threads.push_back(worker);
    }
    queue.push_back(job);
    signal.Broadcast();
}

void JobPool::Start(size_t poolSize, int priority)
{
    maxNumThreads = poolSize > 250 ? 250 : poolSize;
    threadPriority = priority;
    idleThreads = 0;
    numThreads = 0;
}

void JobPool::Stop()
{
    for(size_t i=0; i<threads.size(); i++){
        JobPoolWorker *worker = threads.at(i);
        worker->Stop();
        delete worker;
    }
    threads.clear();
}

