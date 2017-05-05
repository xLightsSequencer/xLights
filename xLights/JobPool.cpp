//
//  JobPool.cpp
//  xLights


#include <string>
#include <mutex>
#include <condition_variable>
#include <sstream> 
#include <iomanip>

#include "JobPool.h"

#ifdef LINUX
    #include <X11/Xlib.h>
#endif

#include <wx/thread.h>
#include <log4cpp/Category.hh>

class JobPoolWorker : public wxThread
{
    std::mutex *lock;
    std::condition_variable *signal;
    volatile int &idleThreads;
    volatile int &numThreads;
    volatile unsigned int &inFlight;
    volatile bool stopped;
    std::deque<Job*> *queue;

    Job *currentJob;

public:
    JobPoolWorker(std::mutex *l,
                  std::condition_variable *signal,
                  std::deque<Job*> *queue,
                  volatile int &idleThreadPtr,
                  volatile int &numThreadsPtr,
                  volatile unsigned int &inFlightPtr);
    virtual ~JobPoolWorker();

    void Stop();
    void Start();
    void* Entry();

    Job *GetJob();

    void ProcessJob(Job *job);
    std::string GetStatus();
};

JobPoolWorker::JobPoolWorker(std::mutex *l, std::condition_variable *s, std::deque<Job*> *queue,
                             volatile int &idleThreadPtr, volatile int &numThreadsPtr, volatile unsigned int &inFlightPtr)
: wxThread(wxTHREAD_JOINABLE), lock(l) ,signal(s), queue(queue), idleThreads(idleThreadPtr), numThreads(numThreadsPtr),
    inFlight(inFlightPtr), currentJob(nullptr), stopped(false)
{
}

JobPoolWorker::~JobPoolWorker()
{
}

std::string JobPoolWorker::GetStatus()
{
    std::stringstream ret;
    ret << "Thread: ";
        
    ret << std::showbase // show the 0x prefix
        << std::internal // fill between the prefix and the number
        << std::setfill('0') << std::setw(10)
        << std::hex << GetId()
        << "    ";
    
    if (currentJob != nullptr) {
        ret << currentJob->GetStatus();
    } else {
        ret << "<idle>";
    }
    
    return ret.str();
}

void JobPoolWorker::Stop()
{
    stopped = true;
    
    if ( IsAlive() ) {
        std::unique_lock<std::mutex> mutLock(*lock);
        signal->notify_all();
    }
    
    if ( IsAlive() ) {
        Delete();
    }

    while ( IsAlive() ) {
        wxThread::Sleep( 5 );
    }
}

void JobPoolWorker::Start()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static const unsigned int stackSize = 1024*128;
    auto rc = Create(stackSize);

    switch(rc)
    {
    case wxTHREAD_NO_ERROR:
        break;
    case wxTHREAD_NO_RESOURCE:
        logger_base.error("Insufficient resources to create worker thread.");
        break;
    case wxTHREAD_NOT_RUNNING:
        logger_base.error("The worker thread is already running.");
        break;
    default:
        logger_base.error("worker thread start returned unexpected result %d.", rc);
        break;
    }

    Run();
}

Job *JobPoolWorker::GetJob()
{
    std::unique_lock<std::mutex> mutLock(*lock);
    Job *req = nullptr;
    if (queue->empty()) {
        idleThreads++;
        long timeout = 100;
        if (idleThreads <= 5) {
            timeout = 30000;
        }
        signal->wait_for(mutLock, std::chrono::milliseconds(timeout));
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
#ifdef LINUX
    XInitThreads();
#endif
    while ( !stopped ) {
        // Did we get a request to terminate?
        if (TestDestroy())
            break;

        Job *job = GetJob();
        if (job) {
            // Call user's implementation for processing request
            ProcessJob(job);
            if (job->DeleteWhenComplete()) {
                delete job;
            }
            job = nullptr;
            std::unique_lock<std::mutex> mutLock(*lock);
            inFlight--;
        } else {
            std::unique_lock<std::mutex> mutLock(*lock);
            if (idleThreads > 5) {
                break;
            }
        }
    }
    std::unique_lock<std::mutex> mutLock(*lock);
    numThreads--;
    return nullptr;
}

void JobPoolWorker::ProcessJob(Job *job)
{
    static log4cpp::Category &logger_jobpool = log4cpp::Category::getInstance(std::string("log_jobpool"));
    if (job) {
		logger_jobpool.debug("Starting job on background thread.");
		currentJob = job;
        job->Process();
        currentJob = nullptr;
		logger_jobpool.debug("Job on background thread done.");
	}
}

JobPool::JobPool() : lock(), signal(), queue()
{
    idleThreads = 0;
    numThreads = 0;
    inFlight = 0;
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
	std::unique_lock<std::mutex> locker(lock);
    queue.push_back(job);
    inFlight++;
    
    int count = inFlight;
    count -= idleThreads;
    count -= numThreads;
    while (count > 0 && (numThreads < maxNumThreads)) {
        count--;
        numThreads++;

        JobPoolWorker *worker = new JobPoolWorker(&lock, &signal, &queue, idleThreads, numThreads, inFlight);
        worker->Start();
		threads.push_back(worker);
    }
    signal.notify_all();
}

void JobPool::Start(size_t poolSize)
{
    static log4cpp::Category &logger_jobpool = log4cpp::Category::getInstance(std::string("log_jobpool"));
    if (poolSize > 250) {
        poolSize = 250;
    }
    
    if (poolSize < 20) {
        poolSize = 20;
    }

    maxNumThreads = poolSize;
    idleThreads = 0;
    numThreads = 0;
    logger_jobpool.info("Background thread pool started with %d threads", maxNumThreads);
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

std::string JobPool::GetThreadStatus() {
    std::stringstream ret;
    ret << "\n";
    for(size_t i=0; i<threads.size(); i++){
        JobPoolWorker *worker = threads.at(i);
        ret << worker->GetStatus();
        ret << "\n\n";
    }
    return ret.str();
}

