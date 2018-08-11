//
//  JobPool.cpp
//  xLights

// needed to ensure the __WXMSW__ is defined
#include <wx/wx.h>

#include <string>
#include <mutex>
#include <condition_variable>
#include <sstream> 
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>

#include "JobPool.h"

#ifdef LINUX
    #include <X11/Xlib.h>
#endif
#ifdef HAVE_ABI_FORCEDUNWIND
    #include <cxxabi.h>
#endif

#include <log4cpp/Category.hh>


const std::string Job::EMPTY_STRING = "";


class JobPoolWorker
{
    JobPool *pool;
    volatile bool stopped;
    std::atomic<Job  *> currentJob;
    std::thread thread;
    enum {
        STARTING,
        IDLE,
        RUNNING_JOB,
        DELETING_JOB,
        FINISHED_JOB,
        STOPPED,
        UNKNOWN
    } status;
public:
    JobPoolWorker(JobPool *p);
    virtual ~JobPoolWorker();

    void Stop();
    void Entry();

    void ProcessJob(Job *job);
    std::string GetStatus();
};

static void startFunc(JobPoolWorker *jpw) {
#ifdef LINUX
    XInitThreads();
#endif
    jpw->Entry();
    delete jpw;
}
JobPoolWorker::JobPoolWorker(JobPool *p)
: pool(p), currentJob(nullptr), stopped(false), status(STARTING), thread(startFunc, this)
{
}

JobPoolWorker::~JobPoolWorker()
{
    thread.detach();
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
        << std::hex << thread.get_id()
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
    } else if (status == FINISHED_JOB) {
        ret << "<finished job>";
    } else if (status == DELETING_JOB) {
        ret << "<deleting job>";
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
static std::map<DWORD, std::string> __threadNames;
static std::string OriginalThreadName()
{
    if (__threadNames.find(::GetCurrentThreadId()) != __threadNames.end())
    {
        return __threadNames[::GetCurrentThreadId()];
    }
    return "";
}
static void SetThreadName(const std::string &name)
{
    __threadNames[::GetCurrentThreadId()] = name;
}
#endif

void JobPoolWorker::Entry()
{
    // KW - extra logging to try to work out why this function crashes so often on windows ... I have tried to limit it to rare events
    static log4cpp::Category &logger_jobpool = log4cpp::Category::getInstance(std::string("log_jobpool"));
    try {

        SetThreadName(pool->threadNameBase);
        while ( !stopped ) {
            status = IDLE;

            Job *job = pool->GetNextJob();
            if (job != nullptr) {
                logger_jobpool.debug("JobPoolWorker::Entry processing job.");
                status = RUNNING_JOB;
                // Call user's implementation for processing request
                ProcessJob(job);
                logger_jobpool.debug("JobPoolWorker::Entry processed job.");
                status = IDLE;
                --pool->inFlight;
            } else if (pool->idleThreads > 12) {
                break;
            }
        }
#ifdef HAVE_ABI_FORCEDUNWIND
    // When using common C++ ABI under Linux we must always rethrow this
    // special exception used to unwind the stack when the thread was
    // cancelled, otherwise the thread library would simply terminate the
    // program, see http://udrepper.livejournal.com/21541.html
    }  catch ( abi::__forced_unwind& ) {
        logger_jobpool.warn("JobPoolWorker::Entry exiting due to __forced_unwind.");
        --pool->numThreads;
        status = STOPPED;
        throw;
#endif // HAVE_ABI_FORCEDUNWIND
    } catch ( ... ) {
        logger_jobpool.warn("JobPoolWorker::Entry exiting due to unknown exception.");
        wxTheApp->OnUnhandledException();
    }
    logger_jobpool.debug("JobPoolWorker::Entry exiting.");
    --pool->numThreads;
    status = STOPPED;
}

void JobPoolWorker::ProcessJob(Job *job)
{
    static log4cpp::Category &logger_jobpool = log4cpp::Category::getInstance(std::string("log_jobpool"));
    if (job) {
		logger_jobpool.debug("Starting job on background thread.");
		currentJob = job;
        
        std::string origName;
        if (job->SetThreadName()) {
            origName = OriginalThreadName();
            SetThreadName(job->GetName());
        }
        bool deleteWhenComplete = job->DeleteWhenComplete();
        job->Process();
        if (job->SetThreadName()) {
            SetThreadName(origName);
        }
        currentJob = nullptr;
        
        if (deleteWhenComplete) {
            status = DELETING_JOB;
            logger_jobpool.debug("Job on background thread done ... deleting job.");
            delete job;
        }
        else
        {
            logger_jobpool.debug("Job on background thread done.");
        }
        status = FINISHED_JOB;
	}
}

JobPool::JobPool(const std::string &n) : threadNameBase(n), queueLock(), threadLock(), signal(), queue(), idleThreads(0),  numThreads(0), inFlight(0), maxNumThreads(8)
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
        threads.push_back(new JobPoolWorker(this));
        numThreads++;
    }
    signal.notify_all();
}

void JobPool::Start(size_t poolSize)
{
    static log4cpp::Category &logger_jobpool = log4cpp::Category::getInstance(std::string("log_jobpool"));
    if (poolSize > 250) {
        poolSize = 250;
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

        std::unique_lock<std::mutex> qlocker(queueLock);
        signal.notify_all();
        qlocker.unlock();
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
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

