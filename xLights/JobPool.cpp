/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

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

#include "../common/xlBaseApp.h"
#include "JobPool.h"

#ifdef LINUX
    #include <X11/Xlib.h>
#endif
#ifdef HAVE_ABI_FORCEDUNWIND
    #include <cxxabi.h>
#endif

#include "ExternalHooks.h"
#include "utils/string_utils.h"

#include "spdlog/spdlog.h"

#include "TraceLog.h"
using namespace TraceLog;

std::string Job::GetStatus() {
    return xlEMPTY_STRING;
}
const std::string Job::GetName() const {
    return xlEMPTY_STRING;
}


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
    std::shared_ptr<spdlog::logger> m_logger{ nullptr };

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
    try
    {
        xlCrashHandler::SetupCrashHandlerForNonWxThread();
        
#ifdef LINUX
        XInitThreads();
#endif
        jpw->Entry();
        delete jpw;
    }
    catch (...)
    {
        wxTheApp->OnUnhandledException();
    }
}

JobPoolWorker::JobPoolWorker(JobPool *p) :
    pool(p), stopped(false), currentJob(nullptr), status(STARTING), thread(nullptr), m_logger(spdlog::get("job")) {

    //
    thread = new std::thread(startFunc, this);
    tid = thread->get_id();
    std::ostringstream oss;
    oss << tid;
    m_logger->debug("JobPoolWorker created {}", oss.str());//fixed in c++23
}

JobPoolWorker::~JobPoolWorker()
{
    //
    std::ostringstream oss;
    oss << tid;
    m_logger->debug("JobPoolWorker destroyed {}", oss.str());
    status = UNKNOWN;
    thread->detach();
    delete thread;
}

std::string JobPoolWorker::GetStatus()
{
    m_logger->debug("Getting status for {}\n", fmt::ptr(this));
    std::stringstream ret;
    ret << "Thread: ";
        
    ret << std::showbase // show the 0x prefix
        << std::internal // fill between the prefix and the number
        << std::setfill('0') << std::setw(10)
        << std::hex << tid
        << "    ";
    
    Job *j = currentJob;
    
    m_logger->debug("     current job {}\n", fmt::ptr(j));

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
    std::ostringstream oss;
    oss << tid;
    m_logger->debug("JobPoolWorker started  {}", oss.str());

    try {
        SetThreadName(pool->threadNameBase);
        SetThreadQOS(0);
        while ( !stopped ) {
            status = IDLE;

            Job *job = pool->GetNextJob();
            if (job != nullptr) {
                m_logger->debug("JobPoolWorker::Entry processing job.   {}", fmt::ptr(this));
                status = RUNNING_JOB;
                // Call user's implementation for processing request
                ProcessJob(job);
                m_logger->debug("JobPoolWorker::Entry processed job.  {}", fmt::ptr(this));
                status = IDLE;
                --pool->inFlight;
            } else if (pool->numThreads > pool->minNumThreads) {
                break;
            }
        }
#ifdef HAVE_ABI_FORCEDUNWIND
    // When using common C++ ABI under Linux we must always rethrow this
    // special exception used to unwind the stack when the thread was
    // cancelled, otherwise the thread library would simply terminate the
    // program, see http://udrepper.livejournal.com/21541.html
    }  catch ( abi::__forced_unwind& ) {
        currentJob = nullptr;
        m_logger->warn("JobPoolWorker::Entry exiting due to __forced_unwind.  {}", mt::ptr(this));
        --(pool->numThreads);
        status = STOPPED;
        pool->RemoveWorker(this);
        throw;
#endif // HAVE_ABI_FORCEDUNWIND
    } catch ( ... ) {
        currentJob = nullptr;
        m_logger->debug("JobPoolWorker::Entry exiting due to unknown exception. {}", oss.str());
        --(pool->numThreads);
        status = STOPPED;
        pool->RemoveWorker(this);
        wxTheApp->OnUnhandledException();
        m_logger->debug("JobPoolWorker done {}", oss.str());
        return;
    }
    currentJob = nullptr;
    m_logger->debug("JobPoolWorker exiting {}", oss.str());
    --(pool->numThreads);
    status = STOPPED;
    pool->RemoveWorker(this);
    m_logger->debug("JobPoolWorker::Entry removed.  {}", fmt::ptr(this));
    RemoveThreadName();
    m_logger->debug("JobPoolWorker done {}", oss.str());
    //clear trace messages for this thread
    ClearTraceMessages();
}

void JobPoolWorker::ProcessJob(Job *job)
{
    
    if (job) {
        m_logger->debug("Starting job on background thread.");
		currentJob = job;
        
        std::string origName;
        bool stn = false;
        if (job->SetThreadName()) {
            origName = OriginalThreadName();
            SetThreadName(job->GetName());
            stn = true;
        }
        bool const deleteWhenComplete = job->DeleteWhenComplete();
        RunInAutoReleasePool([job]() { job->Process(); });
        if (stn) {
            SetThreadName(origName);
        }
        currentJob = nullptr;
        
        if (deleteWhenComplete) {
            status = DELETING_JOB;
            m_logger->debug("Job on background thread done ... deleting job.");
            delete job;
        } else {
            m_logger->debug("Job on background thread done.");
        }
        status = FINISHED_JOB;
	}
}

JobPool::JobPool(const std::string &n) : threadLock(), queueLock(), signal(), queue(), numThreads(0), maxNumThreads(8), minNumThreads(2), idleThreads(0), inFlight(0), threadNameBase(n)
{
}
JobPool::JobPool(const std::string &n, int min, int max) : threadLock(), queueLock(), signal(), queue(), numThreads(0), maxNumThreads(max), minNumThreads(min), idleThreads(0), inFlight(0), threadNameBase(n)
{
}

void JobPool::SetMaxThreadCount(int maxThreads)
{
    if (maxThreads < MIN_JOBPOOLTHREADS) maxThreads = MIN_JOBPOOLTHREADS;
    maxNumThreads = maxThreads;
}

JobPool::~JobPool()
{
    
    //
    if ( !queue.empty() ) {
        std::deque<Job*>::iterator iter = queue.begin();
        for (; iter != queue.end(); ++iter) {
            delete (*iter);
        }
        auto logger = spdlog::get("job");
        logger->debug("Clearing JobPool queue.");
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
        SetThreadQOS(0);
        ++idleThreads;
        signal.wait_for(mutLock, std::chrono::milliseconds(30000));
        --idleThreads;
    }
    if ( !queue.empty() ) {
        req = queue.front();
        queue.pop_front();
        if (req) {
            SetThreadQOS(10);
        }
    }
    return req;
}

void JobPool::PushJob(Job *job)
{
	std::unique_lock<std::mutex> locker(queueLock);
    queue.push_back(job);
    ++inFlight;
    
    int count = inFlight;
    count -= idleThreads;
    count -= numThreads;
    count = std::min(count, maxNumThreads - numThreads);
    locker.unlock();
    
    if (count > 0) {
        LockThreads();
        if (numThreads == 0 && count < MIN_JOBPOOLTHREADS && MIN_JOBPOOLTHREADS < maxNumThreads) {
            //when we create first thread, assume we'll need extras real soon
            count = MIN_JOBPOOLTHREADS;
        }
        for (int i = 0; i < count; i++) {
            threads.push_back(new JobPoolWorker(this));
            ++numThreads;
        }
        UnlockThreads();
    }
    signal.notify_one();
}
void JobPool::PushJobs(const std::list<Job *> &jobs) {
    std::unique_lock<std::mutex> locker(queueLock);
    for (auto job : jobs) {
        queue.push_back(job);
        ++inFlight;
    }
    int count = inFlight;
    count -= idleThreads;
    count -= numThreads;
    count = std::min(count, maxNumThreads - numThreads);
    locker.unlock();
    if (count > 0) {
        LockThreads();
        if (numThreads == 0 && count < MIN_JOBPOOLTHREADS && MIN_JOBPOOLTHREADS < maxNumThreads) {
            //when we create first thread, assume we'll need extras real soon
            count = MIN_JOBPOOLTHREADS;
        }
        for (int i = 0; i < count; i++) {
            threads.push_back(new JobPoolWorker(this));
            ++numThreads;
        }
        UnlockThreads();
    }
    if (jobs.size() > 1) {
        signal.notify_all();
    } else {
        signal.notify_one();
    }
}
bool JobPool::isEmpty() const {
    return inFlight == 0;
}

void JobPool::Start(size_t poolSize, size_t minPoolSize)
{
    if (poolSize > 250) {
        poolSize = 250;
    }

    maxNumThreads = poolSize < MIN_JOBPOOLTHREADS ? MIN_JOBPOOLTHREADS : poolSize;
    minNumThreads = minPoolSize < MIN_JOBPOOLTHREADS ? MIN_JOBPOOLTHREADS : minPoolSize;
    idleThreads = 0;
    numThreads = 0;
    //auto logger = spdlog::get("job");
    //logger->debug("Background thread pool started with {} threads", poolSize);
}

void JobPool::Stop()
{
    LockThreads();
    for (JobPoolWorker *worker : threads) {
        worker->Stop();
    }
    
    while (!threads.empty()) {
        UnlockThreads();
        signal.notify_all();
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
