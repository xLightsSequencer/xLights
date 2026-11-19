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

} // namespace Collab::Wire

#ifdef __APPLE__
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
#endif
