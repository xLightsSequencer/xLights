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

#include "CollabLayout.h"
#include "render/SequenceIds.h"

namespace Collab {

inline constexpr int kProtocolVersion = 3;
// Frames larger than this are dropped unread. Snapshots and embedded media
// are the large ones.
inline constexpr size_t kMaxFrameBytes = 256u * 1024u * 1024u;
inline constexpr int kMaxNesting = 64;
// Manifest / NeedFiles size limits; a frame over them is malformed.
inline constexpr size_t kMaxManifestFiles = 200000;
inline constexpr size_t kMaxPathBytes = 4096;

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
    Manifest,     // host -> client: the show files the session needs
    NeedFiles,    // client -> host: send me these manifest files
    File,         // either way: a blob carrying one file (or why it can't come)
    Snapshot,     // host -> client: the document at a rev, with object ids
    NeedSnapshot, // client -> host: please send a Snapshot (joining, drift)
    Propose,      // client -> host: an edit
    Commit,       // host -> clients: an edit in host order
    Reject,       // host -> author: an edit that was not committed
    Resend,       // client -> host: send commits from `rev` on
    Presence,     // any -> host -> all: what a participant is looking at
    Ping,         // host -> client: liveness, carries the head rev
    Pong,         // client -> host: liveness, carries the applied rev
    Bye,          // either way: leaving / session over
    Roster,       // host -> clients: everyone in the session, their roles and the row assignments
    Notice,       // host -> client: something the user should read (reason, text)
    Claim         // client -> host: assign rows to a participant, or release them
};

const char* MsgTypeName(MsgType t);

// What a proposal / commit is, for per-user undo.
enum class EditKind { Edit, Undo, Redo };

// What a show file is for. A joiner whose layout or audio differs from the
// host's can't take part with its own show folder; missing or different
// media only render blank / differently.
enum class FileRole { Layout, Audio, Media };
const char* FileRoleName(FileRole r);

// One show file: `path` is how the host's document refers to it (relative
// to the show folder where possible; see CollabManifest.h), with its size
// and 64-bit content hash.
struct FileEntry {
    std::string path;
    FileRole role = FileRole::Media;
    uint64_t size = 0;
    uint64_t hash = 0;
    bool operator==(const FileEntry&) const = default;
};

// Presence is advisory and per participant; it never changes the document.
struct PresenceInfo {
    int startMS = -1; // visible time range, -1 = unknown
    int endMS = -1;
    std::vector<SeqObjectId> focusedElements;
    std::vector<SeqObjectId> selectedEffects;
    SeqObjectId softLock = kNoSeqObjectId; // effect being edited in a panel
    // Layout editor: the selected layout objects (layout id, else name), and
    // the object open in a modal editor (empty object: none).
    std::vector<std::string> layoutSelected;
    LayoutLockInfo layoutLock;
    bool operator==(const PresenceInfo&) const = default;
};

// What a participant may do. A viewer sees everything live but changes
// nothing (the host rejects its proposals); the host is always an editor.
enum class ParticipantRole { Editor, Viewer };

// One participant, as the host lists them.
struct RosterEntry {
    ParticipantId participant = kNoParticipant;
    std::string name;
    std::string deviceKind;
    ParticipantRole role = ParticipantRole::Editor;
    // SecretDigest of the participant's rejoin secret: a successor checks a
    // rejoin against it (host migration). Never the secret itself.
    std::string secretHash;
    bool operator==(const RosterEntry&) const = default;
};

// A row (model, group, timing track) only `owner` may edit. A claim on a
// group also covers its members' rows; a claim on a model covers its
// submodels and strands. Claim: owner kNoParticipant releases the row.
struct Assignment {
    SeqObjectId element = kNoSeqObjectId;
    ParticipantId owner = kNoParticipant;
    std::string name; // the row's name when it was claimed, for display
    bool operator==(const Assignment&) const = default;
};
inline constexpr size_t kMaxAssignments = 0x10000;
inline constexpr size_t kMaxRoster = 0x10000;

// One message. Only the fields of its type are written and read; the rest
// keep their defaults.
struct Message {
    MsgType type = MsgType::Ping;
    int protocol = kProtocolVersion;

    // Hello: this app can edit the layout live. Welcome: the session does.
    bool liveLayout = false;
    // Hello / Welcome: a participant coming back to a session whose host
    // changed (host migration). Hello: `participant` is the id it had, `rev`
    // its applied rev, `clientSeq` the first proposal it still waits on.
    // Welcome: `clientSeq` is its last proposal the session had dealt with.
    bool rejoin = false;

    // Welcome (not a rejoin): this participant's rejoin secret, sent to it
    // alone. Hello (rejoin): the secret it was given.
    std::string secret;

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
    // Propose / Commit: a layout edit (LayoutDelta) instead of a command
    nlohmann::json layout;
    // Commit (layout) / Ping / Snapshot / Welcome: LayoutDelta::Hash of the
    // session layout at that rev (0 = no live layout)
    uint64_t layoutHash = 0;
    // Welcome / Snapshot (live layout): the session layout at that rev. Not
    // part of the message: it travels in the blob the message names.
    LayoutFiles layoutFiles;

    // Commit / Snapshot: the document hash after that rev
    uint64_t docHash = 0;
    // Commit: apply in lenient mode. The host's own edits are committed as
    // its app made them, and the app's edit sites apply leniently; the
    // client's document is the host's, so lenient gives the host's result.
    bool lenient = false;

    // Refuse / Reject / Bye / NeedSnapshot; File: why it is not coming
    std::string reason;
    std::string text;

    // Snapshot: the object id table. Not part of the message: it travels in
    // the blob with the .xsq.
    nlohmann::json uids;
    // Snapshot / File: the blob with the content (0 = none: File with a reason).
    // Welcome (live layout): the blob with the session layout.
    uint64_t blob = 0;
    // Manifest: every file; File: exactly one, the file the blob carries
    std::vector<FileEntry> files;
    // NeedFiles: the manifest paths wanted
    std::vector<std::string> paths;

    // Presence
    PresenceInfo presence;
    // Presence relayed by the host: the order in which the host saw the
    // layout lock taken (earlier holds it); 0 = none
    uint64_t lockOrder = 0;

    // Ping / Pong
    uint64_t time = 0;
    // Pong: the client has the document (it is past joining)
    bool active = false;

    // Roster: the host first, then the clients in joining order
    std::vector<RosterEntry> roster;
    // Roster: every assignment in force; Claim: the rows to assign / release
    std::vector<Assignment> assignments;
    // Roster: how many times the host has changed, and the next participant
    // id the host would give out (a successor never reuses one)
    uint64_t epoch = 0;
    ParticipantId nextParticipant = kNoParticipant;
    // Roster / Ping: who takes over if the host goes, best first
    std::vector<ParticipantId> succession;
    // Roster: the role the host gives people who join (so a successor keeps it)
    ParticipantRole joinRole = ParticipantRole::Editor;
};

std::string EncodeMessage(const Message& m);
// nullopt for anything that isn't a well-formed frame. A frame whose
// protocol version differs decodes only if it is a Hello (so the host can
// refuse it with a reason) or a Refuse / Bye.
std::optional<Message> DecodeMessage(std::string_view frame);
// JSON from another participant that isn't a message (a snapshot's id
// table): nullopt when malformed or nested deeper than kMaxNesting.
std::optional<nlohmann::json> ParseBoundedJson(std::string_view text);
// A fresh rejoin secret (random, hex), and the digest of one (SHA-256, hex)
// that the roster carries in its place.
std::string NewRejoinSecret();
std::string SecretDigest(const std::string& secret);

// Reject reasons. These are the only ones the host sends.
namespace Reason {
inline constexpr const char* Overlap = "overlap";         // an effect is already there
inline constexpr const char* TargetGone = "target_gone";  // effect / layer / row was deleted
inline constexpr const char* Locked = "locked";           // the target effect is locked
inline constexpr const char* StaleType = "stale_type";    // effect type changed since the edit was made
inline constexpr const char* ChangedSince = "changed_since"; // undo / redo: someone changed it since
// Layout proposal: a record it adds was added by someone else meanwhile.
inline constexpr const char* NameTaken = "name_taken";
// Layout proposal: nothing of it was left to commit (text says why).
inline constexpr const char* NoChange = "no_change";
// Notice: someone else took the layout lock on that object first.
inline constexpr const char* LockTaken = "lock_taken";
// The author is a viewer: it may not change the sequence or the layout.
inline constexpr const char* Viewer = "viewer";
// The edit touches a row assigned to someone else (`participant` / by: the
// owner). Also a Claim on a row someone else holds.
inline constexpr const char* Assigned = "assigned";
// Notice: the host changed this participant's role (text says to what).
inline constexpr const char* RoleChanged = "role_changed";
// Malformed or refused by validation. Concurrent edits can cause it too:
// two people adding the same model or giving two tracks one name, removing
// an element's last two layers at once, renaming media onto a taken key.
inline constexpr const char* Invalid = "invalid";
// Refuse / Bye
inline constexpr const char* Protocol = "protocol";
inline constexpr const char* Full = "full";
inline constexpr const char* Ended = "ended";
inline constexpr const char* HostLost = "host_lost";
// Reject: the host that decided this proposal was lost before its answer
// arrived (host migration); it may or may not have been committed then.
inline constexpr const char* HostChanged = "host_changed";
inline constexpr const char* Left = "left";
inline constexpr const char* SnapshotFailed = "snapshot_failed";
// Joining: this device's show files don't match the host's (text says how).
inline constexpr const char* LayoutDiffers = "layout_differs";
inline constexpr const char* AudioDiffers = "audio_differs";
// Joining: a file the session can't do without could not be fetched.
inline constexpr const char* FilesUnavailable = "files_unavailable";
// File: the host can't send it
inline constexpr const char* NotInManifest = "not_in_manifest";
inline constexpr const char* Unreadable = "unreadable";
// Reject / Refuse / Notice: a message larger than a transport may send.
inline constexpr const char* TooLarge = "too_large";
inline constexpr const char* DiskError = "disk_error";
// Bye: the host stopped hearing from this device and dropped it.
inline constexpr const char* TimedOut = "timed_out";
// Bye: a message from a connection that isn't in the session.
inline constexpr const char* NotInSession = "not_in_session";
} // namespace Reason

} // namespace Collab
