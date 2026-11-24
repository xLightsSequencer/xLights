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

// How session peers reach each other. The session logic (CollabSession.h)
// only talks to ICollabTransport; real transports (LAN, SharePlay) live
// outside src-core. LoopbackNetwork connects any number of peers inside one
// process for tests, with a virtual clock and optional delay, reordering,
// loss and duplication.

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "CollabMessages.h"

namespace Collab {

// A transport-level connection id. Not the same as a ParticipantId: the
// host maps connections to participants when they join.
using PeerId = uint32_t;
inline constexpr PeerId kNoPeer = 0;
using BlobId = uint64_t;

enum class PeerEvent { Joined, Left, Lost };

class ICollabTransport {
public:
    virtual ~ICollabTransport() = default;

    virtual PeerId LocalPeer() const = 0;
    // Reliable and ordered per destination on a real transport. The session
    // logic still copes with loss, duplication and reordering (it resends
    // and dedupes), so a reconnecting transport may deliver either.
    virtual void Send(PeerId to, const Message& m) = 0;
    // To every connected peer but this one.
    virtual void Broadcast(const Message& m) = 0;
    // A file (snapshot, media) sent out of band; arrives through onBlob.
    // The transport is done with `path` when this returns (it has sent or
    // copied / cloned it), so the caller may delete or change it at once.
    // Blob ids are the sender's own; a blob and the message that names it
    // may arrive in either order.
    virtual void SendBlob(PeerId to, BlobId id, const std::string& path) = 0;

    // All of these are called on the main thread, never from inside Send,
    // Broadcast or SendBlob. The file onBlob hands over is only valid during
    // the call: move it away to keep it.
    std::function<void(PeerId from, const Message& m)> onMessage;
    std::function<void(PeerId peer, PeerEvent e)> onPeer;
    std::function<void(PeerId from, BlobId id, const std::string& path)> onBlob;
    // Receiving: a blob from `from` has started or moved on (`received` of
    // `size` bytes so far), at least every kBlobProgressBytes. A transfer
    // that has stopped reporting has stalled; one that reports is not to be
    // asked for again.
    std::function<void(PeerId from, BlobId id, uint64_t received, uint64_t size)> onBlobProgress;
    // Receiving: a blob from `from` arrived but couldn't be saved here (a
    // local disk problem, not the peer's fault). The connection stays up; the
    // session asks for the file again once it sees the transfer stall.
    std::function<void(PeerId from, BlobId id, const std::string& why)> onBlobFailed;
    // Sending: the transport is done with a blob given to SendBlob: all of
    // it went out (`sent`), or none or only part of it ever will. Every
    // SendBlob gets exactly one, unless the peer goes away first.
    std::function<void(PeerId to, BlobId id, bool sent)> onBlobDone;
    // A message that could not be sent at all (larger than a frame may be).
    // `m` holds its type, rev, clientSeq and blob; `to` is kNoPeer for a
    // Broadcast.
    std::function<void(PeerId to, const Message& m, const std::string& reason)> onSendFailed;
};

inline constexpr uint64_t kBlobProgressBytes = 1u << 20;
// The largest encoded message a transport sends; larger ones fail
// (onSendFailed). Snapshots, layouts and files travel as blobs instead.
inline constexpr size_t kMaxMessageBytes = 64u << 20;

// What onSendFailed carries of a message that could not go.
Message SendFailedSummary(const Message& m);

struct LoopbackOptions {
    uint32_t seed = 1;
    uint64_t minDelayMs = 0;
    uint64_t maxDelayMs = 0;
    // When false, frames between one pair of peers may overtake each other
    // (beyond what a real transport does).
    bool fifoPerLink = true;
    double dropRate = 0.0;      // chance a frame is lost
    double duplicateRate = 0.0; // chance a frame is delivered twice
    // Blob throughput per link, bytes per ms; 0 = a blob arrives whole after
    // the usual delay. With a rate, blobs on one link go one after another
    // and onBlobProgress reports along the way.
    uint64_t blobBytesPerMs = 0;
    size_t maxMessageBytes = kMaxMessageBytes;
};

class LoopbackNetwork;

class LoopbackTransport : public ICollabTransport {
public:
    PeerId LocalPeer() const override { return _id; }
    void Send(PeerId to, const Message& m) override;
    void Broadcast(const Message& m) override;
    void SendBlob(PeerId to, BlobId id, const std::string& path) override;

private:
    friend class LoopbackNetwork;
    LoopbackTransport(LoopbackNetwork& net, PeerId id) : _net(net), _id(id) {}
    LoopbackNetwork& _net;
    PeerId _id;
};

// Frames travel encoded (EncodeMessage) and are decoded on delivery, so the
// framing is exercised exactly as on a wire. Nothing is delivered until the
// owner calls Step / RunUntil / RunUntilIdle.
class LoopbackNetwork {
public:
    explicit LoopbackNetwork(LoopbackOptions opts = {});
    ~LoopbackNetwork();

    LoopbackTransport& AddPeer();
    // The peer goes away abruptly: the others get onPeer(Lost), frames to
    // and from it are dropped.
    void Disconnect(PeerId peer);
    bool IsConnected(PeerId peer) const;

    uint64_t Now() const { return _now; }
    // Delivers the next frame (advancing the clock to it). False when none.
    bool Step();
    // Delivers everything due at or before `time`, then sets the clock there.
    void RunUntil(uint64_t time);
    // Delivers until nothing is in flight or `maxFrames` were delivered.
    // Returns false if it stopped on the limit.
    bool RunUntilIdle(size_t maxFrames = 1000000);
    size_t InFlight() const { return _queue.size(); }
    uint64_t NextDeliveryTime() const;

    // Runs around every delivery to `to` (tests use it to act as that peer).
    std::function<void(PeerId to, const std::function<void()>& deliver)> deliveryWrapper;
    // True: the blob `from` sends `to` is lost without any report (no
    // delivery, no onBlobDone), as a transport that drops a transfer
    // silently would.
    std::function<bool(PeerId from, PeerId to, BlobId id)> loseBlob;
    // True: the blob arrives but `to` can't save it (onBlobFailed instead of
    // onBlob), as after a local disk error.
    std::function<bool(PeerId from, PeerId to, BlobId id)> failBlobSave;
    // Called with each frame as it is sent; may rewrite or clear it (an
    // empty frame is dropped). For corruption tests.
    std::function<void(PeerId from, PeerId to, std::string& frame)> tamper;

    LoopbackOptions& Options() { return _opts; }
    // Blobs in flight are copies kept here (made when sent, deleted after
    // delivery), as a real transport would hand over its own copy.
    const std::string& BlobFolder();
    size_t BlobsDelivered() const { return _blobsDelivered; }
    // Blobs given to SendBlob by `from` (delivered or not).
    size_t BlobsSent(PeerId from) const {
        auto it = _blobsSent.find(from);
        return it == _blobsSent.end() ? 0 : it->second;
    }
    size_t Delivered() const { return _delivered; }
    size_t Dropped() const { return _dropped; }

private:
    friend class LoopbackTransport;
    struct Frame {
        PeerId from = kNoPeer;
        PeerId to = kNoPeer;
        std::string bytes;
        bool blob = false;
        BlobId blobId = 0;
        bool event = false;
        PeerEvent peerEvent = PeerEvent::Joined;
        // Notifications. Progress: to the receiver about `from`'s blob.
        // Done / SendFailed: to the sender (`from`) about what it sent `to`.
        enum class Note { None, Progress, Done, SendFailed };
        Note note = Note::None;
        uint64_t received = 0;
        uint64_t size = 0;
        bool ok = false;
        Message failed;
    };
    void Enqueue(PeerId from, PeerId to, std::string bytes);
    void EnqueueEvent(PeerId to, PeerId about, PeerEvent e);
    void Push(Frame f, uint64_t at);
    void NotifySendFailed(PeerId from, PeerId to, const Message& m, size_t bytes);
    void Deliver(const Frame& f);
    uint64_t RandomDelay();

    LoopbackOptions _opts;
    std::mt19937_64 _rng;
    uint64_t _now = 0;
    uint64_t _order = 0;
    // (deliverAt, order) -> frame
    std::map<std::pair<uint64_t, uint64_t>, Frame> _queue;
    std::map<std::pair<PeerId, PeerId>, uint64_t> _linkLast;
    std::vector<std::unique_ptr<LoopbackTransport>> _peers;
    std::map<PeerId, bool> _connected;
    size_t _delivered = 0;
    size_t _dropped = 0;
    size_t _blobsDelivered = 0;
    std::map<PeerId, size_t> _blobsSent;
    std::map<std::pair<PeerId, PeerId>, uint64_t> _blobLinkFree; // when the link's last blob is through
    std::string _blobFolder;
    uint64_t _blobCopies = 0;
};

} // namespace Collab
