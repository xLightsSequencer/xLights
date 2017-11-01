//
//  JobPool.cpp
//  xLights


#include <string>
#include <mutex>
#include <condition_variable>
#include <sstream> 
#include <iomanip>
#include <algorithm>

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
    Job  * volatile currentJob;
    enum {
        STARTING,
        IDLE,
        RUNNING_JOB,
        STOPPED,
        UNKNOWN
    } status;
public:
    JobPoolWorker(JobPool *p);
    virtual ~JobPoolWorker();

    void Stop();
    bool Start();
    void* Entry();

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
    } else if (status == RUNNING_JOB) {
        ret << "<running job>";
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

void* JobPoolWorker::Entry()
{
    // KW - extra logging to try to work out why this function crashes so often on windows ... I have tried to limit it to rare events
    static log4cpp::Category &logger_jobpool = log4cpp::Category::getInstance(std::string("log_jobpool"));

#ifdef LINUX
    XInitThreads();
#endif
    while ( !stopped ) {
        status = IDLE;
        // Did we get a request to terminate?
        if (TestDestroy())
        {
            logger_jobpool.debug("JobPoolWorker::Entry requested to terminate.");
            break;
        }
        Job *job = pool->GetNextJob();
        if (job != nullptr) {
            logger_jobpool.debug("JobPoolWorker::Entry processing job.");
            status = RUNNING_JOB;
            // Call user's implementation for processing request
            ProcessJob(job);
            logger_jobpool.debug("JobPoolWorker::Entry processed job.");
            status = IDLE;
            pool->inFlight--;
        } else if (pool->idleThreads > 12) {
            break;
        }
    }
    logger_jobpool.debug("JobPoolWorker::Entry exiting.");
    pool->numThreads--;
    status = STOPPED;
    return nullptr;
}

#ifndef __WXMSW__
static std::string OriginalThreadName() {
    char buf[256];
    pthread_getname_np(pthread_self(), buf, 256);
    return buf;
}

static void SetThreadName(const std::string &name) {
#ifdef __WXOSX__
    pthread_setname_np(name.c_str());
#else
    pthread_setname_np(pthread_self(), name.c_str());
#endif
}
#else
//no idea how to do this on Windows or even if there is value in doing so
static std::string OriginalThreadName() { return ""; }
static void SetThreadName(const std::string &name) {}
#endif


void JobPoolWorker::ProcessJob(Job *job)
{
    static log4cpp::Category &logger_jobpool = log4cpp::Category::getInstance(std::string("log_jobpool"));
    if (job) {
		logger_jobpool.debug("Starting job on background thread.");
		currentJob = job;
        std::string origName = OriginalThreadName();
        SetThreadName(job->GetName());
        job->Process();
        SetThreadName(origName);
        currentJob = nullptr;
        
        if (job->DeleteWhenComplete()) {
            logger_jobpool.debug("Job on background thread done ... deleting job.");
            delete job;
        }
        else
        {
            logger_jobpool.debug("Job on background thread done.");
        }
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

Job *JobPool::GetNextJob() {
    std::unique_lock<std::mutex> mutLock(queueLock);
    Job *req = nullptr;
    if (queue.empty()) {
        idleThreads++;
        long timeout = 100;
        if (idleThreads <= 12) {
            timeout = 30000;
        }
        signal.wait_for(mutLock, std::chrono::milliseconds(timeout));
        idleThreads--;
    }
    if ( !queue.empty() ) {
        req = queue.front();
        queue.pop_front();
    }
    return req;
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

