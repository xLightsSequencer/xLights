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

// The byte stream the LAN transport puts on a connection, and the helpers
// around pairing. Not for the app: CollabNet.h is the public side.
//
// Every frame is a 4-byte big-endian length (of what follows), a 1-byte
// type and the payload. Blob chunks share the connection with messages; the
// sender lets at most a little blob data get ahead of a waiting message.

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include "CollabNet.h"

namespace Collab::Wire {

enum class FrameType : uint8_t {
    Welcome = 1,   // host -> client, first frame: u32 the client's PeerId
    Message = 2,   // EncodeMessage output
    BlobStart = 3, // u64 id, u64 size
    BlobData = 4,  // u64 id, bytes
    BlobEnd = 5,   // u64 id
};

inline constexpr size_t kHeaderBytes = 5;
inline constexpr size_t kMaxMessageBytes = Collab::kMaxMessageBytes;
inline constexpr size_t kBlobChunkBytes = 64u << 10;
inline constexpr uint64_t kMaxBlobBytes = 8ull << 30;

std::string EncodeFrame(FrameType type, std::string_view payload);
std::string WelcomeFrame(PeerId assigned);
std::string BlobStartFrame(BlobId id, uint64_t size);
std::string BlobDataFrame(BlobId id, std::string_view bytes);
std::string BlobEndFrame(BlobId id);

struct Frame {
    FrameType type = FrameType::Message;
    std::string payload;
};

// Splits a byte stream into frames. Once it has seen something that can't
// be a frame (an unknown type, a length over the limit for its type, a
// malformed fixed-size payload) it stays Bad: the connection must go.
class FrameReader {
public:
    enum class Result { Frame, More, Bad };
    void Feed(const void* data, size_t len);
    Result Next(Frame& out);
    size_t Buffered() const { return _buf.size() - _pos; }

private:
    std::string _buf;
    size_t _pos = 0;
    bool _bad = false;
};

bool ReadU32(std::string_view p, size_t at, uint32_t& v);
bool ReadU64(std::string_view p, size_t at, uint64_t& v);

// Pairing: the TLS pre-shared key is derived from the code and the
// session's salt (carried in the advertised service name and TXT record),
// so a captured handshake can't be checked against a precomputed table.
inline constexpr const char* kServiceType = "_xlights-collab._tcp";
inline constexpr const char* kServicePrefix = "xlc-";
inline constexpr const char* kPskIdentity = "xlights-collab";
inline constexpr size_t kSaltBytes = 8;
std::string NewSalt();                           // kSaltBytes random bytes
std::string HexOf(std::string_view bytes);
bool SaltFromHex(std::string_view hex, std::string& salt);
std::string ServiceNameFor(const std::string& salt); // "xlc-<hex>"
// The salt in a service name, tolerating a Bonjour rename suffix (" (2)").
bool SaltFromServiceName(std::string_view name, std::string& salt);
// Host migration: the salt the successor `who` of the session `sessionId`
// (a service name) listens under. It changes the service name and the
// pairing key; the code stays the same. False when `sessionId` carries no
// salt (a direct "tcp:" id).
bool SuccessorSalt(std::string_view sessionId, uint16_t who, std::string& salt);

// Joining by address, where the network can't be browsed. The session id is
// kAddressPrefix + "host:port"; the client sends kSaltRequest before TLS and
// a host of the portable transport answers SaltReply (the service name and a
// newline), then the TLS handshake follows on the same connection. Apple
// hosts don't answer it, so Apple-to-Apple sessions are unchanged.
inline constexpr const char* kAddressPrefix = "addr:";
inline constexpr const char* kSaltRequest = "XLCS";
std::string SaltReply(const std::string& salt);
// "host:port", "[v6]:port"; false unless the port is 1-65535.
bool SplitHostPort(std::string_view text, std::string& host, std::string& port);

// What a host's listener changing state means (Apple's nw_listener). Waiting
// means the network can't be used just now, e.g. an interface went down:
// before the listener was ever ready that is a failure to listen, after it
// the listener comes back to ready by itself and the session goes on.
enum class ListenerState { Ready, Waiting, Failed, Other };
enum class ListenerVerdict { Ignore, Listening, Fail };
ListenerVerdict ListenerStateVerdict(ListenerState state, bool hasError, bool wasReady);

// The life of one asynchronous service registration (Windows'
// DnsServiceRegister) shared by its owner, which may stop it at any time, and
// the completion callbacks, which may arrive after the owner is gone. Each
// call says what to do next; Free means nothing refers to it any more.
// Callers serialise the calls.
class RegistrationLifetime {
public:
    enum class Action { None, Deregister, Free };

    // The owner is done with it and won't touch it again.
    Action Stop();
    // A completion callback: of the registration, or of the deregistration
    // once one was sent. `ok`: it succeeded.
    Action Completed(bool ok);
    // Sending the deregistration failed: no completion will come.
    Action DeregisterFailed();
    // The registration failed (the owner should stop counting it).
    bool Failed() const { return _state == State::Failed; }

private:
    enum class State { Registering, Registered, Deregistering, Failed, Done };
    State _state = State::Registering;
    bool _stopped = false;
};

// Callback contexts for asynchronous OS calls that may call back after they
// were cancelled: the OS holds a number, not a pointer, so a late callback
// finds nothing instead of freed memory.
template<class T>
class CallbackRegistry {
public:
    uint64_t Add(std::shared_ptr<T> item) {
        std::lock_guard<std::mutex> l(_m);
        uint64_t id = ++_next;
        _items[id] = std::move(item);
        return id;
    }
    std::shared_ptr<T> Find(uint64_t id) {
        std::lock_guard<std::mutex> l(_m);
        auto it = _items.find(id);
        return it == _items.end() ? nullptr : it->second;
    }
    void Remove(uint64_t id) {
        std::lock_guard<std::mutex> l(_m);
        _items.erase(id);
    }
    size_t Size() {
        std::lock_guard<std::mutex> l(_m);
        return _items.size();
    }

private:
    std::mutex _m;
    uint64_t _next = 0;
    std::map<uint64_t, std::shared_ptr<T>> _items;
};

} // namespace Collab::Wire

namespace Collab::LanTest {
// The 32-byte key for `code` and `salt` (PBKDF2-HMAC-SHA256).
std::string DeriveKey(const std::string& code, const std::string& salt);
// A session id Connect accepts that reaches a listening host at `address`
// and its port without Bonjour (for tests in a process that can't browse).
std::string DirectSessionId(INetTransport& host, const std::string& address = "127.0.0.1");
// Writes `bytes` on the connection to `peer` as they are, after whatever is
// already queued.
void SendRaw(INetTransport& t, PeerId peer, const std::string& bytes);
// The folder received blobs are written to.
std::string BlobFolder(INetTransport& t);
// The host's limit on failed pairing handshakes: after `failures` of them it
// refuses new connections for `firstMs`, doubling each round up to `maxMs`.
struct PairingBackoff {
    int failures = 5;
    int64_t firstMs = 5000;
    int64_t maxMs = 60000;
};
void SetPairingBackoff(INetTransport& host, const PairingBackoff& b);
struct PairingStatus {
    int failedHandshakes = 0;
    int refusedConnections = 0;
    int64_t windowMs = 0; // the current (last started) pause, 0 = none yet
    bool refusing = false;
};
PairingStatus Pairing(INetTransport& host);
} // namespace Collab::LanTest

// The Windows / Linux transport (CollabNetPortable.cpp). It is what
// CreateLanTransport returns there; tests on a Mac build it too, to check it
// against the Apple transport.
namespace Collab::Portable {
std::unique_ptr<INetTransport> CreateTransport(Dispatcher dispatch);
std::unique_ptr<ISessionBrowser> CreateBrowser(Dispatcher dispatch);
bool DiscoveryAvailable();
std::string DeriveKey(const std::string& code, const std::string& salt);
std::string DirectSessionId(INetTransport& host, const std::string& address = "127.0.0.1");
void SetPairingBackoff(INetTransport& host, const LanTest::PairingBackoff& b);
LanTest::PairingStatus Pairing(INetTransport& host);
void SendRaw(INetTransport& t, PeerId peer, const std::string& bytes);
std::string BlobFolder(INetTransport& t);
// Sets up TLS as if starting PSA failed, over memory full of garbage, and
// tears it down again. True when the setup said it failed (and the teardown
// didn't crash).
bool TlsSetupFailureCleansUp();
// The TCP options on the connection to `peer`; 0 / false where the
// system can't report one.
struct SocketOptions {
    bool noDelay = false;
    bool keepAlive = false;
    int idleSec = 0;
    int intervalSec = 0;
    int count = 0;
};
bool PeerSocketOptions(INetTransport& t, PeerId peer, SocketOptions& out);
// The host registered its DNS-SD service.
bool Advertising(INetTransport& host);
// Runs before each name lookup of a join by address, on the thread making
// it (nullptr: none).
void SetBeforeLookup(std::function<void(const std::string& host)> hook);
} // namespace Collab::Portable
