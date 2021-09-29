#pragma once

#include <wx/wx.h>
#include <wx/socket.h>
#include <wx/thread.h>

#include <list>
#include <string>
#include <queue>
#include <optional>
#include <mutex>
#include <atomic>

// seed with ... xLights defined stuff
// seed with networks we are attached to

// work order ... do discovery
    // ping devices

// ping devices
    // http open
    
// http open
    // FPP/Falcon connect

class xScannerFrame;
class WorkManager;
class wxWindow;
class Controller;
class wxSocketClient;
class ScanWork;

template<typename T>
class ThreadsafeQueue {
    std::queue<T> queue_;
    mutable std::mutex mutex_;

    // Moved out of public interface to prevent races between this
    // and pop().
    bool empty() const
    {
        return queue_.empty();
    }

public:
    ThreadsafeQueue() = default;
    ThreadsafeQueue(const ThreadsafeQueue<T>&) = delete;
    ThreadsafeQueue& operator=(const ThreadsafeQueue<T>&) = delete;

    ThreadsafeQueue(ThreadsafeQueue<T>&& other)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_ = std::move(other.queue_);
    }

    virtual ~ThreadsafeQueue() {}

    unsigned long size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    void push(const T& item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
    }

    std::optional<T> pop()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return {};
        }
        T tmp = queue_.front();
        queue_.pop();
        return tmp;
    }
};

typedef enum WorkType {
    COMPUTER,
    PING,
    MAC,
    DISCOVER,
    HTTP,
    FPP,
    FALCON,
    UNKNOWN
} WorkType;

typedef enum ThreadType {
    TTPING,
    TTMAIN,
    TTOTHER
} ThreadType;

class ScanThread : public wxThread
{
    bool _terminate = false;
    ThreadType _threadType = ThreadType::TTOTHER;
    virtual ExitCode Entry();
    WorkManager& _workManager;
    wxSocketClient* _client = nullptr;
    ScanWork* _activeWork = nullptr;
    std::mutex _mutex;
public:
    ScanThread(WorkManager& workManager, ThreadType tt, wxSocketClient* client = nullptr)
        : wxThread(wxTHREAD_DETACHED), _workManager(workManager), _threadType(tt), _client(client) {}
    ~ScanThread() {}
    void Terminate() { _terminate = true; }
    void TerminateWork();
};

class ScanWork {

protected:
    WorkType _type = WorkType::UNKNOWN;
    wxWindow* GetFrameWindow();
    bool _terminate = false;

public:
    WorkType GetType() { return _type; }
    ScanWork(WorkType type) { _type = type; }
    virtual ~ScanWork() {}
    WorkType GetWorkType() const { return _type; }
    virtual void DoWork(WorkManager& workManager, wxSocketClient* client) = 0;
    void PublishResult(WorkManager& workManager, std::list<std::pair<std::string, std::string>>& result);
    virtual bool IsMainThread() { return false; }
    void Terminate() { _terminate = true; }
};

class ComputerWork : public ScanWork
{
protected:
    std::list<std::string> _macsDone;
    std::string _xLightsShowFolder;
    std::string _xScheduleShowFolder;

    std::string GetXScheduleShowFolder();
    std::string GetXLightsShowFolder();
    std::string GetForceIP();
    void ScanARP(WorkManager& workManager);
    void ProcessController(WorkManager& workManager, Controller* controller, const std::string& why);

public:
    ComputerWork() : ScanWork(WorkType::COMPUTER) { }
    virtual ~ComputerWork() {}
    virtual void DoWork(WorkManager& workManager, wxSocketClient* client) override;
};

class WorkManager {
protected:
    ThreadsafeQueue<ScanWork*> _queueHTTP;
    ThreadsafeQueue<ScanWork*> _queuePing;
    ThreadsafeQueue<ScanWork*> _queueOther;
    mutable std::mutex _mutex;
    std::list<std::string> _scannedIP;
    std::list<std::string> _scannedHTTP;
    ThreadsafeQueue < std::list<std::pair<std::string, std::string>>> _results;
    std::list<ScanThread*> _threadsOther;
    std::list<ScanThread*> _threadsPing;
    std::list<ScanThread*> _threadsHTTP;
    std::list<std::string> _found;
    bool _started = false;
    bool _singleThreaded = false;

public:
    WorkManager();
    virtual ~WorkManager();

    void SetSingleThreaded(bool st)
    {
        _singleThreaded = st;
    }
    void Restart()
    {
        while (_queuePing.size() > 0)             {
            _queuePing.pop();
        }
        while (_queueHTTP.size() > 0) {
            _queueHTTP.pop();
        }
        while (_queueOther.size() > 0) {
            _queueOther.pop();
        }
        while (_results.size() > 0) {
            _results.pop();
        }
        for (const auto& it : _threadsOther) {
            it->TerminateWork();
        }
        for (const auto& it : _threadsPing) {
            it->TerminateWork();
        }
        for (const auto& it : _threadsHTTP) {
            it->TerminateWork();
        }
        _scannedIP.clear();
        _scannedHTTP.clear();
        _found.clear();

        // give everything a chance to stop
        wxSleep(1);

        // now do it all again to make sure all is empty
        while (_queuePing.size() > 0) {
            _queuePing.pop();
        }
        while (_queueHTTP.size() > 0) {
            _queueHTTP.pop();
        }
        while (_queueOther.size() > 0) {
            _queueOther.pop();
        }
        while (_results.size() > 0) {
            _results.pop();
        }
        for (const auto& it : _threadsOther) {
            it->TerminateWork();
        }
        for (const auto& it : _threadsPing) {
            it->TerminateWork();
        }
        for (const auto& it : _threadsHTTP) {
            it->TerminateWork();
        }
        _scannedIP.clear();
        _scannedHTTP.clear();
        _found.clear();
    }
    std::string GetPendingWork()
    {
        return wxString::Format("HTTP %d : Ping %d : Other %d", (int)_queueHTTP.size(), (int)_queuePing.size(), (int)_queueOther.size());
    }
    void AddHTTP(const std::string& ip, int port, const std::string& proxy = "");
    void AddIP(const std::string& ip, const std::string& why, const std::string& proxy = "");
    void AddClassDSubnet(const std::string& ip, const std::string& proxy = "");
    void AddComputer()
    {
        _queueOther.push(new ComputerWork());
    }
    void Start();
    void Stop();
    void AddWork(ScanWork* work)
    {
        switch (work->GetType())             {
        case WorkType::HTTP:
            _queueHTTP.push(work);
            break;
        case WorkType::PING:
            _queuePing.push(work);
            break;
        default:
            _queueOther.push(work);
            break;
        }
    }
    void PublishResult(std::list<std::pair<std::string, std::string>>& result)
    {
        _results.push(result);
    }
    std::optional<ScanWork*> GetWork(ThreadType tt)
    {
        switch (tt)             {
        case ThreadType::TTMAIN:
            return _queueHTTP.pop();
            break;
        case ThreadType::TTPING:
            return _queuePing.pop();
            break;
        case ThreadType::TTOTHER:
            return _queueOther.pop();
            break;
        }

        return {};
    }

    const std::list<std::string>& GetFound() const
    {
        return _found;
    }

    void AddFoundIP(const std::string& ip)
    {
        if (std::find(begin(_found), end(_found), ip) == end(_found))             {
            _found.push_back(ip);
        }
    }
    std::list<std::pair<std::string, std::string>> GetNextResult()
    {
        auto res = _results.pop();
        if (res.has_value()) {
            return *res;
        }
        else             {
            return std::list<std::pair<std::string, std::string>>();
        }
    }
};

class PingWork : public ScanWork
{
protected:
    std::string _ip;
    std::string _proxy;
    std::string _why;

public:
    PingWork(const std::string& ip, const std::string& why, const std::string& proxy = "") : ScanWork(WorkType::PING) { _ip = ip; _proxy = proxy; _why = why; }
    virtual ~PingWork() {}
    virtual void DoWork(WorkManager& workManager, wxSocketClient* client) override;
};

class MACWork : public ScanWork
{
protected:
    std::string _ip;
    std::string _mac;

public:
    MACWork(const std::string& ip, const std::string& mac) : ScanWork(WorkType::MAC) { _ip = ip; _mac = mac; }
    virtual ~MACWork() {}
    virtual void DoWork(WorkManager& workManager, wxSocketClient* client) override;
};

class DiscoverWork : public ScanWork
{
public:
    DiscoverWork() : ScanWork(WorkType::DISCOVER) {}
    virtual ~DiscoverWork() {}
    virtual void DoWork(WorkManager& workManager, wxSocketClient* client) override;
};

class HTTPWork : public ScanWork
{
protected:
    std::string _ip;
    int _port = 80;
    std::string _proxy;

    std::string GetTitle(const std::string& page);
    std::string GetControllerTypeBasedOnPageContent(const std::string& page);

public:
    HTTPWork(const std::string& ip, int port = 80, const std::string& proxy = "") : ScanWork(WorkType::HTTP) { _ip = ip; _port = port; _proxy = proxy; }
    virtual ~HTTPWork() {}
    virtual void DoWork(WorkManager& workManager, wxSocketClient* client) override;
    virtual bool IsMainThread() override { return true; }
};

class FPPWork : public ScanWork
{
protected:
    std::string _ip;
    std::string _proxy;

    std::string DecodeWifiStrength(int w)
    {
        if (w > -50) return "Excellent";
        if (w > -60) return "Good";
        if (w > -70) return "Fair";
        return "Weak";
    }

public:
    FPPWork(const std::string& ip, const std::string& proxy = "") : ScanWork(WorkType::FPP) { _ip = ip; _proxy = proxy; }
    virtual ~FPPWork() {}
    virtual void DoWork(WorkManager& workManager, wxSocketClient* client) override;
};

class FalconWork : public ScanWork
{
protected:
    std::string _ip;
    std::string _proxy;

public:
    FalconWork(const std::string& ip, const std::string& proxy = "") : ScanWork(WorkType::FPP) { _ip = ip; _proxy = proxy; }
    virtual ~FalconWork() {}
    virtual void DoWork(WorkManager& workManager, wxSocketClient* client) override;
};

class xScheduleWork : public ScanWork
{
protected:
    std::string _ip;
    std::string _proxy;
    int _port = 80;

public:
    xScheduleWork(const std::string& ip, int port = 80, const std::string& proxy = "") : ScanWork(WorkType::FPP) { _ip = ip; _proxy = proxy; _port = port; }
    virtual ~xScheduleWork() {}
    virtual void DoWork(WorkManager& workManager, wxSocketClient* client) override;
};
