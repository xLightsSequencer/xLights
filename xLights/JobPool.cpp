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
    std::atomic_bool stopped;
    std::atomic<Job  *> currentJob;
    enum STATUS_TYPE {
        STARTING,
        IDLE,
        RUNNING_JOB,
        DELETING_JOB,
        FINISHED_JOB,
        STOPPED,
        UNKNOWN
    };
    std::atomic<STATUS_TYPE> status;
    std::thread *thread;
    std::thread::id tid;
public:
    JobPoolWorker(JobPool *p);
    virtual ~JobPoolWorker();

    void Stop();
    void Entry();

    void ProcessJob(Job *job);
    std::string GetStatus();
    
    std::string GetThreadName() const;
};

static void startFunc(JobPoolWorker *jpw) {
#ifdef LINUX
    XInitThreads();
#endif
    jpw->Entry();
    delete jpw;
}
JobPoolWorker::JobPoolWorker(JobPool *p)
: pool(p), stopped(false), currentJob(nullptr), status(STARTING), thread(nullptr)
{
    static log4cpp::Category &logger_jobpool = log4cpp::Category::getInstance(std::string("log_jobpool"));
    logger_jobpool.debug("JobPoolWorker created  %X\n", this);
    thread = new std::thread(startFunc, this);
    tid = thread->get_id();
}

JobPoolWorker::~JobPoolWorker()
{
    static log4cpp::Category &logger_jobpool = log4cpp::Category::getInstance(std::string("log_jobpool"));
    logger_jobpool.debug("JobPoolWorker destroyed  %X\n", this);
    status = UNKNOWN;
    thread->detach();
    delete thread;
}

std::string JobPoolWorker::GetStatus()
{
    static log4cpp::Category &logger_jobpool = log4cpp::Category::getInstance(std::string("log_jobpool"));

    logger_jobpool.debug("Getting status for %X\n", this);
    std::stringstream ret;
    ret << "Thread: ";
        
    ret << std::showbase // show the 0x prefix
        << std::internal // fill between the prefix and the number
        << std::setfill('0') << std::setw(10)
        << std::hex << tid
        << "    ";
    
    Job *j = currentJob;
    
    logger_jobpool.debug("     current job %X\n", j);

    if (j != nullptr && j->SetThreadName()) {
        ret << j->GetName() << " - ";
    } else {
        ret << pool->threadNameBase << " - ";
    }
    
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
static void RemoveThreadName() {}
#else
//no idea how to do this on Windows or even if there is value in doing so
static std::map<DWORD, std::string> __threadNames;
static std::mutex thread_name_mutex;
static std::string OriginalThreadName()
{
    std::unique_lock<std::mutex> lock(thread_name_mutex);
    if (__threadNames.find(::GetCurrentThreadId()) != __threadNames.end()) {
        return __threadNames[::GetCurrentThreadId()];
    }
    return "";
}

static void SetThreadName(const std::string &name)
{
    std::unique_lock<std::mutex> lock(thread_name_mutex);
    __threadNames[::GetCurrentThreadId()] = name;
}

static void RemoveThreadName()
{
    std::unique_lock<std::mutex> lock(thread_name_mutex);
    auto it = __threadNames.find(::GetCurrentThreadId());
    if (it != __threadNames.end())
        __threadNames.erase(it);
}
#endif

std::string JobPoolWorker::GetThreadName() const
{
    Job *j = currentJob;
    if (j != nullptr) {
        if (j->SetThreadName()) {
            return j->GetName();
        }
    }
    return pool->threadNameBase;
}

void JobPoolWorker::Entry()
{
    static log4cpp::Category &logger_jobpool = log4cpp::Category::getInstance(std::string("log_jobpool"));
    logger_jobpool.debug("JobPoolWorker started  %X\n", this);

    try {
        SetThreadName(pool->threadNameBase);
        while ( !stopped ) {
            status = IDLE;

            Job *job = pool->GetNextJob();
            if (job != nullptr) {
                logger_jobpool.debug("JobPoolWorker::Entry processing job.   %X", this);
                status = RUNNING_JOB;
                // Call user's implementation for processing request
                ProcessJob(job);
                logger_jobpool.debug("JobPoolWorker::Entry processed job.  %X", this);
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
        logger_jobpool.warn("JobPoolWorker::Entry exiting due to __forced_unwind.  %X", this);
        pool->numThreads--;
        status = STOPPED;
        pool->RemoveWorker(this);
        throw;
#endif // HAVE_ABI_FORCEDUNWIND
    } catch ( ... ) {
        logger_jobpool.warn("JobPoolWorker::Entry exiting due to unknown exception.  %X", this);
        --pool->numThreads;
        status = STOPPED;
        pool->RemoveWorker(this);
        wxTheApp->OnUnhandledException();
        return;
    }
    logger_jobpool.debug("JobPoolWorker::Entry exiting.  %X", this);
    --pool->numThreads;
    status = STOPPED;
    pool->RemoveWorker(this);
    logger_jobpool.debug("JobPoolWorker::Entry removed.  %X", this);
    RemoveThreadName();
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
        } else {
            logger_jobpool.debug("Job on background thread done.");
        }
        status = FINISHED_JOB;
	}
}

JobPool::JobPool(const std::string &n) : threadLock(), queueLock(), signal(), queue(), numThreads(0), maxNumThreads(8),  idleThreads(0), inFlight(0), threadNameBase(n)
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

void JobPool::LockThreads() {
    threadLock.lock();
    std::atomic_thread_fence(std::memory_order_acquire);
}
void JobPool::UnlockThreads() {
    std::atomic_thread_fence(std::memory_order_release);
    threadLock.unlock();
}

void JobPool::RemoveWorker(JobPoolWorker *w) {
    LockThreads();
    auto loc = std::find(threads.begin(), threads.end(), w);
    if (loc != threads.end()) {
        threads.erase(loc);
    }
    UnlockThreads();
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
    
    int count = inFlight;
    count -= idleThreads;
    count -= numThreads;
    count = std::min(count, maxNumThreads - numThreads);
    if (count > 0) {
        LockThreads();
        if (numThreads == 0 && count < 4 && 4 < maxNumThreads) {
            //when we create first thread, assume we'll need extras real soon
            count = 4;
        }
        for (int i = 0; i < count; i++) {
            threads.push_back(new JobPoolWorker(this));
            numThreads++;
        }
        UnlockThreads();
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
    LockThreads();
    for (JobPoolWorker *worker : threads) {
        worker->Stop();
    }
    
    while (!threads.empty()) {
        UnlockThreads();

        std::unique_lock<std::mutex> qlocker(queueLock);
        signal.notify_all();
        qlocker.unlock();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        LockThreads();
    }
    UnlockThreads();
}

std::string JobPool::GetThreadStatus() {
    std::stringstream ret;
    ret << "\n";
    LockThreads();
    for (JobPoolWorker *worker : threads) {
        /*
        uint64_t t = (uint64_t)worker;
        ret << std::showbase // show the 0x prefix
            << std::internal // fill between the prefix and the number
            << std::setfill('0') << std::setw(10)
            << std::hex << t
            << "\n";
        */
        ret << worker->GetStatus();
        ret << "\n";
    }
    UnlockThreads();
    return ret.str();
}

