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

// What the group transport (CollabGroup.h) puts in each channel message. Not
// for the app.
//
// Every channel message is one frame: a version byte, a type byte and the
// payload. Integers are big-endian.
//
//   Claim      host -> all     u32 protocol. "I host this session."
//   Hello      client -> all   (empty) asks the host for a PeerId
//   Welcome    host -> client  u32 the client's PeerId
//   Bye        either way      (empty) leaving on purpose
//   Msg        either way      EncodeMessage output, whole
//   MsgPart    either way      u64 message id, u32 index, u32 count, bytes
//   BlobStart  either way      u64 blob id, u64 size
//   BlobData   either way      u64 blob id, bytes
//   BlobEnd    either way      u64 blob id
//   BlobAck    either way      u64 blob id, u64 bytes of it received so far
//                              (kAckDropped: the recipient dropped it)
//   AttachAck  either way      the attachment key, received
//   Succeed    successor -> all u32 protocol, u64 epoch, u32 participant,
//                              u64 applied rev. "I take over this session"
//                              (host migration); the others reconnect to it.
//
// A file goes through the channel's attachment store under the key
// "xlb1:<blob id>:<sequence>:<recipient member id>" when it fits, otherwise
// as BlobStart / BlobData... / BlobEnd. The sender keeps at most
// kBlobWindow bytes of blob data unacknowledged per recipient, so a message
// sent meanwhile waits behind at most that much.

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace Collab::GroupWire {

inline constexpr uint8_t kVersion = 2;

enum class Type : uint8_t {
    Claim = 1,
    Hello = 2,
    Welcome = 3,
    Bye = 4,
    Msg = 5,
    MsgPart = 6,
    BlobStart = 7,
    BlobData = 8,
    BlobEnd = 9,
    BlobAck = 10,
    AttachAck = 11,
    Succeed = 12,
};

inline constexpr size_t kHeaderBytes = 2;
inline constexpr size_t kPartHeaderBytes = kHeaderBytes + 16;
inline constexpr size_t kDataHeaderBytes = kHeaderBytes + 8;
// A channel that can't carry this much per message isn't used.
inline constexpr size_t kMinMessageBytes = 64;
inline constexpr size_t kMaxBlobChunkBytes = 64u << 10;
inline constexpr uint64_t kBlobWindow = 128u << 10;
// The receiver acknowledges blob data in steps of this much (and at the end).
inline constexpr uint64_t kAckEvery = 32u << 10;
inline constexpr uint64_t kAckDropped = ~0ull;
// Message parts one sender may have half-received at once.
inline constexpr size_t kMaxPartials = 8;
inline constexpr const char* kAttachPrefix = "xlb1:";

inline void PutU32(std::string& s, uint32_t v) {
    for (int i = 3; i >= 0; --i) {
        s.push_back((char)((v >> (8 * i)) & 0xFF));
    }
}

inline void PutU64(std::string& s, uint64_t v) {
    for (int i = 7; i >= 0; --i) {
        s.push_back((char)((v >> (8 * i)) & 0xFF));
    }
}

inline std::string Frame(Type type, std::string_view payload = {}) {
    std::string s;
    s.reserve(kHeaderBytes + payload.size());
    s.push_back((char)kVersion);
    s.push_back((char)type);
    s.append(payload);
    return s;
}

} // namespace Collab::GroupWire
