#include "TraceLog.h"


#include <thread>
#include <map>
#include <list>
#include <mutex>

static const std::string CONTEXT_MARKER = "--context--";
static volatile bool TRACE_LOG_VALID = false;
thread_local std::list<std::string> *threadLog = nullptr;

class TraceLogHolder {
public:
    std::mutex MESSAGE_MAP_LOCK;
    std::map<std::thread::id, std::list<std::string>*> LOG_MESSAGES;

    TraceLogHolder() {TRACE_LOG_VALID = true;}
    ~TraceLogHolder() {
        TRACE_LOG_VALID = false;
        for (auto it_thread = LOG_MESSAGES.begin(); it_thread != LOG_MESSAGES.end(); ++it_thread) {
            delete it_thread->second;
        }
        LOG_MESSAGES.clear();
    }
    
    void ClearTraceMessages() {
        if (!TRACE_LOG_VALID) {
            return;
        }
        std::unique_lock<std::mutex> lock(MESSAGE_MAP_LOCK);
        std::thread::id id = std::this_thread::get_id();
        auto search = LOG_MESSAGES.find(id);
        if (search != LOG_MESSAGES.end()) {
            delete search->second;
            LOG_MESSAGES.erase(search);
        }
        threadLog = nullptr;
    }

    std::list<std::string>* GetMessagesForThread(bool create = true) {
        if (!TRACE_LOG_VALID) {
            return nullptr;
        }
        if (threadLog) {
            return threadLog;
        }
        std::unique_lock<std::mutex> lock(MESSAGE_MAP_LOCK);
        std::thread::id id = std::this_thread::get_id();
        auto search = LOG_MESSAGES.find(id);
        std::list<std::string>* ret = nullptr;
        if (search != LOG_MESSAGES.end()) {
            ret = search->second;
        }
        if (ret == nullptr && create) {
            ret = new std::list<std::string>();
            LOG_MESSAGES[id] = ret;
        }
        threadLog = ret;
        return ret;
    }
};

static TraceLogHolder TRACELOG_HOLDER;


void TraceLog::AddTraceMessage(const std::string &msg) {
    std::list<std::string>* traceMessages = TRACELOG_HOLDER.GetMessagesForThread();
    if (!traceMessages) {
        return;
    }
    traceMessages->push_back(msg);
    if (traceMessages->size() > 20) {
        if (traceMessages->front() != CONTEXT_MARKER) {
            traceMessages->pop_front();
        }
    }
}
void TraceLog::PushTraceContext() {
    std::list<std::string>* traceMessages = TRACELOG_HOLDER.GetMessagesForThread();
    traceMessages->push_back(CONTEXT_MARKER);
}
void TraceLog::PopTraceContext() {
    std::list<std::string>* traceMessages = TRACELOG_HOLDER.GetMessagesForThread();
    if (!traceMessages) {
        return;
    }
    while (!traceMessages->empty() && (traceMessages->back() != CONTEXT_MARKER)) {
        traceMessages->pop_back();
    }
    if (!traceMessages->empty() &&  (traceMessages->back() == CONTEXT_MARKER)) {
        traceMessages->pop_back();
    }
}
void TraceLog::ClearTraceMessages() {
    TRACELOG_HOLDER.ClearTraceMessages();
}

void TraceLog::GetTraceMessages(std::list<std::string> &msgs) {
    std::list<std::string>* traceMessages = TRACELOG_HOLDER.GetMessagesForThread(false);
    if (traceMessages != nullptr) {
        for (auto &a : *traceMessages) {
            msgs.push_back(a);
        }
    }
}
