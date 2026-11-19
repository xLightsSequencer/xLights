#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Files on their way through a transport, shared by the LAN transports
// (CollabNetApple.mm, CollabNetPortable.cpp) and the group transport
// (CollabGroup.cpp): private copies of outgoing files, made off the calling
// thread, and incoming files written as their bytes arrive.

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include "CollabTransport.h"

namespace Collab::Files {

// Opens `path` so the caller may delete or replace it at once (the open file
// stays readable). nullptr when it can't be opened.
FILE* OpenForCopy(const std::string& path);

// One worker thread at a time copies outgoing files, in the order given.
// Sending from a private copy keeps a later change to the file out of a
// transfer in flight; making it on the worker keeps a large file (or a copy
// across volumes, where a clone isn't possible) from stalling the caller.
class FileCopier : public std::enable_shared_from_this<FileCopier> {
public:
    struct Job {
        FILE* src = nullptr; // closed by the copier
        std::string dest;
        // Called on the worker thread: the copy's size, or false when it failed
        // (the partial copy is already removed).
        std::function<void(bool ok, uint64_t size)> done;
    };

    void Add(Job j);

private:
    void Run();

    std::mutex _m;
    std::deque<Job> _jobs;
    bool _running = false;
};

// One incoming file, written as its frames arrive. A malformed sequence of
// frames is the peer's fault (Bad: drop the connection); a file that can't
// be written here is a local disk problem: the rest of its bytes are read and
// discarded, and End reports Failed, so the connection stays up.
class IncomingBlob {
public:
    enum class Result { Ok, Bad, Received, Failed };

    IncomingBlob() = default;
    ~IncomingBlob() { Abort(); }
    IncomingBlob(const IncomingBlob&) = delete;
    IncomingBlob& operator=(const IncomingBlob&) = delete;

    bool Active() const { return _active; }
    BlobId Id() const { return _id; }
    uint64_t Received() const { return _got; }
    uint64_t Size() const { return _size; }

    // A file of `size` bytes is coming, to be written at `path`.
    Result Start(const std::string& path, BlobId id, uint64_t size, uint64_t maxSize);
    Result Data(BlobId id, std::string_view bytes);
    // Received: the whole file is at `path`. Failed: it couldn't be saved,
    // and `why` says why.
    Result End(BlobId id, std::string& path, std::string& why);
    // Drops a transfer in progress and its partial file.
    void Abort();

private:
    void DiskFailed(const std::string& what);

    bool _active = false;
    BlobId _id = 0;
    uint64_t _size = 0;
    uint64_t _got = 0;
    FILE* _file = nullptr;
    std::string _path;
    std::string _error;
};

namespace Test {
// Copies of outgoing files never clone (as across volumes).
extern std::atomic<bool> noClone;
// Runs on the worker before each copy (e.g. to hold it up).
extern std::function<void()> beforeCopy;
// Copies queued or running in any FileCopier, until each one's `done` has
// returned.
size_t CopiesInFlight();
} // namespace Test

} // namespace Collab::Files
