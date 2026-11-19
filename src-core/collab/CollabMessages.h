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

// The messages peers in a live editing session exchange, and their JSON
// framing. Every frame carries the protocol version ("v"); a Hello with a
// different version is refused. Decoding never throws: anything malformed,
// oversized or nested too deeply decodes to std::nullopt.

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

#include "render/SequenceIds.h"

namespace Collab {

inline constexpr int kProtocolVersion = 1;
// Frames larger than this are dropped unread. Snapshots and embedded media
// are the large ones.
inline constexpr size_t kMaxFrameBytes = 256u * 1024u * 1024u;
inline constexpr int kMaxNesting = 64;

// Position in the host's commit order. Rev 0 is the document the session
// started from; the first commit is rev 1.
using Rev = uint64_t;
// A participant's number for the session, assigned by the host and never
// reused within it. Also the high 16 bits (SeqIds peer prefix) of every
// object id that participant creates, so ids made on different devices
// cannot collide. The host is participant 1.
using ParticipantId = uint16_t;
inline constexpr ParticipantId kNoParticipant = 0;
inline constexpr ParticipantId kHostParticipant = 1;

enum class MsgType {
    Hello,        // client -> host: wants to join
    Welcome,      // host -> client: joined, here is your participant id
    Refuse,       // host -> client: not joined (reason)
    Snapshot,     // host -> client: the document at a rev, with object ids
    NeedSnapshot, // client -> host: please send a Snapshot (joining, drift)
    Propose,      // client -> host: an edit
    Commit,       // host -> clients: an edit in host order
    Reject,       // host -> author: an edit that was not committed
    Resend,       // client -> host: send commits from `rev` on
    Presence,     // any -> host -> all: what a participant is looking at
    Ping,         // host -> client: liveness, carries the head rev
    Pong,         // client -> host: liveness, carries the applied rev
    Bye           // either way: leaving / session over
};

const char* MsgTypeName(MsgType t);

// What a proposal / commit is, for per-user undo.
enum class EditKind { Edit, Undo, Redo };

// Presence is advisory and per participant; it never changes the document.
struct PresenceInfo {
    int startMS = -1; // visible time range, -1 = unknown
    int endMS = -1;
    std::vector<SeqObjectId> focusedElements;
    std::vector<SeqObjectId> selectedEffects;
    SeqObjectId softLock = kNoSeqObjectId; // effect being edited in a panel
    bool operator==(const PresenceInfo&) const = default;
};

// One message. Only the fields of its type are written and read; the rest
// keep their defaults.
struct Message {
    MsgType type = MsgType::Ping;
    int protocol = kProtocolVersion;

    // Hello
    std::string appVersion;
    std::string name;       // also Welcome: the host's display name
    std::string deviceKind; // "mac", "ipad", "windows", ...

    // Welcome: the id assigned; Commit: the author; Presence: whose;
    // Reject: who changed the target (reason changed_since / stale_type)
    ParticipantId participant = kNoParticipant;

    // Welcome / Snapshot / Commit: that rev; Ping: head; Pong: applied;
    // Resend: first rev wanted; Reject: the host rev it was decided at
    Rev rev = 0;

    // Propose / Commit / Reject; Snapshot: the recipient's last proposal the
    // host had dealt with when it took the snapshot
    uint64_t clientSeq = 0;
    Rev baseRev = 0; // Propose: the last rev the author had applied
    EditKind kind = EditKind::Edit;
    Rev undoOf = 0;  // Undo / Redo: the rev being undone / redone
    nlohmann::json command; // a serialized SequenceCommand

    // Commit / Snapshot: the document hash after that rev
    uint64_t docHash = 0;

    // Refuse / Reject / Bye / NeedSnapshot
    std::string reason;
    std::string text;

    // Snapshot
    std::string xsq;
    nlohmann::json uids;

    // Presence
    PresenceInfo presence;

    // Ping / Pong
    uint64_t time = 0;
};

std::string EncodeMessage(const Message& m);
// nullopt for anything that isn't a well-formed frame. A frame whose
// protocol version differs decodes only if it is a Hello (so the host can
// refuse it with a reason) or a Refuse / Bye.
std::optional<Message> DecodeMessage(std::string_view frame);

// Reject reasons. These are the only ones the host sends.
namespace Reason {
inline constexpr const char* Overlap = "overlap";         // an effect is already there
inline constexpr const char* TargetGone = "target_gone";  // effect / layer / row was deleted
inline constexpr const char* Locked = "locked";           // the target effect is locked
inline constexpr const char* StaleType = "stale_type";    // effect type changed since the edit was made
inline constexpr const char* ChangedSince = "changed_since"; // undo / redo: someone changed it since
// Malformed or refused by validation. Concurrent edits can cause it too:
// two people adding the same model or giving two tracks one name, removing
// an element's last two layers at once, renaming media onto a taken key.
inline constexpr const char* Invalid = "invalid";
// Refuse / Bye
inline constexpr const char* Protocol = "protocol";
inline constexpr const char* Full = "full";
inline constexpr const char* Ended = "ended";
inline constexpr const char* HostLost = "host_lost";
inline constexpr const char* Left = "left";
inline constexpr const char* SnapshotFailed = "snapshot_failed";
} // namespace Reason

} // namespace Collab
