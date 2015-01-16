//
//  JobPool.cpp
//  xLights


#include "JobPool.h"


class JobPoolWorker : public wxThread
{
    wxMutex *lock;
    wxCondition *signal;
    std::deque<Job*> *queue;
    
public:
    JobPoolWorker(wxMutex *l, wxCondition *signal, std::deque<Job*> *queue);
    virtual ~JobPoolWorker();
    
    void Stop();
    void Start(int priority = WXTHREAD_DEFAULT_PRIORITY);
    virtual void* Entry();
    
    Job *GetJob();
    
    virtual void ProcessJob(Job *job);
};

JobPoolWorker::JobPoolWorker(wxMutex *l, wxCondition *s, std::deque<Job*> *queue)
: wxThread(wxTHREAD_JOINABLE), lock(l) ,signal(s), queue(queue)
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
        signal->WaitTimeout(100);
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
        }
    }
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
    queue.push_back(job);
    signal.Broadcast();
}

void JobPool::Start(size_t poolSize, int priority)
{
    size_t maxPoolSize = poolSize > 250 ? 250 : poolSize;
    for(size_t i=0; i<maxPoolSize; i++) {
        //create new thread
        JobPoolWorker *worker = new JobPoolWorker(&lock, &signal, &queue);
        worker->Start(priority);
        threads.push_back(worker);
    }
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

