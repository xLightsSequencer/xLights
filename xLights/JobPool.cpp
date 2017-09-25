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
    JobPool *pool;
    volatile bool stopped;
    Job *currentJob;
    enum {
        STARTING,
        IDLE,
        STOPPED,
        UNKNOWN
    } status;
public:
    JobPoolWorker(JobPool *p);
    virtual ~JobPoolWorker();

    void Stop();
    bool Start();
    void* Entry();

    Job *GetJob();

    void ProcessJob(Job *job);
    std::string GetStatus();
};

JobPoolWorker::JobPoolWorker(JobPool *p)
: wxThread(wxTHREAD_DETACHED), pool(p), currentJob(nullptr), stopped(false), status(STARTING)
{
}

JobPoolWorker::~JobPoolWorker()
{
    status = UNKNOWN;
    pool->RemoveWorker(this);
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
    
    Job *j = currentJob;
    if (j != nullptr) {
        ret << j->GetStatus();
    } else if (status == STARTING) {
        ret << "<starting>";
    } else if (status == IDLE) {
        ret << "<idle>";
    } else if (status == STOPPED) {
        ret << "<stopped>";
    } else {
        ret << "<unknown>";
    }
    
    return ret.str();
}

void JobPoolWorker::Stop()
{
    status = STOPPED;
    stopped = true;

    std::unique_lock<std::mutex> mutLock(pool->queueLock);
    pool->signal.notify_all();
}

bool JobPoolWorker::Start()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static const unsigned int stackSize = (sizeof(size_t) == 8) ? (256 * 1024) : (128 * 1024);
    auto rc = Create(stackSize);

    switch(rc)
    {
    case wxTHREAD_NO_ERROR:
        break;
    case wxTHREAD_NO_RESOURCE:
        logger_base.error("Insufficient resources to create worker thread.");
        return false;
    case wxTHREAD_NOT_RUNNING:
        logger_base.error("The worker thread is already running.");
        return false;
    default:
        logger_base.error("worker thread start returned unexpected result %d.", rc);
        return false;
    }

    return Run() == wxTHREAD_NO_ERROR;
}

Job *JobPoolWorker::GetJob()
{
    std::unique_lock<std::mutex> mutLock(pool->queueLock);
    Job *req = nullptr;
    if (pool->queue.empty()) {
        pool->idleThreads++;
        long timeout = 100;
        if (pool->idleThreads <= 12) {
            timeout = 30000;
        }
        pool->signal.wait_for(mutLock, std::chrono::milliseconds(timeout));
        pool->idleThreads--;
    }
    if ( !pool->queue.empty() ) {
        req = pool->queue.front();
        pool->queue.pop_front();
    }
    return req;
}

void* JobPoolWorker::Entry()
{
    // KW - extra logging to try to work out why this function crashes so often on windows ... I have tried to limit it to rare events
    static log4cpp::Category &logger_jobpool = log4cpp::Category::getInstance(std::string("log_jobpool"));

#ifdef LINUX
    XInitThreads();
#endif
    while ( !stopped ) {
        // Did we get a request to terminate?
        if (TestDestroy())
        {
            logger_jobpool.debug("JobPoolWorker::Entry requested to terminate.");
            break;
        }
        status = IDLE;
        Job *job = GetJob();
        if (job != nullptr) {
            logger_jobpool.debug("JobPoolWorker::Entry processing job.");
            // Call user's implementation for processing request
            ProcessJob(job);
            if (job->DeleteWhenComplete()) {
                logger_jobpool.debug("JobPoolWorker::Entry Job done ... deleting job.");
                delete job;
            }
            else
            {
                logger_jobpool.debug("JobPoolWorker::Entry Job done.");
            }
            std::unique_lock<std::mutex> mutLock(pool->queueLock);
            pool->inFlight--;
        } else {
            std::unique_lock<std::mutex> mutLock(pool->queueLock);
            if (pool->idleThreads > 12) {
                break;
            }
        }
    }
    logger_jobpool.debug("JobPoolWorker::Entry exiting.");
    std::unique_lock<std::mutex> mutLock(pool->threadLock);
    pool->numThreads--;
    status = STOPPED;
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

JobPool::JobPool() : queueLock(), threadLock(), signal(), queue(), idleThreads(0),  numThreads(0), inFlight(0), maxNumThreads(8)
{
}

JobPool::~JobPool()
{
    if ( !queue.empty() ) {
        std::deque<Job*>::iterator iter = queue.begin();
        for (; iter != queue.end(); ++iter) {
            delete (*iter);
        }
        queue.clear();
    }
    Stop();
}

void JobPool::RemoveWorker(JobPoolWorker *w) {
    std::unique_lock<std::mutex> locker(threadLock);
    auto loc = std::find(threads.begin(), threads.end(), w);
    if (loc != threads.end()) {
        threads.erase(loc);
    }
}

void JobPool::PushJob(Job *job)
{
	std::unique_lock<std::mutex> locker(queueLock);
    queue.push_back(job);
    inFlight++;
    
    std::unique_lock<std::mutex> tlocker(threadLock);
    int count = inFlight;
    count -= idleThreads;
    count -= numThreads;
    while (count > 0 && (numThreads < maxNumThreads)) {
        count--;
        JobPoolWorker *worker = new JobPoolWorker(this);
        if (worker->Start()) {
            threads.push_back(worker);
            numThreads++;
        } else {
            delete worker;
        }
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
    logger_jobpool.info("Background thread pool started with %d threads", poolSize);
}

void JobPool::Stop()
{
    std::unique_lock<std::mutex> locker(threadLock);
    for(size_t i=0; i<threads.size(); i++){
        JobPoolWorker *worker = threads.at(i);
        worker->Stop();
    }
    while (!threads.empty()) {
        locker.unlock();
        wxThread::Sleep(5);
        locker.lock();
    }
}

std::string JobPool::GetThreadStatus() {
    std::unique_lock<std::mutex> locker(threadLock);
    std::stringstream ret;
    ret << "\n";
    for(size_t i=0; i<threads.size(); i++){
        JobPoolWorker *worker = threads.at(i);
        ret << worker->GetStatus();
        ret << "\n";
    }
    return ret.str();
}

