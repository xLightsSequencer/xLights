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

// Sessions over a group the operating system sets up (SharePlay, from a
// FaceTime call or a Messages conversation), for people who are not on the
// same network.
//
// The platform only supplies an IGroupChannel: the devices in the group, and
// reliable but size-limited messages between them, plus (where the platform
// has it) a shared file store for large files. Everything else lives here and
// is tested with a fake channel:
//   - the device that started the activity hosts; the others join as
//     clients. The host claims the session in its first frame; a client that
//     hears from no host waits, and two claims resolve to the lower member id;
//   - each member gets a PeerId (host kLanHostPeer, clients 2, 3, ... in
//     the order the host first hears them);
//   - session messages larger than the channel's limit are split into chunks
//     and put back together;
//   - files (snapshot, show files) go through the channel's file store when
//     it has one and the file fits, otherwise in chunks over the messages;
//   - members leaving or the group ending become PeerEvent::Left / Lost and
//     NetState::Closed / Failed.
// There is no pairing code: the group is already limited to the people in
// the call and encrypted end to end.

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "CollabNet.h"

namespace Collab {

struct GroupMember {
    std::string id; // stable for the life of the group
    std::string name;
    bool operator==(const GroupMember&) const = default;
};

// One group session the platform has joined. All calls and callbacks happen
// on the dispatcher thread (the platform hops before calling back).
class IGroupChannel {
public:
    virtual ~IGroupChannel() = default;

    virtual std::string LocalId() const = 0;
    // This device started the activity (it hosts).
    virtual bool IsInitiator() const = 0;
    // The largest payload Send takes.
    virtual size_t MaxMessageBytes() const = 0;
    // Reliable and ordered per sender. `to` empty: every other member.
    virtual void Send(const std::string& to, const std::string& bytes) = 0;

    // The shared file store (SharePlay's journal). Files there reach every
    // member, not one. 0 when there is none.
    virtual uint64_t MaxAttachmentBytes() const { return 0; }
    // Adds the file under `key`; the channel copies or clones it before
    // returning. False when it could not be added (the caller falls back).
    virtual bool Attach(const std::string& /*key*/, const std::string& /*path*/) { return false; }
    // Removes an attachment once every recipient has it.
    virtual void Detach(const std::string& /*key*/) {}

    // Leaves the group session. `endForEveryone`: end the activity itself,
    // as the host does, so nobody is left with a session no one hosts.
    virtual void Leave(bool endForEveryone) = 0;

    std::function<void(const std::string& from, const std::string& bytes)> onMessage;
    std::function<void(const std::vector<GroupMember>& members)> onMembers; // everyone, including this device
    // An attachment arrived; `path` is valid only during the call.
    std::function<void(const std::string& from, const std::string& key, const std::string& path)> onAttachment;
    // Asked before an attachment is downloaded: false skips it (it is for
    // another member). Unset: everything is downloaded.
    std::function<bool(const std::string& key)> wantsAttachment;
    // An attachment this device added could not reach the store after all
    // (Attach returned true, the upload failed later). The caller resends it
    // another way.
    std::function<void(const std::string& key)> onAttachFailed;
    std::function<void(const std::string& reason)> onEnded;
};

// What the platform offers: whether group sessions exist here, whether this
// device is in a call / conversation that can start one, starting one, and
// the session another member started.
class IGroupSessionSource {
public:
    virtual ~IGroupSessionSource() = default;

    // The platform supports it at all (OS version, entitlement).
    virtual bool Available() const = 0;
    // A FaceTime call (or other group) is active, so starting an activity
    // reaches people now. False: Start invites through the share sheet.
    virtual bool InGroup() const = 0;

    // Starts the activity for `advert` and, once the platform hands it back,
    // calls `ready` with this device's channel (initiator), or with nullptr
    // and a reason.
    virtual void Start(const SessionAdvert& advert,
                       std::function<void(std::unique_ptr<IGroupChannel> channel, const std::string& error)> ready) = 0;

    // A session another member started is waiting to be joined: its advert
    // (id, sequence, host name) and how to join it. Called again with an
    // empty advert id when it goes away.
    std::function<void(const SessionAdvert& advert)> onSessionOffered;
    // The session on offer now (id empty: none), for a browser that starts
    // after onSessionOffered was called.
    virtual SessionAdvert OfferedSession() const { return {}; }
    virtual void JoinOffered(const std::string& advertId,
                             std::function<void(std::unique_ptr<IGroupChannel> channel, const std::string& error)> ready) = 0;
};

// The transport and browser over `source`, for CollabNetFactory. Listen
// starts the activity (Start) and hosts on the channel it yields; Connect
// joins the offered session (JoinOffered). Both ignore the pairing code. The
// browser lists the offered session, if any. `source` outlives them.
std::unique_ptr<INetTransport> CreateGroupTransport(IGroupSessionSource& source, Dispatcher dispatch);
std::unique_ptr<ISessionBrowser> CreateGroupBrowser(IGroupSessionSource& source, Dispatcher dispatch);

// The platform's group sessions: SharePlay on Apple platforms (implemented
// in the Apple core), a source whose Available() is false elsewhere. One per
// process; created on first use.
IGroupSessionSource& PlatformGroupSessions();

// The attachment stored under `key` (as the group transport names them) is
// addressed to `member`.
bool GroupAttachmentIsFor(const std::string& key, const std::string& member);

namespace GroupTest {
// Frames a group transport dropped as malformed or out of place.
size_t MalformedFrames(INetTransport& t);
// The clock (ms) the transport times attachment acknowledgements with.
void SetClock(INetTransport& t, std::function<uint64_t()> clock);
// Where the transport keeps its own copies of files on their way.
std::string Folder(INetTransport& t);
} // namespace GroupTest

} // namespace Collab
